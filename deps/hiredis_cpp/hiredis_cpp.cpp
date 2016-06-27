#include "hiredis_cpp.h"
#include "redishelper.h"

#include "../hiredis/win32_hiredis.h"
#include "../hiredis/async.h"
//#include "../hiredis/adapters/ae.h"

#include <iostream>
#include <assert.h>
#include <functional>

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// class HiredisCpp
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
HiredisCpp::HiredisCpp() :
	m_p_redis_ctx(0),
	m_p_pubsub_ctx(0)
{
	m_mutex_redis_ctx = CreateMutex(NULL,		// default security attributes
									FALSE,      // initially not owned
									NULL);      // unnamed mutex

	m_mutex_pubsub_ctx = CreateMutex(NULL,		// default security attributes
									FALSE,      // initially not owned
									NULL);      // unnamed mutex
}

HiredisCpp::~HiredisCpp()
{
	if (m_mutex_pubsub_ctx)
	{
		CloseHandle(m_mutex_pubsub_ctx);
		m_mutex_pubsub_ctx = 0;
	}
	if (m_mutex_redis_ctx)
	{
		CloseHandle(m_mutex_redis_ctx);
		m_mutex_redis_ctx = 0;
	}
	disconnect();
	if (m_p_redis_ctx != 0)
	{
		delete m_p_redis_ctx;
		m_p_redis_ctx = 0;
	}
	if (m_p_pubsub_ctx != 0)
	{
		delete m_p_pubsub_ctx;
		m_p_pubsub_ctx = 0;
	}
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool HiredisCpp::connect(const std::string &in_host, const int in_port, const bool in_blocking, const int in_timeout_sec)
{
	if (in_host.empty()) return false;
	if (in_port == 0) return false;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	if (m_p_redis_ctx == 0)
		m_p_redis_ctx = new RedisContext();

	if (m_p_redis_ctx == 0) return false;

	return m_p_redis_ctx->connect(in_host, in_port, in_blocking, in_timeout_sec);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void* HiredisCpp::connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback, RedisCallback* in_disconnect_callback)
{
	if (in_host.empty()) return 0;
	if (in_port == 0) return 0;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return 0;

	if (m_p_redis_ctx == 0)
		m_p_redis_ctx = new RedisContext();

	void* thread_handle = 0;
	if (m_p_redis_ctx == 0) return 0;
	if ((thread_handle = m_p_redis_ctx->connectAsync(in_host, in_port)) == 0) return 0;

	RedisPrivateData& rpd = RedisPrivateData::getInstance();
	rpd.setConnectCallback(in_connect_callback);
	rpd.setDisconnectCallback(in_disconnect_callback);

	return thread_handle;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::disconnect()
{
	MutexLocker rlocker(m_mutex_redis_ctx);
	if (rlocker.isLocked() == false) return;

	MutexLocker plocker(m_mutex_pubsub_ctx);
	if (plocker.isLocked() == false) return;

	if (m_p_redis_ctx != 0)
		m_p_redis_ctx->disconnect();

	if (m_p_pubsub_ctx != 0)
		m_p_pubsub_ctx->disconnect();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::setTimeout(const int in_seconds)
{
	TIMEVAL tv = {in_seconds, 0};
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	if (m_p_redis_ctx->isValid() == false) return REDIS_ERR;
	return redisSetTimeout(m_p_redis_ctx->m_context.hiredis_ctx, tv);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::enableKeepAlive()
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;
	
	if (m_p_redis_ctx->isValid() == false) return REDIS_ERR;
	return redisEnableKeepAlive(m_p_redis_ctx->m_context.hiredis_ctx);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::exec(const std::string &in_command_string, RedisCallback* in_callback, void* in_pdata)
{
	if (in_command_string.empty()) return 0;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return 0;

	// check context ...
	if (m_p_redis_ctx->isConnected() == false) return 0;

	// basic check if command string is valid ...
	if (checkCommandString(in_command_string) == false) return 0;

	// create RedisCommand ...
	RedisCommand* command = prepareCommand(in_command_string, in_callback, in_pdata);
	if (command == 0) return 0;

	// exec RedisCommand ...
	return execCommand(command, in_callback);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::exec(const std::vector<std::string> &in_command_vector)
{
	if (in_command_vector.size() <= 0) return 0;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return 0;

	// check context ...
	// pipeline commands are only available in sync blocking mode ...
	if (m_p_redis_ctx->isConnected() == false) return 0;
	if (m_p_redis_ctx->isBlocking() == false) return 0;

	RedisReply* rep = 0;
	
	std::string command_string;
	int rv = REDIS_OK;

	// append commands to output buffer ...
	for (int i=0; i<in_command_vector.size(); ++i)
	{
		command_string = in_command_vector.at(i);
		if (command_string.empty()) continue;

		// basic check if command string is valid ...
		if (checkCommandString(command_string) == false) continue;

		rv = redisAppendCommand(m_p_redis_ctx->m_context.hiredis_ctx, command_string.data());
		if (rv != REDIS_OK) continue;
	}

	rep = getReply();
	return rep;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::subscribe(const std::string &in_channel, RedisCallback* in_connect_callback )
{
	return 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::subscribe(const std::vector<std::string> &in_channel_vector, RedisCallback* in_connect_callback)
{
	return 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::unsubscribe(const std::string &in_channel)
{
	return 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::unsubscribe(const std::vector<std::string> &in_channel_vector)
{
	return 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::publish(const std::string &in_channel, const std::string &in_msg)
{
	if (in_channel.empty()) return 0;
	if (in_msg.empty()) return 0;

	MutexLocker locker(m_mutex_pubsub_ctx);
	if (locker.isLocked() == false) return false;

	// check context ...
	if (m_p_redis_ctx->isConnected() == false) return 0;

	RedisCommand* command = prepareCommand("", 0, 0);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCommand* HiredisCpp::prepareCommand(const std::string &in_command_string, RedisCallback* in_callback, void* in_pdata)
{
	RedisCommand* command = new RedisCommand(in_command_string);
	if (command == 0) return 0;
	
	return command;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool HiredisCpp::checkCommandString(const std::string& in_command_string)
{
	if (in_command_string.empty()) return false;

	std::string buffer(in_command_string);
	std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);

	// PUB/SUB commands will get particular attention ...
	if (!buffer.compare(0, PUBSUB_CMD.size(), PUBSUB_CMD))
		return false;
	else if (!buffer.compare(0, PUBLISH_CMD.size(), PUBLISH_CMD))
		return false;
	else if(!buffer.compare(0, SUBSCRIBE_CMD.size(), SUBSCRIBE_CMD))
		return false;
	else if (!buffer.compare(0, PSUBSCRIBE_CMD.size(), PSUBSCRIBE_CMD))
		return false;
	else if (!buffer.compare(0, UNSUBSCRIBE_CMD.size(), UNSUBSCRIBE_CMD))
		return false;
	else if (!buffer.compare(0, PUNSUBSCRIBE_CMD.size(), PUNSUBSCRIBE_CMD))
		return false;

	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::execCommand(RedisCommand *in_command, RedisCallback* in_callback)
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;
	
	redisReply* reply = 0;
	if (m_p_redis_ctx->m_is_async == false)
	{
		reply = static_cast<redisReply*>(redisCommand(m_p_redis_ctx->m_context.hiredis_ctx, in_command->m_command_string.data()));
		RedisReply* rep = RedisReply::createReply(reply);

		return rep;
	}
	else
	{
		redisAsyncCommand(m_p_redis_ctx->m_context.hiredis_async_ctx, (redisCallbackFn*)RedisCallback::backendCommandCallback, &in_callback->m_priv_data, in_command->m_command_string.data());
		return 0;
	}
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::getReply()
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	if (m_p_redis_ctx->isValid() == false) return 0;

	// check if there is any return data ...
	std::vector<RedisReply*> replies = getPendingReplies(false);
	if (replies.size() <= 0) return 0;

	// create envelop for reply array ...
	RedisReply* ret = new RedisReply();
	if (ret == 0) return 0;

	ret->m_reply_data.m_arr = replies;
	ret->m_reply_data.m_type = REDIS_REPLY_TYPE_ARRAY;

	return ret;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::discardReply()
{
	getPendingReplies(true);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::writePendingCommands()
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	// check context ...
	if (m_p_redis_ctx->isValid() == false) return 0;
	if (m_p_redis_ctx->isBlocking() == true) return 0;	// will be done by __redisBlockForReply
	if (m_p_redis_ctx->isAsync() == true) return 0;	// will be done by event loop

	int done = 0;
	// write whole buffer ...
    do {
        if (redisBufferWrite(m_p_redis_ctx->m_context.hiredis_ctx, &done) == REDIS_ERR)
            return REDIS_ERR;
    } while (!done);

	return REDIS_OK;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::vector<RedisReply*> HiredisCpp::getPendingReplies(const bool in_discard)
{
	int rv = REDIS_OK;
	redisReply* reply = 0;
	std::vector<RedisReply*> ret;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return ret;

	if (m_p_redis_ctx->isValid() == false) return ret;
	//if (m_redis_ctx.isAsync() == true) return ret;

	// get replies from input buffer if any available ...
	while (rv == REDIS_OK)
	{
		if (m_p_redis_ctx->isBlocking())
			rv = redisGetReply(m_p_redis_ctx->m_context.hiredis_ctx, (void**)&reply);
		else
			rv = readRedisBuffer(&reply);

		if (reply == 0) break;

		// if not to drop all replies ...
		if (in_discard == false)
		{
			RedisReply* rep = RedisReply::createReply(reply);
			if (rep == 0) continue;

			ret.push_back(rep);
		}

		// nothing more to read ...
		if (m_p_redis_ctx->m_context.hiredis_ctx->reader->pos == m_p_redis_ctx->m_context.hiredis_ctx->reader->len) 
			break;
	}
	return ret;
}

// ----------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------
int HiredisCpp::readRedisBuffer(redisReply** out_reply)
{
	// reset output ...
	if (out_reply != NULL) *out_reply = NULL;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return REDIS_ERR;

	// check context ...
	if (m_p_redis_ctx->isValid() == false) return REDIS_ERR;
	if (m_p_redis_ctx->isAsync() == true) return REDIS_ERR;
	
	// get data from socket into reader ...
    if (redisBufferRead(m_p_redis_ctx->m_context.hiredis_ctx) == REDIS_ERR) 
		return REDIS_ERR;

	// get parsed reply from reader ...
    if (redisGetReplyFromReader(m_p_redis_ctx->m_context.hiredis_ctx, (void**)out_reply) == REDIS_ERR)
        return REDIS_ERR;

	return REDIS_OK;
}

// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void HiredisCpp::freeRedisReply(RedisReply* in_reply)
{
	if (in_reply == 0) return;

	in_reply->cleanup();
	delete in_reply;
}

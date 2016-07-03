#pragma warning(push)
#pragma warning(disable: 4251) // class 'std::vector<_Ty>' needs to have dll-interface
#include "hiredis_cpp.h"
#include "redishelper.h"
#pragma warning(pop)

#ifdef _WIN32
#include "../hiredis/win32_hiredis.h"
#else
#include "../hiredis/hiredis.h"
#endif
#include "../hiredis/async.h"

#ifdef ERROR_ON_WRONG_CALLBACK_TYPE
#undef ERROR_ON_WRONG_CALLBACK_TYPE
#endif
#define ERROR_ON_WRONG_CALLBACK_TYPE 1

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// class HiredisCpp
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
HiredisCpp::HiredisCpp() 
{
#ifdef _WIN32
	m_mutex_redis_ctx = CreateMutex(NULL,		// default security attributes
									FALSE,      // initially not owned
									NULL);      // unnamed mutex

	m_mutex_pubsub_ctx = CreateMutex(NULL,		// default security attributes
									FALSE,      // initially not owned
									NULL);      // unnamed mutex
#else
	m_mutex_redis_ctx = new pthread_mutex_t;
	if (m_mutex_redis_ctx != 0)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(m_mutex_redis_ctx, &attr);
		pthread_mutexattr_destroy(&attr);
	}

	m_mutex_pubsub_ctx = new pthread_mutex_t;
	if (m_mutex_pubsub_ctx != 0)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(m_mutex_pubsub_ctx, &attr);
		pthread_mutexattr_destroy(&attr);
	}
#endif
}

HiredisCpp::~HiredisCpp()
{
	disconnect();

	if (m_mutex_pubsub_ctx)
	{
#ifdef _WIN32
		CloseHandle(m_mutex_pubsub_ctx);
#else
		pthread_mutex_destroy(m_mutex_pubsub_ctx);
		delete m_mutex_pubsub_ctx;
#endif
		m_mutex_pubsub_ctx = 0;
	}
	if (m_mutex_redis_ctx)
	{
#ifdef _WIN32
		CloseHandle(m_mutex_redis_ctx);
#else
		pthread_mutex_destroy(m_mutex_redis_ctx);
		delete m_mutex_redis_ctx;
#endif
		m_mutex_redis_ctx = 0;
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

	connectPubSub(in_host, in_port);
	
	return m_redis_ctx.connect(in_host, in_port, in_blocking, in_timeout_sec);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void* HiredisCpp::connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback, RedisCallback* in_disconnect_callback)
{
	if (in_host.empty()) return 0;
	if (in_port == 0) return 0;

	if (in_connect_callback != 0 && in_connect_callback->callbackType() != REDIS_CALLBACK_TYPE_STATUS)
	{
#if ERROR_ON_WRONG_CALLBACK_TYPE
		std::cout << "ERROR: connect callback is not a status callback! Abort ..." << std::endl;	// TODO: error handling & logging ...
		return 0;
#else
		std::cout << "WARNING: connect callback is not a status callback! Callback will be reseted ..." << std::endl;	// TODO: error handling & logging ...
		in_connect_callback = 0;
#endif
	}
	if (in_disconnect_callback != 0 && in_disconnect_callback->callbackType() != REDIS_CALLBACK_TYPE_STATUS)
	{
#if ERROR_ON_WRONG_CALLBACK_TYPE
		std::cout << "ERROR: disconnect callback is not a status callback! Abort ..." << std::endl;	// TODO: error handling & logging ...
		return 0;
#else
		std::cout << "WARNING: disconnect callback is not a status callback! Callback will be reseted ..." << std::endl;	// TODO: error handling & logging ...
		in_disconnect_callback = 0;
#endif
	}

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return 0;

	connectPubSub(in_host, in_port);

	return m_redis_ctx.connectAsync(in_host, in_port, in_connect_callback, in_disconnect_callback);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void* HiredisCpp::connectPubSub(const std::string &in_host, const int in_port)
{	
	MutexLocker locker(m_mutex_pubsub_ctx);
	if (locker.isLocked() == false) return false;

	void* thread_handle = m_pubsub_ctx.connectAsync(in_host, in_port, 0, 0);
	
	if (WaitForSingleObject(thread_handle, 100) != WAIT_TIMEOUT)
	{
		m_pubsub_ctx.disconnect();
		thread_handle = 0;
	}
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

	m_redis_ctx.disconnect();
	m_pubsub_ctx.disconnect();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::setTimeout(const int in_seconds)
{
	TIMEVAL tv = {in_seconds, 0};
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	if (m_redis_ctx.isValid() == false) return REDIS_ERR;
	return redisSetTimeout(m_redis_ctx.m_context.hiredis_ctx, tv);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::enableKeepAlive()
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;
	
	if (m_redis_ctx.isValid() == false) return REDIS_ERR;
	return redisEnableKeepAlive(m_redis_ctx.m_context.hiredis_ctx);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::exec(const RedisCommand& in_command)
{
	if (in_command.isValid() == false) return 0;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return 0;

	// check context ...
	if (m_redis_ctx.isConnected() == false) return 0;

	// check command ...
	if (checkCommand(in_command) == false) return 0;

	// exec RedisCommand ...
	return execCommand(m_redis_ctx, in_command);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::exec(const std::vector<RedisCommand*> &in_command_vector)
{
	if (in_command_vector.size() <= 0) return 0;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return 0;

	// check context ...
	if (m_redis_ctx.isConnected() == false) return 0;
	// pipeline commands are only available in sync blocking mode ...
	if (m_redis_ctx.isBlocking() == false) return 0;

	RedisReply* rep = 0;
	
	RedisCommand* command;
	int rv = REDIS_OK;

	// append commands to output buffer ...
	for (unsigned int i=0; i<in_command_vector.size(); ++i)
	{
		command = in_command_vector.at(i);
		if (command == 0 || command->isValid() == false) continue;

		// check command ...
		if (checkCommand(*command) == false) return 0;

		rv = redisAppendCommand(m_redis_ctx.m_context.hiredis_ctx, command->getCommand().c_str());
		if (rv != REDIS_OK) continue;
	}

	rep = getReply();
	return rep;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::subscribe(const std::string &in_channel, RedisCallback* in_message_callback )
{
	if (in_channel.empty()) return;

	MutexLocker locker(m_mutex_pubsub_ctx);
	if (locker.isLocked() == false) return;

	if (m_pubsub_ctx.isConnected() == false) return;

	RedisCommand command("SUBSCRIBE " + in_channel);
	command.m_p_callback = in_message_callback;

	execCommand(m_pubsub_ctx, command);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::subscribe(const std::vector<std::string> &in_channel_vector, RedisCallback* in_message_callback)
{
	std::string channel;
	for (unsigned int i = 0; i<in_channel_vector.size(); ++i)
	{
		channel = in_channel_vector[i];
		if (channel.empty()) continue;

		subscribe(channel, in_message_callback);
	}
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::unsubscribe(const std::string &in_channel)
{
	if (in_channel.empty()) return;

	MutexLocker locker(m_mutex_pubsub_ctx);
	if (locker.isLocked() == false) return;

	if (m_pubsub_ctx.isConnected() == false) return;

	RedisCommand command("UNSUBSCRIBE " + in_channel);

	execCommand(m_pubsub_ctx, command);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::unsubscribe(const std::vector<std::string> &in_channel_vector)
{
	std::string channel;
	for (unsigned int i = 0; i<in_channel_vector.size(); ++i)
	{
		channel = in_channel_vector[i];
		if (channel.empty()) continue;

		unsubscribe(channel);
	}
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::publish(const std::string &in_channel, const std::string &in_msg)
{
	if (in_channel.empty()) return;
	if (in_msg.empty()) return;

	MutexLocker locker(m_mutex_pubsub_ctx);
	if (locker.isLocked() == false) return;

	if (m_pubsub_ctx.isConnected() == false) return;

	RedisCommand command("PUBLISH " + in_channel + " %s");
	command << in_msg;

	execCommand(m_pubsub_ctx, command);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::publish(const std::vector<std::string> &in_channel_vector, const std::string &in_msg)
{
	std::string channel;
	for (unsigned int i = 0; i<in_channel_vector.size(); ++i)
	{
		channel = in_channel_vector[i];
		if (channel.empty()) continue;

		publish(channel, in_msg);
	}
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool HiredisCpp::checkCommand(const RedisCommand& in_command)
{
	if (in_command.isValid() == false) return false;	

	std::string buffer(in_command.getCommand());
	if (buffer.empty()) return false;

	std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);

	// filter pub/sub commands ...
	// PUB/SUB commands will be executed in separat context ...
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
RedisReply* HiredisCpp::execCommand(const RedisContext& in_context, const RedisCommand &in_command)
{
	// prepare command string ...
	std::string command = in_command.getCommand();
	if (command.empty()) return 0;
	// prepare arguments ...
	va_list args = in_command.createArgVaList();
	// prepare private data ...
	void* pdata = 0;
	if (in_command.m_p_callback != 0)
		pdata = &in_command.m_p_callback->m_priv_data;

	redisReply* reply = 0;
	if (in_context.m_is_async == false)
	{
		reply = static_cast<redisReply*>(redisvCommand(in_context.m_context.hiredis_ctx, command.c_str(), args));
		in_command.freeArgVaList(args);
		
		return RedisReply::createReply(reply);
	}
	else
	{
		if (in_command.m_p_callback != 0 && in_command.m_p_callback->callbackType() != REDIS_CALLBACK_TYPE_COMMAND)
		{
#if ERROR_ON_WRONG_CALLBACK_TYPE
			std::cout << "ERROR: \"" << command << "\" callback is not a command callback! Abort ..." << std::endl;	// TODO: error handling & logging ...
			return 0;
#else
			std::cout << "WARNING: \"" << command << "\" callback is not a command callback! Callback will be reseted ..." << std::endl;	// TODO: error handling & logging ...
			const_cast<RedisCommand &>(in_command).m_p_callback = 0;
#endif
		}

		redisvAsyncCommand(in_context.m_context.hiredis_async_ctx, (redisCallbackFn*)RedisCallback::backendCommandCallback, pdata, command.c_str(), args);
		in_command.freeArgVaList(args);

		usleep(1000); // thread switch (hopefully) ...
		
		// async mode - no reply ...
	}
	return 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::writePendingCommands()
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	// check context ...
	if (m_redis_ctx.isValid() == false) return 0;
	if (m_redis_ctx.isConnected() == false) return 0;
	// write commands only in sync non-blocking mode ...
	if (m_redis_ctx.isBlocking() == true) return 0;	// will be done by __redisBlockForReply ...
	if (m_redis_ctx.isAsync() == true) return 0;	// will be done by event loop ...

	int done = 0;
	// write whole buffer ...
    do {
        if (redisBufferWrite(m_redis_ctx.m_context.hiredis_ctx, &done) == REDIS_ERR)
            return REDIS_ERR;
    } while (!done);

	usleep(1000); // it takes some time to get all the replies back from server ;-)

	return REDIS_OK;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::getReply()
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	if (m_redis_ctx.isValid() == false) return 0;
	if (m_redis_ctx.isConnected() == false) return 0;

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
std::vector<RedisReply*> HiredisCpp::getPendingReplies(const bool in_discard)
{
	int rv = REDIS_OK;
	redisReply* reply = 0;
	std::vector<RedisReply*> ret;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return ret;

	if (m_redis_ctx.isValid() == false) return ret;
	if (m_redis_ctx.isConnected() == false) return ret;

	// get replies from input buffer if any available ...
	while (rv == REDIS_OK)
	{
		if (m_redis_ctx.isBlocking())
			rv = redisGetReply(m_redis_ctx.m_context.hiredis_ctx, (void**)&reply);
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
		if (m_redis_ctx.m_context.hiredis_ctx->reader->pos == m_redis_ctx.m_context.hiredis_ctx->reader->len) 
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
	if (m_redis_ctx.isValid() == false) return REDIS_ERR;
	if (m_redis_ctx.isConnected() == false) return REDIS_ERR;
	if (m_redis_ctx.isAsync() == true) return REDIS_ERR;
	
	// get data from socket into reader ...
    if (redisBufferRead(m_redis_ctx.m_context.hiredis_ctx) == REDIS_ERR) 
		return REDIS_ERR;

	// get parsed reply from reader ...
    if (redisGetReplyFromReader(m_redis_ctx.m_context.hiredis_ctx, (void**)out_reply) == REDIS_ERR)
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

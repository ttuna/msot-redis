#include "hiredis_cpp.h"

#include "../hiredis/win32_hiredis.h"
#include "../hiredis/async.h"
#include "../hiredis/adapters/ae.h"

#include <iostream>
#include <assert.h>
#include <functional>

const int CONNECT_TIMEOUT_SEC = 5;
const int MAX_COMMAND_ENTRIES = 100;
const std::string SUBSCRIBE_CMD = "subscribe";

namespace HIREDIS_CPP {

class AsyncConnectThreadData;

// ----------------------------------------------------------------------------
//
// MutexLocker
//
// ----------------------------------------------------------------------------
class MutexLocker
{
public:
	MutexLocker(HANDLE in_mutex, const DWORD in_timeout_ms = INFINITE);
	~MutexLocker();

	bool isLocked();
	bool unlock();
	bool relock();

private:
	MutexLocker(const MutexLocker& other);
	MutexLocker& operator=(const MutexLocker&);
	HANDLE m_mutex;
	DWORD m_locked;
	DWORD m_timeout;
};
// ----------------------------------------------------------------------------
MutexLocker::MutexLocker(HANDLE in_mutex, const DWORD in_timeout_ms) :
	m_mutex(in_mutex),
	m_timeout(in_timeout_ms),
	m_locked(0)
{
	if (m_mutex != 0)
		m_locked = WaitForSingleObject(m_mutex, m_timeout);
}
// ----------------------------------------------------------------------------
MutexLocker::~MutexLocker()
{
	if (m_mutex && m_locked == WAIT_OBJECT_0)
		ReleaseMutex(m_mutex);
}
// ----------------------------------------------------------------------------
bool MutexLocker::isLocked()
{
	return (m_mutex != 0 && m_locked == WAIT_OBJECT_0);
}
// ----------------------------------------------------------------------------
bool MutexLocker::unlock()
{
	BOOL ret = FALSE;
	if (m_mutex && m_locked == WAIT_OBJECT_0)
		ret = ReleaseMutex(m_mutex);

	return (ret == TRUE);
}
// ----------------------------------------------------------------------------
bool MutexLocker::relock()
{
	if (m_mutex && m_locked != WAIT_OBJECT_0)
		m_locked = WaitForSingleObject(m_mutex, m_timeout);

	return (m_locked == WAIT_OBJECT_0);
}


// forward declaration ...
class HiredisCpp;

// ----------------------------------------------------------------------------
//
// AsyncConnectThreadContext
//
// ----------------------------------------------------------------------------
class AsyncConnectThreadContext
{
	friend class HiredisCpp;
public:
	AsyncConnectThreadContext();
	virtual ~AsyncConnectThreadContext();
	bool isValid() const;
	bool prepareThreadLoop();
	bool execThreadLoop();
	bool stopThreadLoop();

// ----------------------------------------------------------------------------
	std::string getHost() const
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return std::string("");
		return m_host;
	}
// ----------------------------------------------------------------------------
	void setHost(const std::string& in_host)
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return;
		m_host = in_host;
	}
// ----------------------------------------------------------------------------
	int getPort() const
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return 0;
		return m_port;
	}
// ----------------------------------------------------------------------------
	void setPort(const int in_port)
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return;
		m_port = in_port;
	}

private:
	HANDLE m_mutex_handle;
	HiredisCpp* m_p_client;
	aeEventLoop *m_p_event_loop;
	std::string m_host;
	int m_port;
};
// ----------------------------------------------------------------------------
AsyncConnectThreadContext::AsyncConnectThreadContext() :
	m_p_client(0),
	m_mutex_handle(0),
	m_port(0)
{
	m_mutex_handle = CreateMutex(NULL,              // default security attributes
								 FALSE,             // initially not owned
								 NULL);             // unnamed mutex
}
// ----------------------------------------------------------------------------
AsyncConnectThreadContext::~AsyncConnectThreadContext()
{	
	if (m_mutex_handle)
	{
		CloseHandle(m_mutex_handle);
		m_mutex_handle = 0;
	}

	if (m_p_event_loop)
	{
		aeStop(m_p_event_loop);
		aeDeleteEventLoop(m_p_event_loop);
	}
}
// ----------------------------------------------------------------------------
bool AsyncConnectThreadContext::isValid() const
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return false;

	if (m_mutex_handle == 0) return false;
	if (m_p_client == 0) return false;
	if (m_host.empty()) return false;
	if (m_port <= 0) return false;

	return true;
}
// ----------------------------------------------------------------------------
bool AsyncConnectThreadContext::prepareThreadLoop()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return false;
	if (m_p_client == 0) return false;

	// !!! event loop must be created before redisAsyncConnect ... !!!
	m_p_event_loop = aeCreateEventLoop(1024 * 10);
	if (m_p_event_loop == 0) return false;

	// async connect to redis ...
	redisAsyncContext* ctx = redisAsyncConnect(m_host.data(), m_port);
	if (ctx == 0) return false;

	ctx->data = m_p_client;
	m_p_client->m_redis_ctx.m_context.hiredis_async_ctx = ctx;
	m_p_client->m_redis_ctx.m_is_async = true;

	redisAeAttach(m_p_event_loop, ctx);
	redisAsyncSetConnectCallback(ctx, &m_p_client->backendConnectCallback);
	redisAsyncSetDisconnectCallback(ctx, &m_p_client->backendDisconnectCallback);
}
// ----------------------------------------------------------------------------
bool AsyncConnectThreadContext::execThreadLoop()
{
	//MutexLocker locker(m_mutex_handle);
	//if (locker.isLocked() == false) return false;
	if (m_p_event_loop == 0) return false;

	aeMain(m_p_event_loop);	// blocking ...
	return true;
}
// ----------------------------------------------------------------------------
bool AsyncConnectThreadContext::stopThreadLoop()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return false;
	if (m_p_event_loop == 0) return false;

	aeStop(m_p_event_loop);
	return true;
}


// ----------------------------------------------------------------------------
// thread function ...
// ----------------------------------------------------------------------------
DWORD WINAPI AsyncConnectThreadFunction(LPVOID in_param)
{
	if (in_param == 0) return -1;

	AsyncConnectThreadContext* thread_ctx = (AsyncConnectThreadContext*)(in_param);
	if (thread_ctx == 0 || thread_ctx->isValid() == false) return -1;
	if (thread_ctx->prepareThreadLoop() == false) return -1;
	if (thread_ctx->execThreadLoop() == false) return -1;

	return 0;
};

} // namespace HIREDIS_CPP

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
HiredisCpp::HiredisCpp() :
	m_p_connect_callback(0),
	m_p_disconnect_callback(0),
	m_p_thread_ctx(0),
	m_thread_handle(0),
	m_thread_id(0)
{
	m_mutex_thread_ctx = CreateMutex(NULL,		// default security attributes
									FALSE,      // initially not owned
									NULL);      // unnamed mutex

	m_mutex_redis_ctx = CreateMutex(NULL,		// default security attributes
									FALSE,      // initially not owned
									NULL);      // unnamed mutex
}

HiredisCpp::~HiredisCpp()
{
	if (m_mutex_redis_ctx)
	{
		CloseHandle(m_mutex_redis_ctx);
		m_mutex_redis_ctx = 0;
	}
	if (m_mutex_thread_ctx)
	{
		CloseHandle(m_mutex_thread_ctx);
		m_mutex_thread_ctx = 0;
	}
	disconnect();
	m_redis_ctx.cleanup();
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

	redisContext* ctx = 0;
	if (in_blocking)
	{
		TIMEVAL tv = {(in_timeout_sec == -1) ? CONNECT_TIMEOUT_SEC : in_timeout_sec, 0};
		ctx = redisConnectWithTimeout(in_host.data(), in_port, tv);
	}
	else
	{
		ctx = redisConnectNonBlock(in_host.data(), in_port);
	}
	if (ctx == 0) return false;

	m_redis_ctx.cleanup();
	m_redis_ctx.m_context.hiredis_ctx = ctx;
	m_redis_ctx.m_is_async = false;
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void* HiredisCpp::connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback, RedisCallback* in_disconnect_callback)
{
	if (in_host.empty()) return false;
	if (in_port == 0) return false;

	MutexLocker locker(m_mutex_thread_ctx);
	if (locker.isLocked() == false) return 0;

	m_p_thread_ctx = new AsyncConnectThreadContext;
	if (m_p_thread_ctx == 0) return false;

	m_p_connect_callback = in_connect_callback ;
	m_p_disconnect_callback = in_disconnect_callback;

	m_p_thread_ctx->m_p_client = this;
	m_p_thread_ctx->m_host = in_host;
	m_p_thread_ctx->m_port = in_port;

	if (locker.unlock() == false) return 0;

	m_thread_handle = CreateThread( 
							NULL,						// default security attributes
							0,							// use default stack size  
							AsyncConnectThreadFunction, // thread function
							m_p_thread_ctx,				// argument to thread function 
							0,							// use default creation flags 
							&m_thread_id);				// returns the thread identifier

	return m_thread_handle;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool HiredisCpp::disconnect()
{
	// clear async thread_ctx
	if (m_p_thread_ctx != 0)
	{
		m_p_thread_ctx->stopThreadLoop();
		delete m_p_thread_ctx;
		m_p_thread_ctx = 0;
	}
	if (m_thread_handle)
	{
		CloseHandle(m_thread_handle);
		m_thread_handle = 0;
	}
	m_thread_id = 0;
	m_p_connect_callback = 0;
	m_p_disconnect_callback = 0;
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::setTimeout(const int in_seconds)
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	if (m_redis_ctx.isValid() == false) return REDIS_ERR;

	TIMEVAL tv = {in_seconds, 0};
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
RedisReply* HiredisCpp::exec(const std::string &in_command_string, RedisCallback* in_callback, void* in_pdata)
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	if (m_redis_ctx.isConnected() == false) return 0;

	if (in_command_string.empty() == false)
	{
		redisReply* reply = 0;
		if (m_redis_ctx.m_is_async == false)
		{
			reply = static_cast<redisReply*>(redisCommand(m_redis_ctx.m_context.hiredis_ctx, in_command_string.data()));
			RedisReply* rep = RedisReply::createReply(reply);

			return rep;
		}
		else
		{
			RedisCommand* command = new RedisCommand(in_command_string);
			if (command == 0) return 0;
			
			command->m_p_callback = in_callback;
			command->m_priv_data.pdata = in_pdata;
			
			std::string buffer(in_command_string);
			std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);

			if(!buffer.compare(0, SUBSCRIBE_CMD.size(), SUBSCRIBE_CMD))
				command->m_delete_after_callback_exec = false; // don't delete subscribe callback after first execution ...
			else
				command->m_delete_after_callback_exec = true; // self destruction ...
			
			if (redisAsyncCommand(m_redis_ctx.m_context.hiredis_async_ctx, (redisCallbackFn*)backendCommandCallback, &command->m_priv_data, command->m_command_string.data()) == REDIS_ERR)
			{
				std::cout << "HiredisCpp::exec - redisAsyncCommand failed for " << in_command_string << std::endl;
			}
			return 0;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* HiredisCpp::exec(const std::vector<std::string> &in_command_vector)
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	// pipeline commands are only available in sync blocking mode ...
	if (m_redis_ctx.isConnected() == false) return 0;
	if (m_redis_ctx.isBlocking() == false) return 0;

	RedisReply* rep = 0;
	if (in_command_vector.size() > 0)
	{
		std::string command_string;
		int rv = REDIS_OK;

		// append commands to output buffer ...
		for (int i=0; i<in_command_vector.size(); ++i)
		{
			command_string = in_command_vector.at(i);
			if (command_string.empty()) continue;

			rv = redisAppendCommand(m_redis_ctx.m_context.hiredis_ctx, command_string.data());
			if (rv != REDIS_OK) 
			{
				std::cout << "HiredisCpp::exec - redisAppendCommand failed for " << command_string << std::endl;
				continue;
			}
		}

		rep = getReply();
	}
	return rep;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::writePendingCommands()
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	if (m_redis_ctx.isValid() == false) return 0;
	if (m_redis_ctx.isBlocking() == true) return 0;	// will be done by __redisBlockForReply
	if (m_redis_ctx.isAsync() == true) return 0;	// will be done by event loop

	int done = 0;
	
	// write whole buffer ...
    do {
        if (redisBufferWrite(m_redis_ctx.m_context.hiredis_ctx, &done) == REDIS_ERR)
            return REDIS_ERR;
    } while (!done);

	return REDIS_OK;
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
RedisReply* HiredisCpp::getReply()
{
	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return false;

	if (m_redis_ctx.isValid() == false) return 0;

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
std::vector<RedisReply*> HiredisCpp::getPendingReplies(const bool in_discard)
{
	int rv = REDIS_OK;
	redisReply* reply = 0;
	std::vector<RedisReply*> ret;

	MutexLocker locker(m_mutex_redis_ctx);
	if (locker.isLocked() == false) return ret;

	if (m_redis_ctx.isValid() == false) return ret;
	//if (m_redis_ctx.isAsync() == true) return ret;

	// get replies from input buffer if any available ...
	while (rv == REDIS_OK)
	{
		if (m_redis_ctx.isBlocking())
			rv = redisGetReply(m_redis_ctx.m_context.hiredis_ctx, (void**)&reply);
		else
			rv = readRedisBuffer(&reply);

		if (reply == 0) break;

		// drop all replies ...
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

// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void HiredisCpp::backendConnectCallback(const struct redisAsyncContext* in_ctx, int in_status)
{
	std::cout << "backendConnectCallback called ..." << std::endl;

	if (in_ctx == 0) return;
	if (in_status != REDIS_OK) return;
	
	HiredisCpp* client = (HiredisCpp*)in_ctx->data;
	if (client == 0) return;

	if (client->m_p_connect_callback != 0 && client->m_p_connect_callback->m_p_status_callback != 0)
		client->m_p_connect_callback->m_p_status_callback(in_status);
}

// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void HiredisCpp::backendDisconnectCallback(const struct redisAsyncContext* in_ctx, int in_status)
{
	std::cout << "backendDisconnectCallback called ..." << std::endl;

	if (in_ctx == 0) return;
	if (in_status != REDIS_OK) return;
	
	HiredisCpp* client = (HiredisCpp*)in_ctx->data;
	if (client == 0) return;

	if (client->m_p_disconnect_callback != 0 && client->m_p_disconnect_callback->m_p_status_callback != 0)
		client->m_p_disconnect_callback->m_p_status_callback(in_status);
}


// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void HiredisCpp::backendCommandCallback(struct redisAsyncContext* in_ctx, void* in_reply, void* in_pdata)
{
	std::cout << "backendCommandCallback called ..." << std::endl;

	if (in_reply == 0) return;
	if (in_pdata == 0) return;

	RedisCommand::CallbackPrivateData* priv_data = (RedisCommand::CallbackPrivateData*)in_pdata;
	if (priv_data == 0) return;

	RedisCommand* command = priv_data->command;
	if (command == 0) return;

	RedisReply* rep = RedisReply::createReply((redisReply*)in_reply);
	if (rep == 0) return;

	if (command->m_p_callback != 0 && command->m_p_callback->m_p_command_callback != 0)
		command->m_p_callback->m_p_command_callback(rep, priv_data->pdata);
	else
		delete rep;	// reply not deliverable - cleanup ...

	if (command->m_delete_after_callback_exec)
	{
		delete command;
		command = 0;
	}
}
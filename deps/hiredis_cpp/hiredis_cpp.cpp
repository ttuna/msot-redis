#include "hiredis_cpp.h"

#include "../hiredis/win32_hiredis.h"
#include "../hiredis/async.h"
#include "../hiredis/adapters/ae.h"

#include <iostream>
#include <assert.h>
#include <functional>

namespace HIREDIS_CPP {
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

private:
	MutexLocker(const MutexLocker& other);
	MutexLocker& operator=(const MutexLocker&);
	HANDLE m_mutex;
	DWORD m_locked;
};
// ----------------------------------------------------------------------------
MutexLocker::MutexLocker(HANDLE in_mutex, const DWORD in_timeout_ms) :
	m_mutex(in_mutex),
	m_locked(0)
{
	if (m_mutex != 0)
	{
		m_locked = WaitForSingleObject(m_mutex, in_timeout_ms);
	}
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
//
// AsyncConnectThreadData
//
// ----------------------------------------------------------------------------
class AsyncConnectThreadData 
{
public:
	AsyncConnectThreadData();
	virtual ~AsyncConnectThreadData();

	bool isValid() const;

	std::string getHost();
	void setHost(const std::string& in_host); 

	int getPort();
	void setPort(const int in_port);
	
	RedisContext* getContext();
	void setContext(RedisContext* in_ctx);

	RedisCallback* getConnectCallback();
	void setConnectCallback(RedisCallback* in_callback);

	RedisCallback* getDisconnectCallback();
	void setDisconnectCallback(RedisCallback* in_callback);

	redisAsyncContext* getNativeCtx();
	void setNativeCtx(redisAsyncContext* in_ctx);

	redisConnectCallback* getNativeConnectCallback();
	redisDisconnectCallback* getNativeDisconnectCallback();

private:
	HANDLE m_mutex_handle; 
	std::string m_host;
	int m_port;
	RedisContext* m_p_redis_ctx;
	RedisCallback* m_p_connect_callback;
	RedisCallback* m_p_disconnect_callback;
};

// ----------------------------------------------------------------------------
AsyncConnectThreadData::AsyncConnectThreadData() :
	m_mutex_handle(0)
{
	m_mutex_handle = CreateMutex(NULL,              // default security attributes
								 FALSE,             // initially not owned
								 NULL);             // unnamed mutex
}
// ----------------------------------------------------------------------------
AsyncConnectThreadData::~AsyncConnectThreadData()
{
	if (m_mutex_handle)
	{
		CloseHandle(m_mutex_handle);
		m_mutex_handle = 0;
	}
}
// ----------------------------------------------------------------------------
bool AsyncConnectThreadData::isValid() const
{
	if (m_mutex_handle == 0) return false;
	if (m_host.empty()) return false;
	if (m_port <= 0) return false;
	if (m_p_redis_ctx == 0) return false;
	if (m_p_connect_callback == 0) return false;
	if (m_p_disconnect_callback == 0) return false;

	return true;
}
// ----------------------------------------------------------------------------
std::string AsyncConnectThreadData::getHost()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return std::string();
	return m_host;
}
// ----------------------------------------------------------------------------
void AsyncConnectThreadData::setHost(const std::string& in_host)
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return;
	m_host = in_host;
}
// ----------------------------------------------------------------------------
int AsyncConnectThreadData::getPort()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return -1;
	return m_port;
}
// ----------------------------------------------------------------------------
void AsyncConnectThreadData::setPort(const int in_port)
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return;
	m_port = in_port;
}
// ----------------------------------------------------------------------------
RedisContext* AsyncConnectThreadData::getContext()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return 0;
	return m_p_redis_ctx;
}
// ----------------------------------------------------------------------------
void AsyncConnectThreadData::setContext(RedisContext* in_ctx)
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return;
	m_p_redis_ctx = in_ctx;
}
// ----------------------------------------------------------------------------
RedisCallback* AsyncConnectThreadData::getConnectCallback()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return 0;
	return m_p_connect_callback;
}
// ----------------------------------------------------------------------------
void AsyncConnectThreadData::setConnectCallback(RedisCallback* in_callback)
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return;
	m_p_connect_callback = in_callback;
}
// ----------------------------------------------------------------------------
RedisCallback* AsyncConnectThreadData::getDisconnectCallback()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return 0;
	return m_p_disconnect_callback;
}
// ----------------------------------------------------------------------------
void AsyncConnectThreadData::setDisconnectCallback(RedisCallback* in_callback)
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return;
	m_p_disconnect_callback = in_callback;
}
// ----------------------------------------------------------------------------
redisAsyncContext* AsyncConnectThreadData::getNativeCtx()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return 0;
	if (m_p_redis_ctx == 0) return 0;
	return m_p_redis_ctx->m_context.hiredis_async_ctx;
}
// ----------------------------------------------------------------------------
void AsyncConnectThreadData::setNativeCtx(redisAsyncContext* in_ctx)
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return;
	if (m_p_redis_ctx == 0) return;
	m_p_redis_ctx->m_context.hiredis_async_ctx = in_ctx;
}
// ----------------------------------------------------------------------------
redisConnectCallback* AsyncConnectThreadData::getNativeConnectCallback()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return 0;
	if (m_p_redis_ctx == 0) return 0;
	return (redisConnectCallback*)&m_p_connect_callback->m_p_backend_status_callback;
}
// ----------------------------------------------------------------------------
redisDisconnectCallback* AsyncConnectThreadData::getNativeDisconnectCallback()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return 0;
	if (m_p_redis_ctx == 0) return 0;
	return (redisDisconnectCallback*)&m_p_disconnect_callback->m_p_backend_status_callback;
}

} // namespace HIREDIS_CPP

using namespace HIREDIS_CPP;

namespace {

// ----------------------------------------------------------------------------
// thread event loop ...
// ----------------------------------------------------------------------------
aeEventLoop *g_p_event_loop;

// ----------------------------------------------------------------------------
// thread data ...
// ----------------------------------------------------------------------------
AsyncConnectThreadData* g_p_thread_data;

// ----------------------------------------------------------------------------
// thread function ...
// ----------------------------------------------------------------------------
DWORD WINAPI AsyncConnectThreadFunction(LPVOID in_param)
{
	if (in_param == 0) return -1;

	AsyncConnectThreadData* thread_data = (AsyncConnectThreadData*)(in_param);
	if (thread_data == 0 || thread_data->isValid() == false) return -1;

	g_p_event_loop = aeCreateEventLoop(1024 * 10);
	if (g_p_event_loop == 0) return -1;

	redisAsyncContext* ctx = redisAsyncConnect(thread_data->getHost().data(), thread_data->getPort());
	if (ctx == 0) return -1;

	redisAeAttach(g_p_event_loop, ctx);
	redisAsyncSetConnectCallback(ctx, thread_data->getNativeConnectCallback());
	redisAsyncSetDisconnectCallback(ctx, thread_data->getNativeDisconnectCallback());

	thread_data->setNativeCtx(ctx);

	aeMain(g_p_event_loop);
	return 0;
};

// ----------------------------------------------------------------------------
// FRONTEND: default connect-callback ...
// ----------------------------------------------------------------------------
void defaultConnectCallback(int status)
{
	if(status != REDIS_OK)
		std::cout << "Connection failed!" << std::endl;
	else
		std::cout << "Connected ..." << std::endl;
};


// ----------------------------------------------------------------------------
// FRONTEND: default disconnect-callback ...
// ----------------------------------------------------------------------------
void defaultDisconnectCallback(int status)
{
	if(status != REDIS_OK)
		std::cout << "Couldn't disconnect" << std::endl;
	else
		std::cout << "Disconnected ..." << std::endl;
};

} // namespace

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
HiredisCpp::HiredisCpp() :
	m_default_reader(false),
	m_command_cache(MAX_COMMAND_ENTRIES)
//	m_thread_handle(0),
//	m_thread_id(0)
{
	
}

HiredisCpp::~HiredisCpp()
{
	// avoid double freeing of m_p_hiredis_reader which was taken from m_p_hiredis_ctx ...
	m_default_reader.m_p_hiredis_reader = NULL;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool HiredisCpp::connect(const std::string &in_host, const int in_port, const bool in_blocking, const int in_timeout_sec)
{
	if (in_host.empty()) return false;
	if (in_port == 0) return false;

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

	g_p_thread_data = new AsyncConnectThreadData;
	if (g_p_thread_data == 0) return false;

	m_connect_callback.m_p_status_callback = (in_connect_callback == 0) ?  defaultConnectCallback : in_connect_callback->m_p_status_callback;
	m_disconnect_callback.m_p_status_callback = (in_disconnect_callback == 0) ? defaultDisconnectCallback : in_disconnect_callback->m_p_status_callback;

	g_p_thread_data->setHost(in_host);
	g_p_thread_data->setPort(in_port);
	g_p_thread_data->setContext(&m_redis_ctx);
	g_p_thread_data->setConnectCallback(&m_connect_callback);
	g_p_thread_data->setDisconnectCallback(&m_disconnect_callback);

	m_thread_handle = CreateThread( 
							NULL,						// default security attributes
							0,							// use default stack size  
							AsyncConnectThreadFunction, // thread function name
							g_p_thread_data,			// argument to thread function 
							0,							// use default creation flags 
							&m_thread_id);				// returns the thread identifier

	if (m_thread_handle == 0) return 0;
	return m_thread_handle;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::setTimeout(const int in_seconds)
{
	if (m_redis_ctx.isValid() == false) return REDIS_ERR;

	TIMEVAL tv = {in_seconds, 0};
	return redisSetTimeout(m_redis_ctx.m_context.hiredis_ctx, tv);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::enableKeepAlive()
{
	if (m_redis_ctx.isValid() == false) return REDIS_ERR;
	return redisEnableKeepAlive(m_redis_ctx.m_context.hiredis_ctx);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReader& HiredisCpp::getReader(const bool in_default)
{
	if (m_default_reader.m_p_hiredis_reader == 0)
	{
		if (in_default == true && m_redis_ctx.m_context.hiredis_ctx != 0)
		{
			m_default_reader.m_p_hiredis_reader = m_redis_ctx.m_context.hiredis_ctx->reader;
		}
		else
		{
			m_default_reader.m_p_hiredis_reader = redisReaderCreate();
		}
	}
	
	return m_default_reader;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
const RedisReply* HiredisCpp::exec(const std::string &in_command_string, RedisCallback* in_callback, void* in_pdata)
{
	if (in_command_string.empty() == false)
	{
		redisReply* reply = 0;
		if (m_redis_ctx.m_is_async == false)
		{
			reply = static_cast<redisReply*>(redisCommand(m_redis_ctx.m_context.hiredis_ctx, in_command_string.data()));
			std::vector<RedisReply*> replies = RedisReply::createReply(reply);

			if (replies.size() == 0) return 0;
			return replies[0];
		}
		else
		{
			if (redisAsyncCommand(m_redis_ctx.m_context.hiredis_async_ctx, (redisCallbackFn*)&in_callback->m_p_backend_command_callback, in_pdata, in_command_string.data()) == REDIS_ERR)
			{
				std::cout << "redisAsyncCommand failed for " << in_command_string << std::endl;
			}
			return 0;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
const std::vector<RedisReply*> HiredisCpp::exec(const std::vector<std::string> &in_command_vector)
{
	if (m_redis_ctx.m_is_async = true) return std::vector<RedisReply*>();

	if (in_command_vector.empty() == false)
	{


	}
}

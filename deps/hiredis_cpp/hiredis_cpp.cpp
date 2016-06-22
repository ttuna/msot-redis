#include "hiredis_cpp.h"

#include "../hiredis/win32_hiredis.h"
#include "../hiredis/async.h"
#include "../hiredis/adapters/ae.h"

#include <iostream>
#include <assert.h>
#include <functional>

namespace HIREDIS_CPP {
class AsyncConnectThreadData;
}

namespace {

// ----------------------------------------------------------------------------
// thread event loop ...
// ----------------------------------------------------------------------------
aeEventLoop *g_p_event_loop;

// ----------------------------------------------------------------------------
// thread data ...
// ----------------------------------------------------------------------------
HIREDIS_CPP::AsyncConnectThreadData* g_p_thread_data;

} // namespace

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
	bool unlock();

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
bool MutexLocker::unlock()
{
	BOOL ret = FALSE;
	if (m_mutex && m_locked == WAIT_OBJECT_0)
		ret = ReleaseMutex(m_mutex);

	return (ret == TRUE);
}

// forward declaration ...
class HiredisCpp;

// ----------------------------------------------------------------------------
//
// AsyncConnectThreadData
//
// ----------------------------------------------------------------------------
class AsyncConnectThreadData 
{
	friend class HiredisCpp;
public:
	AsyncConnectThreadData();
	virtual ~AsyncConnectThreadData();
	bool isValid() const;

// ----------------------------------------------------------------------------
	HiredisCpp* getClient() const
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return 0;
		return m_p_client;
	}
// ----------------------------------------------------------------------------
	void setClient(HiredisCpp* in_client)
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return;
		m_p_client = in_client;
	}
// ----------------------------------------------------------------------------
	void* getMutexHandle() const
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return 0;
		return m_mutex_handle;
	}
// ----------------------------------------------------------------------------
	void setMutexHandle(void* in_mutex_handle)
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return;
		m_mutex_handle = in_mutex_handle;
	}
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
// ----------------------------------------------------------------------------
	void setNativeCtx(redisAsyncContext* ctx)
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return;
		if (m_p_client == 0) return;
		ctx->data = m_p_client;
		m_p_client->m_redis_ctx.m_context.hiredis_async_ctx = ctx;
		m_p_client->m_redis_ctx.m_is_async = true;
	}
// ----------------------------------------------------------------------------
	redisConnectCallback* getConnectCallback()
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return 0;
		if (m_p_client == 0) return 0;
		return &m_p_client->backendConnectCallback;
	}
// ----------------------------------------------------------------------------
	redisDisconnectCallback* getDisconnectCallback()
	{
		MutexLocker locker(m_mutex_handle);
		if (locker.isLocked() == false) return 0;
		if (m_p_client == 0) return 0;
		return &m_p_client->backendDisconnectCallback;
	}

private:
	HiredisCpp* m_p_client;
	HANDLE m_mutex_handle; 
	std::string m_host;
	int m_port;
};

// ----------------------------------------------------------------------------
AsyncConnectThreadData::AsyncConnectThreadData() :
	m_p_client(0),
	m_mutex_handle(0),
	m_port(0)
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
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return false;

	if (m_p_client == 0) return false;
	if (m_mutex_handle == 0) return false;
	if (m_host.empty()) return false;
	if (m_port <= 0) return false;

	return true;
}

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

	thread_data->setNativeCtx(ctx);

	redisAeAttach(g_p_event_loop, ctx);
	redisAsyncSetConnectCallback(ctx, thread_data->getConnectCallback());
	redisAsyncSetDisconnectCallback(ctx, thread_data->getDisconnectCallback());

	aeMain(g_p_event_loop);
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
	m_default_reader(false),
	m_command_cache(MAX_COMMAND_ENTRIES),
	m_p_connect_callback(0),
	m_p_disconnect_callback(0),
	m_thread_handle(0),
	m_thread_id(0)
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

	m_p_connect_callback = in_connect_callback ;
	m_p_disconnect_callback = in_disconnect_callback;

	g_p_thread_data->m_p_client = this;
	g_p_thread_data->m_host = in_host;
	g_p_thread_data->m_port = in_port;

	m_thread_handle = CreateThread( 
							NULL,						// default security attributes
							0,							// use default stack size  
							AsyncConnectThreadFunction, // thread function name
							g_p_thread_data,			// argument to thread function 
							0,							// use default creation flags 
							&m_thread_id);				// returns the thread identifier

	return m_thread_handle;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool HiredisCpp::disconnect()
{

	m_redis_ctx.cleanup();
	return true;
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
std::vector<RedisReply*> HiredisCpp::exec(const std::string &in_command_string, RedisCallback* in_callback, void* in_pdata)
{
	if (in_command_string.empty() == false)
	{
		redisReply* reply = 0;
		if (m_redis_ctx.m_is_async == false)
		{
			reply = static_cast<redisReply*>(redisCommand(m_redis_ctx.m_context.hiredis_ctx, in_command_string.data()));
			std::vector<RedisReply*> replies = RedisReply::createReply(reply);

			if (replies.size() == 0) return std::vector<RedisReply*>();
			return replies;
		}
		else
		{
			// TODO ...
			//if (redisAsyncCommand(m_redis_ctx.m_context.hiredis_async_ctx, (redisCallbackFn*)&in_callback->m_p_backend_command_callback, in_pdata, in_command_string.data()) == REDIS_ERR)
			//{
			//	std::cout << "redisAsyncCommand failed for " << in_command_string << std::endl;
			//}
			return std::vector<RedisReply*>();
		}
	}

	return std::vector<RedisReply*>();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::vector<RedisReply*> HiredisCpp::exec(const std::vector<std::string> &in_command_vector)
{
	if (m_redis_ctx.m_is_async = true) return std::vector<RedisReply*>();

	if (in_command_vector.empty() == false)
	{


	}
	return std::vector<RedisReply*>();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::freeRedisReply(RedisReply* in_reply)
{
	if (in_reply == 0) return;
	in_reply->cleanup();
	delete in_reply;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::freeRedisReplies(std::vector<RedisReply*>& in_replies)
{
	if (in_replies.empty()) return;

	for_each(in_replies.begin(), in_replies.end(), freeRedisReply);
	in_replies.clear();
}

// ----------------------------------------------------------------------------
//
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
//
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
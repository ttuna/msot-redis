#pragma warning(push)
#pragma warning(disable: 4251) // class 'std::vector<_Ty>' needs to have dll-interface
#include "rediscontext.h"
#include "rediscallback.h"
#include "redishelper.h"
#include "hiredis_cpp.h"
#pragma warning(pop)

#include "../hiredis/async.h"
#include "../hiredis/adapters/ae.h"

namespace HIREDIS_CPP {

// ----------------------------------------------------------------------------
//
// AsyncConnectThread
//
// ----------------------------------------------------------------------------
class AsyncConnectThread
{
	friend class RedisContext;
public:
	AsyncConnectThread();
	virtual ~AsyncConnectThread();
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
	RedisContext* m_p_context;
	aeEventLoop* m_p_event_loop;
	std::string m_host;
	int m_port;
};
// ----------------------------------------------------------------------------
AsyncConnectThread::AsyncConnectThread() :
	m_p_context(0),
	m_p_event_loop(0),
	m_port(0)
{
	m_mutex_handle = CreateMutex(NULL,              // default security attributes
								 FALSE,             // initially not owned
								 NULL);             // unnamed mutex
}
// ----------------------------------------------------------------------------
AsyncConnectThread::~AsyncConnectThread()
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
bool AsyncConnectThread::isValid() const
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return false;

	if (m_mutex_handle == 0) return false;
	if (m_p_context == 0) return false;
	if (m_host.empty()) return false;
	if (m_port <= 0) return false;

	return true;
}
// ----------------------------------------------------------------------------
bool AsyncConnectThread::prepareThreadLoop()
{
	MutexLocker locker(m_mutex_handle);
	if (locker.isLocked() == false) return false;
	if (m_p_context == 0) return false;

	// !!! event loop must be created before redisAsyncConnect ... !!!
	m_p_event_loop = aeCreateEventLoop(1024 * 10);
	if (m_p_event_loop == 0) return false;

	// async connect to redis ...
	redisAsyncContext* ctx = redisAsyncConnect(m_host.c_str(), m_port);
	if (ctx == 0) return false;

	m_p_context->m_context.hiredis_async_ctx = ctx;
	m_p_context->m_is_async = true;

	redisAeAttach(m_p_event_loop, ctx);
	redisAsyncSetConnectCallback(m_p_context->m_context.hiredis_async_ctx, &RedisCallback::backendConnectCallback);
	redisAsyncSetDisconnectCallback(m_p_context->m_context.hiredis_async_ctx, &RedisCallback::backendDisconnectCallback);

	return true;
}
// ----------------------------------------------------------------------------
bool AsyncConnectThread::execThreadLoop()
{
	//MutexLocker locker(m_mutex_handle);
	//if (locker.isLocked() == false) return false;
	if (m_p_event_loop == 0) return false;

	aeMain(m_p_event_loop);	// blocking ...
	return true;
}
// ----------------------------------------------------------------------------
bool AsyncConnectThread::stopThreadLoop()
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

	AsyncConnectThread* thread_ctx = (AsyncConnectThread*)(in_param);
	if (thread_ctx == 0 || thread_ctx->isValid() == false) return -1;
	if (thread_ctx->prepareThreadLoop() == false) return -1;
	if (thread_ctx->execThreadLoop() == false) return -1;

	return 0;
};

} // namespace HIREDIS_CPP



using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// class ReidsContext
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisContext::RedisContext() :
	m_is_async(false),
	m_p_thread(0),
	m_thread_handle(0),
	m_thread_id(0),
	m_port(0)
{
	m_context.hiredis_ctx = 0;
	m_context.hiredis_async_ctx = 0;

	m_mutex_thread = CreateMutex(NULL,		// default security attributes
								FALSE,      // initially not owned
								NULL);      // unnamed mutex

}

RedisContext::~RedisContext()
{
	if (m_mutex_thread)
	{
		CloseHandle(m_mutex_thread);
		m_mutex_thread = 0;
	}

	cleanup();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisContext::isValid() const
{
	if (m_context.hiredis_ctx == 0) return false;
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisContext::cleanup()
{
	// clear async thread_ctx
	if (m_p_thread != 0)
	{
		m_p_thread->stopThreadLoop();
		WaitForSingleObject(m_thread_handle, 1000);

		delete m_p_thread;
		m_p_thread = 0;
	}
	if (m_thread_handle)
	{
		CloseHandle(m_thread_handle);
		m_thread_handle = 0;
	}
	m_thread_id = 0;

	if (m_is_async && m_context.hiredis_async_ctx != 0)
	{
		redisAsyncDisconnect(m_context.hiredis_async_ctx);
		m_context.hiredis_async_ctx = 0;
	}
	else if (!m_is_async && m_context.hiredis_ctx != 0)
	{
		redisFree(m_context.hiredis_ctx);
		m_context.hiredis_ctx = 0;
	}
	m_is_async = false;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisContext::isAsync() 
{ 
	return m_is_async; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisContext::isBlocking() 
{ 
	return (m_is_async) ? false : ((m_context.hiredis_ctx->flags & REDIS_CONTEXT_FLAG_BLOCK) != 0); 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisContext::isConnected() 
{
	return (bool)((m_context.hiredis_ctx->flags & REDIS_CONTEXT_FLAG_CONNECTED) != 0); 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisContext::connect(const std::string &in_host, const int in_port, const bool in_blocking, const int in_timeout_sec)
{
	cleanup();

	redisContext* ctx = 0;
	if (in_blocking)
	{
		TIMEVAL tv = {(in_timeout_sec == -1) ? CONNECT_TIMEOUT_SEC : in_timeout_sec, 0};
		ctx = redisConnectWithTimeout(in_host.c_str(), in_port, tv);
	}
	else
	{
		ctx = redisConnectNonBlock(in_host.c_str(), in_port);
	}
	if (ctx == 0) return false;

	m_context.hiredis_ctx = ctx;
	m_is_async = false;

	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void* RedisContext::connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback, RedisCallback* in_disconnect_callback)
{
	if (in_host.empty()) return false;
	if (in_port == 0) return false;

	MutexLocker locker(m_mutex_thread);
	if (locker.isLocked() == false) return 0;
	
	m_p_thread = new AsyncConnectThread;
	if (m_p_thread == 0) return false;

	m_p_thread->m_p_context = this;
	m_p_thread->m_host = in_host;
	m_p_thread->m_port = in_port;

	m_connect_callback = in_connect_callback;
	m_disconnect_callback = in_disconnect_callback;

	if (locker.unlock() == false) return 0;

	m_thread_handle = CreateThread( 
							NULL,						// default security attributes
							0,							// use default stack size  
							AsyncConnectThreadFunction, // thread function
							m_p_thread,					// argument to thread function 
							0,							// use default creation flags 
							&m_thread_id);				// returns the thread identifier

	return m_thread_handle;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisContext::disconnect()
{
	cleanup();
}

#ifndef _REDISCONTEXT_H_
#define _REDISCONTEXT_H_

#include "global.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

struct redisContext;
struct redisAsyncContext;
struct aeEventLoop;

namespace HIREDIS_CPP
{

class RedisCallback;

// ----------------------------------------------------------------------------
//
// class RedisContext
//
// ----------------------------------------------------------------------------
class DllExport RedisContext
{
	friend class HiredisCpp;
	friend class AsyncConnectThread;
public:
	virtual ~RedisContext();
	bool isValid() const;
	void cleanup();
	
	bool isAsync();
	bool isBlocking();
	bool isConnected();

	RedisCallback* getConnectionCallback() { return m_connect_callback; };
	RedisCallback* getDisconnectionCallback() { return m_disconnect_callback; }

private:
	RedisContext();
	RedisContext(const RedisContext& other);
	RedisContext& operator=(const RedisContext&);

	bool connect(const std::string &in_host, const int in_port, const bool in_blocking, const int in_timeout_sec);
	void* connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback, RedisCallback* in_disconnect_callback);
	void disconnect();

	// first member of redisAsyncContext is a pointer to redisContext (see async.h)
	// therefore it should be save to call m_p_context->hiredis_ctx independently of m_is_async ...
	union {
		redisContext* hiredis_ctx;				// context for sync connection (blocking, non-blocking) ...
		redisAsyncContext* hiredis_async_ctx;	// context for async connection (non-blocking) ...
	} m_context;

	bool m_is_async;

	AsyncConnectThread* m_p_thread;
	void* m_mutex_thread;
	void* m_thread_handle;
	unsigned long m_thread_id;

	RedisCallback* m_connect_callback;
	RedisCallback* m_disconnect_callback;
	std::string m_host;
	int m_port;
};

}

#endif
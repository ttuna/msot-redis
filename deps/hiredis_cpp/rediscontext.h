#ifndef _REDISCONTEXT_H_
#define _REDISCONTEXT_H_

#include "global.h"

struct redisContext;
struct redisAsyncContext;
struct aeEventLoop;

namespace HIREDIS_CPP
{

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

private:
	RedisContext();
	RedisContext(const RedisContext& other);
	RedisContext& operator=(const RedisContext&);

	bool connect(const std::string &in_host, const int in_port, const bool in_blocking, const int in_timeout_sec);
	void* connectAsync(const std::string &in_host, const int in_port);
	void disconnect();

	// first member of redisAsyncContext is a pointer to redisContext (see async.h)
	// therefore it should be save to call m_p_context->hiredis_ctx independent of m_is_async ...
	union {
		redisContext* hiredis_ctx;				// context for sync connection (blocking, non-blocking) ...
		redisAsyncContext* hiredis_async_ctx;	// context for async connection (non-blocking) ...
	} m_context;

	AsyncConnectThread* m_p_thread;
	void* m_mutex_thread;

	void* m_thread_handle;
	unsigned long m_thread_id;

	bool m_is_async;
};

}

#endif
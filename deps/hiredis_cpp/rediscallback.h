#ifndef _REDISCALLBACK_H_
#define _REDISCALLBACK_H_

#include <functional>
#include "global.h"

struct redisAsyncContext;
struct redisReply;

namespace HIREDIS_CPP
{

class RedisReply;

typedef void (RedisStatusCallback)(int status);
typedef void (RedisCommandCallback)(RedisReply* reply, void* privdata);

class DllExport RedisCallback
{
	friend class HiredisCpp;
	friend class AsyncConnectThreadData;
public:
	typedef void (RedisCallback::*RedisBackendStatusCallback)(const struct redisAsyncContext*, int status);
	typedef void (RedisCallback::*RedisBackendCommandCallback)(struct redisAsyncContext*, void*, void*);

	virtual ~RedisCallback();
	bool isValid() const;
	void cleanup();

	RedisCallback();
	RedisCallback(const RedisCallback& other);
	RedisCallback& operator=(const RedisCallback&);

private:
	RedisStatusCallback* m_p_status_callback;
	RedisCommandCallback* m_p_command_callback;
	RedisBackendStatusCallback m_p_backend_status_callback;
	RedisBackendCommandCallback m_p_backend_command_callback;

	void backendStatusCallback(const struct redisAsyncContext* in_ctx, int in_status);
	void backendCommandCallback(struct redisAsyncContext* in_ctx, void* in_reply, void* in_pdata);

	std::tr1::function<void(const struct redisAsyncContext*, int status)> m_status_func;
	std::tr1::function<void(struct redisAsyncContext*, void*, void*)> m_command_func;

	bool m_delete_after_exec;
};

} // namespace

#endif //_REDISCALLBACK_H_
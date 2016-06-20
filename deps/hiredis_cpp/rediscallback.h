#ifndef _REDISCALLBACK_H_
#define _REDISCALLBACK_H_

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
public:
	virtual ~RedisCallback();
	bool isValid() const;
	void cleanup();

private:
	RedisCallback();
	RedisCallback(const RedisCallback& other);
	RedisCallback& operator=(const RedisCallback&);

	void backendStatusCallback(struct redisAsyncContext* in_ctx, int in_status);
	void backendCommandCallback(struct redisAsyncContext* in_ctx, void* in_reply, void* in_pdata);
	RedisStatusCallback* m_p_status_callback;
	RedisCommandCallback* m_p_command_callback;

	bool m_delete_after_exec;
};

} // namespace

#endif //_REDISCALLBACK_H_
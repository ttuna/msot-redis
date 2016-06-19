#ifndef _REDISCONTEXT_H_
#define _REDISCONTEXT_H_

#include "global.h"

struct redisContext;
struct redisAsyncContext;

namespace HIREDIS_CPP
{

class DllExport RedisContext
{
	friend class HiredisCpp;
public:
	virtual ~RedisContext();
	bool isValid() const;
	void cleanup();

private:
	RedisContext();
	RedisContext(const RedisContext& other);
	RedisContext& operator=(const RedisContext&);
	
	bool m_is_async;

	// first member of redisAsyncContext is a pointer to redisContext (see async.h)
	// therefore it should be save to call m_p_context->hiredis_ctx independent of m_is_async ...
	union {
		redisContext* hiredis_ctx;				// context for sync connection (blocking, non-blocking) ...
		redisAsyncContext* hiredis_async_ctx;	// context for async connection (non-blocking) ...
	} m_context;
};

}

#endif
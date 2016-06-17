#ifndef _REDISCONTEXT_H_
#define _REDISCONTEXT_H_

#include "global.h"

struct redisContext;

namespace HIREDIS_CPP
{

class DllExport RedisContext
{
	friend class HiredisCpp;
public:
	virtual ~RedisContext() {}
	bool isValid();
	void cleanup();

private:
	RedisContext();
	RedisContext(const RedisContext& other);
	RedisContext& operator=(const RedisContext&);

	redisContext* m_p_hiredis_ctx;
};

}

#endif
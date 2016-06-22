#include "rediscontext.h"

#include "../hiredis/win32_hiredis.h"
#include "../hiredis/async.h"

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisContext::RedisContext() :
	m_is_async(false)
{
	m_context.hiredis_ctx = 0;
	m_context.hiredis_async_ctx = 0;
}

RedisContext::~RedisContext()
{
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
	if (m_context.hiredis_ctx == 0) return;

	if (m_is_async)
		// redisAsyncDisconnect calls redisAsyncFree !!!
		redisAsyncDisconnect(m_context.hiredis_async_ctx);
	else
		redisFree(m_context.hiredis_ctx);
}

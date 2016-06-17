#include "rediscontext.h"

#ifdef _WIN32
#include "../hiredis/win32_hiredis.h"
#else
#include "../hiredis/hiredis.h"
#endif

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisContext::RedisContext() :
	m_p_hiredis_ctx(0)
{
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisContext::isValid()
{
	if (m_p_hiredis_ctx == 0) return false;

	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisContext::cleanup()
{
	if (m_p_hiredis_ctx == 0) return;

	redisFree(m_p_hiredis_ctx);
	m_p_hiredis_ctx = 0;
}
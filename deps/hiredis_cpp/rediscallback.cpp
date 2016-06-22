#include "rediscallback.h"
#include "redisreply.h"

#include "../hiredis/win32_hiredis.h"
#include "../hiredis/async.h"

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCallback::RedisCallback() :
	m_p_status_callback(0),
	m_p_command_callback(0),
	m_delete_after_exec(false)
{
}

RedisCallback::RedisCallback(RedisStatusCallback* in_status_callback) :
	m_p_status_callback(in_status_callback),
	m_p_command_callback(0),
	m_delete_after_exec(false)
{
}

RedisCallback::RedisCallback(RedisCommandCallback* in_command_callback) :
	m_p_status_callback(0),
	m_p_command_callback(in_command_callback),
	m_delete_after_exec(false)
{
}

RedisCallback::~RedisCallback()
{
	cleanup();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisCallback::isValid() const
{
	if (m_p_status_callback == 0 && m_p_command_callback == 0) return false;
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCallback::cleanup()
{
	m_p_status_callback = 0;
	m_p_command_callback = 0;
}

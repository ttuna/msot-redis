#include "rediscallback.h"
#include "redisreply.h"

#ifdef _WIN32
#include "../hiredis/win32_hiredis.h"
#else
#include "../hiredis/hiredis.h"
#endif
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

RedisCallback::~RedisCallback()
{
	cleanup();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisCallback::isValid() const
{
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCallback::cleanup()
{

}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCallback::backendStatusCallback(struct redisAsyncContext* in_ctx, int in_status)
{
	if (m_p_status_callback == 0) return;
	m_p_status_callback(in_status);

	if (m_delete_after_exec == true)
		delete this;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCallback::backendCommandCallback(struct redisAsyncContext* in_ctx, void* in_reply, void* in_pdata)
{
	if (m_p_command_callback == 0) return;

	RedisReply reply;
	reply.m_p_hiredis_reply = static_cast<redisReply*>(in_reply);
	m_p_command_callback(&reply, in_pdata);

	if (m_delete_after_exec == true)
		delete this;
}
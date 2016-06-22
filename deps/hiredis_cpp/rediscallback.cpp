#include "rediscallback.h"
#include "redisreply.h"

#include "../hiredis/win32_hiredis.h"
#include "../hiredis/async.h"

using namespace HIREDIS_CPP;
using namespace std::tr1::placeholders; 

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCallback::RedisCallback() :
	m_p_status_callback(0),
	m_p_command_callback(0),
	m_delete_after_exec(false)
{
	m_p_backend_status_callback = &RedisCallback::backendStatusCallback;
	m_p_backend_command_callback = &RedisCallback::backendCommandCallback;
	
	
	m_status_func = std::tr1::bind(&RedisCallback::backendStatusCallback, this, _1, _2);
	m_command_func = std::tr1::bind(&RedisCallback::backendCommandCallback, this, _1, _2, _3);

	// TODO: find a way to pass std::tr1::function to ANSI C function pointer ... :-(
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
void RedisCallback::backendStatusCallback(const struct redisAsyncContext* in_ctx, int in_status)
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

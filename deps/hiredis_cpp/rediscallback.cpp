#include "rediscallback.h"
#include "redisreply.h"
#include "redishelper.h"

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
	m_priv_data.callback = this;
}

RedisCallback::RedisCallback(RedisStatusCallback* in_status_callback) :
	m_p_status_callback(in_status_callback),
	m_p_command_callback(0),
	m_delete_after_exec(false)
{
	m_priv_data.callback = this;
}

RedisCallback::RedisCallback(RedisCommandCallback* in_command_callback) :
	m_p_status_callback(0),
	m_p_command_callback(in_command_callback),
	m_delete_after_exec(false)
{
	m_priv_data.callback = this;
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


// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void RedisCallback::backendConnectCallback(const struct redisAsyncContext* in_ctx, int in_status)
{
	std::cout << "backendConnectCallback called ..." << std::endl;

	if (in_ctx == 0) return;
	if (in_status != REDIS_OK) return;
	
	RedisPrivateData& rpd = RedisPrivateData::getInstance();
	RedisCallback* callback = rpd.getConnectCallback();

	if (callback != 0 && callback->m_p_status_callback != 0)
		callback->m_p_status_callback(in_status);
}

// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void RedisCallback::backendDisconnectCallback(const struct redisAsyncContext* in_ctx, int in_status)
{
	std::cout << "backendDisconnectCallback called ..." << std::endl;

	if (in_ctx == 0) return;
	if (in_status != REDIS_OK) return;
	
	RedisPrivateData& rpd = RedisPrivateData::getInstance();
	RedisCallback* callback = rpd.getDisconnectCallback();

	if (callback != 0 && callback->m_p_status_callback != 0)
		callback->m_p_status_callback(in_status);
}


// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void RedisCallback::backendCommandCallback(const struct redisAsyncContext* in_ctx, void* in_reply, void* in_pdata)
{
	std::cout << "backendCommandCallback called ..." << std::endl;

	if (in_reply == 0) return;
	if (in_pdata == 0) return;

	RedisCallback::CallbackPrivateData* priv_data = (RedisCallback::CallbackPrivateData*)in_pdata;
	if (priv_data == 0) return;

	RedisCallback* callback = priv_data->callback;
	if (callback == 0) return;

	RedisReply* rep = RedisReply::createReply((redisReply*)in_reply);
	if (rep == 0) return;

	if (callback!= 0 && callback->m_p_command_callback != 0)
		callback->m_p_command_callback(rep, priv_data->pdata);
	else
		delete rep;	// reply not deliverable - cleanup ...

	if (callback->m_delete_after_exec)
	{
		delete callback;
		callback = 0;
	}
}

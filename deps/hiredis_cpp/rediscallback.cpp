#pragma warning(push)
#pragma warning(disable: 4251) // class 'std::vector<_Ty>' needs to have dll-interface
#include "rediscallback.h"
#include "redisreply.h"
#include "rediscontext.h"
#include "redishelper.h"
#pragma warning(pop)

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
	m_priv_data.pdata = 0;
	m_priv_data.callback = this;
}

RedisCallback::RedisCallback(RedisStatusCallback* in_status_callback, void* in_priv_data, bool in_delete_after_exec) :
	m_p_status_callback(in_status_callback),
	m_p_command_callback(0),
	m_delete_after_exec(in_delete_after_exec)
{
	m_priv_data.pdata = in_priv_data;
	m_priv_data.callback = this;
}

RedisCallback::RedisCallback(RedisCommandCallback* in_command_callback, void* in_priv_data, bool in_delete_after_exec) :
	m_p_status_callback(0),
	m_p_command_callback(in_command_callback),
	m_delete_after_exec(in_delete_after_exec)
{
	m_priv_data.pdata = in_priv_data;
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
//
// ----------------------------------------------------------------------------
RedisCallbackType RedisCallback::callbackType() const
{
	if (m_p_command_callback != 0)
		return REDIS_CALLBACK_TYPE_COMMAND;
	else if (m_p_status_callback != 0)
		return REDIS_CALLBACK_TYPE_STATUS;
	else 
		return REDIS_CALLBACK_TYPE_UNKNOWN;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCallback::setPrivateData(void* in_priv_data)
{
	m_priv_data.pdata = in_priv_data;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void* RedisCallback::getPrivateData()
{
	return m_priv_data.pdata;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCallback::setDeleteAfterExec(const bool in_delete)
{
	m_delete_after_exec = in_delete;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisCallback::getDeleteAfterExec() const
{
	return m_delete_after_exec;
}

// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void RedisCallback::backendConnectCallback(const struct redisAsyncContext* in_ctx, int in_status)
{
	//std::cout << "backendConnectCallback called ..." << std::endl;

	if (in_ctx == 0) return;
	if (in_status != REDIS_OK) return;
	
	RedisCallback* callback = 0;
	// call global connect callback ...
	RedisGlobalData& rgd = RedisGlobalData::getInstance();
	callback = rgd.getConnectCallback();

	if (callback!= 0 && callback->m_p_status_callback != 0)
		callback->m_p_status_callback(in_status);

	// call specific connect callback ...
	RedisContext* ctx = (RedisContext*)in_ctx->data;
	if (ctx == 0) return;

	callback = ctx->getConnectionCallback();
	if (callback != 0 && callback->m_p_status_callback != 0)
		callback->m_p_status_callback(in_status);
}

// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void RedisCallback::backendDisconnectCallback(const struct redisAsyncContext* in_ctx, int in_status)
{
	//std::cout << "backendDisconnectCallback called ..." << std::endl;

	if (in_ctx == 0) return;
	if (in_status != REDIS_OK) return;
	
	RedisCallback* callback = 0;
	// call global disconnect callback ...
	RedisGlobalData& rgd = RedisGlobalData::getInstance();
	callback = rgd.getDisconnectCallback();

	if (callback!= 0 && callback->m_p_status_callback != 0)
		callback->m_p_status_callback(in_status);

	// call specific disconnect callback ...
	RedisContext* ctx = (RedisContext*)in_ctx->data;
	if (ctx == 0) return;

	callback = ctx->getDisconnectionCallback();
	if (callback != 0 && callback->m_p_status_callback != 0)
		callback->m_p_status_callback(in_status);
}

// ----------------------------------------------------------------------------
// static
// ----------------------------------------------------------------------------
void RedisCallback::backendCommandCallback(const struct redisAsyncContext* in_ctx, void* in_reply, void* in_pdata)
{
	//std::cout << "backendCommandCallback called ..." << std::endl;

	if (in_reply == 0) return;
	if (in_pdata == 0) return;

	RedisCallback::CallbackPrivateData* priv_data = (RedisCallback::CallbackPrivateData*)in_pdata;
	if (priv_data == 0) return;

	RedisReply* rep = RedisReply::createReply((redisReply*)in_reply);
	if (rep == 0) return;

	RedisCallback* callback = 0;
	// call global command callback ...
	RedisGlobalData& rgd = RedisGlobalData::getInstance();
	callback = rgd.getCommandCallback();

	if (callback!= 0 && callback->m_p_command_callback != 0)
		callback->m_p_command_callback(rep, priv_data->pdata);

	// call specific command callback ...
	callback = priv_data->callback;
	if (callback == 0) return;

	if (callback!= 0 && callback->m_p_command_callback != 0)
		callback->m_p_command_callback(rep, priv_data->pdata);
	else
	{
		// reply not deliverable - cleanup ...
		rep->m_p_hiredis_reply = 0;	// avoid in_reply deletion ...
		delete rep;	
	}

	if (callback->m_delete_after_exec)
	{
		delete callback;
		callback = 0;
	}
}

#include "hiredis_cpp.h"
#include "rediscontext.h"
#include "redisreader.h"
#include "rediscommand.h"
#include "redisreply.h"
#include <assert.h>

#ifdef _WIN32
#include "../hiredis/win32_hiredis.h"
#else
#include "../hiredis/hiredis.h"
#endif
#include "../hiredis/async.h"

using namespace HIREDIS_CPP;

const int CONNECT_TIMEOUT_SEC = 5;
const int MAX_HISTORY_ENTRIES = 10;
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
HiredisCpp::HiredisCpp() :
	m_redis_ctx(),
	m_default_reader(false),
	m_last_command("")
{
}

HiredisCpp::~HiredisCpp()
{
	// avoid double freeing of m_p_hiredis_reader which was taken from m_p_hiredis_ctx ...
	m_default_reader.m_p_hiredis_reader = NULL;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool HiredisCpp::connect(const std::string &in_host, const int in_port, const bool in_blocking, const int in_timeout_sec)
{
	if (in_host.empty()) return false;
	if (in_port == 0) return false;

	redisContext* ctx = 0;
	if (in_blocking)
	{
		TIMEVAL tv = {(in_timeout_sec == -1) ? CONNECT_TIMEOUT_SEC : in_timeout_sec, 0};
		ctx = redisConnectWithTimeout(in_host.data(), in_port, tv);
	}
	else
	{
		ctx = redisConnectNonBlock(in_host.data(), in_port);
	}
	if (ctx == 0) return false;

	m_redis_ctx.cleanup();
	m_redis_ctx.m_context.hiredis_ctx = ctx;
	m_redis_ctx.m_is_async = false;
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool HiredisCpp::connectAsync(const std::string &in_host, const int in_port)
{
	if (in_host.empty()) return false;
	if (in_port == 0) return false;

	redisAsyncContext* ctx = redisAsyncConnect(in_host.data(), in_port);
	if (ctx == 0) return false;

	m_redis_ctx.cleanup();
	m_redis_ctx.m_context.hiredis_async_ctx = ctx;
	m_redis_ctx.m_is_async = true;
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::setTimeout(const int in_seconds)
{
	if (m_redis_ctx.isValid() == false) return REDIS_ERR;

	TIMEVAL tv = {in_seconds, 0};
	return redisSetTimeout(m_redis_ctx.m_context.hiredis_ctx, tv);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::enableKeepAlive()
{
	if (m_redis_ctx.isValid() == false) return REDIS_ERR;
	return redisEnableKeepAlive(m_redis_ctx.m_context.hiredis_ctx);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReader& HiredisCpp::getReader(const bool in_default)
{
	if (m_default_reader.m_p_hiredis_reader == 0)
	{
		if (in_default == true && m_redis_ctx.m_context.hiredis_ctx != 0)
		{
			m_default_reader.m_p_hiredis_reader = m_redis_ctx.m_context.hiredis_ctx->reader;
		}
		else
		{
			m_default_reader.m_p_hiredis_reader = redisReaderCreate();
		}
	}
	
	return m_default_reader;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
const RedisReply& HiredisCpp::exec(const std::string &in_command_string)
{
	resetLastCommand();

	if (in_command_string.empty() == false)
	{
		redisReply* reply = static_cast<redisReply*>(redisCommand(m_redis_ctx.m_context.hiredis_ctx, in_command_string.data()));
		m_last_command.m_command_string = in_command_string;
		m_last_command.m_reply.m_p_hiredis_reply = reply;
	}

	return m_last_command.m_reply;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::resetLastCommand()
{
	m_last_command.m_command_string = "";
	m_last_command.m_reply.cleanup();
}

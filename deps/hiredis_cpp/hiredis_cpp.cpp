#include "hiredis_cpp.h"
#include "rediscontext.h"
#include "redisreader.h"
#include <assert.h>

#ifdef _WIN32
#include "../hiredis/win32_hiredis.h"
#else
#include "../hiredis/hiredis.h"
#endif

using namespace HIREDIS_CPP;

const int CONNECT_TIMEOUT_SEC = 5;
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
HiredisCpp::HiredisCpp() :
	m_p_redis_ctx(0),
	m_p_default_reader(0)
{
}

HiredisCpp::~HiredisCpp()
{
	freeCtx();

	if (m_p_default_reader != 0)
	{
		delete m_p_default_reader;
		m_p_default_reader = 0;
	}
}


// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void HiredisCpp::freeCtx()
{
	if (m_p_redis_ctx == 0) return;
	
	m_p_redis_ctx->cleanup();
	delete m_p_redis_ctx;
	m_p_default_reader = 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool HiredisCpp::connect(const std::string &in_host, const int in_port)
{
	if (in_host.empty()) return false;
	if (in_port == 0) return false;

	redisContext* ctx = 0;
	TIMEVAL tv = {CONNECT_TIMEOUT_SEC, 0};
	ctx = redisConnectWithTimeout(in_host.data(), in_port, tv);
	if (ctx == 0) return false;

	m_p_redis_ctx = new RedisContext;
	if (m_p_redis_ctx == 0) return false;

	m_p_redis_ctx->m_p_hiredis_ctx = ctx;
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::setTimeout(const int in_seconds)
{
	if (m_p_redis_ctx == 0) return REDIS_ERR;
	if (m_p_redis_ctx->isValid() == false) return REDIS_ERR;

	TIMEVAL tv = {in_seconds, 0};
	return redisSetTimeout(m_p_redis_ctx->m_p_hiredis_ctx, tv);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int HiredisCpp::enableKeepAlive()
{
	if (m_p_redis_ctx == 0) return REDIS_ERR;
	if (m_p_redis_ctx->isValid() == false) return REDIS_ERR;

	return redisEnableKeepAlive(m_p_redis_ctx->m_p_hiredis_ctx);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReader& HiredisCpp::getReader(const std::string &in_name)
{
	if (in_name.empty()) return getDefaultReader();

	RedisReader* reader = 0;
	std::map<std::string, RedisReader*>::iterator iter = m_reader_map.end();
	if ((iter = m_reader_map.find(in_name)) != m_reader_map.end())
	{
		reader = iter->second;
	}
	else
	{
		reader = createReader();
		m_reader_map[in_name] = reader;
	}
	
	assert(reader != 0);
	return *reader;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReader& HiredisCpp::getDefaultReader()
{
	if (m_p_default_reader == 0)
		m_p_default_reader = createReader();

	assert(m_p_default_reader != 0);
	return *m_p_default_reader;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReader* HiredisCpp::createReader()
{
	RedisReader* reader = new RedisReader;
	return reader;
}

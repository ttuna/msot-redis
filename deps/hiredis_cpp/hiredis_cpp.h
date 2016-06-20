#ifndef _HIREDIS_CPP_H_
#define _HIREDIS_CPP_H_

#include <string>
#include <queue>
#include <map>
#include "rediscontext.h"
#include "redisreader.h"
#include "rediscommand.h"
#include "rediscommandcache.h"
#include "rediscallback.h"
#include "redisreply.h"
#include "global.h"

namespace HIREDIS_CPP
{

const int CONNECT_TIMEOUT_SEC = 5;
const int MAX_COMMAND_ENTRIES = 100;

class DllExport HiredisCpp
{
public:
	HiredisCpp();
	virtual ~HiredisCpp();
	
	bool connect(const std::string &in_host, const int in_port, const bool in_blocking = true, const int in_timeout_sec = -1);
	bool connectAsync(const std::string &in_host, const int in_port, RedisStatusCallback* in_connect_callback = 0, RedisStatusCallback* in_disconnect_callback = 0);
	int setTimeout(const int in_seconds);
	int enableKeepAlive();

	RedisReader& getReader(const bool in_default = false);
	const RedisReply& exec(const std::string &in_command_string, RedisCommandCallback* in_callback = 0);
	const std::vector<RedisReply*> exec(const std::vector<std::string> &in_command_vector);
	
private:
	HiredisCpp(const HiredisCpp& other);
	HiredisCpp& operator=(const HiredisCpp&);

	void resetRedisCtx();
	int prepareCommands(const unsigned int in_count);

	RedisContext m_redis_ctx;
	RedisReader m_default_reader;
	RedisCommandCache m_command_cache;
	RedisCallback m_connect_callback;
	RedisCallback m_disconnect_callback;
};

} // namespace

#endif //_HIREDIS_CPP_H_
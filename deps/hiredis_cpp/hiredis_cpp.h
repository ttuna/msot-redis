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

struct aeEventLoop;

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
	void* connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback = 0, RedisCallback* in_disconnect_callback = 0);
	int setTimeout(const int in_seconds);
	int enableKeepAlive();

	RedisReader& getReader(const bool in_default = false);
	const RedisReply* exec(const std::string &in_command_string, RedisCallback* in_callback = 0, void *in_pdata = 0);
	const std::vector<RedisReply*> exec(const std::vector<std::string> &in_command_vector);
	
private:
	HiredisCpp(const HiredisCpp& other);
	HiredisCpp& operator=(const HiredisCpp&);

	void resetRedisCtx();
	RedisContext m_redis_ctx;
	RedisReader m_default_reader;
	RedisCallback m_connect_callback;
	RedisCallback m_disconnect_callback;
	RedisCommandCache m_command_cache;		// unused for now ...

	void* m_thread_handle;
	unsigned long m_thread_id;
};

} // namespace

#endif //_HIREDIS_CPP_H_
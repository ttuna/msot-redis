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
	friend class AsyncConnectThreadData;
public:
	HiredisCpp();
	virtual ~HiredisCpp();
	
	bool connect(const std::string &in_host, const int in_port, const bool in_blocking = true, const int in_timeout_sec = -1);
	void* connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback = 0, RedisCallback* in_disconnect_callback = 0);
	bool disconnect();

	int setTimeout(const int in_seconds);
	int enableKeepAlive();

	RedisReader& getReader(const bool in_default = false);
	std::vector<RedisReply*> exec(const std::string &in_command_string, RedisCallback* in_callback = 0, void *in_pdata = 0);
	std::vector<RedisReply*> exec(const std::vector<std::string> &in_command_vector);
	static void freeRedisReply(RedisReply* in_reply);
	static void freeRedisReplies(std::vector<RedisReply*>& in_replies);
	
private:
	HiredisCpp(const HiredisCpp& other);
	HiredisCpp& operator=(const HiredisCpp&);

	static void backendConnectCallback(const struct redisAsyncContext*, int status);
	static void backendDisconnectCallback(const struct redisAsyncContext*, int status);

	RedisContext m_redis_ctx;
	RedisReader m_default_reader;
	RedisCallback* m_p_connect_callback;
	RedisCallback* m_p_disconnect_callback;
	RedisCommandCache m_command_cache;		// unused for now ...

	void* m_thread_handle;
	unsigned long m_thread_id;
};

} // namespace

#endif //_HIREDIS_CPP_H_
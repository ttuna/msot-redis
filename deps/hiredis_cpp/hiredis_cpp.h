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

class DllExport HiredisCpp
{
	friend class AsyncConnectThreadContext;
public:
	HiredisCpp();
	virtual ~HiredisCpp();
	
	bool connect(const std::string &in_host, const int in_port, const bool in_blocking = true, const int in_timeout_sec = -1);
	void* connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback = 0, RedisCallback* in_disconnect_callback = 0);
	bool disconnect();

	int setTimeout(const int in_seconds);
	int enableKeepAlive();

	RedisReply* exec(const std::string &in_command_string, RedisCallback* in_callback = 0, void *in_pdata = 0);
	RedisReply* exec(const std::vector<std::string> &in_command_vector);
	int writePendingCommands();
	RedisReply* getReply();
	void discardReply();
	static void freeRedisReply(RedisReply* in_reply);
	
private:
	HiredisCpp(const HiredisCpp& other);
	HiredisCpp& operator=(const HiredisCpp&);

	std::vector<RedisReply*> getPendingReplies(const bool in_discard = false);

	// low level API calls ...
	int readRedisBuffer(redisReply** out_reply);

	// callbacks frontend ...
	RedisCallback* m_p_connect_callback;
	RedisCallback* m_p_disconnect_callback;
	// callbacks backend ...
	static void backendConnectCallback(const struct redisAsyncContext* in_ctx, int status);
	static void backendDisconnectCallback(const struct redisAsyncContext* in_ctx, int status);
	static void backendCommandCallback(struct redisAsyncContext* in_ctx, void* in_reply, void* in_pdata);
	
	RedisContext m_redis_ctx;
	void * m_mutex_redis_ctx;

	AsyncConnectThreadContext* m_p_thread_ctx;
	void* m_mutex_thread_ctx;

	void* m_thread_handle;
	unsigned long m_thread_id;
};

} // namespace

#endif //_HIREDIS_CPP_H_
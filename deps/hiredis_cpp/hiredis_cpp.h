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

class DllExport HiredisCpp
{
public:
	HiredisCpp();
	virtual ~HiredisCpp();
	
	bool connect(const std::string &in_host, const int in_port, const bool in_blocking = true, const int in_timeout_sec = -1);
	void* connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback = 0, RedisCallback* in_disconnect_callback = 0);
	void disconnect();

	int setTimeout(const int in_seconds);
	int enableKeepAlive();

	RedisReply* exec(const std::string &in_command_string, RedisCallback* in_callback = 0, void *in_pdata = 0);
	RedisReply* exec(const std::vector<std::string> &in_command_vector);
	RedisReply* subscribe(const std::string &in_channel, RedisCallback* in_connect_callback = 0);
	RedisReply* subscribe(const std::vector<std::string> &in_channel_vector, RedisCallback* in_connect_callback = 0);
	RedisReply* unsubscribe(const std::string &in_channel);
	RedisReply* unsubscribe(const std::vector<std::string> &in_channel_vector);
	RedisReply* publish(const std::string &in_channel, const std::string &in_msg);

	int writePendingCommands();
	RedisReply* getReply();
	void discardReply();
	static void freeRedisReply(RedisReply* in_reply);
	
private:
	HiredisCpp(const HiredisCpp& other);
	HiredisCpp& operator=(const HiredisCpp&);

	bool checkCommandString(const std::string& in_command_string);
	RedisCommand* prepareCommand(const std::string &in_command_string, RedisCallback* in_callback, void* in_pdata);
	RedisReply* execCommand(RedisCommand *in_command, RedisCallback* in_callback);
	std::vector<RedisReply*> getPendingReplies(const bool in_discard = false);

	// low level API calls ...
	int readRedisBuffer(redisReply** out_reply);
	
	RedisContext* m_p_redis_ctx;
	void* m_mutex_redis_ctx;

	RedisContext* m_p_pubsub_ctx;
	void* m_mutex_pubsub_ctx;
};

} // namespace

#endif //_HIREDIS_CPP_H_
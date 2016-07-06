#ifndef _HIREDIS_CPP_H_
#define _HIREDIS_CPP_H_

#include <string>
#include <iostream>
#include <algorithm>
#include <stdarg.h>

#include "rediscontext.h"
#include "redisreader.h"
#include "rediscommand.h"
#include "rediscommandcache.h"
#include "rediscallback.h"
#include "redisreply.h"
#include "global.h"

#ifdef ERROR_ON_WRONG_CALLBACK_TYPE
#undef ERROR_ON_WRONG_CALLBACK_TYPE
#endif
#define ERROR_ON_WRONG_CALLBACK_TYPE 1

#ifdef ERROR_ON_PUBSUB_FAIL
#undef ERROR_ON_PUBSUB_FAIL
#endif
#define ERROR_ON_PUBSUB_FAIL 1

namespace HIREDIS_CPP
{

// ----------------------------------------------------------------------------
//
// class HiredisCpp
//
// ----------------------------------------------------------------------------
class DllExport HiredisCpp
{
public:
	HiredisCpp();
	virtual ~HiredisCpp();
	
	// establish a sync connection in blocking/non-blocking mode ...
	bool connect(const std::string &in_host, const int in_port, const bool in_blocking = true, const int in_timeout_sec = -1, const bool in_disable_pub_sub = false);
	// establish a async connection ...
	void* connectAsync(const std::string &in_host, const int in_port, RedisCallback* in_connect_callback = 0, RedisCallback* in_disconnect_callback = 0, const bool in_disable_pub_sub = false);
	// release a sync/async connection ...
	void disconnect();

	// turn on some redis features - not tested yet! ...
	int setTimeout(const int in_seconds);
	int enableKeepAlive();

	// execute a single redis command ...
	RedisReply* exec(const RedisCommand& in_command);
	// execute a list of reis commands in pipeline mode - only available/necessary in sync blocking mode ...
	RedisReply* exec(const std::vector<RedisCommand*> &in_command_vector);
	// subscribe to a channel - for each message received a message callback will be called ...
	// receiving is always done in async mode ...
	void subscribe(const std::string &in_channel, RedisCallback* in_message_callback = 0, const bool in_pattern_sub = false);
	// subscribe to a list of channels - for each message received a message callback will be called ...
	// receiving is always done in async mode ...
	void subscribe(const std::vector<std::string> &in_channel_vector, RedisCallback* in_message_callback = 0, const bool in_pattern_sub = false);
	// unsubscribe a channel ...
	void unsubscribe(const std::string &in_channel, const bool in_pattern_sub = false);
	// unsubscribe a list of channels ...
	void unsubscribe(const std::vector<std::string> &in_channel_vector, const bool in_pattern_sub = false);
	// publish a message to a specific channel ...
	// sending is always done in async mode ...
	void publish(const std::string &in_channel, const std::string &in_msg);
	// publish a message to a list of channels ...
	// sending is always done in async mode ...
	void publish(const std::vector<::std::string> &in_channel_vector, const std::string &in_msg);

	// write pending commands - only necessary in sync non-blocking mode ...
	int writePendingCommands();
	// get all pending replies - reply is REDIS_REPLY_TYPE_ARRAY ...
	RedisReply* getReply();
	// discard all pending replies ...
	void discardReply();
	// free received reply ...
	static void freeRedisReply(RedisReply* in_reply);
	
private:
	HiredisCpp(const HiredisCpp& other);
	HiredisCpp& operator=(const HiredisCpp&);

	// check if server is running ...
	bool isServerRunning();
	// establish async connection for pub/sub ...
	void* connectPubSub(const std::string &in_host, const int in_port);
	// filter pub/sub commands from exec() calls (command context) ...
	bool checkCommand(const RedisCommand& in_command);
	// execute redis command within a specific context ...
	RedisReply* execCommand(const RedisContext& in_context, const RedisCommand& in_command);
	// get all pending replies in a vector ...
	std::vector<RedisReply*> getPendingReplies(const bool in_discard = false);
	// low level read from redis in-buffer ...
	int readRedisBuffer(redisReply** out_reply);
	
	// redis command context ...
	RedisContext m_redis_ctx;
#ifdef _WIN32
	void* m_mutex_redis_ctx;
#else
	pthread_mutex_t* m_mutex_redis_ctx;
#endif

	// redis pub/sub context ...
	RedisContext m_pubsub_ctx;
#ifdef _WIN32
	void* m_mutex_pubsub_ctx;
#else
	pthread_mutex_t* m_mutex_pubsub_ctx;
#endif

};

} // namespace

#endif //_HIREDIS_CPP_H_
#ifndef _HIREDIS_CPP_H_
#define _HIREDIS_CPP_H_

#include <string>
#include <queue>
#include <map>
#include "rediscontext.h"
#include "redisreader.h"
#include "rediscommand.h"
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
	bool connectAsync(const std::string &in_host, const int in_port);
	int setTimeout(const int in_seconds);
	int enableKeepAlive();

	RedisReader& getReader(const bool in_default = false);
	const RedisReply& exec(const std::string &in_command_string);
	
private:
	HiredisCpp(const HiredisCpp& other);
	HiredisCpp& operator=(const HiredisCpp&);

	void resetRedisCtx();
	void resetLastCommand();

	RedisContext m_redis_ctx;
	RedisReader m_default_reader;
	RedisCommand m_last_command;
};

} // namespace

#endif //_HIREDIS_CPP_H_
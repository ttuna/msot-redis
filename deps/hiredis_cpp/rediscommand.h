#ifndef _REDISCOMMAND_H_
#define _REDISCOMMAND_H_

#include <string>
#include "redisreply.h"
#include "global.h"

namespace HIREDIS_CPP
{

class RedisCallback;

class DllExport RedisCommand
{
	friend class HiredisCpp;
	friend class RedisCommandCache;
public:
	virtual ~RedisCommand();
	bool isValid() const;
	void cleanup();

	std::string getCommandString() const;
	void setCommandString(const std::string& in_command);

private:
	RedisCommand(const std::string &in_cmd = std::string(""));
	RedisCommand(const RedisCommand& other);
	RedisCommand& operator=(const RedisCommand&);

	std::string m_command_string;
};

}

#endif //_REDISCOMMAND_H_
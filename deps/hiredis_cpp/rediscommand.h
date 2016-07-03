#ifndef _REDISCOMMAND_H_
#define _REDISCOMMAND_H_

#include <string>
#include <vector>
#include <algorithm>
#include <iostream> 

#include "global.h"

namespace HIREDIS_CPP
{

class RedisCallback;

// ----------------------------------------------------------------------------
//
// class RedisCommand
//
// ----------------------------------------------------------------------------
class DllExport RedisCommand
{
	friend class HiredisCpp;
	friend class RedisCommandCache;

public:
	RedisCommand(const std::string &in_cmd = std::string(""), RedisCallback* in_callback = 0);
	virtual ~RedisCommand();
	bool isValid() const;
	void cleanup();

	void setCallback(RedisCallback* in_callback);
	RedisCallback* getCallback();

	void addCommandToken(const std::string& in_command);
	void addCommandToken(const std::vector<std::string>& in_tokens);
	
	std::string getCommand() const;
	std::string getArguments() const;
	char* createArgVaList() const;
	void freeArgVaList(char* in_va_list) const;

	// ----------------------------------------------------------------------------
	// not a member !!!
	friend std::ostream& operator<<(std::ostream& lhs, RedisCommand const& rhs)
	{
		if (rhs.m_command_tokens.empty()) return lhs;

		for (unsigned int i=0; i<rhs.m_command_tokens.size(); ++i)
		{

			lhs << ((i==0) ? rhs.m_command_tokens[i] : std::string(" " + rhs.m_command_tokens[i]));
		}

		return lhs;
	};
	// ----------------------------------------------------------------------------
	// not a member !!!
	friend std::istream& operator>>(std::istream& lhs, RedisCommand& rhs)
	{
		std::string data;
		lhs >> data;
		rhs.m_command_tokens.push_back(data);
		return lhs;
	};
	// ----------------------------------------------------------------------------
	// not a member !!!
	friend RedisCommand& operator<<(RedisCommand& lhs, const std::string &rhs)
	{
		lhs.m_command_tokens.push_back(rhs);
		return lhs;
	}
	// ----------------------------------------------------------------------------
	// not a member !!!
	friend RedisCommand& operator<<(RedisCommand& lhs, const char* rhs)
	{
		lhs.m_command_tokens.push_back(rhs);
		return lhs;
	}

private:
	RedisCommand(const RedisCommand& other);
	RedisCommand& operator=(const RedisCommand&);

	std::vector<std::string> m_command_tokens;
	RedisCallback* m_p_callback;
};

}

#endif //_REDISCOMMAND_H_
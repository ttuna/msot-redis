#ifndef _REDISREADER_H_
#define _REDISREADER_H_

#include <string>
#include "redisreply.h"
#include "global.h"

struct redisReader;

namespace HIREDIS_CPP
{

class RedisReply;

// ----------------------------------------------------------------------------
//
// class RedisReader
//
// ----------------------------------------------------------------------------
class DllExport RedisReader
{
	friend class HiredisCpp;
public:
	virtual ~RedisReader();
	bool isValid() const;
	void cleanup();

	int feed(const std::string &in_data);
	const RedisReply& getReply();

private:
	RedisReader(const bool in_create_hiredis_reader = true);
	RedisReader(const RedisReader& other);
	RedisReader& operator=(const RedisReader&);

	redisReader* m_p_hiredis_reader;
	redisReply* m_p_hiredis_reply;
	RedisReply m_default_reply;
};

} //namespace

#endif
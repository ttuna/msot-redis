#ifndef _REDISREADER_H_
#define _REDISREADER_H_

#include <string>
#include "global.h"

struct redisReader;

namespace HIREDIS_CPP
{

class DllExport RedisReader
{
	friend class HiredisCpp;
public:
	virtual ~RedisReader() {}
	bool isValid();
	void cleanup();

	int feed(const std::string &in_data);
	int getReply(void **reply);

private:
	RedisReader();
	RedisReader(const RedisReader& other);
	RedisReader& operator=(const RedisReader&);

	redisReader* m_p_hiredis_reader;
};

} //namespace

#endif
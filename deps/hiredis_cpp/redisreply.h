#ifndef _REDISREPLY_H_
#define _REDISREPLY_H_

#include "global.h"
#include <string>
#include <vector>

struct redisReply;

namespace HIREDIS_CPP
{

typedef struct 
{
	std::string str;
	long long num;

} RedisReplyData;

class DllExport RedisReply
{
	friend class HiredisCpp;
	friend class RedisReader;
	friend class RedisCommand;
	friend class RedisCallback;
public:
	virtual ~RedisReply();
	bool isValid() const;
	void cleanup();

	int getType() const;
	int getSize() const;
	void *getData();
	std::string getStringData() const;

private:
	static std::vector<RedisReply*> createReply(const redisReply* in_reply);
	RedisReply();
	RedisReply(const RedisReply& other);
	RedisReply& operator=(const RedisReply&);

	redisReply* m_p_hiredis_reply;
};

} // namespace

#endif //_REDISREPLY_H_
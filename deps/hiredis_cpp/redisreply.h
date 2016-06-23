#ifndef _REDISREPLY_H_
#define _REDISREPLY_H_

#include "global.h"
#include <string>
#include <vector>

struct redisReply;

namespace HIREDIS_CPP
{

enum RedisReplyType {
	REDIS_REPLY_TYPE_UNKNOWN = 0,
	REDIS_REPLY_TYPE_STRING = 1,	// = #define REDIS_REPLY_STRING 1 in hiredis.h
	REDIS_REPLY_TYPE_ARRAY = 2,		// = #define REDIS_REPLY_ARRAY 2 in hiredis.h
	REDIS_REPLY_TYPE_INTEGER = 3,	// = #define REDIS_REPLY_INTEGER 3 in hiredis.h
	REDIS_REPLY_TYPE_NIL = 4,		// = #define REDIS_REPLY_NIL 4 in hiredis.h
	REDIS_REPLY_TYPE_STATUS = 5,	// = #define REDIS_REPLY_STATUS 5 in hiredis.h
	REDIS_REPLY_TYPE_ERROR = 6		// = #define REDIS_REPLY_ERROR 6 in hiredis.h
};

class RedisReply;
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
class DllExport RedisReplyData
{
	friend class RedisReply;
	friend class HiredisCpp;
public:
	virtual ~RedisReplyData();
	void cleanup();

	RedisReplyType getValueType() const;
	std::string getErrorValue() const;
	std::string getStatusValue() const;
	std::string getStringValue() const;
	int getIntValue() const;
	std::vector<RedisReply*> getVectorValue();

private:
	RedisReplyData();
	RedisReplyData(const RedisReplyData& other);
	RedisReplyData& operator=(const RedisReplyData&);

	RedisReplyType m_type;

	std::string m_str;
	long long m_int;
	std::vector<RedisReply*> m_arr;
};

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
class DllExport RedisReply
{
	friend class HiredisCpp;
	friend class RedisReader;
	//friend class RedisCommand;
	//friend class RedisCallback;
	friend class RedisReplyData;
public:
	virtual ~RedisReply();
	bool isValid() const;
	void cleanup();

	RedisReplyType getType() const;
	RedisReplyData* getData();
	std::string getErrorData() const;
	std::string getStatusData() const;
	std::string getStringData() const;
	int getIntData() const;
	std::vector<RedisReply*> getVectorData();

private:
	RedisReply();
	RedisReply(const RedisReply& other);
	RedisReply& operator=(const RedisReply&);
	static RedisReply* createReply(const redisReply* in_reply);

	RedisReplyData m_reply_data;
	redisReply* m_p_hiredis_reply;
};

} // namespace

#endif //_REDISREPLY_H_
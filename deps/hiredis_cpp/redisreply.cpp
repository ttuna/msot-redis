#include "redisreply.h"

#include "../hiredis/win32_hiredis.h"

#include <sstream>
#include <algorithm>

using namespace HIREDIS_CPP;
// ----------------------------------------------------------------------------
//
// class RedisReplyData
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReplyData::RedisReplyData() :
	m_type(REDIS_REPLY_TYPE_UNKNOWN),
	m_str(""),
	m_int(0)
{
}

RedisReplyData::~RedisReplyData()
{
	cleanup();
}

RedisReplyData::RedisReplyData(const RedisReplyData& other) :
	m_type(other.m_type),
	m_str(other.m_str),
	m_int(other.m_int)
{
}

RedisReplyData& RedisReplyData::operator=(const RedisReplyData& rhs)
{
	m_type = rhs.m_type;
	m_str = rhs.m_str;
	m_int = rhs.m_int;

	return *this;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisReplyData::cleanup()
{
	m_type = REDIS_REPLY_TYPE_UNKNOWN;
	m_str = std::string("");
	m_int = 0;
	m_arr.clear(); // no deletion of RedisReply pointers ...
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReplyType RedisReplyData::getValueType() const
{
	return m_type;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisReplyData::getErrorValue() const
{
	return m_str;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisReplyData::getStatusValue() const
{
	return m_str;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisReplyData::getStringValue() const
{
	std::ostringstream os;
	switch(m_type)
	{
		case REDIS_REPLY_TYPE_UNKNOWN :
		case REDIS_REPLY_TYPE_NIL :
			break;
		case REDIS_REPLY_TYPE_STRING :
		case REDIS_REPLY_TYPE_STATUS :
		case REDIS_REPLY_TYPE_ERROR :
			return m_str;
		case REDIS_REPLY_TYPE_INTEGER:
			os << m_int;
			return os.str();
		case REDIS_REPLY_TYPE_ARRAY :
			std::string buffer;
			for (int i=0; i<m_arr.size(); ++i)
			{
				RedisReply* reply = m_arr.at(i);
				if (reply == 0) continue;

				buffer += reply->getStringData();
				if (i < m_arr.size() -1) buffer += "\n";
			}
			return buffer;
	}
	return std::string("");
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int RedisReplyData::getIntValue() const
{
	int ret = 0;
	std::istringstream is;
	switch(m_type)
	{
		case REDIS_REPLY_TYPE_UNKNOWN :
		case REDIS_REPLY_TYPE_NIL :
			break;
		case REDIS_REPLY_TYPE_STRING :
		case REDIS_REPLY_TYPE_STATUS :
		case REDIS_REPLY_TYPE_ERROR :
			is.str(m_str);
			is >> ret;
			break;
		case REDIS_REPLY_TYPE_INTEGER:
			return m_int;
		case REDIS_REPLY_TYPE_ARRAY :
			break;
	}
	return ret;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::vector<RedisReply*> RedisReplyData::getVectorValue()
{
	RedisReply* rep = 0;
	std::vector<RedisReply*> ret;
	switch(m_type)
	{
		case REDIS_REPLY_TYPE_UNKNOWN :
		case REDIS_REPLY_TYPE_NIL :
			break;
		case REDIS_REPLY_TYPE_STRING :
		case REDIS_REPLY_TYPE_STATUS :
		case REDIS_REPLY_TYPE_ERROR :
		case REDIS_REPLY_TYPE_INTEGER:
			rep = new RedisReply;
			if (rep == 0) return ret;
			rep->m_reply_data = *this;
			ret.push_back(rep);
			break;
		case REDIS_REPLY_TYPE_ARRAY :
			return m_arr;
	}
	return ret;
}


// ----------------------------------------------------------------------------
//
// class RedisReply
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* RedisReply::createReply(const redisReply* in_reply)
{
	if (in_reply == 0) return 0;

	RedisReply* rep = 0;
	switch (in_reply->type)
	{
		case REDIS_REPLY_TYPE_UNKNOWN :
		case REDIS_REPLY_TYPE_NIL:
			break;
		case REDIS_REPLY_TYPE_STRING:
		case REDIS_REPLY_TYPE_ERROR:
		case REDIS_REPLY_TYPE_STATUS:
		{
			rep = new RedisReply;
			if (rep == 0) return 0;

			rep->m_p_hiredis_reply = const_cast<redisReply*>(in_reply);
			rep->m_reply_data.m_str = rep->m_p_hiredis_reply->str;
			rep->m_reply_data.m_type = (RedisReplyType)in_reply->type;
		} break;
		case REDIS_REPLY_TYPE_INTEGER:
		{
			rep = new RedisReply;
			if (rep == 0) return 0;

			rep->m_p_hiredis_reply = const_cast<redisReply*>(in_reply);
			rep->m_reply_data.m_int = rep->m_p_hiredis_reply->integer;
			rep->m_reply_data.m_type = (RedisReplyType)in_reply->type;
		} break;
		case REDIS_REPLY_TYPE_ARRAY:
		{
			rep = new RedisReply;
			if (rep == 0) return 0;

			for (int i=0; i<in_reply->elements; ++i)
			{
				RedisReply* buffer;
				buffer = createReply(in_reply->element[i]);
				if (buffer == 0) continue;
				
				rep->m_reply_data.m_arr.push_back(buffer);
			}
			rep->m_reply_data.m_type = (RedisReplyType)in_reply->type;
		} break;
	}
	return rep;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply::RedisReply() :
	m_p_hiredis_reply(0)
{
	
}

RedisReply::~RedisReply()
{
	cleanup();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisReply::isValid() const
{
	if (m_p_hiredis_reply == 0) return false;

	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisReply::cleanup()
{
	if (m_p_hiredis_reply == 0) return;

	freeReplyObject(m_p_hiredis_reply);
	m_p_hiredis_reply = 0;
}

// ----------------------------------------------------------------------------
// 
// ----------------------------------------------------------------------------
RedisReplyType RedisReply::getType() const
{
	return m_reply_data.m_type;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReplyData* RedisReply::getData()
{
	return &m_reply_data;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisReply::getStringData() const
{
	return m_reply_data.getStringValue();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisReply::getErrorData() const
{
	return m_reply_data.getErrorValue();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisReply::getStatusData() const
{
	return m_reply_data.getStatusValue();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int RedisReply::getIntData() const
{
	return m_reply_data.getIntValue();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::vector<RedisReply*> RedisReply::getVectorData()
{
	return m_reply_data.getVectorValue();
}

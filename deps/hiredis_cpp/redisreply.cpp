#include "redisreply.h"

#ifdef _WIN32
#include "../hiredis/win32_hiredis.h"
#else
#include "../hiredis/hiredis.h"
#endif

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::vector<RedisReply*> RedisReply::createReply(const redisReply* in_reply)
{
	if (in_reply == 0) return std::vector<RedisReply*>();

	std::vector<RedisReply*> ret;
	RedisReply* rep = 0;
	switch (in_reply->type)
	{
		case REDIS_REPLY_STRING:
		case REDIS_REPLY_ERROR:
		case REDIS_REPLY_STATUS:
		case REDIS_REPLY_INTEGER:
		{
			rep = new RedisReply;
			if (rep == 0) return std::vector<RedisReply*>();

			rep->m_p_hiredis_reply = const_cast<redisReply*>(in_reply);
			ret.push_back(rep);
		}
		case REDIS_REPLY_ARRAY:
		{
			for (int i=0; i<in_reply->elements; ++i)
			{
				std::vector<RedisReply*> buffer;
				buffer = createReply(in_reply->element[i]);
				if (buffer.size() == 0) continue;

				ret.push_back(buffer[0]);
			}
		}
	}
	return ret;
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
int RedisReply::getType() const
{
	if (m_p_hiredis_reply == 0) return REDIS_ERR;
	return m_p_hiredis_reply->type;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int RedisReply::getSize() const
{
	if (m_p_hiredis_reply == 0) return REDIS_ERR;
	// TODO ...
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void *RedisReply::getData()
{
	if (m_p_hiredis_reply == 0) return NULL;

	switch (m_p_hiredis_reply->type)
	{
		case REDIS_REPLY_STRING:
		case REDIS_REPLY_ERROR:
		case REDIS_REPLY_STATUS:
			return (void*)m_p_hiredis_reply->str;
		case REDIS_REPLY_ARRAY:
			//return (void*)m_p_hiredis_reply->element;
		case REDIS_REPLY_INTEGER:
			return (void*)&m_p_hiredis_reply->integer;
	}
	// TODO ...
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisReply::getStringData() const
{
	if (m_p_hiredis_reply == 0) return "";

	switch (m_p_hiredis_reply->type)
	{
		case REDIS_REPLY_STRING:
		case REDIS_REPLY_ERROR:
		case REDIS_REPLY_STATUS:
			return m_p_hiredis_reply->str;
		case REDIS_REPLY_ARRAY:
			return "ArrayData...";
		case REDIS_REPLY_INTEGER:
		{
			char buffer[21];
			sprintf(buffer,"%d",m_p_hiredis_reply->integer);
			return std::string(buffer);
		}
	}
	// TODO ...
}
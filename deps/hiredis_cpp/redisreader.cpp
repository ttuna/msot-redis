#include "redisreader.h"
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
RedisReader::RedisReader(const bool in_create_hiredis_reader) :
	m_p_hiredis_reader(0),
	m_p_hiredis_reply(0),
	m_default_reply()
{
	if (in_create_hiredis_reader == true)
		m_p_hiredis_reader = redisReaderCreate();
}

RedisReader::~RedisReader()
{
	cleanup();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisReader::isValid() const
{
	if (m_p_hiredis_reader == 0) return false;

	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisReader::cleanup()
{
	if (m_p_hiredis_reader != 0)
	{
		redisReaderFree(m_p_hiredis_reader);
		m_p_hiredis_reader = 0;
	}
	if (m_p_hiredis_reply != 0)
	{
		freeReplyObject((void*)m_p_hiredis_reader);
		m_p_hiredis_reader = 0;
	}
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int RedisReader::feed(const std::string &in_data)
{
	// TODO ...
	return REDIS_OK;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
const RedisReply& RedisReader::getReply()
{
	m_default_reply.cleanup();
	m_p_hiredis_reply = NULL;

	if (redisReaderGetReply(m_p_hiredis_reader, (void**)&m_p_hiredis_reply) == REDIS_ERR)
		fprintf(stdout, "redisReaderGetReply failed!\n");
	else
		m_default_reply.m_p_hiredis_reply = m_p_hiredis_reply;

	return m_default_reply;
}

#include "redisreader.h"

#ifdef _WIN32
#include "../hiredis/win32_hiredis.h"
#else
#include "../hiredis/hiredis.h"
#endif

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReader::RedisReader() :
	m_p_hiredis_reader(0)
{
	m_p_hiredis_reader = redisReaderCreate();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisReader::isValid()
{
	if (m_p_hiredis_reader == 0) return false;

	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisReader::cleanup()
{
	if (m_p_hiredis_reader == 0) return;

	redisReaderFree(m_p_hiredis_reader);
	m_p_hiredis_reader = 0;
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
int RedisReader::getReply(void **reply)
{
	// TODO ...
	return REDIS_OK;
}
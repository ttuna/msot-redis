#include "rediscommandcache.h"
#include "rediscommand.h"

#ifdef _WIN32
#include "../hiredis/win32_hiredis.h"
#else
#include "../hiredis/hiredis.h"
#endif

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCommandCache::RedisCommandCache(unsigned int in_size) :
	m_data(in_size),
	m_p_command_array(0)
{
	if (in_size > 0)
		m_p_command_array = new RedisCommand[in_size];

	if (m_p_command_array != 0)
	{
		// init command cache ...
		for (int i=0; i<in_size; ++i)
		{
			m_data[i] = &m_p_command_array[i];
		}
	}
}

RedisCommandCache::~RedisCommandCache()
{
	cleanup();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisCommandCache::isValid() const
{
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCommandCache::cleanup()
{
	m_data.clear();
	if (m_p_command_array != 0)
	{
		delete[] m_p_command_array;
		m_p_command_array = 0;
	}
}
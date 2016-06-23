#include "rediscommandcache.h"
#include "rediscommand.h"

#include "../hiredis/win32_hiredis.h"

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCommandCache::RedisCommandCache(unsigned int in_size) :
	m_data(in_size)
{
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
}
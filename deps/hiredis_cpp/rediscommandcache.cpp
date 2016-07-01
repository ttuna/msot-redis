#pragma warning(push)
#pragma warning(disable: 4251) // class 'std::vector<_Ty>' needs to have dll-interface
#include "rediscommandcache.h"
#include "rediscommand.h"
#pragma warning(pop)

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
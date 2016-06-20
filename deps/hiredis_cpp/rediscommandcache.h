#ifndef _REDISCOMMANDCACHE_H_
#define _REDISCOMMANDCACHE_H_

#include <vector>
#include "global.h"

namespace HIREDIS_CPP
{

class RedisCommand;

class DllExport RedisCommandCache
{
	friend class HiredisCpp;
public:
	virtual ~RedisCommandCache();
	bool isValid() const;
	void cleanup();

private:
	RedisCommandCache(unsigned int in_size = 0);
	RedisCommandCache(const RedisCommandCache& other);
	RedisCommandCache& operator=(const RedisCommandCache&);

	std::vector<RedisCommand*> m_data;
	RedisCommand* m_p_command_array;
};

}

#endif // _REDISCOMMANDCACHE_H_
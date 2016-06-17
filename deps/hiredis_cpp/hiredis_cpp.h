#ifndef _HIREDIS_CPP_H_
#define _HIREDIS_CPP_H_

#include <string>
#include <map>
#include "global.h"

namespace HIREDIS_CPP
{

class RedisContext;
class RedisReader;

class DllExport HiredisCpp
{
public:
	HiredisCpp();
	virtual ~HiredisCpp();
	
	void freeCtx();

	bool connect(const std::string &in_host, const int in_port);
	int setTimeout(const int in_seconds);
	int enableKeepAlive();

	RedisReader& getReader(const std::string &in_name);
	RedisReader& getDefaultReader();
	
private:
	RedisReader* createReader();

	RedisContext* m_p_redis_ctx;
	RedisReader* m_p_default_reader;
	std::map<std::string, RedisReader*> m_reader_map;
};

} // namespace

#endif //_HIREDIS_CPP_H_
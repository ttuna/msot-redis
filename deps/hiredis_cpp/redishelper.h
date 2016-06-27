#ifndef _REDISHELPER_H_
#define _REDISHELPER_H_

namespace HIREDIS_CPP 
{

class RedisCallback;

// ----------------------------------------------------------------------------
//
// class MutexLocker
//
// ----------------------------------------------------------------------------
class MutexLocker
{
public:
	MutexLocker(void* in_mutex, const unsigned long in_timeout_ms = 0xFFFFFFFF);	// 0xFFFFFFFF ... INFINITE
	~MutexLocker();

	bool isLocked();
	bool unlock();
	bool relock();

private:
	MutexLocker(const MutexLocker& other);
	MutexLocker& operator=(const MutexLocker&);
	void* m_mutex;
	unsigned long m_locked;
	unsigned long m_timeout;
};


// ----------------------------------------------------------------------------
//
// class RedisPrivateData
//
// ----------------------------------------------------------------------------
class RedisPrivateData
{
public:
	static RedisPrivateData& getInstance();
	virtual ~RedisPrivateData();

	RedisCallback* getConnectCallback();
	void setConnectCallback(RedisCallback* in_callback);

	RedisCallback* getDisconnectCallback();
	void setDisconnectCallback(RedisCallback* in_callback);

private:
	RedisPrivateData();
	RedisPrivateData(const MutexLocker& other);
	RedisPrivateData& operator=(const RedisPrivateData&);

	RedisCallback* m_connect_callback;
	RedisCallback* m_disconnect_callback;

	void* m_mutex;
};

} // namespace HIREDIS_CPP

#endif //_REDISHELPER_H_
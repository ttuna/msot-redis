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
// class RedisGlobalData
//
// ----------------------------------------------------------------------------
class RedisGlobalData
{
public:
	static RedisGlobalData& getInstance();
	virtual ~RedisGlobalData();

	RedisCallback* getConnectCallback();
	void setConnectCallback(RedisCallback* in_callback);

	RedisCallback* getDisconnectCallback();
	void setDisconnectCallback(RedisCallback* in_callback);

	RedisCallback* getCommandCallback();
	void setCommandCallback(RedisCallback* in_callback);

	RedisCallback* getMessageCallback();
	void setMessageCallback(RedisCallback* in_callback);

private:
	RedisGlobalData();
	RedisGlobalData(const MutexLocker& other);
	RedisGlobalData& operator=(const RedisGlobalData&);

	RedisCallback* m_connect_callback;
	RedisCallback* m_disconnect_callback;
	RedisCallback* m_command_callback;
	RedisCallback* m_msg_callback;

	void* m_mutex;
};

} // namespace HIREDIS_CPP

#endif //_REDISHELPER_H_
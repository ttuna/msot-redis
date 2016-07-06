#ifndef _REDISHELPER_H_
#define _REDISHELPER_H_

struct pthread_mutex_t;

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
	bool lock();

private:
	MutexLocker(const MutexLocker& other);
	MutexLocker& operator=(const MutexLocker&);
#ifdef _WIN32
	void* m_mutex;
#else
	pthread_mutex_t* m_mutex;
#endif
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

	RedisCallback* m_p_connect_callback;
	RedisCallback* m_p_disconnect_callback;
	RedisCallback* m_p_command_callback;
	RedisCallback* m_p_msg_callback;

	// TODO: add global redis context & pub/sub context (threads) ...
	// TODO: introduce switch in hiredis_cpp to toggle between client context and global context usage ...

#ifdef _WIN32
	void* m_mutex;
#else
	pthread_mutex_t* m_mutex;
#endif
};

} // namespace HIREDIS_CPP

#endif //_REDISHELPER_H_
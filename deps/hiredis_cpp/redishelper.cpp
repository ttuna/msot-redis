#pragma warning(push)
#pragma warning(disable: 4251) // class 'std::vector<_Ty>' needs to have dll-interface
#include "redishelper.h"
#include "rediscallback.h"
#pragma warning(pop)

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#include <time.h>
#endif

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// class MutexLocker
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MutexLocker::MutexLocker(void* in_mutex, const unsigned long in_timeout_ms) :
	m_mutex(in_mutex),
	m_timeout(in_timeout_ms),
	m_locked(0)
{
	if (m_mutex != 0)
	{
#ifdef _WIN32
		m_locked = WaitForSingleObject(m_mutex, m_timeout);
#else
		// TODO: change to pthread_mutex_timedlock ...
		pthread_mutex_lock(m_mutex);
#endif
	}
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MutexLocker::~MutexLocker()
{
#ifdef _WIN32
	if (m_mutex && m_locked == WAIT_OBJECT_0)
		ReleaseMutex(m_mutex);
#else
	if (m_mutex)
		pthread_mutex_unlock(m_mutex);
#endif
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool MutexLocker::isLocked()
{
#ifdef _WIN32
	return (m_mutex != 0 && m_locked == WAIT_OBJECT_0);
#else
	return (pthread_mutex_trylock(m_mutex) == 0);
#endif
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool MutexLocker::unlock()
{
	BOOL ret = FALSE;
#ifdef _WIN32
	if (m_mutex && m_locked == WAIT_OBJECT_0)
		ret = ReleaseMutex(m_mutex);
#else
	if (m_mutex)
		pthread_mutex_unlock(m_mutex);
#endif
	return (ret == TRUE);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool MutexLocker::lock()
{
#ifdef _WIN32
	if (m_mutex && m_locked != WAIT_OBJECT_0)
		m_locked = WaitForSingleObject(m_mutex, m_timeout);
	return (m_locked == WAIT_OBJECT_0);
#else
	if (m_mutex)
		pthread_mutex_lock(m_mutex);
	return (m_mutex != 0);
#endif
	
}



// ----------------------------------------------------------------------------
//
// class RedisGlobalData
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisGlobalData& RedisGlobalData::getInstance()
{
	static RedisGlobalData _instance;
	return _instance;
}

RedisGlobalData::RedisGlobalData() :
	m_connect_callback(0),
	m_disconnect_callback(0),
	m_command_callback(0),
	m_msg_callback(0)
{
#ifdef _WIN32
	m_mutex = CreateMutex(NULL,		// default security attributes
						 FALSE,		// initially not owned
						 NULL);		// unnamed mutex
#else
	m_mutex = new pthread_mutex_t;
	if (m_mutex != 0)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(m_mutex, &attr);
		pthread_mutexattr_destroy(&attr);
	}
#endif
}

RedisGlobalData::~RedisGlobalData()
{
	if (m_mutex)
	{
#ifdef _WIN32
		CloseHandle(m_mutex);
#else
		pthread_mutex_destroy(m_mutex);
		delete m_mutex;
#endif
		m_mutex = 0;
	}
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCallback* RedisGlobalData::getConnectCallback() 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return 0;

	return m_connect_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisGlobalData::setConnectCallback(RedisCallback* in_callback) 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return;

	m_connect_callback = in_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCallback* RedisGlobalData::getDisconnectCallback() 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return 0;

	return m_disconnect_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisGlobalData::setDisconnectCallback(RedisCallback* in_callback) 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return;

	m_disconnect_callback = in_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCallback* RedisGlobalData::getCommandCallback() 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return 0;

	return m_command_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisGlobalData::setCommandCallback(RedisCallback* in_callback) 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return;

	m_command_callback = in_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCallback* RedisGlobalData::getMessageCallback() 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return 0;

	return m_msg_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisGlobalData::setMessageCallback(RedisCallback* in_callback) 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return;

	m_msg_callback = in_callback; 
}

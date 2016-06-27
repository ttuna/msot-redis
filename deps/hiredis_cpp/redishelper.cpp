#include "redishelper.h"
#include "rediscallback.h"

#include "../hiredis/win32_hiredis.h"

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
		m_locked = WaitForSingleObject(m_mutex, m_timeout);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MutexLocker::~MutexLocker()
{
	if (m_mutex && m_locked == WAIT_OBJECT_0)
		ReleaseMutex(m_mutex);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool MutexLocker::isLocked()
{
	return (m_mutex != 0 && m_locked == WAIT_OBJECT_0);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool MutexLocker::unlock()
{
	BOOL ret = FALSE;
	if (m_mutex && m_locked == WAIT_OBJECT_0)
		ret = ReleaseMutex(m_mutex);

	return (ret == TRUE);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool MutexLocker::relock()
{
	if (m_mutex && m_locked != WAIT_OBJECT_0)
		m_locked = WaitForSingleObject(m_mutex, m_timeout);

	return (m_locked == WAIT_OBJECT_0);
}



// ----------------------------------------------------------------------------
//
// class RedisPrivateData
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisPrivateData& RedisPrivateData::getInstance()
{
	static RedisPrivateData _instance;
	return _instance;
}

RedisPrivateData::RedisPrivateData() :
	m_connect_callback(0),
	m_disconnect_callback(0)
{
	m_mutex = CreateMutex(NULL,		// default security attributes
						 FALSE,		// initially not owned
						 NULL);		// unnamed mutex
}

RedisPrivateData::~RedisPrivateData()
{
	if (m_mutex)
	{
		CloseHandle(m_mutex);
		m_mutex = 0;
	}
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCallback* RedisPrivateData::getConnectCallback() 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return 0;

	return m_connect_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisPrivateData::setConnectCallback(RedisCallback* in_callback) 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return;

	m_connect_callback = in_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCallback* RedisPrivateData::getDisconnectCallback() 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return 0;

	return m_disconnect_callback; 
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisPrivateData::setDisconnectCallback(RedisCallback* in_callback) 
{ 
	MutexLocker lock(m_mutex);
	if (lock.isLocked() == false) return;

	m_disconnect_callback = in_callback; 
}

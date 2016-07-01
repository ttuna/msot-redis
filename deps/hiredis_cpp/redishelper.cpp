#pragma warning(push)
#pragma warning(disable: 4251) // class 'std::vector<_Ty>' needs to have dll-interface
#include "redishelper.h"
#include "rediscallback.h"
#pragma warning(pop)

#include <Windows.h>

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
	m_mutex = CreateMutex(NULL,		// default security attributes
						 FALSE,		// initially not owned
						 NULL);		// unnamed mutex
}

RedisGlobalData::~RedisGlobalData()
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

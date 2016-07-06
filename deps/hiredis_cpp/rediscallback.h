#ifndef _REDISCALLBACK_H_
#define _REDISCALLBACK_H_

#include <iostream>
#include "global.h"

struct redisAsyncContext;
struct redisReply;

#define USE_STD_TR1_FUNCTION 1

namespace HIREDIS_CPP {
class RedisReply;
typedef void (_RedisStatusCallback)(int status);
typedef void (_RedisCommandCallback)(RedisReply* reply, void* privdata);
}

#if (__cplusplus <= 199711L)
#if (USE_STD_TR1_FUNCTION)
#include <functional>
namespace HIREDIS_CPP {
typedef std::tr1::function<_RedisStatusCallback> RedisStatusCallback;
typedef std::tr1::function<_RedisCommandCallback> RedisCommandCallback;
}
#else
namespace HIREDIS_CPP {
typedef  _RedisStatusCallback* RedisStatusCallback;
typedef _RedisCommandCallback* RedisCommandCallback;
}
#endif
#else
#include <functional>
namespace HIREDIS_CPP {
using RedisStatusCallback = std::function<_RedisCommandCallback>;
using RedisCommandCallback = std::function<_RedisCommandCallback>;
}
#endif

namespace HIREDIS_CPP
{

// ----------------------------------------------------------------------------
//
// class RedisCallback
//
// ----------------------------------------------------------------------------
class DllExport RedisCallback
{
	friend class HiredisCpp;
	friend class AsyncConnectThread;
public:
	RedisCallback(RedisStatusCallback in_status_callback, void* in_priv_data = 0, bool in_delete_after_exec = false);
	RedisCallback(RedisCommandCallback in_command_callback, void* in_priv_data = 0, bool in_delete_after_exec = false);
	virtual ~RedisCallback();
	bool isValid() const;
	void cleanup();
	RedisCallbackType callbackType() const;

	void setPrivateData(void* in_priv_data);
	void* getPrivateData();

	void setDeleteAfterExec(const bool in_delete);
	bool getDeleteAfterExec() const;

private:
	RedisCallback();
	RedisCallback(const RedisCallback& other);
	RedisCallback& operator=(const RedisCallback&);

	// frontend callback ...
	RedisStatusCallback m_status_callback;
	RedisCommandCallback m_command_callback;

	// backend callbacks ...
	static void backendConnectCallback(const struct redisAsyncContext* in_ctx, int status);
	static void backendDisconnectCallback(const struct redisAsyncContext* in_ctx, int status);
	static void backendCommandCallback(const struct redisAsyncContext* in_ctx, void* in_reply, void* in_pdata);
	static void backendPubSubCallback(const struct redisAsyncContext* in_ctx, void* in_reply, void* in_pdata);

	struct CallbackPrivateData {
		void *pdata;				// the original private data - MUST be first member!!!
		RedisCallback* callback;	// will be set to this-pointer
	} m_priv_data;

	bool m_delete_after_exec;
};

} // namespace

#endif //_REDISCALLBACK_H_
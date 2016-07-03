#ifndef _REDISCALLBACK_H_
#define _REDISCALLBACK_H_

#include <iostream>
#include "global.h"

struct redisAsyncContext;
struct redisReply;

namespace HIREDIS_CPP
{

class RedisReply;

#if (__cplusplus <= 199711L)
typedef void (RedisStatusCallback)(int status);
typedef void (RedisCommandCallback)(RedisReply* reply, void* privdata);
#else
#include <functional>
using RedisStatusCallback = std::function<void(int)>;
using RedisCommandCallback = std::function<void(RedisReply*, void*)>;
#endif

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
	RedisCallback(RedisStatusCallback* in_status_callback, void* in_priv_data = 0, bool in_delete_after_exec = false);
	RedisCallback(RedisCommandCallback* in_command_callback, void* in_priv_data = 0, bool in_delete_after_exec = false);
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
	RedisStatusCallback* m_p_status_callback;
	RedisCommandCallback* m_p_command_callback;

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
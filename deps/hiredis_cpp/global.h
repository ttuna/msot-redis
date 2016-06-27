#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define DllExport   __declspec( dllexport )

#include <string>

namespace HIREDIS_CPP 
{

const int CONNECT_TIMEOUT_SEC = 5;
const int MAX_COMMAND_ENTRIES = 100;
const std::string PUBSUB_CMD = "pubsub";
const std::string PUBLISH_CMD = "publish";
const std::string SUBSCRIBE_CMD = "subscribe";
const std::string PSUBSCRIBE_CMD = "psubscribe";
const std::string UNSUBSCRIBE_CMD = "unsubscribe";
const std::string PUNSUBSCRIBE_CMD = "punsubscribe";

enum RedisContextFlags {
	REDIS_CONTEXT_FLAG_UNKNOWN			= 0x00,
	REDIS_CONTEXT_FLAG_BLOCK			= 0x01,		// = #define REDIS_BLOCK 0x1 in hiredis.h
	REDIS_CONTEXT_FLAG_CONNECTED		= 0x02,		// = #define REDIS_CONNECTED 0x2 in hiredis.h
	REDIS_CONTEXT_FLAG_DISCONNECTING	= 0x04,		// = #define REDIS_DISCONNECTING 0x4 in hiredis.h
	REDIS_CONTEXT_FLAG_FREEING			= 0x08,		// = #define REDIS_FREEING 0x8 in hiredis.h
	REDIS_CONTEXT_FLAG_CALLBACK			= 0x10,		// = #define REDIS_IN_CALLBACK 0x10 in hiredis.h
	REDIS_CONTEXT_FLAG_SUBSCRIBED		= 0x20,		// = #define REDIS_SUBSCRIBED 0x20 in hiredis.h
	REDIS_CONTEXT_FLAG_MONITORING		= 0x40		// = #define REDIS_MONITORING 0x40 in hiredis.h
};

enum RedisReplyType {
	REDIS_REPLY_TYPE_UNKNOWN			= 0,
	REDIS_REPLY_TYPE_STRING				= 1,		// = #define REDIS_REPLY_STRING 1 in hiredis.h
	REDIS_REPLY_TYPE_ARRAY				= 2,		// = #define REDIS_REPLY_ARRAY 2 in hiredis.h
	REDIS_REPLY_TYPE_INTEGER			= 3,		// = #define REDIS_REPLY_INTEGER 3 in hiredis.h
	REDIS_REPLY_TYPE_NIL				= 4,		// = #define REDIS_REPLY_NIL 4 in hiredis.h
	REDIS_REPLY_TYPE_STATUS				= 5,		// = #define REDIS_REPLY_STATUS 5 in hiredis.h
	REDIS_REPLY_TYPE_ERROR				= 6			// = #define REDIS_REPLY_ERROR 6 in hiredis.h
};

} // namespace HIREDIS_CPP

#endif
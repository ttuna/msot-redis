#include "rediscommand.h"

#ifdef _WIN32
#include "../hiredis/win32_hiredis.h"
#else
#include "../hiredis/hiredis.h"
#endif

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCommand::RedisCommand(const std::string &in_cmd) :
	m_command_string(in_cmd),
	m_reply()
{
}

RedisCommand::~RedisCommand()
{
	cleanup();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool RedisCommand::isValid() const
{
	if (m_command_string.empty()) return false;
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCommand::cleanup()
{
	m_command_string = "";
	m_reply.cleanup();
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
const RedisReply& RedisCommand::getReply() const
{
	return m_reply;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisCommand::getCommandString() const
{
	return m_command_string;
}
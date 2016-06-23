#include "rediscommand.h"
#include "redisreply.h"
#include "rediscallback.h"
#include "hiredis_cpp.h"

#include "../hiredis/win32_hiredis.h"

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCommand::RedisCommand(const std::string &in_cmd) :
	m_command_string(in_cmd),
	m_p_reply(0),
	m_p_callback(0),
	m_delete_after_callback_exec(false)
{
	m_priv_data.command = this;
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
	m_p_reply = 0;
	m_p_callback = 0;
	m_priv_data.pdata = 0;
	m_priv_data.command = 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisReply* RedisCommand::getReply()
{
	return m_p_reply;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisCommand::getCommandString() const
{
	return m_command_string;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCommand::setCommandString(const std::string& in_command)
{
	m_command_string = in_command;
}

#pragma warning(push)
#pragma warning(disable: 4251) // class 'std::vector<_Ty>' needs to have dll-interface
#include "rediscommand.h"
#include "redisreply.h"
#include "rediscallback.h"
#include "hiredis_cpp.h"
#pragma warning(pop)
 
//using namespace std::tr1::placeholders; 

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCommand::RedisCommand(const std::string &in_cmd, RedisCallback* in_callback) :
	m_p_callback(in_callback)
{
	if (in_cmd.empty() == false)
		m_command_tokens.push_back(in_cmd);
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
	if (m_command_tokens.empty()) return false;
	return true;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCommand::cleanup()
{
	m_command_tokens.clear();
	m_p_callback = 0;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisCommand::getCommand() const
{
	if (m_command_tokens.empty()) return std::string();
	// command string ranked first ...
	return m_command_tokens[0];
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::string RedisCommand::getArguments() const
{
	std::string arg_string;
	// arguments start at index 1! ...
	for (unsigned int i=1; i<m_command_tokens.size(); ++i)
		arg_string += ((i > 1) ? ' ' + m_command_tokens[i] + '\0' : m_command_tokens[i] + '\0');
	return arg_string;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
char* RedisCommand::createArgVaList() const
{
	if (m_command_tokens.size() < 2) return 0;

	void* buffer = calloc((m_command_tokens.size()-1), sizeof(char*));
	if (buffer == 0) return 0;

	memset(buffer, 0, (m_command_tokens.size()-1) * sizeof(char*));
	va_list dummy = (va_list)buffer;

	for (unsigned int i=1, j=0; i<m_command_tokens.size(); ++i, ++j)
	{
		((char**)buffer)[j] = const_cast<char*>(m_command_tokens[i].c_str());
		//printf("%s\n", ((char**)buffer)[j]);	// debug ...
	}

	return (char*)buffer;
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCommand::freeArgVaList(char* in_va_list) const
{
	if (in_va_list == 0) return;
	free(in_va_list);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCommand::addCommandToken(const std::string& in_command)
{
	if (in_command.empty()) return;
	m_command_tokens.push_back(in_command);
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCommand::addCommandToken(const std::vector<std::string>& in_tokens)
{
	if (in_tokens.empty()) return;
	m_command_tokens.insert(m_command_tokens.end(), in_tokens.begin(), in_tokens.end());
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void RedisCommand::setCallback(RedisCallback* in_callback)
{
	m_p_callback = in_callback;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
RedisCallback* RedisCommand::getCallback()
{
	return m_p_callback;
}

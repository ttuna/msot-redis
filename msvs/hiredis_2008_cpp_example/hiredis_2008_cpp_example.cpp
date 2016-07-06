// hiredis_2008_cpp_example.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma warning(push)
#pragma warning(disable: 4251) // class 'std::vector<_Ty>' needs to have dll-interface
#include "hiredis_cpp.h"
#pragma warning(pop)

#include <iostream>
#include <stdio.h>

#include <Windows.h>

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
// global
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void connect_func(int status)
{
	std::cout << "\n*** connect_func - status: " << status << std::endl << std::endl;
}
// ----------------------------------------------------------------------------
void disconnect_func(int status)
{
	std::cout << "\n*** disconnect_func - status: " << status << std::endl << std::endl;
}
// ----------------------------------------------------------------------------
void command_func(RedisReply* in_reply, void* in_pdata)
{
	std::cout << "\n*** command_func - reply:\n" << in_reply->getStringData() << std::endl << std::endl;
}
// ----------------------------------------------------------------------------
void message_func(RedisReply* in_reply, void* in_pdata)
{
	std::cout << "\n*** message_func - reply:\n" << in_reply->getStringData() << std::endl << std::endl;
}
// ----------------------------------------------------------------------------
void print_reply(RedisReply* in_reply)
{
	std::cout << in_reply->getStringData() << std::endl;
}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// class CallbackFunctions
// ----------------------------------------------------------------------------
class CallbackFunctions
{
public:
	void connect_func(int status)							{ ::connect_func(status); }
	void disconnect_func(int status)						{ ::disconnect_func(status); }
	void command_func(RedisReply* in_reply, void* in_pdata) { ::command_func(in_reply, in_pdata); }
	void message_func(RedisReply* in_reply, void* in_pdata) { ::message_func(in_reply, in_pdata); }
};
// ----------------------------------------------------------------------------

#define USE_METHOD_CALLBACK 1

int _tmain(int argc, _TCHAR* argv[])
{
	// blocking client ...
	HiredisCpp client_1;
	RedisCommand command_1;
	RedisReply* reply_1;
	std::vector<std::string> channels_1;
	std::cout << "client 1 connecting ..." << std::endl;
	if (client_1.connect("127.0.0.1", 6379) == false)
	{
		std::cout << "client 1 - connection failed! Exit ..." << std::endl;
		client_1.disconnect();
	}
	else
	{
		std::cout << "client 1 - connected ..." << std::endl;
		std::cout << "---" << std::endl;

		// ------------------------------------------------------
		std::cout << "client 1 - exec command: INFO" << std::endl;
		command_1 << "INFO";
		reply_1 = client_1.exec(command_1);
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		command_1.cleanup();

		// ------------------------------------------------------
		std::cout << "client 1 - exec command: SET blub 10" << std::endl;
		command_1 << "SET blub 10";
		reply_1 = client_1.exec(command_1);
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		command_1.cleanup();

		// ------------------------------------------------------
		std::cout << "client 1 - exec command: GET blub" << std::endl;
		command_1 << "GET blub";
		reply_1 = client_1.exec(command_1);
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		command_1.cleanup();

		// ------------------------------------------------------
		std::vector<RedisCommand*> commands;
		RedisCommand command_1_1("SET blah 10");
		RedisCommand command_1_2("INCR blah");
		RedisCommand command_1_3("GET blah");
		commands.push_back(&command_1_1);
		commands.push_back(&command_1_2);
		commands.push_back(&command_1_3);

		std::cout << "client 1 - exec command pipeline: SET blah 10 | INCR blah | GET blah" << std::endl;
		reply_1 = client_1.exec(commands);
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		commands.clear();
		command_1_1.cleanup();
		command_1_2.cleanup();
		command_1_3.cleanup();

		// ------------------------------------------------------
		std::cout << "client 1 - exec command: HMSET blubh field1 val1 field2 2 field3 17.3" << std::endl;
		command_1 << "HMSET blubh field1 val1 field2 2 field3 17.3";
		reply_1 = client_1.exec(command_1);
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		command_1.cleanup();

		// ------------------------------------------------------
		std::cout << "client 1 - exec command: HGET blubh field1" << std::endl;
		command_1 << "HGET blubh field1";
		reply_1 = client_1.exec(command_1);
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		command_1.cleanup();

		// ------------------------------------------------------
		std::cout << "client 1 - exec command: HMGET blubh field1 field2 field3" << std::endl;
		command_1 << "HMGET blubh %s %s %s" << "field1" << "field2" << "field3";
		reply_1 = client_1.exec(command_1);
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		command_1.cleanup();
	}

	// non-blocking client ...
	HiredisCpp client_2;
	RedisCommand command_2;
	RedisReply* reply_2;
	std::cout << "client 2 connecting ..." << std::endl;
	if (client_2.connect("127.0.0.1", 6379, false) == false)
	{
		std::cout << "client 2 - connection failed! Exit ..." << std::endl;
	}
	else
	{
		std::cout << "client 2 - connected ..." << std::endl;
		std::cout << "---" << std::endl;

		// ------------------------------------------------------
		// command 1 ...
		std::cout << "client 2 - exec command: SET blurb 20" << std::endl;
		command_2 << "SET blurb 20";
		client_2.exec(command_2);
		std::cout << "---" << std::endl;
		command_2.cleanup();

		// ------------------------------------------------------
		// command 2 ...
		std::cout << "client 2 - exec command: GET blurb" << std::endl;
		command_2 << "GET blurb";
		client_2.exec(command_2);
		std::cout << "---" << std::endl;
		command_2.cleanup();

		// ------------------------------------------------------
		// transmit all commands to server ...
		client_2.writePendingCommands();

		// ------------------------------------------------------
		// get reply ...
		reply_2 = client_2.getReply();
		if (reply_2 != 0)
		{
			std::cout << "client 2 - reply: " << std::endl;
			print_reply(reply_2);
			HiredisCpp::freeRedisReply(reply_2);
		}
		else
			std::cout << "client 2 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
	}

	// callbacks for async client ...
#if USE_STD_TR1_FUNCTION
#if USE_METHOD_CALLBACK
	using namespace std::tr1::placeholders;
	CallbackFunctions callbacks;
	RedisStatusCallback _connect_func = std::tr1::bind(&CallbackFunctions::connect_func, callbacks, _1);
	RedisCallback connect_callback(_connect_func);
	RedisStatusCallback _disconnect_func = std::tr1::bind(&CallbackFunctions::disconnect_func, callbacks, _1);
	RedisCallback disconnect_callback(_disconnect_func);
	RedisCommandCallback _command_func = std::tr1::bind(&CallbackFunctions::command_func, callbacks, _1, _2);
	RedisCallback command_callback(_command_func, 0, false);
	RedisCommandCallback _message_func = std::tr1::bind(&CallbackFunctions::message_func, callbacks, _1, _2);
	RedisCallback message_callback(_message_func, 0, false);
	RedisStatusCallback _wrong_callback = std::tr1::bind(&CallbackFunctions::connect_func, callbacks, _1);
	RedisCallback wrong_callback(_wrong_callback, 0, false);
#else
	RedisStatusCallback _connect_func(connect_func);
	RedisCallback connect_callback(_connect_func);
	RedisStatusCallback _disconnect_func(disconnect_func);
	RedisCallback disconnect_callback(_disconnect_func );
	RedisCommandCallback _command_func(command_func);
	RedisCallback command_callback(_command_func, 0, false);
	RedisCommandCallback _message_func(message_func);
	RedisCallback message_callback(_message_func, 0, false);
	RedisStatusCallback _wrong_callback(connect_func);
	RedisCallback wrong_callback(_wrong_callback, 0, false);
#endif
#else
	RedisCallback connect_callback(connect_func);
	RedisCallback disconnect_callback(disconnect_func);
	RedisCallback command_callback(command_func, 0, false);
	RedisCallback message_callback(message_func, 0, false);
	RedisCallback wrong_callback(connect_func, 0, false);
#endif

	// async client ...
	HiredisCpp client_3;
	RedisCommand command_3;
	HANDLE thread_handle;
	std::vector<std::string> channels_3;
	std::cout << "client 3 connecting ..." << std::endl;
	if ((thread_handle = client_3.connectAsync("127.0.0.1", 6379, (RedisCallback*)&connect_callback, (RedisCallback*)&disconnect_callback)) == 0)
	{
		std::cout << "client 3 - connection failed! Exit ..." << std::endl;
	}
	else
	{
		std::cout << "client 3 - connected ..." << std::endl;
		std::cout << "---" << std::endl;

		// ------------------------------------------------------
		std::cout << "client 3 - exec command: SET blumb 30" << std::endl;
		command_3 << "SET blumb 30";
		command_3.setCallback(&command_callback);
		client_3.exec(command_3);
		std::cout << "---" << std::endl;
		command_3.cleanup();

		// ------------------------------------------------------
		std::cout << "client 3 - exec command: GET blumb" << std::endl;
		command_3 << "GET blumb";
		command_3.setCallback(&wrong_callback);
		client_3.exec(command_3);
		std::cout << "---" << std::endl;
		command_3.cleanup();

		// ------------------------------------------------------
		std::cout << "client 3 - exec command: GET blumb" << std::endl;
		command_3 << "GET blumb";
		command_3.setCallback(&command_callback);
		client_3.exec(command_3);
		std::cout << "---" << std::endl;
		command_3.cleanup();

		// ------------------------------------------------------
		std::cout << "client 3 - subscribe: channel = laber" << std::endl;
		client_3.subscribe("laber", &message_callback);
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 1 - publish: channel = laber, msg = Hello from client_1" << std::endl;
		client_1.publish("laber", "Hello from client_1");
		std::cout << "---" << std::endl;

		// ------------------------------------------------------
		std::cout << "client 3 - subscribe: channel = plapper fluester bruell" << std::endl;
		channels_3.push_back("plapper");
		channels_3.push_back("fluester");
		channels_3.push_back("bruell");
		client_3.subscribe(channels_3, &message_callback);
		std::cout << "---" << std::endl;
		channels_3.clear();
		// ------------------------------------------------------
		std::cout << "client 1 - publish: channel = plapper fluester, msg = Again from client_1" << std::endl;
		channels_1.push_back("plapper");
		channels_1.push_back("fluester");
		client_1.publish(channels_1, "Again from client_1");
		std::cout << "---" << std::endl;
		channels_1.clear();

		// ------------------------------------------------------
		std::cout << "client 3 - unsubscribe: channel = plapper laber" << std::endl;
		channels_3.push_back("plapper");
		channels_3.push_back("laber");
		client_3.unsubscribe(channels_3);
		std::cout << "---" << std::endl;
		channels_3.clear();
		// ------------------------------------------------------
		std::cout << "client 1 - publish: channel = plapper fluester, msg = Once again from client_1" << std::endl;
		channels_1.push_back("plapper");
		channels_1.push_back("fluester");
		client_1.publish(channels_1, "Once again from client_1");
		std::cout << "---" << std::endl;
		channels_1.clear();

		// ------------------------------------------------------

		//for (int i=0; i<10; ++i)
		//{
		//	Sleep(100);	// thread switch (hopefully) ...
		//	std::cout << ".";
		//	SwitchToThread();
		//}
		//std::cout << std::endl;

		//WaitMessage();	// wait for pending callbacks ...
		
		// ------------------------------------------------------
	}

	Sleep(100);
	std::cout << "press RETURN to end application ..." << std::endl;
	getchar();

	client_1.disconnect();
	client_2.disconnect();
	client_3.disconnect();

	// wait for async-thread end ... 
	WaitForSingleObject(thread_handle, INFINITE);

	return 0;
}


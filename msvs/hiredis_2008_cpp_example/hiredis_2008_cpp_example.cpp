// hiredis_2008_cpp_example.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "hiredis_cpp.h"
#include <iostream>
#include <stdio.h>
#include <Windows.h>

using namespace HIREDIS_CPP;

// ----------------------------------------------------------------------------
void connect_func(int status)
{
	std::cout << "\nconnect_func - status: " << status << std::endl;
}
// ----------------------------------------------------------------------------
void disconnect_func(int status)
{
	std::cout << "\ndisconnect_func - status: " << status << std::endl;
}
// ----------------------------------------------------------------------------
void command_func(RedisReply* in_reply, void* in_pdata)
{
	std::cout << "\ncommand_func - reply:\n" << in_reply->getStringData() << std::endl;
}
// ----------------------------------------------------------------------------
void subscribe_func(RedisReply* in_reply, void* in_pdata)
{
	std::cout << "\nsubscribe_func - reply:\n" << in_reply->getStringData() << std::endl;
}
// ----------------------------------------------------------------------------
void print_reply(RedisReply* in_reply)
{
	std::cout << in_reply->getStringData() << std::endl;
}
// ----------------------------------------------------------------------------

int _tmain(int argc, _TCHAR* argv[])
{
	// blocking client ...
	HiredisCpp client_1;
	RedisReply* reply_1;
	if (client_1.connect("127.0.0.1", 6379) == false)
	{
		std::cout << "client 1 - connection failed! Exit ..." << std::endl;
	}
	else
	{
		std::cout << "client 1 - connected ..." << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 1 - exec command: INFO" << std::endl;
		reply_1 = client_1.exec("INFO");
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 1 - exec command: SET blub 10" << std::endl;
		reply_1 = client_1.exec("SET blub 10");
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 1 - exec command: GET blub" << std::endl;
		reply_1 = client_1.exec("GET blub");
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::vector<std::string> commands;
		commands.push_back("SET blah 10");
		commands.push_back("INCR blah");
		commands.push_back("GET blah");

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
		// ------------------------------------------------------
		std::cout << "client 1 - exec command: HMSET blubh field1 val1 field2 2 field3 17.3" << std::endl;
		reply_1 = client_1.exec("HMSET blubh field1 val1 field2 2 field3 17.3");
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 1 - exec command: HGET blubh field1" << std::endl;
		reply_1 = client_1.exec("HGET blubh field1 ");
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 1 - exec command: HMGET blubh field1 field2 field3" << std::endl;
		reply_1 = client_1.exec("HMGET blubh field1 field2 field3");
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
	}

	// non-blocking client ...
	HiredisCpp client_2;
	RedisReply* reply_2;
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
		client_2.exec("SET blurb 20");
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		// command 2 ...
		std::cout << "client 2 - exec command: GET blurb" << std::endl;
		client_2.exec("GET blurb");
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		// transmit all commands to server ...
		client_2.writePendingCommands();	
		// ------------------------------------------------------
		Sleep(1); // it takes some time to get all the replies back from server ;-)
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
		// ------------------------------------------------------
		client_2.disconnect();
	}


	// callbacks for async client ...
	RedisCallback connect_callback(connect_func);
	RedisCallback disconnect_callback(disconnect_func);
	RedisCallback data_callback(command_func);
	RedisCallback subscribe_callback(subscribe_func);

	// async client ...
	HiredisCpp client_3;
	RedisReply* reply_3;
	HANDLE thread_handle;
	if ((thread_handle = client_3.connectAsync("127.0.0.1", 6379, &connect_callback, &disconnect_callback)) == 0)
	{
		std::cout << "client 3 - connection failed! Exit ..." << std::endl;
	}
	else
	{
		std::cout << "client 3 - connected ..." << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 3 - exec command: SET blumb 30" << std::endl;
		client_3.exec("SET blumb 30", &data_callback);
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 3 - exec command: GET blumb" << std::endl;
		client_3.exec("GET blumb", &data_callback);
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 3 - exec command: SUBSCRIBE laber" << std::endl;
		client_3.exec("SUBSCRIBE laber", &subscribe_callback);
		std::cout << "---" << std::endl;
		// ------------------------------------------------------

		Sleep(100);	// just for fun ...

		// ------------------------------------------------------
		std::cout << "client 1 - exec command: PUBLISH laber Hello from client_1" << std::endl;
		reply_1 = client_1.exec("PUBLISH laber \"Hello from client_1\" ");
		if (reply_1 != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			print_reply(reply_1);
			HiredisCpp::freeRedisReply(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;

		//client_3.disconnect();
	}

	// wait for async-thread end ... 
	// TODO: end async-thread ... ;-)
	WaitForSingleObject(thread_handle, INFINITE);

	return 0;
}


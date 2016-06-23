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
	std::cout << "connect_func - status: " << status << std::endl;
}
// ----------------------------------------------------------------------------
void disconnect_func(int status)
{
	std::cout << "disconnect_func - status: " << status << std::endl;
}
// ----------------------------------------------------------------------------
void data_func(RedisReply* in_reply, void* in_pdata)
{
	std::cout << "data_func - reply:" << in_reply->getStringData() << std::endl;
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
		std::cout << "client 1 - exec command list: SET blah 10 | INCR blah | GET blah" << std::endl;
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
		std::cout << "client 1 - exec command: HMSET blurb field1 val1 field2 2 field3 17.3" << std::endl;
		reply_1 = client_1.exec("HMSET blurb field1 val1 field2 2 field3 17.3");
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
		std::cout << "client 1 - exec command: HGET blurb field1" << std::endl;
		reply_1 = client_1.exec("HGET blurb field1 ");
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
		std::cout << "client 1 - exec command: HMGET blurb field1 field2 field3" << std::endl;
		reply_1 = client_1.exec("HMGET blurb field1 field2 field3");
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
		std::cout << "client 2 - exec command: SET blub 20" << std::endl;
		reply_2 = client_2.exec("SET blub 20");
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
		// non-blocking GET - not really usefull ;-) ...
		std::cout << "client 2 - exec command: GET blub" << std::endl;
		reply_2 = client_2.exec("GET blub");
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
		//std::cout << "client 2 - get pending replies:" << std::endl;
		//reply_2 = client_2.getReply();
		//if (reply_2 != 0)
		//{
		//	std::cout << "client 2 - reply: " << std::endl;
		//	print_reply(reply_2);
		//	HiredisCpp::freeRedisReply(reply_2);
		//}
		//else
		//	std::cout << "client 2 - reply: NULL" << std::endl;
		//std::cout << "---" << std::endl;
	}

	// async client ...
	HiredisCpp client_3;
	RedisReply* reply_3;
	RedisCallback connect_callback(connect_func);
	RedisCallback disconnect_callback(disconnect_func);
	RedisCallback data_callback(data_func);
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
		std::cout << "client 3 - exec command: SET blub 30" << std::endl;
		reply_3 = client_3.exec("SET blub 20", &data_callback);
		if (reply_3 != 0)
		{
			std::cout << "client 3 - reply: " << std::endl;
			print_reply(reply_3);
			HiredisCpp::freeRedisReply(reply_3);
		}
		else
			std::cout << "client 3 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		// async GET - not really wise ;-) ...
		std::cout << "client 3 - exec command: GET blub" << std::endl;
		reply_3 = client_3.exec("GET blub", &data_callback);
		if (reply_3 != 0)
		{
			std::cout << "client 3 - reply: " << std::endl;
			print_reply(reply_3);
			HiredisCpp::freeRedisReply(reply_3);
		}
		else
			std::cout << "client 3 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------
		std::cout << "client 3 - exec command: SUBSCRIBE laber" << std::endl;
		reply_3 = client_3.exec("SUBSCRIBE laber", &data_callback);
		if (reply_3 != 0)
		{
			std::cout << "client 3 - reply: " << std::endl;
			print_reply(reply_3);
			HiredisCpp::freeRedisReply(reply_3);
		}
		else
			std::cout << "client 3 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		// ------------------------------------------------------

		Sleep(10);
		client_1.exec("PUBLISH laber Hello from client_1");
	}

	// wait for async-thread end ... 
	// TODO: end async-thread ... ;-)
	WaitForSingleObject(thread_handle, INFINITE);

	return 0;
}


// hiredis_2008_cpp_example.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "hiredis_cpp.h"
#include <iostream>
#include <stdio.h>
#include <Windows.h>

using namespace HIREDIS_CPP;

void connect_func(int status)
{
	std::cout << "connect_func - status: " << status << std::endl;
}
void disconnect_func(int status)
{
	std::cout << "disconnect_func - status: " << status << std::endl;
}
void print_reply(RedisReply* &in_reply)
{
	std::cout << in_reply->getStringData() << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// blocking client ...
	HiredisCpp client_1;
	std::vector<RedisReply*> reply_1;
	if (client_1.connect("127.0.0.1", 6379) == false)
	{
		std::cout << "client 1 - connection failed! Exit ..." << std::endl;
	}
	else
	{
		std::cout << "client 1 - connected ..." << std::endl;
		std::cout << "---" << std::endl;

		std::cout << "client 1 - exec command: INFO" << std::endl;
		reply_1 = client_1.exec("INFO");
		if (reply_1.size() != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			for_each(reply_1.begin(), reply_1.end(), print_reply);
			HiredisCpp::freeRedisReplies(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;

		std::cout << "client 1 - exec command: SET blub 10" << std::endl;
		reply_1 = client_1.exec("SET blub 10");
		if (reply_1.size() != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			for_each(reply_1.begin(), reply_1.end(), print_reply);
			HiredisCpp::freeRedisReplies(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		
		std::cout << "client 1 - exec command: GET blub" << std::endl;
		reply_1 = client_1.exec("GET blub");
		if (reply_1.size() != 0)
		{
			std::cout << "client 1 - reply: " << std::endl;
			for_each(reply_1.begin(), reply_1.end(), print_reply);
			HiredisCpp::freeRedisReplies(reply_1);
		}
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
	}

	// non-blocking client ...
	HiredisCpp client_2;
	std::vector<RedisReply*> reply_2;
	if (client_2.connect("127.0.0.1", 6379, false) == false)
	{
		std::cout << "client 2 - connection failed! Exit ..." << std::endl;
	}
	else
	{
		std::cout << "client 2 - connected ..." << std::endl;
		std::cout << "---" << std::endl;

		std::cout << "client 2 - exec command: SET blub 20" << std::endl;
		reply_2 = client_2.exec("SET blub 20");
		if (reply_2.size() != 0)
		{
			std::cout << "client 2 - reply: " << std::endl;
			for_each(reply_2.begin(), reply_2.end(), print_reply);
			HiredisCpp::freeRedisReplies(reply_2);
		}
		else
			std::cout << "client 2 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		
		// non-blocking GET - not really wise ;-) ...
		std::cout << "client 2 - exec command: GET blub" << std::endl;
		reply_2 = client_2.exec("GET blub");
		if (reply_2.size() != 0)
		{
			std::cout << "client 2 - reply: " << std::endl;
			for_each(reply_2.begin(), reply_2.end(), print_reply);
			HiredisCpp::freeRedisReplies(reply_2);
		}
		else
			std::cout << "client 2 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
	}

	// async client ...
	HiredisCpp client_3;
	std::vector<RedisReply*> reply_3;
	RedisCallback connect_callback(connect_func);
	HANDLE thread_handle;
	if ((thread_handle = client_3.connectAsync("127.0.0.1", 6379, &connect_callback)) == 0)
	{
		std::cout << "client 3 - connection failed! Exit ..." << std::endl;
	}
	{
		std::cout << "client 3 - connected ..." << std::endl;
		std::cout << "---" << std::endl;

		std::cout << "client 3 - exec command: SET blub 30" << std::endl;
		reply_3 = client_3.exec("SET blub 20");
		if (reply_3.size() != 0)
		{
			std::cout << "client 3 - reply: " << std::endl;
			for_each(reply_3.begin(), reply_3.end(), print_reply);
			HiredisCpp::freeRedisReplies(reply_3);
		}
		else
			std::cout << "client 3 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		
		// non-blocking GET - not really wise ;-) ...
		std::cout << "client 3 - exec command: GET blub" << std::endl;
		reply_3 = client_3.exec("GET blub");
		if (reply_3.size() != 0)
		{
			std::cout << "client 3 - reply: " << std::endl;
			for_each(reply_3.begin(), reply_3.end(), print_reply);
			HiredisCpp::freeRedisReplies(reply_3);
		}
		else
			std::cout << "client 3 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
	}

	WaitForSingleObject(thread_handle, INFINITE);

	return 0;
}


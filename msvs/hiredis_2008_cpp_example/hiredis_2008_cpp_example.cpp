// hiredis_2008_cpp_example.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "hiredis_cpp.h"
#include <iostream>
#include <stdio.h>
#include <Windows.h>

using namespace HIREDIS_CPP;

int _tmain(int argc, _TCHAR* argv[])
{
	// blocking client ...
	HiredisCpp client_1;
	if (client_1.connect("127.0.0.1", 6379) == false)
	{
		std::cout << "client 1 - connection failed! Exit ..." << std::endl;
	}
	else
	{
		std::cout << "client 1 - connected ..." << std::endl;
		std::cout << "---" << std::endl;

		std::cout << "client 1 - exec command: SET blub 10" << std::endl;
		const RedisReply* reply_1 = client_1.exec("SET blub 10");
		if (reply_1 != 0)
			std::cout << "client 1 - reply: " << reply_1->getStringData() << std::endl;
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		
		std::cout << "client 1 - exec command: GET blub" << std::endl;
		const RedisReply* reply_2 = client_1.exec("GET blub");
		if (reply_2 != 0)
			std::cout << "client 1 - reply: " << reply_2->getStringData() << std::endl;
		else
			std::cout << "client 1 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
	}

	// non-blocking client ...
	HiredisCpp client_2;
	if (client_2.connect("127.0.0.1", 6379, false) == false)
	{
		std::cout << "client 2 - connection failed! Exit ..." << std::endl;
	}
	else
	{
		std::cout << "client 2 - connected ..." << std::endl;
		std::cout << "---" << std::endl;

		std::cout << "client 2 - exec command: SET blub 20" << std::endl;
		const RedisReply* reply_1 = client_2.exec("SET blub 20");
		if (reply_1 != 0)
			std::cout << "client 2 - reply: " << reply_1->getStringData() << std::endl;
		else
			std::cout << "client 2 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
		
		// non-blocking GET - not really wise ;-) ...
		std::cout << "client 2 - exec command: GET blub" << std::endl;
		const RedisReply* reply_2 = client_2.exec("GET blub");
		if (reply_2 != 0)
			std::cout << "client 2 - reply: " << reply_2->getStringData() << std::endl;
		else
			std::cout << "client 2 - reply: NULL" << std::endl;
		std::cout << "---" << std::endl;
	}

	// async client ...
	//HiredisCpp client_3;
	//HANDLE thread_handle;
	//if ((thread_handle = client_3.connectAsync("127.0.0.1", 6379)) == 0)
	//{
	//	std::cout << "client 3 - connection failed! Exit ..." << std::endl;
	//}

	//WaitForSingleObject(thread_handle, INFINITE);

	return 0;
}


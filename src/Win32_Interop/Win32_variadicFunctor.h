/*
* Copyright (c), Microsoft Open Technologies, Inc.
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#include <Windows.h>
#include <string>
#include <map>
//using namespace std;

class DLLMap : std::map<std::string, HMODULE> {
public:
	static DLLMap& getInstance();

private:
	DLLMap();
	DLLMap(DLLMap const&);	  // Don't implement to guarantee singleton semantics
	void operator=(DLLMap const&); // Don't implement to guarantee singleton semantics

public:
	LPVOID getProcAddress(std::string dll, std::string functionName);
	virtual ~DLLMap();
};


#if _MSC_VER >= 1800

template <typename R, typename... T>
class dllfunctor_stdcall {
public:
	dllfunctor_stdcall(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T...))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(T... args) { return _f(args...); }

private:
	R(__stdcall *_f)(T...);
};

#else
// ============================================================================
template <typename R>
class dllfunctor_stdcall_0 {
public:
	dllfunctor_stdcall_0(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)())DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()() { return _f(); }

private:
	R(__stdcall *_f)();
};
// ============================================================================
template <typename R, typename T1>
class dllfunctor_stdcall_1 {
public:
	dllfunctor_stdcall_1(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T1))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(T1 in_param1) { return _f(in_param1); }

private:
	R(__stdcall *_f)(T1);
};
// ============================================================================
template <typename R, typename T1, typename T2>
class dllfunctor_stdcall_2 {
public:
	dllfunctor_stdcall_2(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T1,T2))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(
				T1 in_param1,
				T2 in_param2
				) 
	{ 
		return _f(
				in_param1,
				in_param2
				); }
private:
	R(__stdcall *_f)(T1,T2);
};
// ============================================================================
template <typename R, typename T1, typename T2, typename T3>
class dllfunctor_stdcall_3 {
public:
	dllfunctor_stdcall_3(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T1,T2,T3))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(
				T1 in_param1,
				T2 in_param2,
				T3 in_param3
				) 
	{ 
		return _f(
				in_param1,
				in_param2,
				in_param3
				); }
private:
	R(__stdcall *_f)(T1,T2,T3);
};
// ============================================================================
template <typename R, typename T1, typename T2, typename T3, typename T4>
class dllfunctor_stdcall_4 {
public:
	dllfunctor_stdcall_4(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T1,T2,T3,T4))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(
				T1 in_param1,
				T2 in_param2,
				T3 in_param3,
				T4 in_param4
				) 
	{ 
		return _f(
				in_param1,
				in_param2,
				in_param3,
				in_param4
				); }
private:
	R(__stdcall *_f)(T1,T2,T3,T4);
};
// ============================================================================
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
class dllfunctor_stdcall_5 {
public:
	dllfunctor_stdcall_5(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T1,T2,T3,T4,T5))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(
				T1 in_param1,
				T2 in_param2,
				T3 in_param3,
				T4 in_param4,
				T5 in_param5
				) 
	{ 
		return _f(
				in_param1,
				in_param2,
				in_param3,
				in_param4,
				in_param5
				); }
private:
	R(__stdcall *_f)(T1,T2,T3,T4,T5);
};
// ============================================================================
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class dllfunctor_stdcall_6 {
public:
	dllfunctor_stdcall_6(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T1,T2,T3,T4,T5,T6))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(
				T1 in_param1,
				T2 in_param2,
				T3 in_param3,
				T4 in_param4,
				T5 in_param5,
				T6 in_param6
				) 
	{ 
		return _f(
				in_param1,
				in_param2,
				in_param3,
				in_param4,
				in_param5,
				in_param6
				); }
private:
	R(__stdcall *_f)(T1,T2,T3,T4,T5,T6);
};
// ============================================================================
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class dllfunctor_stdcall_7 {
public:
	dllfunctor_stdcall_7(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T1,T2,T3,T4,T5,T6,T7))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(
				T1 in_param1,
				T2 in_param2,
				T3 in_param3,
				T4 in_param4,
				T5 in_param5,
				T6 in_param6,
				T7 in_param7
				) 
	{ 
		return _f(
				in_param1,
				in_param2,
				in_param3,
				in_param4,
				in_param5,
				in_param6,
				in_param7
				); }
private:
	R(__stdcall *_f)(T1,T2,T3,T4,T5,T6,T7);
};
// ============================================================================
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class dllfunctor_stdcall_8 {
public:
	dllfunctor_stdcall_8(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T1,T2,T3,T4,T5,T6,T7,T8))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(
				T1 in_param1,
				T2 in_param2,
				T3 in_param3,
				T4 in_param4,
				T5 in_param5,
				T6 in_param6,
				T7 in_param7,
				T8 in_param8
				) 
	{ 
		return _f(
				in_param1,
				in_param2,
				in_param3,
				in_param4,
				in_param5,
				in_param6,
				in_param7,
				in_param8
				); }
private:
	R(__stdcall *_f)(T1,T2,T3,T4,T5,T6,T7,T8);
};
// ============================================================================
template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
class dllfunctor_stdcall_9 {
public:
	dllfunctor_stdcall_9(std::string dll, std::string function)
	{
		_f = (R(__stdcall *)(T1,T2,T3,T4,T5,T6,T7,T8,T9))DLLMap::getInstance().getProcAddress(dll, function.c_str());
	}
	R operator()(
				T1 in_param1,
				T2 in_param2,
				T3 in_param3,
				T4 in_param4,
				T5 in_param5,
				T6 in_param6,
				T7 in_param7,
				T8 in_param8,
				T9 in_param9
				) 
	{ 
		return _f(
				in_param1,
				in_param2,
				in_param3,
				in_param4,
				in_param5,
				in_param6,
				in_param7,
				in_param8,
				in_param9
				); }
private:
	R(__stdcall *_f)(T1,T2,T3,T4,T5,T6,T7,T8,T9);
};
// ============================================================================
#endif
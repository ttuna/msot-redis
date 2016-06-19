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

#include "win32fixes.h"
#include <mswsock.h>

#include "Win32_variadicFunctor.h"
#include "Win32_CommandLine.h"

// Win32_FDAPI.h includes modified winsock definitions that are useful in BindParam below. It
// also redefines the CRT close(FD) call as a macro. This conflicts with the fstream close 
// definition. #undef solves the warning messages.
#undef close

#include <Shlwapi.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <exception>
#include <functional>
using namespace std;

#pragma comment (lib, "Shlwapi.lib")

ArgumentMap g_argMap;
vector<string> g_pathsAccessed;

string stripQuotes(string s) {
    if (s.length() >= 2) {
        if (s.at(0) == '\'' &&  s.at(s.length() - 1) == '\'') {
            if (s.length() > 2) {
                return s.substr(1, s.length() - 2);
            } else {
                return string("");
            }
        }
        if (s.at(0) == '\"' &&  s.at(s.length() - 1) == '\"') {
            if (s.length() > 2) {
                return s.substr(1, s.length() - 2);
            } else {
                return string("");
            }
        }
    }
    return s;
}

typedef class ParamExtractor {
public:
    ParamExtractor() {}
    virtual ~ParamExtractor() {}
    virtual vector<string> Extract(int argStartIndex, int argc, char** argv) = 0;
    virtual vector<string> Extract(vector<string> tokens, int StartIndex = 0) = 0;
} ParamExtractor;

typedef map<string, ParamExtractor*> RedisParamterMapper;

typedef class FixedParam : public ParamExtractor {
private:
    int parameterCount;

public:
    FixedParam(int count) {
        parameterCount = count;
    }

    vector<string> Extract(int argStartIndex, int argc, char** argv) {
        if (argStartIndex + parameterCount >= argc) {
            stringstream err;
            err << "Not enough parameters available for " << argv[argStartIndex];
            throw invalid_argument(err.str());
        }
        vector<string> params;
        for (int argIndex = argStartIndex + 1; argIndex < argStartIndex + 1 + parameterCount; argIndex++) {
            string param = string(argv[argIndex]);
            transform(param.begin(), param.end(), param.begin(), ::tolower);
            param = stripQuotes(param);
            params.push_back(param);
        }
        return params;
    }

    vector<string> Extract(vector<string> tokens, int startIndex = 0) {
        if ((int)(tokens.size() - 1) < parameterCount + startIndex) {
            stringstream err;
            err << "Not enough parameters available for " << tokens.at(0);
            throw invalid_argument(err.str());
        }
        vector<string> params;
        int skipCount = 1 + startIndex;
		vector<string>::iterator token_iter = tokens.begin();
        for (; token_iter!=tokens.end(); ++token_iter) {
            if (skipCount > 0) {
                skipCount--;
                continue;
            }
            string param = string(*token_iter);
            transform(param.begin(), param.end(), param.begin(), ::tolower);
            param = stripQuotes(param);
            params.push_back(param);
        }
        return params;
    };
} FixedParam;

static FixedParam fp0 = FixedParam(0);
static FixedParam fp1 = FixedParam(1);
static FixedParam fp2 = FixedParam(2);
static FixedParam fp3 = FixedParam(3);
static FixedParam fp4 = FixedParam(4);

typedef class SaveParams : public ParamExtractor {
public:
    SaveParams() {}

    bool isStringAnInt(string test)  {
        int x;
        char c;
        istringstream s(test);
 
        if (!(s >> x) ||            // not convertable to an int
            (s >> c)) {             // some character past the int
            return false;
        } else {
            return true;
        }
    }

    vector<string> Extract(int argStartIndex, int argc, char** argv) {
        vector<string> params;
        int argIndex = argStartIndex + 1;

        // save [seconds] [changes]
        // or 
        // save ""      -- turns off RDB persistence
        if (strcmp(argv[argIndex], "\"\"") == 0 || strcmp(argv[argIndex], "''") == 0 || strcmp(argv[argIndex], "") == 0) {
            params.push_back(argv[argIndex]);
        } else if (
            isStringAnInt(argv[argIndex]) && 
            isStringAnInt(argv[argIndex+1])) {
            params.push_back(argv[argIndex]);
            params.push_back(argv[argIndex+1]);
        } else {
            stringstream err;
            err << "Not enough parameters available for " << argv[argStartIndex];
            throw invalid_argument(err.str());
        }
        return params;
    }

    virtual vector<string> Extract(vector<string> tokens, int startIndex = 0) {
        vector<string> params;
        unsigned int parameterIndex = 1 + startIndex;
        if ((tokens.size() > parameterIndex) &&
            (tokens.at(parameterIndex) == string("\"\"") ||
            tokens.at(parameterIndex) == string("''"))) {
            params.push_back(tokens.at(parameterIndex));
        } else if ((tokens.size() > parameterIndex + 1) &&
                   isStringAnInt(tokens.at(parameterIndex)) &&
                   isStringAnInt(tokens.at(parameterIndex + 1))) {
            params.push_back(tokens.at(parameterIndex));
            params.push_back(tokens.at(parameterIndex + 1));
        } else {
            stringstream err;
            err << "Not enough parameters available for " << tokens.at(startIndex);
            throw invalid_argument(err.str());
        }
        return params;
    };

} SaveParams;

static SaveParams savep = SaveParams();

typedef class BindParams : public ParamExtractor {
public:
    BindParams() :
		f_WSAStringToAddressA(dllfunctor_stdcall_5<int, LPCSTR, INT, LPWSAPROTOCOL_INFO, LPSOCKADDR, LPINT>("ws2_32.dll", "WSAStringToAddressA"))
	{
		//f_WSAStringToAddressA = dllfunctor_stdcall_5<int, LPCSTR, INT, LPWSAPROTOCOL_INFO, LPSOCKADDR, LPINT>("ws2_32.dll", "WSAStringToAddressA");
	}

#if _MSC_VER >= 1800
    dllfunctor_stdcall<int, LPCSTR, INT, LPWSAPROTOCOL_INFO, LPSOCKADDR, LPINT> f_WSAStringToAddressA =
        dllfunctor_stdcall<int, LPCSTR, INT, LPWSAPROTOCOL_INFO, LPSOCKADDR, LPINT>("ws2_32.dll", "WSAStringToAddressA");
#else
	dllfunctor_stdcall_5<int, LPCSTR, INT, LPWSAPROTOCOL_INFO, LPSOCKADDR, LPINT> f_WSAStringToAddressA;
#endif
    bool IsIPAddress(string address) {
        SOCKADDR_IN sockaddr4;
        sockaddr4.sin_family = AF_INET;
        SOCKADDR_IN6 sockaddr6;
        sockaddr6.sin6_family = AF_INET6;
        int addr4Length = sizeof(SOCKADDR_IN);
        int addr6Length = sizeof(SOCKADDR_IN6);
        DWORD err;
        if (ERROR_SUCCESS ==
            (err = f_WSAStringToAddressA(
            address.c_str(),
            AF_INET,
            NULL,
            (LPSOCKADDR)&sockaddr4,
            &addr4Length))) {
            return true;
        } else if (ERROR_SUCCESS ==
            (err = f_WSAStringToAddressA(
            address.c_str(),
            AF_INET6,
            NULL,
            (LPSOCKADDR)&sockaddr6,
            &addr6Length))) {
            return true;
        } else {
            return false;
        }
    }

    vector<string> Extract(int argStartIndex, int argc, char** argv) {
        vector<string> params;
        int argIndex = argStartIndex + 1;

        // bind [address1] [address2] ...
        while (argIndex < argc) {
            if (IsIPAddress(argv[argIndex])) {
                string param = string(argv[argIndex]);
                transform(param.begin(), param.end(), param.begin(), ::tolower);
                param = stripQuotes(param);
                params.push_back(param);
                argIndex++;
            } else {
                break;
            }
        }
        return params;
    }

    virtual vector<string> Extract(vector<string> tokens, int startIndex = 0) {
        vector<string> params;
        int skipCount = 1 + startIndex;
		vector<string>::iterator token_iter = tokens.begin();
        for (; token_iter!=tokens.end(); ++token_iter) {
            if (skipCount > 0) {
                skipCount--;
                continue;
            }
            if (IsIPAddress(*token_iter)) {
                string param = string(*token_iter);
                transform(param.begin(), param.end(), param.begin(), ::tolower);
                param = stripQuotes(param);
                params.push_back(param);
            } else {
                break;
            }
        }
        return params;
    };

} BindParams;

static BindParams bp = BindParams();

typedef class SentinelParams : public  ParamExtractor {
private:
    RedisParamterMapper subCommands;

public:
    SentinelParams() {
		std::pair<string, ParamExtractor*> subCommands_init_array[] = 
		{
            std::pair<string, ParamExtractor*>( "monitor",                    &fp4 ),    // sentinel monitor [master name] [ip] [port] [quorum]
            std::pair<string, ParamExtractor*>( "auth-pass",                  &fp2 ),    // sentinel auth-pass [master name] [password]
            std::pair<string, ParamExtractor*>( "down-after-milliseconds",    &fp2 ),    // sentinel down-after-milliseconds [master name] [milliseconds]
            std::pair<string, ParamExtractor*>( "parallel-syncs",             &fp2 ),    // sentinel parallel-syncs [master name] [number]
            std::pair<string, ParamExtractor*>( "failover-timeout",           &fp2 ),    // sentinel failover-timeout [master name] [number]
            std::pair<string, ParamExtractor*>( "notification-script",        &fp2 ),    // sentinel notification-script [master name] [scriptPath]
            std::pair<string, ParamExtractor*>( "client-reconfig-script",     &fp2 ),    // sentinel client-reconfig-script [master name] [scriptPath]
            std::pair<string, ParamExtractor*>( "config-epoch",               &fp2 ),    // sentinel config-epoch [name] [epoch]
            std::pair<string, ParamExtractor*>( "current-epoch",              &fp1 ),    // sentinel current-epoch <epoch>
            std::pair<string, ParamExtractor*>( "leader-epoch",               &fp2 ),    // sentinel leader-epoch [name] [epoch]
            std::pair<string, ParamExtractor*>( "known-slave",                &fp3 ),    // sentinel known-slave <name> <ip> <port>
            std::pair<string, ParamExtractor*>( "known-sentinel",             &fp4 ),    // sentinel known-sentinel <name> <ip> <port> [runid]
            std::pair<string, ParamExtractor*>( "announce-ip",                &fp1 ),    // sentinel announce-ip <ip>
            std::pair<string, ParamExtractor*>( "announce-port",              &fp1 )     // sentinel announce-port <port>
        };
        subCommands = RedisParamterMapper(subCommands_init_array, subCommands_init_array + sizeof(subCommands_init_array) / sizeof(std::pair<string, ParamExtractor*>));
    }

    vector<string> Extract(int argStartIndex, int argc, char** argv) {
        stringstream err;
        if (argStartIndex + 1 >= argc) {
            err << "Not enough parameters available for " << argv[argStartIndex];
            throw invalid_argument(err.str());
        }
        if (subCommands.find(argv[argStartIndex + 1]) == subCommands.end()) {
            err << "Could not find sentinal subcommand " << argv[argStartIndex + 1];
            throw invalid_argument(err.str());
        }

        vector<string> params;
        params.push_back(argv[argStartIndex + 1]);
        vector<string> subParams = subCommands[argv[argStartIndex + 1]]->Extract(argStartIndex + 1, argc, argv);
		vector<string>::iterator p_iter = subParams.begin();
        for (; p_iter!=subParams.end(); ++p_iter) {
            transform((*p_iter).begin(), (*p_iter).end(), (*p_iter).begin(), ::tolower);
            *p_iter = stripQuotes(*p_iter);
            params.push_back(*p_iter);
        }
        return params;
    }

    vector<string> Extract(vector<string> tokens, int startIndex = 0) {
        stringstream err;
        if (tokens.size() < 2) {
            err << "Not enough parameters available for " << tokens.at(0);
            throw invalid_argument(err.str());
        }
        string subcommand = tokens.at(startIndex + 1);
        if (subCommands.find(subcommand) == subCommands.end()) {
            err << "Could not find sentinal subcommand " << subcommand;
            throw invalid_argument(err.str());
        }

        vector<string> params;
        params.push_back(subcommand);

        vector<string> subParams = subCommands[subcommand]->Extract(tokens, startIndex + 1);
		vector<string>::iterator p_iter = subParams.begin();
        for (; p_iter!=subParams.end(); ++p_iter) {
            transform((*p_iter).begin(), (*p_iter).end(), (*p_iter).begin(), ::tolower);
            *p_iter = stripQuotes(*p_iter);
            params.push_back(*p_iter);
        }
        return params;
    };

} SentinelParams;

static SentinelParams sp = SentinelParams();

std::pair<string, ParamExtractor*> g_redisArgMap_init_array[] = 
{
    // QFork flags
    std::pair<string, ParamExtractor*>( cQFork,                           &fp2 ),    // qfork [QForkControlMemoryMap handle] [parent process id]
    std::pair<string, ParamExtractor*>( cPersistenceAvailable,            &fp1 ),    // persistence-available [yes/no]

    // service commands
    std::pair<string, ParamExtractor*>( cServiceName,                     &fp1 ),    // service-name [name]
    std::pair<string, ParamExtractor*>( cServiceRun,                      &fp0 ),    // service-run
    std::pair<string, ParamExtractor*>( cServiceInstall,                  &fp0 ),    // service-install
    std::pair<string, ParamExtractor*>( cServiceUninstall,                &fp0 ),    // service-uninstall
    std::pair<string, ParamExtractor*>( cServiceStart,                    &fp0 ),    // service-start
    std::pair<string, ParamExtractor*>( cServiceStop,                     &fp0 ),    // service-stop

    // redis commands
    std::pair<string, ParamExtractor*>( "daemonize",                      &fp1 ),    // daemonize [yes/no]
    std::pair<string, ParamExtractor*>( "pidfile",                        &fp1 ),    // pidfile [file]
    std::pair<string, ParamExtractor*>( "port",                           &fp1 ),    // port [port number]
    std::pair<string, ParamExtractor*>( "tcp-backlog",                    &fp1 ),    // tcp-backlog [number]
    std::pair<string, ParamExtractor*>( "bind",                           &bp ),     // bind [address] [address] ...
    std::pair<string, ParamExtractor*>( "unixsocket",                     &fp1 ),    // unixsocket [path] 
    std::pair<string, ParamExtractor*>( "timeout",                        &fp1 ),    // timeout [value] 
    std::pair<string, ParamExtractor*>( "tcp-keepalive",                  &fp1 ),    // tcp-keepalive [value]
    std::pair<string, ParamExtractor*>( "loglevel",                       &fp1 ),    // lovlevel [value]
    std::pair<string, ParamExtractor*>( "logfile",                        &fp1 ),    // logfile [file]
    std::pair<string, ParamExtractor*>( "syslog-enabled",                 &fp1 ),    // syslog-enabled [yes/no]
    std::pair<string, ParamExtractor*>( "syslog-ident",                   &fp1 ),    // syslog-ident [string]
    std::pair<string, ParamExtractor*>( "syslog-facility",                &fp1 ),    // syslog-facility [string]
    std::pair<string, ParamExtractor*>( "databases",                      &fp1 ),    // databases [number]
    std::pair<string, ParamExtractor*>( "save",                           &savep ),  // save [seconds] [changes] or save ""
    std::pair<string, ParamExtractor*>( "stop-writes-on-bgsave-error",    &fp1 ),    // stop-writes-on-bgsave-error [yes/no] 
    std::pair<string, ParamExtractor*>( "rdbcompression",                 &fp1 ),    // rdbcompression [yes/no]
    std::pair<string, ParamExtractor*>( "rdbchecksum",                    &fp1 ),    // rdbchecksum [yes/no]
    std::pair<string, ParamExtractor*>( "dbfilename",                     &fp1 ),    // dbfilename [filename]
    std::pair<string, ParamExtractor*>( cDir,                             &fp1 ),    // dir [path]
    std::pair<string, ParamExtractor*>( "slaveof",                        &fp2 ),    // slaveof [masterip] [master port] 
    std::pair<string, ParamExtractor*>( "masterauth",                     &fp1 ),    // masterauth [master-password]
    std::pair<string, ParamExtractor*>( "slave-serve-stale-data",         &fp1 ),    // slave-serve-stale-data [yes/no]
    std::pair<string, ParamExtractor*>( "slave-read-only",                &fp1 ),    // slave-read-only [yes/no]
    std::pair<string, ParamExtractor*>( "repl-ping-slave-period",         &fp1 ),    // repl-ping-slave-period [number]
    std::pair<string, ParamExtractor*>( "repl-timeout",                   &fp1 ),    // repl-timeout [number]
    std::pair<string, ParamExtractor*>( "repl-disable-tcp-nodelay",       &fp1 ),    // repl-disable-tcp-nodelay [yes/no]
    std::pair<string, ParamExtractor*>( "repl-diskless-sync",             &fp1 ),    // repl-diskless-sync [yes/no]
    std::pair<string, ParamExtractor*>( "repl-diskless-sync-delay",       &fp1 ),    // repl-diskless-sync-delay [number]
    std::pair<string, ParamExtractor*>( "repl-backlog-size",              &fp1 ),    // repl-backlog-size [number]
    std::pair<string, ParamExtractor*>( "repl-backlog-ttl",               &fp1 ),    // repl-backlog-ttl [number]
    std::pair<string, ParamExtractor*>( "slave-priority",                 &fp1 ),    // slave-priority [number]
    std::pair<string, ParamExtractor*>( "min-slaves-to-write",            &fp1 ),    // min-slaves-to-write [number]
    std::pair<string, ParamExtractor*>( "min-slaves-max-lag",             &fp1 ),    // min-slaves-max-lag [number]
    std::pair<string, ParamExtractor*>( "requirepass",                    &fp1 ),    // requirepass [string]
    std::pair<string, ParamExtractor*>( "rename-command",                 &fp2 ),    // rename-command [command] [string]
    std::pair<string, ParamExtractor*>( "maxclients",                     &fp1 ),    // maxclients [number]
    std::pair<string, ParamExtractor*>( "maxmemory",                      &fp1 ),    // maxmemory [bytes]
    std::pair<string, ParamExtractor*>( "maxmemory-policy",               &fp1 ),    // maxmemory-policy [policy]
    std::pair<string, ParamExtractor*>( "maxmemory-samples",              &fp1 ),    // maxmemory-samples [number]
    std::pair<string, ParamExtractor*>( "appendonly",                     &fp1 ),    // appendonly [yes/no]
    std::pair<string, ParamExtractor*>( "appendfilename",                 &fp1 ),    // appendfilename [filename]
    std::pair<string, ParamExtractor*>( "appendfsync",                    &fp1 ),    // appendfsync [value]
    std::pair<string, ParamExtractor*>( "no-appendfsync-on-rewrite",      &fp1 ),    // no-appendfsync-on-rewrite [value]
    std::pair<string, ParamExtractor*>( "auto-aof-rewrite-percentage",    &fp1 ),    // auto-aof-rewrite-percentage [number]
    std::pair<string, ParamExtractor*>( "auto-aof-rewrite-min-size",      &fp1 ),    // auto-aof-rewrite-min-size [number]
    std::pair<string, ParamExtractor*>( "lua-time-limit",                 &fp1 ),    // lua-time-limit [number]
    std::pair<string, ParamExtractor*>( "slowlog-log-slower-than",        &fp1 ),    // slowlog-log-slower-than [number]
    std::pair<string, ParamExtractor*>( "slowlog-max-len",                &fp1 ),    // slowlog-max-len [number]
    std::pair<string, ParamExtractor*>( "notify-keyspace-events",         &fp1 ),    // notify-keyspace-events [string]
    std::pair<string, ParamExtractor*>( "hash-max-ziplist-entries",       &fp1 ),    // hash-max-ziplist-entries [number]
    std::pair<string, ParamExtractor*>( "hash-max-ziplist-value",         &fp1 ),    // hash-max-ziplist-value [number]
    std::pair<string, ParamExtractor*>( "list-max-ziplist-entries",       &fp1 ),    // list-max-ziplist-entries [number]
    std::pair<string, ParamExtractor*>( "list-max-ziplist-value",         &fp1 ),    // list-max-ziplist-value [number]
    std::pair<string, ParamExtractor*>( "set-max-intset-entries",         &fp1 ),    // set-max-intset-entries [number]
    std::pair<string, ParamExtractor*>( "zset-max-ziplist-entries",       &fp1 ),    // zset-max-ziplist-entries [number]
    std::pair<string, ParamExtractor*>( "zset-max-ziplist-value",         &fp1 ),    // zset-max-ziplist-value [number]
    std::pair<string, ParamExtractor*>( "hll-sparse-max-bytes",           &fp1 ),    // hll-sparse-max-bytes [number]
    std::pair<string, ParamExtractor*>( "activerehashing",                &fp1 ),    // activerehashing [yes/no]
    std::pair<string, ParamExtractor*>( "client-output-buffer-limit",     &fp4 ),    // client-output-buffer-limit [class] [hard limit] [soft limit] [soft seconds]
    std::pair<string, ParamExtractor*>( "hz",                             &fp1 ),    // hz [number]
    std::pair<string, ParamExtractor*>( "aof-rewrite-incremental-fsync",  &fp1 ),    // aof-rewrite-incremental-fsync [yes/no]
    std::pair<string, ParamExtractor*>( "aof-load-truncated",             &fp1 ),    // aof-load-truncated [yes/no]
    std::pair<string, ParamExtractor*>( "latency-monitor-threshold",      &fp1 ),    // latency-monitor-threshold [number]
    std::pair<string, ParamExtractor*>( cInclude,                         &fp1 ),    // include [path]

    // sentinel commands
    std::pair<string, ParamExtractor*>( "sentinel",                       &sp ),

    // cluster commands
    std::pair<string, ParamExtractor*>("cluster-enabled",                 &fp1),     // [yes/no]
    std::pair<string, ParamExtractor*>("cluster-config-file",             &fp1),     // [filename]
    std::pair<string, ParamExtractor*>("cluster-node-timeout",            &fp1),     // [number]
    std::pair<string, ParamExtractor*>("cluster-slave-validity-factor",   &fp1),     // [number]
    std::pair<string, ParamExtractor*>("cluster-migration-barrier",       &fp1),     // [1/0]
    std::pair<string, ParamExtractor*>("cluster-require-full-coverage",   &fp1)      // [yes/no]
};
// Map of argument name to argument processing engine.
static RedisParamterMapper g_redisArgMap = RedisParamterMapper(g_redisArgMap_init_array, g_redisArgMap_init_array + sizeof(g_redisArgMap_init_array) / sizeof(std::pair<string, ParamExtractor*>));


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty())
            elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

vector<string> Tokenize(string line)  {
    vector<string> tokens;
    stringstream token;

    // no need to parse empty lines, or comment lines (which may have unbalanced quotes)
    if ((line.length() == 0)  || 
        ((line.length() != 0) && (*line.begin()) == '#')) {
        return tokens;
    }

    for (string::const_iterator sit = line.begin(); sit != line.end(); sit++) {
        char c = *(sit);
        if (isspace(c) && token.str().length() > 0) {
            tokens.push_back(token.str());
            token.str("");
        } else if (c == '\'' || c == '\"') {
            char endQuote = c;
            string::const_iterator endQuoteIt = sit;
            while (++endQuoteIt != line.end()) {
                if (*endQuoteIt == endQuote) break;
            }
            if (endQuoteIt != line.end())  {
                while (++sit != endQuoteIt) {
                    token << (*sit);
                }

                // The code above strips quotes. In certain cases (save "") the quotes should be preserved around empty strings
                if (token.str().length() == 0)
                    token << endQuote << endQuote;

                // correct paths for windows nomenclature
                string path = token.str();
                replace(path.begin(), path.end(), '/', '\\');
                tokens.push_back(path);

                token.str("");
            } else {
                // stuff the imbalanced quote character and continue
                token << (*sit);
            }
        } else {
            token << c;
        }
    }
    if (token.str().length() > 0) {
        tokens.push_back(token.str());
    }

    return tokens;
}

void ParseConfFile(string confFile, string cwd, ArgumentMap& argMap) {
    ifstream config;
    string line;

    char fullConfFilePath[MAX_PATH];
    if (PathIsRelativeA(confFile.c_str())) {
        if (NULL == PathCombineA(fullConfFilePath, cwd.c_str(), confFile.c_str())) {
#if _MSC_VER >= 1800
            throw std::system_error(GetLastError(), system_category(), "PathCombineA failed");
#else
			throw std::runtime_error("PathCombineA failed");
#endif
        }
    } else {
        strcpy(fullConfFilePath, confFile.c_str());
    }

    config.open(fullConfFilePath);
    if (config.fail()) {
        stringstream ss;
        ss << "Failed to open the .conf file: " << confFile << " CWD=" << cwd.c_str();
        throw invalid_argument(ss.str());
    } else  {
        char confFileDir[MAX_PATH];
        strcpy(confFileDir, fullConfFilePath);
        if (FALSE == PathRemoveFileSpecA(confFileDir)) {
#if _MSC_VER >= 1800
            throw std::system_error(GetLastError(), system_category(), "PathRemoveFileSpecA failed");
#else
			throw std::runtime_error("PathRemoveFileSpecA failed");
#endif
        }
        g_pathsAccessed.push_back(confFileDir);
    }

    while (!config.eof()) {
        getline(config, line);
        vector<string> tokens = Tokenize(line);
        if (tokens.size() > 0) {
            string parameter = tokens.at(0);
            if (parameter.at(0) == '#') {
                continue;
            } else if (parameter.compare(cInclude) == 0) {
                ParseConfFile(tokens.at(1), cwd, argMap);
            } else if (g_redisArgMap.find(parameter) == g_redisArgMap.end()) {
                stringstream err;
                err << "unknown conf file parameter : " + parameter;
                throw invalid_argument(err.str());
            }

            vector<string> params = g_redisArgMap[parameter]->Extract(tokens);
            g_argMap[parameter].push_back(params);
        }
    }
}

string incompatibleNoPersistenceCommands_init_array[] =
{
    "min_slaves_towrite",
    "min_slaves_max_lag",
    "appendonly",
    "appendfilename",
    "appendfsync",
    "no_append_fsync_on_rewrite",
    "auto_aof_rewrite_percentage",
    "auto_aof_rewrite_on_size",
    "aof_rewrite_incremental_fsync",
    "save"
};
vector<string> incompatibleNoPersistenceCommands(incompatibleNoPersistenceCommands_init_array, incompatibleNoPersistenceCommands_init_array + sizeof(incompatibleNoPersistenceCommands_init_array) / sizeof(string));

void ValidateCommandlineCombinations() {
    if (g_argMap.find(cPersistenceAvailable) != g_argMap.end()) {
        if (g_argMap[cPersistenceAvailable].at(0).at(0) == cNo) {
            string incompatibleCommand = "";
			vector<string>::iterator iter = incompatibleNoPersistenceCommands.begin();
			for (; iter!=incompatibleNoPersistenceCommands.end(); ++iter)
			{
                if (g_argMap.find(*iter) != g_argMap.end()) {
                    incompatibleCommand = *iter;
                    break;
                }
            }
            if (incompatibleCommand.length() > 0) {
                stringstream ss;
                ss << "'" << cPersistenceAvailable << " " << cNo << "' command not compatible with '" << incompatibleCommand << "'. Exiting.";
                throw std::invalid_argument(ss.str().c_str());
            }
        }
    }
}

void ParseCommandLineArguments(int argc, char** argv) {
    if (argc < 2) {
        return;
    }

    bool confFile = false;
    string confFilePath;
    for (int n = (confFile ? 2 : 1); n < argc; n++) {
        if (string(argv[n]).substr(0, 2) == "--") {
            string argument = string(argv[n]).substr(2, argument.length() - 2);
            transform(argument.begin(), argument.end(), argument.begin(), ::tolower);

            // Some -- arguments are passed directly to redis.c::main()
            if (find(cRedisArgsForMainC.begin(), cRedisArgsForMainC.end(), argument) != cRedisArgsForMainC.end()) {
                if (strcasecmp(argument.c_str(), "test-memory") == 0) {
                    // The test-memory argument is followed by a integer value
                    n++;
                }
            } else {
                // -- arguments processed before calling redis.c::main()
                if (g_redisArgMap.find(argument) == g_redisArgMap.end()) {
                    stringstream err;
                    err << "unknown argument: " << argument;
                    throw invalid_argument(err.str());
                }

                vector<string> params;
                if (argument == cSentinel) {
                    try {
                        vector<string> sentinelSubCommands = g_redisArgMap[argument]->Extract(n, argc, argv);
						vector<string>::iterator iter = sentinelSubCommands.begin();
						for(; iter!=sentinelSubCommands.end(); ++iter){
                            params.push_back(*iter);
                        }
                    }
                    catch (invalid_argument iaerr) {
                        // if no subcommands could be mapped, then assume this is the parameterless --sentinel command line only argument
                    }
                } else if (argument == cServiceRun) {
                    // When the service starts the current directory is %systemdir%. This needs to be changed to the 
                    // directory the executable is in so that the .conf file can be loaded.
                    char szFilePath[MAX_PATH];
                    if (GetModuleFileNameA(NULL, szFilePath, MAX_PATH) == 0) {
#if _MSC_VER >= 1800
						throw std::system_error(GetLastError(), system_category(), "ParseCommandLineArguments: GetModuleFileName failed");
#else
						throw std::runtime_error("ParseCommandLineArguments: GetModuleFileName failed");
#endif
                    }
                    string currentDir = szFilePath;
                    size_t pos = currentDir.rfind("\\");
                    currentDir.erase(pos);

                    if (FALSE == SetCurrentDirectoryA(currentDir.c_str())) {
#if _MSC_VER >= 1800
						throw std::system_error(GetLastError(), system_category(), "SetCurrentDirectory failed");
#else
						throw std::runtime_error("SetCurrentDirectory failed");
#endif
                    }
                } else {
                    params = g_redisArgMap[argument]->Extract(n, argc, argv);
                }
                g_argMap[argument].push_back(params);
                n += (int) params.size();
            }
        } else if (string(argv[n]).substr(0, 1) == "-") {
            // Do nothing, the - arguments are passed to redis.c::main() as they are
        } else {
            confFile = true;
            confFilePath = argv[n];
        }
    }

    char cwd[MAX_PATH];
    if (0 == ::GetCurrentDirectoryA(MAX_PATH, cwd)) {
#if _MSC_VER >= 1800
		throw std::system_error(GetLastError(), system_category(), "ParseCommandLineArguments: GetCurrentDirectoryA failed");
#else
		throw std::runtime_error("ParseCommandLineArguments: GetCurrentDirectoryA failed");
#endif
    }
    
    if (confFile) {
        ParseConfFile(confFilePath, cwd, g_argMap);
    }

    // grab directory where RDB/AOF files will be created so that service install can add access allowed ACE to path
    string fileCreationDirectory = ".\\";
    if (g_argMap.find(cDir) != g_argMap.end()) {
        fileCreationDirectory = g_argMap[cDir][0][0];
        replace(fileCreationDirectory.begin(), fileCreationDirectory.end(), '/', '\\');
    }
    if (PathIsRelativeA(fileCreationDirectory.c_str())) {
        char fullPath[MAX_PATH];
        if (NULL == PathCombineA(fullPath, cwd, fileCreationDirectory.c_str())) {
#if _MSC_VER >= 1800
		throw std::system_error(GetLastError(), system_category(), "PathCombineA failed");
#else
		throw std::runtime_error("PathCombineA failed");
#endif
        }
        fileCreationDirectory = fullPath;
    }
    g_pathsAccessed.push_back(fileCreationDirectory);

    ValidateCommandlineCombinations();
}

vector<string> GetAccessPaths() {
    return g_pathsAccessed;
}


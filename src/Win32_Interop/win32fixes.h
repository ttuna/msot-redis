/*
* Modified by Henry Rawas (henryr@schakra.com)
*  - make it compatible with Visual Studio builds
*  - added wstrtod to handle INF, NAN
*  - added support for using IOCP with sockets
*/

#ifndef WIN32FIXES_H
#define WIN32FIXES_H

#pragma warning(error: 4005)
#pragma warning(error: 4013)

#ifdef WIN32
#ifndef _WIN32
#define _WIN32
#endif
#endif

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define __USE_W32_SOCKETS

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>      // for _O_BINARY
#include <limits.h>     // for INT_MAX

#include "win32_types.h"
#include "win32_fdapi.h"    
#include "win32_apis.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define WNOHANG 1

/* file mapping */
#define PROT_READ 1
#define PROT_WRITE 2

#define MAP_FAILED   (void *) -1

#define MAP_SHARED 1
#define MAP_PRIVATE 2

/* strtod does not handle Inf and Nan, we need to do the check before calling strtod */
#undef strtod
#define strtod(nptr, eptr) wstrtod((nptr), (eptr))

double wstrtod(const char *nptr, char **eptr);

// access check for executable uses X_OK. For Windows use READ access.
#ifndef X_OK
#define X_OK 4
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifdef __cplusplus
}
#endif

#endif /* WIN32FIXES_H */

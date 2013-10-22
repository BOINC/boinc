// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// boinc_win.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#ifndef _BOINC_WIN_
#define _BOINC_WIN_

#ifndef __CYGWIN32__

// Windows C Runtime Library

#ifndef HAVE_STD_MAX
#define HAVE_STD_MAX 1
#endif 

#ifndef HAVE_STD_MIN
#define HAVE_STD_MIN 1
#endif 

#ifndef HAVE_STD_TRANSFORM
#define HAVE_STD_TRANSFORM 1
#endif 

#ifndef HAVE_ALLOCA
#define HAVE_ALLOCA 1
#endif 

#ifdef __MINGW32__
#define HAVE_STRCASECMP 1
#endif

#else

// Under CYGWIN we need to include config.h first.
#include "config.h"

#endif

// Windows System Libraries
//

// Visual Studio 2005 has extended the C Run-Time Library by including "secure"
// runtime functions and deprecating the previous function prototypes.  Since
// we need to use the previous prototypes to maintain compatibility with other
// platforms we are going to disable the deprecation warnings if we are compiling
// on Visual Studio 2005
#if _MSC_VER >= 1400

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#endif

// Target Windows 2000 or better with Internet Explorer 5.01 or better
#ifndef WINVER
#define WINVER 0x0500
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0500
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif

#include <windows.h>
#include <share.h>
#include <shlobj.h>
#include <userenv.h>
#include <aclapi.h>
#include <psapi.h>
#include <iphlpapi.h>

#if !defined(__CYGWIN32__) || defined(USE_WINSOCK)

/* If we're not running under CYGWIN use windows networking */
#undef USE_WINSOCK
#define USE_WINSOCK 1
#include <winsock.h>
#include <wininet.h>

typedef size_t socklen_t;

#else 

/* Under cygwin, curl was probably compiled to use <sys/socket.h> */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#define _WINSOCK_H
#define _WINSOCKAPI_
#define _WINSOCK2_H
#define _WININET_H
#define _WININETAPI_

#endif

#include <process.h>
#if defined(__MINGW32__) || defined(__CYGWIN32__)
#include <pbt.h>
#endif

#include <commctrl.h>
#include <raserror.h>
#if defined(__MINGW32__)
#include <stdint.h>
#include <imagehlp.h>
#else
#include <dbghelp.h>
#endif
#include <tlhelp32.h>

#include <io.h>
#if !defined(__CYGWIN32__)
#include <direct.h>
#endif

#if !defined(__CYGWIN32__)
#include <tchar.h>
#else
#ifndef _TCHAR_DEFINED
typedef char TCHAR, *PTCHAR;
typedef unsigned char TBYTE , *PTBYTE ;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */
typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR, PUTSTR, LPUTSTR;
typedef LPCSTR PCTSTR, LPCTSTR, PCUTSTR, LPCUTSTR;
#define __TEXT(quote) quote
#endif

// All projects should be using std::min and std::max instead of the Windows
//   version of the symbols.
#undef min
#undef max

// Standard Libraries
//

// C headers
#include <sys/stat.h>
#include <sys/types.h>
#if !defined(__MINGW32__)
#include <fcntl.h>
#endif
#include <malloc.h>

#if !defined(__MINGW32__) && !defined(__CYGWIN32__)
#include <crtdbg.h>
#include <delayimp.h>
#endif

#ifdef __cplusplus
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cfloat>
#include <locale>
#else
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <locale.h>
#endif

// C++ headers
//
#ifdef __cplusplus
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#endif

// Define a generic string type that can be a Unicode string on
// Unicode builds and an ANSI string on ANSI builds
//
#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

#ifndef SIGRTMAX
#if defined(_SIGRTMAX)
#define SIGRTMAX _SIGRTMAX
#elif defined(NSIG)
#define SIGRTMAX (NSIG-1)
#else
#define SIGRTMAX 32
#endif
#endif

#ifndef __GNUC__
#define __attribute__(x)
#endif

#if defined(__MINGW32__)
#ifdef __cplusplus
extern "C" {
#endif
void __cdecl _fpreset (void);
void __cdecl fpreset (void);
#ifdef __cplusplus
}
#endif //cplusplus
#endif //MINGW32

#if defined(__MINGW32__) && (__GNUC__ < 4)
// breaks build on MinGW gcc-4
#define SetClassLongPtr SetClassLong
#define GCLP_HICON GCL_HICON
#define GCLP_HICONSM GCL_HICONSM
#endif //MINGW32 && GNUC < 4

// On the Win32 platform include file and line number information for each
//   memory allocation/deallocation
#ifdef _DEBUG

#define malloc(s)                             _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define calloc(c, s)                          _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define realloc(p, s)                         _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define _expand(p, s)                         _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p)                               _free_dbg(p, _NORMAL_BLOCK)
#define _msize(p)                             _msize_dbg(p, _NORMAL_BLOCK)
#define _aligned_malloc(s, a)                 _aligned_malloc_dbg(s, a, __FILE__, __LINE__)
#define _aligned_realloc(p, s, a)             _aligned_realloc_dbg(p, s, a, __FILE__, __LINE__)
#define _aligned_offset_malloc(s, a, o)       _aligned_offset_malloc_dbg(s, a, o, __FILE__, __LINE__)
#define _aligned_offset_realloc(p, s, a, o)   _aligned_offset_realloc_dbg(p, s, a, o, __FILE__, __LINE__)
#define _aligned_free(p)                      _aligned_free_dbg(p)

#ifndef DEBUG_NEW
#define DEBUG_NEW                             new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// The following macros set and clear, respectively, given bits
// of the C runtime library debug flag, as specified by a bitmask.
#define SET_CRT_DEBUG_FIELD(a)                _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define CLEAR_CRT_DEBUG_FIELD(a)              _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#else //_DEBUG

#ifndef DEBUG_NEW
#define DEBUG_NEW                             new
#endif

#define SET_CRT_DEBUG_FIELD(a)                ((void) 0)
#define CLEAR_CRT_DEBUG_FIELD(a)              ((void) 0)

#endif //_DEBUG

#define new DEBUG_NEW

#endif //_BOINC_WIN_

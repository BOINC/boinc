// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// boinc_win.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#ifndef _BOINC_WIN_
#define _BOINC_WIN_

// Under CYGWIN we need to include config.h first.
#ifdef __CYGWIN32__
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
#pragma warning(disable: 4996) // function deprecation
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                  // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400           // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT            // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINDOWS          // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0400   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0500        // Change this to the appropriate value to target IE 5.0 or later.
#endif

#define WIN32_LEAN_AND_MEAN   // This trims down the windows libraries.
#define WIN32_EXTRA_LEAN      // Trims even farther.

#include <windows.h>
#include <share.h>

#if !defined(__CYGWIN32__) || defined(USE_WINSOCK)
/* If we're not running under CYGWIN use windows networking */
#undef USE_WINSOCK
#define USE_WINSOCK 1
#include <winsock.h>
#include <wininet.h>

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
#include <mmsystem.h>
#if !defined(__CYGWIN32__)
#include <direct.h>
#endif
#include <io.h>
#if !defined(__MINGW32__) && !defined(__CYGWIN32__)
#include <crtdbg.h>
#endif
#if !defined(__CYGWIN32__)
#include <tchar.h>
#endif
#if !defined(__MINGW32__) && !defined(__CYGWIN32__)
#include <crtdbg.h>
#endif
#if !defined(__MINGW32__) && !defined(__CYGWIN32__)
#include <delayimp.h>
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
#include <fcntl.h>

#ifdef __cplusplus
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <csetjmp>
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
#endif

#ifndef __CYGWIN32__
// Some of these have been changed from defines to inlines, primarily because the
// define version changes the names of methods with classes.  For example foo::read()
// becomes foo::_read().
#if !defined(inline) && defined(_MSC_VER)
#define inline __inline
#endif
#define vsnprintf               _vsnprintf
#define snprintf                _snprintf
#define stprintf                _stprintf
static inline int stricmp(const char *s1, const char *s2) { return _stricmp(s1,s2); }
static inline FILE *fdopen(int fd, const char *mode) { return _fdopen(fd,mode); }
static inline int dup(int i) { return _dup(i); }
static inline int unlink(const char *fn) { return _unlink(fn); }
static inline char *strdup(const char *s) { return _strdup(s); }
static inline int read(int fd, void *buf, unsigned int nbyte) {return _read(fd,buf,nbyte);}
static inline int stat(const char *fn,struct stat *buf) { 
	return _stat(fn,(struct _stat *)(buf)); 
}
static inline int finite(double x) { return _finite(x); }
static inline int chdir(const char *d) { return _chdir(d); }
#endif

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

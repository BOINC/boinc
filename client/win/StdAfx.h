// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__B4B76702_EF2D_4BFC_869D_3F7E2CBE9FFC__INCLUDED_)
#define AFX_STDAFX_H__B4B76702_EF2D_4BFC_869D_3F7E2CBE9FFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes
#include <afxtempl.h>
#include <afxcoll.h>

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>		// MFC socket extensions

#include <windows.h>
#include <winsock.h>
#include <wininet.h>
#include <process.h>
#include <commctrl.h>
#include <raserror.h>
#include <mmsystem.h>
#include <direct.h>
#include <io.h>
#include <crtdbg.h>
#include <tchar.h>
#include <crtdbg.h>
#include <delayimp.h>

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
#include <cstdio>
#include <cstdlib>
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


#define vsnprintf               _vsnprintf
#define snprintf                _snprintf
#define fdopen                  _fdopen
#define dup                     _dup
#define unlink                  _unlink
#define strdup                  _strdup
#define read                    _read
#define stat                    _stat
#define finite                  _finite


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

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B4B76702_EF2D_4BFC_869D_3F7E2CBE9FFC__INCLUDED_)

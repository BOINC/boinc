// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _STDAFX_H_
#define _STDAFX_H_

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Windows System Libraries
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32

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

//
// BOINC Windows GUI Applications use MFC libraries and the like.
//
#ifndef _CONSOLE

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxcmn.h>         // MFC support for Windows Common Controls
#include <afxtempl.h>
#include <afxcoll.h>
#include <afxext.h>         // MFC extensions

#endif //_CONSOLE

#define WIN32_LEAN_AND_MEAN   // This trims down the windows libraries.
#define WIN32_EXTRA_LEAN      // Trims even farther.

#include <windows.h>
#include <winsock.h>
#include <wininet.h>
#include <process.h>
#include <commctrl.h>
#include <raserror.h>

#include "Stackwalker.h"

#endif //_WIN32

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Standard Libraries
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//
// Standard Template libraries
//
#include <cassert>
#include <cerrno>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


//
// Standard C Runtime libraries
//
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <crtdbg.h>
#include <tchar.h>


//
// Namespace Modifiers
//
using namespace std;

#endif //_STDAFX_H_
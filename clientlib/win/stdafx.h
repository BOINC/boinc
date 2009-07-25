// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Visual Studio 2005 has extended the C Run-Time Library by including "secure"
// runtime functions and deprecating the previous function prototypes.  Since
// we need to use the previous prototypes to maintain compatibility with other
// platforms we are going to disable the deprecation warnings if we are compiling
// on Visual Studio 2005
#if _MSC_VER >= 1400
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				    // Allow use of features specific to Windows 2000 or later
#define WINVER 0x0500
#endif

#ifndef _WIN32_WINNT		    // Allow use of features specific to Windows 2000 or later.
#define _WIN32_WINNT 0x0500
#endif						

#ifndef _WIN32_IE               // Allow use of features specific to IE 5.01 or later.
#define _WIN32_IE 0x0501
#endif

#include <windows.h>
#include <initguid.h>
#include <wininet.h>
#include <shellapi.h>
#include <crtdbg.h>
#include <tchar.h>

#include <vector>
#include <string>

#ifdef _UNICODE
#define tstring std::wstring
#define tostringstream std::wostringstream
#else
#define tstring std::string
#define tostringstream std::ostringstream
#endif

// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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
//

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef _STDAFX_H_
#define _STDAFX_H_

// Windows System Libraries
//

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0600
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0700
#endif

#ifndef _WIN32_MSI
#define _WIN32_MSI 400
#endif

// ATL Headers Files:
#include <atlbase.h>
#include <atlenc.h>

// Windows Header Files:
#include <windows.h>
#include <crtdbg.h>
#include <delayimp.h>
#include <msiquery.h>
// the following fixes mysterious/sporadic errors in Win include files; see
// https://stackoverflow.com/questions/73025050/how-do-i-solve-c2371-errors-that-arise-from-including-both-ntsecapi-h-and-win
//#include <Winternl.h>
//#define _NTDEF_
#include <ntsecapi.h>
#include <lm.h>
#include <shlobj.h>
#include <sddl.h>
#include <wincrypt.h>
#include <aclapi.h>
#include <winsafer.h>

// CRT Header Files:
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

// STL Header Files:
#include <cassert>
#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <array>
#include <filesystem>

// Misc Includes
#include <tchar.h>

#include <wil/resource.h>

#ifdef _UNICODE
#define tstring std::wstring
#define tostringstream std::wostringstream
#else
#define tstring std::string
#define tostringstream std::ostringstream
#endif


// Define symbols not already defined in the SDK.
//

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif

#endif


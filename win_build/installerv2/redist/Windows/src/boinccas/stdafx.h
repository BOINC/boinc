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
#define _WIN32_IE 0x0400        // Change this to the appropriate value to target IE 4.0 or later.
#endif

#ifndef _WIN32_MSI              // Need to specify the smallest version of Windows Installer
#define _WIN32_MSI 200          // that supports file hashing. This is version 2.0.
#endif

#define WIN32_LEAN_AND_MEAN   // This trims down the windows libraries.
#define WIN32_EXTRA_LEAN      // Trims even farther.


// Windows Header Files:
#include <windows.h>
#include <crtdbg.h>
#include <delayimp.h>
#include <msiquery.h>
#include <tchar.h>
#include <ntsecapi.h>
#include <lm.h>

// STL Header Files:
#include <cassert>
#include <cstdio>
#include <string>
#include <iostream>

#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif


// Define symbols not already defined in the SDK.
//

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif

#endif

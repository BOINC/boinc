// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Need to specify the smallest version of Windows Installer
// that supports file hashing. This is version 2.0.
#define _WIN32_MSI 200

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN


// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <stdio.h>
#include <msiquery.h>
#include <ntsecapi.h>

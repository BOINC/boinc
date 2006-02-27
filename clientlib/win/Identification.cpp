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


#include "stdafx.h"
#include "Identification.h"


/**
 * Find out if we are on a Windows 2000 compatible system
 **/
EXTERN_C __declspec(dllexport) BOOL IsWindows2000Compatible()
{
   OSVERSIONINFO osvi;
   ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
        return FALSE;

    return (osvi.dwMajorVersion >= 5);
}

/**
 * This function performs the basic check to see if
 * the platform on which it is running is Terminal
 * services enabled.  Note, this code is compatible on
 * all Win32 platforms.  For the Windows 2000 platform
 * we perform a "lazy" bind to the new product suite
 * APIs that were first introduced on that platform.
 **/
EXTERN_C __declspec(dllexport) BOOL IsTerminalServicesEnabled()
{
    BOOL    bResult = FALSE;    // assume Terminal Services is not enabled

    DWORD   dwVersion;
    OSVERSIONINFOEXA osVersionInfo;
    DWORDLONG dwlConditionMask = 0;
    HMODULE hmodK32 = NULL;
    HMODULE hmodNtDll = NULL;
    typedef ULONGLONG (WINAPI *PFnVerSetConditionMask)(ULONGLONG,ULONG,UCHAR);
    typedef BOOL (WINAPI *PFnVerifyVersionInfoA)(POSVERSIONINFOEXA, DWORD, DWORDLONG);
    PFnVerSetConditionMask pfnVerSetConditionMask;
    PFnVerifyVersionInfoA pfnVerifyVersionInfoA;

    dwVersion = GetVersion();

    // are we running NT ?
    if (!(dwVersion & 0x80000000))
    {
        // Is it Windows 2000 (NT 5.0) or greater ?
        if (LOBYTE(LOWORD(dwVersion)) > 4)
        {
            // In Windows 2000 we need to use the Product Suite APIs
            // Don't static link because it won't load on non-Win2000 systems
            hmodNtDll = GetModuleHandle( "NTDLL.DLL" );
            if (hmodNtDll != NULL)
            {
                pfnVerSetConditionMask = (PFnVerSetConditionMask )GetProcAddress( hmodNtDll, "VerSetConditionMask");
                if (pfnVerSetConditionMask != NULL)
                {
                    dwlConditionMask = (*pfnVerSetConditionMask)( dwlConditionMask, VER_SUITENAME, VER_AND );
                    hmodK32 = GetModuleHandle( "KERNEL32.DLL" );
                    if (hmodK32 != NULL)
                    {
                        pfnVerifyVersionInfoA = (PFnVerifyVersionInfoA)GetProcAddress( hmodK32, "VerifyVersionInfoA") ;
                        if (pfnVerifyVersionInfoA != NULL)
                        {
                            ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
                            osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
                            osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
                            bResult = (*pfnVerifyVersionInfoA)(
                                              &osVersionInfo,
                                              VER_SUITENAME,
                                              dwlConditionMask);
                        }
                    }
                }
            }
        }
        else
        {
            // This is NT 4.0 or older
            bResult = ValidateProductSuite( "Terminal Server" );
        }
    }

    return bResult;
}

/**
 * This function compares the passed in "suite name" string
 * to the product suite information stored in the registry.
 * This only works on the Terminal Server 4.0 platform.
 **/
EXTERN_C __declspec(dllexport) BOOL ValidateProductSuite (LPSTR SuiteName)
{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPSTR ProductSuite = NULL;
    LPSTR p;

    Rslt = RegOpenKeyA(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Control\\ProductOptions",
        &hKey
        );

    if (Rslt != ERROR_SUCCESS)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type, NULL, &Size );
    if (Rslt != ERROR_SUCCESS || !Size)
        goto exit;

    ProductSuite = (LPSTR) LocalAlloc( LPTR, Size );
    if (!ProductSuite)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type,
        (LPBYTE) ProductSuite, &Size );
     if (Rslt != ERROR_SUCCESS || Type != REG_MULTI_SZ)
        goto exit;

    p = ProductSuite;
    while (*p)
    {
        if (lstrcmpA( p, SuiteName ) == 0)
        {
            rVal = TRUE;
            break;
        }
        p += (lstrlenA( p ) + 1);
    }

exit:
    if (ProductSuite)
        LocalFree( ProductSuite );

    if (hKey)
        RegCloseKey( hKey );

    return rVal;
}

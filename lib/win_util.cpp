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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#define snprintf    _snprintf
#endif

#include "diagnostics.h"
#include "util.h"
#include "filesys.h"
#include "win_util.h"

/**
 * Find out if we are on a Windows 2000 compatible system
 **/
BOOL IsWindows2000Compatible() {
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) {
        return FALSE;
    }

    return (osvi.dwMajorVersion >= 5);
}

/**
 * Define these if they aren't defined.  They are normally found in
 * winnt.h, but some compilers don't have them.
 **/
#ifndef VER_AND
#define VER_AND 6
#endif

#ifndef VER_SUITENAME
#define VER_SUITENAME 0x0000040
#endif

#ifndef VER_SUITE_SINGLEUSERTS
#define VER_SUITE_SINGLEUSERTS              0x00000100
#endif

/**
 * This function performs the basic check to see if
 * the platform on which it is running is Terminal
 * services enabled.  Note, this code is compatible on
 * all Win32 platforms.  For the Windows 2000 platform
 * we perform a "lazy" bind to the new product suite
 * APIs that were first introduced on that platform.
 **/
BOOL IsTerminalServicesEnabled() {
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
            hmodNtDll = GetModuleHandleA( "NTDLL.DLL" );
            if (hmodNtDll != NULL)
            {
                pfnVerSetConditionMask = (PFnVerSetConditionMask )GetProcAddress( hmodNtDll, "VerSetConditionMask");
                if (pfnVerSetConditionMask != NULL)
                {
                    dwlConditionMask = (*pfnVerSetConditionMask)( dwlConditionMask, VER_SUITENAME, VER_AND );
                    hmodK32 = GetModuleHandleA( "KERNEL32.DLL" );
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
BOOL ValidateProductSuite (LPSTR SuiteName) {
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


/**
 * This function terminates a process by process id instead of a handle.
 **/
BOOL TerminateProcessById( DWORD dwProcessID ) {
    HANDLE hProcess;
    BOOL bRetVal = FALSE;

    hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, dwProcessID );

    if (hProcess) {
        bRetVal = TerminateProcess(hProcess, 1);
    }

    CloseHandle( hProcess );

    return bRetVal;
}


void chdir_to_data_dir() {
    LONG    lReturnValue;
    HKEY    hkSetupHive;
    LPSTR  lpszRegistryValue = NULL;
    char    szPath[MAX_PATH];
    DWORD   dwSize = 0;

    // change the current directory to the boinc data directory if it exists
    lReturnValue = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE, 
        "SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup",  
        0, 
        KEY_READ,
        &hkSetupHive
    );
    if (lReturnValue == ERROR_SUCCESS) {
        // How large does our buffer need to be?
        lReturnValue = RegQueryValueExA(
            hkSetupHive,
            "DATADIR",
            NULL,
            NULL,
            NULL,
            &dwSize
        );
        if (lReturnValue != ERROR_FILE_NOT_FOUND) {
            // Allocate the buffer space.
            lpszRegistryValue = (LPSTR) malloc(dwSize);
            (*lpszRegistryValue) = NULL;

            // Now get the data
            lReturnValue = RegQueryValueExA( 
                hkSetupHive,
                "DATADIR",
                NULL,
                NULL,
                (LPBYTE)lpszRegistryValue,
                &dwSize
            );

            SetCurrentDirectoryA(lpszRegistryValue);
        }
    } else {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath))) {
            strncat(szPath, "\\boinc", (sizeof(szPath) - strlen(szPath)));
            if (boinc_file_exists(szPath)) {
                SetCurrentDirectoryA(szPath);
            }
        }
    }

    if (hkSetupHive) RegCloseKey(hkSetupHive);
    if (lpszRegistryValue) free(lpszRegistryValue);
}


// return true if running under remote desktop
// (in which case CUDA and Stream apps don't work)
//
typedef BOOL (__stdcall *tWTSQSI)( IN HANDLE, IN DWORD, IN DWORD, OUT LPTSTR*, OUT DWORD* );
typedef VOID (__stdcall *tWTSFM)( IN PVOID );

bool is_remote_desktop() {
    static HMODULE wtsapi32lib = NULL;
    static tWTSQSI pWTSQSI = NULL;
    static tWTSFM pWTSFM = NULL;
    LPTSTR pBuf = NULL;
    DWORD dwLength;
    USHORT usProtocol=0, usConnectionState=0;

    if (!wtsapi32lib) {
        wtsapi32lib = LoadLibrary(_T("wtsapi32.dll"));
        if (wtsapi32lib) {
            pWTSQSI = (tWTSQSI)GetProcAddress(wtsapi32lib, "WTSQuerySessionInformationA");
            pWTSFM = (tWTSFM)GetProcAddress(wtsapi32lib, "WTSFreeMemory");
        }
    }

    if (pWTSQSI) {

        // WTSQuerySessionInformation(
        //   WTS_CURRENT_SERVER_HANDLE,
        //   WTS_CURRENT_SESSION,
        //   WTSClientProtocolType,
        //   &pBuf,
        //   &dwLength
        // );
        if (pWTSQSI(
            (HANDLE)NULL,
            (DWORD)-1,
            (DWORD)16,
            &pBuf,
            &dwLength
        )) {
            usProtocol = *(USHORT*)pBuf;
            pWTSFM(pBuf);
        }

        // WTSQuerySessionInformation(
        //   WTS_CURRENT_SERVER_HANDLE,
        //   WTS_CURRENT_SESSION,
        //   WTSConnectState,
        //   &pBuf,
        //   &dwLength
        // );
        if (pWTSQSI(
            (HANDLE)NULL,
            (DWORD)-1,
            (DWORD)8,
            &pBuf,
            &dwLength
        )) {
            usConnectionState = *(USHORT*)pBuf;
            pWTSFM(pBuf);
        }

        // RDP Session implies Remote Desktop
        if (usProtocol == 2) return true;

        // Fast User Switching keeps the protocol set to the console but changes
        // the connected state to disconnected.
        if ((usProtocol == 0) && (usConnectionState == 4)) return true;

    }

    return false;
}

std::wstring A2W(const std::string& str) {
  int length_wide = MultiByteToWideChar(CP_ACP, 0, str.data(), -1, NULL, 0);
  wchar_t *string_wide = static_cast<wchar_t*>(_alloca((length_wide * sizeof(wchar_t)) + sizeof(wchar_t)));
  MultiByteToWideChar(CP_ACP, 0, str.data(), -1, string_wide, length_wide);
  std::wstring result(string_wide, length_wide);
  return result;
}

std::string W2A(const std::wstring& str) {
  int length_ansi = WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, NULL, 0, NULL, NULL);
  char* string_ansi = static_cast<char*>(_alloca(length_ansi + sizeof(char)));
  WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, string_ansi, length_ansi, NULL, NULL);
  std::string result(string_ansi, length_ansi);
  return result;
}

// get message for given error
//
char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize
) {
    DWORD dwRet;
    LPWSTR lpszTemp = NULL;

    dwRet = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        dwError,
        LANG_NEUTRAL,
        (LPWSTR)&lpszTemp,
        0,
        NULL
    );

    if (dwRet != 0) {
        // convert from current character encoding into UTF8
        std::string encoded_message = W2A(std::wstring(lpszTemp));

        // include the hex error code as well
        snprintf(pszBuf, iSize, "%s (0x%x)", encoded_message.c_str(), dwError);

        if (lpszTemp) {
            LocalFree((HLOCAL) lpszTemp);
        }
    } else {
        strcpy(pszBuf, "(unknown error)");
    }

    return pszBuf;
}


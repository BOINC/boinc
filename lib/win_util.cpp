// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#if defined(_WIN32)
#include "boinc_win.h"
#endif

#include "diagnostics.h"
#include "util.h"
#include "filesys.h"
#include "win_util.h"
#include "str_replace.h"
#include "str_util.h"

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

// change the current directory to the BOINC data directory
// in the registry, if it exists
//
void chdir_to_data_dir() {
    LONG    lReturnValue;
    HKEY    hkSetupHive;
    char    szPath[MAX_PATH];
    LPSTR   lpszValue = NULL;
    LPSTR   lpszExpandedValue = NULL;
    DWORD   dwValueType = REG_EXPAND_SZ;
    DWORD   dwSize = 0;

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
            &dwValueType,
            NULL,
            &dwSize
        );
        if (lReturnValue != ERROR_FILE_NOT_FOUND) {
            // Allocate the buffer space.
            lpszValue = (LPSTR) malloc(dwSize);
            (*lpszValue) = NULL;

            // Now get the data
            lReturnValue = RegQueryValueExA(
                hkSetupHive,
                "DATADIR",
                NULL,
                &dwValueType,
                (LPBYTE)lpszValue,
                &dwSize
            );

            // Expand the Strings
            // We need to get the size of the buffer needed
            dwSize = 0;
            lReturnValue = ExpandEnvironmentStringsA(lpszValue, NULL, dwSize);

            if (lReturnValue) {
                // Make the buffer big enough for the expanded string
                lpszExpandedValue = (LPSTR) malloc(lReturnValue);
                (*lpszExpandedValue) = NULL;
                dwSize = lReturnValue;

                ExpandEnvironmentStringsA(lpszValue, lpszExpandedValue, dwSize);

                SetCurrentDirectoryA(lpszExpandedValue);
            }
        }
    } else {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath))) {
            safe_strcat(szPath, "\\boinc");
            if (boinc_file_exists(szPath)) {
                SetCurrentDirectoryA(szPath);
            }
        }
    }

    if (hkSetupHive) RegCloseKey(hkSetupHive);
    if (lpszValue) free(lpszValue);
    if (lpszExpandedValue) free(lpszExpandedValue);
}

std::wstring boinc_ascii_to_wide(const std::string& str) {
  int length_wide = MultiByteToWideChar(CP_ACP, 0, str.data(), -1, NULL, 0);
  wchar_t *string_wide = static_cast<wchar_t*>(_alloca((length_wide * sizeof(wchar_t)) + sizeof(wchar_t)));
  MultiByteToWideChar(CP_ACP, 0, str.data(), -1, string_wide, length_wide);
  std::wstring result(string_wide, length_wide - 1);
  return result;
}

std::string boinc_wide_to_ascii(const std::wstring& str) {
  int length_ansi = WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, NULL, 0, NULL, NULL);
  char* string_ansi = static_cast<char*>(_alloca(length_ansi + sizeof(char)));
  WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, string_ansi, length_ansi, NULL, NULL);
  std::string result(string_ansi, length_ansi - 1);
  return result;
}

// get message for given error
//
char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize ...
) {
    DWORD dwRet;
    LPSTR lpszTemp = NULL;

    va_list args;
    va_start(args, iSize);
    dwRet = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM ,
        NULL,
        dwError,
        LANG_NEUTRAL,
#ifdef wxUSE_GUI
        (LPWSTR)&lpszTemp,
#else
        (LPSTR)&lpszTemp,
#endif
        0,
        &args
    );
    va_end(args);

    if (dwRet != 0) {
        // include the hex error code as well
        snprintf(pszBuf, iSize, "%S (0x%x)", lpszTemp, dwError);
        if (lpszTemp) {
            LocalFree((HLOCAL)lpszTemp);
        }
    } else {
        snprintf(pszBuf, iSize, "(unknown error) (%lu)", GetLastError());
    }

    return pszBuf;
}

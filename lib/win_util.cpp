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

// Windows utilities

#include "boinc_win.h"

#include "diagnostics.h"
#include "util.h"
#include "filesys.h"
#include "str_replace.h"
#include "str_util.h"

#include "win_util.h"

using std::string;

// terminate a process by process ID instead of a handle.
//
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

// convert string to wide string
//
std::wstring boinc_ascii_to_wide(const string& str) {
    int length_wide = MultiByteToWideChar(CP_ACP, 0, str.data(), -1, NULL, 0);
    wchar_t *string_wide = static_cast<wchar_t*>(
        _alloca((length_wide * sizeof(wchar_t)) + sizeof(wchar_t))
    );
    MultiByteToWideChar(CP_ACP, 0, str.data(), -1, string_wide, length_wide);
    std::wstring result(string_wide, length_wide - 1);
    return result;
}

// convert wide string to string
//
std::string boinc_wide_to_ascii(const std::wstring& str) {
    int length_ansi = WideCharToMultiByte(
        CP_UTF8, 0, str.data(), -1, NULL, 0, NULL, NULL
    );
    char* string_ansi = static_cast<char*>(_alloca(length_ansi + sizeof(char)));
    WideCharToMultiByte(
        CP_UTF8, 0, str.data(), -1, string_ansi, length_ansi, NULL, NULL
    );
    string result(string_ansi, length_ansi - 1);
    return result;
}

// get message for given error
//
char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize ...
) {
    DWORD dwRet = 0;
    LPSTR lpszTemp = NULL;

    va_list args = NULL;
    va_start(args, iSize);
    try {
        dwRet = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
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
    }
    catch(...) {
        dwRet = 0;
    }
    va_end(args);

    if (dwRet != 0) {
        // include the hex error code as well
        snprintf(pszBuf, iSize, "%s (0x%x)", lpszTemp, dwError);
        if (lpszTemp) {
            LocalFree((HLOCAL)lpszTemp);
        }
    } else {
        snprintf(pszBuf, iSize, "(unknown error) (%lu)", GetLastError());
    }

    return pszBuf;
}

// WSL_CMD: run commands in a WSL distro

typedef HRESULT(WINAPI *PWslLaunch)(
    PCWSTR, PCWSTR, BOOL, HANDLE, HANDLE, HANDLE, HANDLE*
);

static PWslLaunch pWslLaunch = NULL;
static HINSTANCE wsl_lib = NULL;

int WSL_CMD::setup(string &err_msg) {
    in_read = NULL;
    in_write = NULL;
    out_read = NULL;
    out_write = NULL;

    if (!pWslLaunch) {
        wsl_lib = LoadLibraryA("wslapi.dll");
        if (!wsl_lib) {
            err_msg = "Can't load wslapi.dll";
            return -1;
        }
        pWslLaunch = (PWslLaunch)GetProcAddress(wsl_lib, "WslLaunch");
        if (!pWslLaunch) {
            err_msg = "WslLaunch not in wslapi.dll";
            return -1;
        }
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&out_read, &out_write, &sa, 0)) {
        err_msg = "Can't create out pipe";
        return -1;
    }
    if (!SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0)) {
        err_msg = "Can't inherit out pipe";
        return -1;
    }
    if (!CreatePipe(&in_read, &in_write, &sa, 0)) {
        err_msg = "Can't create in pipe";
        return -1;
    }
    if (!SetHandleInformation(in_write, HANDLE_FLAG_INHERIT, 0)) {
        err_msg = "Can't inherit in pipe";
        return -1;
    }
    return 0;
}

int WSL_CMD::setup_root(const char* distro_name) {
    char cmd[1024];
    sprintf(cmd, "wsl -d %s -u root", distro_name);
    int retval = run_program_pipe(cmd, in_write, out_read, proc_handle);
    if (retval) {
        fprintf(stderr, "WSL_CMD::setup_root() failed: %d\n", retval);
        return retval;
    }
    return 0;
}

int WSL_CMD::run_program_in_wsl(
    const string distro_name, const string command, bool use_cwd
) {
    HRESULT ret = pWslLaunch(
        boinc_ascii_to_wide(distro_name).c_str(),
        boinc_ascii_to_wide(command).c_str(),
        use_cwd, in_read, out_write, out_write,
        &proc_handle
    );
    if (ret != S_OK) {
        fprintf(stderr, "pWslLaunch failed: %d\n", ret);
    }
    return 0;
}

int read_from_pipe(
    HANDLE pipe, HANDLE proc_handle, string& out, double timeout,
    const char* eom
) {
    char buf[1024];
    DWORD avail, nread, exit_code;
    bool ret;
    double elapsed = 0;
    bool exited = false;
    out = "";
    while (1) {
        PeekNamedPipe(pipe, NULL, 0, NULL, &avail, NULL);
        if (avail) {
            ret = ReadFile(pipe, buf, sizeof(buf) - 1, &nread, NULL);
            if (!ret) return ERR_READ;
            buf[nread] = 0;
            out += buf;
            if (eom) {
                if (out.find(eom) != std::string::npos) {
                    return 0;
                }
            }
        } else {
            if (exited) {
                return ERR_CONNECT;
            }
            Sleep(200);
            if (timeout) {
                elapsed += .2;
                if (elapsed > timeout) {
                    return ERR_TIMEOUT;
                }
            }
            if (proc_handle) {
                ret = GetExitCodeProcess(proc_handle, &exit_code);
                if (!ret) exited = true;
                if (exit_code != STILL_ACTIVE) exited = true;
            }
        }
    }
}

int write_to_pipe(HANDLE pipe, const char* buf) {
    DWORD n = (DWORD) strlen(buf);
    DWORD nwritten;
    bool ret = WriteFile(pipe, buf, n, &nwritten, NULL);
        // what if nwritten != n?
        // The Win docs and examples do not clarify this
    if (ret) return 0;
    return -1;
}

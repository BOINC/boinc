// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "hostinfo.h"

HANDLE in_read = NULL;
HANDLE in_write = NULL;
HANDLE out_read = NULL;
HANDLE out_write = NULL;

bool CreateWslProcess(const std::string& command, HANDLE& handle) {
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));

    si.cb = sizeof(STARTUPINFO);
    si.hStdError = out_write;
    si.hStdOutput = out_write;
    si.hStdInput = NULL;
    si.dwFlags |= STARTF_USESTDHANDLES;

    const DWORD dwFlags = CREATE_NO_WINDOW;

    const std::string cmd = "bash -c \"" + command + "\"";

    const bool res = (CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, dwFlags, NULL, NULL, &si, &pi) == TRUE);

    if (res) {
        handle = pi.hProcess;
        CloseHandle(pi.hThread);
    }

    return res;
}

inline void close_handle(HANDLE handle) {
    if (handle) {
        CloseHandle(handle);
    }
}

int close_handles_and_exit(const int return_code) {
    close_handle(in_read);
    close_handle(in_write);
    close_handle(out_read);
    close_handle(out_write);

    return return_code;
}

std::string ReadFromPipe(HANDLE handle) {
    DWORD avail, read, exitcode;
    const int bufsize = 256;
    char buf[bufsize];
    std::string res = "";

    for (;;) {
        PeekNamedPipe(out_read, NULL, 0, NULL, &avail, NULL);

        if (avail) {
            if (!ReadFile(out_read, buf, bufsize - 1, &read, NULL) || read == 0) {
                break;
            }

            buf[read] = '\0';
            res += buf;
        }
        else {
            if (!GetExitCodeProcess(handle, &exitcode) || exitcode != STILL_ACTIVE) {
                break;
            }
            Sleep(200);
        }
    }

    close_handle(handle);

    return res;
}

// Returns the OS name and version for WSL when enabled
//
int get_wsl_information(
    bool& wsl_enabled, char* wsl_os_name, const int wsl_os_name_size, char* wsl_os_version, const int wsl_os_version_size
) {
    wsl_enabled = false;

    SECURITY_ATTRIBUTES sa;
    HANDLE handle;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&out_read, &out_write, &sa, 0)) {
        return 1;
    }
    if (!SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0)) {
        return close_handles_and_exit(1);
    }
    if (!CreatePipe(&in_read, &in_write, &sa, 0)) {
        return close_handles_and_exit(1);
    }
    if (!SetHandleInformation(in_write, HANDLE_FLAG_INHERIT, 0)) {
        return close_handles_and_exit(1);
    }

    // lsbrelease
    if (!CreateWslProcess(command_lsbrelease, handle)) {
        return close_handles_and_exit(1);
    }
    wsl_enabled = HOST_INFO::parse_linux_os_info(
        ReadFromPipe(handle), lsbrelease, wsl_os_name, wsl_os_name_size, wsl_os_version, wsl_os_version_size);
    if (wsl_enabled) {
        return close_handles_and_exit(0);
    }

    //osrelease
    const std::string command_osrelease = "cat " + std::string(file_osrelease);
    if (!CreateWslProcess(command_osrelease, handle)) {
        return close_handles_and_exit(1);
    }
    wsl_enabled = HOST_INFO::parse_linux_os_info(
        ReadFromPipe(handle), osrelease, wsl_os_name, wsl_os_name_size, wsl_os_version, wsl_os_version_size);
    if (wsl_enabled) {
        return close_handles_and_exit(0);
    }

    //redhatrelease
    const std::string command_redhatrelease = "cat " + std::string(file_redhatrelease);
    if (!CreateWslProcess(command_redhatrelease, handle)) {
        return close_handles_and_exit(1);
    }
    wsl_enabled = HOST_INFO::parse_linux_os_info(
        ReadFromPipe(handle), redhatrelease, wsl_os_name, wsl_os_name_size, wsl_os_version, wsl_os_version_size);
    if (wsl_enabled) {
        return close_handles_and_exit(0);
    }

    return close_handles_and_exit(0);
}

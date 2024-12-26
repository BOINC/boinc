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

#ifndef BOINC_WIN_UTIL_H
#define BOINC_WIN_UTIL_H

extern BOOL TerminateProcessById(DWORD dwProcessId);
extern void chdir_to_data_dir();

extern std::wstring boinc_ascii_to_wide(const std::string& str);
extern std::string boinc_wide_to_ascii(const std::wstring& str);

extern char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize ...
);

// struct for running a program in a WSL, connected via pipes.
// This can be a one-time command,
// or a shell to which you send a sequence of commands via the pipe.
// In the latter case:
//  - write to the input pipe to run commands from the shell
//  - the output of each command should end with 'EOM'
//      so that you know when you've read the complete output.
//
struct WSL_CMD {
    HANDLE in_read = NULL;
    HANDLE in_write = NULL;
    HANDLE out_read = NULL;
    HANDLE out_write = NULL;
    HANDLE proc_handle = NULL;

    ~WSL_CMD() {
        if (in_read) CloseHandle(in_read);
        if (in_write) CloseHandle(in_write);
        if (out_read) CloseHandle(out_read);
        if (out_write) CloseHandle(out_write);
    }

    // Use WslLaunch() to run a shell in the WSL container
    // The shell will run as the default user
    //
    int setup(std::string&);

    // Use wsl.exe to run a shell as root in the WSL container
    //
    int setup_root(const char* distro_name);

    // run command, direct both stdout and stderr to the out pipe
    // Use read_from_pipe() to get the output.
    //
    int run_program_in_wsl(
        const std::string distro_name, const std::string command,
        bool use_cwd = false
    );
};

// read from the pipe until either
// - we get the eom string (if any)
//      If you want to read at least 1 line, use "\n"
// - there's no more data and the given process (if any) doesn't exist
// - there's no more data and the given timeout (if any) is reached
// - a read fails
//
extern int read_from_pipe(
    HANDLE pipe,
    HANDLE proc_handle,
    std::string& out,
    double timeout = 0,
    const char* eom = NULL
);

extern int write_to_pipe(HANDLE pipe, const char* buf);

#endif

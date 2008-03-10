// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// Code used by BOINC components (screensaver, manager, updater)
// Don't include this in libboinc

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "win_util.h"
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#endif

#include "filesys.h"
#include "error_numbers.h"
#include "common_defs.h"
#include "util.h"

#ifdef _WIN32
HANDLE sandbox_account_token = NULL;
PSID sandbox_account_sid = NULL;
#endif

#ifdef _WIN32
void get_sandbox_account_token() {
    FILE* f;
    char buf[256];
    std::string encoded_username_str;
    std::string encoded_password_str;
    std::string username_str;
    std::string domainname_str; 
    std::string password_str; 
    int retval = 0;
    static bool first=true;

    if (!first) return;
    first = true;
    f = fopen(CLIENT_AUTH_FILENAME, "r");
    if (!f) return;
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<username>", encoded_username_str)) continue;
        if (parse_str(buf, "<password>", encoded_password_str)) continue;
    }
    fclose(f);

    password_str = r_base64_decode(encoded_password_str); 

    if (string::npos != encoded_username_str.find('\\')) {
        domainname_str = encoded_username_str.substr(
            0, encoded_username_str.find('\\')
        );
        username_str = encoded_username_str.substr(
            encoded_username_str.rfind(_T('\\')) + 1,
            encoded_username_str.length() - encoded_username_str.rfind(_T('\\')) - 1
        );
        retval = LogonUser( 
            (char*) username_str.c_str(),
            (char*) domainname_str.c_str(), 
            (char*) password_str.c_str(), 
            LOGON32_LOGON_SERVICE, 
            LOGON32_PROVIDER_DEFAULT, 
            &sandbox_account_token
        );
        if (retval) {
            GetAccountSid(domainname_str.c_str(), username_str.c_str(), &sandbox_account_sid);
        }
    } else {
        username_str = encoded_username_str;
        retval = LogonUser( 
            (char*) username_str.c_str(),
            NULL, 
            (char*) password_str.c_str(), 
            LOGON32_LOGON_SERVICE, 
            LOGON32_PROVIDER_DEFAULT, 
            &sandbox_account_token
        );
        if (retval) {
            GetAccountSid(NULL, username_str.c_str(), &sandbox_account_sid);
        }
    }

    if (!retval) {
        sandbox_account_token = NULL;
        sandbox_account_sid = NULL;
    } else {
        // Adjust the permissions on the current desktop and window station
        //   to allow the sandbox user account to create windows and such.
        //
        if (!AddAceToWindowStation(GetProcessWindowStation(), sandbox_account_sid)) {
            fprintf(stderr, "Failed to add ACE to current WindowStation\n");
        }
        if (!AddAceToDesktop(GetThreadDesktop(GetCurrentThreadId()), sandbox_account_sid)) {
            fprintf(stderr, "Failed to add ACE to current Desktop\n");
        }
    }
}
#endif

// chdir into the given directory, and run a program there.
// If nsecs is nonzero, make sure it's still running after that many seconds.
//
// argv is set up Unix-style, i.e. argv[0] is the program name
//

#ifdef _WIN32
int run_program(
    const char* dir, const char* file, int argc, char *const argv[], double nsecs, HANDLE& id
) {
    int retval;
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    LPVOID environment_block = NULL;
    char cmdline[1024];
    char error_msg[1024];
    unsigned long status;

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    strcpy(cmdline, "");
    for (int i=0; i<argc; i++) {
        strcat(cmdline, argv[i]);
        if (i<argc-1) {
            strcat(cmdline, " ");
        }
    }

    get_sandbox_account_token();
    if (sandbox_account_token != NULL) {

        // Retrieve the current window station and desktop names
        char szWindowStation[256];
        memset(szWindowStation, 0, sizeof(szWindowStation));
        char szDesktop[256];
        memset(szDesktop, 0, sizeof(szDesktop));
        char szDesktopName[512];
        memset(szDesktopName, 0, sizeof(szDesktopName));

        if (!GetUserObjectInformation(
                GetProcessWindowStation(),
                UOI_NAME,
                &szWindowStation,
                sizeof(szWindowStation),
                NULL)
            ) {
            windows_error_string(error_msg, sizeof(error_msg));
            fprintf(stderr, "GetUserObjectInformation failed: %s\n", error_msg);
        }
        if (!GetUserObjectInformation(
                GetThreadDesktop(GetCurrentThreadId()),
                UOI_NAME,
                &szDesktop,
                sizeof(szDesktop),
                NULL)
            ) {
            windows_error_string(error_msg, sizeof(error_msg));
            fprintf(stderr, "GetUserObjectInformation failed: %s\n", error_msg);
        }

        // Construct the destination desktop name
        strncat(szDesktopName, szWindowStation, sizeof(szDesktopName) - strlen(szDesktopName));
        strncat(szDesktopName, "\\", sizeof(szDesktopName) - strlen(szDesktopName));
        strncat(szDesktopName, szDesktop, sizeof(szDesktopName) - strlen(szDesktopName));

        // Tell CreateProcessAsUser which desktop to use explicitly.
        startup_info.lpDesktop = szDesktopName;
                 
        // Construct an environment block that contains environment variables that don't
        //   describe the current user.
        if (!CreateEnvironmentBlock(&environment_block, sandbox_account_token, FALSE)) {
            windows_error_string(error_msg, sizeof(error_msg));
            fprintf(stderr, "CreateEnvironmentBlock failed: %s\n", error_msg);
        }

        retval = CreateProcessAsUser( 
            sandbox_account_token, 
            file, 
            cmdline, 
            NULL, 
            NULL, 
            FALSE, 
            CREATE_NEW_PROCESS_GROUP|CREATE_UNICODE_ENVIRONMENT, 
            environment_block, 
            dir, 
            &startup_info, 
            &process_info 
        );

        if (!DestroyEnvironmentBlock(environment_block)) {
            windows_error_string(error_msg, sizeof(error_msg));
            fprintf(stderr, "DestroyEnvironmentBlock failed: %s\n", error_msg);
        }
    } else {
        retval = CreateProcess(
            file,
            cmdline,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            dir,
            &startup_info,
            &process_info
        );
    }
    if (!retval) {
        windows_error_string(error_msg, sizeof(error_msg));
        fprintf(stderr, "CreateProcess failed: '%s'\n", error_msg);
        return -1; // CreateProcess returns 1 if successful, false if it failed.
    }

    if (nsecs) {
        boinc_sleep(nsecs);
        if (GetExitCodeProcess(process_info.hProcess, &status)) {
            if (status != STILL_ACTIVE) {
                return -1;
            }
        }
    }

    id = process_info.hProcess;

    return 0;
}
#else
int run_program(
    const char* dir, const char* file, int , char *const argv[], double nsecs, int& id
) {
    int retval;
    int pid = fork();
    if (pid == 0) {
        if (dir) {
            retval = chdir(dir);
            if (retval) return retval;
        }
        execv(file, argv);
        perror("execv");
        exit(errno);
    }

    if (nsecs) {
        boinc_sleep(3);
        if (waitpid(pid, 0, WNOHANG) == pid) {
            return -1;
        }
    }
    id = pid;
    return 0;
}
#endif

#ifdef _WIN32
void kill_program(HANDLE pid) {
    TerminateProcess(pid, 0);
}
#else
void kill_program(int pid) {
    kill(pid, SIGKILL);
}
#endif

#ifdef _WIN32
int get_exit_status(HANDLE pid_handle) {
    unsigned long status=1;
    while (1) {
        if (GetExitCodeProcess(pid_handle, &status)) {
            if (status == STILL_ACTIVE) {
                boinc_sleep(1);
            }
        }
    }
    return (int) status;
}
bool process_exists(HANDLE h) {
    unsigned long status=1;
    if (GetExitCodeProcess(h, &status)) {
        if (status == STILL_ACTIVE) return true;
    }
    return false;
}

#else
int get_exit_status(int pid) {
    int status;
    waitpid(pid, &status, 0);
    return status;
}
bool process_exists(int pid) {
    int p = waitpid(pid, 0, WNOHANG);
    if (p == pid) return false;     // process has exited
    if (p == -1) return false;      // PID doesn't exist
    return true;
}
#endif

#ifdef _WIN32
static int get_client_mutex(const char*) {
    char buf[MAX_PATH] = "";
    
    // Global mutex on Win2k and later
    //
    if (IsWindows2000Compatible()) {
        strcpy(buf, "Global\\");
    }
    strcat( buf, RUN_MUTEX);

    HANDLE h = CreateMutex(NULL, true, buf);
    if ((h==0) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
        return ERR_ALREADY_RUNNING;
    }
#else
static int get_client_mutex(const char* dir) {
    char path[1024];
    static FILE_LOCK file_lock;

    sprintf(path, "%s/%s", dir, LOCK_FILE_NAME);
    if (file_lock.lock(path)) {
        return ERR_ALREADY_RUNNING;
    }
#endif
    return 0;
}

int wait_client_mutex(const char* dir, double timeout) {
    double start = dtime();
    while (1) {
        int retval = get_client_mutex(dir);
        if (!retval) return 0;
        boinc_sleep(1);
        if (dtime() - start > timeout) break;
    }
    return ERR_ALREADY_RUNNING;
}

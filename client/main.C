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

// command-line version of the BOINC core client

// This file contains no GUI-related code,
// and is not included in the source code for Mac or Win GUI clients

#ifdef WIN32
#include "boinc_win.h"
#include "win_service.h"
#include "win_util.h"

typedef BOOL (CALLBACK* IdleTrackerInit)();
typedef void (CALLBACK* IdleTrackerTerm)();

#else
#include "config.h"
#ifdef HAVE_SYS_SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <unistd.h>
#include <csignal>
//#include "synch.h"
#endif

#ifdef __APPLE__
#include <sys/stat.h>   // for umask()
#endif

#include "diagnostics.h"
#include "error_numbers.h"
#include "util.h"
#include "prefs.h"
#include "filesys.h"
#include "network.h"

#include "client_state.h"
#include "file_names.h"
#include "log_flags.h"
#include "client_msgs.h"
#include "main.h"




// Display a message to the user.
// Depending on the priority, the message may be more or less obtrusive
//
void show_message(PROJECT *p, char* msg, int priority) {
    const char* x;
    char message[1024];
    time_t now = time(0);
    char* time_string = time_to_string(now);
#if defined(WIN32) && defined(_CONSOLE)
    char event_message[2048];
#endif

    // Cycle the log files if we need to
    diagnostics_cycle_logs();

    strcpy(message, msg);
    while (strlen(message)&&message[strlen(message)-1] == '\n') {
        message[strlen(message)-1] = 0;
    }

    if (p) {
        x = p->get_project_name();
    } else {
        x = "---";
    }

    record_message(p, priority, now, message);

    switch (priority) {
    case MSG_ERROR:
    case MSG_ALERT_ERROR:
        fprintf(stderr, "%s [%s] %s\n", time_string, x, message);
        printf("%s [%s] %s\n", time_string, x, message);
        if (gstate.executing_as_daemon) {
#if defined(WIN32) && defined(_CONSOLE)
            _stprintf(event_message, TEXT("%s [%s] %s\n"), time_string,  x, message);
            // TODO: Refactor messages so that we do not overload the event log
            // RTW 08/24/2004 
            //LogEventErrorMessage(event_message);
#endif
        }
        break;
    case MSG_INFO:
    case MSG_ALERT_INFO:
        printf("%s [%s] %s\n", time_string,  x, message);
        if (gstate.executing_as_daemon) {
#if defined(WIN32) && defined(_CONSOLE)
            _stprintf(event_message, TEXT("%s [%s] %s\n"), time_string,  x, message);
            // TODO: Refactor messages so that we do not overload the event log
            // RTW 08/24/2004 
            //LogEventInfoMessage(event_message);
#endif
        }
        break;
    }
}

#ifdef WIN32
void quit_client() {
    gstate.requested_exit = true;
}

void suspend_client() {
    gstate.user_run_request = USER_RUN_REQUEST_NEVER;
}

void resume_client() {
    gstate.user_run_request = USER_RUN_REQUEST_AUTO;
}

BOOL WINAPI ConsoleControlHandler ( DWORD dwCtrlType ){
    BOOL bReturnStatus = FALSE;
    switch( dwCtrlType ){
    case CTRL_C_EVENT:
        if(gstate.activities_suspended) {
            resume_client();
        } else {
            suspend_client();
        }
        bReturnStatus =  TRUE;
        break;
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        quit_client();
        bReturnStatus =  TRUE;
        break;
    case CTRL_LOGOFF_EVENT:
        if (!gstate.executing_as_daemon) {
           quit_client();
        }
        bReturnStatus =  TRUE;
        break;
    }
    return bReturnStatus;
}
#else
static void signal_handler(int signum) {
    msg_printf(NULL, MSG_INFO, "Received signal %d", signum);
    switch(signum) {
    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
        gstate.requested_exit = true;
        break;
    case SIGTSTP:
        gstate.user_run_request = USER_RUN_REQUEST_NEVER;
        break;
    case SIGCONT:
        gstate.user_run_request = USER_RUN_REQUEST_AUTO;
        break;
    default:
        msg_printf(NULL, MSG_ERROR, "Signal not handled");
    }
}
#endif

static FILE_LOCK file_lock;

int check_unique_instance() {
#ifdef _WIN32
    // on Windows, we set a mutex so that the screensaver
    // can find out that the core client is running
    //
    HANDLE h = CreateMutex(NULL, true, RUN_MUTEX);
    if ((h==0) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
        return ERR_ALREADY_RUNNING;
    }
#else
    if (file_lock.lock(LOCK_FILE_NAME)) {
        return ERR_ALREADY_RUNNING;
    }
#endif
    return 0;
}

static void init_core_client(int argc, char** argv) {
    int retval;

    setbuf(stdout, 0);
    setbuf(stderr, 0);

#ifdef _WIN32

    TCHAR   szPath[MAX_PATH-1];

    // change the current directory to the boinc install directory
    GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR)));

    TCHAR *pszProg = strrchr(szPath, '\\');
    if (pszProg) {
        szPath[pszProg - szPath + 1] = 0;
        SetCurrentDirectory(szPath);
    }

#endif

#ifdef __APPLE__

// We have postponed implementing the umask change due to security concerns.
//    umask(0);   // Set file creation mask to make all files world-writable
                // Our umask will be inherited by all our child processes
#endif

    read_log_flags();
    gstate.parse_cmdline(argc, argv);

    // Initialize the BOINC Diagnostics Framework
    int dwDiagnosticsFlags =
        BOINC_DIAG_DUMPCALLSTACKENABLED | 
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_HEAPCHECKEVERYALLOC |
        BOINC_DIAG_TRACETOSTDOUT;

    if (gstate.redirect_io || gstate.executing_as_daemon) {
        dwDiagnosticsFlags |= 
            BOINC_DIAG_REDIRECTSTDERR | 
            BOINC_DIAG_REDIRECTSTDOUT;
    }

    diagnostics_init(
        dwDiagnosticsFlags,
        "stdoutdae",
        "stderrdae"
    );

    retval = check_unique_instance();
    if (retval) {
        msg_printf(NULL, MSG_INFO, "Another instance of BOINC is running");
        exit(1);
    }

// Unix/Linux console controls
#ifndef _WIN32
    // Handle quit signals gracefully
    boinc_set_signal_handler(SIGHUP, signal_handler);
    boinc_set_signal_handler(SIGINT, signal_handler);
    boinc_set_signal_handler(SIGQUIT, signal_handler);
    boinc_set_signal_handler(SIGTERM, signal_handler);
#ifdef SIGPWR
    boinc_set_signal_handler(SIGPWR, signal_handler);
#endif
    //boinc_set_signal_handler_force(SIGTSTP, signal_handler);
    //boinc_set_signal_handler_force(SIGCONT, signal_handler);
#endif

// Windows console controls
#ifdef _WIN32
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleControlHandler, TRUE)){
        fprintf(stderr, "Failed to register the console control handler\n");
        exit(1);
    } else {
        printf(
            "\nTo pause/resume tasks hit CTRL-C, to exit hit CTRL-BREAK\n"
        );
    }
#endif

}

int boinc_main_loop() {
    int retval;
    double dt;

    retval = gstate.init();
    if (retval) {
        fprintf(stderr, "gstate.init() failed: %d\n", retval);
        exit(retval);
    }

    // must parse env vars AFTER gstate.init();
    // otherwise env vars will get overwritten with state file info
    //
    gstate.parse_env_vars();

    while (1) {
        if (!gstate.do_something()) {
            dt = dtime();
            gstate.net_sleep(0.1);
            dt = dtime() - dt;
            log_messages.printf(CLIENT_MSG_LOG::DEBUG_TIME, "SLEPT %f SECONDS\n", dt);
            fflush(stdout);
        }

        if (gstate.time_to_exit()) {
            msg_printf(NULL, MSG_INFO, "Time to exit");
            break;
        }
        if (gstate.requested_exit) {
            msg_printf(NULL, MSG_INFO, "Exit requested by user");
            break;
        }
    }
    gstate.quit_activities();

    return 0;
}

int main(int argc, char** argv) {
    int retval = 0;

    init_core_client(argc, argv);

#ifdef _WIN32
    // Initialize WinSock
    if ( WinsockInitialize() != 0 ) {
        printf(
            "BOINC Core Client Error Message\n"
            "Failed to initialize the Windows Sockets interface\n"
            "Terminating Application...\n"
        );
        return ERR_IO;
    }

    g_hIdleDetectionDll = LoadLibrary("boinc.dll");
    if(!g_hIdleDetectionDll) {
        printf(
            "BOINC Core Client Error Message\n"
            "Failed to initialize the BOINC Idle Detection Interface\n"
            "BOINC will not be able to determine if the user is idle or not...\n"
        );
    }

    if(g_hIdleDetectionDll) {
        IdleTrackerInit fnIdleTrackerInit;
        fnIdleTrackerInit = (IdleTrackerInit)GetProcAddress(g_hIdleDetectionDll, _T("IdleTrackerInit"));
        if(!fnIdleTrackerInit) {
            FreeLibrary(g_hIdleDetectionDll);
            g_hIdleDetectionDll = NULL;
        } else {
            if(!fnIdleTrackerInit()) {
                FreeLibrary(g_hIdleDetectionDll);
                g_hIdleDetectionDll = NULL;
            }
        }
    }


    SERVICE_TABLE_ENTRY dispatchTable[] = {
        { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main },
        { NULL, NULL }
    };

    if ( (argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/')) ) {
        if ( _stricmp( "daemon", argv[1]+1 ) == 0 ) {

            // allow the system to know it is running as a Windows service
            // and adjust it's diagnostics schemes accordingly.
            gstate.executing_as_daemon = true;

            printf( "\nStartServiceCtrlDispatcher being called.\n" );
            printf( "This may take several seconds.  Please wait.\n" );

            if (!StartServiceCtrlDispatcher(dispatchTable)) {
                LogEventErrorMessage(TEXT("StartServiceCtrlDispatcher failed."));
            }
        } else {
            retval = boinc_main_loop();
        }
    } else {
        retval = boinc_main_loop();
    }
#else

#ifdef __APPLE__
        // Initialize Mac OS X idle time measurement / idle detection
        gEventHandle = NXOpenEventStatus();
#endif  // __APPLE__

    retval = boinc_main_loop();
#endif

#ifdef _WIN32
    if(g_hIdleDetectionDll) {
        IdleTrackerTerm fnIdleTrackerTerm;
        fnIdleTrackerTerm = (IdleTrackerTerm)GetProcAddress(g_hIdleDetectionDll, _T("IdleTrackerTerm"));
        if(fnIdleTrackerTerm) {
            fnIdleTrackerTerm();
        }
        if(!FreeLibrary(g_hIdleDetectionDll)) {
            printf(
                "BOINC Core Client Error Message\n"
                "Failed to cleanup the BOINC Idle Detection Interface\n"
            );
        }
        g_hIdleDetectionDll = NULL;
    }

    if ( WinsockCleanup() != 0 ) {
        printf(
            "BOINC Core Client Error Message\n"
            "Failed to cleanup the Windows Sockets interface\n"
        );
        return ERR_IO;
    }
#endif
    return retval;
}


const char *BOINC_RCSID_f02264aefe = "$Id$";

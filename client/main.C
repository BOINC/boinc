// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

// command-line version of the BOINC core client

// This file contains no GUI-related code,
// and is not included in the source code for Mac or Win GUI clients

#ifdef WIN32
#include "boinc_win.h"
#include "win_service.h"
#include "win_net.h"

typedef BOOL (CALLBACK* IdleTrackerInit)();
typedef void (CALLBACK* IdleTrackerTerm)();

#endif

#ifndef _WIN32
#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <csignal>
#include "synch.h"
#endif

#include "diagnostics.h"
#include "client_state.h"
#include "error_numbers.h"
#include "file_names.h"
#include "log_flags.h"
#include "prefs.h"
#include "util.h"
#include "client_msgs.h"
#include "main.h"

// dummies
void guiOnBenchmarksBegin(){}
void guiOnBenchmarksEnd(){}

// This gets called when the client fails to add a project
//
void project_add_failed(PROJECT* project) {
    if (project->scheduler_urls.size()) {
        printf(
            "BOINC failed to log in to %s.\n "
            "Please check your account key and try again.\n",
            project->master_url
        );
    } else {
        printf(
            "BOINC couldn't get main page for %s.\n"
            "Please check the URL and try again.\n",
            project->master_url
        );
    }
    gstate.detach_project(project);
    gstate.quit_activities();
    exit(1);
}

// Display a message to the user.
// Depending on the priority, the message may be more or less obtrusive
//
void show_message(PROJECT *p, char* msg, int priority) {
    char* x;
    char message[1024];
    time_t now = time(0);
    char* time_string = time_to_string(now);
#if defined(WIN32) && defined(_CONSOLE)
    char event_message[2048];
#endif

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
        fprintf(stderr, "%s [%s] %s\n", time_string, x, message);
        printf("%s [%s] %s\n", time_string, x, message);
        if (gstate.executing_as_windows_service) {
#if defined(WIN32) && defined(_CONSOLE)
            _stprintf(event_message, TEXT("%s [%s] %s\n"), time_string,  x, message);
            // TODO: Refactor messages so that we do not overload the event log
            // RTW 08/24/2004 
            //LogEventErrorMessage(event_message);
#endif
        }
        break;
    case MSG_WARNING:
        printf("%s [%s] %s\n", time_string,  x, message);
        if (gstate.executing_as_windows_service) {
#if defined(WIN32) && defined(_CONSOLE)
            _stprintf(event_message, TEXT("%s [%s] %s\n"), time_string,  x, message);
            // TODO: Refactor messages so that we do not overload the event log
            // RTW 08/24/2004 
            //LogEventWarningMessage(event_message);
#endif
        }
        break;
    case MSG_INFO:
        printf("%s [%s] %s\n", time_string,  x, message);
        if (gstate.executing_as_windows_service) {
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

// Prompt user for project URL and authenticator,
// and create an account file
//
int add_new_project() {
#ifdef WIN32
    return 0;
#else
    PROJECT project;

    printf("Enter the URL of the project: ");
    scanf("%s", project.master_url);
    printf(
        "You should have already registered with the project\n"
        "and received an account key by email.\n"
        "Paste the account key here: "
    );
    scanf("%s", project.authenticator);

    if (!strlen(project.master_url) || !strlen(project.authenticator)) {
        printf("URL and account key must be nonempty\n");
        return ERR_INVALID_URL;
    }

    project.tentative = true;
    return project.write_account_file();
#endif
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
        if (!gstate.executing_as_windows_service) {
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
        msg_printf(NULL, MSG_WARNING, "Signal not handled");
    }
}
#endif

int boinc_main_loop(int argc, char** argv) {
    int retval;
    double dt;

    setbuf(stdout, 0);

    boinc_init_diagnostics(
        BOINC_DIAG_DUMPCALLSTACKENABLED
        | BOINC_DIAG_HEAPCHECKENABLED
        | BOINC_DIAG_TRACETOSTDERR
#ifdef _WIN32
        //| BOINC_DIAG_REDIRECTSTDERR
        //| BOINC_DIAG_REDIRECTSTDOUT
#endif
    );

    retval = check_unique_instance();
    if (retval) {
        msg_printf(NULL, MSG_INFO, "Another instance of BOINC is running");
        exit(1);
    }

// Unix/Linux console controls
#ifndef WIN32
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
#ifdef WIN32
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleControlHandler, TRUE)){
        fprintf(stderr, "Failed to register the console control handler\n");
        exit(1);
    } else {
        printf(
            "\nTo pause/resume tasks hit CTRL-C, to exit hit CTRL-BREAK\n"
        );
    }
#endif

    read_log_flags();
    gstate.parse_cmdline(argc, argv);
    gstate.parse_env_vars();
    retval = gstate.init();
    if (retval) {
        fprintf(stderr, "gstate.init() failed: %d\n", retval);
        exit(retval);
    }


    while (1) {
        dt = dtime();
        if (!gstate.do_something(dt)) {
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


#if defined(WIN32) && defined(_CONSOLE)

//
// On Windows, we support running as a Windows Service which requires
//     a different startup method
//
int main(int argc, char** argv) {
    int retval = 0;

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
    if(!g_hIdleDetectionDll)
    {
        printf(
            "BOINC Core Client Error Message\n"
            "Failed to initialize the BOINC Idle Detection Interface\n"
            "BOINC will not be able to determine if the user is idle or not...\n"
        );
    }

    if(g_hIdleDetectionDll)
    {
        IdleTrackerInit fnIdleTrackerInit;
        fnIdleTrackerInit = (IdleTrackerInit)GetProcAddress(g_hIdleDetectionDll, _T("IdleTrackerInit"));
        if(!fnIdleTrackerInit)
        {
            FreeLibrary(g_hIdleDetectionDll);
            g_hIdleDetectionDll = NULL;
        }
        else 
        {
            if(!fnIdleTrackerInit())
            {
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
        if ( _stricmp( "win_service", argv[1]+1 ) == 0 ) {

            // allow the system to know it is running as a Windows service
            // and adjust it's diagnostics schemes accordingly.
            gstate.executing_as_windows_service = true;

            printf( "\nStartServiceCtrlDispatcher being called.\n" );
            printf( "This may take several seconds.  Please wait.\n" );

            if (!StartServiceCtrlDispatcher(dispatchTable)) {
                LogEventErrorMessage(TEXT("StartServiceCtrlDispatcher failed."));
            }
        } else {
            retval = boinc_main_loop(argc, argv);
        }
    } else {
        retval = boinc_main_loop(argc, argv);
    }

    if(g_hIdleDetectionDll)
    {
        IdleTrackerTerm fnIdleTrackerTerm;
        fnIdleTrackerTerm = (IdleTrackerTerm)GetProcAddress(g_hIdleDetectionDll, _T("IdleTrackerTerm"));
        if(fnIdleTrackerTerm)
        {
            fnIdleTrackerTerm();
        }
        if(!FreeLibrary(g_hIdleDetectionDll))
        {
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

    return retval;
}

#else

//
// For platforms other than windows just treat it as a console application
//
int main(int argc, char** argv) {
    return boinc_main_loop(argc, argv);
}

#endif



const char *BOINC_RCSID_f02264aefe = "$Id$";

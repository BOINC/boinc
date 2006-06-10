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
#define _CONSOLE 1
#include "boinc_win.h"
#include "win_service.h"

extern HINSTANCE g_hClientLibraryDll;
static HANDLE g_hWin9xMonitorSystemThread = NULL;
static DWORD g_Win9xMonitorSystemThreadID = NULL;
static BOOL g_bIsWin9x = FALSE;

typedef BOOL (*pfnIsWindows2000Compatible)();
typedef BOOL (CALLBACK* ClientLibraryStartup)();
typedef void (CALLBACK* ClientLibraryShutdown)();
#ifndef _T
#define _T(X) X
#endif

#else
#include "config.h"
#ifdef HAVE_SYS_SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <syslog.h>
#include <unistd.h>
#include <csignal>
#endif

#ifdef __APPLE__
#include <sys/stat.h>   // for umask()
#endif

#ifdef __EMX__
#define INCL_DOS
#include <os2.h>
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

extern int curl_init(void);
extern int curl_cleanup(void);

static bool boinc_cleanup_completed = false;
    // Used on Windows 95/98/ME to determine when it is safe to leave
    //   the WM_ENDSESSION message handler and allow Windows to finish
    //   cleaning up.

// Display a message to the user.
// Depending on the priority, the message may be more or less obtrusive
//
void show_message(PROJECT *p, char* msg, int priority) {
    const char* x;
    char message[1024];
    time_t now = time(0);
    char* time_string = time_to_string((double)now);
#if defined(WIN32) && defined(_CONSOLE)
    char event_message[2048];
#endif

    // Cycle the log files if we need to
    diagnostics_cycle_logs();

    strlcpy(message, msg, sizeof(message));
    while (strlen(message)&&message[strlen(message)-1] == '\n') {
        message[strlen(message)-1] = 0;
    }

    if (p) {
        x = p->get_project_name();
    } else {
        x = "---";
    }

    record_message(p, priority, (int)now, message);

    switch (priority) {
    case MSG_ERROR:
    case MSG_ALERT_ERROR:
        fprintf(stderr, "%s [%s] %s\n", time_string, x, message);
        printf("%s [%s] %s\n", time_string, x, message);
        if (gstate.executing_as_daemon) {
#if defined(WIN32) && defined(_CONSOLE)
            stprintf(event_message, TEXT("%s [%s] %s\n"), time_string,  x, message);
            ::OutputDebugString(event_message);
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
            stprintf(event_message, TEXT("%s [%s] %s\n"), time_string,  x, message);
            ::OutputDebugString(event_message);
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
    gstate.suspend_tasks(SUSPEND_REASON_USER_REQ);
    gstate.suspend_network(SUSPEND_REASON_USER_REQ);
}

void resume_client() {
    gstate.resume_tasks();
    gstate.resume_network();
}

// Trap logoff and shutdown events on Win9x so we can clean ourselves up.
LRESULT CALLBACK Win9xMonitorSystemWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_QUERYENDSESSION)
    {
        BOINCTRACE("***** Win9x Monitor System Shutdown/Logoff Event Detected *****\n");
        quit_client();
        while (!boinc_cleanup_completed) {
            Sleep(1000);    // Win95 is stupid, we really only need to wait until we have
                            //   successfully shutdown the active tasks and cleaned everything
                            //   up.  Luckly WM_QUERYENDSESSION is sent before Win9x checks for any
                            //   existing console and that gives us a chance to clean-up and
                            //   then clear the console window.  Win9x will not close down
                            //   a console window if anything is displayed on it.
        }
        Sleep(2000);        // For good measure.
        system("cls");
        return TRUE;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

DWORD WINAPI Win9xMonitorSystemThread( LPVOID lpParam ) {
    HWND hwndMain;
    WNDCLASS wc;
    MSG msg;

    wc.style         = CS_GLOBALCLASS;
    wc.lpfnWndProc   = (WNDPROC)Win9xMonitorSystemWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = NULL;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
	wc.lpszClassName = "BOINCWin9xMonitorSystem";

    if (!RegisterClass(&wc))
    {
        fprintf(stderr, "Failed to register the Win9xMonitorSystem window class.\n");
        return 1;
    }

    /* Create an invisible window */
    hwndMain = CreateWindow(
        wc.lpszClassName,
		"BOINC Monitor System",
        WS_OVERLAPPEDWINDOW & ~WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        NULL,
        NULL);

    if (!hwndMain)
    {
        fprintf(stderr, "Failed to create the Win9xMonitorSystem window.\n");
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

BOOL WINAPI ConsoleControlHandler ( DWORD dwCtrlType ){
    BOOL bReturnStatus = FALSE;
    BOINCTRACE("***** Console Event Detected *****\n");
    switch( dwCtrlType ){
    case CTRL_C_EVENT:
        BOINCTRACE("Event: CTRL-C Event\n");
        if(gstate.tasks_suspended) {
            resume_client();
        } else {
            suspend_client();
        }
        bReturnStatus =  TRUE;
        break;
    case CTRL_BREAK_EVENT:
        BOINCTRACE("Event: CTRL-BREAK Event\n");
        quit_client();
        bReturnStatus =  TRUE;
        break;
    case CTRL_CLOSE_EVENT:
        BOINCTRACE("Event: CTRL-CLOSE Event\n");
        quit_client();
        break;
    case CTRL_SHUTDOWN_EVENT:
        BOINCTRACE("Event: CTRL-SHUTDOWN Event\n");
        quit_client();
        break;
    case CTRL_LOGOFF_EVENT:
        BOINCTRACE("Event: CTRL-LOGOFF Event\n");
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
#ifdef __EMX__
        // close socket
        shutdown(gstate.gui_rpcs.lsock, 2);
#endif
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

    // on Windows, we set a mutex so that only one copy
    // of the core client can run at a time
    //
    BOOL bIsWin2k = FALSE;
    char buf[MAX_PATH] = "";

    if (g_hClientLibraryDll) {   
        pfnIsWindows2000Compatible fn;
        fn = (pfnIsWindows2000Compatible)GetProcAddress(g_hClientLibraryDll, _T("IsWindows2000Compatible"));
        if (fn) {
            bIsWin2k = fn();
        }
    }
    
    // Global mutex on Win2k and later
    //
    if (bIsWin2k) {
        strcpy(buf, "Global\\");
    }
    strcat( buf, RUN_MUTEX);

    HANDLE h = CreateMutex(NULL, true, buf);
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

#ifdef sandbox
    umask (2);  // Set file creation mask to be writable by both user and group
                // Our umask will be inherited by all our child processes
#endif

    read_config_file();
    gstate.parse_cmdline(argc, argv);

    // Initialize the BOINC Diagnostics Framework
    int dwDiagnosticsFlags =
        BOINC_DIAG_DUMPCALLSTACKENABLED |
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_HEAPCHECKEVERYALLOC |
        BOINC_DIAG_TRACETOSTDOUT;

    if (gstate.redirect_io || gstate.executing_as_daemon || gstate.detach_console) {
        dwDiagnosticsFlags |=
            BOINC_DIAG_REDIRECTSTDERR |
            BOINC_DIAG_REDIRECTSTDOUT;
    }

    diagnostics_init(
        dwDiagnosticsFlags,
        "stdoutdae",
        "stderrdae"
    );


	// Win32 - detach from console if requested
#ifdef _WIN32
	if (gstate.detach_console) {
		FreeConsole();
	}
#endif

// Unix: install signal handlers
#ifndef _WIN32
    // Handle quit signals gracefully
    boinc_set_signal_handler(SIGHUP, signal_handler);
    boinc_set_signal_handler(SIGINT, signal_handler);
    boinc_set_signal_handler(SIGQUIT, signal_handler);
    boinc_set_signal_handler(SIGTERM, signal_handler);
#ifdef SIGPWR
    boinc_set_signal_handler(SIGPWR, signal_handler);
#endif
#endif

// Windows: install console controls
#ifdef _WIN32
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleControlHandler, TRUE)){
        if (!gstate.executing_as_daemon) {
            fprintf(stderr, "Failed to register the console control handler\n");
        } else {
            LogEventErrorMessage(TEXT("Failed to register the console control handler\n"));
        }
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
#ifdef _WIN32
    char event_message[2048];
#endif


#ifdef _WIN32
    g_hClientLibraryDll = LoadLibrary("boinc.dll");
    if(!g_hClientLibraryDll) {
        stprintf(event_message, 
            TEXT("BOINC Core Client Error Message\n"
                 "Failed to initialize the BOINC Client Library.\n"
                 "Load failed: %s\n"), windows_error_string(event_message, sizeof(event_message))
        );
        if (!gstate.executing_as_daemon) {
            fprintf(stderr, event_message);
        } else {
            LogEventErrorMessage(event_message);
        }
    }
#endif


    retval = check_unique_instance();
    if (retval) {
        fprintf(stderr, 
            "Another instance of BOINC is running\n"
        );
#ifdef _WIN32
        if (!gstate.executing_as_daemon) {
            LogEventErrorMessage(
                TEXT("Another instance of BOINC is running")
            );
        }
#endif
        return ERR_EXEC;
    }


    // Initialize WinSock
#if defined(_WIN32) && defined(USE_WINSOCK)
    if ( WinsockInitialize() != 0 ) {
        if (!gstate.executing_as_daemon) {
            fprintf(stderr,
                TEXT("BOINC Core Client Error Message\n"
                     "Failed to initialize the Windows Sockets interface\n"
                     "Terminating Application...\n")
            );
        } else {
            LogEventErrorMessage(
                TEXT("BOINC Core Client Error Message\n"
                     "Failed to initialize the Windows Sockets interface\n"
                     "Terminating Application...\n")
            );
        }
        return ERR_IO;
    }
#endif


	curl_init();


#ifdef _WIN32

    if(g_hClientLibraryDll) {
        ClientLibraryStartup fnClientLibraryStartup;
        fnClientLibraryStartup = (ClientLibraryStartup)GetProcAddress(g_hClientLibraryDll, _T("ClientLibraryStartup"));
        if(fnClientLibraryStartup) {
            if(!fnClientLibraryStartup()) {
                stprintf(event_message, 
                    TEXT("BOINC Core Client Error Message\n"
                        "Failed to initialize the BOINC Idle Detection Interface.\n"
                        "BOINC will not be able to determine if the user is idle or not...\n"
                        "Load failed: %s\n"), windows_error_string(event_message, sizeof(event_message))
                );
                if (!gstate.executing_as_daemon) {
                    fprintf(stderr, event_message);
                } else {
                    LogEventErrorMessage(event_message);
                }
            }
        }
    }

#endif

    retval = gstate.init();
    if (retval) {
        fprintf(stderr, "gstate.init() failed: %d\n", retval);
#ifdef _WIN32
        if (gstate.executing_as_daemon) {
            stprintf(event_message, TEXT("gstate.init() failed: %d\n"), retval);
            LogEventErrorMessage(event_message);
        }
#endif
        return retval;
    }


#ifdef _WIN32
    if (gstate.executing_as_daemon) {
        LogEventInfoMessage(
            TEXT("BOINC service is initialization completed, now begining process execution...\n")
        );
    }
#endif


    // must parse env vars after gstate.init();
    // otherwise items will get overwritten with state file info
    //
    gstate.parse_env_vars();

    if (gstate.projects.size() == 0) {
        msg_printf(NULL, MSG_INFO,
            "This computer is not attached to any projects"
        );
        msg_printf(NULL, MSG_INFO,
            "Visit http://boinc.berkeley.edu for instructions"
        );
    }

    while (1) {
        if (!gstate.poll_slow_events()) {
            gstate.do_io_or_sleep(POLL_INTERVAL);
        }
        fflush(stdout);

        if (gstate.time_to_exit()) {
            msg_printf(NULL, MSG_INFO, "Time to exit");
            break;
        }
        if (gstate.requested_exit) {
            msg_printf(NULL, MSG_INFO, "Exit requested by user");
            break;
        }
#ifdef __EMX__
        // give timeslice also to other processes,
        // otherwise we will get 100% cpu
        DosSleep(0);
#endif
    }
    gstate.quit_activities();


#ifdef _WIN32
    if(g_hClientLibraryDll) {
        ClientLibraryShutdown fnClientLibraryShutdown;
        fnClientLibraryShutdown = (ClientLibraryShutdown)GetProcAddress(g_hClientLibraryDll, _T("ClientLibraryShutdown"));
        if(fnClientLibraryShutdown) {
            fnClientLibraryShutdown();
        }
        if(!FreeLibrary(g_hClientLibraryDll)) {
            stprintf(event_message, 
                TEXT("BOINC Core Client Error Message\n"
                    "Failed to cleanup the BOINC Idle Detection Interface\n"
                    "Unload failed: %s\n"), windows_error_string(event_message, sizeof(event_message))
            );
            if (!gstate.executing_as_daemon) {
                fprintf(stderr, event_message);
            } else {
                LogEventErrorMessage(event_message);
            }
        }
        g_hClientLibraryDll = NULL;
    }

#ifdef USE_WINSOCK
    if ( WinsockCleanup() != 0 ) {
        stprintf(event_message, 
            TEXT("BOINC Core Client Error Message\n"
                "Failed to cleanup the Windows Sockets interface\n"
                "Unload failed: %s\n"), windows_error_string(event_message, sizeof(event_message))
        );
        if (!gstate.executing_as_daemon) {
            fprintf(stderr, event_message);
        } else {
            LogEventErrorMessage(event_message);
        }
        return ERR_IO;
    }
#endif

    if (g_bIsWin9x && g_Win9xMonitorSystemThreadID)
	    PostThreadMessage(g_Win9xMonitorSystemThreadID, WM_QUIT, 0, 0);

#endif

	curl_cleanup();

    boinc_cleanup_completed = true;

    return 0;
}

int main(int argc, char** argv) {
    int retval = 0;

#ifdef _WIN32
    int i;
    int len;
    char *commandLine;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Allow the system to know it is running as a Windows service
    // and adjust it's diagnostics schemes accordingly.
    if ( (argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/')) ) {
        if ( stricmp( "daemon", argv[1]+1 ) == 0 ) {
            gstate.executing_as_daemon = true;
            LogEventInfoMessage(
                TEXT("BOINC service is initializing...\n")
            );
        }
    }

    // This bit of silliness is required to properly detach when run from within a command
    // prompt under Win32.  The root cause of the problem is that CMD.EXE does not return
    // control to the user until the spawned program exits, detaching from the console is
    // not enough.  So we need to do the following.  If the -detach flag is given, trap it
    // prior to the main setup in init_core_client.  Reinvoke the program, changing the
    // -detach into -detach_phase_two, and then exit.  At this point, cmd.exe thinks all is
    // well, and returns control to the user.  Meanwhile the second invocation will grok the
    // -detach_phase_two flag, and detach itself from the console, finally getting us to
    // where we want to be.
    for (i = 1; i < argc; i++) {
        // FIXME FIXME.  Duplicate instances of -detach may cause this to be
        // executed unnecessarily.  At worst, I think it leads to a few extra
        // processes being created and destroyed.
        if (strcmp(argv[i], "-detach") == 0 || strcmp(argv[i], "--detach") == 0) {
            argv[i] = "-detach_phase_two";

            // start with space for two '"'s
            len = 2;
            for (i = 0; i < argc; i++) {
                len += strlen(argv[i]) + 1;
            }
            if ((commandLine = (char *) malloc(len)) == NULL) {
                // Drop back ten and punt.  Can't do the detach thing, so we just carry on.
                // At least the program will start.
                break;
            }
            commandLine[0] = '"';
            // OK, we can safely use strcpy and strcat, since we know that we allocated enough
            strcpy(&commandLine[1], argv[0]);
            strcat(commandLine, "\"");
            for (i = 1; i < argc; i++) {
                strcat(commandLine, " ");
                strcat(commandLine, argv[i]);
            }

            memset(&si, 0, sizeof(si));
            si.cb = sizeof(si);

            // If process creation succeeds, we exit, if it fails punt and continue
            // as usual.  We won't detach properly, but the program will run.
            if (CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                exit(0);
            }
            break;
        }
    }
#elif defined linux
    int i;
    
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-daemon") == 0 || strcmp(argv[i], "--daemon") == 0) {
            syslog(LOG_DAEMON, "Starting Boinc-Daemon, listening on port %d.", GUI_RPC_PORT);
            // from <unistd.h>:
            // Detach from the controlling terminal and run in the background as system daemon.
            // Don't change working directory to root ("/"), but redirect
            // standard input, standard output and standard error to /dev/null.
            daemon(1, 0);
            break;
        }
    }
#endif

    boinc_cleanup_completed = false;

    init_core_client(argc, argv);

#ifdef _WIN32
    // Figure out if we're on Win9x
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    g_bIsWin9x = osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;
    if (g_bIsWin9x) {
        // Win9x doesn't send us the shutdown or close console
        //   event, so we are going to create a hidden window
        //   to trap a WM_QUERYENDSESSION event.
        g_hWin9xMonitorSystemThread = CreateThread(
            NULL,
            0,
            Win9xMonitorSystemThread,
            NULL,
            0,
            &g_Win9xMonitorSystemThreadID);

        if (g_hWin9xMonitorSystemThread) {
            CloseHandle(g_hWin9xMonitorSystemThread);
        } else {
            g_hWin9xMonitorSystemThread = NULL;
            g_Win9xMonitorSystemThreadID = NULL;
        }
    }


    SERVICE_TABLE_ENTRY dispatchTable[] = {
        { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main },
        { NULL, NULL }
    };

    if ( (argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/')) ) {
        if ( stricmp( "daemon", argv[1]+1 ) == 0 ) {

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

    return retval;
}


const char *BOINC_RCSID_f02264aefe = "$Id$";

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

// command-line version of the BOINC client

// This file contains no GUI-related code.

#include "cpp.h"

#ifdef WIN32
#include "boinc_win.h"
#include "sysmon_win.h"
#include "win_util.h"
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#else
#include "config.h"
#if HAVE_SYS_SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <sys/stat.h>
#include <syslog.h>
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#endif

#if (defined (__APPLE__) && defined(SANDBOX) && defined(_DEBUG))
#include "SetupSecurity.h"
#endif

#include "diagnostics.h"
#include "error_numbers.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"
#include "prefs.h"
#include "filesys.h"
#include "network.h"
#include "idlemon.h"

#include "cs_proxy.h"
#include "client_state.h"
#include "file_names.h"
#include "log_flags.h"
#include "client_msgs.h"
#include "http_curl.h"
#include "sandbox.h"

#include "main.h"

#ifdef ANDROID
#include "android_log.h"
#endif

// Log informational messages to system specific places
//
void log_message_startup(const char* msg) {
    char evt_msg[2048];
    snprintf(evt_msg, sizeof(evt_msg),
        "%s\n",
        msg
    );
    if (!gstate.executing_as_daemon) {
        fprintf(stdout, evt_msg);
    } else {
#ifdef _WIN32
        LogEventInfoMessage(evt_msg);
#elif defined(__EMX__)
#elif defined (__APPLE__)
#elif defined (ANDROID)
        LOGD(evt_msg);
#else
        syslog(LOG_DAEMON|LOG_INFO, evt_msg);
#endif
    }
}

// Log error messages to system specific places
//
void log_message_error(const char* msg) {
    char evt_msg[2048];
#ifdef _WIN32
    snprintf(evt_msg, sizeof(evt_msg),
        "%s\n"
        "GLE: %s\n",
        msg, windows_error_string(evt_msg, (sizeof(evt_msg)-((int)strlen(msg)+7)))
    );
#else
    snprintf(evt_msg, sizeof(evt_msg),
        "%s\n",
        msg
    );
#endif
    if (!gstate.executing_as_daemon) {
        fprintf(stderr, evt_msg);
    } else {
#ifdef _WIN32
        LogEventErrorMessage(evt_msg);
#elif defined(__EMX__)
#elif defined (__APPLE__)
#elif defined (ANDROID)
        LOGD(evt_msg);
#else
        syslog(LOG_DAEMON|LOG_ERR, evt_msg);
#endif
    }
}

void log_message_error(const char* msg, int error_code) {
    char evt_msg[2048];
    snprintf(evt_msg, sizeof(evt_msg),
        "%s\n"
        "Error Code: %d\n",
        msg, error_code
    );
    if (!gstate.executing_as_daemon) {
        fprintf(stderr, evt_msg);
    } else {
#ifdef _WIN32
        LogEventErrorMessage(evt_msg);
#elif defined(__EMX__)
#elif defined (__APPLE__)
#elif defined (ANDROID)
        LOGD(evt_msg);
#else
        syslog(LOG_DAEMON|LOG_ERR, evt_msg);
#endif
    }
}

#ifndef _WIN32
static void signal_handler(int signum) {
    msg_printf(NULL, MSG_INFO, "Received signal %d", signum);
    switch(signum) {
    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
#ifdef SIGPWR
    case SIGPWR:
#endif
        gstate.requested_exit = true;
#ifdef __EMX__
        // close socket
        shutdown(gstate.gui_rpcs.lsock, 2);
#endif
        break;
    default:
        msg_printf(NULL, MSG_INTERNAL_ERROR, "Signal not handled");
    }
}
#endif

static void init_core_client(int argc, char** argv) {
    setbuf(stdout, 0);
    setbuf(stderr, 0);

    config.defaults();
    gstate.parse_cmdline(argc, argv);
    gstate.now = dtime();

#ifdef _WIN32
    if (!config.allow_multiple_clients) {
        chdir_to_data_dir();
    }
#endif

#ifndef _WIN32
    if (g_use_sandbox)
        // Set file creation mask to be writable by both user and group and
        // world-executable but neither world-readable nor world-writable
        // Our umask will be inherited by all our child processes
        //
        umask (6);
#endif

    // Initialize the BOINC Diagnostics Framework
    int flags =
#ifdef _DEBUG
        BOINC_DIAG_MEMORYLEAKCHECKENABLED |
#endif
        BOINC_DIAG_DUMPCALLSTACKENABLED |
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_TRACETOSTDOUT;

    if (gstate.redirect_io || gstate.executing_as_daemon || gstate.detach_console) {
        flags |=
            BOINC_DIAG_REDIRECTSTDERR |
            BOINC_DIAG_REDIRECTSTDOUT;
    }

    diagnostics_init(flags, "stdoutdae", "stderrdae");

#ifdef _WIN32
    // Specify which allocation will cause a debugger to break.  Use a previous
    // memory leak detection report which looks like this:
    //   {650} normal block at 0x000000000070A6F0, 24 bytes long.
    //   Data: <  N     P p     > 80 1E 4E 00 00 00 00 00 50 AE 70 00 00 00 00 00
    //_CrtSetBreakAlloc(650);
    //_CrtSetBreakAlloc(651);
    //_CrtSetBreakAlloc(652);
    //_CrtSetBreakAlloc(653);
    //_CrtSetBreakAlloc(654);
#endif

    read_config_file(true);

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
}

static int initialize() {
    int retval;

    if (!config.allow_multiple_clients) {
        retval = wait_client_mutex(".", 10);
        if (retval) {
            log_message_error("Another instance of BOINC is running.");
            return ERR_EXEC;
        }
    }


    // Initialize WinSock
#if defined(_WIN32) && defined(USE_WINSOCK)
    if (WinsockInitialize() != 0) {
        log_message_error("Failed to initialize the Windows Sockets interface.");
        return ERR_IO;
    }
#endif

    curl_init();

#ifdef _WIN32
    if(!startup_idle_monitor()) {
        log_message_error(
            "Failed to initialize the BOINC idle monitor interface."
            "BOINC will not be able to determine if the user is idle or not...\n"
        );
    }
#endif

    return 0;
}

static int finalize() {
    static bool finalized = false;
    if (finalized) return 0;
    finalized = true;
    gstate.quit_activities();
    daily_xfer_history.write_file();

#ifdef _WIN32
    shutdown_idle_monitor();

#ifdef USE_WINSOCK
    if (WinsockCleanup()) {
        log_message_error("WinSockCleanup() failed");
        return ERR_IO;
    }
#endif

    cleanup_system_monitor();

#endif

    curl_cleanup();

#ifdef _DEBUG
    gstate.free_mem();
#endif

    diagnostics_finish();
    gstate.cleanup_completed = true;
    return 0;
}

int boinc_main_loop() {
    int retval;

    retval = initialize();
    if (retval) return retval;

    retval = gstate.init();
    if (retval) {
        log_message_error("gstate.init() failed", retval);
        return retval;
    }

    // must parse env vars after gstate.init();
    // otherwise items will get overwritten with state file info
    //
    gstate.parse_env_vars();

    // do this after parsing env vars
    //
    proxy_info_startup();

    if (gstate.projects.size() == 0) {
        msg_printf(NULL, MSG_INFO,
            "This computer is not attached to any projects"
        );
        msg_printf(NULL, MSG_INFO,
            "Visit http://boinc.berkeley.edu for instructions"
        );
    }

    log_message_startup("Initialization completed");

    while (1) {
        if (!gstate.poll_slow_events()) {
            gstate.do_io_or_sleep(POLL_INTERVAL);
        }
        fflush(stderr);
        fflush(stdout);

        if (gstate.time_to_exit()) {
            msg_printf(NULL, MSG_INFO, "Time to exit");
            break;
        }
        if (gstate.requested_exit) {
            if (config.abort_jobs_on_exit) {
                if (!gstate.in_abort_sequence) {
                    msg_printf(NULL, MSG_INFO,
                        "Exit requested; starting abort sequence"
                    );
                    gstate.start_abort_sequence();
                }
            } else {
                msg_printf(NULL, MSG_INFO, "Exit requested by user");
                break;
            }
        }
        if (gstate.in_abort_sequence) {
            if (gstate.abort_sequence_done()) {
                msg_printf(NULL, MSG_INFO, "Abort sequence done; exiting");
                break;
            }
        }
    }

    return finalize();
}

int main(int argc, char** argv) {
    int retval = 0;

#ifdef ANDROID
    char ccwd[1024];
    getcwd(ccwd, sizeof(ccwd));
    char msg[1024];
    snprintf(msg, sizeof(msg), "Hello Logcat! cwd at: %s", ccwd);
    LOGD(msg);
#endif

    for (int index = 1; index < argc; index++) {
        if (strcmp(argv[index], "-daemon") == 0 || strcmp(argv[index], "--daemon") == 0) {
            gstate.executing_as_daemon = true;
            log_message_startup("BOINC is initializing...");
#if !defined(_WIN32) && !defined(__EMX__) && !defined(__APPLE__)
            // from <unistd.h>:
            // Detach from the controlling terminal and run in the background as system daemon.
            // Don't change working directory to root ("/"), but redirect
            // standard input, standard output and standard error to /dev/null.
            retval = daemon(1, 0);
            break;
#endif
        }

#ifdef _WIN32
        // This bit of silliness is required to properly detach when run from within a command
        // prompt under Win32.  The root cause of the problem is that CMD.EXE does not return
        // control to the user until the spawned program exits, detaching from the console is
        // not enough.  So we need to do the following.  If the -detach flag is given, trap it
        // prior to the main setup in init_core_client.  Reinvoke the program, changing the
        // -detach into -detach_phase_two, and then exit.  At this point, cmd.exe thinks all is
        // well, and returns control to the user.  Meanwhile the second invocation will grok the
        // -detach_phase_two flag, and detach itself from the console, finally getting us to
        // where we want to be.

        // FIXME FIXME.  Duplicate instances of -detach may cause this to be
        // executed unnecessarily.  At worst, I think it leads to a few extra
        // processes being created and destroyed.
        if (strcmp(argv[index], "-detach") == 0 || strcmp(argv[index], "--detach") == 0) {
            int i, len;
            char *commandLine;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            argv[index] = "-detach_phase_two";

            // start with space for two '"'s
            len = 2;
            for (i = 0; i < argc; i++) {
                len += (int)strlen(argv[i]) + 1;
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
#endif
    }

    init_core_client(argc, argv);

#ifdef _WIN32

    retval = initialize_system_monitor(argc, argv);
    if (retval) return retval;

    if ( (argc > 1) && (strcmp(argv[1], "-daemon") == 0 || strcmp(argv[1], "--daemon") == 0) ) {
        retval = initialize_service_dispatcher(argc, argv);
    } else {
        retval = boinc_main_loop();
    }

#else

#ifdef SANDBOX
    // Make sure owners, groups and permissions are correct
    // for the current setting of g_use_sandbox
    //
#if defined(_DEBUG) && defined(__APPLE__)
    // GDB can't attach to applications which are running as a different user
    // or group, so fix up data with current user and group during debugging
    //
    if (check_security(g_use_sandbox, false)) {
        SetBOINCDataOwnersGroupsAndPermissions();
    }
#endif  // _DEBUG && __APPLE__
    int securityErr = check_security(g_use_sandbox, false);
    if (securityErr) {
        printf(
            "File ownership or permissions are set in a way that\n"
            "does not allow sandboxed execution of BOINC applications.\n"
            "To use BOINC anyway, use the -insecure command line option.\n"
            "To change ownership/permission, reinstall BOINC"
#ifdef __APPLE__
            " or run\n the shell script Mac_SA_Secure.sh"
#else
            " or run\n the shell script secure.sh"
#endif
            ". (Error code %d)\n", securityErr
        );
        return ERR_USER_PERMISSION;
    }
#endif  // SANDBOX

    retval = boinc_main_loop();

#endif
    return retval;
}


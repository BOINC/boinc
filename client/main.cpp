// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.

// command-line version of the BOINC client

// This file contains no GUI-related code.

#include "cpp.h"

#ifdef WIN32
#include "boinc_win.h"
#include "sysmon_win.h"
#include "win_util.h"
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

#ifdef ANDROID
#include "android/log.h"
#endif

#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include "hostinfo.h"
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

#include "client_state.h"
#include "file_names.h"
#include "log_flags.h"
#include "client_msgs.h"
#include "http_curl.h"
#include "sandbox.h"

#include "main.h"

// Log informational messages to system specific places
//
void log_message_startup(const char* msg) {
    char evt_msg[2048];
    char* time_string = time_to_string(dtime());

    snprintf(evt_msg, sizeof(evt_msg),
        "%s %s\n",
        time_string, msg
    );
    if (!gstate.executing_as_daemon) {
        fprintf(stdout, "%s", evt_msg);
#ifdef _WIN32
        // MSVCRT doesn't support line buffered streams
        fflush(stdout);
#endif
    } else {
#ifdef _WIN32
        LogEventInfoMessage(evt_msg);
#elif defined(__EMX__)
#elif defined (__APPLE__)
#elif defined (ANDROID)
        __android_log_print(ANDROID_LOG_INFO, "BOINC", evt_msg);
#else
        syslog(LOG_DAEMON|LOG_INFO, "%s", evt_msg);
#endif
    }
}

// Log error messages to system specific places
//
void log_message_error(const char* msg) {
    char evt_msg[2048];
    char* time_string = time_to_string(dtime());
#ifdef _WIN32
    char buf[1024];
    snprintf(evt_msg, sizeof(evt_msg),
        "%s %s\n"
        "GLE: %s\n",
        time_string, msg,
        windows_format_error_string(GetLastError(), buf, sizeof(buf))
    );
#else
    snprintf(evt_msg, sizeof(evt_msg),
        "%s %s\n",
        time_string, msg
    );
#endif
    if (!gstate.executing_as_daemon) {
        fprintf(stderr, "%s", evt_msg);
    } else {
#ifdef _WIN32
        LogEventErrorMessage(evt_msg);
#elif defined(__EMX__)
#elif defined (__APPLE__)
#elif defined (ANDROID)
        __android_log_print(ANDROID_LOG_ERROR, "BOINC", evt_msg);
#else
        syslog(LOG_DAEMON|LOG_ERR, "%s", evt_msg);
#endif
    }
}

void log_message_error(const char* msg, int error_code) {
    char evt_msg[2048];
    char* time_string = time_to_string(dtime());
    snprintf(evt_msg, sizeof(evt_msg),
        "%s %s\n"
        "Error Code: %d\n",
        time_string, msg, error_code
    );
    if (!gstate.executing_as_daemon) {
        fprintf(stderr, "%s", evt_msg);
    } else {
#ifdef _WIN32
        LogEventErrorMessage(evt_msg);
#elif defined(__EMX__)
#elif defined (__APPLE__)
#elif defined (ANDROID)
        __android_log_print(ANDROID_LOG_ERROR, "BOINC", evt_msg);
#else
        syslog(LOG_DAEMON|LOG_ERR, "%s", evt_msg);
#endif
    }
}

#ifndef _WIN32
static void signal_handler(int signum, siginfo_t*, void*) {
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

    cc_config.defaults();
    nvc_config.defaults();
    gstate.parse_cmdline(argc, argv);
    gstate.now = dtime();

#ifdef _WIN32
    if (!cc_config.allow_multiple_clients && !gstate.cmdline_dir) {
        chdir_to_data_dir();
    }
#endif

#ifndef _WIN32
    if (g_use_sandbox) {
        // Set file creation mask to be writable by both user and group and
        // world-executable but neither world-readable nor world-writable
        // Our umask will be inherited by all our child processes
        //
        umask (6);
    }
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

    // Win32 - detach from console if requested
#ifdef _WIN32
    if (gstate.detach_console) {
        FreeConsole();
    }
#endif

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

    // NOTE: this must be called BEFORE newer_version_startup_check()
    // Only branded builds of BOINC should have an nvc_config.xml file
    // in the BOINC Data directory. See comments in current_version.cpp.
    //
    if (read_nvc_config_file()) {
       // msg_printf(NULL, MSG_INFO, "nvc_config.xml not found - using defaults");
    }

    // Unix: install signal handlers
#ifndef _WIN32
    // Handle quit signals gracefully
    boinc_set_signal_handler(SIGHUP, (handler_t)signal_handler);
    boinc_set_signal_handler(SIGINT, (handler_t)signal_handler);
    boinc_set_signal_handler(SIGQUIT, (handler_t)signal_handler);
    boinc_set_signal_handler(SIGTERM, (handler_t)signal_handler);
#ifdef SIGPWR
    boinc_set_signal_handler(SIGPWR, (handler_t)signal_handler);
#endif
#endif
}

// detect GPUs and write a description of them (and error/warning messages)
// to coproc_info.xml.
//
// We do this in a separate process for two reasons:
// 1) GPU detection can crash even if we catch signals
// 2) Some dual-GPU laptops (e.g., Macbook Pro) don't power down
//  the more powerful GPU until all processes which used them exit.
//  If we detected such GPUs in the client, they'd never power down.
//
static void do_gpu_detection(int argc, char** argv) {
    vector<string> warnings;

    boinc_install_signal_handlers();
    gstate.parse_cmdline(argc, argv);
    gstate.now = dtime();

    int flags =
        BOINC_DIAG_DUMPCALLSTACKENABLED |
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_TRACETOSTDOUT |
        BOINC_DIAG_REDIRECTSTDERR |
        BOINC_DIAG_REDIRECTSTDOUT;

    diagnostics_init(flags, "stdoutgpudetect", "stderrgpudetect");

    read_config_file(true);

    coprocs.detect_gpus(warnings);
    coprocs.write_coproc_info_file(warnings);
    warnings.clear();
}

//////// functions to prevent two clients from running on the same host

#ifdef _WIN32
static int get_client_mutex(const char*) {
    char buf[MAX_PATH] = "";

    // Global mutex on Win2k and later
    //
    safe_strcpy(buf, "Global\\");
    safe_strcat(buf, RUN_MUTEX);

    HANDLE h = CreateMutexA(NULL, true, buf);
    if ((h==0) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
        return ERR_ALREADY_RUNNING;
    }
#else
static int get_client_mutex(const char* dir) {
    char path[MAXPATHLEN];
    static FILE_LOCK file_lock;

    snprintf(path, sizeof(path), "%s/%s", dir, LOCK_FILE_NAME);
    path[sizeof(path)-1] = 0;

    int retval = file_lock.lock(path);
    if (retval == ERR_FCNTL) {
        return ERR_ALREADY_RUNNING;
    } else if (retval) {
        return retval;
    }
#endif
    return 0;
}

int wait_client_mutex(const char* dir, double timeout) {
    double start = dtime();
    int retval = 0;
    while (1) {
        retval = get_client_mutex(dir);
        if (!retval) return 0;
        boinc_sleep(1);
        if (dtime() - start > timeout) break;
    }
    return retval;
}

static int initialize() {
    int retval;

    if (!cc_config.allow_multiple_clients) {
        retval = wait_client_mutex(".", 10);
        if (retval) {
            if (retval == ERR_ALREADY_RUNNING) {
                log_message_error("Another instance of BOINC is running.");
            } else if (retval == ERR_OPEN) {
                log_message_error("Failed to open lockfile. Check file/directory permissions.");
            } else {
                log_message_error("Failed to lock directory.", retval);
            }
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

#ifdef __APPLE__
    // If we run too soon during system boot we can cause a kernel panic.
    // Sleep if system has been up for less than 2 minutes
    //
    if (gstate.executing_as_daemon) {
        if (get_system_uptime() < 120) {
            boinc_sleep(30.);
        }
    }
#endif

    retval = gstate.init();
    if (retval) {
        log_message_error("gstate.init() failed", retval);
        return retval;
    }

    log_message_startup("Initialization completed");

    // client main loop; poll interval is 1 sec
    while (1) {
        if (!gstate.poll_slow_events()) {
            gstate.do_io_or_sleep(POLL_INTERVAL);
        }

        if (gstate.time_to_exit()) {
            msg_printf(NULL, MSG_INFO, "Time to exit");
            break;
        }
        if (gstate.requested_exit) {
            if (cc_config.abort_jobs_on_exit) {
                if (!gstate.in_abort_sequence) {
                    msg_printf(NULL, MSG_INFO,
                        "Exit requested; starting abort sequence"
                    );
                    gstate.start_abort_sequence();
                }
            } else {
                msg_printf(NULL, MSG_INFO, "Exiting");
                break;
            }
        }
        if (gstate.in_abort_sequence) {
            if (gstate.abort_sequence_done()) {
                msg_printf(NULL, MSG_INFO, "Abort sequence done; exiting");
                break;
            }
        }
        gstate.check_overdue();
    }

    return finalize();
}

int main(int argc, char** argv) {
    int retval = 0;

    coprocs.set_path_to_client(argv[0]);    // Used to launch a child process for --detect_gpus

    for (int index = 1; index < argc; index++) {
        if (strcmp(argv[index], "-daemon") == 0 || strcmp(argv[index], "--daemon") == 0) {
            gstate.executing_as_daemon = true;
            log_message_startup("BOINC is initializing...");
#if !defined(_WIN32) && !defined(__EMX__) && !defined(__APPLE__)
            // from <unistd.h>:
            // Detach from the controlling terminal and run in the background
            // as system daemon.
            // Don't change working directory to root ("/"), but redirect
            // standard input, standard output and standard error to /dev/null.
            //
            retval = daemon(1, 0);
            break;
#endif
        }

        if (!strcmp(argv[index], "--detect_gpus")) {
            do_gpu_detection(argc, argv);
            return 0;
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
        if (strcmp(argv[index], "-detach") == 0 || strcmp(argv[index], "--detach") == 0 ||
            strcmp(argv[index], "-detach_console") == 0 || strcmp(argv[index], "--detach_console") == 0
        ) {
            int i, len=1024;
            char commandLine[1024];
            char execpath[MAXPATHLEN];
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            if (get_real_executable_path(execpath, sizeof(execpath))) {
                strlcpy(execpath, argv[0], sizeof(execpath));
            }

            argv[index] = "-detach_phase_two";

            snprintf(commandLine, sizeof(commandLine), "\"%s\"", execpath);
            for (i = 1; i < argc; i++) {
                strlcat(commandLine, " ", len);
                strlcat(commandLine, argv[i], len);
            }

            memset(&si, 0, sizeof(si));
            si.cb = sizeof(si);

            // If process creation succeeds, we exit,
            // if it fails punt and continue as usual.
            // We won't detach properly, but the program will run.
            //
            if (CreateProcess(NULL, commandLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
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
    // NOTE: GDB and LLDB can't attach to applications which are running as
    // a different user or group.
    // Normally, the Mac Development (Debug) builds do not define SANDBOX, so
    // check_security() is never called. However, it is possible to use GDB
    // or LLDB on sandbox-specific code, as long as the code is run as the
    // current user (i.e., not as boinc_master or boinc_project), and the
    // current user is a member of both groups boinc_master and boinc_project.
    // However, this has not been thoroughly tested. Please see the comments
    // in SetupSecurity.cpp and check_security.cpp for more details.
    //
    int securityErr = check_security(g_use_sandbox, false, NULL, 0);
    if (securityErr) {
#if (defined(__APPLE__) && defined (_DEBUG))
        printf(
            "To debug with sandbox security enabled, the current user\n"
            "must be a member of both groups boinc_master and boinc_project."
        );
#else  // ! (defined(__APPLE__) && defined (_DEBUG))
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
#endif  // ! (defined(__APPLE__) && defined (_DEBUG))
        return ERR_USER_PERMISSION;
    }
#endif  // SANDBOX

    retval = boinc_main_loop();

#endif
    main_exited = true;
    return retval;
}

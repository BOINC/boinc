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

// Stuff related to stderr/stdout direction and exception handling;
// used by both core client and by apps

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#define snprintf    _snprintf
#define strdate     _strdate
#define strtime     _strtime
#endif

#ifdef __EMX__
#include <sys/stat.h>
#endif

#ifndef _WIN32
#include "config.h"
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <unistd.h>
#endif

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#ifdef __APPLE__
#include "mac_backtrace.h"
#endif

#ifdef __GLIBC__
#include <execinfo.h>
#endif


#include "app_ipc.h"
#include "error_numbers.h"
#include "filesys.h"
#include "util.h"
#include "parse.h"
#include "diagnostics.h"


#if defined(_WIN32) && defined(_MSC_VER)

static _CrtMemState start_snapshot; 
static _CrtMemState finish_snapshot; 
static _CrtMemState difference_snapshot;

#endif


static int         diagnostics_initialized = false;
static int         flags;
static char        stdout_log[256];
static char        stdout_archive[256];
static FILE*       stdout_file;
static char        stderr_log[256];
static char        stderr_archive[256];
static FILE*       stderr_file;
static char        boinc_dir[256];
static char        boinc_install_dir[256];
static int         boinc_proxy_enabled;
static char        boinc_proxy[256];
static char        symstore[256];
static int         aborted_via_gui;
static double      stderr_file_size = 0;
static double      max_stderr_file_size = 2048*1024;
static double      stdout_file_size = 0;
static double      max_stdout_file_size = 2048*1024;


#if defined(_WIN32) && defined(_DEBUG)

// Trap ASSERTs and TRACEs from the CRT and spew them to stderr.
//
int __cdecl boinc_message_reporting(int reportType, char *szMsg, int *retVal){
    int n;
    (*retVal) = 0;

    switch(reportType){

    case _CRT_WARN:
    case _CRT_ERROR:

        if (flags & BOINC_DIAG_TRACETOSTDERR) {
            n = fprintf(stderr, szMsg);
            if (n > 0) stderr_file_size += n;
        }

        if (flags & BOINC_DIAG_TRACETOSTDOUT) {
            n = fprintf(stdout, szMsg);
            if (n > 0) stdout_file_size += n;
        }

        break;
    case _CRT_ASSERT:

        n = fprintf(stderr, "ASSERT: %s\n", szMsg);
        if (n > 0) stderr_file_size += n;

        (*retVal) = 1;
        break;

    }

    return(TRUE);
}

#endif // _WIN32 && _DEBUG


// initialize the app diagnostic environment.
//
int boinc_init_diagnostics(int _flags) {
    return diagnostics_init(
        BOINC_DIAG_BOINCAPPLICATION | _flags,
        BOINC_DIAG_STDOUT, BOINC_DIAG_STDERR
    );
}


// initialize the graphic diagnostic environment.
//
int boinc_init_graphics_diagnostics(int _flags) {
    return diagnostics_init(
        BOINC_DIAG_BOINCAPPLICATION | _flags,
        BOINC_DIAG_GFX_STDOUT, BOINC_DIAG_GFX_STDERR
    );
}


// Used to cleanup the diagnostics environment.
//
int boinc_finish_diag() {
    return diagnostics_finish();
}

int boinc_install_signal_handlers() {
#ifdef _WIN32
    SetUnhandledExceptionFilter(boinc_catch_signal);
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	_set_invalid_parameter_handler(boinc_catch_signal_invalid_parameter);
#endif
#else  //_WIN32

    // register handlers for fatal internal signals
    // so that they get reported in stderr.txt
    // Do NOT catch SIGQUIT because core client uses that to kill app
    //
    boinc_set_signal_handler(SIGILL, boinc_catch_signal);
    boinc_set_signal_handler(SIGABRT, boinc_catch_signal);
    boinc_set_signal_handler(SIGBUS, boinc_catch_signal);
    boinc_set_signal_handler(SIGSEGV, boinc_catch_signal);
    boinc_set_signal_handler(SIGSYS, boinc_catch_signal);
    boinc_set_signal_handler(SIGPIPE, boinc_catch_signal);
#endif //_WIN32
    return 0;
}


// initialize the diagnostics environment.
//
int diagnostics_init(
    int _flags, const char* stdout_prefix, const char* stderr_prefix
) {
    // Check to see if we have already been called
    if (diagnostics_initialized) {
        return ERR_INVALID_PARAM;
    }
    diagnostics_initialized = true;

    // Setup initial values
    flags = _flags;
    snprintf(stdout_log, sizeof(stdout_log), "%s.txt", stdout_prefix);
    snprintf(stdout_archive, sizeof(stdout_archive), "%s.old", stdout_prefix);
    snprintf(stderr_log, sizeof(stderr_log), "%s.txt", stderr_prefix);
    snprintf(stderr_archive, sizeof(stderr_archive), "%s.old", stderr_prefix);
    strcpy(boinc_dir, "");
    strcpy(boinc_install_dir, "");
    boinc_proxy_enabled = 0;
    strcpy(boinc_proxy, "");
    strcpy(symstore, "");

    // Check for invalid parameter combinations
    if ((flags & BOINC_DIAG_REDIRECTSTDERR) && (flags & BOINC_DIAG_REDIRECTSTDERROVERWRITE)) {
        return ERR_INVALID_PARAM;
    }

    if ((flags & BOINC_DIAG_REDIRECTSTDOUT) && (flags & BOINC_DIAG_REDIRECTSTDOUTOVERWRITE)) {
        return ERR_INVALID_PARAM;
    }


    // Archive any old stderr.txt and stdout.txt files, if requested
    if (flags & BOINC_DIAG_ARCHIVESTDERR) {
        boinc_copy(stderr_log, stderr_archive);
    }

    if (flags & BOINC_DIAG_ARCHIVESTDOUT) {
        boinc_copy(stdout_log, stdout_archive);
    }


    // Redirect stderr and/or stdout, if requested
    //
    if (flags & BOINC_DIAG_REDIRECTSTDERR) {
        file_size(stderr_log, stderr_file_size);
        stderr_file = freopen(stderr_log, "a", stderr);
        if (!stderr_file) {
            return ERR_FOPEN;
        }
        setbuf(stderr_file, 0);
    }

    if (flags & BOINC_DIAG_REDIRECTSTDERROVERWRITE) {
        stderr_file = freopen(stderr_log, "w", stderr);
        if (!stderr_file) {
            return ERR_FOPEN;
        }
        setbuf(stderr_file, 0);
    }

    if (flags & BOINC_DIAG_REDIRECTSTDOUT) {
        file_size(stdout_log, stdout_file_size);
        stdout_file = freopen(stdout_log, "a", stdout);
        if (!stdout_file) {
            return ERR_FOPEN;
        }
    }

    if (flags & BOINC_DIAG_REDIRECTSTDOUTOVERWRITE) {
        stdout_file = freopen(stdout_log, "w", stdout);
        if (!stdout_file) {
            return ERR_FOPEN;
        }
    }


#if defined(_WIN32)

#if defined(_DEBUG)

    _CrtSetReportHook(boinc_message_reporting);

    if (flags & BOINC_DIAG_MEMORYLEAKCHECKENABLED) {
        SET_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF);
    }

    if (flags & BOINC_DIAG_HEAPCHECKENABLED) {
        if (flags & BOINC_DIAG_HEAPCHECKEVERYALLOC) {
            SET_CRT_DEBUG_FIELD(_CRTDBG_CHECK_ALWAYS_DF);
        } else {
            SET_CRT_DEBUG_FIELD(_CRTDBG_CHECK_EVERY_1024_DF);
        }
    }

    if (flags & BOINC_DIAG_BOINCAPPLICATION) {
        if (flags & BOINC_DIAG_MEMORYLEAKCHECKENABLED) {
            _CrtMemCheckpoint(&start_snapshot); 
        }
    }

#endif // defined(_DEBUG)

    // Initialize the thread list structure
    //   The data for this structure should be set by
    //   boinc_init or boinc_init_graphics.
    diagnostics_init_thread_list();

    diagnostics_init_unhandled_exception_monitor();

    diagnostics_init_message_monitor();

#endif // defined(_WIN32)


    // Install unhandled exception filters and signal traps.
    if (BOINC_SUCCESS != boinc_install_signal_handlers()) {
        return ERR_SIGNAL_OP;
    }


    // Store various pieces of inforation for future use.
    if (flags & BOINC_DIAG_BOINCAPPLICATION) {
        char    buf[256];
        char    proxy_address[256];
        int     proxy_port;
        MIOFILE mf;
        FILE*   p;
#ifdef _WIN32
        LONG    lReturnValue;
        HKEY    hkSetupHive;
        DWORD   dwSize = 0;
#endif

        strcpy(buf, "");
        strcpy(proxy_address, "");
        proxy_port = 0;

#ifndef _USING_FCGI_
        p = fopen(INIT_DATA_FILE, "r");
#else
        p = FCGI::fopen(INIT_DATA_FILE, "r");
#endif
 
		if (p) {
			mf.init_file(p);
			while(mf.fgets(buf, sizeof(buf))) {
				if (match_tag(buf, "</app_init_data>")) break;
				else if (parse_str(buf, "<boinc_dir>", boinc_dir, 256)) continue;
				else if (parse_str(buf, "<symstore>", symstore, 256)) continue;
				else if (match_tag(buf, "<use_http_proxy/>")) {
					boinc_proxy_enabled = true;
					continue;
				}
				else if (parse_str(buf, "<http_server_name>", proxy_address, 256)) continue;
				else if (parse_int(buf, "<http_server_port>", proxy_port)) continue;
			}
			fclose(p);
		}

        if (boinc_proxy_enabled) {
            int buffer_used = snprintf(boinc_proxy, sizeof(boinc_proxy), "%s:%d", proxy_address, proxy_port);
            if ((sizeof(boinc_proxy) == buffer_used) || (-1 == buffer_used)) { 
                boinc_proxy[sizeof(boinc_proxy)-1] = '\0';
            }
        }

#ifdef _WIN32
        // Lookup the location of where BOINC was installed to and store
        //   that for future use.
        lReturnValue = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, 
            _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup"),  
	        0, 
            KEY_READ,
            &hkSetupHive
        );
        if (lReturnValue == ERROR_SUCCESS) {
            // How large does our buffer need to be?
            dwSize = sizeof(boinc_install_dir);

            lReturnValue = RegQueryValueEx(
                hkSetupHive,
                _T("INSTALLDIR"),
                NULL,
                NULL,
                (LPBYTE)&boinc_install_dir,
                &dwSize
            );
        }

        if (hkSetupHive) RegCloseKey(hkSetupHive);
#endif
    }

    return BOINC_SUCCESS;
}


// Cleanup the diagnostic framework before dumping any memory leaks.
//
int diagnostics_finish() {

#ifdef _WIN32

    // Shutdown the message monitor thread and handles
    diagnostics_finish_message_monitor();

    // Shutdown the unhandled exception filter thread and handles
    diagnostics_finish_unhandled_exception_monitor();

    // Cleanup internal thread list structures and free up any
    //   used memory.
    diagnostics_finish_thread_list();

#ifdef _DEBUG

    // Only perform the memory leak dump if it is a boinc application
    //   and not the BOINC Manager, BOINC Core Client, or BOINC
    //   Screen saver since they'll check on close.
    if (flags & BOINC_DIAG_BOINCAPPLICATION) {
        if (flags & BOINC_DIAG_MEMORYLEAKCHECKENABLED) {
            _CrtMemCheckpoint(&finish_snapshot);
            if (_CrtMemDifference(&difference_snapshot, &start_snapshot, &finish_snapshot)) {
                fprintf(stdout, "\n\n");
                fprintf(stdout, "**********\n");
                fprintf(stdout, "**********\n");
                fprintf(stdout, "\n");
                fprintf(stdout, "Memory Leaks Detected!!!\n");
                fprintf(stdout, "\n");
                fprintf(stdout, "Memory Statistics:\n");
                _CrtMemDumpStatistics(&difference_snapshot);
                fprintf(stdout, "\n");
                _CrtMemDumpAllObjectsSince(&difference_snapshot);
                fprintf(stdout, "\n");
            }
        }
    }

#endif // defined(_DEBUG)
#endif // defined(_WIN32)

    // Set initalization flag to false.
    diagnostics_initialized = false;

    return BOINC_SUCCESS;
}

// has the diagnostics library been initialized?
//
int diagnostics_is_initialized(){
    return diagnostics_initialized;
}


// return true if the specified flag is set.
//
int diagnostics_is_flag_set(int _flags) {
    return flags & _flags;
}


// return the location of the BOINC directory.
//
char* diagnostics_get_boinc_dir() {
    return boinc_dir;
}


// return the location of the BOINC install directory.
//
char* diagnostics_get_boinc_install_dir() {
    return boinc_install_dir;
}


// return the location of the symbol store.
//
char* diagnostics_get_symstore() {
    return symstore;
}


// store the location of the symbol store.
//
int diagnostics_set_symstore(char* project_symstore) {
    if (!strlen(symstore)) {
        int buffer_used = snprintf(symstore, sizeof(symstore), "%s", project_symstore);
        if ((sizeof(symstore) == buffer_used) || (-1 == buffer_used)) { 
            symstore[sizeof(symstore)-1] = '\0';
        }
    }
    return 0;
}


// do we need to worry about a proxy server?
//
int diagnostics_is_proxy_enabled() {
    return boinc_proxy_enabled;
}


// proxy server address and port
//
char* diagnostics_get_proxy() {
    return boinc_proxy;
}


// Set the value of the flag
int diagnostics_set_aborted_via_gui() {
    aborted_via_gui = 1;
    return 0;
}


// Return the value of he flag
int diagnostics_is_aborted_via_gui() {
    return aborted_via_gui;
}

// Cycle the log files at regular events.
//
int diagnostics_cycle_logs() {
    // If the stderr.txt or stdout.txt files are too big, cycle them
    //
    if (flags & BOINC_DIAG_REDIRECTSTDERR) {
        if (stderr_file_size > max_stderr_file_size) {
            if (NULL == stderr_file) return ERR_FOPEN;
            fclose(stderr_file);
            boinc_copy(stderr_log, stderr_archive);
            stderr_file_size = 0;
            stderr_file = freopen(stderr_log, "w", stderr);
            if (NULL == stderr_file) return ERR_FOPEN;
        }
    }

    if (flags & BOINC_DIAG_REDIRECTSTDOUT) {
        if (stdout_file_size > max_stdout_file_size) {
            if (NULL == stdout_file) return ERR_FOPEN;
            fclose(stdout_file);
            stdout_file_size = 0;
            boinc_copy(stdout_log, stdout_archive);
            stdout_file = freopen(stdout_log, "w", stdout);
            if (NULL == stdout_file) return ERR_FOPEN;
        }
    }
    return BOINC_SUCCESS;
}


// Diagnostics for POSIX Compatible systems.
//

#if HAVE_SIGNAL_H

// Set a signal handler only if it is not currently ignored
//
extern "C" void boinc_set_signal_handler(int sig, void(*handler)(int)) {
#if HAVE_SIGACTION
    struct sigaction temp;
    sigaction(sig, NULL, &temp);
    if (temp.sa_handler != SIG_IGN) {
        temp.sa_handler = handler;
    //        sigemptyset(&temp.sa_mask);
        sigaction(sig, &temp, NULL);
    }
#else
    void (*temp)(int);
    temp = signal(sig, boinc_catch_signal);
    if (temp == SIG_IGN) {
        signal(sig, SIG_IGN);
    }
#endif /* HAVE_SIGACTION */
}


// Set a signal handler even if it is currently ignored
//
void boinc_set_signal_handler_force(int sig, void(*handler)(int)) {
#if HAVE_SIGACTION
    struct sigaction temp;
    sigaction(sig, NULL, &temp);
    temp.sa_handler = handler;
    //    sigemptyset(&temp.sa_mask);
    sigaction(sig, &temp, NULL);
#else
    void (*temp)(int);
    temp = signal(sig, boinc_catch_signal);
    signal(sig, SIG_IGN);
#endif /* HAVE_SIGACTION */
}

// exit code to use if signalled; can be changed
static int signal_exit_code = EXIT_SIGNAL;

void set_signal_exit_code(int x) {
    signal_exit_code = x;
}

void boinc_catch_signal(int signal) {
    switch(signal) {
    case SIGHUP: fprintf(stderr, "SIGHUP: terminal line hangup\n");
         return;
    case SIGINT: fprintf(stderr, "SIGINT: interrupt program\n"); break;
    case SIGILL: fprintf(stderr, "SIGILL: illegal instruction\n"); break;
    case SIGABRT: fprintf(stderr, "SIGABRT: abort called\n"); break;
#if SIGBUS != SIGSEGV
    // in case SIGBUS == SIGSEGV (e.g., Haiku)
    case SIGBUS: fprintf(stderr, "SIGBUS: bus error\n"); break;
#endif
    case SIGSEGV: fprintf(stderr, "SIGSEGV: segmentation violation\n"); break;
    case SIGSYS: fprintf(stderr, "SIGSYS: system call given invalid argument\n"); break;
    case SIGPIPE: fprintf(stderr, "SIGPIPE: write on a pipe with no reader\n");
        return;
    default: fprintf(stderr, "unknown signal %d\n", signal); break;
    }

#ifdef __GLIBC__
    void *array[64];
    size_t size;
    size = backtrace (array, 64);
//  Anything that calls malloc here (i.e *printf()) will probably fail
//  so we'll do it the hard way.
    write(fileno(stderr),"Stack trace (",strlen("Stack trace ("));
    char mbuf[10];
    char *p=mbuf+9;
    int i=size;
    *(p--)=0;
    while (i) {
      *(p--)=i%10+'0';
      i/=10;
    }
    write(fileno(stderr),p+1,strlen(p+1));
    write(fileno(stderr)," frames):",strlen(" frames):"));
    mbuf[0]=10;
    write(fileno(stderr),mbuf,1);
    backtrace_symbols_fd(array, size, fileno(stderr));
#endif

#ifdef __APPLE__
    PrintBacktrace();
#endif

    fprintf(stderr, "\nExiting...\n");
    _exit(signal_exit_code);
}

#endif

//
// Diagnostics Routines common to all Platforms
//

// Converts the BOINCTRACE macro into a single string and report it
//   to the CRT so it can be reported via the normal means.
//
void boinc_trace(const char *pszFormat, ...) {
    static char szBuffer[4096];
    static char szDate[64];
    static char szTime[64];
    int n;

    // Trace messages should only be reported if running as a standalone
    //   application or told too.
    if ((flags & BOINC_DIAG_TRACETOSTDERR) ||
        (flags & BOINC_DIAG_TRACETOSTDOUT)) {

        memset(szBuffer, 0, sizeof(szBuffer));
        memset(szDate, 0, sizeof(szDate));
        memset(szTime, 0, sizeof(szTime));

#ifdef _WIN32
        strdate(szDate);
        strtime(szTime);
#else
        time_t t;
        char *theCR;
    
        time(&t);
        strcpy(szTime, asctime(localtime(&t)));
        theCR = strrchr(szTime, '\n');
        if (theCR) *theCR = '\0';
        theCR = strrchr(szTime, '\r');
        if (theCR) *theCR = '\0';
#endif

        va_list ptr;
        va_start(ptr, pszFormat);

        vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, ptr);

        va_end(ptr);

#if defined(_WIN32) && defined(_DEBUG)
        n = _CrtDbgReport(_CRT_WARN, NULL, NULL, NULL, "[%s %s] TRACE [%d]: %s", szDate, szTime, GetCurrentThreadId(), szBuffer);
#else
        if (flags & BOINC_DIAG_TRACETOSTDERR) {
#ifdef _WIN32
            n = fprintf(stderr, "[%s %s] TRACE [%d]: %s\n", szDate, szTime, GetCurrentThreadId(), szBuffer);
#else
            n = fprintf(stderr, "[%s] TRACE: %s\n", szTime, szBuffer);
#endif
            if (n > 0) stderr_file_size += n;
        }

        if (flags & BOINC_DIAG_TRACETOSTDOUT) {
#ifdef _WIN32
            n = fprintf(stdout, "[%s %s] TRACE [%d]: %s\n", szDate, szTime, GetCurrentThreadId(), szBuffer);
#else
            n = fprintf(stdout, "[%s] TRACE: %s\n", szTime, szBuffer);
#endif
            if (n > 0) stdout_file_size += n;
        }
#endif
    }
}


// Converts the BOINCINFO macro into a single string and report it
//   to stderr so it can be reported via the normal means.
//
#ifndef BOINC_INFOMSGS
void boinc_info(const char* /*pszFormat*/, ... ){ return; }
#else
void boinc_info(const char* pszFormat, ...){
    static char szBuffer[4096];
    static char szDate[64];
    static char szTime[64];
    int n;

    memset(szBuffer, 0, sizeof(szBuffer));
    memset(szDate, 0, sizeof(szDate));
    memset(szTime, 0, sizeof(szTime));

    strdate(szDate);
    strtime(szTime);

    va_list ptr;
    va_start(ptr, pszFormat);

    vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, ptr);

    va_end(ptr);

#if defined(_WIN32) && defined(_DEBUG)
    _CrtDbgReport(_CRT_WARN, NULL, NULL, NULL, "[%s %s] BOINCMSG: %s\n", szDate, szTime, szBuffer);
#else
    if (flags & BOINC_DIAG_TRACETOSTDERR) {
        n = fprintf(stderr, "[%s %s] BOINCMSG: %s\n", szDate, szTime, szBuffer);
        if (n > 0) stderr_file_size += n;
    }

    if (flags & BOINC_DIAG_TRACETOSTDOUT) {
        n = fprintf(stdout, "[%s %s] BOINCMSG: %s\n", szDate, szTime, szBuffer);
        if (n > 0) stdout_file_size += n;
    }
#endif
}
#endif

void diagnostics_set_max_file_sizes(int stdout_size, int stderr_size) {
    if (stdout_size) max_stdout_file_size = stdout_size;
    if (stderr_size) max_stderr_file_size = stderr_size;
}


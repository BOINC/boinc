// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

// API_IGNORE_CLIENT will make the app ignore the core client
// this is useful for debugging just the application
//#define API_IGNORE_CLIENT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <io.h>
#include <sys/stat.h>
#include <afxwin.h>
#include <winuser.h>
#include <mmsystem.h>    // for timing

MMRESULT timer_id;
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <signal.h>

#include <fcntl.h>
#include <sys/types.h>

#ifdef BOINC_APP_GRAPHICS
#ifdef __APPLE_CC__
    #include <OpenGL/gl.h>
#endif

#ifdef _WIN32
#include <gl\gl.h>            // Header File For The OpenGL32 Library
#include <gl\glu.h>            // Header File For The GLu32 Library
#include <gl\glaux.h>        // Header File For The Glaux Library
HANDLE hQuitEvent;
extern HANDLE graphics_threadh;
extern BOOL    win_loop_done;
#endif

#ifdef HAVE_GL_LIB
#include <GL/gl.h>
#endif
#endif

#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "graphics_api.h"

#ifdef __APPLE_CC__
#include <CoreServices/CoreServices.h>
#include "mac_app_opengl.h"

MPQueueID drawQueue;
#endif

#include "boinc_api.h"

#ifdef _WIN32
LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *ExceptionInfo);
#else
extern void boinc_catch_signal(int signal);
#endif

static APP_INIT_DATA aid;
GRAPHICS_INFO gi;
static double timer_period = 1.0/50.0;    // 50 Hz timer
static double time_until_checkpoint;
static double time_until_fraction_done_update;
static double time_until_suspend_check;
static double fraction_done;
static double last_checkpoint_cpu_time;
static bool ready_to_checkpoint = false;
static bool check_susp_quit = false;
static bool write_frac_done = false;
static bool this_process_active;
static bool time_to_suspend = false,time_to_quit = false;
int ok_to_draw = 0;
bool using_opengl = false;

// read the INIT_DATA and FD_INIT files
//
int boinc_init() {
#ifdef _WIN32
    freopen(STDERR_FILE, "a", stderr);
#endif

#ifndef API_IGNORE_CLIENT
    FILE* f;
    int retval;
    f = fopen(INIT_DATA_FILE, "r");
    if (!f) {
        fprintf(stderr, "boinc_init(): can't open init data file\n");
        return ERR_FOPEN;
    }
    retval = parse_init_data_file(f, aid);
    if (retval) {
        fprintf(stderr, "boinc_init(): can't parse init data file\n");
        return retval;
    }
    fclose(f);

    f = fopen(GRAPHICS_DATA_FILE, "r");
    if (!f) {
        fprintf(stderr, "boinc_init(): can't open graphics data file\n");
        fprintf(stderr, "Using default graphics settings.\n");
        gi.refresh_period = 0.5;
        gi.xsize = 640;
        gi.ysize = 480;
    }
    if (f) {
        retval = parse_graphics_file(f, &gi);
        if (retval) {
            fprintf(stderr, "boinc_init(): can't parse graphics data file\n");
            return retval;
        }
        fclose(f);
    }

    f = fopen(FD_INIT_FILE, "r");
    if (f) {
        parse_fd_init_file(f);
        fclose(f);
    }
#else
    strcpy(aid.app_preferences, "");
    strcpy(aid.user_name, "John Smith");
    strcpy(aid.team_name, "The A-Team");
    aid.wu_cpu_time = 1000;
    aid.total_cobblestones = 1000;
    aid.recent_avg_cobblestones = 500;
    aid.checkpoint_period = DEFAULT_CHECKPOINT_PERIOD;
    aid.fraction_done_update_period = DEFAULT_FRACTION_DONE_UPDATE_PERIOD;

    gi.graphics_mode = MODE_WINDOW;
    gi.refresh_period = 0.1; // 1/10th of a second
    gi.xsize = 640;
    gi.ysize = 480;
#endif
    time_until_checkpoint = aid.checkpoint_period;
    time_until_fraction_done_update = aid.fraction_done_update_period;
    time_until_suspend_check = 1;  // check every 1 second for suspend request from core client
    this_process_active = true;
    
    boinc_install_signal_handlers();
    set_timer(timer_period);

    return 0;
}

// Install signal handlers to aid in debugging
// TODO: write Windows equivalent error handlers?
//
int boinc_install_signal_handlers() {
#ifdef HAVE_SIGNAL_H
    signal( SIGHUP, boinc_catch_signal );  // terminal line hangup
    signal( SIGINT, boinc_catch_signal );  // interrupt program
    signal( SIGQUIT, boinc_catch_signal ); // quit program
    signal( SIGILL, boinc_catch_signal );  // illegal instruction
    signal( SIGABRT, boinc_catch_signal ); // abort(2) call
    signal( SIGBUS, boinc_catch_signal );  // bus error
    signal( SIGSEGV, boinc_catch_signal ); // segmentation violation
    signal( SIGSYS, boinc_catch_signal );  // system call given invalid argument
    signal( SIGPIPE, boinc_catch_signal ); // write on a pipe with no reader
#endif
#ifdef _WIN32
    SetUnhandledExceptionFilter( boinc_catch_signal );
#endif
    return 0;
}

#ifdef _WIN32
LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *ExceptionInfo) {
    PVOID exceptionAddr = ExceptionInfo->ExceptionRecord->ExceptionAddress;
    DWORD exceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
    char  status[256];
    
    switch (exceptionCode) {
        case STATUS_WAIT_0: strcpy(status,"Wait 0"); break;
        case STATUS_ABANDONED_WAIT_0: strcpy(status,"Abandoned Wait 0"); break;
        case STATUS_USER_APC: strcpy(status,"User APC"); break;
        case STATUS_TIMEOUT: strcpy(status,"Timeout"); break;
        case STATUS_PENDING: strcpy(status,"Pending"); break;
        case STATUS_SEGMENT_NOTIFICATION: return DBG_EXCEPTION_NOT_HANDLED;
        case STATUS_GUARD_PAGE_VIOLATION: strcpy(status,"Guard Page Violation"); break;
        case STATUS_DATATYPE_MISALIGNMENT: strcpy(status,"Data Type Misalignment"); break;
        case STATUS_BREAKPOINT: return DBG_EXCEPTION_NOT_HANDLED;
        case STATUS_SINGLE_STEP: return DBG_EXCEPTION_NOT_HANDLED;
        case STATUS_ACCESS_VIOLATION: strcpy(status,"Access Violation"); break;
        case STATUS_IN_PAGE_ERROR: strcpy(status,"In Page Error"); break;
        case STATUS_NO_MEMORY: strcpy(status,"No Memory"); break;
        case STATUS_ILLEGAL_INSTRUCTION: strcpy(status,"Illegal Instruction"); break;
        case STATUS_NONCONTINUABLE_EXCEPTION: strcpy(status,"Noncontinuable Exception"); break;
        case STATUS_INVALID_DISPOSITION: strcpy(status,"Invalid Disposition"); break;
        case STATUS_ARRAY_BOUNDS_EXCEEDED: strcpy(status,"Array Bounds Exceeded"); break;
        case STATUS_FLOAT_DENORMAL_OPERAND: strcpy(status,"Float Denormal Operand"); break;
        case STATUS_FLOAT_DIVIDE_BY_ZERO: strcpy(status,"Divide by Zero"); break;
        case STATUS_FLOAT_INEXACT_RESULT: strcpy(status,"Float Inexact Result"); break;
        case STATUS_FLOAT_INVALID_OPERATION: strcpy(status,"Float Invalid Operation"); break;
        case STATUS_FLOAT_OVERFLOW: strcpy(status,"Float Overflow"); break;
        case STATUS_FLOAT_STACK_CHECK: strcpy(status,"Float Stack Check"); break;
        case STATUS_FLOAT_UNDERFLOW: strcpy(status,"Float Uderflow"); break;
        case STATUS_INTEGER_DIVIDE_BY_ZERO: strcpy(status,"Integer Divide by Zero"); break;
        case STATUS_INTEGER_OVERFLOW: strcpy(status,"Integer Overflow"); break;
        case STATUS_PRIVILEGED_INSTRUCTION: strcpy(status,"Privileged Instruction"); break;
        case STATUS_STACK_OVERFLOW: strcpy(status,"Stack Overflow"); break;
        case STATUS_CONTROL_C_EXIT: strcpy(status,"Ctrl+C Exit"); break;
        default: strcpy(status,"Unknown exception"); break;
    }
    // TODO: also output info in CONTEXT structure?
    fprintf( stderr, "\n***UNHANDLED EXCEPTION****\n");
    fprintf( stderr, "Reason: %s at address 0x%p\n",status,exceptionAddr);
    fprintf( stderr, "Exiting...\n" );
    fflush(stderr);
    exit(ERR_SIGNAL_CATCH);
    return(EXCEPTION_EXECUTE_HANDLER);
}
#endif

#ifdef HAVE_SIGNAL_H
void boinc_catch_signal(int signal) {
    switch(signal) {
        case SIGHUP: fprintf( stderr, "SIGHUP: terminal line hangup" ); break;
        case SIGINT: fprintf( stderr, "SIGINT: interrupt program" ); break;
        case SIGQUIT: fprintf( stderr, "SIGQUIT: quit program" ); break;
        case SIGILL: fprintf( stderr, "SIGILL: illegal instruction" ); break;
        case SIGABRT: fprintf( stderr, "SIGABRT: abort called" ); break;
        case SIGBUS: fprintf( stderr, "SIGBUS: bus error" ); break;
        case SIGSEGV: fprintf( stderr, "SIGSEGV: segmentation violation" ); break;
        case SIGSYS: fprintf( stderr, "SIGSYS: system call given invalid argument" ); break;
        case SIGPIPE: fprintf( stderr, "SIGPIPE: write on a pipe with no reader" ); break;
        default: fprintf( stderr, "unknown signal %d", signal ); break;
    }
    fprintf( stderr, "\nExiting...\n" );
    exit(ERR_SIGNAL_CATCH);
}
#endif

int boinc_finish(int status) {
    last_checkpoint_cpu_time = boinc_cpu_time();
    write_fraction_done_file(fraction_done,last_checkpoint_cpu_time,last_checkpoint_cpu_time);
#ifdef _WIN32
    // Stop the timer
    timeKillEvent(timer_id);
#ifdef BOINC_APP_GRAPHICS
    if (using_opengl) {
        // If the graphics thread is running, tell it to quit and wait for it
        win_loop_done = TRUE;
        if (hQuitEvent != NULL) {
            WaitForSingleObject(hQuitEvent, 1000);  // Wait up to 1000 ms
        }
    }
#endif
#endif
    exit(status);
    return 0;
}

int boinc_get_init_data(APP_INIT_DATA& app_init_data) {
    app_init_data = aid;
    return 0;
}

// resolve XML soft links
//
int boinc_resolve_filename(char *virtual_name, char *physical_name, int len) {
    FILE *fp;
    char buf[512];

    strcpy(physical_name, virtual_name);

    // Open the file and load the first line
    fp = fopen(virtual_name, "r");
    if (!fp) return 0;

    fgets(buf, 512, fp);
    fclose(fp);

    // If it's the <soft_link> XML tag, return its value,
    // otherwise, return the original file name
    //
    parse_str(buf, "<soft_link>", physical_name, len);
    return 0;
}

bool boinc_time_to_checkpoint() {
    if (check_susp_quit) {
        FILE* f = fopen(SUSPEND_QUIT_FILE, "r");
        if(f) {
            parse_suspend_quit_file(f,time_to_suspend,time_to_quit);
            fclose(f);
        }
        time_until_suspend_check = 1;    // reset to 1 second
        check_susp_quit = false;
    }

    if (write_frac_done) {
        write_fraction_done_file(fraction_done, boinc_cpu_time(), last_checkpoint_cpu_time);
        time_until_fraction_done_update = aid.fraction_done_update_period;
        write_frac_done = false;
    }

    // If the application has received a quit or suspend request
    // it should checkpoint
    if (time_to_quit || time_to_suspend) {
        return true;
    }

    return ready_to_checkpoint;
}

int boinc_checkpoint_completed() {
    last_checkpoint_cpu_time = boinc_cpu_time();
    write_fraction_done_file(fraction_done,last_checkpoint_cpu_time,last_checkpoint_cpu_time);
    ready_to_checkpoint = false;
    time_until_checkpoint = aid.checkpoint_period;
    // If it's time to quit, call boinc_finish which will
    // exit the app properly
    if (time_to_quit) {
        boinc_finish(ERR_QUIT_REQUEST);
    }
    // If we're in a suspended state, sleep until instructed otherwise
    while (time_to_suspend) {
        if (time_to_quit) {
            boinc_finish(ERR_QUIT_REQUEST);
        }
        boinc_sleep(1);  // Should this be a smaller value?
        FILE* f = fopen(SUSPEND_QUIT_FILE, "r");
        if(f) {
            parse_suspend_quit_file(f,time_to_suspend,time_to_quit);
            fclose(f);
        } else {
            time_to_suspend = time_to_quit = false;
        }
    }
    return 0;
}

int boinc_fraction_done(double x) {
    fraction_done = x;
    return 0;
}

int boinc_child_start() {
    this_process_active = false;
    return 0;
}

int boinc_child_done(double cpu) {
    this_process_active = true;
    return 0;
}

double boinc_cpu_time() {
    double cpu_secs;

    // Start with the CPU time from previous runs, then
    // add the CPU time of the current run
    cpu_secs = aid.wu_cpu_time;

#ifdef HAVE_SYS_RESOURCE_H
    int retval, pid = getpid();
    struct rusage ru;
    retval = getrusage(RUSAGE_SELF, &ru);
    if(retval) fprintf(stderr, "error: could not get cpu time for %d\n", pid);
    // Sum the user and system time spent in this process
    cpu_secs += (double)ru.ru_utime.tv_sec + (((double)ru.ru_utime.tv_usec) / ((double)1000000.0));
    cpu_secs += (double)ru.ru_stime.tv_sec + (((double)ru.ru_stime.tv_usec) / ((double)1000000.0));
    return cpu_secs;
#else
#ifdef _WIN32
    HANDLE hProcess;
    FILETIME creationTime,exitTime,kernelTime,userTime;

    // TODO: Could we speed this up by retaining the process handle?
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, GetCurrentProcessId());
    if (GetProcessTimes(
        hProcess, &creationTime, &exitTime, &kernelTime, &userTime)
    ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        CloseHandle(hProcess);

        tKernel.LowPart  = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart    = userTime.dwLowDateTime;
        tUser.HighPart   = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu_secs += totTime / 10000000.0;

        // Convert to seconds and return
        return cpu_secs;
    }
    CloseHandle(hProcess);

    // TODO: Handle timer wraparound
    static bool first=true;
    static DWORD first_count = 0;

    if (first) {
        first_count = GetTickCount();
        first = false;
    }
    DWORD cur = GetTickCount();
    return cpu_secs + ((cur - first_count)/1000.);
#endif  // _WIN32
#endif

    fprintf(stderr, "boinc_cpu_time(): not implemented\n");
}

// This function should be as fast as possible, and shouldn't make any system calls
//
#ifdef _WIN32
void CALLBACK on_timer(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {
#else
void on_timer(int a) {
#endif

    if (!ready_to_checkpoint) {
        time_until_checkpoint -= timer_period;
        if (time_until_checkpoint <= 0) {
            ready_to_checkpoint = true;
        }
    }

    if (!check_susp_quit) {
        time_until_suspend_check -= timer_period;
        if (time_until_suspend_check <= 0) {
            check_susp_quit = true;
        }
    }

    if (!write_frac_done && this_process_active) {
        time_until_fraction_done_update -= timer_period;
        if (time_until_fraction_done_update <= 0) {
            write_frac_done = true;
        }
    }

#if defined __APPLE_CC__ && defined BOINC_APP_GRAPHICS
    // Yield to the graphics thread to let it draw if needed
    YieldToAnyThread();
#endif
}


int set_timer(double period) {
    int retval=0;
#ifdef _WIN32
    // Use Windows multimedia timer, since it is more accurate
    // than SetTimer and doesn't require an associated event loop
    timer_id = timeSetEvent(
        (int)(period*1000), // uDelay
        (int)(period*1000), // uResolution
        on_timer, // lpTimeProc
        NULL, // dwUser
        TIME_PERIODIC  // fuEvent
        );
#ifdef BOINC_APP_GRAPHICS
    // Create the event object used to signal between the
    // worker and event threads

    hQuitEvent = CreateEvent( 
            NULL,     // no security attributes
            TRUE,    // manual reset event
            TRUE,     // initial state is signaled
            NULL);    // object not named
#endif
#endif

#if defined BOINC_APP_GRAPHICS && defined __APPLE_CC__
    // Create notification queue for drawing
    MPCreateQueue( &drawQueue );
#endif

#if HAVE_SIGNAL_H
#if HAVE_SYS_TIME_H
    struct sigaction sa;
    itimerval value;
    sa.sa_handler = on_timer;
    sa.sa_flags = SA_RESTART;
    retval = sigaction(SIGALRM, &sa, NULL);
    if (retval) {
        perror( "boinc set_timer() sigaction" );
        return retval;
    }
    value.it_value.tv_sec = (int)period;
    value.it_value.tv_usec = ((int)(period*1000000))%1000000;
    value.it_interval = value.it_value;
    retval = setitimer(ITIMER_REAL, &value, NULL);
    if (retval) {
        perror( "boinc set_timer() setitimer" );
    }
#endif
#endif
    return retval;
}

int write_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    if (strlen(ai.app_preferences)) {
        fprintf(f, "<app_preferences>\n%s</app_preferences>\n", ai.app_preferences);
    }
    if (strlen(ai.team_name)) {
        fprintf(f, "<team_name>%s</team_name>\n", ai.team_name);
    }
    if (strlen(ai.user_name)) {
        fprintf(f, "<user_name>%s</user_name>\n", ai.user_name);
    }
    fprintf(f,
        "<wu_cpu_time>%f</wu_cpu_time>\n"
        "<total_cobblestones>%f</total_cobblestones>\n"
        "<recent_avg_cobblestones>%f</recent_avg_cobblestones>\n"
        "<checkpoint_period>%f</checkpoint_period>\n"
        "<fraction_done_update_period>%f</fraction_done_update_period>\n",
        ai.wu_cpu_time,
        ai.total_cobblestones,
        ai.recent_avg_cobblestones,
        ai.checkpoint_period,
        ai.fraction_done_update_period
    );
    return 0;
}

int parse_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    char buf[256];
    memset(&ai, 0, sizeof(ai));
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<app_preferences>")) {
            strcpy(ai.app_preferences, "");
            while (fgets(buf, 256, f)) {
                if (match_tag(buf, "</app_preferences>")) break;
                strcat(ai.app_preferences, buf);
            }
            continue;
        }
        else if (parse_str(buf, "<user_name>", ai.user_name, sizeof(ai.user_name))) continue;
        else if (parse_str(buf, "<team_name>", ai.team_name, sizeof(ai.team_name))) continue;
        else if (parse_double(buf, "<total_cobblestones>", ai.total_cobblestones)) continue;
        else if (parse_double(buf, "<recent_avg_cobblestones>", ai.recent_avg_cobblestones)) continue;
        else if (parse_double(buf, "<wu_cpu_time>", ai.wu_cpu_time)) continue;
        else if (parse_double(buf, "<checkpoint_period>", ai.checkpoint_period)) continue;
        else if (parse_double(buf, "<fraction_done_update_period>", ai.fraction_done_update_period)) continue;
        else fprintf(stderr, "parse_init_data_file: unrecognized %s", buf);
    }
    return 0;
}

int write_fraction_done_file(double pct, double cpu, double checkpoint_cpu) {
    FILE* f = fopen(FRACTION_DONE_TEMP_FILE, "w");

    if (!f)
        return -1;

    fprintf(f,
        "<fraction_done>%f</fraction_done>\n"
        "<cpu_time>%f</cpu_time>\n"
        "<checkpoint_cpu_time>%f</checkpoint_cpu_time>\n",
        pct,
        cpu,
        checkpoint_cpu
    );

    fclose(f);
#ifdef _WIN32
    unlink(FRACTION_DONE_FILE);
#endif
    rename(FRACTION_DONE_TEMP_FILE, FRACTION_DONE_FILE);

    return 0;
}

int parse_fraction_done_file(FILE* f, double& pct, double& cpu, double& checkpoint_cpu) {
    char buf[256];
    while (fgets(buf, 256, f)) {
        if (parse_double(buf, "<fraction_done>", pct)) continue;
        else if (parse_double(buf, "<cpu_time>", cpu)) continue;
        else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu)) continue;
        else fprintf(stderr, "parse_fraction_done_file: unrecognized %s", buf);
    }
    return 0;
}

int write_suspend_quit_file(FILE* f, bool suspend, bool quit) {
    if (suspend) {
        fprintf(f, "<suspend/>\n");
    }
    if (quit) {
        fprintf(f, "<quit/>\n");
    }
    return 0;
}

int parse_suspend_quit_file(FILE* f, bool& suspend, bool& quit) {
    char buf[256];
    
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<suspend/>")) suspend = true;
        else if (match_tag(buf, "<quit/>")) quit = true;
        else fprintf(stderr, "parse_suspend_quit_file: unrecognized %s", buf);
    }
    return 0;
}

// TODO: this should handle arbitrarily many fd/filename pairs.
// Also, give the tags better names
int write_fd_init_file(FILE* f, char *file_name, int fdesc, int input_file ) {
    if (input_file) {
        fprintf(f, "<fdesc_dup_infile>%s</fdesc_dup_infile>\n", file_name);
        fprintf(f, "<fdesc_dup_innum>%d</fdesc_dup_innum>\n", fdesc);
    } else {
        fprintf(f, "<fdesc_dup_outfile>%s</fdesc_dup_outfile>\n", file_name);
        fprintf(f, "<fdesc_dup_outnum>%d</fdesc_dup_outnum>\n", fdesc);
    }
    return 0;
}

// TODO: this should handle arbitrarily many fd/filename pairs.
// Also, this shouldn't be doing the actual duping!
//
int parse_fd_init_file(FILE* f) {
    char buf[256],filename[256];
    int filedesc;
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<fdesc_dup_infile>", filename, sizeof(filename))) {
            if (fgets(buf, 256, f)) {
                if (parse_int(buf, "<fdesc_dup_innum>", filedesc)) {
                    freopen(filename, "r", stdin);
                    fprintf(stderr, "opened input file %s\n", filename);
                }
            }
        } else if (parse_str(buf, "<fdesc_dup_outfile>", filename, sizeof(filename))) {
            if (fgets(buf, 256, f)) {
                if (parse_int(buf, "<fdesc_dup_outnum>", filedesc)) {
                    freopen(filename, "w", stdout);
                    fprintf(stderr, "opened output file %s\n", filename);
                }
            }
        } else fprintf(stderr, "parse_fd_init_file: unrecognized %s", buf);
    }
    return 0;
}

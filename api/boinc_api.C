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

// Code that's in the BOINC app library (but NOT in the core client)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
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
#ifdef _WIN32
HANDLE hQuitEvent;
extern HANDLE graphics_threadh;
extern BOOL    win_loop_done;
#endif
#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#endif
#endif

#include "parse.h"
#include "shmem.h"
#include "util.h"
#include "error_numbers.h"
#include "app_ipc.h"

#include "boinc_api.h"

#ifdef _WIN32
HANDLE hQuitRequest, hSharedMem;
LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *ExceptionInfo);
#else
extern void boinc_catch_signal(int signal);
extern void boinc_quit(int sig);
#endif

static APP_INIT_DATA aid;
GRAPHICS_INFO gi;
static double timer_period = 1.0/50.0;    // 50 Hz timer
static double time_until_checkpoint;
static double time_until_fraction_done_update;
static double fraction_done;
static double last_checkpoint_cpu_time;
static bool ready_to_checkpoint = false;
static bool write_frac_done = false;
static bool this_process_active;
static bool time_to_quit = false;
bool using_opengl = false;
APP_CLIENT_SHM *app_client_shm;

// read the INIT_DATA and FD_INIT files
//
int boinc_init() {
    bool standalone = false;
    FILE* f;
    int retval;

#ifdef _WIN32
    freopen(STDERR_FILE, "a", stderr);
#endif

#ifdef API_STANDALONE
    standalone = true;
#endif

    // If in standalone mode, use init files if they're there,
    // but don't demand that they exist
    //
    f = fopen(INIT_DATA_FILE, "r");
    if (!f) {
        if (standalone) {
            safe_strncpy(aid.app_preferences, "", sizeof(aid.app_preferences));
            safe_strncpy(aid.user_name, "John Smith", sizeof(aid.user_name));
            safe_strncpy(aid.team_name, "The A-Team", sizeof(aid.team_name));
            aid.wu_cpu_time = 1000;
            aid.user_total_credit = 1000;
            aid.user_expavg_credit = 500;
            aid.host_total_credit = 1000;
            aid.host_expavg_credit = 500;
            aid.checkpoint_period = DEFAULT_CHECKPOINT_PERIOD;
            aid.fraction_done_update_period = DEFAULT_FRACTION_DONE_UPDATE_PERIOD;
        } else {
            fprintf(stderr, "boinc_init(): can't open init data file\n");
            return ERR_FOPEN;
        }
    } else {
        retval = parse_init_data_file(f, aid);
        fclose(f);
        if (retval) {
            fprintf(stderr, "boinc_init(): can't parse init data file\n");
            return retval;
        }
    }

    f = fopen(GRAPHICS_DATA_FILE, "r");
    if (!f) {
        fprintf(stderr, "boinc_init(): can't open graphics data file\n");
        fprintf(stderr, "Using default graphics settings.\n");
        gi.refresh_period = 0.1; // 1/10th of a second
        gi.xsize = 640;
        gi.ysize = 480;
    } else {
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

    time_until_checkpoint = aid.checkpoint_period;
    time_until_fraction_done_update = aid.fraction_done_update_period;
    this_process_active = true;
    
    boinc_install_signal_handlers();
    set_timer(timer_period);
    setup_shared_mem();

    return 0;
}

// Install signal handlers to aid in debugging
// TODO: write Windows equivalent error handlers?
//
int boinc_install_signal_handlers() {
#ifdef HAVE_SIGNAL_H
    signal( SIGHUP, boinc_catch_signal );  // terminal line hangup
    signal( SIGINT, boinc_catch_signal );  // interrupt program
    signal( SIGQUIT, boinc_quit );         // quit program
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
    static int already_caught_signal = 0;

    // If we've been in this procedure before, something went wrong so we immediately exit
    if (already_caught_signal) _exit(ERR_SIGNAL_CATCH);
    already_caught_signal = 1;
    
    switch (exceptionCode) {
        case STATUS_WAIT_0: safe_strncpy(status,"Wait 0",sizeof(status)); break;
        case STATUS_ABANDONED_WAIT_0: safe_strncpy(status,"Abandoned Wait 0",sizeof(status)); break;
        case STATUS_USER_APC: safe_strncpy(status,"User APC",sizeof(status)); break;
        case STATUS_TIMEOUT: safe_strncpy(status,"Timeout",sizeof(status)); break;
        case STATUS_PENDING: safe_strncpy(status,"Pending",sizeof(status)); break;
        case STATUS_SEGMENT_NOTIFICATION: return DBG_EXCEPTION_NOT_HANDLED;
        case STATUS_GUARD_PAGE_VIOLATION: safe_strncpy(status,"Guard Page Violation",sizeof(status)); break;
        case STATUS_DATATYPE_MISALIGNMENT: safe_strncpy(status,"Data Type Misalignment",sizeof(status)); break;
        case STATUS_BREAKPOINT: return DBG_EXCEPTION_NOT_HANDLED;
        case STATUS_SINGLE_STEP: return DBG_EXCEPTION_NOT_HANDLED;
        case STATUS_ACCESS_VIOLATION: safe_strncpy(status,"Access Violation",sizeof(status)); break;
        case STATUS_IN_PAGE_ERROR: safe_strncpy(status,"In Page Error",sizeof(status)); break;
        case STATUS_NO_MEMORY: safe_strncpy(status,"No Memory",sizeof(status)); break;
        case STATUS_ILLEGAL_INSTRUCTION: safe_strncpy(status,"Illegal Instruction",sizeof(status)); break;
        case STATUS_NONCONTINUABLE_EXCEPTION: safe_strncpy(status,"Noncontinuable Exception",sizeof(status)); break;
        case STATUS_INVALID_DISPOSITION: safe_strncpy(status,"Invalid Disposition",sizeof(status)); break;
        case STATUS_ARRAY_BOUNDS_EXCEEDED: safe_strncpy(status,"Array Bounds Exceeded",sizeof(status)); break;
        case STATUS_FLOAT_DENORMAL_OPERAND: safe_strncpy(status,"Float Denormal Operand",sizeof(status)); break;
        case STATUS_FLOAT_DIVIDE_BY_ZERO: safe_strncpy(status,"Divide by Zero",sizeof(status)); break;
        case STATUS_FLOAT_INEXACT_RESULT: safe_strncpy(status,"Float Inexact Result",sizeof(status)); break;
        case STATUS_FLOAT_INVALID_OPERATION: safe_strncpy(status,"Float Invalid Operation",sizeof(status)); break;
        case STATUS_FLOAT_OVERFLOW: safe_strncpy(status,"Float Overflow",sizeof(status)); break;
        case STATUS_FLOAT_STACK_CHECK: safe_strncpy(status,"Float Stack Check",sizeof(status)); break;
        case STATUS_FLOAT_UNDERFLOW: safe_strncpy(status,"Float Underflow",sizeof(status)); break;
        case STATUS_INTEGER_DIVIDE_BY_ZERO: safe_strncpy(status,"Integer Divide by Zero",sizeof(status)); break;
        case STATUS_INTEGER_OVERFLOW: safe_strncpy(status,"Integer Overflow",sizeof(status)); break;
        case STATUS_PRIVILEGED_INSTRUCTION: safe_strncpy(status,"Privileged Instruction",sizeof(status)); break;
        case STATUS_STACK_OVERFLOW: safe_strncpy(status,"Stack Overflow",sizeof(status)); break;
        case STATUS_CONTROL_C_EXIT: safe_strncpy(status,"Ctrl+C Exit",sizeof(status)); break;
        default: safe_strncpy(status,"Unknown exception",sizeof(status)); break;
    }
    // TODO: also output info in CONTEXT structure?
    fprintf( stderr, "\n***UNHANDLED EXCEPTION****\n");
    fprintf( stderr, "Reason: %s at address 0x%p\n",status,exceptionAddr);
    fprintf( stderr, "Exiting...\n" );
    fflush(stderr);
    _exit(ERR_SIGNAL_CATCH);
    return(EXCEPTION_EXECUTE_HANDLER);
}
#endif

#ifdef HAVE_SIGNAL_H
void boinc_catch_signal(int signal) {
    switch(signal) {
        case SIGHUP: fprintf( stderr, "SIGHUP: terminal line hangup" ); break;
        case SIGINT: fprintf( stderr, "SIGINT: interrupt program" ); break;
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

void boinc_quit(int sig) {
    signal( SIGQUIT, boinc_quit );    // reset signal
    time_to_quit = true;
}
#endif

int boinc_finish(int status) {
    int cur_mem;

    boinc_cpu_time(last_checkpoint_cpu_time, cur_mem);
    update_app_progress(fraction_done, last_checkpoint_cpu_time, last_checkpoint_cpu_time, cur_mem);
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
    cleanup_shared_mem();
    exit(status);
    return 0;
}

int boinc_get_init_data(APP_INIT_DATA& app_init_data) {
    app_init_data = aid;
    return 0;
}

bool boinc_time_to_checkpoint() {
#ifdef __APPLE_CC__
    YieldToAnyThread();
#elif _WIN32
    DWORD eventState;
    // Check if core client has requested us to exit
    eventState = WaitForSingleObject(hQuitRequest, 0L);
    
    switch (eventState) {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:
            time_to_quit = true;
            break;
    }
#endif

    if (write_frac_done) {
        double cur_cpu;
        int cur_mem;
        boinc_cpu_time(cur_cpu, cur_mem);
        update_app_progress(fraction_done, cur_cpu, last_checkpoint_cpu_time, cur_mem);
        time_until_fraction_done_update = aid.fraction_done_update_period;
        write_frac_done = false;
    }

    // If the application has received a quit request it should checkpoint
    // 
    if (time_to_quit) {
        return true;
    }

    return ready_to_checkpoint;
}

int boinc_checkpoint_completed() {
    int cur_mem;
    boinc_cpu_time(last_checkpoint_cpu_time, cur_mem);
    update_app_progress(fraction_done, last_checkpoint_cpu_time, last_checkpoint_cpu_time, cur_mem);
    ready_to_checkpoint = false;
    time_until_checkpoint = aid.checkpoint_period;
    // If it's time to quit, call boinc_finish which will
    // exit the app properly
    if (time_to_quit) {
        boinc_finish(ERR_QUIT_REQUEST);
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

int boinc_cpu_time(double &cpu_t, int &ws_t) {
    double cpu_secs;

    // Start with the CPU time from previous runs, then
    // add the CPU time of the current run
    cpu_secs = aid.wu_cpu_time;

#ifdef HAVE_SYS_RESOURCE_H
    int retval, pid = getpid();
    struct rusage ru;
    retval = getrusage(RUSAGE_SELF, &ru);
    if(retval) {
        fprintf(stderr, "error: could not get cpu time for %d\n", pid);
    	return ERR_GETRUSAGE;
	}
    // Sum the user and system time spent in this process
    cpu_secs += (double)ru.ru_utime.tv_sec + (((double)ru.ru_utime.tv_usec) / ((double)1000000.0));
    cpu_secs += (double)ru.ru_stime.tv_sec + (((double)ru.ru_stime.tv_usec) / ((double)1000000.0));
    cpu_t = cpu_secs;
    ws_t = ru.ru_idrss;
	return 0;
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
        cpu_secs += totTime / 1.e7;

        // Convert to seconds and return
		cpu_t = cpu_secs;
		ws_t = 0;
        return 0;
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
	cpu_t = cpu_secs + ((cur - first_count)/1000.);
	ws_t = 0;
    return 0;
#endif  // _WIN32
#endif

    fprintf(stderr, "boinc_cpu_time(): not implemented\n");
	return -1;
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
    char buf[256];

    // Use Windows multimedia timer, since it is more accurate
    // than SetTimer and doesn't require an associated event loop
    timer_id = timeSetEvent(
        (int)(period*1000), // uDelay
        (int)(period*1000), // uResolution
        on_timer, // lpTimeProc
        NULL, // dwUser
        TIME_PERIODIC  // fuEvent
        );
    sprintf(buf, "%s%s", QUIT_PREFIX, aid.comm_obj_name);
    hQuitRequest = OpenEvent(EVENT_ALL_ACCESS, FALSE, buf);
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

void setup_shared_mem(void) {
	app_client_shm = new APP_CLIENT_SHM;
#ifdef API_STANDALONE
    app_client_shm->shm = NULL;
    fprintf( stderr, "Standalone mode, so not attaching to shared memory.\n" );
    return;
#endif

#ifdef _WIN32
    char buf[256];
    sprintf(buf, "%s%s", SHM_PREFIX, aid.comm_obj_name);
    hSharedMem = attach_shmem(buf, (void**)&app_client_shm->shm);
    if (hSharedMem == NULL)
        app_client_shm = NULL;
#endif

#ifdef HAVE_SYS_SHM_H
#ifdef HAVE_SYS_IPC_H
    if (attach_shmem(aid.shm_key, (void**)&app_client_shm->shm)) {
        app_client_shm = NULL;
    }
#endif
#endif
}

void cleanup_shared_mem(void) {
#ifdef _WIN32
    if (app_client_shm != NULL)
        detach_shmem(hSharedMem, app_client_shm->shm);
#endif

#ifdef HAVE_SYS_SHM_H
#ifdef HAVE_SYS_IPC_H
    if (app_client_shm != NULL)
        detach_shmem(app_client_shm->shm);
#endif
#endif
}


int update_app_progress(double frac_done, double cpu_t, double cp_cpu_t, int ws_t) {
    char msg_buf[SHM_SEG_SIZE];
    
    sprintf( msg_buf,
        "<fraction_done>%2.8f</fraction_done>\n"
        "<current_cpu_time>%10.4f</current_cpu_time>\n"
        "<checkpoint_cpu_time>%10.4f</checkpoint_cpu_time>\n"
        "<working_set_size>%d</working_set_size>\n",
        frac_done, cpu_t, cp_cpu_t, ws_t
    );

    return app_client_shm->send_msg(msg_buf, APP_CORE_WORKER_SEG);
}

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

// Code that's in the BOINC app library (but NOT in the core client)
// graphics-related code goes in graphics_api.C, not here

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#include <afxwin.h>
#include <winuser.h>
#include <mmsystem.h>    // for timing
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

#include "parse.h"
#include "shmem.h"
#include "util.h"
#include "error_numbers.h"
#include "app_ipc.h"

#include "boinc_api.h"

#ifdef _WIN32
HANDLE hQuitRequest, hSharedMem, worker_thread_handle;
LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *ExceptionInfo);
MMRESULT timer_id;
#else
extern void boinc_catch_signal(int signal);
extern void boinc_quit(int sig);
#endif

static APP_INIT_DATA aid;
static double timer_period = 1.0/50.0;    // 50 Hz timer
static double time_until_checkpoint;
static double time_until_fraction_done_update;
static double fraction_done;
static double last_checkpoint_cpu_time;
static bool ready_to_checkpoint = false;
static bool this_process_active;
static bool time_to_quit = false;
static double last_wu_cpu_time;
static bool standalone = false;
static double initial_wu_cpu_time;

APP_CLIENT_SHM *app_client_shm;

bool boinc_is_standalone() {
    return standalone;
}

// parse the init data file.
// This is done at startup, and also if a "reread prefs" message is received
//
int boinc_parse_init_data_file() {
    FILE* f;
    int retval;

    // If in standalone mode, use init files if they're there,
    // but don't demand that they exist
    //
    f = fopen(INIT_DATA_FILE, "r");
    if (!f) {
        if (standalone) {
            safe_strncpy(aid.app_preferences, "", sizeof(aid.app_preferences));
            safe_strncpy(aid.user_name, "Unknown user", sizeof(aid.user_name));
            safe_strncpy(aid.team_name, "Unknown team", sizeof(aid.team_name));
            aid.wu_cpu_time = 1000;
            aid.user_total_credit = 1000;
            aid.user_expavg_credit = 500;
            aid.host_total_credit = 1000;
            aid.host_expavg_credit = 500;
            aid.checkpoint_period = DEFAULT_CHECKPOINT_PERIOD;
            aid.fraction_done_update_period = DEFAULT_FRACTION_DONE_UPDATE_PERIOD;
        } else {
            fprintf(stderr,
                "boinc_parse_init_data_file(): can't open init data file\n"
            );
            return ERR_FOPEN;
        }
    } else {
        retval = parse_init_data_file(f, aid);
        fclose(f);
        if (retval) {
            fprintf(stderr,
                "boinc_parse_init_data_file(): can't parse init data file\n"
            );
            return retval;
        }
    }
    return 0;
}


// Install signal handlers to aid in debugging
// TODO: write Windows equivalent error handlers?
//
int boinc_install_signal_handlers() {
#ifdef HAVE_SIGNAL_H
    signal(SIGHUP, boinc_catch_signal);  // terminal line hangup
    signal(SIGINT, boinc_catch_signal);  // interrupt program
    signal(SIGQUIT, boinc_quit);         // quit program
    signal(SIGILL, boinc_catch_signal);  // illegal instruction
    signal(SIGABRT, boinc_catch_signal); // abort(2) call
    signal(SIGBUS, boinc_catch_signal);  // bus error
    signal(SIGSEGV, boinc_catch_signal); // segmentation violation
    signal(SIGSYS, boinc_catch_signal);  // system call given invalid argument
    signal(SIGPIPE, boinc_catch_signal); // write on a pipe with no reader
#endif
#ifdef _WIN32
    //SetUnhandledExceptionFilter(boinc_catch_signal);
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
    fprintf(stderr, "\n***UNHANDLED EXCEPTION****\n");
    fprintf(stderr, "Reason: %s at address 0x%p\n",status,exceptionAddr);
    fprintf(stderr, "Exiting...\n");
    fflush(stderr);
    _exit(ERR_SIGNAL_CATCH);
    return(EXCEPTION_EXECUTE_HANDLER);
}
#endif

#ifdef HAVE_SIGNAL_H
void boinc_catch_signal(int signal) {
    switch(signal) {
        case SIGHUP: fprintf(stderr, "SIGHUP: terminal line hangup"); break;
        case SIGINT: fprintf(stderr, "SIGINT: interrupt program"); break;
        case SIGILL: fprintf(stderr, "SIGILL: illegal instruction"); break;
        case SIGABRT: fprintf(stderr, "SIGABRT: abort called"); break;
        case SIGBUS: fprintf(stderr, "SIGBUS: bus error"); break;
        case SIGSEGV: fprintf(stderr, "SIGSEGV: segmentation violation"); break;
        case SIGSYS: fprintf(stderr, "SIGSYS: system call given invalid argument"); break;
        case SIGPIPE: fprintf(stderr, "SIGPIPE: write on a pipe with no reader"); break;
        default: fprintf(stderr, "unknown signal %d", signal); break;
    }
    fprintf(stderr, "\nExiting...\n");
    exit(ERR_SIGNAL_CATCH);
}

void boinc_quit(int sig) {
    signal(SIGQUIT, boinc_quit);    // reset signal
    time_to_quit = true;
}
#endif

// communicate to the core client (via shared mem)
// the current CPU time and fraction done
//
static int update_app_progress(
    double frac_done, double cpu_t, double cp_cpu_t, double ws_t
) {
    char msg_buf[SHM_SEG_SIZE];

    if (!app_client_shm) return 0;

    sprintf(msg_buf,
        "<fraction_done>%2.8f</fraction_done>\n"
        "<current_cpu_time>%10.4f</current_cpu_time>\n"
        "<checkpoint_cpu_time>%.15e</checkpoint_cpu_time>\n"
        "<working_set_size>%f</working_set_size>\n",
        frac_done, cpu_t, cp_cpu_t, ws_t
    );

    return app_client_shm->send_msg(msg_buf, APP_CORE_WORKER_SEG);
}

int boinc_get_init_data(APP_INIT_DATA& app_init_data) {
    app_init_data = aid;
    return 0;
}


// this can be called from the graphics thread
//
int boinc_wu_cpu_time(double& cpu_t) {
    cpu_t = last_wu_cpu_time;
    return 0;
}

#ifdef _WIN32
int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu, double& ws) {
    FILETIME creationTime,exitTime,kernelTime,userTime;
    static bool first = true;
    static DWORD first_count = 0;

    if (first) {
        first_count = GetTickCount();
        first = false;
    }
    if (GetThreadTimes(
        thread_handle, &creationTime, &exitTime, &kernelTime, &userTime)
    ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        tKernel.LowPart  = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart    = userTime.dwLowDateTime;
        tUser.HighPart   = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu = totTime / 1.e7;
		ws = 0;
    } else {
        // TODO: Handle timer wraparound
        DWORD cur = GetTickCount();
	    cpu = ((cur - first_count)/1000.);
	    ws = 0;
    }
    return 0;
}

int boinc_worker_thread_cpu_time(double& cpu, double& ws) {
    return boinc_thread_cpu_time(worker_thread_handle, cpu, ws);
}

int boinc_thread_cpu_time(double& cpu, double& ws) {
    return boinc_thread_cpu_time(GetCurrentThread(), cpu, ws);
}
#else
#ifdef HAVE_SYS_RESOURCE_H
int boinc_worker_thread_cpu_time(double &cpu_t, double &ws_t) {
    int retval;
    struct rusage ru;
    retval = getrusage(RUSAGE_SELF, &ru);
    if (retval) {
        fprintf(stderr, "error: could not get CPU time\n");
    	return ERR_GETRUSAGE;
    }
    // Sum the user and system time spent in this process
    cpu_t = (double)ru.ru_utime.tv_sec + (((double)ru.ru_utime.tv_usec) / ((double)1000000.0));
    cpu_t += (double)ru.ru_stime.tv_sec + (((double)ru.ru_stime.tv_usec) / ((double)1000000.0));
    ws_t = ru.ru_idrss;     // TODO: fix this (mult by page size)
    return 0;
}

int boinc_thread_cpu_time(double& cpu, double& ws) {
    return boinc_thread_cpu_time(cpu, ws);
}
#endif
#endif  // _WIN32

#ifdef _WIN32
static void CALLBACK on_timer(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {
#else
static void on_timer(int a) {
#endif

    if (!ready_to_checkpoint) {
        time_until_checkpoint -= timer_period;
        if (time_until_checkpoint <= 0) {
            ready_to_checkpoint = true;
        }
    }

    if (this_process_active) {
        time_until_fraction_done_update -= timer_period;
        if (time_until_fraction_done_update <= 0) {
            double cur_cpu;
            double cur_mem;
            boinc_worker_thread_cpu_time(cur_cpu, cur_mem);
            last_wu_cpu_time = cur_cpu + initial_wu_cpu_time;
            update_app_progress(fraction_done, last_wu_cpu_time, last_checkpoint_cpu_time, cur_mem);
            time_until_fraction_done_update = aid.fraction_done_update_period;
        }
    }
}


static int set_timer(double period) {
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
#endif

#if HAVE_SIGNAL_H
#if HAVE_SYS_TIME_H
    struct sigaction sa;
    itimerval value;
    sa.sa_handler = on_timer;
    sa.sa_flags = SA_RESTART;
    retval = sigaction(SIGALRM, &sa, NULL);
    if (retval) {
        perror("boinc set_timer() sigaction");
        return retval;
    }
    value.it_value.tv_sec = (int)period;
    value.it_value.tv_usec = ((int)(period*1000000))%1000000;
    value.it_interval = value.it_value;
    retval = setitimer(ITIMER_REAL, &value, NULL);
    if (retval) {
        perror("boinc set_timer() setitimer");
    }
#endif
#endif
    return retval;
}

static void setup_shared_mem() {
	app_client_shm = new APP_CLIENT_SHM;
    if (standalone) {
        app_client_shm->shm = NULL;
        fprintf(stderr, "Standalone mode, so not attaching to shared memory.\n");
        return;
    }

#ifdef _WIN32
    char buf[256];
    sprintf(buf, "%s%s", SHM_PREFIX, aid.comm_obj_name);
    hSharedMem = attach_shmem(buf, (void**)&app_client_shm->shm);
    if (hSharedMem == NULL) {
        app_client_shm = NULL;
    }
#endif

#ifdef HAVE_SYS_SHM_H
#ifdef HAVE_SYS_IPC_H
    if (attach_shmem(aid.shm_key, (void**)&app_client_shm->shm)) {
        app_client_shm = NULL;
    }
#endif
#endif
}

static void cleanup_shared_mem() {
    if (!app_client_shm) return;

#ifdef _WIN32
    if (app_client_shm->shm != NULL) {
        detach_shmem(hSharedMem, app_client_shm->shm);
    }
#endif

#ifdef HAVE_SYS_SHM_H
#ifdef HAVE_SYS_IPC_H
    if (app_client_shm->shm != NULL) {
        detach_shmem(app_client_shm->shm);
    }
#endif
#endif
}

int boinc_init(bool standalone_ /* = false */) {
    FILE* f;
    int retval;

#ifdef _WIN32
    freopen(STDERR_FILE, "a", stderr);
    DuplicateHandle(
        GetCurrentProcess(),
        GetCurrentThread(),
        GetCurrentProcess(),
        &worker_thread_handle,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS
    );
#endif

    standalone = standalone_;

    retval = boinc_parse_init_data_file();
    if (retval) return retval;

    // copy the WU CPU time to a separate var,
    // since we may reread the structure again later.
    //
    initial_wu_cpu_time = aid.wu_cpu_time;

    f = fopen(FD_INIT_FILE, "r");
    if (f) {
        parse_fd_init_file(f);
        fclose(f);
    }

    time_until_checkpoint = aid.checkpoint_period;
    last_checkpoint_cpu_time = aid.wu_cpu_time;
    time_until_fraction_done_update = aid.fraction_done_update_period;
    this_process_active = true;
    last_wu_cpu_time = aid.wu_cpu_time;

    boinc_install_signal_handlers();
    set_timer(timer_period);
    setup_shared_mem();

    return 0;
}


int boinc_finish(int status) {
    double cur_mem;

    boinc_thread_cpu_time(last_checkpoint_cpu_time, cur_mem);
    last_checkpoint_cpu_time += aid.wu_cpu_time;
    update_app_progress(fraction_done, last_checkpoint_cpu_time, last_checkpoint_cpu_time, cur_mem);
#ifdef _WIN32
    // Stop the timer
    timeKillEvent(timer_id);
#endif
    cleanup_shared_mem();
    exit(status);
    return 0;
}


bool boinc_time_to_checkpoint() {
#ifdef _WIN32
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

    // If the application has received a quit request it should checkpoint
    //
    if (time_to_quit) {
        return true;
    }

    return ready_to_checkpoint;
}

int boinc_checkpoint_completed() {
    double cur_cpu, cur_mem;
    boinc_thread_cpu_time(cur_cpu, cur_mem);
    last_wu_cpu_time = cur_cpu + aid.wu_cpu_time;
    last_checkpoint_cpu_time = last_wu_cpu_time;
    update_app_progress(fraction_done, last_checkpoint_cpu_time, last_checkpoint_cpu_time, cur_mem);
    ready_to_checkpoint = false;
    time_until_checkpoint = aid.checkpoint_period;

    // If it's time to quit, call boinc_finish which will exit the app properly
    //
    if (time_to_quit) {
        fprintf(stderr, "Received quit request from core client\n");
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

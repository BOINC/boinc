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

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#include <fcntl.h>
#include <algorithm>
#include <sys/types.h>

using namespace std;
#endif

#include "diagnostics.h"
#include "parse.h"
#include "shmem.h"
#include "util.h"
#include "filesys.h"
#include "error_numbers.h"
#include "app_ipc.h"
#include "boinc_api.h"

static APP_INIT_DATA		aid;
APP_CLIENT_SHM		*app_client_shm;

static	double				timer_period = 1.0/50.0;    // 50 Hz timer
static	double				time_until_checkpoint;
static	double				time_until_fraction_done_update;
static	double				fraction_done;
static	double				last_checkpoint_cpu_time;
static	bool				ready_to_checkpoint = false;
static	bool				this_process_active;
static	bool				time_to_quit = false;
static	double				last_wu_cpu_time;
static	bool				standalone = false;
static	double				initial_wu_cpu_time;
static	bool				have_new_trickle = false;

#ifdef _WIN32
HANDLE				hErrorNotification;
HANDLE				hQuitRequest;
HANDLE				hSuspendRequest;
HANDLE				hResumeRequest;
HANDLE				hSharedMem;
HANDLE				worker_thread_handle;
MMRESULT			timer_id;
#endif


//
// Forward declare implementation functions.
//
static void		setup_shared_mem();
static void		cleanup_shared_mem();
static int		update_app_progress(double frac_done, double cpu_t, double cp_cpu_t, double ws_t);
static int		set_timer(double period);

#ifndef _WIN32
static bool core_client_is_running() {
    int retval;
    bool running = true;
    retval = lock_semaphore(aid.core_semaphore_key);
    if (!retval) {
        // we got the semaphore - core client must not be running.
        // release semaphore so that other apps can get it
        //
        running = false;
        unlock_semaphore(aid.core_semaphore_key);
    }
    return running;
}
#endif

// Standard BOINC APIs
//

int boinc_init(bool standalone_ /* = false */) {
    FILE* f;
    int retval;

#ifdef _WIN32

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

    // Store startup mode for later use.
    standalone = standalone_;

	// Parse initial data file.
    retval = boinc_parse_init_data_file();
    if (retval) return retval;

    // copy the WU CPU time to a separate var,
    // since we may reread the structure again later.
    //
    initial_wu_cpu_time = aid.wu_cpu_time;

    f = boinc_fopen(FD_INIT_FILE, "r");
    if (f) {
        parse_fd_init_file(f);
        fclose(f);
    }

    time_until_checkpoint = aid.checkpoint_period;
    last_checkpoint_cpu_time = aid.wu_cpu_time;
    time_until_fraction_done_update = aid.fraction_done_update_period;
    this_process_active = true;
    last_wu_cpu_time = aid.wu_cpu_time;

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
    CloseHandle(worker_thread_handle);
#endif
    cleanup_shared_mem();
    exit(status);
    return 0;
}


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
    f = boinc_fopen(INIT_DATA_FILE, "r");
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
    if (have_new_trickle) {
        strcat(msg_buf, "<have_new_trickle/>\n");
        have_new_trickle = false;
    }

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
    return boinc_worker_thread_cpu_time(cpu, ws);
}
#endif
#endif  // _WIN32

#ifdef _WIN32
static void CALLBACK on_timer(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {
#else
static RETSIGTYPE on_timer(int a) {
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
    //
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
    if (standalone) {
        fprintf(stderr, "Standalone mode, so not using shared memory.\n");
        return;
    }
	app_client_shm = new APP_CLIENT_SHM;

#ifdef _WIN32
    char buf[256];
    sprintf(buf, "%s%s", SHM_PREFIX, aid.comm_obj_name);
    hSharedMem = attach_shmem(buf, (void**)&app_client_shm->shm);
    if (hSharedMem == NULL) {
        delete app_client_shm;
        app_client_shm = NULL;
    }
#endif

#ifdef HAVE_SYS_SHM_H
#ifdef HAVE_SYS_IPC_H
    if (attach_shmem(aid.shm_key, (void**)&app_client_shm->shm)) {
        delete app_client_shm;
        app_client_shm = NULL;
    }
#endif
#endif
}

static void cleanup_shared_mem() {
    if (!app_client_shm) return;

#ifdef _WIN32
    detach_shmem(hSharedMem, app_client_shm->shm);
#endif

#ifdef HAVE_SYS_SHM_H
#ifdef HAVE_SYS_IPC_H
    detach_shmem(app_client_shm->shm);
#endif
#endif
    delete app_client_shm;
    app_client_shm = NULL;
}


int boinc_trickle(char* p) {
    FILE* f = boinc_fopen("trickle", "wb");
    if (!f) return ERR_FOPEN;
    size_t n = fwrite(p, strlen(p), 1, f);
    fclose(f);
    if (n != 1) return ERR_WRITE;
    have_new_trickle = true;
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


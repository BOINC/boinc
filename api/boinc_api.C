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
#include "boinc_win.h"
#else
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <algorithm>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#include "config.h"
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

// The BOINC API communicates CPU time and fraction done to the core client.
// Currently this is done using a timer.
// Remember that the processing of a result can be divided
// into multiple "episodes" (executions of the app),
// each of which resumes from the checkpointed state of the previous episode.
// Unless otherwise noted, "CPU time" refers to the sum over all episodes
// (not counting the part after the last checkpoint in an episode).

static APP_INIT_DATA  aid;
APP_CLIENT_SHM	     *app_client_shm      = 0;
static	double	      timer_period        = 1.0;    // period of API timer
// This determines the resolution of fraction done and CPU time reporting
// to the core client, and of checkpoint enabling.
// It doesn't influence graphics, so 1 sec is enough.
static	double	      time_until_checkpoint;
// countdown timer until enable checkpoint
static	double	      time_until_fraction_done_update;
// countdown timer until report fraction done to core
static	double	      fraction_done;
static	double	      last_checkpoint_cpu_time;
static	bool	      ready_to_checkpoint = false;
//static	bool	      this_process_active;
static	bool	      time_to_quit        = false;
static	double	      last_wu_cpu_time;
static	bool	      standalone          = false;
static	double	      initial_wu_cpu_time;
static	bool	      have_new_trickle_up = false;
static double seconds_until_heartbeat_giveup;
static bool heartbeat_active;   // if false, suppress heartbeat mechanism

#define HEARTBEAT_GIVEUP_PERIOD 5.0
    // quit if no heartbeat from core in this #secs

#ifdef _WIN32
HANDLE	 hErrorNotification;
HANDLE	 hQuitRequest;
HANDLE	 hSuspendRequest;
HANDLE	 hResumeRequest;
HANDLE	 hSharedMem;
HANDLE	 worker_thread_handle;
MMRESULT timer_id;
#endif

static int  setup_shared_mem();
static void cleanup_shared_mem();
static int  update_app_progress(double cpu_t, double cp_cpu_t, double ws_t);
static int  set_timer(double period);

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

    standalone = standalone_;

    retval = boinc_parse_init_data_file();
    if (retval) {
        standalone = true;
    } else {
        retval = setup_shared_mem();
        if (retval) {
            standalone = true;
        }
    }

    // copy the WU CPU time to a separate var,
    // since we may reread the structure again later.
    //
    initial_wu_cpu_time = aid.wu_cpu_time;

    f = boinc_fopen(FD_INIT_FILE, "r");
    if (f) {
        parse_fd_init_file(f);
        fclose(f);
    }

    fraction_done = -1;
    time_until_checkpoint = aid.checkpoint_period;
    last_checkpoint_cpu_time = aid.wu_cpu_time;
    time_until_fraction_done_update = aid.fraction_done_update_period;
    //this_process_active = true;
    last_wu_cpu_time = aid.wu_cpu_time;

    heartbeat_active = !standalone;
    seconds_until_heartbeat_giveup = HEARTBEAT_GIVEUP_PERIOD;

    set_timer(timer_period);

    return 0;
}


int boinc_finish(int status) {
    double cur_mem;

    boinc_calling_thread_cpu_time(last_checkpoint_cpu_time, cur_mem);
    last_checkpoint_cpu_time += aid.wu_cpu_time;
    update_app_progress(last_checkpoint_cpu_time, last_checkpoint_cpu_time, cur_mem);
#ifdef _WIN32
    // Stop the timer
    timeKillEvent(timer_id);
    CloseHandle(worker_thread_handle);
#endif
    cleanup_shared_mem();
    if (status == 0) {
        FILE* f = fopen(BOINC_FINISH_CALLED_FILE, "w");
        if (f) fclose(f);
    }
    aid.wu_cpu_time = last_checkpoint_cpu_time;
    boinc_write_init_data_file();
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
    if (!boinc_file_exists(INIT_DATA_FILE)) {
        if (standalone) {
            safe_strncpy(aid.project_preferences, "", sizeof(aid.project_preferences));
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
        f = boinc_fopen(INIT_DATA_FILE, "r");
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

int boinc_write_init_data_file() {
    FILE* f = boinc_fopen(INIT_DATA_FILE, "w");
    if (!f) return ERR_FOPEN;
    int retval = write_init_data_file(f, aid);
    fclose(f);
    return retval;
}

// communicate to the core client (via shared mem)
// the current CPU time and fraction done
//
static int update_app_progress(
    double cpu_t, double cp_cpu_t, double ws_t
) {
    char msg_buf[SHM_SEG_SIZE], buf[256];

    if (!app_client_shm) return 0;

    sprintf(msg_buf,
        "<current_cpu_time>%10.4f</current_cpu_time>\n"
        "<checkpoint_cpu_time>%.15e</checkpoint_cpu_time>\n"
        "<working_set_size>%f</working_set_size>\n",
        cpu_t, cp_cpu_t, ws_t
    );
    if (fraction_done >= 0) {
        double range = aid.fraction_done_end - aid.fraction_done_start;
        double fdone = aid.fraction_done_start + fraction_done*range;
        sprintf(buf, "<fraction_done>%2.8f</fraction_done>\n", fdone);
        strcat(msg_buf, buf);
    }

    if (have_new_trickle_up) {
        strcat(msg_buf, "<have_new_trickle_up/>\n");
        have_new_trickle_up = false;
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

int boinc_worker_thread_cpu_time(double& cpu, double& ws) {
    return boinc_thread_cpu_time(worker_thread_handle, cpu, ws);
}

#else

// For now, the UNIX API involves only one thread.
//
int boinc_worker_thread_cpu_time(double& cpu, double& ws) {
    return boinc_calling_thread_cpu_time(cpu, ws);
}

#endif  // _WIN32

static void handle_core_app_msgs() {
    char msg_buf[SHM_SEG_SIZE];
    if (!app_client_shm->get_msg(msg_buf, CORE_APP_WORKER_SEG)) return;
    if (match_tag(msg_buf, "<heartbeat/>")) {
        seconds_until_heartbeat_giveup = HEARTBEAT_GIVEUP_PERIOD;
    }
    if (match_tag(msg_buf, "<enable_heartbeat/>")) {
        heartbeat_active = true;
    }
    if (match_tag(msg_buf, "<disable_heartbeat/>")) {
        heartbeat_active = false;
    }
    if (match_tag(msg_buf, "<have_trickle_down/>")) {
    }
}

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

	// handle messages from the core client
	//
    if (app_client_shm) {
        handle_core_app_msgs();
    }

	// see if the core client has died, and we need to die too
	//
    if (heartbeat_active) {
        seconds_until_heartbeat_giveup -= timer_period;
        if (seconds_until_heartbeat_giveup < 0) {
            fprintf(stderr, "No heartbeat from core client - exiting\n");
            exit(0);
        }
    }

    //if (this_process_active) {
        time_until_fraction_done_update -= timer_period;
        if (time_until_fraction_done_update <= 0) {
            double cur_cpu;
            double cur_mem;
            boinc_worker_thread_cpu_time(cur_cpu, cur_mem);
            last_wu_cpu_time = cur_cpu + initial_wu_cpu_time;
            update_app_progress(last_wu_cpu_time, last_checkpoint_cpu_time, cur_mem);
            time_until_fraction_done_update = aid.fraction_done_update_period;
        }
    //}
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

static int setup_shared_mem() {
    if (standalone) {
        fprintf(stderr, "Standalone mode, so not using shared memory.\n");
        return 0;
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
#else
    if (attach_shmem(aid.shm_key, (void**)&app_client_shm->shm)) {
        delete app_client_shm;
        app_client_shm = NULL;
    }
#endif
    if (app_client_shm == NULL) return -1;
    return 0;
}

static void cleanup_shared_mem() {
    if (!app_client_shm) return;

#ifdef _WIN32
    detach_shmem(hSharedMem, app_client_shm->shm);
#else
    detach_shmem(app_client_shm->shm);
#endif
    delete app_client_shm;
    app_client_shm = NULL;
}

int boinc_send_trickle_up(char* p) {
    FILE* f = boinc_fopen(TRICKLE_UP_FILENAME, "wb");
    if (!f) return ERR_FOPEN;
    size_t n = fwrite(p, strlen(p), 1, f);
    fclose(f);
    if (n != 1) return ERR_WRITE;
    have_new_trickle_up = true;
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
    boinc_calling_thread_cpu_time(cur_cpu, cur_mem);
    last_wu_cpu_time = cur_cpu + aid.wu_cpu_time;
    last_checkpoint_cpu_time = last_wu_cpu_time;
    update_app_progress(last_checkpoint_cpu_time, last_checkpoint_cpu_time, cur_mem);
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

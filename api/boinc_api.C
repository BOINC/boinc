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

// Code that's in the BOINC app library (but NOT in the core client)
// graphics-related code goes in graphics_api.C, not here

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "version.h"
#else
#include "config.h"
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#ifndef __EMX__
#include <sched.h>
#endif
using namespace std;
#endif

#include "diagnostics.h"
#include "parse.h"
#include "shmem.h"
#include "util.h"
#include "filesys.h"
#include "mem_usage.h"
#include "error_numbers.h"
#include "app_ipc.h"

#include "boinc_api.h"

// The BOINC API has various functions:
// - check heartbeat from core client, exit if none
// - handle trickle up/down messages
// - report CPU time and fraction done to the core client.

// Implementation notes:
// Unix: getting CPU time and suspend/resume have to be done
// in the worker thread, so we use a SIGALRM signal handler.
// However, many library functions and system calls
// are not "asynch signal safe": see, e.g.
// http://www.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_04.html#tag_02_04_03
// (e.g. sprintf() in a signal handler hangs Mac OS X)
// so we do as little as possible in the signal handler,
// and do the rest in a separate "timer thread".

// Terminology:
// The processing of a result can be divided
// into multiple "episodes" (executions of the app),
// each of which resumes from the checkpointed state of the previous episode.
// Unless otherwise noted, "CPU time" refers to the sum over all episodes
// (not counting the part after the last checkpoint in an episode).

void (*stop_graphics_thread_ptr)() = 0;
static APP_INIT_DATA aid;
static FILE_LOCK file_lock;
APP_CLIENT_SHM* app_client_shm = 0;
static volatile int time_until_checkpoint;
    // time until enable checkpoint
static volatile int time_until_fraction_done_update;
    // time until report fraction done to core client
static double fraction_done;
static double last_checkpoint_cpu_time;
static bool ready_to_checkpoint = false;
static bool in_critical_section = false;
static volatile double last_wu_cpu_time;
static bool standalone          = false;
static double initial_wu_cpu_time;
static volatile bool have_new_trickle_up = false;
static volatile bool have_trickle_down = true;
    // on first call, scan slot dir for msgs
static volatile int heartbeat_giveup_time;
    // interrupt count value at which to give up on core client
static volatile bool heartbeat_active;
    // if false, suppress heartbeat mechanism
#ifdef _WIN32
static volatile int nrunning_ticks = 0;
#endif
static volatile int interrupt_count = 0;
    // number of timer interrupts
    // used to measure elapsed time in a way that's
    // not affected by user changing system clock,
    // and that doesn't have big jumps around hibernation
static double fpops_per_cpu_sec = 0;
static double fpops_cumulative = 0;
static double intops_per_cpu_sec = 0;
static double intops_cumulative = 0;
static int non_cpu_intensive = 0;
static int want_network = 0;
static int have_network = 1;

#define TIMER_PERIOD 1
    // period of worker-thread timer interrupts.
    // This determines the resolution of fraction done and CPU time reporting
    // to the core client, and of checkpoint enabling.
    // It doesn't influence graphics, so 1 sec is enough.
#define HEARTBEAT_GIVEUP_PERIOD (30/TIMER_PERIOD)
    // quit if no heartbeat from core in this #interrupts
#define HEARTBEAT_TIMEOUT_PERIOD 35
    // quit if we cannot aquire slot resource in this #secs

#ifdef _WIN32
static HANDLE hSharedMem;
HANDLE worker_thread_handle;
    // used to suspend worker thread, and to measure its CPU time
static MMRESULT timer_id;
#else
static pthread_t timer_thread_handle;
static struct rusage worker_thread_ru;
#endif

static BOINC_OPTIONS options;
static volatile BOINC_STATUS boinc_status;

// vars related to intermediate file upload
struct UPLOAD_FILE_STATUS {
    std::string name;
    int status;
};
static bool have_new_upload_file;
static std::vector<UPLOAD_FILE_STATUS> upload_file_status;

static int setup_shared_mem() {
    if (standalone) {
        fprintf(stderr, "Standalone mode, so not using shared memory.\n");
        return 0;
    }
    app_client_shm = new APP_CLIENT_SHM;

#ifdef _WIN32
    char buf[256];
    sprintf(buf, "%s%s", SHM_PREFIX, aid.shmem_seg_name);
    hSharedMem = attach_shmem(buf, (void**)&app_client_shm->shm);
    if (hSharedMem == NULL) {
        delete app_client_shm;
        app_client_shm = NULL;
    }
#else
    if (attach_shmem(aid.shmem_seg_name, (void**)&app_client_shm->shm)) {
        delete app_client_shm;
        app_client_shm = NULL;
    }
#endif
    if (app_client_shm == NULL) return -1;
    return 0;
}

// Return CPU time of worker thread.
// This may be called from other threads
//
static int boinc_worker_thread_cpu_time(double& cpu) {
#ifdef _WIN32
    if (boinc_thread_cpu_time(worker_thread_handle, cpu)) {
        cpu = nrunning_ticks * TIMER_PERIOD;   // for Win9x
    }
#else
    cpu = (double)worker_thread_ru.ru_utime.tv_sec
        + (((double)worker_thread_ru.ru_utime.tv_usec)/1000000.0);
    cpu += (double)worker_thread_ru.ru_stime.tv_sec
        + (((double)worker_thread_ru.ru_stime.tv_usec)/1000000.0);
#endif
    return 0;
}

// communicate to the core client (via shared mem)
// the current CPU time and fraction done
//
static bool update_app_progress(
    double cpu_t, double cp_cpu_t, double rss=0, double vm=0
) {
    char msg_buf[MSG_CHANNEL_SIZE], buf[256];

    if (standalone) return true;

    sprintf(msg_buf,
        "<current_cpu_time>%.15e</current_cpu_time>\n"
        "<checkpoint_cpu_time>%.15e</checkpoint_cpu_time>\n"
        "<non_cpu_intensive>%d</non_cpu_intensive>\n",
        cpu_t, cp_cpu_t, non_cpu_intensive
    );
    if (want_network) {
        strcat(msg_buf, "<want_network>1</want_network>\n");
    }
    if (fraction_done >= 0) {
        double range = aid.fraction_done_end - aid.fraction_done_start;
        double fdone = aid.fraction_done_start + fraction_done*range;
        sprintf(buf, "<fraction_done>%2.8f</fraction_done>\n", fdone);
        strcat(msg_buf, buf);
    }
    if (rss) {
        sprintf(buf, "<rss_bytes>%f</rss_bytes>\n", rss);
        strcat(msg_buf, buf);
    }
    if (vm) {
        sprintf(buf, "<vm_bytes>%f</vm_bytes>\n", vm);
        strcat(msg_buf, buf);
    }
    if (fpops_per_cpu_sec) {
        sprintf(buf, "<fpops_per_cpu_sec>%f</fpops_per_cpu_sec>\n", fpops_per_cpu_sec);
        strcat(msg_buf, buf);
    }
    if (fpops_cumulative) {
        sprintf(buf, "<fpops_cumulative>%f</fpops_cumulative>\n", fpops_cumulative);
        strcat(msg_buf, buf);
    }
    if (intops_per_cpu_sec) {
        sprintf(buf, "<intops_per_cpu_sec>%f</intops_per_cpu_sec>\n", intops_per_cpu_sec);
        strcat(msg_buf, buf);
    }
    if (intops_cumulative) {
        sprintf(buf, "<intops_cumulative>%f</intops_cumulative>\n", intops_cumulative);
        strcat(msg_buf, buf);
    }
    return app_client_shm->shm->app_status.send_msg(msg_buf);
}

// the following 2 functions are used for apps without graphics
//
int boinc_init() {
    boinc_options_defaults(options);
    return boinc_init_options(options);
}

int boinc_init_options(BOINC_OPTIONS& opt) {
    int retval;
    retval = boinc_init_options_general(opt);
    if (retval) return retval;
    return set_worker_timer();
}

// the following can be called by either graphics or worker thread
//
int boinc_init_options_general(BOINC_OPTIONS& opt) {
    int retval;
    options = opt;

    boinc_status.no_heartbeat = false;
    boinc_status.suspended = false;
    boinc_status.quit_request = false;

    if (options.main_program) {
        // make sure we're the only app running in this slot
        //
        retval = file_lock.lock(LOCKFILE);
        if (retval) {
            // give any previous occupant a chance to timeout and exit
            //
            boinc_sleep(HEARTBEAT_TIMEOUT_PERIOD);
            retval = file_lock.lock(LOCKFILE);
        }
        if (retval) {
            fprintf(stderr, "Can't acquire lockfile - exiting\n");
            boinc_exit(0);           // not un-recoverable ==> status=0
        }
    }

    retval = boinc_parse_init_data_file();
    if (retval) {
        standalone = true;
    } else {
        retval = setup_shared_mem();
        if (retval) {
            fprintf(stderr, "Can't set up shared mem: %d\n", retval);
            standalone = true;
        }
    }

    // copy the WU CPU time to a separate var,
    // since we may reread the structure again later.
    //
    initial_wu_cpu_time = aid.wu_cpu_time;

    // the following may not be needed, but do it anyway
    //
    fraction_done = -1;
    time_until_checkpoint = (int)aid.checkpoint_period;
    last_checkpoint_cpu_time = aid.wu_cpu_time;
    time_until_fraction_done_update = (int)aid.fraction_done_update_period;
    last_wu_cpu_time = aid.wu_cpu_time;

    heartbeat_active = !standalone;
    heartbeat_giveup_time = interrupt_count + HEARTBEAT_GIVEUP_PERIOD;

    return 0;
}

int boinc_get_status(BOINC_STATUS& s) {
    s.no_heartbeat = boinc_status.no_heartbeat;
    s.suspended = boinc_status.suspended;
    s.quit_request = boinc_status.quit_request;
    return 0;
}

// if we have any new trickle-ups or file upload requests,
// send a message describing them
//
static void send_trickle_up_msg() {
    char buf[MSG_CHANNEL_SIZE];
    strcpy(buf, "");
    if (have_new_trickle_up) {
        strcat(buf, "<have_new_trickle_up/>\n");
    }
    if (have_new_upload_file) {
        strcat(buf, "<have_new_upload_file/>\n");
    }
    if (strlen(buf)) {
        if (app_client_shm->shm->trickle_up.send_msg(buf)) {
            have_new_trickle_up = false;
            have_new_upload_file = false;
        }
    }
}


// NOTE: a non-zero status tells the core client that we're exiting with 
// an "unrecoverable error", which will be reported back to server. 
// A zero exit-status tells the client we've successfully finished the result.
//
int boinc_finish(int status) {
    if (options.send_status_msgs) {
        double total_cpu;
        boinc_worker_thread_cpu_time(total_cpu);
        total_cpu += initial_wu_cpu_time;

        // NOTE: the app_status slot may already contain a message.
        // So retry a couple of times.
        //
        for (int i=0; i<3; i++) {
            if (update_app_progress(total_cpu, total_cpu)) break;
            boinc_sleep(1.0);
        }
    }
    if (options.handle_trickle_ups) {
        send_trickle_up_msg();
    }
#ifdef _WIN32
    // Stop the timer
    timeKillEvent(timer_id);
    CloseHandle(worker_thread_handle);
#endif
    if (options.main_program && status==0) {
        FILE* f = fopen(BOINC_FINISH_CALLED_FILE, "w");
        if (f) fclose(f);
    }
    if (options.send_status_msgs) {
        aid.wu_cpu_time = last_checkpoint_cpu_time;
        boinc_write_init_data_file();
    }

    boinc_exit(status);

    return(0); // doh... we never get here
}


// unlock the lockfile and call the appropriate exit function
// This is called from the worker thread or the timer thread.
//
void boinc_exit(int status) {
    // Shutdown graphics thread if it is running
    //
    if (stop_graphics_thread_ptr) {
        (*stop_graphics_thread_ptr)();
    }

    // Unlock the lock file
    //
    file_lock.unlock(LOCKFILE);

    // on Mac, calling exit() can lead to infinite exit-atexit loops,
    // while _exit() seems to behave nicely.
    // This is not pretty but unless someone finds a cleaner solution, 
    // we handle the Mac-case separately .
#ifdef __APPLE_CC__
    fflush(NULL);
    _exit(status);
#else
    exit(status);
#endif
}

int boinc_is_standalone() {
    if (standalone) return 1;
    return 0;
}


// parse the init data file.
// This is done at startup, and also if a "reread prefs" message is received
//
int boinc_parse_init_data_file() {
    FILE* f;
    int retval;

    memset(&aid, 0, sizeof(aid));
    strlcpy(aid.user_name, "Unknown user", sizeof(aid.user_name));
    strlcpy(aid.team_name, "Unknown team", sizeof(aid.team_name));
    aid.wu_cpu_time = 1000;
    aid.user_total_credit = 1000;
    aid.user_expavg_credit = 500;
    aid.host_total_credit = 1000;
    aid.host_expavg_credit = 500;
    aid.checkpoint_period = DEFAULT_CHECKPOINT_PERIOD;
    aid.fraction_done_update_period = DEFAULT_FRACTION_DONE_UPDATE_PERIOD;

    if (!boinc_file_exists(INIT_DATA_FILE)) {
        fprintf(stderr,
            "Can't open init data file - running in standalone mode\n"
        );
        return ERR_FOPEN;
    }
    f = boinc_fopen(INIT_DATA_FILE, "r");
    retval = parse_init_data_file(f, aid);
    fclose(f);
    if (retval) {
        fprintf(stderr,
            "Can't parse init data file - running in standalone mode\n"
        );
        return retval;
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

int boinc_report_app_status(
    double cpu_time,
    double checkpoint_cpu_time,
    double _fraction_done
) {
    char msg_buf[MSG_CHANNEL_SIZE];
    sprintf(msg_buf,
        "<current_cpu_time>%10.4f</current_cpu_time>\n"
        "<checkpoint_cpu_time>%.15e</checkpoint_cpu_time>\n"
        "<fraction_done>%2.8f</fraction_done>\n",
        cpu_time,
        checkpoint_cpu_time,
        _fraction_done
    );
    app_client_shm->shm->app_status.send_msg(msg_buf);
    return 0;
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

static void handle_heartbeat_msg() {
    char buf[MSG_CHANNEL_SIZE];
    if (app_client_shm->shm->heartbeat.get_msg(buf)) {
        if (match_tag(buf, "<heartbeat/>")) {
            heartbeat_giveup_time = interrupt_count + HEARTBEAT_GIVEUP_PERIOD;
        }
        if (match_tag(buf, "<enable_heartbeat/>")) {
            heartbeat_active = true;
        }
        if (match_tag(buf, "<disable_heartbeat/>")) {
            heartbeat_active = false;
        }
    }
}

static void handle_upload_file_status() {
    char path[256], buf[256], log_name[256];
    std::string filename;
    int status;

    relative_to_absolute("", path);
    DirScanner dirscan(path);
    while (dirscan.scan(filename)) {
        strcpy(buf, filename.c_str());
        if (strstr(buf, UPLOAD_FILE_STATUS_PREFIX) != buf) continue;
        strcpy(log_name, buf+strlen(UPLOAD_FILE_STATUS_PREFIX));
        FILE* f = boinc_fopen(filename.c_str(), "r");
        if (!f) {
            fprintf(stderr, "handle_file_upload_status: can't open %s\n", filename.c_str());
            continue;
        }
        fgets(buf, 256, f);
        fclose(f);
        if (parse_int(buf, "<status>", status)) {
            UPLOAD_FILE_STATUS uf;
            uf.name = std::string(log_name);
            uf.status = status;
            upload_file_status.push_back(uf);
        } else {
            fprintf(stderr, "handle_upload_file_status: can't parse %s\n", buf);
        }
    }
}

// handle trickle and file upload messages
//
static void handle_trickle_down_msg() {
    char buf[MSG_CHANNEL_SIZE];
    if (app_client_shm->shm->trickle_down.get_msg(buf)) {
        if (match_tag(buf, "<have_trickle_down/>")) {
            have_trickle_down = true;
        }
        if (match_tag(buf, "<upload_file_status/>")) {
            handle_upload_file_status();
        }
    }
}

static void handle_process_control_msg() {
    char buf[MSG_CHANNEL_SIZE];
    if (app_client_shm->shm->process_control_request.get_msg(buf)) {
        if (match_tag(buf, "<suspend/>")) {
            boinc_status.suspended = true;
#ifdef _WIN32
            if (options.direct_process_action) {
                // in Windows this is called from a separate "timer thread",
                // and Windows lets us suspend the worker thread
                //
                SuspendThread(worker_thread_handle);
            }
#endif
        }

        if (match_tag(buf, "<resume/>")) {
            boinc_status.suspended = false;
#ifdef _WIN32
            if (options.direct_process_action) {
                // in Windows this is called from a separate "timer thread",
                // and Windows lets us resume the worker thread
                //
                ResumeThread(worker_thread_handle);
            }
#endif
        }

        if (match_tag(buf, "<quit/>")) {
            boinc_status.quit_request = true;
            if (options.direct_process_action) {
                boinc_exit(0);
            }
        }
        if (match_tag(buf, "<reread_app_info/>")) {
            boinc_status.reread_init_data_file = true;
        }
        if (match_tag(buf, "<network_available/>")) {
            have_network = 1;
        }
    }
}

#ifdef _WIN32
static void CALLBACK worker_timer(
    UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2
) {
#else
static void worker_timer(int /*a*/) {
#endif
    interrupt_count++;
    if (!ready_to_checkpoint) {
        time_until_checkpoint -= TIMER_PERIOD;
        if (time_until_checkpoint <= 0) {
            ready_to_checkpoint = true;
        }
    }

    // handle messages from the core client
    //
    if (app_client_shm) {
        if (options.check_heartbeat) {
            handle_heartbeat_msg();
        }
        if (options.handle_trickle_downs) {
            handle_trickle_down_msg();
        }
        if (!in_critical_section && options.handle_process_control) {
            handle_process_control_msg();
        }
    }

    // see if the core client has died, which means we need to die too
    // (unless we're in a critical section)
    //
    if (!in_critical_section && options.check_heartbeat && heartbeat_active) {
        if (heartbeat_giveup_time < interrupt_count) {
            fprintf(stderr,
                "No heartbeat from core client for %d sec - exiting\n",
                interrupt_count - (heartbeat_giveup_time - HEARTBEAT_GIVEUP_PERIOD)
            );
            if (options.direct_process_action) {
                boinc_exit(0);
            } else {
                boinc_status.no_heartbeat = true;
            }
        }
    }

    if (options.send_status_msgs) {
        time_until_fraction_done_update -= TIMER_PERIOD;
        if (time_until_fraction_done_update <= 0) {
            double cur_cpu;
            boinc_worker_thread_cpu_time(cur_cpu);
            last_wu_cpu_time = cur_cpu + initial_wu_cpu_time;
            update_app_progress(last_wu_cpu_time, last_checkpoint_cpu_time);
            time_until_fraction_done_update = (int)aid.fraction_done_update_period;
        }
    }
    if (options.handle_trickle_ups) {
        send_trickle_up_msg();
    }
#ifdef _WIN32
    // poor man's CPU time accounting for Win9x
    //
    if (!boinc_status.suspended) {
        nrunning_ticks++;
    }
#endif
}

#ifndef _WIN32
void* timer_thread(void*) {
    block_sigalrm();
    while(1) {
        boinc_sleep(TIMER_PERIOD);
        worker_timer(0);
    }
    return 0;
}

void worker_signal_handler(int) {
    getrusage(RUSAGE_SELF, &worker_thread_ru);
    if (options.direct_process_action) {
        while (boinc_status.suspended) {
            sleep(1);   // don't use boinc_sleep() because it does FP math
        }
    }
}

#endif

// set up timer actitivies.
// This is called only and always by the worker thread
//
int set_worker_timer() {
    int retval=0;

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

    // Use Windows multimedia timer, since it is more accurate
    // than SetTimer and doesn't require an associated event loop
    //
    timer_id = timeSetEvent(
        (int)(TIMER_PERIOD*1000), // uDelay
        (int)(TIMER_PERIOD*1000), // uResolution
        worker_timer, // lpTimeProc
        NULL, // dwUser
        TIME_PERIODIC  // fuEvent
    );

    // lower our priority here
    //
    SetThreadPriority(worker_thread_handle, THREAD_PRIORITY_IDLE);
#else
    retval = pthread_create(&timer_thread_handle, NULL, timer_thread, NULL);
    if (retval) {
        fprintf(stderr, "set_worker_timer(): pthread_create(): %d", retval);
    }

    struct sigaction sa;
    itimerval value;
    sa.sa_handler = worker_signal_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    retval = sigaction(SIGALRM, &sa, NULL);
    if (retval) {
        perror("boinc set_worker_timer() sigaction");
        return retval;
    }
    value.it_value.tv_sec = TIMER_PERIOD;
    value.it_value.tv_usec = 0;
    value.it_interval = value.it_value;
    retval = setitimer(ITIMER_REAL, &value, NULL);
    if (retval) {
        perror("boinc set_worker_timer() setitimer");
    }
#endif
    return retval;
}

int boinc_send_trickle_up(char* variety, char* p) {
    if (!options.handle_trickle_ups) return ERR_NO_OPTION;
    FILE* f = boinc_fopen(TRICKLE_UP_FILENAME, "wb");
    if (!f) return ERR_FOPEN;
    fprintf(f, "<variety>%s</variety>\n", variety);
    size_t n = fwrite(p, strlen(p), 1, f);
    fclose(f);
    if (n != 1) return ERR_WRITE;
    have_new_trickle_up = true;
    return 0;
}

// logically this should be a bool.
// But it needs to be an int to be compatible with C
//
int boinc_time_to_checkpoint() {
    if (ready_to_checkpoint) {
        in_critical_section = true;
        return 1;
    }
    return 0;
}

int boinc_checkpoint_completed() {
    double cur_cpu;
    boinc_calling_thread_cpu_time(cur_cpu);
    last_wu_cpu_time = cur_cpu + aid.wu_cpu_time;
    last_checkpoint_cpu_time = last_wu_cpu_time;
    update_app_progress(last_checkpoint_cpu_time, last_checkpoint_cpu_time);
    time_until_checkpoint = (int)aid.checkpoint_period;
    in_critical_section = false;
    ready_to_checkpoint = false;

    return 0;
}

void boinc_begin_critical_section() {
    in_critical_section = true;
}
void boinc_end_critical_section() {
    in_critical_section = false;
}

int boinc_fraction_done(double x) {
    fraction_done = x;
    return 0;
}

bool boinc_receive_trickle_down(char* buf, int len) {
    std::string filename;
    char path[256];

    if (!options.handle_trickle_downs) return false;

    if (have_trickle_down) {
        relative_to_absolute("", path);
        DirScanner dirscan(path);
        while (dirscan.scan(filename)) {
            fprintf(stderr, "scan: %s\n", filename.c_str());
            if (strstr(filename.c_str(), "trickle_down")) {
                strncpy(buf, filename.c_str(), len);
                return true;
            }
        }
        have_trickle_down = false;
    }
    return false;
}

int boinc_upload_file(std::string& name) {
    char buf[256];
    std::string pname;
    int retval;

    retval = boinc_resolve_filename_s(name.c_str(), pname);
    if (retval) return retval;
    sprintf(buf, "%s%s", UPLOAD_FILE_REQ_PREFIX, name.c_str());
    FILE* f = boinc_fopen(buf, "w");
    if (!f) return ERR_FOPEN;
    have_new_upload_file = true;
    fclose(f);
    return 0;
}


int boinc_upload_status(std::string& name) {
    for (unsigned int i=0; i<upload_file_status.size(); i++) {
        UPLOAD_FILE_STATUS& ufs = upload_file_status[i];
        if (ufs.name == name) {
            return ufs.status;
        }
    }
    return ERR_NOT_FOUND;
}

void boinc_ops_per_cpu_sec(double fp, double i) {
    fpops_per_cpu_sec = fp;
    intops_per_cpu_sec = i;
}

void boinc_ops_cumulative(double fp, double i) {
    fpops_cumulative = fp;
    intops_cumulative = i;
}

void boinc_not_using_cpu() {
    non_cpu_intensive = 1;
}

void boinc_using_cpu() {
    non_cpu_intensive = 0;
}

void boinc_need_network() {
    want_network = 1;
    have_network = 0;
}

int boinc_network_poll() {
    return have_network?0:1;
}

void boinc_network_done() {
    want_network = 0;
}

#ifndef _WIN32
// block SIGALRM, so that the worker thread will be forced to handle it
//
void block_sigalrm() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}
#endif

const char *BOINC_RCSID_0fa0410386 = "$Id$";

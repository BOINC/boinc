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
#include "win_config.h"
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
#include <csignal>
#ifdef HAVE_PROCFS_H
#include <procfs.h> // definitions for solaris /proc structs
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

APP_INIT_DATA  aid;

APP_CLIENT_SHM       *app_client_shm      = 0;
static double         timer_period        = 1.0;    // period of API timer
// This determines the resolution of fraction done and CPU time reporting
// to the core client, and of checkpoint enabling.
// It doesn't influence graphics, so 1 sec is enough.
static double         time_until_checkpoint;
// countdown timer until enable checkpoint
static double         time_until_fraction_done_update;
// countdown timer until report fraction done to core
static double         fraction_done;
static double         last_checkpoint_cpu_time;
static bool           ready_to_checkpoint = false;
static bool           time_to_quit        = false;
static double         last_wu_cpu_time;
static bool           standalone          = false;
static double         initial_wu_cpu_time;
static bool           have_new_trickle_up = false;
static bool           have_trickle_down   = true;
    // on first call, scan slot dir for msgs
static double         heartbeat_giveup_time;
static bool           heartbeat_active;             // if false, suppress heartbeat mechanism

#define HEARTBEAT_GIVEUP_PERIOD 30.0
    // quit if no heartbeat from core in this #secs
#define HEARTBEAT_TIMEOUT_PERIOD 35.0
    // quit if we cannot aquire slot resource in this #secs

#ifdef _WIN32
//HANDLE   hErrorNotification;
//HANDLE   hQuitRequest;
//HANDLE   hSuspendRequest;
//HANDLE   hResumeRequest;
static HANDLE   hSharedMem;
static HANDLE   worker_thread_handle;
static MMRESULT timer_id;
#endif

static int  setup_shared_mem();
static void cleanup_shared_mem();
static int  update_app_progress(double cpu_t, double cp_cpu_t, double ws_t);
static int  set_timer(double period);
static int  mem_usage(unsigned long& vm_kb, unsigned long& rs_kb);
static BOINC_OPTIONS options;
static BOINC_STATUS boinc_status;

int boinc_init() {
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_trickle_ups = true;
    options.handle_trickle_downs = true;
    options.handle_process_control = true;
    options.send_status_msgs = true;
    options.direct_process_action = true;
    return boinc_init_options(options);
}

int boinc_init_options(BOINC_OPTIONS& opt) {
    FILE* f;
    int retval;
    options = opt;

    memset(&boinc_status, 0, sizeof(boinc_status));

    if (options.main_program) {
        // make sure we're the only app running in this slot
        //
        retval = lock_file(LOCKFILE);
        if (retval) {
            // give any previous occupant a chance to timeout and exit
            //
            boinc_sleep(HEARTBEAT_TIMEOUT_PERIOD);
            retval = lock_file(LOCKFILE);
        }
        if (retval) {
            fprintf(stderr, "Can't acquire lockfile - exiting\n");
            exit(0);
        }
    }

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

    retval = boinc_parse_init_data_file();
    if (retval) {
        standalone = true;
    } else {
        if (aid.core_version && (aid.core_version/100 != BOINC_MAJOR_VERSION)) {
            fprintf(stderr, "Core client has wrong major version: wanted %d, got %d\n",
                BOINC_MAJOR_VERSION, aid.core_version/100
            );
            exit(ERR_MAJOR_VERSION);
        }
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

    f = boinc_fopen(FD_INIT_FILE, "r");
    if (f) {
        parse_fd_init_file(f);
        fclose(f);
    }

    // the following may not be needed, but do it anyway
    //
    fraction_done = -1;
    time_until_checkpoint = aid.checkpoint_period;
    last_checkpoint_cpu_time = aid.wu_cpu_time;
    time_until_fraction_done_update = aid.fraction_done_update_period;
    last_wu_cpu_time = aid.wu_cpu_time;

    heartbeat_active = !standalone;
    heartbeat_giveup_time = dtime() + HEARTBEAT_GIVEUP_PERIOD;

    set_timer(timer_period);

    return 0;
}

int boinc_get_status(BOINC_STATUS& s) {
    s = boinc_status;
    return 0;
}

static void send_trickle_up_msg() {
    if (have_new_trickle_up) {
        if (app_client_shm->shm->trickle_up.send_msg("<have_new_trickle_up/>\n")) {
            have_new_trickle_up = false;
        }
    }
}

int boinc_finish(int status) {
    double cur_mem;
    if (options.send_status_msgs) {
        boinc_calling_thread_cpu_time(last_checkpoint_cpu_time, cur_mem);
        last_checkpoint_cpu_time += aid.wu_cpu_time;
        update_app_progress(last_checkpoint_cpu_time, last_checkpoint_cpu_time, cur_mem);
    }
    if (options.handle_trickle_ups) {
        send_trickle_up_msg();
    }
#ifdef _WIN32
    // Stop the timer
    timeKillEvent(timer_id);
    CloseHandle(worker_thread_handle);
#endif
    cleanup_shared_mem();
    if (options.main_program && status==0) {
        FILE* f = fopen(BOINC_FINISH_CALLED_FILE, "w");
        if (f) fclose(f);
    }
    if (options.send_status_msgs) {
        aid.wu_cpu_time = last_checkpoint_cpu_time;
        boinc_write_init_data_file();
    }
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
    char msg_buf[MSG_CHANNEL_SIZE], buf[256];
    unsigned long vm = 0, rs = 0;

    if (!app_client_shm) return 0;

    sprintf(msg_buf,
        "<current_cpu_time>%10.4f</current_cpu_time>\n"
        "<checkpoint_cpu_time>%.15e</checkpoint_cpu_time>\n",
        cpu_t, cp_cpu_t
    );
    if (fraction_done >= 0) {
        double range = aid.fraction_done_end - aid.fraction_done_start;
        double fdone = aid.fraction_done_start + fraction_done*range;
        sprintf(buf, "<fraction_done>%2.8f</fraction_done>\n", fdone);
        strcat(msg_buf, buf);
    }
    if (!mem_usage(vm, rs)) {
        sprintf(buf,
            "<vm_size>%lu</vm_size>\n"
            "<resident_set_size>%lu</resident_set_size>\n",
            vm, rs
        );
        strcat(msg_buf, buf);
    }
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

static void handle_heartbeat_msg() {
    char buf[MSG_CHANNEL_SIZE];
    if (app_client_shm->shm->heartbeat.get_msg(buf)) {
        if (match_tag(buf, "<heartbeat/>")) {
            heartbeat_giveup_time = dtime() + HEARTBEAT_GIVEUP_PERIOD;
        }
        if (match_tag(buf, "<enable_heartbeat/>")) {
            heartbeat_active = true;
        }
        if (match_tag(buf, "<disable_heartbeat/>")) {
            heartbeat_active = false;
        }
    }
}

static void handle_trickle_down_msg() {
    char buf[MSG_CHANNEL_SIZE];
    if (app_client_shm->shm->trickle_down.get_msg(buf)) {
        if (match_tag(buf, "<have_trickle_down/>")) {
            have_trickle_down = true;
        }
    }
}

static void handle_process_control_msg() {
    char buf[MSG_CHANNEL_SIZE];
    if (app_client_shm->shm->process_control_request.get_msg(buf)) {
        if (match_tag(buf, "<suspend/>")) {
            if (options.direct_process_action) {
#ifdef _WIN32
                SuspendThread(worker_thread_handle);
#else
                while (1) {
                    if (app_client_shm->shm->process_control_request.get_msg(buf)) {
                        if (match_tag(buf, "<resume/>")) {
                            break;
                        }
                    }
                    boinc_sleep(1.0);
                }
                heartbeat_giveup_time = dtime() + HEARTBEAT_GIVEUP_PERIOD;
#endif
            } else {
                boinc_status.suspended = true;
            }
        }

        if (match_tag(buf, "<resume/>")) {
            if (options.direct_process_action) {
#ifdef _WIN32
                ResumeThread(worker_thread_handle);
#endif
            } else {
                boinc_status.suspended = false;
            }
        }

        if (match_tag(buf, "<quit/>")) {
            if (options.direct_process_action) {
                exit(0);
            } else {
                boinc_status.quit_request = true;
            }
        }
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
        if (options.check_heartbeat) {
            handle_heartbeat_msg();
        }
        if (options.handle_trickle_downs) {
            handle_trickle_down_msg();
        }
        if (options.handle_process_control) {
            handle_process_control_msg();
        }
    }

    // see if the core client has died, and we need to die too
    //
    if (options.check_heartbeat && heartbeat_active) {
        if (heartbeat_giveup_time < dtime()) {
            fprintf(stderr, "No heartbeat from core client - exiting\n");
            if (options.direct_process_action) {
                exit(0);
            } else {
                boinc_status.no_heartbeat = true;
            }
        }
    }

    if (options.send_status_msgs) {
        time_until_fraction_done_update -= timer_period;
        if (time_until_fraction_done_update <= 0) {
            double cur_cpu;
            double cur_mem;
            boinc_worker_thread_cpu_time(cur_cpu, cur_mem);
            last_wu_cpu_time = cur_cpu + initial_wu_cpu_time;
            update_app_progress(last_wu_cpu_time, last_checkpoint_cpu_time, cur_mem);
            time_until_fraction_done_update = aid.fraction_done_update_period;
        }
    }
    if (options.handle_trickle_ups) {
        send_trickle_up_msg();
    }
}


static int set_timer(double period) {
    int retval=0;
#ifdef _WIN32

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

bool boinc_time_to_checkpoint() {

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

bool boinc_receive_trickle_down(char* buf, int len) {
    std::string filename;
    char path[256];

    if (!options.handle_trickle_downs) return false;

    if (have_trickle_down) {
        relative_to_absolute("", path);
        DirScanner dirscan(path);
        fprintf(stderr, "starting scan of %s\n", path);
        while (dirscan.scan(filename)) {
            fprintf(stderr, "scan: %s\n", filename.c_str());
            if (strstr(filename.c_str(), "trickle_down")) {
                strncpy(buf, filename.c_str(), len);
                return true;
            }
        }
    }
    return false;
}

static int mem_usage(unsigned long& vm_kb, unsigned long& rs_kb) {

#ifdef _WIN32
    return ERR_NOT_IMPLEMENTED;
#else

    FILE* f;

#if defined(HAVE_PROCFS_H) && defined(HAVE__PROC_SELF_PSINFO)

    // guess that this is solaris
    // need psinfo_t from procfs.h
    //
    if ((f = fopen("/proc/self/psinfo", "r")) != 0) {
        psinfo_t psinfo;

        if (fread(&psinfo, sizeof(psinfo_t), 1, f) == 1) {
            vm_kb = psinfo.pr_size;
            rs_kb = psinfo.pr_rssize;
            fclose(f);
            return 0;
        } else {
            fclose(f);
            return ERR_FREAD;
        }
    }
#endif

#if defined(HAVE__PROC_SELF_STAT)
    // guess that this is linux
    //
    if ((f = fopen("/proc/self/stat", "r")) != 0) {
        char buf[256];
        char* p;
        int i;
        unsigned long tmp;

        i = fread(buf, sizeof(char), 255, f);
        buf[i] = '\0'; // terminate string
        p = &buf[0];

        // skip over first 22 fields
        //
        for (i = 0; i < 22; ++i) {
            p = strchr(p, ' ');
            if (!p) break;
            ++p; // move past space
        }
        if (!p) {
            return ERR_NOT_IMPLEMENTED;
        }

        // read virtual memory size in bytes.
        //
        tmp = strtol(p, &p, 0); // in bytes
        vm_kb = tmp>>10; // bytes to Kb

        // read resident set size: number of  pages  the  process has in
        // real  memory, minus 3 for administrative purposes.
        //
        tmp = strtol(p, 0, 0); // in pages
        rs_kb = ((tmp + 3)*getpagesize())>>10; // getpagesize() is bytes/page

        fclose(f);
        return 0;
    }
#endif

    return ERR_NOT_IMPLEMENTED;
#endif
}


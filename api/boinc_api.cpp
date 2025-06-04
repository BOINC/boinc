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

// The BOINC API and runtime system.
//
// Notes:
// 1) Thread structure:
//  Sequential apps
//    Unix
//      Suspend/resume have to be done in the worker thread,
//      so we use a 10 Hz SIGALRM signal handler.
//      Also get CPU time (getrusage()) in the signal handler.
//      Note: many library functions and system calls
//      are not "asynch signal safe": see, e.g.
//      http://www.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_04.html#tag_02_04_03
//      (e.g. sprintf() in a signal handler hangs Mac OS X).
//      Can't do floating-point math because FP regs not saved.
//      So we do as little as possible in the signal handler,
//      and do the rest in a separate "timer thread".
//          - send status and graphics messages to client
//          - handle messages from client
//          - set ready-to-checkpoint flag
//          - check heartbeat
//          - call app-defined timer callback function
//    Mac: similar to Linux,
//          but getrusage() in the worker signal handler causes crashes,
//          so do it in the timer thread (GETRUSAGE_IN_TIMER_THREAD)
//          TODO: why not do this on Linux too?
//    Android: similar to Linux,
//          but setitimer() causes crashes on some Android versions,
//          so instead of using a periodic signal,
//          have the timer thread send SIGALRM signals to the worker thread
//          every .1 sec.
//          TODO: for uniformity should we do this on Linux as well?
//    Win
//      the timer thread does everything
//  Multi-thread apps:
//    Unix:
//      fork
//      original process runs timer loop:
//        handle suspend/resume/quit, heartbeat (use signals)
//      new process call boinc_init_options() with flags to
//        send status messages and handle checkpoint stuff,
//        and returns from boinc_init_parallel()
//      NOTE: THIS DOESN'T RESPECT CRITICAL SECTIONS.
//      NEED TO MASK SIGNALS IN CHILD DURING CRITICAL SECTIONS
//    Win:
//      like sequential case, except suspend/resume must enumerate
//      all threads (except timer) and suspend/resume them all
//
// 2) All variables that are accessed by two threads (i.e. worker and timer)
//  MUST be declared volatile.
//
// 3) For compatibility with C, we use int instead of bool various places
//
// 4) We must periodically check that the client is still alive and exit if not.
//      Originally this was done using heartbeat msgs from client.
//      This is unreliable, e.g. if the client is blocked for a long time.
//      As of Oct 11 2012 we use a different mechanism:
//      the client passes its PID and we periodically check whether it exists.
//      But we need to support the heartbeat mechanism also for compatibility.
//
// Terminology:
// The processing of a result can be divided
// into multiple "episodes" (executions of the app),
// each of which resumes from the checkpointed state of the previous episode.
// Unless otherwise noted, "CPU time" refers to the sum over all episodes
// (not counting the part after the last checkpoint in an episode).


#ifdef _WIN32
#include "boinc_win.h"
#include "version.h"
#include "win_util.h"
#else
#include "config.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <pthread.h>
#include <vector>
#ifndef __EMX__
#include <sched.h>
#endif
#endif

#include "app_ipc.h"
#include "common_defs.h"
#include "diagnostics.h"
#include "error_numbers.h"
#include "filesys.h"
#include "mem_usage.h"
#include "parse.h"
#include "proc_control.h"
#include "shmem.h"
#include "str_replace.h"
#include "str_util.h"
#include "util.h"

#include "boinc_api.h"

using std::vector;
using std::string;

//#define VERBOSE
    // enable a bunch of fprintfs to stderr

//#define MSGS_FROM_FILE
    // get messages from a file "msgs.txt" instead of shared mem
    // write messages to a file "out_msgs.txt" instead of shared mem

//#define ANDROID
    // use the Android thread/signal logic, which works on Linux too

#ifdef __APPLE__
#include "mac_backtrace.h"
#endif
#if defined(__APPLE__) || defined(ANDROID)
#define GETRUSAGE_IN_TIMER_THREAD
    // call getrusage() in the timer thread,
    // rather than in the worker thread's signal handler
    // (which can cause crashes on Mac)
    // If you want, you can set this for Linux too:
    // CPPFLAGS=-DGETRUSAGE_IN_TIMER_THREAD
#endif

// Anything shared between the worker and timer thread
// must be declared volatile to ensure that writes in one thread
// are seen immediately by the other.

const char* api_version = "API_VERSION_" PACKAGE_VERSION;
static APP_INIT_DATA aid;
static FILE_LOCK file_lock;
APP_CLIENT_SHM* app_client_shm = 0;
static volatile int time_until_checkpoint;
    // time until enable checkpoint
static volatile double fraction_done;
static volatile double last_checkpoint_cpu_time;
static volatile bool ready_to_checkpoint = false;
static volatile int in_critical_section = 0;
static volatile double last_wu_cpu_time;
static volatile bool standalone = true;
static volatile double initial_wu_cpu_time;
static volatile bool have_new_trickle_up = false;
static volatile bool have_trickle_down = true;
    // set if the client notified us of a trickle-down.
    // init to true so the first call to boinc_receive_trickle_down()
    // will scan the slot dir for old trickle-down files
static volatile bool handle_trickle_downs = false;
    // whether we should check for notifications of trickle_downs
    // and file upload status.
    // set by boinc_receive_trickle_down() and boinc_upload_file().
static volatile int heartbeat_giveup_count;
    // interrupt count value at which to give up on client
#ifdef _WIN32
static volatile int nrunning_ticks = 0;
#endif
static volatile int interrupt_count = 0;
    // number of timer interrupts
    // used to measure elapsed time in a way that's
    // not affected by user changing system clock,
    // and doesn't have big jump after hibernation
static volatile int running_interrupt_count = 0;
    // number of timer interrupts while not suspended.
    // Used to compute elapsed time
static volatile bool finishing;
    // used for worker/timer synch during boinc_finish();
static int want_network = 0;
static int have_network = 1;
static double bytes_sent = 0;
static double bytes_received = 0;
bool boinc_disable_timer_thread = false;
    // simulate unresponsive app by setting to true (debugging)
static FUNC_PTR timer_callback = 0;
char web_graphics_url[256];
bool send_web_graphics_url = false;
char remote_desktop_addr[256];
bool send_remote_desktop_addr = false;
int app_min_checkpoint_period = 0;
    // min checkpoint period requested by app
static volatile SPORADIC_AC_STATE ac_state;
static volatile int ac_fd, ca_fd;
static volatile bool do_sporadic_files;

#define TIMER_PERIOD 0.1
    // Sleep interval for timer thread;
    // determines max rate of handling messages from client.
    // Unix: period of worker-thread timer interrupts.
#define TIMERS_PER_SEC 10
    // reciprocal of TIMER_PERIOD
    // This determines the resolution of fraction done and CPU time reporting
    // to the client, and of checkpoint enabling.
#define HEARTBEAT_GIVEUP_SECS 30
#define HEARTBEAT_GIVEUP_COUNT ((int)(HEARTBEAT_GIVEUP_SECS/TIMER_PERIOD))
    // quit if no heartbeat from client in this #interrupts
#define LOCKFILE_TIMEOUT_PERIOD 35
    // quit if we cannot aquire slot lock file in this #secs after startup

#ifdef _WIN32
static HANDLE hSharedMem;
HANDLE worker_thread_handle;
    // used to suspend worker thread, and to measure its CPU time
DWORD timer_thread_id;
#else
static volatile bool worker_thread_exit_flag = false;
static volatile int worker_thread_exit_status;
    // the above are used by the timer thread to tell
    // the worker thread to exit
static pthread_t worker_thread_handle;
static pthread_t timer_thread_handle;
#ifndef GETRUSAGE_IN_TIMER_THREAD
static struct rusage worker_thread_ru;
#endif
#endif

static BOINC_OPTIONS options;
volatile BOINC_STATUS boinc_status;

#ifdef MSGS_FROM_FILE
static FILE* fout;
#endif

// vars related to intermediate file upload
struct UPLOAD_FILE_STATUS {
    std::string name;
    int status;
};
static bool have_new_upload_file;
static std::vector<UPLOAD_FILE_STATUS> upload_file_status;

static int resume_activities();
static void boinc_exit(int);
static void block_sigalrm();
static int start_worker_signals();

char* boinc_msg_prefix(char* sbuf, int len) {
#ifdef ANDROID
    // the time stuff crashes on Android if in a signal handler
    //
    sbuf[0] = 0;
#else
    char buf[256];
    struct tm tm;
    struct tm *tmp = &tm;
    int n;

    time_t x = time(0);
    if (x == -1) {
        strlcpy(sbuf, "time() failed", len);
        return sbuf;
    }
#ifdef _WIN32
#ifdef __MINGW32__
    if ((tmp = localtime(&x)) == NULL) {
#else
    if (localtime_s(&tm, &x) == EINVAL) {
#endif
#else
    if (localtime_r(&x, &tm) == NULL) {
#endif
        strlcpy(sbuf, "localtime() failed", len);
        return sbuf;
    }
    if (strftime(buf, sizeof(buf)-1, "%Y-%m-%d %H:%M:%S", tmp) == 0) {
        strlcpy(sbuf, "strftime() failed", len);
        return sbuf;
    }
#ifdef _WIN32
    n = _snprintf(sbuf, len, "%s (%d):", buf, GetCurrentProcessId());
#else
    n = snprintf(sbuf, len, "%s (%d):", buf, getpid());
#endif
    if (n < 0) {
        strlcpy(sbuf, "sprintf() failed", len);
        return sbuf;
    }
    sbuf[len-1] = 0;    // just in case
#endif  // ANDROID
    return sbuf;
}

#ifndef MSGS_FROM_FILE

static int setup_shared_mem() {
    char buf[256];
    if (standalone) {
        fprintf(stderr,
            "%s Standalone mode, so not using shared memory.\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
        return 0;
    }
    app_client_shm = new APP_CLIENT_SHM;

#ifdef _WIN32
    snprintf(buf, sizeof(buf), "%s%s", SHM_PREFIX, aid.shmem_seg_name);
    hSharedMem = attach_shmem(buf, (void**)&app_client_shm->shm);
    if (hSharedMem == NULL) {
        delete app_client_shm;
        app_client_shm = NULL;
    }
#else
#ifdef __EMX__
    if (attach_shmem(aid.shmem_seg_name, (void**)&app_client_shm->shm)) {
        delete app_client_shm;
        app_client_shm = NULL;
    }
#else
    if (aid.shmem_seg_name == -1) {
        // Version 6 Unix/Linux/Mac client
        if (attach_shmem_mmap(MMAPPED_FILE_NAME, (void**)&app_client_shm->shm)) {
            delete app_client_shm;
            app_client_shm = NULL;
        }
    } else {
        // version 5 Unix/Linux/Mac client
        if (attach_shmem(aid.shmem_seg_name, (void**)&app_client_shm->shm)) {
            delete app_client_shm;
            app_client_shm = NULL;
        }
    }
#endif
#endif  // ! _WIN32
    if (app_client_shm == NULL) return -1;
    return 0;
}
#endif      // MSGS_FROM_FILE

// a mutex for data structures shared between time and worker threads
//
#ifdef _WIN32
static HANDLE mutex;
static void init_mutex() {
    mutex = CreateMutex(NULL, FALSE, NULL);
}
static inline void acquire_mutex() {
    WaitForSingleObject(mutex, INFINITE);
}
static inline void release_mutex() {
    ReleaseMutex(mutex);
}
#else
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static void init_mutex() {}
static inline void acquire_mutex() {
#ifdef VERBOSE
    char buf[256];
    fprintf(stderr, "%s acquiring mutex\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );
#endif
    pthread_mutex_lock(&mutex);
}
static inline void release_mutex() {
#ifdef VERBOSE
    char buf[256];
    fprintf(stderr, "%s releasing mutex\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );
#endif
    pthread_mutex_unlock(&mutex);
}
#endif

// Return CPU time of process.
//
double boinc_worker_thread_cpu_time() {
    double cpu;
#ifdef _WIN32
    int retval;
    retval = boinc_process_cpu_time(GetCurrentProcess(), cpu);
    if (retval) {
        cpu = nrunning_ticks * TIMER_PERIOD;   // for Win9x
    }
#else
#ifdef GETRUSAGE_IN_TIMER_THREAD
    struct rusage worker_thread_ru;
    getrusage(RUSAGE_SELF, &worker_thread_ru);
#endif
    cpu = (double)worker_thread_ru.ru_utime.tv_sec
      + (((double)worker_thread_ru.ru_utime.tv_usec)/1000000.0);
    cpu += (double)worker_thread_ru.ru_stime.tv_sec
      + (((double)worker_thread_ru.ru_stime.tv_usec)/1000000.0);
#endif

    return cpu;
}

// Communicate to the client (via shared mem)
// the current CPU time and fraction done.
// NOTE: various bugs could cause some of these FP numbers to be enormous,
// possibly overflowing the buffer.
// So use strlcat() instead of strcat()
//
// This is called only from the timer thread (so no need for synch)
//
static bool update_app_progress(double cpu_t, double cp_cpu_t) {
    char msg_buf[MSG_CHANNEL_SIZE], buf[256];

    if (standalone) return true;

    snprintf(msg_buf, sizeof(msg_buf),
        "<current_cpu_time>%e</current_cpu_time>\n"
        "<checkpoint_cpu_time>%e</checkpoint_cpu_time>\n",
        cpu_t, cp_cpu_t
    );
    if (want_network) {
        strlcat(msg_buf, "<want_network>1</want_network>\n", sizeof(msg_buf));
    }
    if (fraction_done >= 0) {
        double range = aid.fraction_done_end - aid.fraction_done_start;
        double fdone = aid.fraction_done_start + fraction_done*range;
        snprintf(buf, sizeof(buf), "<fraction_done>%e</fraction_done>\n", fdone);
        strlcat(msg_buf, buf, sizeof(msg_buf));
    }
    if (bytes_sent) {
        snprintf(buf, sizeof(buf), "<bytes_sent>%f</bytes_sent>\n", bytes_sent);
        strlcat(msg_buf, buf, sizeof(msg_buf));
    }
    if (bytes_received) {
        snprintf(buf, sizeof(buf), "<bytes_received>%f</bytes_received>\n", bytes_received);
        strlcat(msg_buf, buf, sizeof(msg_buf));
    }
    if (ac_state) {
        sprintf(buf, "<sporadic_ac>%d</sporadic_ac>\n", ac_state);
        strlcat(msg_buf, buf, sizeof(msg_buf));
    }
#ifdef MSGS_FROM_FILE
    if (fout) {
        fputs(msg_buf, fout);
    }
    return 0;
#else
    return app_client_shm->shm->app_status.send_msg(msg_buf);
#endif
}

// called in timer thread
//
static void handle_heartbeat_msg() {
    char buf[MSG_CHANNEL_SIZE];
    double dtemp;
    bool btemp;
    int i;

    if (!app_client_shm->shm->heartbeat.get_msg(buf)) {
        return;
    }
    boinc_status.network_suspended = false;
    if (match_tag(buf, "<heartbeat/>")) {
        heartbeat_giveup_count = interrupt_count + HEARTBEAT_GIVEUP_COUNT;
    }
    if (parse_double(buf, "<wss>", dtemp)) {
        boinc_status.working_set_size = dtemp;
    }
    if (parse_double(buf, "<max_wss>", dtemp)) {
        boinc_status.max_working_set_size = dtemp;
    }
    if (parse_bool(buf, "suspend_network", btemp)) {
        boinc_status.network_suspended = btemp;
    }
    if (parse_int(buf, "<sporadic_ca>", i)) {
        boinc_status.ca_state = (SPORADIC_CA_STATE)i;
    }
}

// called in timer thread
//
static bool client_dead() {
    char buf[256];
    bool dead;
    if (aid.client_pid) {
        // check every 10 sec
        //
        if (interrupt_count%(TIMERS_PER_SEC*10)) return false;
#ifdef _WIN32
        HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, aid.client_pid);
        // If the process exists but is running under a different user account (boinc_master)
        // then the handle returned is NULL and GetLastError() returns ERROR_ACCESS_DENIED.
        //
        if ((h == NULL) && (GetLastError() != ERROR_ACCESS_DENIED)) {
            dead = true;
        } else {
            if (h) CloseHandle(h);
            dead = false;
        }
#else
        int retval = kill(aid.client_pid, 0);
        dead = (retval == -1 && errno == ESRCH);
#endif
    } else {
        dead = (interrupt_count > heartbeat_giveup_count);
    }
    if (dead) {
        boinc_msg_prefix(buf, sizeof(buf));
        fputs(buf, stderr);     // don't use fprintf() here
        if (aid.client_pid) {
            fputs(" BOINC client no longer exists - exiting\n", stderr);
        } else {
            fputs(" No heartbeat from client for 30 sec - exiting\n", stderr);
        }
        return true;
    }
    return false;
}

// called once/sec in timer thread.
// Copy sporadic app messages to/from files (for wrappers)
//
static void sporadic_files() {
    static time_t last_ac_mod_time = 0;
    static SPORADIC_CA_STATE last_ca_state = CA_NONE;
    char buf[256];

    // if C->A state has changed, write to file
    //
    if (last_ca_state != boinc_status.ca_state) {
        sprintf(buf, "%d\n", boinc_status.ca_state);
        lseek(ca_fd, 0, SEEK_SET);
        if (write(ca_fd, buf, sizeof(buf))) {}
            // one way to avoid warnings
        last_ca_state = boinc_status.ca_state;
    }

    // check if app has updated file with A->C state
    //
    struct stat sbuf;
    int ret = fstat(ac_fd, &sbuf);
    if (!ret) {
#ifdef _WIN32
        time_t t = sbuf.st_mtime;
#elif defined(__APPLE__)
        time_t t = sbuf.st_mtimespec.tv_sec;
#else
        time_t t = sbuf.st_mtim.tv_sec;
#endif
        if (t != last_ac_mod_time) {
            lseek(ac_fd, 0, SEEK_SET);
            int nc = read(ac_fd, buf, sizeof(buf));
            if (nc>0) {
                int val;
                buf[nc] = 0;
                int n = sscanf(buf, "%d", &val);
                if (n == 1) {
                    ac_state = (SPORADIC_AC_STATE)val;
                } else {
                    ac_state = AC_NONE;
                    fprintf(stderr, "API: error parsing AC state: %s\n", buf);
                }
                last_ac_mod_time = t;
            } else {
                fprintf(stderr, "API: error reading AC state: %d\n", nc);
            }
        }
    }
}

#ifndef _WIN32
// For multithread apps on Unix, the main process executes the following.
//
static void parallel_master(int child_pid) {
    char buf[MSG_CHANNEL_SIZE];
    int exit_status;
    while (1) {
        boinc_sleep(TIMER_PERIOD);
        interrupt_count++;
        if (app_client_shm) {
            handle_heartbeat_msg();
            if (app_client_shm->shm->process_control_request.get_msg(buf)) {
                if (match_tag(buf, "<suspend/>")) {
                    kill(child_pid, SIGSTOP);
                } else if (match_tag(buf, "<resume/>")) {
                    kill(child_pid, SIGCONT);
                } else if (match_tag(buf, "<quit/>")) {
                    kill(child_pid, SIGKILL);
                    exit(0);
                } else if (match_tag(buf, "<abort/>")) {
                    kill(child_pid, SIGKILL);
                    exit(EXIT_ABORTED_BY_CLIENT);
                }
            }

            if (client_dead()) {
                kill(child_pid, SIGKILL);
                exit(0);
            }
        }
        if (interrupt_count % TIMERS_PER_SEC) continue;
        if (waitpid(child_pid, &exit_status, WNOHANG) == child_pid) break;
    }
    boinc_finish(exit_status);
}
#endif

int boinc_init() {
#ifndef MSGS_FROM_FILE
    int retval;
    if (!diagnostics_is_initialized()) {
        retval = boinc_init_diagnostics(BOINC_DIAG_DEFAULTS);
        if (retval) return retval;
    }
#endif
    boinc_options_defaults(options);
    return boinc_init_options(&options);
}

int boinc_init_options(BOINC_OPTIONS* opt) {
    int retval;
#ifndef _WIN32
    if (options.multi_thread) {
        int child_pid = fork();
        if (child_pid) {
            // original process - master
            //
            options.send_status_msgs = false;
            retval = boinc_init_options_general(options);
            if (retval) {
                kill(child_pid, SIGKILL);
                return retval;
            }
            parallel_master(child_pid);
        }
        // new process - slave
        //
        options.main_program = false;
        options.check_heartbeat = false;
        options.handle_process_control = false;
        options.multi_thread = false;
        options.multi_process = false;
        return boinc_init_options(&options);
    }
#endif
    retval = boinc_init_options_general(*opt);
    if (retval) return retval;
    retval = start_timer_thread();
    if (retval) return retval;
#ifndef _WIN32
    retval = start_worker_signals();
    if (retval) return retval;
#endif
    return 0;
}

int boinc_init_parallel() {
    BOINC_OPTIONS _options;
    boinc_options_defaults(_options);
    _options.multi_thread = true;
    return boinc_init_options(&_options);
}

static int min_checkpoint_period() {
    int x = (int)aid.checkpoint_period;
    if (app_min_checkpoint_period > x) {
        x = app_min_checkpoint_period;
    }
    if (x == 0) x = DEFAULT_CHECKPOINT_PERIOD;
    return x;
}

int boinc_set_min_checkpoint_period(int x) {
    app_min_checkpoint_period = x;
    if (x > time_until_checkpoint) {
        time_until_checkpoint = x;
    }
    return 0;
}

int boinc_init_options_general(BOINC_OPTIONS& opt) {
    options = opt;

#ifndef MSGS_FROM_FILE
    int retval;
    if (!diagnostics_is_initialized()) {
        retval = boinc_init_diagnostics(BOINC_DIAG_DEFAULTS);
        if (retval) return retval;
    }
#endif

    boinc_status.no_heartbeat = false;
    boinc_status.suspended = false;
    boinc_status.quit_request = false;
    boinc_status.abort_request = false;

#ifdef MSGS_FROM_FILE
    fout = fopen("out_msgs.txt", "w");
    if (!fout) {
        fprintf(stderr, "Can't open out_msgs.txt\n");
    }
    options.check_heartbeat = false;
#else
    char buf[256];
    if (options.main_program) {
        // make sure we're the only app running in this slot
        //
        retval = file_lock.lock(LOCKFILE);
        if (retval) {
            // give any previous occupant a chance to timeout and exit
            //
            fprintf(stderr, "%s Can't acquire lockfile (%d) - waiting %ds\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                retval, LOCKFILE_TIMEOUT_PERIOD
            );
            boinc_sleep(LOCKFILE_TIMEOUT_PERIOD);
            retval = file_lock.lock(LOCKFILE);
        }
        if (retval) {
            fprintf(stderr, "%s Can't acquire lockfile (%d) - exiting\n",
                boinc_msg_prefix(buf, sizeof(buf)),
                retval
            );
#ifdef _WIN32
            char buf2[256];
            windows_format_error_string(GetLastError(), buf2, 256);
            fprintf(stderr, "%s Error: %s\n", boinc_msg_prefix(buf, sizeof(buf)), buf2);
#endif
            // if we can't acquire the lock file there must be
            // another app instance running in this slot.
            // If we exit(0), the client will keep restarting us.
            // Instead, tell the client not to restart us for 10 min.
            //
            boinc_temporary_exit(600,
                "Waiting to acquire slot directory lock.  Another instance may be running."
            );
        }
    }

    standalone = false;
    retval = boinc_parse_init_data_file();
    if (retval) {
        standalone = true;
    } else {
        retval = setup_shared_mem();
        if (retval) {
            fprintf(stderr,
                "%s Can't set up shared mem: %d. Will run in standalone mode.\n",
                boinc_msg_prefix(buf, sizeof(buf)), retval
            );
            standalone = true;
        }
    }
#endif      // MSGS_FROM_FILE

    // copy the WU CPU time to a separate var,
    // since we may reread the structure again later.
    //
    initial_wu_cpu_time = aid.wu_cpu_time;

    fraction_done = -1;
    time_until_checkpoint = min_checkpoint_period();
    last_checkpoint_cpu_time = aid.wu_cpu_time;
    last_wu_cpu_time = aid.wu_cpu_time;

    if (standalone) {
        options.check_heartbeat = false;
    }
    heartbeat_giveup_count = interrupt_count + HEARTBEAT_GIVEUP_COUNT;

    init_mutex();

    return 0;
}

int boinc_get_status(BOINC_STATUS *s) {
    // can just do a struct copy??
    s->no_heartbeat = boinc_status.no_heartbeat;
    s->suspended = boinc_status.suspended;
    s->quit_request = boinc_status.quit_request;
    s->reread_init_data_file = boinc_status.reread_init_data_file;
    s->abort_request = boinc_status.abort_request;
    s->working_set_size = boinc_status.working_set_size;
    s->max_working_set_size = boinc_status.max_working_set_size;
    s->network_suspended = boinc_status.network_suspended;
    s->ca_state = boinc_status.ca_state;
    return 0;
}

// Resolve virtual name (in slot dir) to physical path (in project dir).
// Cases:
// - Windows and pre-6.12 Unix:
//   virtual name refers to a "soft link" (XML file acting as symbolic link)
// - 6.12+ Unix:
//   virtual name is a symbolic link
// - Standalone: physical path is same as virtual name
//
int boinc_resolve_filename(
    const char *virtual_name, char *physical_name, int len
) {
    return resolve_soft_link(virtual_name, physical_name, len);
}

// same, std::string version
//
int boinc_resolve_filename_s(const char *virtual_name, string& physical_name) {
    char buf[512], *p;
    if (!virtual_name) return ERR_NULL;
    physical_name = virtual_name;
#ifndef _WIN32
    if (is_symlink(virtual_name)) {
        return 0;
    }
#endif
    FILE *fp = boinc_fopen(virtual_name, "r");
    if (!fp) return 0;
    buf[0] = 0;
    p = fgets(buf, 512, fp);
    fclose(fp);
    if (p) parse_str(buf, "<soft_link>", physical_name);
    return 0;
}

// if we have any new trickle-ups or file upload requests,
// send a message describing them
//
static void send_trickle_up_msg() {
    char buf[MSG_CHANNEL_SIZE];
    if (standalone) return;
    safe_strcpy(buf, "");
    if (have_new_trickle_up) {
        safe_strcat(buf, "<have_new_trickle_up/>\n");
    }
    if (have_new_upload_file) {
        safe_strcat(buf, "<have_new_upload_file/>\n");
    }
    if (strlen(buf)) {
        BOINCINFO("Sending Trickle Up Message");
        if (app_client_shm->shm->trickle_up.send_msg(buf)) {
            have_new_trickle_up = false;
            have_new_upload_file = false;
        }
    }
}

// NOTE: a non-zero status tells the client that we're exiting with
// an "unrecoverable error", which will be reported back to server.
// A zero exit-status tells the client we've successfully finished the result.
//
int boinc_finish_message(int status, const char* msg, bool is_notice) {
    char buf[256];
    fraction_done = 1;
    fprintf(stderr,
        "%s called boinc_finish(%d)\n",
        boinc_msg_prefix(buf, sizeof(buf)), status
    );
    finishing = true;
    if (!standalone) {
        boinc_sleep(2.0);   // let the timer thread send final messages
        boinc_disable_timer_thread = true;     // then disable it
    }

    if (options.main_program) {
        FILE* f = fopen(BOINC_FINISH_CALLED_FILE, "w");
        if (f) {
            fprintf(f, "%d\n", status);
            if (msg) {
                fprintf(f, "%s\n%s\n", msg, is_notice?"notice":"");
            }
            fclose(f);
        }
    }

    boinc_exit(status);

    return 0;   // never reached
}

int boinc_finish(int status) {
    return boinc_finish_message(status, NULL, false);
}

int boinc_temporary_exit(int delay, const char* reason, bool is_notice) {
    FILE* f = fopen(TEMPORARY_EXIT_FILE, "w");
    if (!f) {
        return ERR_FOPEN;
    }
    fprintf(f, "%d\n", delay);
    if (reason) {
        fprintf(f, "%s\n", reason);
        if (is_notice) {
            fprintf(f, "notice\n");
        }
    }
    fclose(f);
    boinc_exit(0);
    return 0;
}

// unlock the lockfile and call the appropriate exit function
// Unix: called only from the worker thread.
// Win: called from the worker or timer thread.
//
// make static eventually
//
void boinc_exit(int status) {
    int retval;
    char buf[256];

    if (options.main_program && file_lock.locked) {
        retval = file_lock.unlock(LOCKFILE);
        if (retval) {
#ifdef _WIN32
            windows_format_error_string(GetLastError(), buf, 256);
            fprintf(stderr,
                "%s Can't unlock lockfile (%d): %s\n",
                boinc_msg_prefix(buf, sizeof(buf)), retval, buf
            );
#else
            fprintf(stderr,
                "%s Can't unlock lockfile (%d)\n",
                boinc_msg_prefix(buf, sizeof(buf)), retval
            );
            perror("file unlock failed");
#endif
        }
    }

    // kill any processes the app may have created
    //
    if (options.multi_process) {
        kill_descendants();
    }

    boinc_finish_diag();

    // various platforms have problems shutting down a process
    // while other threads are still executing,
    // or triggering endless exit()/atexit() loops.
    //
    BOINCINFO("Exit Status: %d", status);
    fflush(NULL);

#if defined(_WIN32)
    // Halt all the threads and clean up.
    TerminateProcess(GetCurrentProcess(), status);
    // note: the above CAN return!
    Sleep(1000);
    DebugBreak();
#elif defined(__APPLE_CC__)
    // stops endless exit()/atexit() loops.
    _exit(status);
#else
    // arrange to exit with given status even if errors happen
    // in atexit() functions
    //
    set_signal_exit_code(status);
    exit(status);
#endif
}

void boinc_network_usage(double sent, double received) {
    bytes_sent = sent;
    bytes_received = received;
}

int boinc_is_standalone() {
    if (standalone) return 1;
    return 0;
}

int boinc_sporadic_dir(const char* dir) {
    char buf[MAXPATHLEN];

    do_sporadic_files = true;
    sprintf(buf, "%s/ac", dir);
    ac_fd = open(buf, O_CREAT|O_RDONLY, 0666);
    if (ac_fd < 0) {
        fprintf(stderr, "can't open sporadic file %s\n", buf);
        do_sporadic_files = false;
    }
    sprintf(buf, "%s/ca", dir);
    ca_fd = open(buf, O_CREAT|O_WRONLY, 0666);
    if (ca_fd < 0) {
        fprintf(stderr, "can't open sporadic file %s\n", buf);
        do_sporadic_files = false;
    }
    if (!do_sporadic_files) return ERR_FOPEN;
    boinc_status.ca_state = CA_DONT_COMPUTE;
    ac_state = AC_NONE;
    return 0;
}

// called from the timer thread if we need to exit,
// e.g. quit message from client, or client has gone away
//
// On Linux we can't exit directly from the timer thread.
// Set a flag telling the worker thread to exit.
//
static void exit_from_timer_thread(int status) {
#ifdef VERBOSE
    char buf[256];
    fprintf(stderr, "%s exit_from_timer_thread(%d) called\n",
        boinc_msg_prefix(buf, sizeof(buf)), status
    );
#endif
#ifdef _WIN32
    // TerminateProcess() doesn't work if there are suspended threads?
    if (boinc_status.suspended) {
        resume_activities();
    }
    // this seems to work OK on Windows
    //
    boinc_exit(status);
#else
    // but on Unix there are synchronization problems if we exit here;
    // set a flag telling the worker thread to exit
    //
    worker_thread_exit_status = status;
    worker_thread_exit_flag = true;
#ifdef ANDROID
    // trigger the worker signal handler, which will call boinc_exit()
    //
    pthread_kill(worker_thread_handle, SIGALRM);

    // the exit should happen more or less instantly.
    // But if we're still here after 5 sec, exit directly
    //
    sleep(5.0);
    boinc_exit(status);
#else
    pthread_exit(NULL);
#endif
#endif
}

// parse the init data file.
// This is done at startup, and also if a "reread prefs" message is received
//
int boinc_parse_init_data_file() {
    FILE* f;
    int retval;
    char buf[256];

    if (aid.project_preferences) {
        free(aid.project_preferences);
        aid.project_preferences = NULL;
    }
    aid.clear();
    aid.checkpoint_period = DEFAULT_CHECKPOINT_PERIOD;

    if (!boinc_file_exists(INIT_DATA_FILE)) {
        fprintf(stderr,
            "%s Can't open init data file - running in standalone mode\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
        return ERR_FOPEN;
    }
    f = boinc_fopen(INIT_DATA_FILE, "r");
    retval = parse_init_data_file(f, aid);
    fclose(f);
    if (retval) {
        fprintf(stderr,
            "%s Can't parse init data file - running in standalone mode\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
        return retval;
    }
    return 0;
}

// used by wrappers
//
int boinc_report_app_status_aux(
    double cpu_time,
    double checkpoint_cpu_time,
    double _fraction_done,
    int other_pid,
    double _bytes_sent,
    double _bytes_received,
    double wss
) {
    char msg_buf[MSG_CHANNEL_SIZE], buf[1024];
    if (standalone) return 0;

    snprintf(msg_buf, sizeof(msg_buf),
        "<current_cpu_time>%e</current_cpu_time>\n"
        "<checkpoint_cpu_time>%e</checkpoint_cpu_time>\n"
        "<fraction_done>%e</fraction_done>\n",
        cpu_time,
        checkpoint_cpu_time,
        _fraction_done
    );
    if (other_pid) {
        snprintf(buf, sizeof(buf), "<other_pid>%d</other_pid>\n", other_pid);
        safe_strcat(msg_buf, buf);
    }
    if (_bytes_sent) {
        snprintf(buf, sizeof(buf), "<bytes_sent>%f</bytes_sent>\n", _bytes_sent);
        safe_strcat(msg_buf, buf);
    }
    if (_bytes_received) {
        snprintf(buf, sizeof(buf), "<bytes_received>%f</bytes_received>\n", _bytes_received);
        safe_strcat(msg_buf, buf);
    }
    if (ac_state) {
        sprintf(buf, "<sporadic_ac>%d</sporadic_ac>\n", ac_state);
        strlcat(msg_buf, buf, sizeof(msg_buf));
    }
    if (wss) {
        sprintf(buf, "<wss>%f</wss>\n", wss);
        strlcat(msg_buf, buf, sizeof(msg_buf));
    }
#ifdef MSGS_FROM_FILE
    if (fout) {
        fputs(msg_buf, fout);
    }
    return 0;
#else
    if (app_client_shm->shm->app_status.send_msg(msg_buf)) {
        return 0;
    }
    return ERR_WRITE;
#endif
}

int boinc_report_app_status(
    double cpu_time,
    double checkpoint_cpu_time,
    double _fraction_done
){
    return boinc_report_app_status_aux(
        cpu_time, checkpoint_cpu_time, _fraction_done, 0, 0, 0, 0
    );
}

int boinc_get_init_data_p(APP_INIT_DATA* app_init_data) {
    *app_init_data = aid;
    return 0;
}

int boinc_get_init_data(APP_INIT_DATA& app_init_data) {
    app_init_data = aid;
    return 0;
}

int boinc_wu_cpu_time(double& cpu_t) {
    cpu_t = last_wu_cpu_time;
    return 0;
}

// Suspend this job.
// Can be called from either timer or worker thread.
//
static int suspend_activities(bool called_from_worker) {
#ifdef VERBOSE
    char log_buf[256];
    fprintf(stderr, "%s suspend_activities() called from %s\n",
        boinc_msg_prefix(log_buf, sizeof(log_buf)),
        called_from_worker?"worker thread":"timer thread"
    );
#endif
#ifdef _WIN32
    (void) called_from_worker;  // suppress warning
    static vector<int> pids;
    if (options.multi_thread) {
        if (pids.size() == 0) {
            pids.push_back(GetCurrentProcessId());
        }
        suspend_or_resume_threads(pids, timer_thread_id, false, true);
    } else {
        SuspendThread(worker_thread_handle);
    }
#else
    if (options.multi_process) {
        suspend_or_resume_descendants(false);
    }
    // if called from worker thread, sleep until suspension is over
    // if called from timer thread, don't need to do anything;
    // suspension is done by signal handler in worker thread
    //
    if (called_from_worker) {
        while (boinc_status.suspended) {
            sleep(1);
        }
    }
#endif
    return 0;
}

int resume_activities() {
#ifdef VERBOSE
    char log_buf[256];
    fprintf(stderr, "%s resume_activities()\n",
        boinc_msg_prefix(log_buf, sizeof(log_buf))
    );
#endif
#ifdef _WIN32
    static vector<int> pids;
    if (options.multi_thread) {
        if (pids.size() == 0) {
            pids.push_back(GetCurrentProcessId());
        }
        suspend_or_resume_threads(pids, timer_thread_id, true, true);
    } else {
        ResumeThread(worker_thread_handle);
    }
#else
    if (options.multi_process) {
        suspend_or_resume_descendants(true);
    }
#endif
    return 0;
}

#ifndef MSGS_FROM_FILE
static void handle_upload_file_status() {
    char path[MAXPATHLEN], buf[256], log_name[256], *p, log_buf[256];
    std::string filename;
    int status;
    const size_t prefix_len = strlen(UPLOAD_FILE_STATUS_PREFIX);

    relative_to_absolute("", path);
    DirScanner dirscan(path);
    while (dirscan.scan(filename)) {
        strlcpy(buf, filename.c_str(), sizeof(buf));
        if (strstr(buf, UPLOAD_FILE_STATUS_PREFIX) != buf) continue;
        strlcpy(log_name, buf+prefix_len, sizeof(log_name));
        FILE* f = boinc_fopen(filename.c_str(), "r");
        if (!f) {
            fprintf(stderr,
                "%s handle_file_upload_status: can't open %s\n",
                boinc_msg_prefix(buf, sizeof(buf)), filename.c_str()
            );
            continue;
        }
        p = fgets(buf, sizeof(buf), f);
        fclose(f);
        if (p && parse_int(buf, "<status>", status)) {
            UPLOAD_FILE_STATUS uf;
            uf.name = std::string(log_name);
            uf.status = status;
            upload_file_status.push_back(uf);
        } else {
            fprintf(stderr, "%s handle_upload_file_status: can't parse %s\n",
                boinc_msg_prefix(log_buf, sizeof(log_buf)), buf
            );
        }
    }
}

// handle trickle and file upload status messages
//
static void handle_trickle_down_msg() {
    char buf[MSG_CHANNEL_SIZE];
    if (app_client_shm->shm->trickle_down.get_msg(buf)) {
        BOINCINFO("Received Trickle Down Message");
        if (match_tag(buf, "<have_trickle_down/>")) {
            have_trickle_down = true;
        }
        if (match_tag(buf, "<upload_file_status/>")) {
            handle_upload_file_status();
        }
    }
}
#endif

// This flag is set of we get a suspend request while in a critical section,
// and options.direct_process_action is set.
// As soon as we're not in the critical section we'll do the suspend.
//
static bool suspend_request = false;

// runs in timer thread
//
static void handle_process_control_msg() {
    char buf[MSG_CHANNEL_SIZE];
#ifdef MSGS_FROM_FILE
    strcpy(buf, "");
    if (boinc_file_exists("msgs.txt")) {
        FILE* f = fopen("msgs.txt", "r");
        if (!f) {
            fprintf(stderr, "msgs.txt exists but can't open it\n");
            return;
        }
        fgets(buf, sizeof(buf), f);
        fclose(f);
        unlink("msgs.txt");
    }
    if (!strlen(buf)) {
        return;
    }
#else
    if (!app_client_shm->shm->process_control_request.get_msg(buf)) {
        return;
    }
#endif

    // here if we have a message to process

    acquire_mutex();
#ifdef VERBOSE
    char log_buf[256];
    fprintf(stderr, "%s got process control msg %s\n",
        boinc_msg_prefix(log_buf, sizeof(log_buf)), buf
    );
#endif
    if (match_tag(buf, "<suspend/>")) {
        BOINCINFO("Received suspend message");
        if (options.direct_process_action) {
            if (in_critical_section) {
                suspend_request = true;
            } else {
                boinc_status.suspended = true;
                suspend_request = false;
                suspend_activities(false);
            }
        } else {
            boinc_status.suspended = true;
        }
    }

    if (match_tag(buf, "<resume/>")) {
        BOINCINFO("Received resume message");
        if (options.direct_process_action) {
            if (boinc_status.suspended) {
                resume_activities();
            } else if (suspend_request) {
                suspend_request = false;
            }
        }
        boinc_status.suspended = false;
    }

    if (boinc_status.quit_request || match_tag(buf, "<quit/>")) {
        BOINCINFO("Received quit message");
        boinc_status.quit_request = true;
        if (!in_critical_section && options.direct_process_action) {
            release_mutex();
                // we hold mutex, and it's possible that worker
                // is waiting on it, so release it
            exit_from_timer_thread(0);
        }
    }
    if (boinc_status.abort_request || match_tag(buf, "<abort/>")) {
        BOINCINFO("Received abort message");
        boinc_status.abort_request = true;
        if (!in_critical_section && options.direct_process_action) {
            diagnostics_set_aborted_via_gui();
#if   defined(_WIN32)
            // Cause a controlled assert and dump the callstacks.
            DebugBreak();
#elif defined(__APPLE__)
            PrintBacktrace();
#endif
            release_mutex();
            exit_from_timer_thread(EXIT_ABORTED_BY_CLIENT);
        }
    }
    if (match_tag(buf, "<reread_app_info/>")) {
        boinc_status.reread_init_data_file = true;
    }
    if (match_tag(buf, "<network_available/>")) {
        have_network = 1;
    }
#ifdef ANDROID
    // Trigger call to worker_signal_handler() in the worker thread
    //
    pthread_kill(worker_thread_handle, SIGALRM);
#endif
    release_mutex();
}

// timer handler; called every 0.1 sec in the timer thread
//
static void timer_handler() {
    char buf[512];
//#ifdef VERBOSE
#if 0
    fprintf(stderr,
        "%s timer handler: disabled %s; in critical section %s; finishing %s\n",
        boinc_msg_prefix(buf, sizeof(buf)),
        boinc_disable_timer_thread?"yes":"no",
        in_critical_section?"yes":"no",
        finishing?"yes":"no"
    );
#endif
    if (boinc_disable_timer_thread) {
        return;
    }
    if (finishing) {
        if (options.send_status_msgs) {
            double cur_cpu = boinc_worker_thread_cpu_time();
            last_wu_cpu_time = cur_cpu + initial_wu_cpu_time;
            update_app_progress(last_wu_cpu_time, last_checkpoint_cpu_time);
        }
        boinc_disable_timer_thread = true;
        return;
    }
    interrupt_count++;
    if (!boinc_status.suspended) {
        running_interrupt_count++;
    }
    // handle messages from the client
    //
#ifdef MSGS_FROM_FILE
    handle_process_control_msg();
#else
    if (app_client_shm) {
        if (options.check_heartbeat) {
            handle_heartbeat_msg();
        }
        if (handle_trickle_downs) {
            handle_trickle_down_msg();
        }
        if (options.handle_process_control) {
            handle_process_control_msg();
        }
    }
#endif
    if (interrupt_count % TIMERS_PER_SEC) return;

#ifdef VERBOSE
    fprintf(stderr, "%s 1 sec elapsed - doing slow actions\n", boinc_msg_prefix(buf, sizeof(buf)));
#endif

    // here if we're at a one-second boundary; do slow stuff
    //

    if (!ready_to_checkpoint) {
        time_until_checkpoint -= 1;
        if (time_until_checkpoint <= 0) {
            ready_to_checkpoint = true;
        }
    }

    // see if the client has died, which means we need to die too
    // (unless we're in a critical section)
    //
    if (options.check_heartbeat) {
        if (client_dead()) {
            fprintf(stderr, "%s timer handler: client dead, exiting\n",
                boinc_msg_prefix(buf, sizeof(buf))
            );
            if (options.direct_process_action && !in_critical_section) {
                exit_from_timer_thread(0);
            } else {
                boinc_status.no_heartbeat = true;
            }
        }
    }

    // don't bother reporting CPU time etc. if we're suspended
    //
    if (options.send_status_msgs && !boinc_status.suspended) {
        double cur_cpu = boinc_worker_thread_cpu_time();
        last_wu_cpu_time = cur_cpu + initial_wu_cpu_time;
        update_app_progress(last_wu_cpu_time, last_checkpoint_cpu_time);
    }

    if (have_new_trickle_up || have_new_upload_file) {
        send_trickle_up_msg();
    }
    if (timer_callback) {
        timer_callback();
    }

    // send graphics-related messages
    //
    if (send_web_graphics_url && !app_client_shm->shm->graphics_reply.has_msg()) {
        snprintf(buf, sizeof(buf),
            "<web_graphics_url>%s</web_graphics_url>",
            web_graphics_url
        );
        app_client_shm->shm->graphics_reply.send_msg(buf);
        send_web_graphics_url = false;
    }
    if (send_remote_desktop_addr && !app_client_shm->shm->graphics_reply.has_msg()) {
        snprintf(buf, sizeof(buf),
            "<remote_desktop_addr>%s</remote_desktop_addr>",
            remote_desktop_addr
        );
        app_client_shm->shm->graphics_reply.send_msg(buf);
        send_remote_desktop_addr = false;
    }

    if (do_sporadic_files) {
        sporadic_files();
    }
}

#ifdef _WIN32

DWORD WINAPI timer_thread(void *) {
    while (1) {
        Sleep((int)(TIMER_PERIOD*1000));
        timer_handler();

        // poor man's CPU time accounting for Win9x
        //
        if (!boinc_status.suspended) {
            nrunning_ticks++;
        }
    }
    return 0;
}

#else

static void* timer_thread(void*) {
    block_sigalrm();
    while(1) {
        boinc_sleep(TIMER_PERIOD);
        timer_handler();
    }
    return 0;
}

// This SIGALRM handler gets handled only by the worker thread.
// It gets CPU time and implements sleeping.
// It must call only signal-safe functions, and must not do FP math
//
static void worker_signal_handler(int) {
#ifdef ANDROID
    // per-thread signal masking doesn't work on pre-4.1 Android.
    // If we're handling this signal in the timer thread,
    // send signal explicitly to worker thread.
    //
    if (pthread_self() != worker_thread_handle) {
#ifdef VERBOSE
        fprintf(stderr, "worker signal handler: called in timer thread; forwarding to worker\n");
#endif
        pthread_kill(worker_thread_handle, SIGALRM);
        return;
    }
#endif
#ifndef GETRUSAGE_IN_TIMER_THREAD
    getrusage(RUSAGE_SELF, &worker_thread_ru);
#endif
    if (worker_thread_exit_flag) {
#ifdef VERBOSE
        fprintf(stderr, "worker signal handler: exiting\n");
#endif
        boinc_exit(worker_thread_exit_status);
    }
    if (options.direct_process_action) {
        while (boinc_status.suspended && in_critical_section==0) {
#ifdef VERBOSE
            fprintf(stderr, "worker signal handler: sleeping\n");
#endif
            sleep(1);   // don't use boinc_sleep() because it does FP math
        }
    }
}

#endif


// Called from the worker thread; create the timer thread
//
int start_timer_thread() {
    char buf[256];

#ifdef _WIN32

    // get the worker thread handle
    //
    DuplicateHandle(
        GetCurrentProcess(),
        GetCurrentThread(),
        GetCurrentProcess(),
        &worker_thread_handle,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS
    );

    // Create the timer thread
    //
    if (!CreateThread(NULL, 0, timer_thread, 0, 0, &timer_thread_id)) {
        fprintf(stderr,
            "%s start_timer_thread(): CreateThread() failed, errno %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), errno
        );
        return errno;
    }

    if (!options.normal_thread_priority) {
        // lower our (worker thread) priority
        //
        SetThreadPriority(worker_thread_handle, THREAD_PRIORITY_IDLE);
    }
#else
    worker_thread_handle = pthread_self();
    pthread_attr_t thread_attrs;
    pthread_attr_init(&thread_attrs);
    pthread_attr_setstacksize(&thread_attrs, 32768);
    int retval = pthread_create(&timer_thread_handle, &thread_attrs, timer_thread, NULL);
    if (retval) {
        fprintf(stderr,
            "%s start_timer_thread(): pthread_create(): %d",
            boinc_msg_prefix(buf, sizeof(buf)), retval
        );
        return retval;
    }
#endif
    return 0;
}

#ifndef _WIN32

// called in the worker thread.
// set up a handler for SIGALRM.
// If Android, we'll get signals from the timer thread.
// otherwise, set an interval timer to deliver signals
//
static int start_worker_signals() {
    int retval;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = worker_signal_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    retval = sigaction(SIGALRM, &sa, NULL);
    if (retval) {
        perror("boinc start_worker_signals(): sigaction failed");
        return retval;
    }
#ifndef ANDROID
    itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = (int)(TIMER_PERIOD*1e6);
    value.it_interval = value.it_value;
    retval = setitimer(ITIMER_REAL, &value, NULL);
    if (retval) {
        perror("boinc start_worker_thread(): setitimer failed");
        return retval;
    }
#endif
    return 0;
}
#endif

int boinc_send_trickle_up(char* variety, char* p) {
    FILE* f = boinc_fopen(TRICKLE_UP_FILENAME, "wb");
    if (!f) return ERR_FOPEN;
    fprintf(f, "<variety>%s</variety>\n", variety);
    size_t n = 1;
    if (strlen(p)) {
        n = fwrite(p, strlen(p), 1, f);
    }
    fclose(f);
    if (n != 1) return ERR_WRITE;
    have_new_trickle_up = true;
    return 0;
}

int boinc_time_to_checkpoint() {
    if (ready_to_checkpoint) {
        boinc_begin_critical_section();
        return 1;
    }
    return 0;
}

int boinc_checkpoint_completed() {
    double cur_cpu;
    cur_cpu = boinc_worker_thread_cpu_time();
    last_wu_cpu_time = cur_cpu + aid.wu_cpu_time;
    last_checkpoint_cpu_time = last_wu_cpu_time;
    time_until_checkpoint = min_checkpoint_period();
    boinc_end_critical_section();
    ready_to_checkpoint = false;

    return 0;
}

void boinc_begin_critical_section() {
#ifdef VERBOSE
    char buf[256];
    fprintf(stderr,
        "%s begin_critical_section\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );
#endif
    in_critical_section++;
}

void boinc_end_critical_section() {
#ifdef VERBOSE
    char buf[256];
    fprintf(stderr,
        "%s end_critical_section\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );
#endif
    in_critical_section--;
    if (in_critical_section < 0) {
        in_critical_section = 0;        // just in case
    }

    if (in_critical_section) return;

    // We're out of the critical section.
    // See if we got suspend/quit/abort while in critical section,
    // and handle them here.
    //
    if (options.direct_process_action) {
        if (boinc_status.no_heartbeat) {
            boinc_exit(0);
        }
        if (boinc_status.quit_request) {
            boinc_exit(0);
        }
        if (boinc_status.abort_request) {
            boinc_exit(EXIT_ABORTED_BY_CLIENT);
        }
        acquire_mutex();
        if (suspend_request) {
            suspend_request = false;
            boinc_status.suspended = true;
            release_mutex();
            suspend_activities(true);
        } else {
            release_mutex();
        }
    }
}

int boinc_fraction_done(double x) {
    fraction_done = x;
    return 0;
}

int boinc_receive_trickle_down(char* buf, int len) {
    std::string filename;
    char path[MAXPATHLEN];

    handle_trickle_downs = true;

    if (have_trickle_down) {
        relative_to_absolute("", path);
        DirScanner dirscan(path);
        while (dirscan.scan(filename)) {
            if (strstr(filename.c_str(), "trickle_down")) {
                strlcpy(buf, filename.c_str(), len);
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
    snprintf(buf, sizeof(buf), "%s%s", UPLOAD_FILE_REQ_PREFIX, name.c_str());
    FILE* f = boinc_fopen(buf, "w");
    if (!f) return ERR_FOPEN;
    have_new_upload_file = true;
    fclose(f);

    // file upload status messages are on same channel as
    // trickle down messages, so listen to that channel
    //
    handle_trickle_downs = true;

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
static void block_sigalrm() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}
#endif

void boinc_register_timer_callback(FUNC_PTR p) {
    timer_callback = p;
}

double boinc_get_fraction_done() {
    return fraction_done;
}

double boinc_elapsed_time() {
    return running_interrupt_count*TIMER_PERIOD;
}

void boinc_web_graphics_url(char* url) {
    if (standalone) return;
    strlcpy(web_graphics_url, url, sizeof(web_graphics_url));
    send_web_graphics_url = true;
}

void boinc_remote_desktop_addr(char* addr) {
    if (standalone) return;
    strlcpy(remote_desktop_addr, addr, sizeof(remote_desktop_addr));
    send_remote_desktop_addr = true;
}

void boinc_sporadic_set_ac_state(SPORADIC_AC_STATE a) {
    ac_state = a;
}

SPORADIC_CA_STATE boinc_sporadic_get_ca_state() {
    return boinc_status.ca_state;
}


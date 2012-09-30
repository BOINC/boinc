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
//      getting CPU time and suspend/resume have to be done
//      in the worker thread, so we use a SIGALRM signal handler.
//      However, many library functions and system calls
//      are not "asynch signal safe": see, e.g.
//      http://www.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_04.html#tag_02_04_03
//      (e.g. sprintf() in a signal handler hangs Mac OS X)
//      so we do as little as possible in the signal handler,
//      and do the rest in a separate "timer thread".
//    Win
//      the timer thread does everything
//  Parallel apps.
//    Unix:
//      fork
//      original process runs timer loop:
//        handle suspend/resume/quit, heartbeat (use signals)
//      new process call boinc_init_options() with flags to
//        send status messages and handle checkpoint stuff,
//        and returns from boinc_init_parallel()
//    Win:
//      like sequential case, except suspend/resume must enumerate
//      all threads (except timer) and suspend/resume them all
//
// 2) All variables that are accessed by two threads (i.e. worker and timer)
//  MUST be declared volatile.
//
// 3) For compatibility with C, we use int instead of bool various places
//
// Terminology:
// The processing of a result can be divided
// into multiple "episodes" (executions of the app),
// each of which resumes from the checkpointed state of the previous episode.
// Unless otherwise noted, "CPU time" refers to the sum over all episodes
// (not counting the part after the last checkpoint in an episode).

#include <vector>

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "version.h"
#include "win_util.h"
#else
#include "config.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <pthread.h>
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
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "boinc_api.h"

using std::vector;

//#define DEBUG_BOINC_API

#ifdef __APPLE__
#include "mac_backtrace.h"
#define GETRUSAGE_IN_TIMER_THREAD
    // call getrusage() in the timer thread,
    // rather than in the worker thread's signal handler
    // (which can cause crashes on Mac)
    // If you want, you can set this for Linux too:
    // CPPFLAGS=-DGETRUSAGE_IN_TIMER_THREAD
#endif

const char* api_version="API_VERSION_"PACKAGE_VERSION;
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
static volatile bool standalone = false;
static volatile double initial_wu_cpu_time;
static volatile bool have_new_trickle_up = false;
static volatile bool have_trickle_down = true;
    // on first call, scan slot dir for msgs
static volatile int heartbeat_giveup_time;
    // interrupt count value at which to give up on core client
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
static int want_network = 0;
static int have_network = 1;
static double bytes_sent = 0;
static double bytes_received = 0;
bool g_sleep = false;
    // simulate unresponsive app by setting to true (debugging)
static FUNC_PTR timer_callback = 0;
char web_graphics_url[256];
bool send_web_graphics_url = false;
char remote_desktop_addr[256];
bool send_remote_desktop_addr = false;
int app_min_checkpoint_period = 0;
    // min checkpoint period requested by app

#define TIMER_PERIOD 0.1
    // period of worker-thread timer interrupts.
    // Determines rate of handlling messages from client.
#define TIMERS_PER_SEC 10
    // This determines the resolution of fraction done and CPU time reporting
    // to the core client, and of checkpoint enabling.
    // It doesn't influence graphics, so 1 sec is enough.
#define HEARTBEAT_GIVEUP_COUNT ((int)(30/TIMER_PERIOD))
    // quit if no heartbeat from core in this #interrupts
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
static pthread_t timer_thread_handle;
#ifndef GETRUSAGE_IN_TIMER_THREAD
static struct rusage worker_thread_ru;
#endif
#endif

static BOINC_OPTIONS options;
volatile BOINC_STATUS boinc_status;

// vars related to intermediate file upload
struct UPLOAD_FILE_STATUS {
    std::string name;
    int status;
};
static bool have_new_upload_file;
static std::vector<UPLOAD_FILE_STATUS> upload_file_status;

static void graphics_cleanup();
static int suspend_activities();
static int resume_activities();
static void boinc_exit(int);
static void block_sigalrm();
static int start_worker_signals();

char* boinc_msg_prefix(char* sbuf, int len) {
    char buf[256];
    struct tm tm;
    struct tm *tmp = &tm;
    int n;

    time_t x = time(0);
    if (x == -1) {
        strcpy(sbuf, "time() failed");
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
        strcpy(sbuf, "localtime() failed");
        return sbuf;
    }
    if (strftime(buf, sizeof(buf)-1, "%H:%M:%S", tmp) == 0) {
        strcpy(sbuf, "strftime() failed");
        return sbuf;
    }
#ifdef _WIN32
    n = _snprintf(sbuf, len, "%s (%d):", buf, GetCurrentProcessId());
#else
    n = snprintf(sbuf, len, "%s (%d):", buf, getpid());
#endif
    if (n < 0) {
        strcpy(sbuf, "sprintf() failed");
        return sbuf;
    }
    sbuf[len-1] = 0;    // just in case
    return sbuf;
}

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
    sprintf(buf, "%s%s", SHM_PREFIX, aid.shmem_seg_name);
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

// Communicate to the core client (via shared mem)
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

    sprintf(msg_buf,
        "<current_cpu_time>%e</current_cpu_time>\n"
        "<checkpoint_cpu_time>%e</checkpoint_cpu_time>\n",
        cpu_t, cp_cpu_t
    );
    if (want_network) {
        strlcat(msg_buf, "<want_network>1</want_network>\n", MSG_CHANNEL_SIZE);
    }
    if (fraction_done >= 0) {
        double range = aid.fraction_done_end - aid.fraction_done_start;
        double fdone = aid.fraction_done_start + fraction_done*range;
        sprintf(buf, "<fraction_done>%e</fraction_done>\n", fdone);
        strlcat(msg_buf, buf, MSG_CHANNEL_SIZE);
    }
    if (bytes_sent) {
        sprintf(buf, "<bytes_sent>%f</bytes_sent>\n", bytes_sent);
        strcat(msg_buf, buf);
    }
    if (bytes_received) {
        sprintf(buf, "<bytes_received>%f</bytes_received>\n", bytes_received);
        strcat(msg_buf, buf);
    }
    return app_client_shm->shm->app_status.send_msg(msg_buf);
}

static void handle_heartbeat_msg() {
    char buf[MSG_CHANNEL_SIZE];
    double dtemp;
    bool btemp;

    if (app_client_shm->shm->heartbeat.get_msg(buf)) {
        boinc_status.network_suspended = false;
        if (match_tag(buf, "<heartbeat/>")) {
            heartbeat_giveup_time = interrupt_count + HEARTBEAT_GIVEUP_COUNT;
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

            if (heartbeat_giveup_time < interrupt_count) {
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
    int retval;
    if (!diagnostics_is_initialized()) {
        retval = boinc_init_diagnostics(BOINC_DIAG_DEFAULTS);
        if (retval) return retval;
    }
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
    int retval;
    char buf[256];
    options = opt;

    if (!diagnostics_is_initialized()) {
        retval = boinc_init_diagnostics(BOINC_DIAG_DEFAULTS);
        if (retval) return retval;
    }

    boinc_status.no_heartbeat = false;
    boinc_status.suspended = false;
    boinc_status.quit_request = false;
    boinc_status.abort_request = false;

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
            windows_error_string(buf2, 256);
            fprintf(stderr, "%s Error: %s\n", boinc_msg_prefix(buf, sizeof(buf)), buf2);
#endif
            // if we can't acquire the lock file there must be
            // another app instance running in this slot.
            // If we exit(0), the client will keep restarting us.
            // Instead, tell the client not to restart us for 10 min.
            //
            boinc_temporary_exit(600, "Waiting to acquire lock");
        }
    }

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
    heartbeat_giveup_time = interrupt_count + HEARTBEAT_GIVEUP_COUNT;

    return 0;
}

int boinc_get_status(BOINC_STATUS *s) {
    s->no_heartbeat = boinc_status.no_heartbeat;
    s->suspended = boinc_status.suspended;
    s->quit_request = boinc_status.quit_request;
    s->reread_init_data_file = boinc_status.reread_init_data_file;
    s->abort_request = boinc_status.abort_request;
    s->working_set_size = boinc_status.working_set_size;
    s->max_working_set_size = boinc_status.max_working_set_size;
    s->network_suspended = boinc_status.network_suspended;
    return 0;
}

// if we have any new trickle-ups or file upload requests,
// send a message describing them
//
static void send_trickle_up_msg() {
    char buf[MSG_CHANNEL_SIZE];
    BOINCINFO("Sending Trickle Up Message");
    if (standalone) return;
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
    char buf[256];
    fraction_done = 1;
    fprintf(stderr,
        "%s called boinc_finish\n",
        boinc_msg_prefix(buf, sizeof(buf))
    );
    boinc_sleep(2.0);   // let the timer thread send final messages
    g_sleep = true;     // then disable it

    if (options.main_program && status==0) {
        FILE* f = fopen(BOINC_FINISH_CALLED_FILE, "w");
        if (f) fclose(f);
    }

    boinc_exit(status);

    return 0;   // never reached
}

int boinc_temporary_exit(int delay, const char* reason) {
    FILE* f = fopen(TEMPORARY_EXIT_FILE, "w");
    if (!f) {
        return ERR_FOPEN;
    }
    fprintf(f, "%d\n", delay);
    if (reason) {
        fprintf(f, "%s\n", reason);
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

    graphics_cleanup();
    
    if (options.main_program && file_lock.locked) {
        retval = file_lock.unlock(LOCKFILE);
        if (retval) {
#ifdef _WIN32
            windows_error_string(buf, 256);
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

static void exit_from_timer_thread(int status) {
#ifdef DEBUG_BOINC_API
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
    // but on Unix there are synchronization problems;
    // set a flag telling the worker thread to exit
    //
    worker_thread_exit_status = status;
    worker_thread_exit_flag = true;
    pthread_exit(NULL);
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

int boinc_report_app_status_aux(
    double cpu_time,
    double checkpoint_cpu_time,
    double _fraction_done,
    int other_pid,
    double _bytes_sent,
    double _bytes_received
) {
    char msg_buf[MSG_CHANNEL_SIZE], buf[256];
    if (standalone) return 0;

    sprintf(msg_buf,
        "<current_cpu_time>%e</current_cpu_time>\n"
        "<checkpoint_cpu_time>%e</checkpoint_cpu_time>\n"
        "<fraction_done>%e</fraction_done>\n",
        cpu_time,
        checkpoint_cpu_time,
        _fraction_done
    );
    if (other_pid) {
        sprintf(buf, "<other_pid>%d</other_pid>\n", other_pid);
        strcat(msg_buf, buf);
    }
    if (_bytes_sent) {
        sprintf(buf, "<bytes_sent>%f</bytes_sent>\n", _bytes_sent);
        strcat(msg_buf, buf);
    }
    if (_bytes_received) {
        sprintf(buf, "<bytes_received>%f</bytes_received>\n", _bytes_received);
        strcat(msg_buf, buf);
    }
    if (app_client_shm->shm->app_status.send_msg(msg_buf)) {
        return 0;
    }
    return ERR_WRITE;
}

int boinc_report_app_status(
    double cpu_time,
    double checkpoint_cpu_time,
    double _fraction_done
){
    return boinc_report_app_status_aux(
        cpu_time, checkpoint_cpu_time, _fraction_done, 0, 0, 0
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

int suspend_activities() {
#ifdef _WIN32
    static DWORD pid;
    if (!pid) pid = GetCurrentProcessId();
    if (options.multi_thread) {
        suspend_or_resume_threads(pid, timer_thread_id, false);
    } else {
        SuspendThread(worker_thread_handle);
    }
#else
    // don't need to do anything in single-process case;
    // suspension is done by signal handler in worker thread
    //
    if (options.multi_process) {
        suspend_or_resume_descendants(0, false);
    }
#endif
    return 0;
}

int resume_activities() {
#ifdef _WIN32
    static DWORD pid;
    if (!pid) pid = GetCurrentProcessId();
    if (options.multi_thread) {
        suspend_or_resume_threads(pid, timer_thread_id, true);
    } else {
        ResumeThread(worker_thread_handle);
    }
#else
    if (options.multi_process) {
        suspend_or_resume_descendants(0, true);
    }
#endif
    return 0;
}

static void handle_upload_file_status() {
    char path[MAXPATHLEN], buf[256], log_name[256], *p, log_buf[256];
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
            fprintf(stderr,
                "%s handle_file_upload_status: can't open %s\n",
                boinc_msg_prefix(buf, sizeof(buf)), filename.c_str()
            );
            continue;
        }
        p = fgets(buf, 256, f);
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

// handle trickle and file upload messages
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

// This flag is set of we get a suspend request while in a critical section,
// and options.direct_process_action is set.
// As soon as we're not in the critical section we'll do the suspend.
//
static bool suspend_request = false;

// runs in timer thread
//
static void handle_process_control_msg() {
    char buf[MSG_CHANNEL_SIZE];
    if (app_client_shm->shm->process_control_request.get_msg(buf)) {
#ifdef DEBUG_BOINC_API
        char log_buf[256]
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
                    suspend_activities();
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
                exit_from_timer_thread(EXIT_ABORTED_BY_CLIENT);
            }
        }
        if (match_tag(buf, "<reread_app_info/>")) {
            boinc_status.reread_init_data_file = true;
        }
        if (match_tag(buf, "<network_available/>")) {
            have_network = 1;
        }
    }

    // if we got a suspend/quit/abort msg while in critical section,
    // and we've left the critical section, suspend/quit/abort now
    //
    if (options.direct_process_action && !in_critical_section) {
        if (boinc_status.quit_request) {
            exit_from_timer_thread(0);
        }
        if (boinc_status.abort_request) {
            exit_from_timer_thread(EXIT_ABORTED_BY_CLIENT);
        }
        if (suspend_request && !boinc_status.suspended) {
            boinc_status.suspended = true;
            suspend_activities();
        }
        suspend_request = false;
    }
}

// The following is used by V6 apps so that graphics
// will work with pre-V6 clients.
// If we get a graphics message, run/kill the (separate) graphics app
//
//
struct GRAPHICS_APP {
    bool fullscreen;
#ifdef _WIN32
    HANDLE pid;
#else
    int pid;
#endif
    GRAPHICS_APP(bool f) {fullscreen=f;}
    void run(char* path) {
        int argc;
        char* argv[4];
        char abspath[MAXPATHLEN];
#ifdef _WIN32
        GetFullPathName(path, MAXPATHLEN, abspath, NULL);
#else
        strcpy(abspath, path);
#endif
        argv[0] = const_cast<char*>(GRAPHICS_APP_FILENAME);
        if (fullscreen) {
            argv[1] = const_cast<char*>("--fullscreen");
            argv[2] = 0;
            argc = 2;
        } else {
            argv[1] = 0;
            argc = 1;
        }
        int retval = run_program(0, abspath, argc, argv, 0, pid);
        if (retval) {
            pid = 0;
        }
    }
    bool is_running() {
        if (pid && process_exists(pid)) return true;
        pid = 0;
        return false;
    }
    void kill() {
        if (pid) {
            kill_program(pid);
            pid = 0;
        }
    }
};

static GRAPHICS_APP ga_win(false), ga_full(true);
static bool have_graphics_app;

// The following is for backwards compatibility with version 5 clients.
//
static inline void handle_graphics_messages() {
    static char graphics_app_path[MAXPATHLEN];
    char buf[MSG_CHANNEL_SIZE];
    GRAPHICS_MSG m;
    static bool first=true;
    if (first) {
        first = false;
        boinc_resolve_filename(
            GRAPHICS_APP_FILENAME, graphics_app_path,
            sizeof(graphics_app_path)
        );
        // if the above returns "graphics_app", there was no link file,
        // so there's no graphics app
        //
        if (!strcmp(graphics_app_path, GRAPHICS_APP_FILENAME)) {
            have_graphics_app = false;
        } else {
            have_graphics_app = true;
            app_client_shm->shm->graphics_reply.send_msg(
                xml_graphics_modes[MODE_HIDE_GRAPHICS]
            );
        }
    }

    if (!have_graphics_app) return;

    if (app_client_shm->shm->graphics_request.get_msg(buf)) {
        app_client_shm->decode_graphics_msg(buf, m);
        switch (m.mode) {
        case MODE_HIDE_GRAPHICS:
            if (ga_full.is_running()) {
                ga_full.kill();
            } else if (ga_win.is_running()) {
                ga_win.kill();
            }
            break;
        case MODE_WINDOW:
            if (!ga_win.is_running()) ga_win.run(graphics_app_path);
            break;
        case MODE_FULLSCREEN:
            if (!ga_full.is_running()) ga_full.run(graphics_app_path);
            break;
        case MODE_BLANKSCREEN:
            // we can't actually blank the screen; just kill the app
            //
            if (ga_full.is_running()) {
                ga_full.kill();
            }
            break;
        }
        app_client_shm->shm->graphics_reply.send_msg(
            xml_graphics_modes[m.mode]
        );
    }
}

static void graphics_cleanup() {
    if (!have_graphics_app) return;
    if (ga_full.is_running()) ga_full.kill();
    if (ga_win.is_running()) ga_win.kill();
}

// timer handler; runs in the timer thread
//
static void timer_handler() {
    char buf[512];
    if (g_sleep) return;
    interrupt_count++;
    if (!boinc_status.suspended) {
        running_interrupt_count++;
    }

#ifdef DEBUG_BOINC_API
    if (in_critical_section) {
        fprintf(stderr,
            "%s: timer_handler(): in critical section\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
    }
#endif

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
        handle_graphics_messages();
    }

    if (interrupt_count % TIMERS_PER_SEC) return;

#ifdef DEBUG_BOINC_API
    fprintf(stderr, "%s 1 sec elapsed\n", boinc_msg_prefix(buf, sizeof(buf)));
#endif

    // here if we're at a one-second boundary; do slow stuff
    //

    if (!ready_to_checkpoint) {
        time_until_checkpoint -= 1;
        if (time_until_checkpoint <= 0) {
            ready_to_checkpoint = true;
        }
    }

    // see if the core client has died, which means we need to die too
    // (unless we're in a critical section)
    //
    if (in_critical_section==0 && options.check_heartbeat) {
        if (heartbeat_giveup_time < interrupt_count) {
            boinc_msg_prefix(buf, sizeof(buf));
            fputs(buf, stderr);
            fputs(" No heartbeat from core client for 30 sec - exiting\n", stderr);
            if (options.direct_process_action) {
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
    
    // If running under V5 client, notify the client if the graphics app exits
    // (e.g., if user clicked in the graphics window's close box.)
    //
    if (ga_win.pid) {
        if (!ga_win.is_running()) {
            app_client_shm->shm->graphics_reply.send_msg(
                xml_graphics_modes[MODE_HIDE_GRAPHICS]
            );
        }
    }
    
    if (options.handle_trickle_ups) {
        send_trickle_up_msg();
    }
    if (timer_callback) {
        timer_callback();
    }

    // send graphics-related messages
    //
    if (send_web_graphics_url && !app_client_shm->shm->graphics_reply.has_msg()) {
        sprintf(buf,
            "<web_graphics_url>%s</web_graphics_url>",
            web_graphics_url
        );
        app_client_shm->shm->graphics_reply.send_msg(buf);
        send_web_graphics_url = false;
    }
    if (send_remote_desktop_addr && !app_client_shm->shm->graphics_reply.has_msg()) {
        sprintf(buf,
            "<remote_desktop_addr>%s</remote_desktop_addr>",
            remote_desktop_addr
        );
        app_client_shm->shm->graphics_reply.send_msg(buf);
        send_remote_desktop_addr = false;
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
#ifndef GETRUSAGE_IN_TIMER_THREAD
    getrusage(RUSAGE_SELF, &worker_thread_ru);
#endif
    if (worker_thread_exit_flag) {
        boinc_exit(worker_thread_exit_status);
    }
    if (options.direct_process_action) {
        while (boinc_status.suspended && in_critical_section==0) {
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
// set up a periodic SIGALRM, to be handled by the worker thread
//
static int start_worker_signals() {
    int retval;
    struct sigaction sa;
    itimerval value;
    sa.sa_handler = worker_signal_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    retval = sigaction(SIGALRM, &sa, NULL);
    if (retval) {
        perror("boinc start_timer_thread() sigaction");
        return retval;
    }
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = (int)(TIMER_PERIOD*1e6);
    value.it_interval = value.it_value;
    retval = setitimer(ITIMER_REAL, &value, NULL);
    if (retval) {
        perror("boinc start_timer_thread() setitimer");
        return retval;
    }
    return 0;
}
#endif

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
    in_critical_section++;
}

void boinc_end_critical_section() {
    in_critical_section--;
    if (in_critical_section < 0) {
        in_critical_section = 0;        // just in case
    }
}

int boinc_fraction_done(double x) {
    fraction_done = x;
    return 0;
}

int boinc_receive_trickle_down(char* buf, int len) {
    std::string filename;
    char path[MAXPATHLEN];

    if (!options.handle_trickle_downs) return false;

    if (have_trickle_down) {
        relative_to_absolute("", path);
        DirScanner dirscan(path);
        while (dirscan.scan(filename)) {
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
    strcpy(web_graphics_url, url);
    send_web_graphics_url = true;
}

void boinc_remote_desktop_addr(char* addr) {
    if (standalone) return;
    strcpy(remote_desktop_addr, addr);
    send_remote_desktop_addr = true;
}

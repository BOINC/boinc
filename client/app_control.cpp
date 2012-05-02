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

// monitoring and process control of running apps

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0x0                 // may be in ntstatus.h
#endif
#ifndef STATUS_DLL_INIT_FAILED
#define STATUS_DLL_INIT_FAILED 0xC0000142  // may be in ntstatus.h
#endif

#else
#include "config.h"
#include <string>
#include <unistd.h>

#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_CSIGNAL
#include <csignal>
#elif HAVE_SYS_SIGNAL_H
#include <sys/signal.h>
#elif HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <vector>

#endif

using std::vector;

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "shmem.h"
#include "str_util.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "proc_control.h"
#include "result.h"
#include "sandbox.h"

#include "app.h"

// Do periodic checks on running apps:
// - get latest CPU time and % done info
// - check if any has exited, and clean up
// - see if any has exceeded its CPU or disk space limits, and abort it
//
bool ACTIVE_TASK_SET::poll() {
    bool action;
    unsigned int i;
    static double last_time = 0;
    if (gstate.now - last_time < TASK_POLL_PERIOD) return false;
    last_time = gstate.now;

    action = check_app_exited();
    send_heartbeats();
    send_trickle_downs();
    process_control_poll();
    action |= check_rsc_limits_exceeded();
    get_msgs();
    for (i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->task_state() == PROCESS_ABORT_PENDING) {
            if (gstate.now > atp->abort_time + ABORT_TIMEOUT) {
                atp->kill_task(false);
            }
        }
        if (atp->task_state() == PROCESS_QUIT_PENDING) {
            if (gstate.now > atp->quit_time + QUIT_TIMEOUT) {
                atp->kill_task(true);
            }
        }
    }

    if (action) {
        gstate.set_client_state_dirty("ACTIVE_TASK_SET::poll");
    }

    return action;
}

#if 0
// deprecated; TerminateProcessById() doesn't work if
// the process is running as a different user
//
#ifdef _WIN32
bool ACTIVE_TASK::kill_all_children() {
    unsigned int i,j;
    std::vector<PROCINFO> ps;
    std::vector<PROCINFO> tps;

    procinfo_setup(ps);

    PROCINFO pi;
    pi.id = pid;
    tps.push_back(pi);

    for (i=0; i < tps.size(); i++) {
        PROCINFO tp = tps[i];
        for (j=0; j < ps.size(); j++) {
            PROCINFO p = ps[j];
            if (tp.id == p.parentid) {
                if (TerminateProcessById(p.id)) {
                    tps.push_back(p);
                }
            }
        }
    }
    return true;
}
#endif
#endif

// Send a quit message, start timer, get descendants
//
int ACTIVE_TASK::request_exit() {
    if (app_client_shm.shm) {
        process_control_queue.msg_queue_send(
            "<quit/>",
            app_client_shm.shm->process_control_request
        );
    }
    set_task_state(PROCESS_QUIT_PENDING, "request_exit()");
    quit_time = gstate.now;
    get_descendants(pid, descendants);
    return 0;
}

// Send an abort message, start timer, get descendants
//
int ACTIVE_TASK::request_abort() {
    if (app_client_shm.shm) {
        process_control_queue.msg_queue_send(
            "<abort/>",
            app_client_shm.shm->process_control_request
        );
    }
    set_task_state(PROCESS_ABORT_PENDING, "request_abort");
    abort_time = gstate.now;
    get_descendants(pid, descendants);
    return 0;
}

#ifdef _WIN32
static void kill_app_process(int pid, bool will_restart) {
    HANDLE h = OpenProcess(READ_CONTROL | PROCESS_TERMINATE, false, pid);
    if (h == NULL) return;
    TerminateProcess(h, will_restart?0:EXIT_ABORTED_BY_CLIENT);
    CloseHandle(h);
#else
static void kill_app_process(int pid, bool) {
#ifdef SANDBOX
    kill_via_switcher(pid);
#endif
    kill(pid, SIGKILL);
#endif
}

static inline void kill_processes(vector<int> pids, bool will_restart) {
    for (unsigned int i=0; i<pids.size(); i++) {
        kill_app_process(pids[i], will_restart);
    }
}

// Kill the task (and descendants) by OS-specific means.
//
int ACTIVE_TASK::kill_task(bool will_restart) {
    vector<int>pids;
#ifdef _WIN32
    // On Win, in protected mode we won't be able to get
    // handles for the descendant processes;
    // all we can do is terminate the main process,
    // using the handle we got when we created it.
    //
    if (g_use_sandbox) {
        TerminateProcess(process_handle, will_restart?0:EXIT_ABORTED_BY_CLIENT);
        return 0;
    }
#endif
    get_descendants(pid, pids);
    pids.push_back(pid);
    for (unsigned int i=0; i<other_pids.size(); i++) {
        pids.push_back(other_pids[i]);
    }
    kill_processes(pids, will_restart);
    return 0;
}

// We have sent a quit request to the process; see if it's exited.
// This is called when the core client exits,
// or when a project is detached or reset
//
bool ACTIVE_TASK::has_task_exited() {
    bool exited = false;

    if (!process_exists()) return true;

#ifdef _WIN32
    unsigned long exit_code;
    if (GetExitCodeProcess(process_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            exited = true;
        }
    }
#else
    if (waitpid(pid, 0, WNOHANG) == pid) {
        exited = true;
    }
#endif
    if (exited) {
        set_task_state(PROCESS_EXITED, "has_task_exited");
        cleanup_task();
    }
    return exited;
}


static void limbo_message(ACTIVE_TASK& at) {
#ifdef _WIN32
    if (at.result->exit_status == STATUS_DLL_INIT_FAILED) {
        msg_printf(at.result->project, MSG_INFO,
            "Task %s exited with a DLL initialization error.",
            at.result->name
        );
        msg_printf(at.result->project, MSG_INFO,
            "If this happens repeatedly you may need to reboot your computer."
        );
        return;
    }
#endif
    msg_printf(at.result->project, MSG_INFO,
        "Task %s exited with zero status but no 'finished' file",
        at.result->name
    );
    msg_printf(at.result->project, MSG_INFO,
        "If this happens repeatedly you may need to reset the project."
    );
}

// the job just exited.  If it's a GPU job,
// clear the "schedule_backoff" field of all other jobs
// that use the GPU type, in case they're waiting for GPU RAM
//
static void clear_schedule_backoffs(ACTIVE_TASK* atp) {
    int rt = atp->result->avp->rsc_type();
    if (rt == RSC_TYPE_CPU) return;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->avp->rsc_type() == rt) {
            rp->schedule_backoff = 0;
        }
    }
}

// handle a task that exited prematurely (i.e. no finish file)
//
void ACTIVE_TASK::handle_premature_exit(bool& will_restart) {
    // keep count of premature exits;
    // if this happens 100 times w/o a checkpoint, abort job
    //
    premature_exit_count++;
    if (premature_exit_count > 100) {
        will_restart = false;
        set_task_state(PROCESS_ABORTED, "handle_premature_exit");
        result->exit_status = ERR_TOO_MANY_EXITS;
        gstate.report_result_error(*result, "too many exit(0)s");
        result->set_state(RESULT_ABORTED, "handle_premature_exit");
    } else {
        will_restart = true;
        limbo_message(*this);
        set_task_state(PROCESS_UNINITIALIZED, "handle_premature_exit");
    }
}

// handle a temporary exit
//
void ACTIVE_TASK::handle_temporary_exit(
    bool& will_restart, double backoff, const char* reason
) {
    premature_exit_count++;
    if (premature_exit_count > 100) {
        will_restart = false;
        set_task_state(PROCESS_ABORTED, "handle_temporary_exit");
        result->exit_status = ERR_TOO_MANY_EXITS;
        gstate.report_result_error(*result, "too many boinc_temporary_exit()s");
        result->set_state(RESULT_ABORTED, "handle_temporary_exit");
    } else {
        if (log_flags.task_debug) {
            msg_printf(result->project, MSG_INFO,
                "[task] task called temporary_exit(%f, %s)", backoff, reason
            );
        }
        will_restart = true;
        result->schedule_backoff = gstate.now + backoff;
        strcpy(result->schedule_backoff_reason, reason);
        set_task_state(PROCESS_UNINITIALIZED, "handle_temporary_exit");
    }
}

// deal with a process that has exited, for whatever reason:
// - completion
// - crash
// - quit or abort message sent by client
// - killed by client
//
#ifdef _WIN32
void ACTIVE_TASK::handle_exited_app(unsigned long exit_code) {
    if (log_flags.task_debug) {
        msg_printf(result->project, MSG_INFO,
            "[task] Process for %s exited, exit code %lu, task state %d",
            result->name, exit_code, task_state()
        );
    }
#else
void ACTIVE_TASK::handle_exited_app(int stat) {
    if (log_flags.task_debug) {
        msg_printf(result->project, MSG_INFO,
            "[task] Process for %s exited, status %d, task state %d",
            result->name, stat, task_state()
        );
    }
#endif
    bool will_restart = false;

    get_app_status_msg();
    get_trickle_up_msg();
    result->final_cpu_time = current_cpu_time;
    result->final_elapsed_time = elapsed_time;

    // if an abort or quit is pending,
    // the process may have exited itself, or we may have killed it.
    // Ignore exit status.
    //
    if (task_state() == PROCESS_ABORT_PENDING) {
        set_task_state(PROCESS_ABORTED, "handle_exited_app");
        kill_processes(descendants, false);
    } else if (task_state() == PROCESS_QUIT_PENDING) {
        set_task_state(PROCESS_UNINITIALIZED, "handle_exited_app");
        kill_processes(descendants, true);
        will_restart = true;
    } else {
#ifdef _WIN32
        result->exit_status = exit_code;
        switch(exit_code) {
        case STATUS_SUCCESS:
            // if another process killed the app, it looks like exit(0).
            // So check for the finish file
            //
            if (finish_file_present()) {
                set_task_state(PROCESS_EXITED, "handle_exited_app");
                break;
            }
            double x;
            char buf[256];
            strcpy(buf, "");
            if (temporary_exit_file_present(x, buf)) {
                handle_temporary_exit(will_restart, x, buf);
            }
            handle_premature_exit(will_restart);
            break;
        case 0xc000013a:        // control-C??
        case 0x40010004:        // vista shutdown?? can someone explain this?
        case STATUS_DLL_INIT_FAILED:
            // This can happen because:
            // - The OS is shutting down, and attempting to start
            //   any new application fails automatically.
            // - The OS has run out of desktop heap
            // - (reportedly) The computer has just come out of hibernation
            //
            handle_premature_exit(will_restart);
            break;
        default:
            char szError[1024];
            set_task_state(PROCESS_EXITED, "handle_exited_app");
            gstate.report_result_error(
                *result,
                "%s - exit code %d (0x%x)",
                windows_format_error_string(exit_code, szError, sizeof(szError)),
                exit_code, exit_code
            );
            if (log_flags.task_debug) {
                msg_printf(result->project, MSG_INFO,
                    "[task] Process for %s exited",
                    result->name
                );
                msg_printf(result->project, MSG_INFO,
                    "[task] exit code %d (0x%x): %s",
                    exit_code, exit_code,
                    windows_format_error_string(exit_code, szError, sizeof(szError))
                );
            }
            break;
        }
#else
        if (WIFEXITED(stat)) {
            result->exit_status = WEXITSTATUS(stat);

            double x;
            char buf[256];
            if (temporary_exit_file_present(x, buf)) {
                if (log_flags.task_debug) {
                    msg_printf(result->project, MSG_INFO,
                        "[task] task called temporary_exit(%f)", x
                    );
                }
                set_task_state(PROCESS_UNINITIALIZED, "temporary exit");
                will_restart = true;
                result->schedule_backoff = gstate.now + x;
                strcpy(result->schedule_backoff_reason, buf);
            } else {
                if (log_flags.task_debug) {
                    msg_printf(result->project, MSG_INFO,
                        "[task] process exited with status %d\n",
                        result->exit_status
                    );
                }
                if (result->exit_status) {
                    set_task_state(PROCESS_EXITED, "handle_exited_app");
                    gstate.report_result_error(
                        *result,
                        "process exited with code %d (0x%x, %d)",
                        result->exit_status, result->exit_status,
                        (-1<<8)|result->exit_status
                    );
                } else {
                    if (finish_file_present()) {
                        set_task_state(PROCESS_EXITED, "handle_exited_app");
                    } else {
                        handle_premature_exit(will_restart);
                    }
                }
            }
        } else if (WIFSIGNALED(stat)) {
            int got_signal = WTERMSIG(stat);

            if (log_flags.task_debug) {
                msg_printf(result->project, MSG_INFO,
                    "[task] process got signal %d", got_signal
                );
            }

            // if the process was externally killed, let it restart.
            //
            switch (got_signal) {
            case SIGHUP:
            case SIGINT:
            case SIGQUIT:
            case SIGKILL:
            case SIGTERM:
            case SIGSTOP:
                will_restart = true;
                set_task_state(PROCESS_UNINITIALIZED, "handle_exited_app");
                break;
            default:
                result->exit_status = stat;
                set_task_state(PROCESS_WAS_SIGNALED, "handle_exited_app");
                signal = got_signal;
                gstate.report_result_error(
                    *result, "process got signal %d", signal
                );
            }
        } else {
            result->exit_status = EXIT_UNKNOWN;
            set_task_state(PROCESS_EXIT_UNKNOWN, "handle_exited_app");
            gstate.report_result_error(*result, "process exit, unknown");
            msg_printf(result->project, MSG_INTERNAL_ERROR,
                "process exited for unknown reason"
            );
        }
#endif
    }

    cleanup_task();

    if (config.exit_after_finish) {
        exit(0);
    }

    if (!will_restart) {
        copy_output_files();
        int retval = read_stderr_file();
        if (retval) {
            msg_printf(result->project, MSG_INTERNAL_ERROR,
                "read_stderr_file(): %s", boincerror(retval)
            );
        }
        client_clean_out_dir(slot_dir, "handle_exited_app()");
        clear_schedule_backoffs(this);
            // clear scheduling backoffs of jobs waiting for GPU
    }
    gstate.request_schedule_cpus("application exited");
    gstate.request_work_fetch("application exited");
}

bool ACTIVE_TASK::finish_file_present() {
    char path[256];
    sprintf(path, "%s/%s", slot_dir, BOINC_FINISH_CALLED_FILE);
    return (boinc_file_exists(path) != 0);
}

bool ACTIVE_TASK::temporary_exit_file_present(double& x, char* buf) {
    char path[256];
    sprintf(path, "%s/%s", slot_dir, TEMPORARY_EXIT_FILE);
    FILE* f = fopen(path, "r");
    if (!f) return false;
    strcpy(buf, "");
    int y;
    int n = fscanf(f, "%d", &y);
    if (n != 1 || y < 0 || y > 86400) {
        x = 300;
    } else {
        x = y;
    }
    fgets(buf, 256, f);     // read the \n
    fgets(buf, 256, f);
    strip_whitespace(buf);
    fclose(f);
    return true;
}

void ACTIVE_TASK_SET::send_trickle_downs() {
    unsigned int i;
    ACTIVE_TASK* atp;
    bool sent;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (atp->have_trickle_down) {
            if (!atp->app_client_shm.shm) continue;
            sent = atp->app_client_shm.shm->trickle_down.send_msg("<have_trickle_down/>\n");
            if (sent) atp->have_trickle_down = false;
        }
        if (atp->send_upload_file_status) {
            if (!atp->app_client_shm.shm) continue;
            sent = atp->app_client_shm.shm->trickle_down.send_msg("<upload_file_status/>\n");
            if (sent) atp->send_upload_file_status = false;
       }
    }
}

void ACTIVE_TASK_SET::send_heartbeats() {
    unsigned int i;
    ACTIVE_TASK* atp;
    char buf[1024];
    double ar = gstate.available_ram();

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (!atp->app_client_shm.shm) continue;
        snprintf(buf, sizeof(buf), "<heartbeat/>"
            "<wss>%e</wss>"
            "<max_wss>%e</max_wss>",
            atp->procinfo.working_set_size, ar
        );
        if (gstate.network_suspended) {
            strcat(buf, "<network_suspended/>");
        }
        bool sent = atp->app_client_shm.shm->heartbeat.send_msg(buf);
        if (log_flags.heartbeat_debug) {
            if (sent) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[heartbeat] Heartbeat sent to task %s",
                    atp->result->name
                );
            } else {
                msg_printf(atp->result->project, MSG_INFO,
                    "[heartbeat] Heartbeat to task %s failed, previous message unread",
                    atp->result->name
                );
            }
        }
    }
}

void ACTIVE_TASK_SET::process_control_poll() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (!atp->app_client_shm.shm) continue;

        // if app has had the same message in its send buffer for 180 sec,
        // assume it's hung and restart it
        //
        if (atp->process_control_queue.timeout(180)) {
            if (log_flags.task_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "Restarting %s - message timeout", atp->result->name
                );
            }
            atp->kill_task(true);
        } else {
            atp->process_control_queue.msg_queue_poll(
                atp->app_client_shm.shm->process_control_request
            );
        }
    }
}

// See if any processes have exited
//
bool ACTIVE_TASK_SET::check_app_exited() {
    ACTIVE_TASK* atp;
    bool found = false;

#ifdef _WIN32
    unsigned long exit_code;
    unsigned int i;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (GetExitCodeProcess(atp->process_handle, &exit_code)) {
            if (exit_code != STILL_ACTIVE) {
                found = true;
                atp->handle_exited_app(exit_code);
            }
        } else {
            if (log_flags.task_debug) {
                char errmsg[1024];
                msg_printf(atp->result->project, MSG_INFO,
                    "[task] task %s GetExitCodeProcess() failed - %s GLE %d (0x%x)",
                    atp->result->name,
                    windows_format_error_string(
                        GetLastError(), errmsg, sizeof(errmsg)
                    ),
                    GetLastError(), GetLastError()
                );
            }

            // The process doesn't seem to be there.
            // Mark task as aborted so we don't check it again.
            //
            atp->set_task_state(PROCESS_ABORTED, "check_app_exited");
        }
    }
#else
    int pid, stat;

    if ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        atp = lookup_pid(pid);
        if (!atp) {
            // if we're running benchmarks, exited process
            // is probably a benchmark process; don't show error
            //
            if (!gstate.are_cpu_benchmarks_running() && log_flags.task_debug) {
                msg_printf(NULL, MSG_INTERNAL_ERROR,
                    "Process %d not found\n", pid
                );
            }
            return false;
        }
        atp->handle_exited_app(stat);
        found = true;
    }
#endif

    return found;
}

// if an app has exceeded its maximum disk usage, abort it
//
bool ACTIVE_TASK::check_max_disk_exceeded() {
    double disk_usage;
    int retval;

    retval = current_disk_usage(disk_usage);
    if (retval) {
        msg_printf(this->wup->project, MSG_INTERNAL_ERROR,
            "Can't get task disk usage: %s", boincerror(retval)
        );
    } else {
        if (disk_usage > max_disk_usage) {
            msg_printf(
                result->project, MSG_INFO,
                "Aborting task %s: exceeded disk limit: %.2fMB > %.2fMB\n",
                result->name, disk_usage/MEGA, max_disk_usage/MEGA
            );
            abort_task(EXIT_DISK_LIMIT_EXCEEDED, "Maximum disk usage exceeded");
            return true;
        }
    }
    return false;
}

// Check if any of the active tasks have exceeded their
// resource limits on disk, CPU time or memory
//
bool ACTIVE_TASK_SET::check_rsc_limits_exceeded() {
    unsigned int i;
    ACTIVE_TASK *atp;
    static double last_disk_check_time = 0;
    bool do_disk_check = false;
    bool did_anything = false;

    double ram_left = gstate.available_ram();
    double max_ram = gstate.max_available_ram();

    // Some slot dirs have lots of files,
    // so only check every min(disk_interval, 300) secs
    //
    double min_interval = gstate.global_prefs.disk_interval;
    if (min_interval < 300) min_interval = 300;
    if (gstate.now > last_disk_check_time + min_interval) {
        do_disk_check = true;
    }
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->task_state() != PROCESS_EXECUTING) continue;
        if (!atp->result->non_cpu_intensive() && (atp->elapsed_time > atp->max_elapsed_time)) {
            msg_printf(atp->result->project, MSG_INFO,
                "Aborting task %s: exceeded elapsed time limit %.2f (%.2fG/%.2fG)",
                atp->result->name, atp->max_elapsed_time,
                atp->result->wup->rsc_fpops_bound/1e9,
                atp->result->avp->flops/1e9
            );
            atp->abort_task(EXIT_TIME_LIMIT_EXCEEDED, "Maximum elapsed time exceeded");
            did_anything = true;
            continue;
        }
        if (atp->procinfo.working_set_size_smoothed > max_ram) {
            msg_printf(atp->result->project, MSG_INFO,
                "Aborting task %s: exceeded memory limit %.2fMB > %.2fMB\n",
                atp->result->name,
                atp->procinfo.working_set_size_smoothed/MEGA, max_ram/MEGA
            );
            atp->abort_task(EXIT_MEM_LIMIT_EXCEEDED, "Maximum memory exceeded");
            did_anything = true;
            continue;
        }
        if (do_disk_check && atp->check_max_disk_exceeded()) {
            did_anything = true;
            continue;
        }
        ram_left -= atp->procinfo.working_set_size_smoothed;
    }
    if (ram_left < 0) {
        gstate.request_schedule_cpus("RAM usage limit exceeded");
    }
    if (do_disk_check) {
        last_disk_check_time = gstate.now;
    }
    return did_anything;
}

// If process is running, send it an "abort" message,
// Set a flag so that if it doesn't exit within 5 seconds,
// kill it by OS-specific mechanism (e.g. KILL signal).
// This is done when app has exceeded CPU, disk, or mem limits,
// or when the user has requested it.
// The task won't be restarted.
//
int ACTIVE_TASK::abort_task(int exit_status, const char* msg) {
    if (task_state() == PROCESS_EXECUTING || task_state() == PROCESS_SUSPENDED) {
        request_abort();
    } else {
        set_task_state(PROCESS_ABORTED, "abort_task");
    }
    result->exit_status = exit_status;
    gstate.report_result_error(*result, msg);
    result->set_state(RESULT_ABORTED, "abort_task");
    return 0;
}

// check for the stderr file, copy to result record
//
int ACTIVE_TASK::read_stderr_file() {
    char* buf1, *buf2;
    char path[256];

    // truncate stderr output to the last 63KB;
    // it's unlikely that more than that will be useful
    //
    int max_len = 63*1024;
    sprintf(path, "%s/%s", slot_dir, STDERR_FILE);
    if (!boinc_file_exists(path)) return 0;
    if (read_file_malloc(path, buf1, max_len, !config.stderr_head)) {
        return ERR_MALLOC;
    }

    // if it's a vbox app, check for string in stderr saying
    // the job failed because CPU VM extensions disabled
    //
    if (strstr(app_version->plan_class, "vbox")) {
        if (strstr(buf1, "ERR_CPU_VM_EXTENSIONS_DISABLED")) {
            msg_printf(0, MSG_INFO,
                "Vbox app stderr indicates CPU VM extensions disabled"
            );
            gstate.host_info.p_vm_extensions_disabled = true;
        }
    }

    buf2 = (char*)malloc(2*max_len);
    if (!buf2) {
        free(buf1);
        return ERR_MALLOC;
    }
    non_ascii_escape(buf1, buf2, 2*max_len);
    result->stderr_out += "<stderr_txt>\n";
    result->stderr_out += buf2;
    result->stderr_out += "\n</stderr_txt>\n";
    free(buf1);
    free(buf2);
    return 0;
}

// tell a running app to reread project preferences.
// This is called when project prefs change,
// or when a user file has finished downloading.
//
int ACTIVE_TASK::request_reread_prefs() {
    int retval;
    APP_INIT_DATA aid;

    link_user_files();

    init_app_init_data(aid);
    retval = write_app_init_file(aid);
    if (retval) return retval;
    graphics_request_queue.msg_queue_send(
        xml_graphics_modes[MODE_REREAD_PREFS],
        app_client_shm.shm->graphics_request
    );
    return 0;
}

// tell a running app to reread the app_info file
// (e.g. because proxy settings have changed: this is for F@h)
//
int ACTIVE_TASK::request_reread_app_info() {
    APP_INIT_DATA aid;
    init_app_init_data(aid);
    int retval = write_app_init_file(aid);
    if (retval) return retval;
    process_control_queue.msg_queue_send(
        "<reread_app_info/>",
        app_client_shm.shm->process_control_request
    );
    return 0;
}

// tell all running apps of a project to reread prefs
//
void ACTIVE_TASK_SET::request_reread_prefs(PROJECT* project) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->result->project != project) continue;
        if (!atp->process_exists()) continue;
        atp->request_reread_prefs();
    }
}

void ACTIVE_TASK_SET::request_reread_app_info() {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        atp->request_reread_app_info();
    }
}


// send quit message to all tasks in the project
// (or all tasks, if proj==0).
// If they don't exit in 5 seconds,
// send them a kill signal and wait up to 5 more seconds to exit.
// This is called when the core client exits,
// or when a project is detached or reset
//
int ACTIVE_TASK_SET::exit_tasks(PROJECT* proj) {
    if (log_flags.task_debug) {
        msg_printf(NULL, MSG_INFO, "[task_debug] requesting tasks to exit");
    }
    request_tasks_exit(proj);

    // Wait 15 seconds for them to exit normally; if they don't then kill them
    //
    if (wait_for_exit(MAX_EXIT_TIME, proj)) {
        if (log_flags.task_debug) {
            msg_printf(NULL, MSG_INFO,
                "[task_debug] all tasks haven't exited after %d sec; killing them",
                MAX_EXIT_TIME
            );
        }
        kill_tasks(proj);
        if (wait_for_exit(5, proj)) {
            if (log_flags.task_debug) {
                msg_printf(NULL, MSG_INFO,
                    "[task_debug] tasks still not exited after 5 secs; giving up"
                );
            }
        } else {
            if (log_flags.task_debug) {
                msg_printf(NULL, MSG_INFO, "[task_debug] all tasks exited");
            }
        }
    } else {
        if (log_flags.task_debug) {
            msg_printf(NULL, MSG_INFO, "[task_debug] all tasks exited");
        }
    }

    // get final checkpoint_cpu_times
    //
    get_msgs();

    gstate.request_schedule_cpus("exit_tasks");
    return 0;
}

// Wait up to wait_time seconds for processes to exit
// If proj is zero, wait for all processes, else that project's
// NOTE: it's bad form to sleep, but it would be complex to avoid it here
//
int ACTIVE_TASK_SET::wait_for_exit(double wait_time, PROJECT* proj) {
    bool all_exited;
    unsigned int i,n;
    ACTIVE_TASK *atp;

    for (i=0; i<10; i++) {
        all_exited = true;

        for (n=0; n<active_tasks.size(); n++) {
            atp = active_tasks[n];
            if (proj && atp->wup->project != proj) continue;
            if (!atp->has_task_exited()) {
                all_exited = false;
                break;
            }
        }

        if (all_exited) return 0;
        boinc_sleep(wait_time/10.0);
    }

    return ERR_NOT_EXITED;
}

int ACTIVE_TASK_SET::abort_project(PROJECT* project) {
    vector<ACTIVE_TASK*>::iterator task_iter;
    ACTIVE_TASK* atp;

    exit_tasks(project);
    task_iter = active_tasks.begin();
    while (task_iter != active_tasks.end()) {
        atp = *task_iter;
        if (atp->result->project == project) {
            task_iter = active_tasks.erase(task_iter);
            delete atp;
        } else {
            task_iter++;
        }
    }
    return 0;
}

// suspend all currently running tasks
// called only from CLIENT_STATE::suspend_tasks(),
// e.g. because on batteries, time of day, benchmarking, CPU throttle, etc.
//
void ACTIVE_TASK_SET::suspend_all(int reason) {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->task_state() != PROCESS_EXECUTING) continue;
        switch (reason) {
        case SUSPEND_REASON_CPU_THROTTLE:
            // if we're doing CPU throttling, don't bother suspending apps
            // that don't use a full CPU
            //
            if (atp->result->dont_throttle()) continue;
            if (atp->app_version->avg_ncpus < 1) continue;
            atp->preempt(REMOVE_NEVER);
            break;
        case SUSPEND_REASON_BENCHMARKS:
            atp->preempt(REMOVE_NEVER);
            break;
        case SUSPEND_REASON_CPU_USAGE:
            // If we're suspending because of non-BOINC CPU load,
            // don't remove from memory.
            // Some systems do a security check when apps are launched,
            // which uses a lot of CPU.
            // Avoid going into a preemption loop.
            //
            if (atp->result->non_cpu_intensive()) break;
            atp->preempt(REMOVE_NEVER);
            break;
        default:
            atp->preempt(REMOVE_MAYBE_USER);
        }
    }
}

// resume all currently scheduled tasks
//
void ACTIVE_TASK_SET::unsuspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        if (atp->task_state() == PROCESS_UNINITIALIZED) {
            if (atp->start()) {
                msg_printf(atp->wup->project, MSG_INTERNAL_ERROR,
                    "Couldn't restart task %s", atp->result->name
                );
            }
        } else if (atp->task_state() == PROCESS_SUSPENDED) {
            atp->unsuspend();
        }
    }
}

// Check to see if any tasks are running
// called if benchmarking and waiting for suspends to happen
// or the system needs to suspend itself so we are suspending
// the applications
//
bool ACTIVE_TASK_SET::is_task_executing() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->task_state() == PROCESS_EXECUTING) {
            return true;
        }
    }
    return false;
}

// Send quit message to all app processes
// This is called when the core client exits,
// or when a project is detached or reset
//
void ACTIVE_TASK_SET::request_tasks_exit(PROJECT* proj) {
    unsigned int i;
    ACTIVE_TASK *atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (proj && atp->wup->project != proj) continue;
        if (!atp->process_exists()) continue;
        atp->request_exit();
    }
}

// Send kill signal to all app processes
// Don't wait for them to exit
//
void ACTIVE_TASK_SET::kill_tasks(PROJECT* proj) {
    unsigned int i;
    ACTIVE_TASK *atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (proj && atp->wup->project != proj) continue;
        if (!atp->process_exists()) continue;
        atp->kill_task(true);
    }
}

// send a <suspend> message
//
int ACTIVE_TASK::suspend() {
    if (!app_client_shm.shm) return 0;
    if (task_state() != PROCESS_EXECUTING) {
        msg_printf(result->project, MSG_INTERNAL_ERROR,
            "ACTIVE_TASK::SUSPEND(): expected task %s to be executing",
            result->name
        );
    }
    int n = process_control_queue.msg_queue_purge("<resume/>");
    if (n == 0) {
        process_control_queue.msg_queue_send(
            "<suspend/>",
            app_client_shm.shm->process_control_request
        );
    }
    set_task_state(PROCESS_SUSPENDED, "suspend");
    return 0;
}

// resume a suspended task
//
int ACTIVE_TASK::unsuspend() {
    if (!app_client_shm.shm) return 0;
    if (task_state() != PROCESS_SUSPENDED) {
        msg_printf(result->project, MSG_INFO,
            "Internal error: expected process %s to be suspended", result->name
        );
    }
    if (log_flags.cpu_sched) {
        msg_printf(result->project, MSG_INFO,
            "[cpu_sched] Resuming %s", result->name
        );
    }
    int n = process_control_queue.msg_queue_purge("<suspend/>");
    if (n == 0) {
        process_control_queue.msg_queue_send(
            "<resume/>",
            app_client_shm.shm->process_control_request
        );
    }
    set_task_state(PROCESS_EXECUTING, "unsuspend");
    return 0;
}

void ACTIVE_TASK::send_network_available() {
    if (!app_client_shm.shm) return;
    process_control_queue.msg_queue_send(
        "<network_available/>",
        app_client_shm.shm->process_control_request
    );
    return;
}

// See if the app has placed a new message in shared mem
// (with CPU done, frac done etc.)
// If so parse it and return true.
//
bool ACTIVE_TASK::get_app_status_msg() {
    char msg_buf[MSG_CHANNEL_SIZE];
    double fd;
    int other_pid;
    double dtemp;

    if (!app_client_shm.shm) {
        msg_printf(result->project, MSG_INFO,
            "Task %s: no shared memory segment", result->name
        );
        return false;
    }
    if (!app_client_shm.shm->app_status.get_msg(msg_buf)) {
        return false;
    }
    if (log_flags.app_msg_receive) {
        msg_printf(this->wup->project, MSG_INFO,
            "[app_msg_receive] got msg from slot %d: %s", slot, msg_buf
        );
    }
    want_network = 0;
    current_cpu_time = checkpoint_cpu_time = 0.0;
    if (parse_double(msg_buf, "<fraction_done>", fd)) {
        // fraction_done will be reported as zero
        // until the app's first call to boinc_fraction_done().
        // So ignore zeros.
        //
        if (fd) {
            fraction_done = fd;
            fraction_done_elapsed_time = elapsed_time;
        }
    }
    parse_double(msg_buf, "<current_cpu_time>", current_cpu_time);
    parse_double(msg_buf, "<checkpoint_cpu_time>", checkpoint_cpu_time);
    parse_double(msg_buf, "<fpops_per_cpu_sec>", result->fpops_per_cpu_sec);
    parse_double(msg_buf, "<fpops_cumulative>", result->fpops_cumulative);
    parse_double(msg_buf, "<intops_per_cpu_sec>", result->intops_per_cpu_sec);
    parse_double(msg_buf, "<intops_cumulative>", result->intops_cumulative);
    if (parse_double(msg_buf, "<bytes_sent>", dtemp)) {
        if (dtemp > bytes_sent) {
            daily_xfer_history.add(dtemp - bytes_sent, true);
        }
        bytes_sent = dtemp;
    }
    if (parse_double(msg_buf, "<bytes_received>", dtemp)) {
        if (dtemp > bytes_received) {
            daily_xfer_history.add(dtemp - bytes_received, false);
        }
        bytes_received = dtemp;
    }
    parse_int(msg_buf, "<want_network>", want_network);
    if (parse_int(msg_buf, "<other_pid>", other_pid)) {
        // for now, we handle only one of these
        other_pids.clear();
        other_pids.push_back(other_pid);
    }
    if (current_cpu_time < 0) {
        msg_printf(result->project, MSG_INFO,
            "app reporting negative CPU: %f", current_cpu_time
        );
        current_cpu_time = 0;
    }
    if (checkpoint_cpu_time < 0) {
        msg_printf(result->project, MSG_INFO,
            "app reporting negative checkpoint CPU: %f", checkpoint_cpu_time
        );
        checkpoint_cpu_time = 0;
    }
    return true;
}

void ACTIVE_TASK::get_graphics_msg() {
    char msg_buf[MSG_CHANNEL_SIZE];

    if (!app_client_shm.shm) return;
    if (app_client_shm.shm->graphics_reply.get_msg(msg_buf)) {
        if (log_flags.app_msg_receive) {
            msg_printf(this->wup->project, MSG_INFO,
                "[app_msg_receive] got msg from slot %d: %s", slot, msg_buf
            );
        }

        parse_str(msg_buf, "<web_graphics_url>", web_graphics_url, sizeof(web_graphics_url));
        parse_str(msg_buf, "<remote_desktop_addr>", remote_desktop_addr, sizeof(remote_desktop_addr));
    }
}

bool ACTIVE_TASK::get_trickle_up_msg() {
    char msg_buf[MSG_CHANNEL_SIZE];
    bool found = false;
    int retval;

    if (!app_client_shm.shm) return false;
    if (app_client_shm.shm->trickle_up.get_msg(msg_buf)) {
        if (match_tag(msg_buf, "<have_new_trickle_up/>")) {
            if (log_flags.app_msg_receive) {
                msg_printf(NULL, MSG_INFO,
                    "[app_msg_receive] got msg from slot %d: %s", slot, msg_buf
                );
            }
            retval = move_trickle_file();
            if (!retval) {
                wup->project->trickle_up_pending = true;
            }
        }
        if (match_tag(msg_buf, "<have_new_upload_file/>")) {
            if (log_flags.app_msg_receive) {
                msg_printf(NULL, MSG_INFO,
                    "[app_msg_receive] got msg from slot %d: %s", slot, msg_buf
                );
            }
            handle_upload_files();
        }
        found = true;
    }
    return found;
}

// check for msgs from active tasks,
// and update their elapsed time and other info
//
void ACTIVE_TASK_SET::get_msgs() {
    unsigned int i;
    ACTIVE_TASK *atp;
    double old_time;
    static double last_time=0;
    double delta_t;
    if (last_time) {
        delta_t = gstate.now - last_time;

        // Normally this is called every second.
        // If delta_t is > 10, we'll assume that a period of hibernation
        // or suspension happened, and treat it as zero.
        // If negative, must be clock reset.  Ignore.
        //
        if (delta_t > 10 || delta_t < 0) {
            delta_t = 0;
        }
    } else {
        delta_t = 0;
    }
    last_time = gstate.now;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        old_time = atp->checkpoint_cpu_time;
        if (atp->task_state() == PROCESS_EXECUTING) {
            atp->elapsed_time += delta_t;
        }
        if (atp->get_app_status_msg()) {
            if (old_time != atp->checkpoint_cpu_time) {
                char buf[256];
                sprintf(buf, "%s checkpointed", atp->result->name);
                if (atp->overdue_checkpoint) {
                    gstate.request_schedule_cpus(buf);
                }
                atp->checkpoint_wall_time = gstate.now;
                atp->premature_exit_count = 0;
                atp->checkpoint_elapsed_time = atp->elapsed_time;
                atp->checkpoint_fraction_done = atp->fraction_done;
                atp->checkpoint_fraction_done_elapsed_time = atp->fraction_done_elapsed_time;
                if (log_flags.checkpoint_debug) {
                    msg_printf(atp->wup->project, MSG_INFO,
                        "[checkpoint] result %s checkpointed",
                        atp->result->name
                    );
                } else if (log_flags.task_debug) {
                    msg_printf(atp->wup->project, MSG_INFO,
                        "[task] result %s checkpointed",
                        atp->result->name
                    );
                }
                atp->write_task_state_file();
            }
        }
        atp->get_trickle_up_msg();
        atp->get_graphics_msg();
    }
}

// write checkpoint state to a file in the slot dir
// (this avoids rewriting the state file on each checkpoint)
//
void ACTIVE_TASK::write_task_state_file() {
    char path[1024];
    sprintf(path, "%s/%s", slot_dir, TASK_STATE_FILENAME);
    FILE* f = fopen(path, "w");
    if (!f) return;
    fprintf(f,
        "<active_task>\n"
        "    <project_master_url>%s</project_master_url>\n"
        "    <result_name>%s</result_name>\n"
        "    <checkpoint_cpu_time>%f</checkpoint_cpu_time>\n"
        "    <checkpoint_elapsed_time>%f</checkpoint_elapsed_time>\n"
        "    <fraction_done>%f</fraction_done>\n"
        "</active_task>\n",
        result->project->master_url,
        result->name,
        checkpoint_cpu_time,
        checkpoint_elapsed_time,
        fraction_done
    );
    fclose(f);
}

// called on startup; read the task state file in case it's more recent
// then the main state file
//
void ACTIVE_TASK::read_task_state_file() {
    char buf[4096], path[1024], s[1024];
    sprintf(path, "%s/%s", slot_dir, TASK_STATE_FILENAME);
    FILE* f = fopen(path, "r");
    if (!f) return;
    buf[0] = 0;
    fread(buf, 1, 4096, f);
    fclose(f);
    buf[4095] = 0;
    double x;
    // sanity checks - project and result name must match
    //
    if (!parse_str(buf, "<project_master_url>", s, sizeof(s))) {
        msg_printf(wup->project, MSG_INTERNAL_ERROR,
            "no project URL in task state file"
        );
        return;
    }
    if (strcmp(s, result->project->master_url)) {
        msg_printf(wup->project, MSG_INTERNAL_ERROR,
            "wrong project URL in task state file"
        );
        return;
    }
    if (!parse_str(buf, "<result_name>", s, sizeof(s))) {
        msg_printf(wup->project, MSG_INTERNAL_ERROR,
            "no task name in task state file"
        );
        return;
    }
    if (strcmp(s, result->name)) {
        msg_printf(wup->project, MSG_INTERNAL_ERROR,
            "wrong task name in task state file"
        );
        return;
    }
    if (parse_double(buf, "<checkpoint_cpu_time>", x)) {
        if (x > checkpoint_cpu_time) {
            checkpoint_cpu_time = x;
        }
    }
    if (parse_double(buf, "<checkpoint_elapsed_time>", x)) {
        if (x > checkpoint_elapsed_time) {
            checkpoint_elapsed_time = x;
        }
    }
}


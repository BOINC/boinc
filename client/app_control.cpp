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
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                0x0         // may be in ntstatus.h
#endif
#ifndef STATUS_DLL_INIT_FAILED
#define STATUS_DLL_INIT_FAILED        0xC0000142  // may be in ntstatus.h
#endif
#ifndef STATUS_DLL_INIT_FAILED_LOGOFF
#define STATUS_DLL_INIT_FAILED_LOGOFF 0xC000026B  // may be in ntstatus.h
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
#include "str_replace.h"
#include "str_util.h"
#include "url.h"
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
    if (!gstate.clock_change && gstate.now - last_time < TASK_POLL_PERIOD) return false;
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
                if (log_flags.task_debug) {
                    msg_printf(atp->result->project, MSG_INFO,
                        "[task] abort request timed out, killing task %s",
                        atp->result->name
                    );
                }
                atp->kill_running_task(false);
            }
        }
        if (atp->task_state() == PROCESS_QUIT_PENDING) {
            if (gstate.now > atp->quit_time + QUIT_TIMEOUT) {
                if (log_flags.task_debug) {
                    msg_printf(atp->result->project, MSG_INFO,
                        "[task] quit request timed out, killing task %s",
                        atp->result->name
                    );
                }
                atp->kill_running_task(true);
            }
        }
    }

    // Check for finish files every 10 sec.
    // If we already found a finish file, abort the app;
    // it must be hung somewhere in boinc_finish();
    //
    static double last_finish_check_time = 0;
    if (gstate.clock_change || gstate.now - last_finish_check_time > 10) {
        last_finish_check_time = gstate.now;
        int exit_code;
        for (i=0; i<active_tasks.size(); i++) {
            ACTIVE_TASK* atp = active_tasks[i];
            if (atp->task_state() == PROCESS_UNINITIALIZED) continue;
            if (atp->finish_file_time) {
                if (gstate.now - atp->finish_file_time > FINISH_FILE_TIMEOUT) {
                    // process is still there 5 min after it wrote finish file.
                    // abort the job
                    // Note: actually we should treat it as successful.
                    // But this would be tricky.
                    //
                    atp->abort_task(EXIT_ABORTED_BY_CLIENT,
                        "Process still present 5 min after writing finish file; aborting"
                    );
                }
            } else if (atp->finish_file_present(exit_code)) {
                atp->finish_file_time = gstate.now;
            }
        }
    }

    // check if a job is "stuck" (did not make progress in the last hour)
    // notify the user about the issue
    // abort after some time
    static double last_stuck_check_time = 0;
    if (gstate.now - last_stuck_check_time > STUCK_CHECK_POLL_PERIOD) {
        last_stuck_check_time = gstate.now;
        for (i=0; i<active_tasks.size(); i++){
            ACTIVE_TASK* atp = active_tasks[i];
            if (atp->non_cpu_intensive()) continue;
            if (atp->sporadic()) continue;
            if (atp->stuck_check_elapsed_time == 0) {
                // first pass
                atp->stuck_check_elapsed_time = atp->elapsed_time;
                atp->stuck_check_fraction_done = atp->fraction_done;
                atp->stuck_check_cpu_time = atp->current_cpu_time;
                continue;
            }
            if (atp->elapsed_time < atp->stuck_check_elapsed_time + STUCK_CHECK_POLL_PERIOD) continue;
            if (atp->stuck_check_fraction_done == atp->fraction_done &&
                    (atp->current_cpu_time - atp->stuck_check_cpu_time) < 10) {
                // if fraction done does not change and cpu time is <10, message the user
                msg_printf(atp->result->project, MSG_USER_ALERT,
                    "[task] has not made progress in last hour, consider aborting task %s",
                    atp->result->name
                );
            }
            atp->stuck_check_elapsed_time = atp->elapsed_time;
            atp->stuck_check_fraction_done = atp->fraction_done;
            atp->stuck_check_cpu_time = atp->current_cpu_time;
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

static void print_descendants(int pid, const vector<int>& desc, const char* where) {
    msg_printf(0, MSG_INFO, "%s: PID %d has %d descendants",
        where, pid, (int)desc.size()
    );
    for (unsigned int i=0; i<desc.size(); i++) {
        msg_printf(0, MSG_INFO, "   PID %d", desc[i]);
    }
}

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
    if (log_flags.task_debug) {
        print_descendants(pid, descendants, "request_exit()");
    }
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
    if (log_flags.task_debug) {
        print_descendants(pid, descendants, "request_abort()");
    }
    return 0;
}

#ifdef _WIN32
static void kill_app_process(int pid, bool will_restart) {
    int retval = 0;
    retval = kill_process_with_status(pid, will_restart?0:EXIT_ABORTED_BY_CLIENT);
    if (retval && log_flags.task_debug) {
        msg_printf(0, MSG_INFO,
            "[task] kill_process_with_status(%d) failed: %s",
            pid, boincerror(retval)
        );
    }
}
#else
static void kill_app_process(int pid, bool) {
    int retval = 0;
    if (g_use_sandbox) {
        retval = kill_via_switcher(pid);
        if (retval && log_flags.task_debug) {
            msg_printf(0, MSG_INFO,
                "[task] kill_via_switcher(%d) failed: %s (%d)",
                pid,
                (retval>=0) ? strerror(errno) : boincerror(retval), retval
            );
        }
    } else {
        retval = kill_process(pid);
        if (retval && log_flags.task_debug) {
            msg_printf(0, MSG_INFO,
                "[task] kill_process(%d) failed: %s",
                pid, strerror(errno)
            );
        }
    }
}
#endif

// Kill a task whose main process is still running
// Just kill the main process; shared mem and subsidiary processes
// will be cleaned up after it exits, by cleanup_task();
//
int ACTIVE_TASK::kill_running_task(bool will_restart) {
    kill_app_process(pid, will_restart);
    return 0;
}

// Kill any remaining subsidiary processes
// of a task whose main process has exited,
// namely:
// - its descendants (as recently enumerated; it's too late to do that now)
//   This list will be populated only in the quit and abort cases.
// - its "other" processes, e.g. VMs
//
int ACTIVE_TASK::kill_subsidiary_processes() {
    unsigned int i;
    for (i=0; i<other_pids.size(); i++) {
        kill_app_process(other_pids[i], false);
    }
    for (i=0; i<descendants.size(); i++) {
        kill_app_process(descendants[i], false);
    }
    return 0;
}

// We have sent a quit request to the process; see if it's exited.
// This is called when the client exits,
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
    int rt = atp->result->resource_usage.rsc_type;
    if (rt == RSC_TYPE_CPU) return;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->resource_usage.rsc_type == rt) {
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
    bool& will_restart, double backoff, const char* reason, bool is_notice
) {
    premature_exit_count++;
    if (premature_exit_count > 100) {
        will_restart = false;
        set_task_state(PROCESS_ABORTED, "handle_temporary_exit");
        result->exit_status = ERR_TOO_MANY_EXITS;
        gstate.report_result_error(*result, "too many boinc_temporary_exit()s");
        result->set_state(RESULT_ABORTED, "handle_temporary_exit");
    } else {
        if (is_notice) {
            msg_printf(result->project, MSG_USER_ALERT,
                "Task %s postponed for %.f seconds: %s", result->name, backoff, reason
            );
        } else {
            if (log_flags.task) {
                msg_printf(result->project, MSG_INFO,
                    "Task %s postponed for %.f seconds: %s", result->name, backoff, reason
                );
            }
        }
        will_restart = true;
        result->schedule_backoff = gstate.now + backoff;
        safe_strcpy(result->schedule_backoff_reason, reason);
        set_task_state(PROCESS_UNINITIALIZED, "handle_temporary_exit");
    }
}

void ACTIVE_TASK::copy_final_info() {
    result->final_cpu_time = current_cpu_time;
    result->final_elapsed_time = elapsed_time;
    result->final_peak_working_set_size = peak_working_set_size;
    result->final_peak_swap_size = peak_swap_size;
    result->final_peak_disk_usage = peak_disk_usage;
    result->final_bytes_sent = bytes_sent;
    result->final_bytes_received = bytes_received;
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
    char err_msg[4096];
    bool will_restart = false;

    get_app_status_msg();
    get_trickle_up_msg();
    copy_final_info();

    // if an abort or quit is pending,
    // the process may have exited itself, or we may have killed it.
    // Ignore exit status.
    //
    if (task_state() == PROCESS_ABORT_PENDING) {
        set_task_state(PROCESS_ABORTED, "handle_exited_app");
    } else if (task_state() == PROCESS_QUIT_PENDING) {
        set_task_state(PROCESS_UNINITIALIZED, "handle_exited_app");
        will_restart = true;
    } else {
#ifdef _WIN32
        result->exit_status = exit_code;
        switch(exit_code) {
        case STATUS_SUCCESS:
            // if another process killed the app, it looks like exit(0).
            // So check for the finish file
            //
            int e;
            if (finish_file_present(e)) {
                set_task_state(PROCESS_EXITED, "handle_exited_app");
                break;
            }
            double x;
            bool is_notice;
            char buf[256];
            safe_strcpy(buf, "");
            if (temporary_exit_file_present(x, buf, is_notice)) {
                handle_temporary_exit(will_restart, x, buf, is_notice);
            } else {
                handle_premature_exit(will_restart);
            }
            break;
        case 0xc000013a:        // control-C??
        case 0x40010004:        // vista shutdown?? can someone explain this?
        case STATUS_DLL_INIT_FAILED:
		case STATUS_DLL_INIT_FAILED_LOGOFF:
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
            snprintf(err_msg, sizeof(err_msg),
                "%s - exit code %lu (0x%x)",
                windows_format_error_string(exit_code, szError, sizeof(szError)),
                exit_code, exit_code
            );
            gstate.report_result_error(*result, err_msg);
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
            bool is_notice;
            int e;
            if (temporary_exit_file_present(x, buf, is_notice)) {
                handle_temporary_exit(will_restart, x, buf, is_notice);
            } else {
                if (log_flags.task_debug) {
                    msg_printf(result->project, MSG_INFO,
                        "[task] process exited with status %d\n",
                        result->exit_status
                    );
                }
                if (result->exit_status) {
                    set_task_state(PROCESS_EXITED, "handle_exited_app");
                    snprintf(err_msg, sizeof(err_msg),
                        "process exited with code %d (0x%x, %d)",
                        result->exit_status, result->exit_status,
                        (~0xff)|result->exit_status
                    );
                    gstate.report_result_error(*result, err_msg);
                } else {
                    if (finish_file_present(e)) {
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
                snprintf(err_msg, sizeof(err_msg),
                    "process got signal %d", signal
                );
                gstate.report_result_error(*result, err_msg);
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

    // get rid of shared-mem segment and kill subsidiary processes
    //
    cleanup_task();

    if (!will_restart) {
        copy_output_files();
        int retval = read_stderr_file();
        if (retval) {
            msg_printf(result->project, MSG_INTERNAL_ERROR,
                "read_stderr_file(): %s", boincerror(retval)
            );
        }
        if (!wup->project->app_test) {
            client_clean_out_dir(slot_dir, "handle_exited_app()");
        }
        clear_schedule_backoffs(this);
            // clear scheduling backoffs of jobs waiting for GPU
    }
    gstate.request_schedule_cpus("application exited");
    gstate.request_work_fetch("application exited");
}

// structure of a finish file (see boinc_api.cpp)):
// exit status (int)
// optional:
//  message to show user
//  "notice" or blank line
//
bool ACTIVE_TASK::finish_file_present(int &exit_code) {
    char path[MAXPATHLEN], buf[1024], buf2[256];
    safe_strcpy(buf, "");
    safe_strcpy(buf2, "");

    exit_code = 0;

    snprintf(path, sizeof(path), "%s/%s", slot_dir, BOINC_FINISH_CALLED_FILE);
    FILE* f = boinc_fopen(path, "r");
    if (!f) return false;
    char* p = fgets(buf, sizeof(buf), f);
    if (p && strlen(buf)) {
        int e;
        if (sscanf(buf, "%d", &e) == 1) {
            exit_code = e;
        }
    }
    p = fgets(buf, sizeof(buf), f);
    if (p && strlen(buf)) {
        if (fgets(buf2, sizeof(buf2), f)) {
            msg_printf(result->project,
                strstr(buf2, "notice")?MSG_USER_ALERT:MSG_INFO,
                "Message from task: %s", buf
            );
        }
    }
    fclose(f);
    return true;
}

bool ACTIVE_TASK::temporary_exit_file_present(
    double& x, char* buf, bool& is_notice
) {
    char path[MAXPATHLEN], buf2[256];
    snprintf(path, sizeof(path), "%s/%s", slot_dir, TEMPORARY_EXIT_FILE);
    FILE* f = boinc_fopen(path, "r");
    if (!f) return false;
    strcpy(buf, "");
    int y;
    int n = fscanf(f, "%d", &y);
    if (n != 1 || y < 0 || y > 86400) {
        x = 300;
    } else {
        x = y;
    }
    char *p = fgets(buf, 256, f);     // read the \n
    if (p) {
        p = fgets(buf, 256, f);
    }
    if (p == NULL) {
        fclose(f);
        return false;
    }
    strip_whitespace(buf);
    is_notice = false;
    if (fgets(buf2, 256, f)) {
        if (strstr(buf2, "notice")) {
            is_notice = true;
        }
    }
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
            safe_strcat(buf, "<network_suspended/>");
        }
        if (atp->sporadic_ca_state != CA_NONE) {
            char buf2[256];
            sprintf(buf2, "<sporadic_ca>%d</sporadic_ca>",
                atp->sporadic_ca_state
            );
            safe_strcat(buf, buf2);
        }
        bool sent = atp->app_client_shm.shm->heartbeat.send_msg(buf);
        if (log_flags.heartbeat_debug) {
            if (sent) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[heartbeat] Heartbeat sent to task %s: %s",
                    atp->result->name, buf
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

// send queued process-control messages; check for timeout
//
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
            atp->kill_running_task(true);
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
            atp->cleanup_task();
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
            if (!gstate.benchmarks_running && log_flags.task_debug) {
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
            abort_task(EXIT_DISK_LIMIT_EXCEEDED, "Disk usage limit exceeded");
            return true;
        }
    }
    return false;
}

// Check if any of the active tasks have exceeded their
// resource limits on disk, CPU time or memory
//
// TODO: this gets called ever 1 sec,
// but mem and disk usage are computed less often.
// refactor.
//
bool ACTIVE_TASK_SET::check_rsc_limits_exceeded() {
    unsigned int i;
    ACTIVE_TASK *atp;
    static double last_disk_check_time = 0;
    bool do_disk_check = false;
    bool did_anything = false;
    char buf[256];

    double ram_left = gstate.available_ram();
    double max_ram = gstate.max_available_ram();

    // Some slot dirs have lots of files,
    // so only check every min(disk_interval, 300) secs
    //
    double min_interval = gstate.global_prefs.disk_interval;
    if (min_interval < 300) min_interval = 300;
    if (gstate.clock_change || gstate.now > last_disk_check_time + min_interval) {
        do_disk_check = true;
    }
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->task_state() != PROCESS_EXECUTING) continue;
        if (!atp->always_run() && (atp->elapsed_time > atp->max_elapsed_time)) {
            snprintf(buf, sizeof(buf), "exceeded elapsed time limit %.2f (%.2fG/%.2fG)",
                atp->max_elapsed_time,
                atp->result->wup->rsc_fpops_bound/1e9,
                atp->result->resource_usage.flops/1e9
            );
            msg_printf(atp->result->project, MSG_INFO,
                "Aborting task %s: %s", atp->result->name, buf
            );
            atp->abort_task(EXIT_TIME_LIMIT_EXCEEDED, buf);
            did_anything = true;
            continue;
        }
#if 0
        // removing this for now because most projects currently
        // have too-low values of workunit.rsc_memory_bound
        // (causing lots of aborts)
        // and I don't think we can expect projects to provide
        // accurate bounds.
        //
        if (atp->procinfo.working_set_size_smoothed > atp->max_mem_usage) {
            snprintf(buf, sizeof(buf), "working set size > workunit.rsc_memory_bound: %.2fMB > %.2fMB",
                atp->procinfo.working_set_size_smoothed/MEGA, atp->max_mem_usage/MEGA
            );
            msg_printf(atp->result->project, MSG_INFO,
                "Aborting task %s: %s",
                atp->result->name, buf
            );
            atp->abort_task(EXIT_MEM_LIMIT_EXCEEDED, buf);
            did_anything = true;
            continue;
        }
#endif
        if (atp->procinfo.working_set_size_smoothed > max_ram) {
            snprintf(buf, sizeof(buf), "working set size > client RAM limit: %.2fMB > %.2fMB",
                atp->procinfo.working_set_size_smoothed/MEGA, max_ram/MEGA
            );
            msg_printf(atp->result->project, MSG_INFO,
                "Aborting task %s: %s",
                atp->result->name, buf
            );
            atp->abort_task(EXIT_MEM_LIMIT_EXCEEDED, buf);
            did_anything = true;
            continue;
        }
        if (do_disk_check || atp->peak_disk_usage == 0) {
            if (atp->check_max_disk_exceeded()) {
                did_anything = true;
                continue;
            }
        }

        // don't count RAM usage of non-CPU-intensive jobs
        //
        if (!atp->non_cpu_intensive()) {
            ram_left -= atp->procinfo.working_set_size_smoothed;
        }
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
    if (task_state() == PROCESS_ABORTED) {
        copy_final_info();
        read_stderr_file();
    }
    return 0;
}

// check for the stderr file, copy to result record
//
int ACTIVE_TASK::read_stderr_file() {
    char* buf1, *buf2;
    char path[MAXPATHLEN];

    // truncate stderr output to the last 63KB;
    // it's unlikely that more than that will be useful
    //
    int max_len = 63*1024;
    snprintf(path, sizeof(path), "%s/%s", slot_dir, STDERR_FILE);
    if (!boinc_file_exists(path)) return 0;
    int retval  = read_file_malloc(
        path, buf1, max_len, !cc_config.stderr_head
    );
    if (retval) return retval;

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
// TODO: get rid of this function
//
int ACTIVE_TASK::request_reread_prefs() {
    int retval;
    APP_INIT_DATA aid;

    link_user_files();

    init_app_init_data(aid);
    retval = write_app_init_file(aid);
    if (retval) return retval;
#if 0
    graphics_request_queue.msg_queue_send(
        xml_graphics_modes[MODE_REREAD_PREFS],
        app_client_shm.shm->graphics_request
    );
#endif
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
// (or all tasks, if proj is NULL).
// If they don't exit in QUIT_TIMEOUT seconds,
// send them a kill signal and wait up to 5 more seconds to exit.
// This is called when the client exits,
// or when a project is detached or reset
//
int ACTIVE_TASK_SET::exit_tasks(PROJECT* proj) {
    if (log_flags.task_debug) {
        msg_printf(NULL, MSG_INFO, "[task_debug] requesting tasks to exit");
    }
    request_tasks_exit(proj);

    // Wait for tasks to exit normally; if they don't then kill them
    //
    if (wait_for_exit(QUIT_TIMEOUT, proj)) {
        if (log_flags.task_debug) {
            msg_printf(NULL, MSG_INFO,
                "[task_debug] all tasks haven't exited after %d sec; killing them",
                QUIT_TIMEOUT
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
            client_clean_out_dir(atp->slot_dir, "abort_project()");
            remove_project_owned_dir(atp->slot_dir);
            task_iter = active_tasks.erase(task_iter);
            delete atp;
        } else {
            ++task_iter;
        }
    }
    return 0;
}

// suspend all currently running tasks
// e.g. because on batteries, time of day, benchmarking, CPU throttle, etc.
//
void ACTIVE_TASK_SET::suspend_all(int reason) {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];

        // don't suspend if process doesn't exist,
        // or if quit/abort is pending.
        // If process is currently suspended, proceed;
        // the new suspension may require it to be removed from memory.
        // E.g. a GPU job may currently be suspended due to CPU throttling,
        // and therefore left in memory,
        // but this suspension (say, a user request)
        // might require it to be removed from memory.
        //
        switch (atp->task_state()) {
        case PROCESS_EXECUTING:
        case PROCESS_SUSPENDED:
            break;
        default:
            continue;
        }

        // special cases for non-CPU-intensive apps
        //
        if (atp->non_cpu_intensive()) {
            if (cc_config.dont_suspend_nci) {
                continue;
            }
            if (reason == SUSPEND_REASON_BATTERIES) {
                continue;
            }
        }

        // handle CPU throttling separately
        //
        if (reason == SUSPEND_REASON_CPU_THROTTLE) {
            if (atp->dont_throttle()) continue;
            atp->preempt(REMOVE_NEVER, reason);
            continue;
        }

#ifdef ANDROID
        // On Android, remove apps from memory if on batteries
        // no matter what the reason for suspension.
        // The message polling in the BOINC runtime system
        // imposes an overhead which drains the battery
        //
        if (gstate.host_info.host_is_running_on_batteries()) {
            atp->preempt(REMOVE_ALWAYS);
            continue;
        }
#endif

        switch (reason) {
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
            if (atp->always_run()) break;
            atp->preempt(REMOVE_NEVER);
            break;
        case SUSPEND_REASON_BATTERY_OVERHEATED:
        case SUSPEND_REASON_BATTERY_CHARGING:
            // these conditions can oscillate, so leave apps in mem
            //
            atp->preempt(REMOVE_NEVER);
            break;
        default:
            atp->preempt(REMOVE_MAYBE_USER);
            break;
        }
    }
}

// resume all currently scheduled tasks
//
void ACTIVE_TASK_SET::unsuspend_all(int reason) {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        if (atp->task_state() == PROCESS_UNINITIALIZED) {
            if (atp->resume_or_start(false)) {
                msg_printf(atp->wup->project, MSG_INTERNAL_ERROR,
                    "Couldn't restart task %s", atp->result->name
                );
            }
        } else if (atp->task_state() == PROCESS_SUSPENDED) {
            atp->unsuspend(reason);
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
// This is called when the client exits,
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
        atp->kill_running_task(true);
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
int ACTIVE_TASK::unsuspend(int reason) {
    if (!app_client_shm.shm) return 0;
    if (task_state() != PROCESS_SUSPENDED) {
        msg_printf(result->project, MSG_INFO,
            "Internal error: expected process %s to be suspended", result->name
        );
    }
    if (log_flags.cpu_sched && reason != SUSPEND_REASON_CPU_THROTTLE) {
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
    int other_pid, i;
    double dtemp;
    static double last_msg_time=0;

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
            if (!first_fraction_done) {
                first_fraction_done = fd;
                first_fraction_done_elapsed_time = elapsed_time;
            }
            if (log_flags.task_debug && (fd<0 || fd>1)) {
                if (gstate.now > last_msg_time + 60) {
                    msg_printf(this->wup->project, MSG_INFO,
                        "[task_debug] app reported bad fraction done: %f", fd
                    );
                    last_msg_time = gstate.now;
                }
            }
        }
    }
    if (parse_double(msg_buf, "<current_cpu_time>", current_cpu_time)) {
        if (current_cpu_time < 0) {
            msg_printf(result->project, MSG_INFO,
                "app reporting negative CPU: %f", current_cpu_time
            );
            current_cpu_time = 0;
        }
    }
    if (parse_double(msg_buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) {
        if (checkpoint_cpu_time < 0) {
            msg_printf(result->project, MSG_INFO,
                "app reporting negative checkpoint CPU: %f", checkpoint_cpu_time
            );
            checkpoint_cpu_time = 0;
        }
    }
    parse_double(msg_buf, "<wss>", wss_from_app);
    parse_double(msg_buf, "<fpops_per_cpu_sec>", result->fpops_per_cpu_sec);
    parse_double(msg_buf, "<fpops_cumulative>", result->fpops_cumulative);
    parse_double(msg_buf, "<intops_per_cpu_sec>", result->intops_per_cpu_sec);
    parse_double(msg_buf, "<intops_cumulative>", result->intops_cumulative);
    if (parse_double(msg_buf, "<bytes_sent>", dtemp)) {
        if (dtemp > bytes_sent_episode) {
            double nbytes = dtemp - bytes_sent_episode;
            daily_xfer_history.add(nbytes, true);
            bytes_sent += nbytes;
        }
        bytes_sent_episode = dtemp;
    }
    if (parse_double(msg_buf, "<bytes_received>", dtemp)) {
        if (dtemp > bytes_received_episode) {
            double nbytes = dtemp - bytes_received_episode;
            daily_xfer_history.add(nbytes, false);
            bytes_received += nbytes;
        }
        bytes_received_episode = dtemp;
    }
    parse_int(msg_buf, "<want_network>", want_network);
    if (parse_int(msg_buf, "<other_pid>", other_pid)) {
        // for now, we handle only one of these
        other_pids.clear();
        other_pids.push_back(other_pid);
    }
    if (parse_int(msg_buf, "<sporadic_ac>", i)) {
        sporadic_ac_state = (SPORADIC_AC_STATE)i;
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
    if (!gstate.clock_change && last_time) {
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

    double et_diff = delta_t;
    double et_diff_throttle = delta_t * gstate.current_cpu_usage_limit()/100;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        old_time = atp->checkpoint_cpu_time;
        if (atp->scheduler_state == CPU_SCHED_SCHEDULED && !gstate.tasks_suspended) {
            double x = atp->result->dont_throttle()?et_diff:et_diff_throttle;
            atp->elapsed_time += x;
            atp->wup->project->elapsed_time += x;
        }
        if (atp->get_app_status_msg()) {
            if (old_time != atp->checkpoint_cpu_time) {
                char buf[512];
                snprintf(buf, sizeof(buf), "%s checkpointed", atp->result->name);
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
                }
                atp->write_task_state_file();
            }
        }
        atp->get_trickle_up_msg();
        atp->get_graphics_msg();
    }
}

// The job just checkpointed.
// Write some state items to a file in the slot dir
// (this avoids rewriting the state file on each checkpoint)
//
void ACTIVE_TASK::write_task_state_file() {
    char path[MAXPATHLEN];
    snprintf(path, sizeof(path), "%s/%s", slot_dir, TASK_STATE_FILENAME);
    FILE* f = boinc_fopen(path, "w");
    if (!f) return;
    fprintf(f,
        "<active_task>\n"
        "    <project_master_url>%s</project_master_url>\n"
        "    <result_name>%s</result_name>\n"
        "    <checkpoint_cpu_time>%f</checkpoint_cpu_time>\n"
        "    <checkpoint_elapsed_time>%f</checkpoint_elapsed_time>\n"
        "    <fraction_done>%f</fraction_done>\n"
        "    <peak_working_set_size>%.0f</peak_working_set_size>\n"
        "    <peak_swap_size>%.0f</peak_swap_size>\n"
        "    <peak_disk_usage>%.0f</peak_disk_usage>\n"
        "</active_task>\n",
        result->project->master_url,
        result->name,
        checkpoint_cpu_time,
        checkpoint_elapsed_time,
        checkpoint_fraction_done,
        peak_working_set_size,
        peak_swap_size,
        peak_disk_usage
    );
    fclose(f);
}

// called on startup; read the task state file in case it's more recent
// than the main state file
//
void ACTIVE_TASK::read_task_state_file() {
    char buf[4096], path[MAXPATHLEN], s[1024];
    snprintf(path, sizeof(path), "%s/%s", slot_dir, TASK_STATE_FILENAME);
    FILE* f = boinc_fopen(path, "r");
    if (!f) return;
    buf[0] = 0;
    size_t n = fread(buf, 1, 4096, f);
    fclose(f);
    if (n == 0) {
        return;
    }
    buf[4095] = 0;
    double x;
    // TODO: use XML parser

    // sanity checks - project and result name must match
    //
    if (!parse_str(buf, "<project_master_url>", s, sizeof(s))) {
        msg_printf(wup->project, MSG_INTERNAL_ERROR,
            "no project URL in task state file"
        );
        return;
    }
    if (!urls_match(s, result->project->master_url)) {
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
    if (parse_double(buf, "<peak_working_set_size>", x)) {
        peak_working_set_size = x;
    }
    if (parse_double(buf, "<peak_swap_size>", x)) {
        peak_swap_size = x;
    }
    if (parse_double(buf, "<peak_disk_usage>", x)) {
        peak_disk_usage = x;
    }
}

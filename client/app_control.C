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

// monitoring and process control of running apps

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_CSIGNAL
#include <csignal>
#elif defined(HAVE_SYS_SIGNAL_H)
#include <sys/signal.h>
#elif defined(HAVE_SIGNAL_H)
#include <signal.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#endif

using std::vector;

#include "filesys.h"
#include "error_numbers.h"
#include "util.h"
#include "parse.h"
#include "shmem.h"
#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"

#include "app.h"

bool ACTIVE_TASK::process_exists() {
    switch (task_state) {
    case PROCESS_EXECUTING:
    case PROCESS_SUSPENDED:
    case PROCESS_ABORT_PENDING:
        return true;
    }
    return false;
}

// Send a quit message.
//
int ACTIVE_TASK::request_exit() {
    if (!app_client_shm.shm) return 1;
    process_control_queue.msg_queue_send(
        "<quit/>",
        app_client_shm.shm->process_control_request
    );
    return 0;
}

// send a kill signal.
// This is not caught by the process
//
int ACTIVE_TASK::kill_task() {
#ifdef _WIN32
    return !TerminateProcess(pid_handle, -1);
#else
    return kill(pid, SIGKILL);
#endif
    cleanup_task();
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
    if (GetExitCodeProcess(pid_handle, &exit_code)) {
        if (exit_code != STILL_ACTIVE) {
            exited = true;
        }
    }
#else
    // We don't use status
    if (waitpid(pid, 0, WNOHANG) == pid) {
        exited = true;
    }
#endif
    if (exited) {
        task_state = PROCESS_EXITED;
        cleanup_task();
    }
    return exited;
}

// preempt this task
// called from the CLIENT_STATE::schedule_cpus()
// if quit_task is true do this by quitting
//
int ACTIVE_TASK::preempt(bool quit_task) {
    int retval;

    // If the app hasn't checkpoint yet, suspend instead of quit
    // (accommodate apps that never checkpoint)
    //
    if (quit_task && (checkpoint_cpu_time>0)) {
        retval = request_exit();
        pending_suspend_via_quit = true;
    } else {
        retval = suspend();
    }

    scheduler_state = CPU_SCHED_PREEMPTED;

    msg_printf(result->project, MSG_INFO,
        "Pausing result %s (%s)",
        result->name, (quit_task ? "removed from memory" : "left in memory")
    );
    return 0;
}

static void limbo_message(ACTIVE_TASK& at) {
    msg_printf(at.result->project, MSG_INFO,
        "Result %s exited with zero status but no 'finished' file",
        at.result->name
    );
    msg_printf(at.result->project, MSG_INFO,
        "If this happens repeatedly you may need to reset the project."
    );
}

// deal with a process that has exited, for whatever reason
// (including preemption)
//
#ifdef _WIN32
bool ACTIVE_TASK::handle_exited_app(unsigned long exit_code) {
    get_app_status_msg();
    get_trickle_up_msg();
    result->final_cpu_time = current_cpu_time;
    if (task_state == PROCESS_ABORT_PENDING) {
        task_state = PROCESS_ABORTED;
    } else {
        task_state = PROCESS_EXITED;

        if (exit_code) {
            char szError[1024];
            gstate.report_result_error(
                *result,
                "%s - exit code %d (0x%x)",
                windows_format_error_string(exit_code, szError, sizeof(szError)),
                exit_code, exit_code
            );
        } else {
            if (pending_suspend_via_quit) {
                pending_suspend_via_quit = false;
                task_state = PROCESS_UNINITIALIZED;
                close_process_handles();
                return true;
            }
            if (!finish_file_present()) {
                scheduler_state = CPU_SCHED_PREEMPTED;
                task_state = PROCESS_UNINITIALIZED;
                close_process_handles();
                limbo_message(*this);
                return true;
            }
        }
        result->exit_status = exit_code;
    }

    if (app_client_shm.shm) {
        detach_shmem(shm_handle, app_client_shm.shm);
        app_client_shm.shm = NULL;
    }

    read_stderr_file();
    clean_out_dir(slot_dir);
    return true;
}
#else
bool ACTIVE_TASK::handle_exited_app(int stat) {
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    get_app_status_msg();
    get_trickle_up_msg();
    result->final_cpu_time = current_cpu_time;
    if (task_state == PROCESS_ABORT_PENDING) {
        task_state = PROCESS_ABORTED;
    } else {
        if (WIFEXITED(stat)) {
            task_state = PROCESS_EXITED;
            result->exit_status = WEXITSTATUS(stat);

            if (result->exit_status) {
                gstate.report_result_error(
                    *result,
                    "process exited with code %d (0x%x)",
                    result->exit_status, result->exit_status
                );
            } else {
                // check for cases where an app exits
                // without it being done from core client's point of view;
                // in these cases, don't clean out slot dir
                //
                if (pending_suspend_via_quit) {
                    pending_suspend_via_quit = false;
                    task_state = PROCESS_UNINITIALIZED;

                    // destroy shm, since restarting app will re-create it
                    //
                    cleanup_task();
                    return true;
                }
                if (!finish_file_present()) {
                    // The process looks like it exited normally
                    // but there's no "finish file".
                    // Assume it was externally killed,
                    // and arrange for it to get restarted.
                    //
                    scheduler_state = CPU_SCHED_PREEMPTED;
                    task_state = PROCESS_UNINITIALIZED;
                    cleanup_task();
                    limbo_message(*this);
                    return true;
                }
            }
            scope_messages.printf(
                "ACTIVE_TASK::handle_exited_app(): process exited: status %d\n",
                result->exit_status
            );
        } else if (WIFSIGNALED(stat)) {
            int got_signal = WTERMSIG(stat);

            // if the process was externally killed, allow it to restart.
            //
            switch (got_signal) {
            case SIGHUP:
            case SIGINT:
            case SIGQUIT:
            case SIGKILL:
            case SIGTERM:
            case SIGSTOP:
                scheduler_state = CPU_SCHED_PREEMPTED;
                task_state = PROCESS_UNINITIALIZED;
                limbo_message(*this);
                return true;
            }
            result->exit_status = stat;
            task_state = PROCESS_WAS_SIGNALED;
            signal = got_signal;
            gstate.report_result_error(
                *result, "process got signal %d", signal
            );
            scope_messages.printf(
                "ACTIVE_TASK::handle_exited_app(): process got signal %d\n",
                signal
            );
        } else {
            task_state = PROCESS_EXIT_UNKNOWN;
            result->state = PROCESS_EXIT_UNKNOWN;
        }
    }

    read_stderr_file();
    clean_out_dir(slot_dir);
    return true;
}
#endif

bool ACTIVE_TASK::finish_file_present() {
    char path[256];
    sprintf(path, "%s%s%s", slot_dir, PATH_SEPARATOR, BOINC_FINISH_CALLED_FILE);
    return boinc_file_exists(path);
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

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (!atp->app_client_shm.shm) continue;
        atp->app_client_shm.shm->heartbeat.send_msg("<heartbeat/>\n");
    }
}

void ACTIVE_TASK_SET::process_control_poll() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (!atp->app_client_shm.shm) continue;
        atp->process_control_queue.msg_queue_poll(
            atp->app_client_shm.shm->process_control_request
        );
    }
}

// See if any processes have exited
//
bool ACTIVE_TASK_SET::check_app_exited() {
    ACTIVE_TASK* atp;
    bool found = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

#ifdef _WIN32
    unsigned long exit_code;
    unsigned int i;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        if (GetExitCodeProcess(atp->pid_handle, &exit_code)) {
            if (exit_code != STILL_ACTIVE) {
                found = true;
                atp->handle_exited_app(exit_code);
            }
        } else {
            char errmsg[1024];
            scope_messages.printf(
                "ACTIVE_TASK_SET::check_app_exited(): task %s GetExitCodeProcess Failed - GLE %d (0x%x)",
                windows_format_error_string(GetLastError(), errmsg, sizeof(errmsg)),
                GetLastError(), GetLastError()
            );
        }
    }
#else
    int pid, stat;

    if ((pid = waitpid(0, &stat, WNOHANG)) > 0) {
        scope_messages.printf("ACTIVE_TASK_SET::check_app_exited(): process %d is done\n", pid);
        atp = lookup_pid(pid);
        if (!atp) {
            // if we're running benchmarks, exited process
            // is probably a benchmark process
            //
            if (!gstate.are_cpu_benchmarks_running()) {
                msg_printf(NULL, MSG_ERROR,
                    "ACTIVE_TASK_SET::check_app_exited(): pid %d not found\n",
                    pid
                );
            }
            return false;
        }
        atp->handle_exited_app(stat);
        found = true;
    }
#endif

    if (found) {
        gstate.request_schedule_cpus("process exited");
    }
    return found;
}

// if an app has exceeded its maximum CPU time, abort it
//
bool ACTIVE_TASK::check_max_cpu_exceeded() {
    if (current_cpu_time > max_cpu_time) {
        msg_printf(result->project, MSG_INFO,
            "Aborting result %s: exceeded CPU time limit %f\n",
            result->name, max_cpu_time
        );
        abort_task(ERR_RSC_LIMIT_EXCEEDED, "Maximum CPU time exceeded");
        return true;
    }
    return false;
}

// if an app has exceeded its maximum disk usage, abort it
//
bool ACTIVE_TASK::check_max_disk_exceeded() {
    double disk_usage;
    int retval;

    // don't do disk check too often
    //
    retval = current_disk_usage(disk_usage);
    if (retval) {
        msg_printf(0, MSG_ERROR, "Can't get application disk usage: %s", boincerror(retval));
    } else {
        if (disk_usage > max_disk_usage) {
            msg_printf(
                result->project, MSG_INFO,
                "Aborting result %s: exceeded disk limit: %f > %f\n",
                result->name, disk_usage, max_disk_usage
            );
            abort_task(ERR_RSC_LIMIT_EXCEEDED, "Maximum disk usage exceeded");
            return true;
        }
    }
    return false;
}

#if 0
// if an app has exceeded its maximum allowed memory, abort it
//
bool ACTIVE_TASK::check_max_mem_exceeded() {
    // TODO: calculate working set size elsewhere
    if (working_set_size > max_mem_usage || working_set_size/1048576 > gstate.global_prefs.max_memory_mbytes) {
        msg_printf(
            result->project, MSG_INFO,
            "Aborting result %s: exceeded memory limit %f\n",
            result->name,
            min(max_mem_usage, gstate.global_prefs.max_memory_mbytes*1048576)
        );
        abort_task(ERR_RSC_LIMIT_EXCEEDED, "Maximum memory usage exceeded");
        return true;
    }
    return false;
}
#endif

bool ACTIVE_TASK::check_max_mem_exceeded() {
    if (max_mem_usage != 0 && rss_bytes > max_mem_usage) {
        msg_printf(
            result->project, MSG_INFO,
            "result %s: memory usage %f exceeds limit %f\n",
            result->name,
            rss_bytes,
            max_mem_usage
        );
        //abort_task(ERR_RSC_LIMIT_EXCEEDED, "Maximum memory usage exceeded");
        return true;
    }
    return false;
}

bool ACTIVE_TASK_SET::vm_limit_exceeded(double vm_limit) {
    unsigned int i;
    ACTIVE_TASK *atp;

    double total_vm_usage = 0;

    for (i=0; i<active_tasks.size(); ++i) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        total_vm_usage += atp->vm_bytes;
    }

    return (total_vm_usage > vm_limit);
}

// Check if any of the active tasks have exceeded their
// resource limits on disk, CPU time or memory
//
bool ACTIVE_TASK_SET::check_rsc_limits_exceeded() {
    unsigned int j;
    ACTIVE_TASK *atp;
    static double last_disk_check_time = 0;
    bool do_disk_check = false;
    bool did_anything = false;

    // disk_interval is typically 60 sec,
    // and some slot dirs have lots of files.
    // So only check every 5*disk_interval
    //
    if (gstate.now > last_disk_check_time + 5*gstate.global_prefs.disk_interval) {
        do_disk_check = true;
    }
    for (j=0;j<active_tasks.size();j++) {
        atp = active_tasks[j];
        if (atp->task_state != PROCESS_EXECUTING) continue;
        if (atp->check_max_cpu_exceeded()) did_anything = true;
        else if (atp->check_max_mem_exceeded()) did_anything = true;
        else if (do_disk_check && atp->check_max_disk_exceeded()) {
            did_anything = true;
        }
    }
    if (do_disk_check) {
        last_disk_check_time = gstate.now;
    }
    return did_anything;
}

// If process is running, send it a kill signal
// This is done when app has exceeded CPU, disk, or mem limits
//
int ACTIVE_TASK::abort_task(int exit_status, const char* msg) {
    if (task_state == PROCESS_EXECUTING || task_state == PROCESS_SUSPENDED) {
        task_state = PROCESS_ABORT_PENDING;
        kill_task();
    } else {
        task_state = PROCESS_ABORTED;
    }
    result->exit_status = exit_status;
    gstate.report_result_error(*result, msg);
    return 0;
}

// check for the stderr file, copy to result record
//
bool ACTIVE_TASK::read_stderr_file() {
    std::string stderr_file;
    char path[256];

    sprintf(path, "%s%s%s", slot_dir, PATH_SEPARATOR, STDERR_FILE);
    if (boinc_file_exists(path) && !read_file_string(path, stderr_file)) {
        // truncate stderr output to 63KB;
        // it's unlikely that more than that will be useful
        //
        int max_len = 63*1024;
        int len = stderr_file.length();
        if (len > max_len) {
            stderr_file = stderr_file.substr(len-max_len, len);
        }
        result->stderr_out += "<stderr_txt>\n";
        result->stderr_out += stderr_file;
        result->stderr_out += "\n</stderr_txt>\n";
        return true;
    }
    return false;
}

// tell a running app to reread project preferences.
// This is called when project prefs change,
// or when a user file has finished downloading.
//
int ACTIVE_TASK::request_reread_prefs() {
    int retval;

    link_user_files();

    retval = write_app_init_file();
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
    int retval = write_app_init_file();
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


// send quit signal to all tasks in the project
// (or all tasks, if proj==0).
// If they don't exit in 5 seconds,
// send them a kill signal and wait up to 5 more seconds to exit.
// This is called when the core client exits,
// or when a project is detached or reset
//
int ACTIVE_TASK_SET::exit_tasks(PROJECT* proj) {
    request_tasks_exit(proj);

    // Wait 5 seconds for them to exit normally; if they don't then kill them
    //
    if (wait_for_exit(5, proj)) {
        kill_tasks(proj);
    }
    wait_for_exit(5, proj);

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
    project->long_term_debt = 0;
    return 0;
}

// Find the ACTIVE_TASK in the current set with the matching PID
//
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_pid(int pid) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->pid == pid) return atp;
    }
    return NULL;
}

// Find the ACTIVE_TASK in the current set with the matching result
//
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_result(RESULT* result) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->result == result) {
            return atp;
        }
    }
    return NULL;
}

// suspend all currently running tasks
// called only from CLIENT_STATE::suspend_activities(),
// e.g. because on batteries, time of day, benchmarking, etc.
//
void ACTIVE_TASK_SET::suspend_all(bool leave_apps_in_memory) {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->task_state != PROCESS_EXECUTING) continue;
        if (atp->non_cpu_intensive) continue;
        atp->preempt(!leave_apps_in_memory);
    }
}

// resume all currently running tasks
//
void ACTIVE_TASK_SET::unsuspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->non_cpu_intensive) continue;
        if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
        if (atp->task_state == PROCESS_UNINITIALIZED) {
            if (atp->start(false)) {
                msg_printf(
                    atp->wup->project,
                    MSG_ERROR,
                    "ACTIVE_TASK_SET::unsuspend_all(): could not restart active_task"
                );
            }
        } else if (atp->task_state == PROCESS_SUSPENDED) {
            atp->unsuspend();
        }
    }
}

// Check to see if any tasks are running
// called if benchmarking and waiting for suspends to happen
//
bool ACTIVE_TASK_SET::is_task_executing() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->task_state == PROCESS_EXECUTING) {
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
        atp->kill_task();
    }
}

// send a <suspend> message
//
int ACTIVE_TASK::suspend() {
    if (!app_client_shm.shm) return 0;
    process_control_queue.msg_queue_send(
        "<suspend/>",
        app_client_shm.shm->process_control_request
    );
    task_state = PROCESS_SUSPENDED;
    return 0;
}

// resume a suspended task
//
int ACTIVE_TASK::unsuspend() {
    if (!app_client_shm.shm) return 0;
    process_control_queue.msg_queue_send(
        "<resume/>",
        app_client_shm.shm->process_control_request
    );
    task_state = PROCESS_EXECUTING;
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

    if (!app_client_shm.shm) {
        msg_printf(result->project, MSG_INFO,
            "%s: no shared memory segment", result->name
        );
        return false;
    }
    if (app_client_shm.shm->app_status.get_msg(msg_buf)) {
        fraction_done = current_cpu_time = checkpoint_cpu_time = 0.0;
        parse_double(msg_buf, "<fraction_done>", fraction_done);
        parse_double(msg_buf, "<current_cpu_time>", current_cpu_time);
        parse_double(msg_buf, "<checkpoint_cpu_time>", checkpoint_cpu_time);
        parse_double(msg_buf, "<vm_bytes>", vm_bytes);
        parse_double(msg_buf, "<rss_bytes>", rss_bytes);
        parse_int(msg_buf, "<non_cpu_intensive>", non_cpu_intensive);
        parse_double(msg_buf, "<fpops_per_cpu_sec>", result->fpops_per_cpu_sec);
        parse_double(msg_buf, "<fpops_cumulative>", result->fpops_cumulative);
        parse_double(msg_buf, "<intops_per_cpu_sec>", result->intops_per_cpu_sec);
        parse_double(msg_buf, "<intops_cumulative>", result->intops_cumulative);
    } else {
        return false;
    }
    return true;
}

bool ACTIVE_TASK::get_trickle_up_msg() {
    char msg_buf[MSG_CHANNEL_SIZE];
    bool found = false;
    int retval;

    if (!app_client_shm.shm) return false;
    if (app_client_shm.shm->trickle_up.get_msg(msg_buf)) {
        if (match_tag(msg_buf, "<have_new_trickle_up/>")) {
            retval = move_trickle_file();
            if (!retval) {
                wup->project->trickle_up_pending = true;
            }
        }
        if (match_tag(msg_buf, "<have_new_upload_file/>")) {
            handle_upload_files();
        }
        found = true;
    }
    return found;
}

// check for msgs from active tasks.
// Return true if any of them has changed its checkpoint_cpu_time
// (since in that case we need to write state file)
//
bool ACTIVE_TASK_SET::get_msgs() {
    unsigned int i;
    ACTIVE_TASK *atp;
    double old_time;
    bool action = false;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        old_time = atp->checkpoint_cpu_time;
        if (atp->get_app_status_msg()) {
            if (old_time != atp->checkpoint_cpu_time) {
                action = true;
            }
        }
        atp->get_trickle_up_msg();
    }
    return action;
}

const char *BOINC_RCSID_10ca137461 = "$Id$";

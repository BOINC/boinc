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

// Abstraction of a set of executing applications,
// connected to I/O files in various ways.
// Shouldn't depend on CLIENT_STATE.

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#ifndef _WIN32
#include <unistd.h>
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <cctype>
#include <ctime>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#endif


#include "error_numbers.h"
#include "filesys.h"
#include "file_names.h"
#include "parse.h"
#include "shmem.h"
#include "str_replace.h"
#include "str_util.h"
#include "util.h"

#include "async_file.h"
#include "client_msgs.h"
#include "client_state.h"
#include "procinfo.h"
#include "sandbox.h"

#include "app.h"

using std::max;
using std::min;

double exclusive_app_running = 0;
double exclusive_gpu_app_running = 0;
int gpu_suspend_reason;
double non_boinc_cpu_usage;

ACTIVE_TASK::~ACTIVE_TASK() {
    if (async_copy) {
        remove_async_copy(async_copy);
    }
}

ACTIVE_TASK::ACTIVE_TASK() {
    result = NULL;
    wup = NULL;
    app_version = NULL;
    pid = 0;

    _task_state = PROCESS_UNINITIALIZED;
    slot = 0;
    checkpoint_cpu_time = 0;
    checkpoint_elapsed_time = 0;
    checkpoint_fraction_done = 0;
    checkpoint_fraction_done_elapsed_time = 0;
    current_cpu_time = 0;
    once_ran_edf = false;

    fraction_done = 0;
    fraction_done_elapsed_time = 0;
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    signal = 0;
    run_interval_start_wall_time = gstate.now;
    checkpoint_wall_time = 0;
    elapsed_time = 0;
    bytes_sent = 0;
    bytes_received = 0;
    strcpy(slot_dir, "");
    have_trickle_down = false;
    send_upload_file_status = false;
    too_large = false;
    needs_shmem = false;
    want_network = 0;
    premature_exit_count = 0;
    quit_time = 0;
    procinfo.clear();
    procinfo.working_set_size_smoothed = 0;
#ifdef _WIN32
    process_handle = NULL;
    shm_handle = NULL;
#endif
    premature_exit_count = 0;
    overdue_checkpoint = false;
    last_deadline_miss_time = 0;
    strcpy(web_graphics_url, "");
    strcpy(remote_desktop_addr, "");
    async_copy = NULL;
}

// preempt this task;
// called from the CLIENT_STATE::enforce_schedule()
// and ACTIVE_TASK_SET::suspend_all()
//
int ACTIVE_TASK::preempt(int preempt_type) {
    bool remove=false;

    switch (preempt_type) {
    case REMOVE_NEVER:
        remove = false;
        break;
    case REMOVE_MAYBE_USER:
    case REMOVE_MAYBE_SCHED:
        // GPU jobs: always remove from mem, since it's tying up GPU RAM
        //
        if (result->uses_coprocs()) {
            remove = true;
            break;
        }
        // if it's never checkpointed, leave in mem
        //
        if (checkpoint_elapsed_time == 0) {
            remove = false;
            break;
        }
        // otherwise obey user prefs
        //
        remove = !gstate.global_prefs.leave_apps_in_memory;
        break;
    case REMOVE_ALWAYS:
        remove = true;
        break;
    }

    if (remove) {
        if (log_flags.cpu_sched) {
            msg_printf(result->project, MSG_INFO,
                "[cpu_sched] Preempting %s (removed from memory)",
                result->name
            );
        }
        return request_exit();
    } else {
        if (log_flags.cpu_sched) {
            msg_printf(result->project, MSG_INFO,
                "[cpu_sched] Preempting %s (left in memory)",
                result->name
            );
        }
        return suspend();
    }
    return 0;
}

// called when a process has exited or we've killed it
//
void ACTIVE_TASK::cleanup_task() {
#ifdef _WIN32
    if (process_handle) {
        CloseHandle(process_handle);
        process_handle = NULL;
    }
    // detach from shared mem.
    // This will destroy shmem seg since we're the last attachment
    //
    if (app_client_shm.shm) {
        detach_shmem(shm_handle, app_client_shm.shm);
        app_client_shm.shm = NULL;
    }
#else
    int retval;

    if (app_client_shm.shm) {
#ifndef __EMX__
        if (app_version->api_major_version() >= 6) {
            retval = detach_shmem_mmap(app_client_shm.shm, sizeof(SHARED_MEM));
        } else
#endif
        {
            retval = detach_shmem(app_client_shm.shm);
            if (retval) {
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Couldn't detach shared memory: %s", boincerror(retval)
                );
            }
            retval = destroy_shmem(shmem_seg_name);
            if (retval) {
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Couldn't destroy shared memory: %s", boincerror(retval)
                );
            }
        }
        app_client_shm.shm = NULL;
        gstate.retry_shmem_time = 0;
    }
#endif

    if (config.exit_after_finish) {
        exit(0);
    }
}

#ifndef SIM
int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = rp->avp;
    max_elapsed_time = rp->wup->rsc_fpops_bound/rp->avp->flops;
    max_disk_usage = rp->wup->rsc_disk_bound;
    max_mem_usage = rp->wup->rsc_memory_bound;
    get_slot_dir(slot, slot_dir, sizeof(slot_dir));
    relative_to_absolute(slot_dir, slot_path);
    return 0;
}
#endif

// Deallocate memory to prevent unneeded reporting of memory leaks
//
void ACTIVE_TASK_SET::free_mem() {
    vector<ACTIVE_TASK*>::iterator at_iter;
    ACTIVE_TASK *at;

    at_iter = active_tasks.begin();
    while (at_iter != active_tasks.end()) {
        at = active_tasks[0];
        at_iter = active_tasks.erase(at_iter);
        delete at;
    }
}

#ifndef SIM

bool app_running(PROC_MAP& pm, const char* p) {
    PROC_MAP::iterator i;
    for (i=pm.begin(); i!=pm.end(); i++) {
        PROCINFO& pi = i->second;
        //msg_printf(0, MSG_INFO, "running: [%s]", pi.command);
        if (!strcasecmp(pi.command, p)) {
            return true;
        }
    }
    return false;
}

#if 0  // debugging
void procinfo_show(PROCINFO& pi, PROC_MAP& pm) {
    unsigned int i;
    memset(&pi, 0, sizeof(pi));
    PROC_MAP::iterator i;
    for (i=pm.begin(); i!=pm.end(); i++) {
        PROCINFO& p = i->second;

        pi.kernel_time += p.kernel_time;
        pi.user_time += p.user_time;
        msg_printf(NULL, MSG_INFO, "%d %s: boinc %d low %d (%f %f) total (%f %f)",
            p.id, p.command, p.is_boinc_app, p.is_low_priority, p.kernel_time, p.user_time, pi.kernel_time, pi.user_time
        );
    }
}
#endif

void ACTIVE_TASK_SET::get_memory_usage() {
    static double last_mem_time=0;
    unsigned int i;
    int retval;
    static bool first = true;
    static double last_cpu_time;

    double diff = gstate.now - last_mem_time;
    if (diff < 0 || diff > MEMORY_USAGE_PERIOD + 10) {
        // user has changed system clock,
        // or there has been a long system sleep
        //
        last_mem_time = gstate.now;
        return;
    }
    if (diff < MEMORY_USAGE_PERIOD) return;

    last_mem_time = gstate.now;
    PROC_MAP pm;
    retval = procinfo_setup(pm);
    if (retval) {
        if (log_flags.mem_usage_debug) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "[mem_usage] procinfo_setup() returned %d", retval
            );
        }
        return;
    }
    for (i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->task_state() == PROCESS_UNINITIALIZED) continue;
        if (atp->pid ==0) continue;

        // scan all active tasks with a process, even if not scheduled, because
        // 1) we might have recently suspended a tasks,
        //    and we still need to count its time
        // 2) preempted tasks might not actually suspend themselves
        //    (and we'd count that as non-BOINC CPU usage
        //    and suspend everything).

        PROCINFO& pi = atp->procinfo;
        unsigned long last_page_fault_count = pi.page_fault_count;
        pi.clear();
        pi.id = atp->pid;
        vector<int>* v = NULL;
        if (atp->other_pids.size()>0) {
            v = &(atp->other_pids);
        }
        procinfo_app(pi, v, pm, atp->app_version->graphics_exec_file);
        pi.working_set_size_smoothed = .5*(pi.working_set_size_smoothed + pi.working_set_size);

        int pf = pi.page_fault_count - last_page_fault_count;
        pi.page_fault_rate = pf/diff;
        if (log_flags.mem_usage_debug) {
            msg_printf(atp->result->project, MSG_INFO,
                "[mem_usage] %s: WS %.2fMB, smoothed %.2fMB, page %.2fMB, %.2f page faults/sec, user CPU %.3f, kernel CPU %.3f",
                atp->result->name,
                pi.working_set_size/MEGA,
                pi.working_set_size_smoothed/MEGA,
                pi.swap_size/MEGA,
                pi.page_fault_rate,
                pi.user_time, pi.kernel_time
            );
        }
    }

    for (i=0; i<config.exclusive_apps.size(); i++) {
        if (app_running(pm, config.exclusive_apps[i].c_str())) {
            exclusive_app_running = gstate.now;
            break;
        }
    }
    for (i=0; i<config.exclusive_gpu_apps.size(); i++) {
        if (app_running(pm, config.exclusive_gpu_apps[i].c_str())) {
            exclusive_gpu_app_running = gstate.now;
            break;
        }
    }

    // get info on non-BOINC processes.
    // mem usage info is not useful because most OSs don't
    // move idle processes out of RAM, so physical memory is always full.
    // Also (at least on Win) page faults are used for various things,
    // not all of them generate disk I/O,
    // so they're not useful for detecting paging/thrashing.
    //
    PROCINFO pi;
    //procinfo_show(pi, pm);
    procinfo_non_boinc(pi, pm);
    if (log_flags.mem_usage_debug) {
        msg_printf(NULL, MSG_INFO,
            "[mem_usage] All others: RAM %.2fMB, page %.2fMB, user %.3f, kernel %.3f",
            pi.working_set_size/MEGA, pi.swap_size/MEGA,
            pi.user_time, pi.kernel_time
        );
    }
    double new_cpu_time = pi.user_time + pi.kernel_time;
    if (first) {
        first = false;
    } else {
        non_boinc_cpu_usage = (new_cpu_time - last_cpu_time)/(diff*gstate.host_info.p_ncpus);
        // processes might have exited in the last 10 sec,
        // causing this to be negative.
        if (non_boinc_cpu_usage < 0) non_boinc_cpu_usage = 0;
        if (log_flags.mem_usage_debug) {
            msg_printf(NULL, MSG_INFO,
                "[mem_usage] non-BOINC CPU usage: %.2f%%", non_boinc_cpu_usage*100
            );
        }
    }
    last_cpu_time = new_cpu_time;
}

#endif

// There's a new trickle file.
// Move it from slot dir to project dir
//
int ACTIVE_TASK::move_trickle_file() {
    char project_dir[256], new_path[1024], old_path[1024];
    int retval;

    get_project_dir(result->project, project_dir, sizeof(project_dir));
    sprintf(old_path, "%s/trickle_up.xml", slot_dir);
    sprintf(new_path,
        "%s/trickle_up_%s_%d.xml",
        project_dir, result->name, (int)time(0)
    );
    retval = boinc_rename(old_path, new_path);

    // if can't move it, remove
    //
    if (retval) {
        delete_project_owned_file(old_path, true);
        return ERR_RENAME;
    }
    return 0;
}

// size of output files and files in slot dir
//
int ACTIVE_TASK::current_disk_usage(double& size) {
    double x;
    unsigned int i;
    int retval;
    FILE_INFO* fip;
    char path[1024];

    retval = dir_size(slot_dir, size);
    if (retval) return retval;
    for (i=0; i<result->output_files.size(); i++) {
        fip = result->output_files[i].file_info;
        get_pathname(fip, path, sizeof(path));
        retval = file_size(path, x);
        if (!retval) size += x;
    }
    return 0;
}

bool ACTIVE_TASK_SET::is_slot_in_use(int slot) {
    unsigned int i;
    for (i=0; i<active_tasks.size(); i++) {
        if (active_tasks[i]->slot == slot) {
            return true;
        }
    }
    return false;
}

bool ACTIVE_TASK_SET::is_slot_dir_in_use(char* dir) {
    char path[1024];
    unsigned int i;
    for (i=0; i<active_tasks.size(); i++) {
        get_slot_dir(active_tasks[i]->slot, path, sizeof(path));
        if (!strcmp(path, dir)) return true;
    }
    return false;
}

// Get a free slot,
// and make a slot dir if needed
//
void ACTIVE_TASK::get_free_slot(RESULT* rp) {
#ifndef SIM
    int j, retval;
    char path[1024];

    for (j=0; ; j++) {
        if (gstate.active_tasks.is_slot_in_use(j)) continue;

        // make sure we can make an empty directory for this slot
        //
        get_slot_dir(j, path, sizeof(path));
        if (boinc_file_exists(path)) {
            if (is_dir(path)) {
                retval = client_clean_out_dir(path, "get_free_slot()");
                if (!retval) break;
            }
        } else {
            retval = make_slot_dir(j);
            if (!retval) break;
        }
    }
    slot = j;
    if (log_flags.slot_debug) {
        msg_printf(rp->project, MSG_INFO, "[slot] assigning slot %d to %s", j, rp->name);
    }
#endif
}

bool ACTIVE_TASK_SET::slot_taken(int slot) {
    unsigned int i;
    for (i=0; i<active_tasks.size(); i++) {
        if (active_tasks[i]->slot == slot) return true;
    }
    return false;
}

// <active_task_state> is here for the benefit of 3rd-party software
// that reads the client state file
//
int ACTIVE_TASK::write(MIOFILE& fout) {
    fout.printf(
        "<active_task>\n"
        "    <project_master_url>%s</project_master_url>\n"
        "    <result_name>%s</result_name>\n"
        "    <active_task_state>%d</active_task_state>\n"
        "    <app_version_num>%d</app_version_num>\n"
        "    <slot>%d</slot>\n"
        "    <checkpoint_cpu_time>%f</checkpoint_cpu_time>\n"
        "    <checkpoint_elapsed_time>%f</checkpoint_elapsed_time>\n"
        "    <checkpoint_fraction_done>%f</checkpoint_fraction_done>\n"
        "    <checkpoint_fraction_done_elapsed_time>%f</checkpoint_fraction_done_elapsed_time>\n"
        "    <current_cpu_time>%f</current_cpu_time>\n"
        "    <once_ran_edf>%d</once_ran_edf>\n"
        "    <swap_size>%f</swap_size>\n"
        "    <working_set_size>%f</working_set_size>\n"
        "    <working_set_size_smoothed>%f</working_set_size_smoothed>\n"
        "    <page_fault_rate>%f</page_fault_rate>\n",
        result->project->master_url,
        result->name,
        task_state(),
        app_version->version_num,
        slot,
        checkpoint_cpu_time,
        checkpoint_elapsed_time,
        checkpoint_fraction_done,
        checkpoint_fraction_done_elapsed_time,
        current_cpu_time,
        once_ran_edf?1:0,
        procinfo.swap_size,
        procinfo.working_set_size,
        procinfo.working_set_size_smoothed,
        procinfo.page_fault_rate
    );
    fout.printf("</active_task>\n");
    return 0;
}

#ifndef SIM

int ACTIVE_TASK::write_gui(MIOFILE& fout) {
    fout.printf(
        "<active_task>\n"
        "    <active_task_state>%d</active_task_state>\n"
        "    <app_version_num>%d</app_version_num>\n"
        "    <slot>%d</slot>\n"
        "    <pid>%d</pid>\n"
        "    <scheduler_state>%d</scheduler_state>\n"
        "    <checkpoint_cpu_time>%f</checkpoint_cpu_time>\n"
        "    <fraction_done>%f</fraction_done>\n"
        "    <current_cpu_time>%f</current_cpu_time>\n"
        "    <elapsed_time>%f</elapsed_time>\n"
        "    <swap_size>%f</swap_size>\n"
        "    <working_set_size>%f</working_set_size>\n"
        "    <working_set_size_smoothed>%f</working_set_size_smoothed>\n"
        "    <page_fault_rate>%f</page_fault_rate>\n"
        "%s"
        "%s",
        task_state(),
        app_version->version_num,
        slot,
        pid,
        scheduler_state,
        checkpoint_cpu_time,
        fraction_done,
        current_cpu_time,
        elapsed_time,
        procinfo.swap_size,
        procinfo.working_set_size,
        procinfo.working_set_size_smoothed,
        procinfo.page_fault_rate,
        too_large?"   <too_large/>\n":"",
        needs_shmem?"   <needs_shmem/>\n":""
    );
    if (strlen(app_version->graphics_exec_path)) {
        fout.printf(
            "   <graphics_exec_path>%s</graphics_exec_path>\n"
            "   <slot_path>%s</slot_path>\n",
            app_version->graphics_exec_path,
            slot_path
        );
    }
    if (strlen(web_graphics_url)) {
        fout.printf(
            "   <web_graphics_url>%s</web_graphics_url>\n",
            web_graphics_url
        );
    }
    if (strlen(remote_desktop_addr)) {
        fout.printf(
            "   <remote_desktop_addr>%s</remote_desktop_addr>\n",
            remote_desktop_addr
        );
    }
    fout.printf("</active_task>\n");
    return 0;
}

#endif

int ACTIVE_TASK::parse(XML_PARSER& xp) {
    char result_name[256], project_master_url[256];
    int n, dummy;
    unsigned int i;
    PROJECT* project=0;
    double x;

    strcpy(result_name, "");
    strcpy(project_master_url, "");

    while (!xp.get_tag()) {
        if (xp.match_tag("/active_task")) {
            project = gstate.lookup_project(project_master_url);
            if (!project) {
                msg_printf(
                    NULL, MSG_INTERNAL_ERROR,
                    "State file error: project %s not found for task\n",
                    project_master_url
                );
                return ERR_NULL;
            }
            result = gstate.lookup_result(project, result_name);
            if (!result) {
                msg_printf(
                    project, MSG_INTERNAL_ERROR,
                    "State file error: result %s not found for task\n",
                    result_name
                );
                return ERR_NULL;
            }

            // various sanity checks
            //
            if (result->got_server_ack
                || result->ready_to_report
                || result->state() != RESULT_FILES_DOWNLOADED
            ) {
                return ERR_BAD_RESULT_STATE;
            }

            wup = result->wup;
            app_version = gstate.lookup_app_version(
                result->app, result->platform, result->version_num,
                result->plan_class
            );
            if (!app_version) {
                msg_printf(
                    project, MSG_INTERNAL_ERROR,
                    "State file error: app %s platform %s version %d not found\n",
                    result->app->name, result->platform, result->version_num
                );
                return ERR_NULL;
            }

            // make sure no two active tasks are in same slot
            //
            for (i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
                ACTIVE_TASK* atp = gstate.active_tasks.active_tasks[i];
                if (atp->slot == slot) {
                    msg_printf(project, MSG_INTERNAL_ERROR,
                        "State file error: two tasks in slot %d\n", slot
                    );
                    return ERR_BAD_RESULT_STATE;
                }
            }

            // for 6.2/6.4 transition
            //
            if (checkpoint_elapsed_time == 0) {
                elapsed_time = checkpoint_cpu_time;
                checkpoint_elapsed_time = elapsed_time;
            }

            // for 6.12.25-26 transition;
            // old clients write fraction_done to state file;
            // new clients don't
            if (fraction_done && checkpoint_elapsed_time) {
                checkpoint_fraction_done = fraction_done;
                checkpoint_fraction_done_elapsed_time = checkpoint_elapsed_time;
                fraction_done_elapsed_time = checkpoint_elapsed_time;
            } else {
                fraction_done = checkpoint_fraction_done;
                fraction_done_elapsed_time = checkpoint_fraction_done_elapsed_time;
            }
            return 0;
        }
        else if (xp.parse_str("result_name", result_name, sizeof(result_name))) continue;
        else if (xp.parse_str("project_master_url", project_master_url, sizeof(project_master_url))) continue;
        else if (xp.parse_int("slot", slot)) continue;
        else if (xp.parse_int("active_task_state", dummy)) continue;
        else if (xp.parse_double("checkpoint_cpu_time", checkpoint_cpu_time)) continue;
        else if (xp.parse_double("checkpoint_elapsed_time", checkpoint_elapsed_time)) continue;
        else if (xp.parse_double("checkpoint_fraction_done", checkpoint_fraction_done)) continue;
        else if (xp.parse_double("checkpoint_fraction_done_elapsed_time", checkpoint_fraction_done_elapsed_time)) continue;
        else if (xp.parse_bool("once_ran_edf", once_ran_edf)) continue;
        else if (xp.parse_double("fraction_done", fraction_done)) continue;
            // deprecated - for backwards compat
        else if (xp.parse_int("app_version_num", n)) continue;
        else if (xp.parse_double("swap_size",  procinfo.swap_size)) continue;
        else if (xp.parse_double("working_set_size", procinfo.working_set_size)) continue;
        else if (xp.parse_double("working_set_size_smoothed", procinfo.working_set_size_smoothed)) continue;
        else if (xp.parse_double("page_fault_rate", procinfo.page_fault_rate)) continue;
        else if (xp.parse_double("current_cpu_time", x)) continue;
        else {
            if (log_flags.unparsed_xml) {
                msg_printf(project, MSG_INFO,
                    "[unparsed_xml] ACTIVE_TASK::parse(): unrecognized %s\n",
                    xp.parsed_tag
                );
            }
        }
    }
    return ERR_XML_PARSE;
}

int ACTIVE_TASK_SET::write(MIOFILE& fout) {
    unsigned int i;
    int retval;

    fout.printf("<active_task_set>\n");
    for (i=0; i<active_tasks.size(); i++) {
        retval = active_tasks[i]->write(fout);
        if (retval) return retval;
    }
    fout.printf("</active_task_set>\n");
    return 0;
}

int ACTIVE_TASK_SET::parse(XML_PARSER& xp) {
    ACTIVE_TASK* atp;
    int retval;

    while (!xp.get_tag()) {
        if (xp.match_tag("/active_task_set")) return 0;
        else if (xp.match_tag("active_task")) {
#ifdef SIM
            ACTIVE_TASK at;
            at.parse(xp);
#else
            atp = new ACTIVE_TASK;
            retval = atp->parse(xp);
            if (!retval) {
                if (slot_taken(atp->slot)) {
                    msg_printf(atp->result->project, MSG_INTERNAL_ERROR,
                        "slot %d in use; discarding result %s",
                        atp->slot, atp->result->name
                    );
                    retval = ERR_XML_PARSE;
                }
            }
            if (!retval) active_tasks.push_back(atp);
            else delete atp;
#endif
        } else {
            if (log_flags.unparsed_xml) {
                msg_printf(NULL, MSG_INFO,
                    "[unparsed_xml] ACTIVE_TASK_SET::parse(): unrecognized %s\n", xp.parsed_tag
                );
            }
        }
    }
    return ERR_XML_PARSE;
}

#ifndef SIM

void MSG_QUEUE::init(char* n) {
    strcpy(name, n);
    last_block = 0;
    msgs.clear();
}

void MSG_QUEUE::msg_queue_send(const char* msg, MSG_CHANNEL& channel) {
    if ((msgs.size()==0) && channel.send_msg(msg)) {
        if (log_flags.app_msg_send) {
            msg_printf(NULL, MSG_INFO,
                "[app_msg_send] sent %s to %s", msg, name
            );
        }
        last_block = 0;
        return;
    }
    if (log_flags.app_msg_send) {
        msg_printf(NULL, MSG_INFO,
            "[app_msg_send] deferred %s to %s", msg, name
        );
    }
    msgs.push_back(string(msg));
    if (!last_block) last_block = gstate.now;
}

void MSG_QUEUE::msg_queue_poll(MSG_CHANNEL& channel) {
    if (msgs.empty()) return;
    if (log_flags.app_msg_send) {
        msg_printf(NULL, MSG_INFO,
            "[app_msg_send] poll: %d msgs queued for %s:",
            (int)msgs.size(), name
        );
    }
    if (channel.send_msg(msgs[0].c_str())) {
        if (log_flags.app_msg_send) {
            msg_printf(NULL, MSG_INFO,
                "[app_msg_send] poll: delayed sent %s", msgs[0].c_str()
            );
        }
        msgs.erase(msgs.begin());
        last_block = 0;
    }
    for (unsigned int i=0; i<msgs.size(); i++) {
        if (log_flags.app_msg_send) {
            msg_printf(NULL, MSG_INFO,
                "[app_msg_send] poll: deferred: %s", msgs[0].c_str()
            );
        }
    }
}

// if the last message in the buffer is "msg", remove it and return 1
//
int MSG_QUEUE::msg_queue_purge(const char* msg) {
    if (msgs.empty()) return 0;
    string last_msg = msgs.back();
    if (log_flags.app_msg_send) {
        msg_printf(NULL, MSG_INFO,
            "[app_msg_send] purge: wanted %s last msg is %s in %s",
            msg, last_msg.c_str(), name
        );
    }
    if (!strcmp(msg, last_msg.c_str())) {
        if (log_flags.app_msg_send) {
            msg_printf(NULL, MSG_INFO,
                "[app_msg_send] purged %s from %s", msg, name
            );
        }
        msgs.pop_back();
        return 1;
    }
    return 0;
}

bool MSG_QUEUE::timeout(double diff) {
    if (!last_block) return false;
    if (gstate.now - last_block > diff) {
        return true;
    }
    return false;
}

#endif

void ACTIVE_TASK_SET::report_overdue() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        double diff = (gstate.now - atp->result->report_deadline)/86400;
        if (diff > 0) {
            msg_printf(atp->result->project, MSG_INFO,
                "Task %s is %.2f days overdue; you may not get credit for it.  Consider aborting it.", atp->result->name, diff
            );
        }
    }
}

// scan the slot directory, looking for files with names
// of the form boinc_ufr_X.
// Then mark file X as being present (and uploadable)
//
int ACTIVE_TASK::handle_upload_files() {
    std::string filename;
    char buf[256], path[1024];
    int retval;

    DirScanner dirscan(slot_dir);
    while (dirscan.scan(filename)) {
        strcpy(buf, filename.c_str());
        if (strstr(buf, UPLOAD_FILE_REQ_PREFIX) == buf) {
            char* p = buf+strlen(UPLOAD_FILE_REQ_PREFIX);
            FILE_INFO* fip = result->lookup_file_logical(p);
            if (fip) {
                get_pathname(fip, path, sizeof(path));
                retval = md5_file(path, fip->md5_cksum, fip->nbytes);
                if (retval) {
                    fip->status = retval;
                } else {
                    fip->status = FILE_PRESENT;
                }
            } else {
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Can't find uploadable file %s", p
                );
            }
            sprintf(path, "%s/%s", slot_dir, buf);
            delete_project_owned_file(path, true);  // delete the link file
        }
    }
    return 0;
}

void ACTIVE_TASK_SET::handle_upload_files() {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        atp->handle_upload_files();
    }
}

bool ACTIVE_TASK_SET::want_network() {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->want_network) return true;
    }
    return false;
}

void ACTIVE_TASK_SET::network_available() {
#ifndef SIM
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        if (atp->want_network) {
            atp->send_network_available();
        }
    }
#endif
}

void ACTIVE_TASK::upload_notify_app(const FILE_INFO* fip, const FILE_REF* frp) {
    char path[256];
    sprintf(path, "%s/%s%s", slot_dir, UPLOAD_FILE_STATUS_PREFIX, frp->open_name);
    FILE* f = boinc_fopen(path, "w");
    if (!f) return;
    fprintf(f, "<status>%d</status>\n", fip->status);
    fclose(f);
    send_upload_file_status = true;
}

// a file upload has finished.
// If any running apps are waiting for it, notify them
//
void ACTIVE_TASK_SET::upload_notify_app(FILE_INFO* fip) {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        RESULT* rp = atp->result;
        FILE_REF* frp = rp->lookup_file(fip);
        if (frp) {
            atp->upload_notify_app(fip, frp);
        }
    }
}

#ifndef SIM
void ACTIVE_TASK_SET::init() {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        atp->init(atp->result);
        atp->scheduler_state = CPU_SCHED_PREEMPTED;
        atp->read_task_state_file();
        atp->current_cpu_time = atp->checkpoint_cpu_time;
        atp->elapsed_time = atp->checkpoint_elapsed_time;
    }
}

#endif

static const char* task_state_name(int val) {
    switch (val) {
    case PROCESS_UNINITIALIZED: return "UNINITIALIZED";
    case PROCESS_EXECUTING: return "EXECUTING";
    case PROCESS_SUSPENDED: return "SUSPENDED";
    case PROCESS_ABORT_PENDING: return "ABORT_PENDING";
    case PROCESS_EXITED: return "EXITED";
    case PROCESS_WAS_SIGNALED: return "WAS_SIGNALED";
    case PROCESS_EXIT_UNKNOWN: return "EXIT_UNKNOWN";
    case PROCESS_ABORTED: return "ABORTED";
    case PROCESS_COULDNT_START: return "COULDNT_START";
    case PROCESS_QUIT_PENDING: return "QUIT_PENDING";
    case PROCESS_COPY_PENDING: return "COPY_PENDING";
    }
    return "Unknown";
}

void ACTIVE_TASK::set_task_state(int val, const char* where) {
    _task_state = val;
    if (log_flags.task_debug) {
        msg_printf(result->project, MSG_INFO,
            "[task] task_state=%s for %s from %s",
            task_state_name(val), result->name, where
        );
    }
}


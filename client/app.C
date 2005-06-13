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

// Abstraction of a set of executing applications,
// connected to I/O files in various ways.
// Shouldn't depend on CLIENT_STATE.

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
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

#include "client_state.h"
#include "client_types.h"
#include "error_numbers.h"
#include "filesys.h"
#include "file_names.h"
#include "parse.h"
#include "shmem.h"
#include "util.h"

#include "client_msgs.h"
#include "app.h"

using std::vector;
using std::max;
using std::min;

ACTIVE_TASK::ACTIVE_TASK() {
    result = NULL;
    wup = NULL;
    app_version = NULL;
    pid = 0;
    slot = 0;
    task_state = PROCESS_UNINITIALIZED;
    scheduler_state = CPU_SCHED_UNINITIALIZED;
    signal = 0;
    strcpy(slot_dir, "");
    is_ss_app = false;
    graphics_mode_acked = MODE_UNSUPPORTED;
    graphics_mode_before_ss = MODE_HIDE_GRAPHICS;

    fraction_done = 0;
#if 0
    frac_rate_of_change = 0;
    last_frac_done = 0;
    recent_change = 0;
    last_frac_update = 0;
#endif
    episode_start_cpu_time = 0;
    cpu_time_at_last_sched = 0;
    checkpoint_cpu_time = 0;
    current_cpu_time = 0;
    vm_bytes = 0;
    rss_bytes = 0;
    have_trickle_down = false;
    send_upload_file_status = false;
    pending_suspend_via_quit = false;
#ifdef _WIN32
    pid_handle = 0;
    thread_handle = 0;
    quitRequestEvent = 0;
    shm_handle = 0;
#endif
}

#ifdef _WIN32

// call this when a process has existed but will be started again
// (e.g. suspend via quit, exited but no finish file).
// In these cases we want to keep the shmem and events
//
void ACTIVE_TASK::close_process_handles() {
    if (pid_handle) {
        CloseHandle(pid_handle);
        pid_handle = NULL;
    }
    if (thread_handle) {
        CloseHandle(thread_handle);
        thread_handle = NULL;
    }
}
#endif

// call this when a process has exited and we're not going to restart it
//
void ACTIVE_TASK::cleanup_task() {
#ifdef _WIN32
    close_process_handles();
    if (quitRequestEvent) {
        CloseHandle(quitRequestEvent);
        quitRequestEvent = NULL;
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
        retval = detach_shmem(app_client_shm.shm);
        if (retval) {
            msg_printf(NULL, MSG_ERROR, "detach_shmem: %d", retval);
        }
        retval = destroy_shmem(shmem_seg_name);
        if (retval) {
            msg_printf(NULL, MSG_ERROR, "destroy_shmem: %d", retval);
        }
        app_client_shm.shm = NULL;
    }
#endif
}

ACTIVE_TASK::~ACTIVE_TASK() {
    cleanup_task();
}

int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = wup->avp;
    max_cpu_time = rp->wup->rsc_fpops_bound/gstate.host_info.p_fpops;
    max_disk_usage = rp->wup->rsc_disk_bound;
    max_mem_usage = rp->wup->rsc_memory_bound;

    strcpy(process_control_queue.name, rp->name);
    strcpy(graphics_request_queue.name, rp->name);

    return 0;
}

#if 0
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
#endif

// Do period checks on running apps:
// - get latest CPU time and % done info
// - check if any has exited, and clean up
// - see if any has exceeded its CPU or disk space limits, and abort it
//
bool ACTIVE_TASK_SET::poll() {
    bool action;
    static double last_time = 0;
    if (gstate.now - last_time < 1.0) return false;
    last_time = gstate.now;

    action = check_app_exited();
    send_heartbeats();
    send_trickle_downs();
    graphics_poll();
    process_control_poll();
    action |= check_rsc_limits_exceeded();
    if (get_msgs()) {
        action = true;
    }
    if (action) {
        gstate.set_client_state_dirty("ACTIVE_TASK_SET::poll");
    }
    return action;
}

// Remove an ACTIVE_TASK from the set.
// Does NOT delete the ACTIVE_TASK object.
//
int ACTIVE_TASK_SET::remove(ACTIVE_TASK* atp) {
    vector<ACTIVE_TASK*>::iterator iter;

    iter = active_tasks.begin();
    while (iter != active_tasks.end()) {
        if (*iter == atp) {
            active_tasks.erase(iter);
            return 0;
        }
        iter++;
    }
    msg_printf(NULL, MSG_ERROR, "ACTIVE_TASK_SET::remove(): not found\n");
    return ERR_NOT_FOUND;
}

// There's a new trickle file.
// Move it from slot dir to project dir
//
int ACTIVE_TASK::move_trickle_file() {
    char project_dir[256], new_path[256], old_path[256];
    int retval;

    get_project_dir(result->project, project_dir);
    sprintf(old_path, "%s%strickle_up.xml", slot_dir, PATH_SEPARATOR);
    sprintf(new_path,
        "%s%strickle_up_%s_%d.xml",
        project_dir, PATH_SEPARATOR, result->name, (int)time(0)
    );
    retval = boinc_rename(old_path, new_path);

    // if can't move it, remove
    //
    if (retval) {
        boinc_delete_file(old_path);
        return ERR_RENAME;
    }
    return 0;
}

// Returns the estimated CPU time to completion (in seconds) of this task.
// Compute this as a weighted average of estimates based on
// 1) the workunit's flops count
// 2) the current reported CPU time and fraction done
//
double ACTIVE_TASK::est_cpu_time_to_completion() {
    if (fraction_done >= 1) return 0;
    double wu_est = result->estimated_cpu_time();
    if (fraction_done <= 0) return wu_est;
    double frac_est = (current_cpu_time / fraction_done) - current_cpu_time;
    return fraction_done*frac_est + (1-fraction_done)*wu_est;
}

// size of output files and files in slot dir
//
int ACTIVE_TASK::current_disk_usage(double& size) {
    double x;
    unsigned int i;
    int retval;
    FILE_INFO* fip;
    char path[256];

    retval = dir_size(slot_dir, size);
    if (retval) return retval;
    for (i=0; i<result->output_files.size(); i++) {
        fip = result->output_files[i].file_info;
        get_pathname(fip, path);
        retval = file_size(path, x);
        if (!retval) size += x;
    }
    return 0;
}

// Get the next free slot
//
int ACTIVE_TASK_SET::get_free_slot() {
    unsigned int i;
    int j;
    bool found;

    for (j=0; ; j++) {
        found = false;
        for (i=0; i<active_tasks.size(); i++) {
            if (active_tasks[i]->slot == j) {
                found = true;
                break;
            }
        }
        if (!found) return j;
    }
    return ERR_NOT_FOUND;   // probably never get here
}

int ACTIVE_TASK::write(MIOFILE& fout) {
    fout.printf(
        "<active_task>\n"
        "    <project_master_url>%s</project_master_url>\n"
        "    <result_name>%s</result_name>\n"
        "    <active_task_state>%d</active_task_state>\n"
        "    <app_version_num>%d</app_version_num>\n"
        "    <slot>%d</slot>\n"
        "    <scheduler_state>%d</scheduler_state>\n"
        "    <checkpoint_cpu_time>%f</checkpoint_cpu_time>\n"
        "    <fraction_done>%f</fraction_done>\n"
        "    <current_cpu_time>%f</current_cpu_time>\n"
        "    <vm_bytes>%f</vm_bytes>\n"
        "    <rss_bytes>%f</rss_bytes>\n",
        result->project->master_url,
        result->name,
        task_state,
        app_version->version_num,
        slot,
        scheduler_state,
        checkpoint_cpu_time,
        fraction_done,
        current_cpu_time,
        vm_bytes,
        rss_bytes
    );
    if (supports_graphics()) {
        fout.printf(
            "   <supports_graphics/>\n"
            "   <graphics_mode_acked>%d</graphics_mode_acked>\n",
            graphics_mode_acked
        );
    }
    fout.printf("</active_task>\n");
    return 0;
}

int ACTIVE_TASK::parse(MIOFILE& fin) {
    char buf[256], result_name[256], project_master_url[256];
    int app_version_num=0, n;
    double x;
    PROJECT* project;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    strcpy(result_name, "");
    strcpy(project_master_url, "");
    scheduler_state = CPU_SCHED_SCHEDULED;

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</active_task>")) {
            project = gstate.lookup_project(project_master_url);
            if (!project) {
                msg_printf(
                    NULL, MSG_ERROR,
                    "ACTIVE_TASK::parse(): project not found: %s\n",
                    project_master_url
                );
                return ERR_NULL;
            }
            result = gstate.lookup_result(project, result_name);
            if (!result) {
                msg_printf(
                    project, MSG_ERROR, "ACTIVE_TASK::parse(): result not found\n"
                );
                return ERR_NULL;
            }

            // various sanity checks
            //
            if (result->got_server_ack
                || result->ready_to_report
                || result->state != RESULT_FILES_DOWNLOADED
            ) {
                msg_printf(project, MSG_ERROR,
                    "ACTIVE_TASK::parse(): result is in wrong state\n"
                );
                return ERR_BAD_RESULT_STATE;
            }

            wup = result->wup;
            app_version = gstate.lookup_app_version(
                result->app, app_version_num
            );
            if (!app_version) {
                msg_printf(
                    project, MSG_ERROR,
                    "ACTIVE_TASK::parse(): app_version not found\n"
                );
                return ERR_NULL;
            }
            return 0;
        }
        else if (parse_str(buf, "<result_name>", result_name, sizeof(result_name))) continue;
        else if (parse_str(buf, "<project_master_url>", project_master_url, sizeof(project_master_url))) continue;
        else if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        else if (parse_int(buf, "<slot>", slot)) continue;
        else if (parse_int(buf, "<scheduler_state>", scheduler_state)) continue;
        else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        else if (parse_double(buf, "<fraction_done>", fraction_done)) continue;
        else if (parse_double(buf, "<current_cpu_time>", current_cpu_time)) continue;
        else if (parse_int(buf, "<active_task_state>", n)) continue;
        else if (parse_double(buf, "<vm_bytes>", x)) continue;
        else if (parse_double(buf, "<rss_bytes>", x)) continue;
        else if (match_tag(buf, "<supports_graphics/>")) continue;
        else if (parse_int(buf, "<graphics_mode_acked>", n)) continue;
        else scope_messages.printf("ACTIVE_TASK::parse(): unrecognized %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// Write XML information about this active task set
//
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

// Parse XML information about an active task set
//
int ACTIVE_TASK_SET::parse(MIOFILE& fin) {
    ACTIVE_TASK* atp;
    char buf[256];
    int retval;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_TASK);

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</active_task_set>")) return 0;
        else if (match_tag(buf, "<active_task>")) {
            atp = new ACTIVE_TASK;
            retval = atp->parse(fin);
            if (!retval) active_tasks.push_back(atp);
            else delete atp;
        } else scope_messages.printf("ACTIVE_TASK_SET::parse(): unrecognized %s\n", buf);
    }
    return 0;
}

void MSG_QUEUE::msg_queue_send(const char* msg, MSG_CHANNEL& channel) {
    if (channel.send_msg(msg)) {
        //msg_printf(NULL, MSG_INFO, "sent %s to %s", msg, name);
        return;
    }
    msgs.push_back(std::string(msg));
}

void MSG_QUEUE::msg_queue_poll(MSG_CHANNEL& channel) {
    if (msgs.size() > 0) {
        if (channel.send_msg(msgs[0].c_str())) {
            //msg_printf(NULL, MSG_INFO, "sent %s to %s (delayed)", (msgs[0].c_str()), name);
            msgs.erase(msgs.begin());
        }
    }
}

void ACTIVE_TASK_SET::report_overdue() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        double diff = (gstate.now - atp->result->report_deadline)/86400;
        if (diff > 0) {
            msg_printf(atp->result->project, MSG_ERROR,
                "Result %s is %.2f days overdue.", atp->result->name, diff
            );
            msg_printf(atp->result->project, MSG_ERROR,
                "You may not get credit for it.  Consider aborting it."
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
    char buf[256], path[256];
    int retval;

    DirScanner dirscan(slot_dir);
    while (dirscan.scan(filename)) {
        strcpy(buf, filename.c_str());
        if (strstr(buf, UPLOAD_FILE_REQ_PREFIX) == buf) {
            char* p = buf+strlen(UPLOAD_FILE_REQ_PREFIX);
            FILE_INFO* fip = result->lookup_file_logical(p);
            if (fip) {
                get_pathname(fip, path);
                retval = md5_file(path, fip->md5_cksum, fip->nbytes);
                if (retval) {
                    fip->status = retval;
                } else {
                    fip->status = FILE_PRESENT;
                }
            } else {
                msg_printf(0, MSG_ERROR, "Can't find %s", p);
            }
            sprintf(path, "%s/%s", slot_dir, buf);
            boinc_delete_file(path);
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

const char *BOINC_RCSID_778b61195e = "$Id$";

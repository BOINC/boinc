// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// The "policy" part of task execution is here.
// The "mechanism" part is in app.cpp
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cassert>
#include <csignal>
#endif

#include <algorithm>

#include "error_numbers.h"
#include "filesys.h"
#include "md5_file.h"
#include "shmem.h"
#include "util.h"
#include "url.h"

#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "log_flags.h"
#include "project.h"
#include "result.h"

using std::vector;

// Clean up after finished apps.
// Called every second from the main polling loop.
//
bool CLIENT_STATE::handle_finished_apps() {
    ACTIVE_TASK* atp;
    bool action = false;
    static double last_time = 0;
    if (!clock_change && now - last_time < HANDLE_FINISHED_APPS_PERIOD) return false;
    last_time = now;

    vector<ACTIVE_TASK*>::iterator iter;

    iter = active_tasks.active_tasks.begin();
    while (iter != active_tasks.active_tasks.end()) {
        atp = *iter;
        switch (atp->task_state()) {
        case PROCESS_EXITED:
        case PROCESS_WAS_SIGNALED:
        case PROCESS_EXIT_UNKNOWN:
        case PROCESS_COULDNT_START:
        case PROCESS_ABORTED:
            if (log_flags.task) {
                msg_printf(atp->wup->project, MSG_INFO,
                    "Computation for task %s finished", atp->result->name
                );
            }
            app_finished(*atp);
            if (!action) {
                adjust_rec();     // update REC before erasing ACTIVE_TASK
            }
            iter = active_tasks.active_tasks.erase(iter);
            delete atp;
            set_client_state_dirty("handle_finished_apps");

            // the following is critical; otherwise the result is
            // still in the "scheduled" list and enforce_schedule()
            // will try to run it again.
            //
            request_schedule_cpus("handle_finished_apps");
            action = true;
            break;
        default:
            ++iter;
        }
    }
    return action;
}

// Handle a task that has finished.
// Mark its output files as present, and delete scratch files.
// Don't delete input files because they might be shared with other WUs.
// Update state of result record.
//
int CLIENT_STATE::app_finished(ACTIVE_TASK& at) {
    RESULT* rp = at.result;
    bool had_error = false;

#ifndef SIM
    FILE_INFO* fip;
    unsigned int i;
    char path[MAXPATHLEN];
    int retval;
    double size;

    // scan the output files, check if missing or too big.
    // Don't bother doing this if result was aborted via GUI or by project
    //
    switch (rp->exit_status) {
    case EXIT_ABORTED_VIA_GUI:
    case EXIT_ABORTED_BY_PROJECT:
        break;
    default:
        for (i=0; i<rp->output_files.size(); i++) {
            FILE_REF& fref = rp->output_files[i];
            fip = fref.file_info;
            if (fip->uploaded) continue;
            get_pathname(fip, path, sizeof(path));
            retval = file_size(path, size);
            if (retval) {
                if (fref.optional) {
                    fip->upload_urls.clear();
                    continue;
                }

                // an output file is unexpectedly absent.
                //
                fip->status = retval;
                had_error = true;
                msg_printf(
                    rp->project, MSG_INFO,
                    "Output file %s for task %s absent",
                    fip->name, rp->name
                );
            } else if (size > fip->max_nbytes) {
                // Note: this is only checked when the application finishes.
                // The total disk space is checked while the application is running.
                //
                msg_printf(
                    rp->project, MSG_INFO,
                    "Output file %s for task %s exceeds size limit.",
                    fip->name, rp->name
                );
                msg_printf(
                    rp->project, MSG_INFO,
                    "File size: %f bytes.  Limit: %f bytes",
                    size, fip->max_nbytes
                );

                fip->delete_file();
                fip->status = ERR_FILE_TOO_BIG;
                had_error = true;
            } else {
                if (!fip->uploadable() && !fip->sticky) {
                    fip->delete_file();     // sets status to NOT_PRESENT
                } else {
                    retval = 0;
                    if (fip->gzip_when_done) {
                        retval = fip->gzip();
                    }
                    if (!retval) {
                        retval = md5_file(path, fip->md5_cksum, fip->nbytes);
                    }
                    if (retval) {
                        fip->status = retval;
                        had_error = true;
                    } else {
                        fip->status = FILE_PRESENT;
                    }
                }
            }
        }
    }
#endif

    if (rp->exit_status != 0) {
        had_error = true;
    }

    if (had_error) {
        switch (rp->exit_status) {
        case EXIT_ABORTED_VIA_GUI:
        case EXIT_ABORTED_BY_PROJECT:
        case EXIT_OVERDUE_EXCEEDED:
            rp->set_state(RESULT_ABORTED, "CS::app_finished");
            break;
        default:
            rp->set_state(RESULT_COMPUTE_ERROR, "CS::app_finished");
        }
        rp->project->njobs_error++;
    } else {
#ifdef SIM
        rp->set_state(RESULT_FILES_UPLOADED, "CS::app_finished");
        rp->set_ready_to_report();
        rp->completed_time = now;
#else
        rp->set_state(RESULT_FILES_UPLOADING, "CS::app_finished");
        rp->append_log_record();
#endif
        rp->project->update_duration_correction_factor(&at);
        rp->project->njobs_success++;
    }

    double elapsed_time = now - rec_interval_start;
    work_fetch.accumulate_inst_sec(&at, elapsed_time);

    rp->project->pwf.request_if_idle_and_uploading = true;
        // set this to allow work fetch if idle instance,
        // even before upload finishes

    return 0;
}

// Check whether the input and app version files for a result are
// marked as FILE_PRESENT.
// If check_size is set, also check whether they exist and have the right size.
// (Side-effect: files with a size mismatch will be deleted.)
// Side effect: files with size mismatch are deleted.
//
// If fipp is nonzero, return a pointer to offending FILE_INFO on error
//
// Called from:
// CLIENT_STATE::update_results (with check_size=false)
//      to transition result from DOWNLOADING to DOWNLOADED.
// ACTIVE_TASK::start() (with check_size=true)
//      to check files before running a task
//
int CLIENT_STATE::task_files_present(
    RESULT* rp, bool check_size, FILE_INFO** fipp
) {
    WORKUNIT* wup = rp->wup;
    FILE_INFO* fip;
    unsigned int i;
    APP_VERSION* avp = rp->avp;
    int retval, ret = 0;

    for (i=0; i<avp->app_files.size(); i++) {
        fip = avp->app_files[i].file_info;
        if (fip->status != FILE_PRESENT) {
            if (fipp) *fipp = fip;
            ret = ERR_FILE_MISSING;
        } else if (check_size) {
            retval = fip->check_size();
            if (retval) {
                if (fipp) *fipp = fip;
                ret = retval;
            }
        }
    }

    for (i=0; i<wup->input_files.size(); i++) {
        if (wup->input_files[i].optional) continue;
        fip = wup->input_files[i].file_info;
        if (fip->status != FILE_PRESENT) {
            if (fipp) *fipp = fip;
            ret = ERR_FILE_MISSING;
        } else if (check_size) {
            retval = fip->check_size();
            if (retval) {
                if (fipp) *fipp = fip;
                ret = retval;
            }
        }
    }
    return ret;
}

// The app for the given result failed to start.
// Verify the app version files; maybe one of them was corrupted.
//
int CLIENT_STATE::verify_app_version_files(RESULT* rp) {
    int ret = 0;
    FILE_INFO* fip;
    PROJECT* project = rp->project;

    if (project->anonymous_platform) return 0;
    APP_VERSION* avp = rp->avp;
    for (unsigned int i=0; i<avp->app_files.size(); i++) {
        fip = avp->app_files[i].file_info;
        int retval = fip->verify_file(true, true, false);
        if (retval && log_flags.task_debug) {
            msg_printf(fip->project, MSG_INFO,
                "app version file %s: bad contents",
                fip->name
            );
            ret = retval;
        }
    }
    return ret;
}

inline double force_fraction(double f) {
    if (f < 0) return 0;
    if (f > 1) return 1;
    return f;
}

double CLIENT_STATE::get_fraction_done(RESULT* result) {
    ACTIVE_TASK* atp = active_tasks.lookup_result(result);
    return atp ? force_fraction(atp->fraction_done) : 0.0;
}

// Find latest version of app for given platform
// or -1 if can't find one
//
int CLIENT_STATE::latest_version(APP* app, char* platform) {
    unsigned int i;
    int best = -1;

    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION* avp = app_versions[i];
        if (avp->app != app) continue;
        if (strcmp(platform, avp->platform)) continue;
        if (avp->version_num < best) continue;
        best = avp->version_num;
    }
    return best;
}

// Find the ACTIVE_TASK in the current set with the matching PID
//
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_pid(int pid) {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK *atp = active_tasks[i];
        if (atp->pid == pid) return atp;
    }
    return NULL;
}

// Find the ACTIVE_TASK in the current set with the matching result
//
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_result(RESULT* result) {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK *atp = active_tasks[i];
        if (atp->result == result) {
            return atp;
        }
    }
    return NULL;
}

ACTIVE_TASK* ACTIVE_TASK_SET::lookup_slot(int slot) {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK *atp = active_tasks[i];
        if (atp->slot == slot) {
            return atp;
        }
    }
    return NULL;
}

#ifndef SIM
// on startup, see if any active tasks have a finished file
// i.e. they finished as the client was shutting down
//
void ACTIVE_TASK_SET::check_for_finished_jobs() {
    for (unsigned int i=0; i<active_tasks.size(); i++) {
        ACTIVE_TASK* atp = active_tasks[i];
        int exit_code;
        if (atp->finish_file_present(exit_code)) {
            msg_printf(atp->wup->project, MSG_INFO,
                "Found finish file for %s; exit code %d",
                atp->result->name, exit_code
            );
            atp->handle_exited_app(exit_code);
        }
    }
}
#endif

// check for overdue results once/day
// called at startup and once/sec after
//
void CLIENT_STATE::check_overdue() {
    static double t = 0;
    if (now < t) return;
    active_tasks.report_overdue();
    t = now + 86400;
}

////////////// DOCKER CLEANUP ///////////////////

// lists of image and container names for active jobs
//
struct DOCKER_JOB_INFO {
    vector<string> images;
    vector<string> containers;
    bool image_present(string name) {
        return std::find(images.begin(), images.end(), name) != images.end();
    }
    bool container_present(string name) {
        return std::find(containers.begin(), containers.end(), name) != containers.end();
    }
};

// clean up a Docker installation
// (Unix: the host; Win: a WSL distro)
//
void cleanup_docker(DOCKER_JOB_INFO &info, DOCKER_CONN &dc) {
    int retval;
    vector<string> out, out2;
    char cmd[1024];
    string name;

    // debug
    // dc.verbose = true;
    // dc.command("system info; printenv", out);

    // first containers
    //
    retval = dc.command("ps --all", out);
    if (retval) {
        fprintf(stderr, "Docker command failed: ps --all\n");
    } else {
        for (string line: out) {
            retval = dc.parse_container_name(line, name);
            if (retval) continue;
            if (!docker_is_boinc_name(name.c_str())) continue;
            if (info.container_present(name)) continue;
            sprintf(cmd, "rm %s", name.c_str());
            retval = dc.command(cmd, out2);
            if (retval) {
                fprintf(stderr, "Docker command failed: %s\n", cmd);
                continue;
            }
            msg_printf(NULL, MSG_INFO,
                "Removed unused Docker container: %s", name.c_str()
            );
        }
    }

    // then images
    //
    retval = dc.command("images", out);
    if (retval) {
        fprintf(stderr, "Docker command failed: images\n");
    } else {
        for (string line: out) {
            retval = dc.parse_image_name(line, name);
            if (retval) continue;
            if (!docker_is_boinc_name(name.c_str())) continue;
            if (info.image_present(name)) continue;
            sprintf(cmd, "image rm %s", name.c_str());
            retval = dc.command(cmd, out2);
            if (retval) {
                fprintf(stderr, "Docker command failed: %s\n", cmd);
                continue;
            }
            msg_printf(NULL, MSG_INFO,
                "Removed unused Docker image: %s", name.c_str()
            );
        }
    }
}

// remove old BOINC images and containers from Docker installations
//
void CLIENT_STATE::docker_cleanup() {
    // make lists of the images and containers used by active jobs
    //
    DOCKER_JOB_INFO info;
    for (ACTIVE_TASK *atp: active_tasks.active_tasks) {
        if (!strstr(atp->app_version->plan_class, "docker")) continue;
        char buf[256];
        escape_project_url(atp->wup->project->master_url, buf);
        string s = docker_image_name(buf, atp->wup->name);
        info.images.push_back(s);
        s = docker_container_name(buf, atp->result->name);
        info.containers.push_back(s);
    }

    // go through local Docker installations and remove
    // BOINC images and containers not in the above lists
    //
#ifdef _WIN32
    for (WSL_DISTRO &wd: host_info.wsl_distros.distros) {
        if (wd.docker_version.empty()) continue;
        DOCKER_CONN dc;
        dc.init(wd.docker_type, wd.distro_name);
        cleanup_docker(info, dc);
    }
#else
    if (strlen(host_info.docker_version)) {
        DOCKER_CONN dc;
        dc.init(host_info.docker_type);
        cleanup_docker(info, dc);
    }
#endif
}

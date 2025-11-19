// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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
// try to send a job for the given app; used for non-compute-intensive apps

// Logic for sending non-compute-intensive (NCI) jobs

#include "sched_check.h"
#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_version.h"

#include "sched_nci.h"

static bool can_send_nci(
    WU_RESULT& wu_result,
    WORKUNIT& wu,
    BEST_APP_VERSION* &bavp,
    APP* app
) {
    bavp = get_app_version(wu, true, false);
    if (!bavp) {
        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] [WU#%lu] No app version for NCI job; skipping\n",
                wu.id
            );
        }
        return false;
    }
    int retval = wu_is_infeasible_fast(
        wu,
        wu_result.res_server_state, wu_result.res_priority,
        wu_result.res_report_deadline,
        *app, *bavp
    );
    if (retval) {
        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[send_job] [WU#%lu] wu_is_infeasible_fast() failed for NCI job; skipping\n",
                wu.id
            );
        }
        return false;
    }
    return true;
}

static int send_job_for_app(APP& app) {
    BEST_APP_VERSION* bavp;
    SCHED_DB_RESULT result;

    lock_sema();
    for (int i=0; i<ssp->max_wu_results; i++) {
        WU_RESULT& wu_result = ssp->wu_results[i];
        if (wu_result.state != WR_STATE_PRESENT && wu_result.state != g_pid) {
            continue;
        }
        WORKUNIT wu = wu_result.workunit;

        if (wu.appid != app.id) continue;
        if (is_buda(wu)) {
            log_messages.printf(MSG_CRITICAL,
                "BUDA NCI jobs are not currently supported"
            );
            return -1;
        }

        if (!can_send_nci(wu_result, wu, bavp, &app)) {
            // All jobs for a given NCI app are identical.
            // If we can't send one, we can't send any.
            //
            unlock_sema();
            log_messages.printf(MSG_NORMAL,
                "can_send_nci() failed for NCI job\n"
            );
            return -1;
        }
        wu_result.state = g_pid;
        unlock_sema();
        result.id = wu_result.resultid;
        wu_result.state = WR_STATE_EMPTY;
        if (result_still_sendable(result, wu)) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "Sending non-CPU-intensive job: %s\n", wu.name
                );
            }
            add_result_to_reply(result, wu, bavp, bavp->host_usage, NULL, false);
            return 0;
        }
        log_messages.printf(MSG_NORMAL,
            "NCI job was not still sendable\n"
        );
        lock_sema();
    }
    log_messages.printf(MSG_NORMAL,
        "no sendable NCI jobs for %s\n", app.user_friendly_name
    );
    unlock_sema();
    return 1;
}

// try to send jobs for non-CPU-intensive (NCI) apps
// for which the host doesn't have a job in progress
//
int send_nci() {
    int retval;
    vector<APP> nci_apps;
    char buf[1024];

    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL, "checking for NCI jobs\n");
    }

    // make a vector of NCI apps
    //
    for (int i=0; i<ssp->napps; i++) {
        if (!ssp->apps[i].non_cpu_intensive) continue;
        APP app = ssp->apps[i];
        app.have_job = false;
        nci_apps.push_back(app);
    }

    // scan through the list of in-progress jobs,
    // flagging the associated apps as having jobs
    //
    for (unsigned int i=0; i<g_request->other_results.size(); i++) {
        DB_RESULT r;
        OTHER_RESULT &ores = g_request->other_results[i];
        sprintf(buf, "where name='%s'", ores.name);
        retval = r.lookup(buf);
        if (retval) {
            log_messages.printf(MSG_NORMAL, "No such result: %s\n", ores.name);
            continue;
        }
        for (unsigned int j=0; j<nci_apps.size(); j++) {
            APP& app = nci_apps[j];
            if (app.id == r.appid) {
                app.have_job = true;
                break;
            }
        }
    }

    // For each NCI app w/o a job, try to send one
    //
    for (unsigned int i=0; i<nci_apps.size(); i++) {
        APP& app = nci_apps[i];
        if (app.have_job) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "Already have job for %s\n", app.name
                );
            }
            continue;
        }
        if (app.beta  && !g_wreq->project_prefs.allow_beta_work) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL, "%s is beta\n", app.name);
            }
            continue;
        }
        if (app_not_selected(app.id)) {
            if (!g_wreq->project_prefs.allow_non_preferred_apps) {
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "%s is not selected\n", app.name
                    );
                }
                continue;
            }
        }
        retval = send_job_for_app(app);
        if (retval) {
            log_messages.printf(MSG_NORMAL,
                "failed to send job for NCI app %s\n", app.user_friendly_name
            );
        }
    }
    return 0;
}

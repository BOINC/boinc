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

// High-level logic for communicating with scheduling servers,
// and for merging the result of a scheduler RPC into the client state

// The scheduler RPC mechanism is in scheduler_op.C

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <strings.h>
#include <map>
#include <set>
#endif

#include "crypt.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"
#include "util.h"

#include "client_msgs.h"
#include "scheduler_op.h"

#include "client_state.h"

using std::max;
using std::vector;
using std::string;

// quantities like avg CPU time decay by a factor of e every week
//
#define EXP_DECAY_RATE  (1./(SECONDS_PER_DAY*7))

// how often to show user "backing off" messages
//
const int SECONDS_BEFORE_REPORTING_MIN_RPC_TIME_AGAIN = 60*60;


// try to report results this much before their deadline
//
#define REPORT_DEADLINE_CUSHION SECONDS_PER_DAY

// how many CPUs should this project occupy on average,
// based on its resource share relative to a given set
//
int CLIENT_STATE::proj_min_results(PROJECT* p, double subset_resource_share) {
    if (p->non_cpu_intensive) {
        return 1;
    }
    if (!subset_resource_share) return 1;   // TODO - fix
    return (int)(ceil(ncpus*p->resource_share/subset_resource_share));
}

void PROJECT::set_min_rpc_time(double future_time) {
    if (future_time > min_rpc_time) {
        min_rpc_time = future_time;
        msg_printf(this, MSG_INFO,
            "Deferring scheduler requests for %s\n",
            timediff_format(min_rpc_time - gstate.now).c_str()
        );
    }
    min_report_min_rpc_time = 0;
}

// Return true iff we should not contact the project yet.
// Print a message to the user if we haven't recently
//
bool PROJECT::waiting_until_min_rpc_time() {
    return (min_rpc_time > gstate.now);
}

// find a project that needs to have its master file fetched
//
PROJECT* CLIENT_STATE::next_project_master_pending() {
    unsigned int i;
    PROJECT* p;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time()) continue;
        if (p->suspended_via_gui) continue;
        if (p->master_url_fetch_pending) {
            return p;
        }
    }
    return 0;
}

// find a project for which a scheduler RPC is pending
// and we're not backed off
//
PROJECT* CLIENT_STATE::next_project_sched_rpc_pending() {
    unsigned int i;
    PROJECT* p;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time()) continue;
        //if (p->suspended_via_gui) continue;
        // do the RPC even if suspended.
        // This is critical for acct mgrs, to propagate new host CPIDs
        //
        if (p->sched_rpc_pending) {
            return p;
        }
    }
    return 0;
}

PROJECT* CLIENT_STATE::next_project_trickle_up_pending() {
    unsigned int i;
    PROJECT* p;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time()) continue;
        if (p->suspended_via_gui) continue;
        if (p->trickle_up_pending) {
            return p;
        }
    }
    return 0;
}

// Return the best project to fetch work from, NULL if none
//
// Pick the one with largest (long term debt - amount of current work)
//
// PRECONDITIONS:
//   - work_request_urgency and work_request set for all projects
//   - CLIENT_STATE::overall_work_fetch_urgency is set
// (by previous call to compute_work_requests())
//
PROJECT* CLIENT_STATE::next_project_need_work() {
    PROJECT *p, *p_prospect = NULL;
    double work_on_prospect=0;
    unsigned int i;
    double prrs = potentially_runnable_resource_share();

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->work_request_urgency == WORK_FETCH_DONT_NEED) continue;
        if (p->work_request == 0) continue;
        if (!p->contactable()) continue;

        // if we don't really need work,
        // and we don't really need work from this project, pass.
        //
        if (overall_work_fetch_urgency <= WORK_FETCH_OK) {
            if (p->work_request_urgency <= WORK_FETCH_OK) {
                continue;
            }
        }

        double work_on_current = time_until_work_done (p, 0, prrs);
        if (p_prospect) {
            if (p->work_request_urgency == WORK_FETCH_OK && 
                p_prospect->work_request_urgency > WORK_FETCH_OK
            ) {
                continue;
            }

            if (p->long_term_debt - work_on_current < p_prospect->long_term_debt - work_on_prospect
                && !p->non_cpu_intensive
            ) {
                continue;
            }
        }

        p_prospect = p;
        work_on_prospect = work_on_current;
    }
    if (p_prospect && (p_prospect->work_request <= 0)) {
        p_prospect->work_request = 1.0;
    }
    return p_prospect;
}

// Write a scheduler request to a disk file,
// to be sent to a scheduling server
//
int CLIENT_STATE::make_scheduler_request(PROJECT* p) {
    char buf[1024];
    MIOFILE mf;
    unsigned int i;
    RESULT* rp;
    int retval;
    double disk_total, disk_project;

    get_sched_request_filename(*p, buf);
    FILE* f = boinc_fopen(buf, "wb");

    double trs = total_resource_share();
    double rrs = runnable_resource_share();
    double prrs = potentially_runnable_resource_share();
    double resource_share_fraction, rrs_fraction, prrs_fraction;
    if (trs) {
        resource_share_fraction = p->resource_share / trs;
    } else {
        resource_share_fraction = 1;
    }
    if (rrs) {
        rrs_fraction = p->resource_share / rrs;
    } else {
        rrs_fraction = 1;
    }
    if (prrs) {
        prrs_fraction = p->resource_share / prrs;
    } else {
        prrs_fraction = 1;
    }

    // if hostid is zero, rpc_seqno better be also
    //
    if (!p->hostid) {
        p->rpc_seqno = 0;
    }

    if (!f) return ERR_FOPEN;
    mf.init_file(f);
    fprintf(f,
        "<scheduler_request>\n"
        "    <authenticator>%s</authenticator>\n"
        "    <hostid>%d</hostid>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <platform_name>%s</platform_name>\n"
        "    <core_client_major_version>%d</core_client_major_version>\n"
        "    <core_client_minor_version>%d</core_client_minor_version>\n"
        "    <core_client_release>%d</core_client_release>\n"
        "    <work_req_seconds>%f</work_req_seconds>\n"
        "    <resource_share_fraction>%f</resource_share_fraction>\n"
        "    <rrs_fraction>%f</rrs_fraction>\n"
        "    <prrs_fraction>%f</prrs_fraction>\n"
        "    <estimated_delay>%f</estimated_delay>\n"
        "    <duration_correction_factor>%f</duration_correction_factor>\n",
        p->authenticator,
        p->hostid,
        p->rpc_seqno,
        p->anonymous_platform?"anonymous":platform_name,
        core_client_major_version,
        core_client_minor_version,
        core_client_release,
        p->work_request,
        resource_share_fraction,
        rrs_fraction,
        prrs_fraction,
        time_until_work_done(p, proj_min_results(p, prrs)-1, prrs),
        p->duration_correction_factor
    );
    if (p->anonymous_platform) {
        fprintf(f, "    <app_versions>\n");
        for (i=0; i<app_versions.size(); i++) {
            APP_VERSION* avp = app_versions[i];
            if (avp->project != p) continue;
            avp->write(mf);
        }
        fprintf(f, "    </app_versions>\n");
    }
    if (strlen(p->code_sign_key)) {
        fprintf(f, "    <code_sign_key>\n%s</code_sign_key>\n", p->code_sign_key);
    }

    // insert global preferences if present
    //
    if (boinc_file_exists(GLOBAL_PREFS_FILE_NAME)) {
        FILE* fprefs = fopen(GLOBAL_PREFS_FILE_NAME, "r");
        if (fprefs) {
            copy_stream(fprefs, f);
            fclose(fprefs);
        }
        PROJECT* pp = lookup_project(global_prefs.source_project);
        if (pp && strlen(pp->email_hash)) {
            fprintf(f,
                "<global_prefs_source_email_hash>%s</global_prefs_source_email_hash>\n",
                pp->email_hash
            );
        }
    }

    // Of the projects with same email hash as this one,
    // send the oldest cross-project ID.
    // Use project URL as tie-breaker.
    //
    PROJECT* winner = p;
    for (i=0; i<projects.size(); i++ ) {
        PROJECT* project = projects[i];
        if (project == p) continue;
        if (strcmp(project->email_hash, p->email_hash)) continue;
        if (project->user_create_time < winner->user_create_time) {
            winner = project;
        } else if (project->user_create_time == winner->user_create_time) {
            if (strcmp(project->master_url, winner->master_url) < 0) {
                winner = project;
            }
        }
    }
    fprintf(f,
        "<cross_project_id>%s</cross_project_id>\n",
        winner->cross_project_id
    );

    retval = time_stats.write(mf, true);
    if (retval) return retval;
    retval = net_stats.write(mf);
    if (retval) return retval;

    // update hardware info, and write host info
    //
    host_info.get_host_info();
    retval = host_info.write(mf);
    if (retval) return retval;

    // get and write disk usage
    //
    total_disk_usage(disk_total);
    project_disk_usage(p, disk_project);
    fprintf(f,
        "    <disk_usage>\n"
        "        <d_boinc_used_total>%f</d_boinc_used_total>\n"
        "        <d_boinc_used_project>%f</d_boinc_used_project>\n"
        "    </disk_usage>\n",
        disk_total, disk_project
    );

    // report results
    //
    p->nresults_returned = 0;
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project == p && rp->ready_to_report) {
            p->nresults_returned++;
            rp->write(mf, true);
        }
    }

    read_trickle_files(p, f);

    // report sticky files as needed
    //
    for (i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if (fip->project != p) continue;
        if (!fip->report_on_rpc) continue;
        if (fip->marked_for_delete) continue;
        fprintf(f,
            "    <file_info>\n"
            "        <name>%s</name>\n"
            "        <nbytes>%f</nbytes>\n"
            "        <status>%d</status>\n"
            "        <report_on_rpc/>\n"
            "    </file_info>\n",
            fip->name, fip->nbytes, fip->status
        );
    }

    // send names of results in progress for this project
    //
    fprintf(f, "<other_results>\n");
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project == p && !rp->ready_to_report) {
            fprintf(f,
                "    <other_result>\n"
                "        <name>%s</name>\n"
                "    </other_result>\n",
                rp->name
            );
        }
    }
    fprintf(f, "</other_results>\n");

    // send summary of in-progress results
    // to give scheduler info on our CPU commitment
    //
    fprintf(f, "<in_progress_results>\n");
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        double x = rp->estimated_cpu_time_remaining();
        if (x == 0) continue;
        fprintf(f,
            "    <ip_result>\n"
            "        <report_deadline>%f</report_deadline>\n"
            "        <cpu_time_remaining>%f</cpu_time_remaining>\n"
            "    </ip_result>\n",
            rp->report_deadline,
            x
        );
    }
    fprintf(f, "</in_progress_results>\n");

    fprintf(f, "</scheduler_request>\n");

    fclose(f);
    return 0;
}

// find a project with finished results that should be reported.
// This means:
//    - we're not backing off contacting the project
//    - the result is ready_to_report (compute done; files uploaded)
//    - we're either within a day of the report deadline,
//      or at least work_buf_min_days time has elapsed since
//      result was completed,
//      or we have a sporadic connection
//
PROJECT* CLIENT_STATE::find_project_with_overdue_results() {
    unsigned int i;
    RESULT* r;

    for (i=0; i<results.size(); i++) {
        r = results[i];
        // return the project for this result to report if:
        //

        PROJECT* p = r->project;
        if (p->waiting_until_min_rpc_time()) continue;
        if (p->suspended_via_gui) continue;

        if (!r->ready_to_report) continue;
        if (have_sporadic_connection) {
            return p;
        }
        if (gstate.now > r->report_deadline - REPORT_DEADLINE_CUSHION) {
            return p;
        }
        if (gstate.now > r->completed_time + global_prefs.work_buf_min_days*SECONDS_PER_DAY) {
            return p;
        }
    }

    return 0;
}

// return the expected number of CPU seconds completed by the client
// in a second of wall-clock time.
// May be > 1 on a multiprocessor.
//
double CLIENT_STATE::avg_proc_rate() {
    double running_frac = time_stats.on_frac * time_stats.active_frac;
    if (running_frac < 0.1) running_frac = 0.1;
    if (running_frac > 1) running_frac = 1;
    return ncpus*running_frac*time_stats.cpu_efficiency;
}

// estimate wall-clock time until the number of uncompleted results
// for project p will reach k,
// given the total resource share of a set of competing projects
//
double CLIENT_STATE::time_until_work_done(
    PROJECT *p, int k, double subset_resource_share
) {
    int num_results_to_skip = k;
    double est = 0;
    
    // total up the estimated time for this project's unstarted
    // and partially completed results,
    // omitting the last k
    //
    for (vector<RESULT*>::reverse_iterator iter = results.rbegin();
         iter != results.rend(); iter++
    ) {
        RESULT *rp = *iter;
        if (rp->project != p
            || rp->state > RESULT_FILES_DOWNLOADED
            || rp->ready_to_report
        ) continue;
        if (num_results_to_skip > 0) {
            --num_results_to_skip;
            continue;
        }
        if (rp->project->non_cpu_intensive) {
            // if it is a non_cpu intensive project,
            // it needs only one at a time.
            //
            est = max(rp->estimated_cpu_time_remaining(), global_prefs.work_buf_min_days * SECONDS_PER_DAY);  
        } else {
            est += rp->estimated_cpu_time_remaining();
        }
    }
    if (subset_resource_share) {
        double apr = avg_proc_rate()*p->resource_share/subset_resource_share;
        return est/apr;
    } else {
        return est/avg_proc_rate();     // TODO - fix
    }
}

// Top-level function for work fetch policy.
// Outputs:
// - overall_work_fetch_urgency
// - for each contactable project:
//     - work_request and work_request_urgency
//
int CLIENT_STATE::compute_work_requests() {
    unsigned int i;
    double work_min_period = global_prefs.work_buf_min_days * SECONDS_PER_DAY;
    double global_work_need = work_needed_secs();
    double prrs;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    overall_work_fetch_urgency = WORK_FETCH_DONT_NEED;
    for (i=0; i< projects.size(); i++) {
        projects[i]->work_request_urgency = WORK_FETCH_DONT_NEED;
        projects[i]->work_request = 0;
    }


    if (!should_get_work()) {
        scope_messages.printf("compute_work_requests(): we don't need any work\n");
        overall_work_fetch_urgency = WORK_FETCH_DONT_NEED;
        return 0;
    } else if (no_work_for_a_cpu()) {
        scope_messages.printf("compute_work_requests(): CPU is idle\n");
        overall_work_fetch_urgency = WORK_FETCH_NEED_IMMEDIATELY;
    } else if (global_work_need > 0) {
        scope_messages.printf("compute_work_requests(): global work needed is greater than zero\n");
        overall_work_fetch_urgency = WORK_FETCH_NEED;
    } else {
        overall_work_fetch_urgency = WORK_FETCH_OK;
    }

    double max_fetch = work_min_period;

    // it is possible to have a work fetch policy of no new work and also have 
    // a CPU idle or not enough to fill the cache.
    // In this case, we get work, but in small increments
    // as we are already in trouble and we need to minimize the damage.
    //
    if (work_fetch_no_new_work) {
        max_fetch = 1.0;
    }

    prrs = potentially_runnable_resource_share();

    // for each project, compute
    // min_results = min # of results for project needed by CPU scheduling,
    // to avoid "starvation".
    // Then estimate how long it's going to be until we have fewer
    // than this # of results remaining.
    //
    for (i=0; i<projects.size(); i++) {
        PROJECT *p = projects[i];

        p->work_request = 0;
        p->work_request_urgency = WORK_FETCH_DONT_NEED;
        if (!p->contactable()) continue;

        // if system has been running in round robin,
        // then all projects will have a LT debt greater than 
        // -global_prefs.cpu_scheduling_period_minutes * 60
        // Therefore any project that has a LT debt greater than this 
        // is a candidate for more work.
        // Also if the global need is immediate, we need to get work from 
        // any contactable project, even if its LT debt is extremely negative.
        // Also, if there is only one potentially runnable project,
        // we can get work from it no matter what.
        //
        if ((p->long_term_debt < -global_prefs.cpu_scheduling_period_minutes*60)
            && (overall_work_fetch_urgency != WORK_FETCH_NEED_IMMEDIATELY)
            && (prrs != p->resource_share)
        ) {
            continue;
        }

        // if it is non cpu intensive and we have work, we don't need any more.
        //
        if (p->non_cpu_intensive && p->runnable()) continue;

        int min_results = proj_min_results(p, prrs);
        double estimated_time_to_starvation = time_until_work_done(p, min_results-1, prrs);

        // determine project urgency
        //
        if (estimated_time_to_starvation < work_min_period) {
            if (estimated_time_to_starvation == 0) {
                scope_messages.printf(
                    "CLIENT_STATE::compute_work_requests(): project '%s' is starved\n",
                    p->project_name
                );
                p->work_request_urgency = WORK_FETCH_NEED_IMMEDIATELY;
            } else {
                scope_messages.printf(
                    "CLIENT_STATE::compute_work_requests(): project '%s' will starve in %.2f sec\n",
                    p->project_name, estimated_time_to_starvation
                );
                p->work_request_urgency = WORK_FETCH_NEED;
            }
            // determine work requests for each project
            // NOTE: don't need to divide by active_frac etc.;
            // the scheduler does that (see sched/sched_send.C)
            //
            p->work_request = max(0.0,
                //(2*work_min_period - estimated_time_to_starvation)
                (work_min_period - estimated_time_to_starvation)
                * ncpus
            );

        } else if (overall_work_fetch_urgency > WORK_FETCH_OK) {
            p->work_request_urgency = WORK_FETCH_OK;
            p->work_request = max(global_work_need, 1.0);
                //In the case of an idle CPU, we need at least one second.
        }

        scope_messages.printf(
            "CLIENT_STATE::compute_work_requests(): project %s work req: %f sec  urgency: %d\n",
            p->project_name, p->work_request, p->work_request_urgency
        );
    }

    scope_messages.printf(
        "CLIENT_STATE::compute_work_requests(): client work need: %f sec, urgency %d\n",
        global_work_need, overall_work_fetch_urgency
    );

    return 0;
}

// called from the client's polling loop.
// initiate scheduler RPC activity if needed and possible
//
bool CLIENT_STATE::scheduler_rpc_poll() {
    overall_work_fetch_urgency = WORK_FETCH_DONT_NEED;
    PROJECT *p;
    bool action=false;
    static double last_time=0;

    if (!have_tentative_project && gstate.now - last_time < 5.0) return false;
    last_time = gstate.now;

    switch(scheduler_op->state) {
    case SCHEDULER_OP_STATE_IDLE:
        if (scheduler_op->check_master_fetch_start()) {
            action = true;
            break;
        }

        compute_work_requests(); 

        // contact project requested by user
        //
        p = next_project_sched_rpc_pending();
        if (p) {
            scheduler_op->init_op_project(p, REASON_USER_REQ);
            action = true;
            break;
        }
        if (network_suspended) break;
        p = next_project_trickle_up_pending();
        if (p) {
            scheduler_op->init_op_project(p, REASON_TRICKLE_UP);
            action = true;
            break;
        }
        
        // report overdue results
        //
        p = find_project_with_overdue_results();
        if (p) {
            scheduler_op->init_op_project(p, REASON_RESULTS_DUE);
            action = true;
            break;
        }
        if (!(exit_when_idle && contacted_sched_server) && overall_work_fetch_urgency != WORK_FETCH_DONT_NEED) {
            scheduler_op->init_get_work();
            if (scheduler_op->state != SCHEDULER_OP_STATE_IDLE) {
                break;
            }
        }
        break;
    default:
        scheduler_op->poll();
        if (scheduler_op->state == SCHEDULER_OP_STATE_IDLE) {
            action = true;
        }
        break;
    }
    return action;
}

// Handle the reply from a scheduler
//
int CLIENT_STATE::handle_scheduler_reply(
    PROJECT* project, char* scheduler_url, int& nresults
) {
    SCHEDULER_REPLY sr;
    FILE* f;
    int retval;
    unsigned int i;
    bool signature_valid, update_global_prefs=false, update_project_prefs=false;
    char buf[256], filename[256];
    std::string old_gui_urls = project->gui_urls;
    PROJECT* p2;

    nresults = 0;
    contacted_sched_server = true;
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_OP);

    get_sched_reply_filename(*project, filename);
    scope_messages.printf_file(filename, "reply: ");

    f = fopen(filename, "r");
    if (!f) return ERR_FOPEN;
    retval = sr.parse(f, project);
    fclose(f);
    if (retval) return retval;

    // check that master URL is correct
    //
    if (strlen(sr.master_url)) {
        canonicalize_master_url(sr.master_url);
        if (strcmp(sr.master_url, project->master_url)) {
            msg_printf(project, MSG_ERROR,
                "You are using the wrong URL for this project"
            );
            msg_printf(project, MSG_ERROR,
                "The correct URL is %s", sr.master_url
            );
            if (project->tentative) {
                return ERR_WRONG_URL;
            }
            p2 = gstate.lookup_project(sr.master_url);
            if (p2) {
                msg_printf(project, MSG_ERROR,
                    "Duplicate attachment detected - detach all projects named %s",
                    project->project_name
                );
                msg_printf(project, MSG_ERROR,
                    "Then reattach to %s", sr.master_url
                );
            } else {
                msg_printf(project, MSG_ERROR,
                    "Detach this project, then reattach to %s",
                    sr.master_url
                );
            }
        }
    }

    // make sure we don't already have a project of same name
    //
    if (project->tentative) {
        bool dup_name = false;
        for (i=0; i<gstate.projects.size(); i++) {
            p2 = gstate.projects[i];
            if (project == p2) continue;
            if (!strcmp(p2->project_name, project->project_name)) {
                dup_name = true;
                break;
            }
        }
        if (dup_name) {
            msg_printf(project, MSG_ERROR,
                "Already attached to a project named %s (possibly with wrong URL)",
                project->project_name
            );
            msg_printf(project, MSG_ERROR,
                "Consider detaching this project, then trying again"
            );
            return ERR_DUP_NAME;
        }
    }

    // on the off chance that this is the initial RPC for a project
    // being attached, copy messages to a safe place
    //
    for (i=0; i<sr.messages.size(); i++) {
        USER_MESSAGE& um = sr.messages[i];
        sprintf(buf, "Message from server: %s", um.message.c_str());
        int prio = (!strcmp(um.priority.c_str(), "high"))?MSG_ERROR:MSG_INFO;
        show_message(project, buf, prio);
        gstate.project_attach.messages.push_back(um.message);
    }

    // if project is down, return error (so that we back off)
    // and don't do anything else
    //
    if (sr.project_is_down) {
        if (sr.request_delay) {
            double x = gstate.now + sr.request_delay;
            if (x > project->min_rpc_time) project->min_rpc_time = x;
        }
        return ERR_PROJECT_DOWN;
    }

    // see if we have a new venue from this project
    //
    if (strlen(sr.host_venue) && strcmp(project->host_venue, sr.host_venue)) {
        safe_strcpy(project->host_venue, sr.host_venue);
        msg_printf(project, MSG_INFO, "New host venue: %s", sr.host_venue);
        update_project_prefs = true;
        if (project == global_prefs_source_project()) {
            strcpy(main_host_venue, sr.host_venue);
            update_global_prefs = true;
        }
    }

    // if the scheduler reply includes global preferences,
    // insert extra elements, write to disk, and parse
    //
    if (sr.global_prefs_xml) {
        f = boinc_fopen(GLOBAL_PREFS_FILE_NAME, "w");
        if (!f) return ERR_FOPEN;
        fprintf(f,
            "<global_preferences>\n"
        );

        // tag with the project and scheduler URL,
        // but only if not already tagged
        //
        if (!strstr(sr.global_prefs_xml, "<source_project>")) {
            fprintf(f,
                "    <source_project>%s</source_project>\n"
                "    <source_scheduler>%s</source_scheduler>\n",
                project->master_url,
                scheduler_url
            );
        }
        fprintf(f,
            "%s"
            "</global_preferences>\n",
            sr.global_prefs_xml
        );
        fclose(f);
        update_global_prefs = true;
    }

    if (update_global_prefs) {
        bool found_venue;
        retval = global_prefs.parse_file(
            GLOBAL_PREFS_FILE_NAME, project->host_venue, found_venue
        );
        if (retval) {
            msg_printf(project, MSG_ERROR, "Can't parse general preferences");
        } else {
            show_global_prefs_source(found_venue);
            int ncpus_old = ncpus;
            install_global_prefs();
            if (ncpus != ncpus_old) {
                msg_printf(0, MSG_INFO,
                    "Number of usable CPUs has changed.  Running benchmarks."
                );
                run_cpu_benchmarks = true;
            }
        }
    }

    // deal with project preferences (should always be there)
    // If they've changed, write to account file,
    // then parse to get our venue, and pass to running apps
    //
    if (sr.project_prefs_xml) {
        if (strcmp(project->project_prefs.c_str(), sr.project_prefs_xml)) {
            project->project_prefs = string(sr.project_prefs_xml);
            update_project_prefs = true;
        }
    }

    // the account file has GUI URLs and project prefs.
    // rewrite if either of these has changed
    //
    if (project->gui_urls != old_gui_urls || update_project_prefs) {
        retval = project->write_account_file();
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "Can't write account file: %s", boincerror(retval)
            );
            return retval;
        }
    }

    if (update_project_prefs) {
        project->parse_account_file();
        project->parse_preferences_for_user_files();
        active_tasks.request_reread_prefs(project);
    }

    // if the scheduler reply includes a code-signing key,
    // accept it if we don't already have one from the project.
    // Otherwise verify its signature, using the key we already have.
    //

    if (sr.code_sign_key) {
        if (!strlen(project->code_sign_key)) {
            safe_strcpy(project->code_sign_key, sr.code_sign_key);
        } else {
            if (sr.code_sign_key_signature) {
                retval = verify_string2(
                    sr.code_sign_key, sr.code_sign_key_signature,
                    project->code_sign_key, signature_valid
                );
                if (!retval && signature_valid) {
                    safe_strcpy(project->code_sign_key, sr.code_sign_key);
                } else {
                    msg_printf(project, MSG_ERROR,
                        "New code signing key doesn't validate"
                    );
                }
            } else {
                msg_printf(project, MSG_ERROR,
                    "Missing code sign key signature"
                );
            }
        }
    }

    // copy new entities to client state
    //
    for (i=0; i<sr.apps.size(); i++) {
        APP* app = lookup_app(project, sr.apps[i].name);
        if (!app) {
            app = new APP;
            *app = sr.apps[i];
            retval = link_app(project, app);
            if (retval) {
                msg_printf(project, MSG_ERROR,
                    "Can't handle application %s in scheduler reply", app->name
                );
                delete app;
            } else {
                apps.push_back(app);
            }
        }
    }
    FILE_INFO* fip;
    for (i=0; i<sr.file_infos.size(); i++) {
        fip = lookup_file_info(project, sr.file_infos[i].name);
        if (fip) {
            fip->merge_info(sr.file_infos[i]);
        } else {
            fip = new FILE_INFO;
            *fip = sr.file_infos[i];
            retval = link_file_info(project, fip);
            if (retval) {
                msg_printf(project, MSG_ERROR,
                    "Can't handle file %s in scheduler reply", fip->name
                );
                delete fip;
            } else {
                file_infos.push_back(fip);
            }
        }
    }
    for (i=0; i<sr.file_deletes.size(); i++) {
        fip = lookup_file_info(project, sr.file_deletes[i].c_str());
        if (fip) {
            msg_printf(project, MSG_INFO,
                "Got server request to delete file %s", fip->name
            );
            fip->marked_for_delete = true;
        }
    }
    for (i=0; i<sr.app_versions.size(); i++) {
        APP* app = lookup_app(project, sr.app_versions[i].app_name);
        APP_VERSION* avp = lookup_app_version(app, sr.app_versions[i].version_num);
        if (avp) {
            // if we had download failures, clear them
            //
            avp->clear_errors();
            continue;
        }
        avp = new APP_VERSION;
        *avp = sr.app_versions[i];
        retval = link_app_version(project, avp);
        if (retval) {
             msg_printf(project, MSG_ERROR,
                 "Can't handle application version %s %d in scheduler reply",
                 avp->app_name, avp->version_num
             );
             delete avp;
             continue;
        }
        app_versions.push_back(avp);
    }
    for (i=0; i<sr.workunits.size(); i++) {
        if (lookup_workunit(project, sr.workunits[i].name)) continue;
        WORKUNIT* wup = new WORKUNIT;
        *wup = sr.workunits[i];
        int vnum = choose_version_num(wup->app_name, sr);
        if (vnum < 0) {
            msg_printf(project, MSG_ERROR,
                "Can't find application version for task %s", wup->name
            );
            delete wup;
            continue;
        }

        wup->version_num = vnum;
        retval = link_workunit(project, wup);
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "Can't handle task %s in scheduler reply", wup->name
            );
            delete wup;
            continue;
        }
        wup->clear_errors();
        workunits.push_back(wup);
    }
    for (i=0; i<sr.results.size(); i++) {
        if (lookup_result(project, sr.results[i].name)) {
            msg_printf(project, MSG_ERROR,
                "Already have task %s\n", sr.results[i].name
            );
            continue;
        }
        RESULT* rp = new RESULT;
        *rp = sr.results[i];
        retval = link_result(project, rp);
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "Can't handle task %s in scheduler reply", rp->name
            );
            delete rp;
            continue;
        }
        results.push_back(rp);
        rp->state = RESULT_NEW;
        nresults++;
    }

    // update records for ack'ed results
    //
    for (i=0; i<sr.result_acks.size(); i++) {
        RESULT* rp = lookup_result(project, sr.result_acks[i].name);
        scope_messages.printf(
            "CLIENT_STATE::handle_scheduler_reply(): got ack for result %s\n",
            sr.result_acks[i].name
        );
        if (rp) {
            rp->got_server_ack = true;
        } else {
            msg_printf(project, MSG_ERROR,
                "Got ack for task %s, but can't find it", sr.result_acks[i].name
            );
        }
    }

    // remove acked trickle files
    //
    if (sr.message_ack) {
        remove_trickle_files(project);
    }
    if (sr.send_file_list) {
        project->send_file_list = true;
    }
    project->sched_rpc_pending = false;
    project->trickle_up_pending = false;

    // handle delay request
    //
    if (sr.request_delay) {
        double x = gstate.now + sr.request_delay;
        if (x > project->min_rpc_time) project->min_rpc_time = x;
    } else {
        project->min_rpc_time = 0;
    }

    // The project returns a hostid only if it has created a new host record.
    // In that case reset RPC seqno
    //
    if (sr.hostid) {
        if (project->hostid) {
            // if we already have a host ID for this project,
            // we must have sent it a stale seqno,
            // which usually means our state file was copied from another host.
            // So generate a new host CPID.
            //
            generate_new_host_cpid();
            msg_printf(project, MSG_INFO,
                "Generated new host CPID: %s", host_info.host_cpid
            );
        }
        //msg_printf(project, MSG_INFO, "Changing host ID from %d to %d", project->hostid, sr.hostid);
        project->hostid = sr.hostid;
        project->rpc_seqno = 0;
    }

    set_client_state_dirty("handle_scheduler_reply");
    scope_messages.printf("CLIENT_STATE::handle_scheduler_reply(): State after handle_scheduler_reply():\n");
    print_summary();
    return 0;
}

bool CLIENT_STATE::should_get_work() {
    // if there are fewer runnable results than CPUS, we need more work.
    //
    if (no_work_for_a_cpu()) return true;

    double tot_cpu_time_remaining = 0;
    for (unsigned int i=0; i<results.size(); i++) {
        tot_cpu_time_remaining += results[i]->estimated_cpu_time_remaining();
    }

    // ????? shouldn't we scale by ncpus?  by avg_proc_rate()??
    //
    if (tot_cpu_time_remaining < global_prefs.work_buf_min_days*SECONDS_PER_DAY) {
        return true;
    }

    set_work_fetch_mode();

    return !work_fetch_no_new_work;
}

// Decide on work-fetch policy
// Namely, set the variable work_fetch_no_new_work
// and print a message if we're changing its value
//
void CLIENT_STATE::set_work_fetch_mode() {
    bool should_not_fetch_work = false;
    double total_proc_rate = avg_proc_rate();
    double per_cpu_proc_rate = total_proc_rate/ncpus;
    double rrs = runnable_resource_share();

    if (rr_misses_deadline(per_cpu_proc_rate, rrs)) {
        if (!no_work_for_a_cpu()) {
            should_not_fetch_work = true;
        }
    } else {
        // if fetching more work would cause round-robin to
        // miss a deadline, don't fetch more work
        //
        PROJECT* p = next_project_need_work();
        if (p && !p->runnable()) {
            rrs += p->resource_share;
            if (rr_misses_deadline(per_cpu_proc_rate, rrs)) {
                should_not_fetch_work = true;
            }
        }
    }
    if (work_fetch_no_new_work && !should_not_fetch_work) {
        msg_printf(NULL, MSG_INFO, "Allowing work fetch again.");
    }

    if (!work_fetch_no_new_work && should_not_fetch_work) {
        msg_printf(NULL, MSG_INFO,
            "Suspending work fetch because computer is overcommitted."
        );
    }
    work_fetch_no_new_work = should_not_fetch_work;
}

double CLIENT_STATE::work_needed_secs() {
    double total_work = 0;
    for(unsigned int i=0; i<results.size(); i++) {
        if (results[i]->project->non_cpu_intensive) continue;
        total_work += results[i]->estimated_cpu_time_remaining();
    }
    double x = global_prefs.work_buf_min_days*SECONDS_PER_DAY*avg_proc_rate() - total_work;
    if (x < 0) {
        return 0;
    }
    return x;
}

void CLIENT_STATE::scale_duration_correction_factors(double factor) {
    if (factor <= 0) return;
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->duration_correction_factor *= factor;
    }
}

// Choose a new host CPID.
// If using account manager, do scheduler RPCs
// to all acct-mgr-attached projects to propagate the CPID
//
void CLIENT_STATE::generate_new_host_cpid() {
    host_info.generate_host_cpid();
    for (unsigned int i=0; i<projects.size(); i++) {
        if (projects[i]->attached_via_acct_mgr) {
            projects[i]->sched_rpc_pending = true;
            projects[i]->min_rpc_time = now + 15;
        }
    }
}

const char *BOINC_RCSID_d35a4a7711 = "$Id$";

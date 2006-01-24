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

// assume actual CPU utilization will be this multiple
// of what we've actually measured recently
//
#define CPU_PESSIMISM_FACTOR 0.9

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

// find a project for which the user has requested a scheduler RPC
//
PROJECT* CLIENT_STATE::next_project_sched_rpc_pending() {
    unsigned int i;
    PROJECT* p;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time()) continue;
        if (p->suspended_via_gui) continue;
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
// Basically, pick the one with largest long term debt - amount of current work
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

    for (i=0; i<projects.size(); ++i) {
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

// Compute:
// - work_request and work_request_urgency for all projects.
// - overall_work_fetch_urgency
//
// Only set non-zero work requests for projects that are contactable
//
int CLIENT_STATE::compute_work_requests() {
    unsigned int i;
    double work_min_period = global_prefs.work_buf_min_days * SECONDS_PER_DAY;
    double global_work_need = work_needed_secs();
    double prrs;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    overall_work_fetch_urgency = WORK_FETCH_DONT_NEED;
    for (i = 0; i < projects.size(); ++i) {
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
    for (i=0; i<projects.size(); ++i) {
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
    // In that case we should reset RPC seqno
    // and generate a new host CPID
    //
    if (sr.hostid) {
        project->hostid = sr.hostid;
        project->rpc_seqno = 0;
        generate_new_host_cpid();
        msg_printf(project, MSG_INFO, "Generated new host CPID: %s", host_info.host_cpid);
    }

    set_client_state_dirty("handle_scheduler_reply");
    scope_messages.printf("CLIENT_STATE::handle_scheduler_reply(): State after handle_scheduler_reply():\n");
    print_summary();
    return 0;
}

bool CLIENT_STATE::should_get_work() {
    // if there are fewer runnable results then CPUS, we need more work.
    //
    if (no_work_for_a_cpu()) return true;

    double tot_cpu_time_remaining = 0;
    for (unsigned int i = 0; i < results.size();++i) {
        tot_cpu_time_remaining += results[i]->estimated_cpu_time_remaining();
    }
    if (tot_cpu_time_remaining < global_prefs.work_buf_min_days*SECONDS_PER_DAY) {
        return true;
    }

    // if the CPU started this time period overloaded,
    // let it process for a while to get out of the CPU overload state.
    //
    if (!work_fetch_no_new_work) {
        set_scheduler_modes();
    }

    return !work_fetch_no_new_work;
}

void PROJECT::set_rrsim_proc_rate(double per_cpu_proc_rate, double rrs) {
    int nactive = (int)active.size();
    if (nactive == 0) return;
    double x;
    if (rrs) {
        x = resource_share/rrs;
    } else {
        x = 1;      // TODO - fix
    }

    // if this project has fewer active results than CPUs,
    // scale up its share to reflect this
    //
    if (nactive < gstate.ncpus) {
        x *= ((double)gstate.ncpus)/nactive;
    }

    // But its rate on a given CPU can't exceed the CPU speed
    //
    if (x>1) {
        x = 1;
    }
    rrsim_proc_rate = x*per_cpu_proc_rate*CPU_PESSIMISM_FACTOR;
}

// return true if we don't have enough runnable tasks to keep all CPUs busy
//
bool CLIENT_STATE::no_work_for_a_cpu() {
    unsigned int i;
    int count = 0;

    for (i=0; i< results.size(); i++){
        RESULT* rp = results[i];
        if (!rp->runnable_soon()) continue;
        if (rp->project->non_cpu_intensive) continue;
        count++;
    }
    return ncpus > count;
}

// return true if round-robin scheduling will miss a deadline
//
bool CLIENT_STATE::rr_misses_deadline(double per_cpu_proc_rate, double rrs) {
    PROJECT* p, *pbest;
    RESULT* rp, *rpbest;
    vector<RESULT*> active;
    unsigned int i;
    double x;
    vector<RESULT*>::iterator it;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    // Initilize the "active" and "pending" lists for each project.
    // These keep track of that project's results
    //
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->active.clear();
        p->pending.clear();
    }

    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (!rp->runnable()) continue;
        if (rp->aborted_via_gui) continue;
        if (rp->project->non_cpu_intensive) continue;
        rp->rrsim_cpu_left = rp->estimated_cpu_time_remaining();
        p = rp->project;
        if (p->active.size() < (unsigned int)ncpus) {
            active.push_back(rp);
            p->active.push_back(rp);
        } else {
            p->pending.push_back(rp);
        }
    }

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        p->set_rrsim_proc_rate(per_cpu_proc_rate, rrs);
    }

    // Simulation loop.  Keep going until work done
    //
    double sim_now = now;
    while (active.size()) {

        // compute finish times and see which result finishes first
        //
        rpbest = NULL;
        for (i=0; i<active.size(); i++) {
            rp = active[i];
            p = rp->project;
            rp->rrsim_finish_delay = rp->rrsim_cpu_left/p->rrsim_proc_rate;
            if (!rpbest || rp->rrsim_finish_delay < rpbest->rrsim_finish_delay) {
                rpbest = rp;
            }
        }

        // "rpbest" is first result to finish.  Does it miss its deadline?
        //
        double diff = sim_now + rpbest->rrsim_finish_delay - rpbest->report_deadline;
        if (diff > 0) {
            scope_messages.printf(
                "rr_sim: result %s misses deadline by %f\n", rpbest->name, diff
            );
            return true;
        }

        // remove *rpbest from active set,
        // and adjust CPU time left for other results
        //
        it = active.begin();
        while (it != active.end()) {
            rp = *it;
            if (rp == rpbest) {
                it = active.erase(it);
            } else {
                x = rp->project->rrsim_proc_rate*rpbest->rrsim_finish_delay;
                rp->rrsim_cpu_left -= x;
                it++;
            }
        }

        pbest = rpbest->project;

        // remove *rpbest from its project's active set
        //
        it = pbest->active.begin();
        while (it != pbest->active.end()) {
            rp = *it;
            if (rp == rpbest) {
                it = pbest->active.erase(it);
            } else {
                it++;
            }
        }

        // If project has more results, add one to active set.
        //
        if (pbest->pending.size()) {
            rp = pbest->pending[0];
            pbest->pending.erase(pbest->pending.begin());
            active.push_back(rp);
            pbest->active.push_back(rp);
        }

        // If all work done for a project, subtract that project's share
        // and recompute processing rates
        //
        if (pbest->active.size() == 0) {
            rrs -= pbest->resource_share;
            for (i=0; i<projects.size(); i++) {
                p = projects[i];
                p->set_rrsim_proc_rate(per_cpu_proc_rate, rrs);
            }
        }

        sim_now += rpbest->rrsim_finish_delay;
    }
    scope_messages.printf( "rr_sim: deadlines met\n");
    return false;
}

#if 0
// simulate weighted round-robin scheduling,
// and see if any result misses its deadline.
//
bool CLIENT_STATE::round_robin_misses_deadline(
    double per_cpu_proc_rate, double rrs
) {
    std::vector <double> booked_to;
    int k;
    unsigned int i, j;
    RESULT* rp;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    for (k=0; k<ncpus; k++) {
        booked_to.push_back(now);
    }
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (!p->runnable()) continue;
        double project_proc_rate;
        if (rrs) {
            project_proc_rate = per_cpu_proc_rate * (p->resource_share/rrs);
        } else {
            project_proc_rate = per_cpu_proc_rate;      // TODO - fix
        }
        for (j=0; j<results.size(); j++) {
            rp = results[j];
            if (rp->project != p) continue;
            if (!rp->runnable()) continue;
            double first = booked_to[0];
            int ifirst = 0;
            for (k=1; k<ncpus; k++) {
                if (booked_to[k] < first) {
                    first = booked_to[k];
                    ifirst = k;
                }
            }
            booked_to[ifirst] += rp->estimated_cpu_time_remaining()/project_proc_rate;
            scope_messages.printf("set_scheduler_modes() result %s: est %f, deadline %f\n",
                rp->name, booked_to[ifirst], rp->report_deadline
            );
            if (booked_to[ifirst] > rp->report_deadline) {
                return true;
            }
        }
    }
    return false;
}
#endif

#if 0
// Simulate what will happen if we do EDF schedule starting now.
// Go through non-done results in EDF order,
// keeping track in "booked_to" of how long each CPU is occupied
//
bool CLIENT_STATE::edf_misses_deadline(double per_cpu_proc_rate) {
    std::map<double, RESULT*>::iterator it;
    std::map<double, RESULT*> results_by_deadline;
    std::vector <double> booked_to;
    unsigned int i;
    int j;
    RESULT* rp;

    for (j=0; j<ncpus; j++) {
        booked_to.push_back(now);
    }

    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->computing_done()) continue;
        if (rp->project->non_cpu_intensive) continue;
        results_by_deadline[rp->report_deadline] = rp;
    }

    for (
        it = results_by_deadline.begin();
        it != results_by_deadline.end();
        it++
    ) {
        rp = (*it).second;

        // find the CPU that will be free first
        //
        double lowest_book = booked_to[0];
        int lowest_booked_cpu = 0;
        for (j=1; j<ncpus; j++) {
            if (booked_to[j] < lowest_book) {
                lowest_book = booked_to[j];
                lowest_booked_cpu = j;
            }
        }
        booked_to[lowest_booked_cpu] += rp->estimated_cpu_time_remaining()
            /(per_cpu_proc_rate*CPU_PESSIMISM_FACTOR);
        if (booked_to[lowest_booked_cpu] > rp->report_deadline) {
            return true;
        }
    }
    return false;
}
#endif

// Decide on modes for work-fetch and CPU sched policies.
// Namely, set the variables
// - work_fetch_no_new_work
// - cpu_earliest_deadline_first
// and print a message if we're changing their value
//
void CLIENT_STATE::set_scheduler_modes() {
    RESULT* rp;
    unsigned int i;
    bool should_not_fetch_work = false;
    bool use_earliest_deadline_first = false;
    double total_proc_rate = avg_proc_rate();
    double per_cpu_proc_rate = total_proc_rate/ncpus;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    double rrs = runnable_resource_share();
    if (rr_misses_deadline(per_cpu_proc_rate, rrs)) {
        // if round robin would miss a deadline, use EDF
        //
        use_earliest_deadline_first = true;
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

    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->computing_done()) continue;
        if (rp->project->non_cpu_intensive) continue;

        // Is the nearest deadline within a day?
        //
        if (rp->report_deadline - gstate.now < 60 * 60 * 24) {
            use_earliest_deadline_first = true; 
            scope_messages.printf(
                "set_scheduler_modes(): Less than 1 day until deadline.\n"
            );
        }

        // is there a deadline < twice the users connect period?
        //
        if (rp->report_deadline - gstate.now < global_prefs.work_buf_min_days * SECONDS_PER_DAY * 2) {
            use_earliest_deadline_first = true;
            scope_messages.printf(
                "set_scheduler_modes(): Deadline is before reconnect time.\n"
            );
        }
    }

    // display only when the policy changes to avoid once per second
    //
    if (work_fetch_no_new_work && !should_not_fetch_work) {
        msg_printf(NULL, MSG_INFO,
            "Allowing work fetch again."
        );
    }

    if (!work_fetch_no_new_work && should_not_fetch_work) {
        msg_printf(NULL, MSG_INFO,
            "Suspending work fetch because computer is overcommitted."
        );
    }

    if (cpu_earliest_deadline_first && !use_earliest_deadline_first) {
        msg_printf(NULL, MSG_INFO,
            "Resuming round-robin CPU scheduling."
        );
    }
    if (!cpu_earliest_deadline_first && use_earliest_deadline_first) {
        msg_printf(NULL, MSG_INFO,
            "Using earliest-deadline-first scheduling because computer is overcommitted."
        );
    }

    work_fetch_no_new_work = should_not_fetch_work;
    cpu_earliest_deadline_first = use_earliest_deadline_first;
}

double CLIENT_STATE::work_needed_secs() {
    double total_work = 0;
    for( unsigned int i = 0; i < results.size(); ++i) {
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
// Do scheduler RPCs to all projects to propagate the CPID
//
void CLIENT_STATE::generate_new_host_cpid() {
    host_info.generate_host_cpid();
    for (unsigned int i=0; i<projects.size(); i++) {
        projects[i]->sched_rpc_pending = true;
        projects[i]->min_rpc_time = 0;
    }
}

const char *BOINC_RCSID_d35a4a7711 = "$Id$";

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

static double trs;
// quantities like avg CPU time decay by a factor of e every week
//
#define EXP_DECAY_RATE  (1./(SECONDS_PER_DAY*7))

// how often to show user "backing off" messages
//
const int SECONDS_BEFORE_REPORTING_MIN_RPC_TIME_AGAIN = 60*60;


// try to report results this much before their deadline
//
#define REPORT_DEADLINE_CUSHION SECONDS_PER_DAY

// try to maintain no more than this load factor on the CPU.
//
#define MAX_CPU_LOAD_FACTOR 0.8

static int proj_min_results(PROJECT* p, int ncpus) {
    return p->non_cpu_intensive ? 1 : (int)(ceil(ncpus*p->resource_share/trs));
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
bool PROJECT::waiting_until_min_rpc_time(double now) {
    if (min_rpc_time > now ) {
        if (now >= min_report_min_rpc_time) {
            min_report_min_rpc_time = now + SECONDS_BEFORE_REPORTING_MIN_RPC_TIME_AGAIN;
            msg_printf(
                this, MSG_ERROR,
               "Deferring communication with project for %s\n",
               timediff_format(min_rpc_time - now).c_str()
            );
        }
        return true;
    }
    return false;
}

// find a project that needs to have its master file fetched
//
PROJECT* CLIENT_STATE::next_project_master_pending() {
    unsigned int i;
    PROJECT* p;
    double now = dtime();

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time(now)) continue;
        if (p->suspended_via_gui) continue;
        if (p->master_url_fetch_pending) {
            return p;
        }
    }
    return 0;
}

// find a project that needs to contact its scheduling server
//
PROJECT* CLIENT_STATE::next_project_sched_rpc_pending() {
    unsigned int i;
    double now = dtime();
    PROJECT* p;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time(now)) continue;
        if (p->suspended_via_gui) continue;
        if (p->sched_rpc_pending) {
            return p;
        }
    }
    return 0;
}

// return the next project after "old" that
// 1) is eligible for a scheduler RPC
// 2) has work_request > 0
// 3) has master_url_fetch_pending == false
// 4) has dont_request_more_work == false
//
PROJECT* CLIENT_STATE::next_project_need_work(PROJECT* old, int urgency) {
    PROJECT *p, *p_prospect = NULL;
    double work_on_prospect=0;
    double now = dtime();
    unsigned int i;
    bool found_old = (old == 0);
    bool cpu_idle = no_work_for_a_cpu();

    for (i=0; i<projects.size(); ++i) {
        p = projects[i];
        if (p == old) {
            found_old = true;
            continue;
        }
        if (p->master_url_fetch_pending) continue;
        if (p->waiting_until_min_rpc_time(now)) continue;
        if (p->suspended_via_gui) continue;
        if (p->dont_request_more_work) continue;
        if (p->long_term_debt < 0 && !cpu_idle) continue;
        if (p->work_request_urgency == WORK_FETCH_DONT_NEED) continue;

        // if we don't really need work,
        // and we don't really need work from this project, pass.
        //
        if (urgency <= WORK_FETCH_OK && p->work_request_urgency <= WORK_FETCH_OK) continue;

        // if there is a project for which a work request is OK
        // and one that has a higher priority,
        // take the one with the higher priority.
        //
        if (p_prospect && p->work_request_urgency == WORK_FETCH_OK && 
            p_prospect->work_request_urgency > p->work_request_urgency) {
        
            continue;
        }

        double work_on_current = ettprc(p, 0);
        if (p_prospect
            && p->long_term_debt - work_on_current < p_prospect->long_term_debt - work_on_prospect
            && !p->non_cpu_intensive
        ) {
            continue;
        }

        if (p->work_request > 0) {
            p_prospect = p;
            work_on_prospect = work_on_current;
        }
    }
    if (p_prospect && !(p_prospect->work_request > 0.0)) p_prospect->work_request = 1.0;
    return p_prospect;
}

// Write a scheduler request to a disk file
// (later sent to the scheduling server)
//
int CLIENT_STATE::make_scheduler_request(PROJECT* p, double work_req) {
    char buf[1024];

    get_sched_request_filename(*p, buf);
    FILE* f = boinc_fopen(buf, "wb");
    MIOFILE mf;
    unsigned int i;
    RESULT* rp;
    int retval;
#if 0
    double free, possible;
#endif

    trs = total_resource_share();

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
        "    <work_req_seconds>%f</work_req_seconds>\n"
        "    <resource_share_fraction>%f</resource_share_fraction>\n"
        "    <estimated_delay>%f</estimated_delay>\n",
        p->authenticator,
        p->hostid,
        p->rpc_seqno,
        p->anonymous_platform?"anonymous":platform_name,
        core_client_major_version,
        core_client_minor_version,
        work_req,
        p->resource_share / trs,
        ettprc(p, proj_min_results(p, ncpus)-1)
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
#if 0
    anything_free(free);
    fprintf(f, "    <project_disk_free>%f</project_disk_free>\n", free);
    total_potential_offender(p, possible);
    fprintf(f, "    <potentially_free_offender>%f</potentially_free_offender>\n", possible);
    total_potential_self(p, possible);
    fprintf(f, "    <potentially_free_self>%f</potentially_free_self>\n", possible);
#endif
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
    retval = host_info.write(mf);
    if (retval) return retval;
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project == p && rp->ready_to_report) {
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
    fprintf(f, "</scheduler_request>\n");

    fclose(f);
    return 0;
}

// find a project with results that are overdue to report,
// and which we're allowed to contact.
//
PROJECT* CLIENT_STATE::find_project_with_overdue_results() {
    unsigned int i;
    RESULT* r;
    double now = dtime();

    for (i=0; i<results.size(); i++) {
        r = results[i];
        // return the project for this result to report if:
        //    - we're not backing off a scheduler request for its project
        //    - we're ready_to_report (compute done; files uploaded)
        //    - we're almost at the report_deadline
        //

        PROJECT* p = r->project;
        if (p->waiting_until_min_rpc_time(now)) continue;
        if (p->suspended_via_gui) continue;

        if (!r->ready_to_report) continue;
        if (r->report_deadline <= (now + REPORT_DEADLINE_CUSHION)) {
            return p;
        }
    }

    return 0;
}

#if 0
// return true if we're allowed to do a scheduler RPC to at least one project
//
bool CLIENT_STATE::some_project_rpc_ok() {
    unsigned int i;
    double now = dtime();

    for (i=0; i<projects.size(); i++) {
        if (projects[i]->min_rpc_time < now) return true;
    }
    return false;
}
#endif

// return the expected number of CPU seconds completed by the client
// for project p in a second of wall-clock time.
// May be > 1 on a multiprocessor.
//
double CLIENT_STATE::avg_proc_rate(PROJECT *p) {
    double running_frac = time_stats.on_frac * time_stats.active_frac;
    if (running_frac < 0.1) running_frac = 0.1;
    if (running_frac > 1) running_frac = 1;
    return (p ? (p->resource_share / trs) : 1) * ncpus * running_frac;
}

// "estimated time to project result count"
// return the estimated wall-clock time until the
// number of results for project p will reach k
//
double CLIENT_STATE::ettprc(PROJECT *p, int k) {
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
            // if it is a non_cpu intensive project, it needs only one at a time.
            est = max(rp->estimated_cpu_time_remaining(), global_prefs.work_buf_min_days * SECONDS_PER_DAY);  
        } else {
            est += rp->estimated_cpu_time_remaining();
        }
    }
    double apr = avg_proc_rate(p);
    return est/apr;
}

// set work_request for each project and return the urgency level for
// requesting more work
// only set non-zero work requests for projects that are allowed to do
// a scheduler RPC
//
int CLIENT_STATE::compute_work_requests() {
    int urgency = WORK_FETCH_DONT_NEED;
    int highest_project_urgency = WORK_FETCH_DONT_NEED;
    unsigned int i;
    double work_min_period = global_prefs.work_buf_min_days * SECONDS_PER_DAY;
    double now = dtime();
    double global_work_need = work_needed_secs();

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_CPU);

    for (i = 0; i < projects.size(); ++i) {
        projects[i]->work_request_urgency = WORK_FETCH_DONT_NEED;
        projects[i]->work_request = 0;
    }


    if (!should_get_work()) {
        scope_messages.printf("CLIENT_STATE::compute_work_requests(): we don't need any work\n");
        return WORK_FETCH_DONT_NEED;
    } else if (no_work_for_a_cpu()) {
        scope_messages.printf("CLIENT_STATE::compute_work_requests(): CPU is idle\n");
        urgency = WORK_FETCH_NEED_IMMEDIATELY;
    } else if (global_work_need > 0) {
        scope_messages.printf("CLIENT_STATE::compute_work_requests(): global work needed is greater than one\n");
        urgency = WORK_FETCH_NEED;
    } else {
        urgency = WORK_FETCH_OK;
    }

    double max_fetch = work_min_period;
        // it is possible to have a work fetch policy of no new work and also have 
        // a CPU idle or not enough to fill the cache.  In this case, we get work, but in little tiny increments
        // as we are already in trouble and we need to minimize the damage.
    if (this->work_fetch_no_new_work) {
        max_fetch = 1.0;
    }

    trs = total_resource_share();

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
        if (p->master_url_fetch_pending) continue;
        if (p->waiting_until_min_rpc_time(now)) continue;
        if (p->dont_request_more_work) continue;
        if (p->suspended_via_gui) continue;
        if ((p->long_term_debt < 0) && (urgency != WORK_FETCH_NEED_IMMEDIATELY)) continue;

        int min_results = proj_min_results(p, ncpus);
        double estimated_time_to_starvation = ettprc(p, min_results-1);

        // determine urgency
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
        } else if (WORK_FETCH_OK < urgency) {
            p->work_request_urgency = WORK_FETCH_OK;
            p->work_request = global_work_need;
        }

        highest_project_urgency = max(highest_project_urgency, p->work_request_urgency);

        // determine work requests for each project
        // NOTE: don't need to divide by active_frac etc.;
        // the scheduler does that (see sched/sched_send.C)
        //
        p->work_request = max(0.0,
            //(2*work_min_period - estimated_time_to_starvation)
            (work_min_period - estimated_time_to_starvation)
            * ncpus
        );

        scope_messages.printf(
            "CLIENT_STATE::compute_work_requests(): project '%s' work req: %f sec\n",
            p->project_name, p->work_request
        );
    }

    scope_messages.printf(
        "CLIENT_STATE::compute_work_requests(): returning urgency '%d'\n",
        urgency
    );

    return urgency;
}

// called from the client's polling loop.
// initiate scheduler RPC activity if needed and possible
//
bool CLIENT_STATE::scheduler_rpc_poll(double now) {
    int urgency = WORK_FETCH_DONT_NEED;
    PROJECT *p;
    bool action=false;
    static double last_time=0;
    static double work_need_inform_time = 0;

    if (now - last_time < 1.0) return false;
    last_time = now;

    switch(scheduler_op->state) {
    case SCHEDULER_OP_STATE_IDLE:
        if (network_suspended) break;
        if (should_get_work()) {
            urgency = compute_work_requests(); 
        }
        
        // highest priority is to report overdue results
        //
        p = find_project_with_overdue_results();
        if (p) {
            scheduler_op->init_return_results(p);
            action = true;
            break;
        }
        if (!(exit_when_idle && contacted_sched_server) && urgency != WORK_FETCH_DONT_NEED) {
            if (work_need_inform_time < now) {
                if (urgency == WORK_FETCH_NEED) {
                    msg_printf(NULL, MSG_INFO,
                        "May run out of work in %.2f days; requesting more",
                        global_prefs.work_buf_min_days
                    );
                } else if (urgency == WORK_FETCH_NEED_IMMEDIATELY) {
                    msg_printf(NULL, MSG_INFO,
                        "Insufficient work; requesting more"
                    );
                }
                work_need_inform_time = now + 3600;
            }
            scheduler_op->init_get_work(urgency);
            if (scheduler_op->state != SCHEDULER_OP_STATE_IDLE) {
                break;
            }
        }
        if (scheduler_op->check_master_fetch_start()) {
            action = true;
            break;
        }
        p = next_project_sched_rpc_pending();
        if (p) {
            scheduler_op->init_return_results(p);
            action = true;
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

    for (i=0; i<sr.messages.size(); i++) {
        USER_MESSAGE& um = sr.messages[i];
        sprintf(buf, "Message from server: %s", um.message.c_str());
        int prio = (!strcmp(um.priority.c_str(), "high"))?MSG_ERROR:MSG_INFO;
        show_message(project, buf, prio);
    }

    // if project is down, return error (so that we back off)
    // and don't do anything else
    //
    if (sr.project_is_down) {
        return ERR_PROJECT_DOWN;
    }

    // The project returns a hostid only if it has created a new host record.
    // In that case we should reset RPC seqno
    //
    if (sr.hostid) {
        project->hostid = sr.hostid;
        project->rpc_seqno = 0;
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
            install_global_prefs();
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
    if (update_project_prefs) {
        retval = project->write_account_file();
        if (retval) {
            msg_printf(project, MSG_ERROR, "Can't write account file: %d", retval);
            return retval;
        }
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
                    msg_printf(project, MSG_ERROR, "New code signing key doesn't validate");
                }
            } else {
                msg_printf(project, MSG_ERROR, "Missing code sign key signature");
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
                    "Can't link app %s in sched reply", app->name
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
                    "Can't link file_info %s in sched reply", fip->name
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
            msg_printf(project, MSG_INFO, "Got server request to delete file %s\n", fip->name);
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
                 "Can't link app version %s %d in sched reply",
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
                "Can't find app version for WU %s", wup->name
            );
            delete wup;
            continue;
        }

        wup->version_num = vnum;
        retval = link_workunit(project, wup);
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "Can't link workunit %s in sched reply", wup->name
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
                "Already have result %s\n", sr.results[i].name
            );
            continue;
        }
        RESULT* rp = new RESULT;
        *rp = sr.results[i];
        retval = link_result(project, rp);
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "Can't link result %s in sched reply", rp->name
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
                "Got ack for result %s, can't find", sr.result_acks[i].name
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

    // handle delay request
    //
    if (sr.request_delay) {
        double x = dtime() + sr.request_delay;
        if (x > project->min_rpc_time) project->min_rpc_time = x;
    } else {
        project->min_rpc_time = 0;
    }

    set_client_state_dirty("handle_scheduler_reply");
    scope_messages.printf("CLIENT_STATE::handle_scheduler_reply(): State after handle_scheduler_reply():\n");
    print_summary();
    return 0;
}

bool CLIENT_STATE::should_get_work() {
    // if there are fewer wus available then CPUS, then we need more work.
    if (no_work_for_a_cpu()) return true;

    double tot_cpu_time_remaining = 0;
    for (unsigned int i = 0; i < results.size();++i) {
        tot_cpu_time_remaining += results[i]->estimated_cpu_time_remaining();
    }
    if (tot_cpu_time_remaining < global_prefs.work_buf_min_days * SECONDS_PER_DAY) return true;

    // if the CPU started this time period overloaded,
    // let it process for a while to get out of the CPU overload state.
    //
    if (!work_fetch_no_new_work) {
        set_cpu_scheduler_modes();
    }
    bool ret = !work_fetch_no_new_work;

    return ret;
}

// return true if we don't have enough runnable tasks to keep all CPUs busy
//
bool CLIENT_STATE::no_work_for_a_cpu() {
    unsigned int i = 0;
    int count = 0;
    for (i = 0; i < results.size(); i++){
        RESULT* rp = results[i];
        if (rp->project->non_cpu_intensive ) continue;
        if (RESULT_COMPUTE_ERROR <= rp->state) continue;
        if (rp->suspended_via_gui) continue;
        if (rp->project->suspended_via_gui) continue;
        count++;
    }
    return ncpus > count;
}

// decide on the CPU scheduler state
//
void CLIENT_STATE::set_cpu_scheduler_modes() {
    std::map<double, RESULT*> results_by_deadline;
    std::set<PROJECT*> projects_with_work;

    std::vector<RESULT*>::iterator it_u;
    for (it_u = results.begin(); it_u != results.end(); ++it_u) {
        if ((RESULT_COMPUTE_ERROR > (*it_u)->state) && !(*it_u)->project->non_cpu_intensive) {
            results_by_deadline[(*it_u)->report_deadline] = *it_u;
            projects_with_work.insert((*it_u)->project);
        }
    }

    bool should_not_fetch_work = false;
    bool use_earliest_deadline_first = false;
    double now;
    double frac_booked;
    std::vector <double> booked_to;
    now = dtime();
    frac_booked = 0;
    for (int i=0; i<ncpus; i++) {
        booked_to.push_back(now);
    }

    std::map<double, RESULT*>::iterator it;
    double up_frac = avg_proc_rate(0);
    for (
        it = results_by_deadline.begin();
        it != results_by_deadline.end() && !should_not_fetch_work;
        it++
    ) {
        RESULT *r = (*it).second;

        // non CPU intensive results don't count.
        if (r->project->non_cpu_intensive) continue;

        if (RESULT_COMPUTE_ERROR > ((*it).second)->state) {
            double lowest_book = booked_to[0];
            int lowest_booked_cpu = 0;
            for (int i=1; i<ncpus; i++) {
                if (booked_to[i] < lowest_book) {
                    lowest_book = booked_to[i];
                    lowest_booked_cpu = i;
                }
            }
            booked_to[lowest_booked_cpu] += ((*it).second)->estimated_cpu_time_remaining();

            // Are the deadlines too tight to meet reliably?
            //
            if (booked_to[lowest_booked_cpu] - now > (r->report_deadline - now) * MAX_CPU_LOAD_FACTOR * up_frac) {
                should_not_fetch_work = true;
                use_earliest_deadline_first = true;
                if (!cpu_earliest_deadline_first || !work_fetch_no_new_work) {
                    msg_printf(NULL, MSG_INFO,
                        "Computer is overcommitted"
                    );
                }
            }
            // Is the nearest deadline within a day?
            //
            if (r->report_deadline - now < 60 * 60 * 24) {
                use_earliest_deadline_first = true; 
                if (!cpu_earliest_deadline_first) {
                    msg_printf(NULL, MSG_INFO,
                        "Less than 1 day until deadline."
                    );
                }
            }

            // is there a deadline < twice the users connect period?
            //
            if (r->report_deadline - now < global_prefs.work_buf_min_days * SECONDS_PER_DAY * 2) {
                use_earliest_deadline_first = true; 
                if (!cpu_earliest_deadline_first) {
                    msg_printf(NULL, MSG_INFO,
                        "Deadline is before reconnect time"
                    );
                }
            }

            frac_booked += r->estimated_cpu_time_remaining() / (r->report_deadline - now);
        }
    }

    if (frac_booked > MAX_CPU_LOAD_FACTOR * up_frac * ncpus) {
        should_not_fetch_work = true;
        if (!work_fetch_no_new_work) {
            msg_printf(NULL, MSG_INFO,
                "Nearly overcommitted."
            );
        }
    }

    // check for too many projects that have work
    //
    if (projects_with_work.size() >= (unsigned int)global_prefs.max_projects_on_client) {
        should_not_fetch_work = true;
        if (!work_fetch_no_new_work) {
            msg_printf(NULL, MSG_INFO,
                "Too many projects have work."
            );
        }
    }

    // display only when the policy changes to avoid once per second
    //
    if (work_fetch_no_new_work && !should_not_fetch_work) {
        msg_printf(NULL, MSG_INFO,
            "New work fetch policy: work fetch allowed."
        );
    }

    if (!work_fetch_no_new_work && should_not_fetch_work) {
        msg_printf(NULL, MSG_INFO,
            "New work fetch policy: no work fetch allowed."
        );
    }

    if (cpu_earliest_deadline_first && !use_earliest_deadline_first) {
        msg_printf(NULL, MSG_INFO,
            "New CPU scheduler policy: highest debt first."
        );
    }
    if (!cpu_earliest_deadline_first && use_earliest_deadline_first) {
        msg_printf(NULL, MSG_INFO,
            "New CPU scheduler policy: earliest deadline first."
        );
    }

    work_fetch_no_new_work = should_not_fetch_work;
    cpu_earliest_deadline_first = use_earliest_deadline_first;
}

double CLIENT_STATE::work_needed_secs()
{
    double total_work = 0;
    for( unsigned int i = 0; i < results.size(); ++i) {
        if (results[i]->project->non_cpu_intensive) continue;
        total_work += results[i]->estimated_cpu_time_remaining();
    }
    if (total_work > global_prefs.work_buf_min_days) {
        return 0;
    } else {
        return global_prefs.work_buf_min_days - total_work;
    }
}

const char *BOINC_RCSID_d35a4a7711 = "$Id$";

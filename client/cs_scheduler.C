// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

// This file contains high-level logic for communicating with
// scheduling servers,
// and for merging the result of a scheduler RPC into the client state

// Note: code for actually doing a scheduler RPC is in scheduler_op.C

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "windows_cpp.h"

#include "crypt.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"
#include "log_flags.h"
#include "message.h"
#include "scheduler_op.h"

#include "client_state.h"

// quantities like avg CPU time decay by a factor of e every week
//
#define SECONDS_IN_DAY (3600*24)
#define EXP_DECAY_RATE  (1./(SECONDS_IN_DAY*7))

// estimate the days of work remaining
//
double CLIENT_STATE::current_water_days() {
    unsigned int i;
    RESULT* rp;
    double seconds_remaining=0;

    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->state > RESULT_COMPUTE_DONE) continue;
        // TODO: subtract time already finished for WUs in progress
        seconds_remaining += rp->wup->seconds_to_complete;
    }
    return (seconds_remaining * SECONDS_IN_DAY);
}

// seconds of work needed to come up to high-water mark
//
double CLIENT_STATE::work_needed_secs() {
    double x = current_water_days();
    if (x > global_prefs.high_water_days) return 0;
    return (global_prefs.high_water_days - x)*86400;
}

// update exponentially-averaged CPU times of all projects
//
void CLIENT_STATE::update_avg_cpu(PROJECT* p) {
    time_t now = time(0);
    double deltat = now - p->exp_avg_mod_time;
    if (deltat > 0) {
        if (p->exp_avg_cpu != 0) {
            p->exp_avg_cpu *= exp(deltat*EXP_DECAY_RATE);
        }
        p->exp_avg_mod_time = now;
    }
}

// find a project that needs its master file parsed
//
PROJECT* CLIENT_STATE::next_project_master_pending() {
    unsigned int i;
    PROJECT* p;
    time_t now = time(0);

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->min_rpc_time > now ) continue;
        if (p->master_url_fetch_pending) {
            return p;
        }
    }
    return 0;
}

// return the next project after "old", in debt order,
// that is eligible for a scheduler RPC
//
PROJECT* CLIENT_STATE::next_project(PROJECT* old) {
    PROJECT* p, *pbest;
    int best = 999;
    time_t now = time(0);
    unsigned int i;
    pbest = 0;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->min_rpc_time > now ) continue;
        if (old && p->debt_order <= old->debt_order) continue;
        if (p->debt_order < best) {
            pbest = p;
            best = p->debt_order;
        }
    }
    return pbest;
}

// Compute the "resource debt" of each project.
// This is used to determine what project we will focus on next,
// based on the user-specified resource share.
// TODO: this counts only CPU time.  Should reflect disk/network usage too.
//
void CLIENT_STATE::compute_resource_debts() {
    unsigned int i, j;
    PROJECT* p, *pbest;
    double best;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        update_avg_cpu(p);
        if (p->exp_avg_cpu == 0) {
            p->resource_debt = p->resource_share;
        } else {
            p->resource_debt = p->resource_share/p->exp_avg_cpu;
        }
        p->debt_order = -1;
    }

    // put in decreasing order.  Should use qsort or some stdlib thang
    //
    for (i=0; i<projects.size(); i++) {
        best = -2;
        for (j=0; j<projects.size(); j++) {
            p = projects[j];
            if (p->debt_order >= 0) continue;
            if (p->resource_debt > best) {
                best = p->resource_debt;
                pbest = p;
            }
        }
        pbest->debt_order = i;
    }
}

// Prepare the scheduler request.  This writes the request in XML to a
// file (SCHED_OP_REQUEST_FILE) which is later sent to the scheduling
// server
//
int CLIENT_STATE::make_scheduler_request(PROJECT* p, double work_req) {
    FILE* f = fopen(SCHED_OP_REQUEST_FILE, "wb");
    unsigned int i;
    RESULT* rp;

    if (!f) return ERR_FOPEN;
    fprintf(f,
        "<scheduler_request>\n"
        "    <authenticator>%s</authenticator>\n"
        "    <hostid>%d</hostid>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <platform_name>%s</platform_name>\n"
        "    <core_client_version>%d</core_client_version>\n"
        "    <work_req_seconds>%f</work_req_seconds>\n",
        p->authenticator,
        p->hostid,
        p->rpc_seqno,
        platform_name,
        core_client_version,
        work_req
    );
    if (p->code_sign_key) {
        fprintf(f, "<code_sign_key>\n%s</code_sign_key>\n", p->code_sign_key);
    }

    // insert global preferences if present
    //
    FILE* fprefs = fopen(GLOBAL_PREFS_FILE_NAME, "r");
    if (fprefs) {
        copy_stream(fprefs, f);
        fclose(fprefs);
    }

    time_stats.write(f, true);
    net_stats.write(f, true);
    host_info.write(f);
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project == p && rp->state == RESULT_READY_TO_ACK) {
            rp->write(f, true);
        }
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
    time_t now = time(0);

    for (i=0; i<results.size(); i++) {
        r = results[i];
        // If we've completed computation but haven't finished reporting the
        // results to the server, return the project for this result
        if (r->state == RESULT_READY_TO_ACK) {
            if (r->project->min_rpc_time < now) {
                return r->project;
            }
        }
    }
    return 0;
}

// return true if we're allowed to do a scheduler RPC to at least one project
//
bool CLIENT_STATE::some_project_rpc_ok() {
    unsigned int i;
    time_t now = time(0);

    for (i=0; i<projects.size(); i++) {
        if (projects[i]->min_rpc_time < now) return true;
    }
    return false;
}

// called from the client's polling loop.
// initiate scheduler RPC activity if needed and possible
//
bool CLIENT_STATE::scheduler_rpc_poll() {
    double work_secs;
    PROJECT* p;
    bool action=false, below_low_water;

    switch(scheduler_op->state) {
    case SCHEDULER_OP_STATE_IDLE:
        below_low_water = (current_water_days() <= global_prefs.low_water_days);
        if (below_low_water && some_project_rpc_ok()) {
            compute_resource_debts();
            scheduler_op->init_get_work();
            action = true;
        } else {
            p = find_project_with_overdue_results();
            if (p) {
                compute_resource_debts();
                if (p->debt_order == 0) {
                    work_secs = work_needed_secs();
                } else {
                    work_secs = 0;
                }
                scheduler_op->init_return_results(p, work_secs);
                action = true;
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
void CLIENT_STATE::handle_scheduler_reply(
    PROJECT* project, char* scheduler_url
) {
    SCHEDULER_REPLY sr;
    FILE* f;
    int retval;
    unsigned int i;
    bool signature_valid;

    contacted_sched_server = true;
    if (log_flags.sched_op_debug) {
        f = fopen(SCHED_OP_RESULT_FILE, "r");
        printf("------------- SCHEDULER REPLY ----------\n");
        copy_stream(f, stdout);
        fclose(f);
        printf("------------- END ----------\n");
    }

    f = fopen(SCHED_OP_RESULT_FILE, "r");
    retval = sr.parse(f);
    fclose(f);

    if (strlen(sr.project_name)) {
        strcpy(project->project_name, sr.project_name);
    }
    if (strlen(sr.user_name)) {
        strcpy(project->user_name, sr.user_name);
    }
    project->total_credit = sr.total_credit;
    project->expavg_credit = sr.expavg_credit;
    if (strlen(sr.message)) {
        show_message(sr.message, sr.message_priority);
    }

    if (sr.request_delay) {
        project->min_rpc_time = time(0) + sr.request_delay;
    }

    if (sr.hostid) {
        project->hostid = sr.hostid;
        project->rpc_seqno = 0;
    }

    // if the scheduler reply includes global preferences,
    // insert extra elements, write to disk, and parse
    //
    if (sr.global_prefs_xml) {
        f = fopen(GLOBAL_PREFS_FILE_NAME, "w");
        fprintf(f,
            "<global_preferences>\n"
            "    <source_project>%s</source_project>\n"
            "    <source_scheduler>%s</source_scheduler>\n"
            "%s"
            "</global_preferences>\n",
            project->master_url,
            scheduler_url,
            sr.global_prefs_xml
        );
        fclose(f);
        global_prefs.parse_file();
    }

    // deal with project preferences (should always be there)
    //
    if (sr.project_prefs_xml) {
        char path[256];
        f = fopen(TEMP_FILE_NAME, "w");
        fprintf(f,
            "<account>\n"
            "    <master_url>%s</master_url>\n"
            "    <authenticator>%s</authenticator>\n"
            "%s"
            "</account>\n",
            project->master_url,
            project->authenticator,
            sr.project_prefs_xml
        );
        fclose(f);
        get_account_filename(project->master_url, path);
        retval = boinc_rename(TEMP_FILE_NAME, path);
        f = fopen(path, "r");
        project->parse_account(f);
        fclose(f);
    }

    // if the scheduler reply includes a code-signing key,
    // accept it if we don't already have one from the project.
    // Otherwise verify its signature, using the key we already have.
    //

    if (sr.code_sign_key) {
        if (!project->code_sign_key) {
            project->code_sign_key = strdup(sr.code_sign_key);
        } else {
            if (sr.code_sign_key_signature) {
                retval = verify_string2(
                    sr.code_sign_key, sr.code_sign_key_signature,
                    project->code_sign_key, signature_valid
                );
                if (!retval && signature_valid) {
                    free(project->code_sign_key);
                    project->code_sign_key = strdup(sr.code_sign_key);
                } else {
                    fprintf(stdout,
                        "New code signing key from %s doesn't validate\n",
                        project->project_name
                    );
                }
            } else {
                fprintf(stdout, "Missing code sign key signature\n");
            }
        }
    }

    // copy new entities to client state
    //

    for (i=0; i<sr.apps.size(); i++) {
        APP* app = lookup_app(project, sr.apps[i].name);
        if (app) {
            //*app = sr.apps[i];
            retval = link_app(project,app); // not sure about this
        } else {
            app = new APP;
            *app = sr.apps[i];
            retval = link_app(project, app);
            if (!retval) apps.push_back(app);
        }
    }
    for (i=0; i<sr.file_infos.size(); i++) {
        if (!lookup_file_info(project, sr.file_infos[i].name)) {
            FILE_INFO* fip = new FILE_INFO;
            *fip = sr.file_infos[i];
            retval = link_file_info(project, fip);
            if (!retval) file_infos.push_back(fip);
        }
    }
    for (i=0; i<sr.app_versions.size(); i++) {
        APP* app = lookup_app(project, sr.app_versions[i].app_name);
        if (!lookup_app_version(app, sr.app_versions[i].version_num)) {
            APP_VERSION* avp = new APP_VERSION;
            *avp = sr.app_versions[i];
            retval = link_app_version(project, avp);
            if (!retval) app_versions.push_back(avp);
        }
    }
    for (i=0; i<sr.workunits.size(); i++) {
        if (!lookup_workunit(project, sr.workunits[i].name)) {
            WORKUNIT* wup = new WORKUNIT;
            *wup = sr.workunits[i];
            wup->version_num = latest_version_num(wup->app_name);
            retval = link_workunit(project, wup);
            if (!retval) {
                workunits.push_back(wup);
            }
        }
    }
    for (i=0; i<sr.results.size(); i++) {
        if (!lookup_result(project, sr.results[i].name)) {
            RESULT* rp = new RESULT;
            *rp = sr.results[i];
            retval = link_result(project, rp);
            if (!retval) results.push_back(rp);
            rp->state = RESULT_NEW;
        }
    }

    // update records for ack'ed results
    //
    for (i=0; i<sr.result_acks.size(); i++) {
        RESULT* rp = lookup_result(project, sr.result_acks[i].name);
        if (log_flags.sched_op_debug) {
            printf("got ack for result %s\n", sr.result_acks[i].name);
        }
        if (rp) {
            rp->state = RESULT_SERVER_ACK;
        } else {
            fprintf(stderr,
                "ERROR: got ack for result %s, can't find\n",
                sr.result_acks[i].name
            );
        }
    }
    set_client_state_dirty("handle_scheduler_reply");
    if (log_flags.state_debug) {
        printf("State after handle_scheduler_reply():\n");
        print_counts();
    }
}

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

#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef _USING_FCGI_
#undef _USING_FCGI_
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "parse.h"
#include "log_flags.h"
#include "message.h"
#include "scheduler_reply.h"

#include "client_state.h"

// quantities like avg CPU time decay by a factor of e every week
#define EXP_DECAY_RATE  (1./(3600*24*7))

//estimates amount of time a workunit will take to complete
//
double CLIENT_STATE::estimate_duration(WORKUNIT* wup) {
    return wup->rsc_fpops/host_info.p_fpops + wup->rsc_iops/host_info.p_iops;
}

//estimates the number of days of work remaining
//
double CLIENT_STATE::current_water_days() {
    unsigned int i;
    double seconds_remaining=0;
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->is_compute_done) continue;
	if (rp->cpu_time > 0)
	    seconds_remaining += (estimate_duration(rp->wup) - rp->cpu_time);
	else
	    seconds_remaining += estimate_duration(rp->wup);
    }
    return (seconds_remaining * 86400);
}

bool CLIENT_STATE::need_work() {
    return (current_water_days() <= prefs.low_water_days);
}

void CLIENT_STATE::update_avg_cpu(PROJECT* p) {
    int now = time(0);
    double deltat = now - p->exp_avg_mod_time;
    if (deltat > 0) {
        if (p->exp_avg_cpu != 0) {
            p->exp_avg_cpu *= exp(deltat*EXP_DECAY_RATE);
        }
        p->exp_avg_mod_time = now;
    }
}

// choose a project to ask for work
//
PROJECT* CLIENT_STATE::choose_project() {
    PROJECT* p, *bestp;
    unsigned int i;
    double best_ratio, ratio;

    // update the average CPU times of all projects
    //
    for (i=0; i<projects.size(); i++) {
        update_avg_cpu(projects[i]);
    }

    // pick the project for which share/avg is largest
    // (and we have permission to contact it)
    //
    best_ratio = 0;
    bestp = 0;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (time(0) < p->next_request_time) continue;
        if (p->exp_avg_cpu == 0) return p;
        ratio = p->resource_share/p->exp_avg_cpu;
        if (ratio > best_ratio) {
            best_ratio = ratio;
            bestp = p;
        }
    }
    return bestp;
}

int CLIENT_STATE::make_scheduler_request(PROJECT* p, int work_req) {
    FILE* f = fopen(SCHED_OP_REQUEST_FILE, "w");
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
        "    <work_req_seconds>%d</work_req_seconds>\n",
        p->authenticator,
        p->hostid,
        p->rpc_seqno,
	platform_name,
	version,
        work_req
    );

    fprintf(f,
        "    <prefs_mod_time>%d</prefs_mod_time>\n",
        prefs.mod_time
    );

    time_stats.write(f, true);
    net_stats.write(f, true);
    host_info.write(f);
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project == p && !rp->is_server_ack) {
            if (rp->is_upload_done()) {
                rp->write(f, true);
            }
        }
    }
    fprintf(f, "</scheduler_request>\n");
    fclose(f);
    if (log_flags.sched_ops) {
        printf("Sending request to scheduler: %s\n", p->scheduler_url);
    }
    if (log_flags.sched_op_debug) {
        f = fopen(SCHED_OP_REQUEST_FILE, "r");
        printf("--------- SCHEDULER REQUEST ---------\n");
        copy_stream(f, stdout);
        printf("--------- END ---------\n");
        fclose(f);
    }
    return 0;
}

// manage the task of maintaining a supply of work.
//
// todo: determine how to calculate current_water_level
bool CLIENT_STATE::get_work() {
    PROJECT* p;
    int retval;
    bool action=false;

    if (need_work()) {
        if (scheduler_op.http_op_state == HTTP_STATE_IDLE) {
            // if no scheduler request pending, start one
            //
            p = choose_project();
            if (!p) {
                if (log_flags.sched_op_debug) {
                    printf("all projects temporarily backed off\n");
                }
                return false;
            }
            retval = make_scheduler_request(p, 
		  (int)(prefs.high_water_days - current_water_days())*86400);
            scheduler_op.init_post(
                p->scheduler_url, SCHED_OP_REQUEST_FILE, SCHED_OP_RESULT_FILE
            );
            scheduler_op_project = p;
            http_ops->insert(&scheduler_op);
            p->rpc_seqno++;
            action = true;
        } else {
            if (scheduler_op.http_op_state == HTTP_STATE_DONE) {
                action = true;
                http_ops->remove(&scheduler_op);
                scheduler_op.http_op_state = HTTP_STATE_IDLE;
                int retval = scheduler_op.http_op_retval;
                if (!retval) {
                    handle_scheduler_reply(scheduler_op_project);
                    client_state_dirty = true;
                } else {
                    if (log_flags.sched_ops) {
                        printf("HTTP work request failed: %d\n", retval);
                    }
                }
            }
        }
    }
    return action;
}

void CLIENT_STATE::handle_scheduler_reply(PROJECT* project) {
    SCHEDULER_REPLY sr;
    FILE* f;
    int retval;
    unsigned int i;

    contacted_sched_server = true;
    if (log_flags.sched_ops) {
        printf("Got reply from scheduler %s\n", project->scheduler_url);
    }
    if (log_flags.sched_op_debug) {
        f = fopen(SCHED_OP_RESULT_FILE, "r");
        printf("------------- SCHEDULER REPLY ----------\n");
        copy_stream(f, stdout);
        fclose(f);
        printf("------------- END ----------\n");
    }

    f = fopen(SCHED_OP_RESULT_FILE, "r");
    retval = sr.parse(f);

    if (strlen(sr.message)) {
        show_message(sr.message, sr.message_priority);
    }

    // copy new entities to client state
    //

    if (sr.request_delay) {
        project->next_request_time = time(0) + sr.request_delay;
    }
    if (sr.hostid) {
        project->hostid = sr.hostid;
        project->rpc_seqno = 0;
    }

    if (sr.prefs.mod_time > prefs.mod_time && strlen(sr.prefs.prefs_xml)) {
        if (prefs.prefs_xml) free(prefs.prefs_xml);
        prefs = sr.prefs;
    }

    for (i=0; i<sr.apps.size(); i++) {
        APP* app = lookup_app(project, sr.apps[i].name);
        if (app) {
            *app = sr.apps[i];
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
            retval = link_workunit(project, wup);
            if (!retval) workunits.push_back(wup);
        }
    }
    for (i=0; i<sr.results.size(); i++) {
        if (!lookup_result(project, sr.results[i].name)) {
            RESULT* rp = new RESULT;
            *rp = sr.results[i];
            retval = link_result(project, rp);
            if (!retval) results.push_back(rp);
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
            rp->is_server_ack = true;
        } else {
            fprintf(stderr,
                "ERROR: got ack for result %s, can't find\n",
                sr.result_acks[i].name
            );
        }
    }
    print_counts();
}

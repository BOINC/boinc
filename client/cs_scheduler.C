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
// scheduling servers:
// - what project to ask for work
// - how much work to ask for
// - merging the result of a scheduler RPC into the client state

// Note: code for actually doing a scheduler RPC is elsewhere,
// namely scheduler_op.C

#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef _USING_FCGI_
#undef _USING_FCGI_
#endif

#include "crypt.h"
#include "error_numbers.h"
#include "file_names.h"
#include "parse.h"
#include "log_flags.h"
#include "message.h"
#include "scheduler_op.h"

#include "client_state.h"

// quantities like avg CPU time decay by a factor of e every week
#define EXP_DECAY_RATE  (1./(3600*24*7))
#define SECONDS_IN_DAY 86400

//estimates the number of days of work remaining
//
double CLIENT_STATE::current_water_days() {
    unsigned int i;
    double seconds_remaining=0;
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->is_compute_done) continue;
	if (rp->cpu_time > 0)
	    seconds_remaining += (rp->wup->seconds_to_complete - rp->cpu_time);
	else
	    seconds_remaining += rp->wup->seconds_to_complete;
    }
    return (seconds_remaining * SECONDS_IN_DAY);
}

bool CLIENT_STATE::need_work() {
    double temp;
    if(prefs->high_water_days < prefs->low_water_days) {
        temp = prefs->high_water_days;
        prefs->high_water_days = prefs->low_water_days;
        prefs->low_water_days = temp;
    }
    return (current_water_days() <= prefs->low_water_days);
}

void CLIENT_STATE::update_avg_cpu(PROJECT* p) {
    int now = time(0);
    if(p==NULL) {
        fprintf(stderr, "error: CLIENT_STATE.update_avg_cpu: unexpected NULL pointer p\n");
    }
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
        if (ratio >= best_ratio) {
            best_ratio = ratio;
            bestp = p;
        }
    }
    return bestp;
}

int CLIENT_STATE::make_scheduler_request(PROJECT* p, int work_req) {
    FILE* f = fopen(SCHED_OP_REQUEST_FILE, "wb");
    unsigned int i;
    RESULT* rp;
    if(p==NULL) {
        fprintf(stderr, "error: CLIENT_STATE.make_scheduler_request: unexpected NULL pointer p\n");
        return ERR_NULL;
    }
    if(work_req<0) {
        fprintf(stderr, "error: CLIENT_STATE.make_scheduler_request: negative work_req\n");
        return ERR_NEG;
    }
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
    if (p->code_sign_key) {
        fprintf(f, "<code_sign_key>\n%s</code_sign_key>\n", p->code_sign_key);
    }

    FILE* fprefs = fopen(PREFS_FILE_NAME, "r");
    copy_stream(fprefs, f);
    fclose(fprefs);

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
    return 0;
}

// manage the task of maintaining an adequate supply of work.
//
bool CLIENT_STATE::get_work() {
    PROJECT* project;
    int retval, work_secs;
    bool action=false;

    if (need_work()) {
        switch(scheduler_op->state) {
        case SCHEDULER_OP_STATE_IDLE:
            // if no scheduler request pending, start one
            //
            project = choose_project();
            if (!project) {
                if (log_flags.sched_op_debug) {
                    printf("all projects temporarily backed off\n");
                }
                return false;
            }
            work_secs =
                (int) (prefs->high_water_days - current_water_days())*86400;
            retval = make_scheduler_request(project, work_secs);
            if (retval) {
                fprintf(stderr, "make_scheduler_request: %d\n", retval);
                break;
            }

            scheduler_op->start_op(project);
            action = true;
            break;
        default:
            scheduler_op->poll();
            if (scheduler_op->state == SCHEDULER_OP_STATE_DONE) {
                scheduler_op->state = SCHEDULER_OP_STATE_IDLE;
                action = true;
                if (scheduler_op->scheduler_op_retval) {
                    fprintf(stderr,
                        "scheduler RPC to %s failed: %d\n",
                        scheduler_op->project->master_url,
                        scheduler_op->scheduler_op_retval
                    );
                    if (log_flags.sched_ops) {
                        printf("HTTP work request failed: %d\n", retval);
                    }
                    break;
                } else {
                    handle_scheduler_reply(*scheduler_op);
                    client_state_dirty = true;
                }
            }
            break;
        }
    }
    return action;
}

// see whether a new preferences set, obtained from
// the given project, looks "reasonable".
// Currently this is primitive: just make sure there's at least 1 project
//
bool PREFS::looks_reasonable(PROJECT& project) {
    if (projects.size() > 0) return true;
    return false;
}

void CLIENT_STATE::handle_scheduler_reply(SCHEDULER_OP& sched_op) {
    SCHEDULER_REPLY sr;
    FILE* f;
    int retval;
    unsigned int i;
    char prefs_backup[256];
    PROJECT *project, *pp, *sp;
    PREFS* new_prefs;
    bool signature_valid;

    project = sched_op.project;
    contacted_sched_server = true;
    if (log_flags.sched_ops) {
        printf("Got reply from scheduler %s\n", sched_op.scheduler_url);
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

    if (sr.request_delay) {
        project->next_request_time = time(0) + sr.request_delay;
    }
    if (sr.hostid) {
        project->hostid = sr.hostid;
        project->rpc_seqno = 0;
    }

    // if the scheduler reply includes preferences
    // that are newer than what we have on disk, then
    // - verify that the new prefs look reasonable;
    //   they should include at least one project.
    // - rename the current prefs file
    // - copy new preferences to prefs.xml
    // - copy any new projects into the CLIENT_STATE structure
    // - update the preferences info of projects already in CLIENT_STATE
    //
    // There may be projects in CLIENT_STATE that are not in the new prefs;
    // i.e. the user has dropped out of these projects.
    // We'll continue with any ongoing work for these projects,
    // but they'll be dropped the next time the client starts up.
    //

    if (sr.prefs_mod_time > prefs->mod_time) {
        f = fopen(PREFS_TEMP_FILE_NAME, "w");
        fprintf(f,
            "<preferences>\n"
            "    <prefs_mod_time>%d</prefs_mod_time>\n"
            "    <from_project>%s</from_project>\n"
            "    <from_scheduler>%s</from_scheduler>\n",
            sr.prefs_mod_time,
            project->master_url,
            sched_op.scheduler_url
        );
        fputs(sr.prefs_xml, f);
        fprintf(f,
            "</preferences>\n"
        );
        fclose(f);
        f = fopen(PREFS_TEMP_FILE_NAME, "r");
        new_prefs = new PREFS;
        new_prefs->parse(f);
        fclose(f);
        if (new_prefs->looks_reasonable(*project)) {
            make_prefs_backup_name(*prefs, prefs_backup);
            rename(PREFS_FILE_NAME, prefs_backup);
            rename(PREFS_TEMP_FILE_NAME, PREFS_FILE_NAME);
            for (i=0; i<new_prefs->projects.size(); i++) {
                pp = new_prefs->projects[i];
                sp = lookup_project(pp->master_url);
                if (sp) {
                    sp->copy_prefs_fields(*pp);
                } else {
                    projects.push_back(pp);
                }
            }
            delete prefs;
            prefs = new_prefs;
        } else {
            fprintf(stderr, "New preferences don't look reasonable, ignoring\n");
        }
    }

    // if the scheduler reply includes a code-signing key,
    // accept it if we don't already have one from the project.
    // Otherwise verify its signature, using the key we already have.
    //

    if (sr.code_sign_key) {
        if (!project->code_sign_key) {
            project->code_sign_key = strdup(sr.code_sign_key);
        } else {
            retval = verify_string2(
                sr.code_sign_key, sr.code_sign_key_signature,
                project->code_sign_key, signature_valid
            );
            if (!retval && signature_valid) {
                free(project->code_sign_key);
                project->code_sign_key = strdup(sr.code_sign_key);
            } else {
                fprintf(stderr,
                    "New code signing key from %s doesn't validate\n",
                    project->project_name
                );
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
    if (log_flags.state_debug) {
        printf("State after handle_scheduler_reply():\n");
        print_counts();
    }
}

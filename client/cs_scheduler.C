// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

static int proj_min_results(PROJECT* p, int ncpus) {
    return (int)(ceil(ncpus*p->resource_share/trs));
}
void PROJECT::set_min_rpc_time(time_t future_time) {
	if (future_time > min_rpc_time) {
		min_rpc_time = future_time;
	}
    min_report_min_rpc_time = 0; // report immediately
}

// Return true iff we should not contact the project yet.
// Print a message to the user if we haven't recently
//
bool PROJECT::waiting_until_min_rpc_time(time_t now) {
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

// find a project that needs its master file parsed
//
PROJECT* CLIENT_STATE::next_project_master_pending() {
    unsigned int i;
    PROJECT* p;
    time_t now = time(0);

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->waiting_until_min_rpc_time(now)) continue;
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
    time_t now = time(0);

    for (i=0; i<projects.size(); i++) {
        if (projects[i]->waiting_until_min_rpc_time(now)) continue;
        if (projects[i]->sched_rpc_pending) {
            return projects[i];
        }
    }
    return 0;
}

// return the next project after "old" that is eligible for a
// scheduler RPC
// It excludes projects that have (p->master_url_fetch_pending) set to
// true.
// Such projects will be returned by next_project_master_pending
// routine.
//
PROJECT* CLIENT_STATE::next_project(PROJECT *old) {
    PROJECT *p;
    time_t now = time(0);
    unsigned int i;
    bool found_old = (old == 0);
    for (i=0; i<projects.size(); ++i) {
        p = projects[i];
        if (p == old) {
            found_old = true;
            continue;
        }
        if (p->master_url_fetch_pending) continue;
        if (p->waiting_until_min_rpc_time(now)) continue;
        if (found_old && p->work_request > 0) {
            return p;
        }
    }
    return 0;
}

// Prepare the scheduler request.  This writes the request to a
// file (SCHED_OP_REQUEST_FILE) which is later sent to the scheduling server
//
int CLIENT_STATE::make_scheduler_request(PROJECT* p, double work_req) {
    FILE* f = boinc_fopen(SCHED_OP_REQUEST_FILE, "wb");
    MIOFILE mf;
    unsigned int i;
    RESULT* rp;
    int retval;
#if 0
    double free, possible;
#endif
    char cross_project_id[MD5_LEN];

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
        "    <estimated_delay>%f</estimated_delay>\n"
        "    <host_venue>%s</host_venue>\n",
        p->authenticator,
        p->hostid,
        p->rpc_seqno,
		p->anonymous_platform?"anonymous":platform_name,
        core_client_major_version,
        core_client_minor_version,
        work_req,
        p->resource_share / trs,
        ettprc(p, proj_min_results(p, ncpus)-1),
        host_venue
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
        PROJECT* pp = lookup_project(global_prefs.source_project.c_str());
        if (pp && strlen(pp->email_hash)) {
            fprintf(f,
                "<global_prefs_source_email_hash>%s</global_prefs_source_email_hash>\n",
                pp->email_hash
            );
        }
    }

    // send the maximum of cross_project_id over projects
    // with the same email hash as this one
    //
    strcpy(cross_project_id, p->cross_project_id);
    for (i=0; i<projects.size(); i++ ) {
        PROJECT* project = projects[i];
        if (project == p) continue;
        if (strcmp(project->email_hash, p->email_hash)) continue;
        if (strcmp(project->cross_project_id, cross_project_id) > 0) {
            strcpy(cross_project_id, project->cross_project_id);
        }
    }
    fprintf(f, "<cross_project_id>%s</cross_project_id>\n", cross_project_id);

    fprintf(f, "<projects>\n");
    for (i=0; i<projects.size(); i++ ) {
        PROJECT* project = projects[i];
        fprintf(f,
            "    <project>\n"
            "        <master_url>%s</master_url>\n"
            "        <resource_share>%f</resource_share>\n"
            "    </project>\n",
            project->master_url,
            project->resource_share
        );
    }
    fprintf(f, "</projects>\n");

    retval = time_stats.write(mf, true);
    if (retval) return retval;
    retval = net_stats.write(mf, true);
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
        // return the project for this result to report if:
        //    - we're not backing off a scheduler request for its project
        //    - we're ready_to_report (compute done; files uploaded)
        //    - we're almost at the report_deadline
        //

        if (r->project->waiting_until_min_rpc_time(now)) continue;

        if (!r->ready_to_report) continue;
        if (return_results_immediately ||
             (r->report_deadline <= (now + REPORT_DEADLINE_CUSHION))
        ) {
            return r->project;
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

// return the average number of CPU seconds completed by the client
// for project p in a second of (wall-clock) time
//
double CLIENT_STATE::avg_proc_rate(PROJECT *p) {
    return (p->resource_share / trs)
        * ncpus
        * time_stats.active_frac;
}

// "estimated time to project result count"
// return the estimated amount of time that will elapse until the
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
            || rp->state >= RESULT_COMPUTE_DONE
            || rp->ready_to_report
        ) continue;
        if (num_results_to_skip > 0) {
            --num_results_to_skip;
            continue;
        }
        if (rp->wup) { // just being paranoid...
            est += estimate_cpu_time(*rp->wup) * (1.0 - get_fraction_done(rp));
        }
    }
    est /= avg_proc_rate(p);
    return est;
}

// set work_request for each project and return the urgency level for
// requesting more work
// only set non-zero work requests for projects that are allowed to do
// a scheduler RPC
//
int CLIENT_STATE::compute_work_requests() {
    int urgency = DONT_NEED_WORK;
    unsigned int i;
    double work_min_period = global_prefs.work_buf_min_days * SECONDS_PER_DAY;
    time_t now = time(0);
    
    trs = total_resource_share();

    // for each project, compute
    // min_results = min # of results for project needed by CPU scheduling,
    // to avoid "starvation".
    // Then estimate how long it's going to be until we have fewer
    // than this # of results remaining.
    //
    for (i=0; i<projects.size(); ++i) {
        PROJECT *p = projects[i];
        int min_results = proj_min_results(p, ncpus);
        double estimated_time_to_starvation = ettprc(p, min_results-1);

        p->work_request = 0;
        if (p->min_rpc_time >= now) continue;

        // determine urgency
        //
        if (estimated_time_to_starvation < work_min_period) {
            if (estimated_time_to_starvation == 0) {
//                msg_printf(p, MSG_INFO, "Will starve!");
                urgency = NEED_WORK_IMMEDIATELY;
            } else {
//                msg_printf(p, MSG_INFO, "Will starve in %.2fs!",
//                    estimated_time_to_starvation
//                );
                urgency = max(NEED_WORK, urgency);
            }
        }

        // determine work requests for each project
        //
        p->work_request =
            max(0.0,
                (2*work_min_period - estimated_time_to_starvation)
                * avg_proc_rate(p)
            );
    }

    if (urgency == DONT_NEED_WORK) {
        for (i=0; i<projects.size(); ++i) {
            projects[i]->work_request = 0;
        }
    }

    return urgency;
}

// called from the client's polling loop.
// initiate scheduler RPC activity if needed and possible
//
bool CLIENT_STATE::scheduler_rpc_poll() {
    int urgency = DONT_NEED_WORK;
    PROJECT *p;
    bool action=false;

    switch(scheduler_op->state) {
    case SCHEDULER_OP_STATE_IDLE:
        if (network_suspended) break;
        urgency = compute_work_requests();
        
        // highest priority is to report overdue results
        //
        p = find_project_with_overdue_results();
        if (p) {
            scheduler_op->init_return_results(p, p->work_request);
            action = true;
        } else if (!(exit_when_idle && contacted_sched_server) && urgency != DONT_NEED_WORK) {
            if (urgency == NEED_WORK) {
                msg_printf(NULL, MSG_INFO,
                    "May run out of work in %.2f days; requesting more",
                    global_prefs.work_buf_min_days
                );
            } else if (urgency == NEED_WORK_IMMEDIATELY) {
                msg_printf(NULL, MSG_INFO,
                    "Insufficient work; requesting more"
                );
            }
            scheduler_op->init_get_work();
            action = true;
        } else if ((p=next_project_master_pending())) {
            scheduler_op->init_get_work();
            action = true;
        } else if ((p=next_project_sched_rpc_pending())) {
            scheduler_op->init_return_results(p, p->work_request);
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
    bool signature_valid, need_to_install_prefs=false;
    char buf[256];

    nresults = 0;
    contacted_sched_server = true;
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_SCHED_OP);

    scope_messages.printf_file(SCHED_OP_RESULT_FILE, "reply: ");

    f = fopen(SCHED_OP_RESULT_FILE, "r");
    if (!f) return ERR_FOPEN;
    retval = sr.parse(f, project);
    fclose(f);
    if (retval) return retval;

    if (strlen(sr.project_name)) {
        safe_strcpy(project->project_name, sr.project_name);
    }

    if (sr.request_delay) {
        time_t x = time(0) + sr.request_delay;
        if (x > project->min_rpc_time) project->min_rpc_time = x;
    }

    if (strlen(sr.message)) {
        sprintf(buf, "Message from server: %s", sr.message);
        int prio = (!strcmp(sr.message_priority, "high"))?MSG_ERROR:MSG_INFO;
        show_message(project, buf, prio);
    }

    // if project is down, return error (so that we back off)
    // and don't do anything else
    //
    if (sr.project_is_down) {
        return ERR_PROJECT_DOWN;
    }

    if (strlen(sr.user_name)) {
        safe_strcpy(project->user_name, sr.user_name);
    }
    if (strlen(sr.team_name)) {
        safe_strcpy(project->team_name, sr.team_name);
    }
    if (sr.user_total_credit >= 0) {
        project->user_total_credit = sr.user_total_credit;
    }
    if (sr.user_expavg_credit >= 0) {
        project->user_expavg_credit = sr.user_expavg_credit;
    }
    if (sr.user_create_time > 0) {
        project->user_create_time = sr.user_create_time;
    }
    if (sr.host_total_credit >= 0) {
        project->host_total_credit = sr.host_total_credit;
    }
    if (sr.host_expavg_credit >= 0) {
        project->host_expavg_credit = sr.host_expavg_credit;
    }
    if (sr.hostid) {
        project->hostid = sr.hostid;
        project->host_create_time = sr.host_create_time;
        project->rpc_seqno = 0;
	    if (strcmp(host_venue, sr.host_venue)) {
            safe_strcpy(host_venue, sr.host_venue);
            need_to_install_prefs = true;
        }
    }

#if 0
    if (sr.deletion_policy_priority) project->deletion_policy_priority = true;
    if (sr.deletion_policy_expire) project->deletion_policy_expire = true;
#endif

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
		need_to_install_prefs = true;
	}


	if (need_to_install_prefs) {
        bool found_venue;
        retval = global_prefs.parse_file(
            GLOBAL_PREFS_FILE_NAME, host_venue, found_venue
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
            retval = project->write_account_file();
            if (retval) {
                msg_printf(project, MSG_ERROR, "Can't write account file: %d", retval);
                return retval;
            }
            project->parse_account_file();
            project->parse_preferences_for_user_files();
            active_tasks.request_reread_prefs(project);
        }
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
            retval = link_file_info(project, fip, true);
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
        fip = lookup_file_info(project, sr.file_deletes[i].text);
        if (fip) {
            msg_printf(project, MSG_INFO, "Got request to delete file: %s\n", fip->name);
            fip->sticky = false;
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
        scope_messages.printf("CLIENT_STATE::handle_scheduler_reply(): got ack for result %s\n", sr.result_acks[i].name);
        if (rp) {
            rp->got_server_ack = true;
        } else {
            msg_printf(project, MSG_ERROR,
                "Got ack for result %s, can't find", rp->name
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
    set_client_state_dirty("handle_scheduler_reply");
    scope_messages.printf("CLIENT_STATE::handle_scheduler_reply(): State after handle_scheduler_reply():\n");
    print_summary();
    return 0;
}

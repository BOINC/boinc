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

// quantities like avg CPU time decay by a factor of e every week
//
#define EXP_DECAY_RATE  (1./(SECONDS_PER_DAY*7))

// how often to show user "backing off" messages
//
const int SECONDS_BEFORE_REPORTING_MIN_RPC_TIME_AGAIN = 60*60;


// try to report results this much before their deadline
//
#define REPORT_DEADLINE_CUSHION SECONDS_PER_DAY

// estimate the days of work remaining
//
void CLIENT_STATE::current_work_buf_days(
    double& work_buf, int& nactive_results
) {
    unsigned int i;
    RESULT* rp;
    double seconds_remaining=0, x;

	nactive_results = 0;
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        // Don't count result if we've already computed it,
        // or if it had an error
        //
        if (rp->state >= RESULT_COMPUTE_DONE) continue;
        if (rp->ready_to_report) continue;

		nactive_results++;

        // TODO: subtract time already finished for WUs in progress

        seconds_remaining += estimate_cpu_time(*rp->wup) * (1.0-get_fraction_done(rp));
    }
    x = seconds_remaining / SECONDS_PER_DAY;
    x /= host_info.p_ncpus;
    x /= time_stats.active_frac;
    work_buf = x;
}

// seconds of CPU work needed to come up to the max buffer level
//
double CLIENT_STATE::work_needed_secs() {
    double x;
	int n;

	current_work_buf_days(x, n);
    if (x > global_prefs.work_buf_max_days) return 0;

	// TODO: take into account preference # CPUS
    double y = (global_prefs.work_buf_max_days - x)*SECONDS_PER_DAY;
    y *= time_stats.active_frac;
    y *= host_info.p_ncpus;
    return y;
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

// return the next project after "old", in debt order,
// that is eligible for a scheduler RPC
// It excludes projects that have (p->master_url_fetch_pending) set to true.
// Such projects will be returned by next_project_master_pending routine.
//
PROJECT* CLIENT_STATE::next_project(PROJECT* old) {
    PROJECT* p, *pbest;
    int best = 999;
    time_t now = time(0);
    unsigned int i;
    pbest = 0;
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->master_url_fetch_pending) continue;
        if (p->waiting_until_min_rpc_time(now)) continue;
        if (old && p->debt_order <= old->debt_order) continue;
        if (p->debt_order < best) {
            pbest = p;
            best = p->debt_order;
        }
    }
    return pbest;
}

#if 0
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
    bool found_old = old == 0;
    for (i=0; i<projects.size(); ++i) {
        p = projects[i];
        if (p == old) found_old = true;
        if (p->master_url_fetch_pending) continue;
        if (p->waiting_until_min_rpc_time(now)) continue;
        if (found_old && p->work_request > 0) {
            return p;
        }
    }
    return 0;
}
#endif

// Compute the "resource debt" of each project.
// This is used to determine what project we will ask for work next,
// based on the user-specified resource share.
// TODO: this counts only CPU time.  Should reflect disk/network usage too.
//
// Note: it has been argued that we should use granted credit
// instead of or in addition to locally measured work.
// The problem with this is that we'd do more work
// for projects that fail to grant credit for whatever reason.
//
// Note: it's also been argued that we should average work
// over all hosts of this user, to stay closer to the
// target resource share globally.
// This bears some thinking about.
//
void CLIENT_STATE::compute_resource_debts() {
    unsigned int i, j;
    PROJECT* p, *pbest=0;
    double best=0;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        update_average(
            0, 0, CPU_HALF_LIFE, p->exp_avg_cpu, p->exp_avg_mod_time
        );
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
        pbest = NULL;
        for (j=0; j<projects.size(); j++) {
            p = projects[j];
            if (p->debt_order >= 0) continue;
            if (!pbest || (p->resource_debt > best)) {
                best = p->resource_debt;
                pbest = p;
            }
        }
        if (pbest) {
            pbest->debt_order = i;
        } else {
            msg_printf(NULL, MSG_ERROR,
                "compute_resource_debts(): sorting error"
            );
        }
    }
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
    double size;
    char cross_project_id[MD5_LEN];

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
        "    <work_req_seconds>%f</work_req_seconds>\n",
        p->authenticator,
        p->hostid,
        p->rpc_seqno,
		p->anonymous_platform?"anonymous":platform_name,
        core_client_major_version,
        core_client_minor_version,
        work_req
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
    if (!project_disk_usage(p, size)) {
        fprintf(f, "<project_disk_usage>%f</project_disk_usage>\n", size);
    }
    if (!total_disk_usage(size)) {
        fprintf(f, "<total_disk_usage>%f</total_disk_usage>\n", size);
    }
    if (strlen(p->code_sign_key)) {
        fprintf(f, "<code_sign_key>\n%s</code_sign_key>\n", p->code_sign_key);
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

// set projects' work_request and return the urgency of requesting
// more work
//
int CLIENT_STATE::compute_work_requests() {
    double total_share = 0;
    int urgency = 0;
    double work_min_period = global_prefs.work_buf_min_days * SECONDS_PER_DAY;

    for (unsigned int i=0; i<projects.size(); ++i) {
        PROJECT *p = projects[i];
        total_share += p->resource_share;
    }

    for (unsigned int i=0; i<projects.size(); ++i) {
        PROJECT *p = projects[i];
        double work_remaining = 0;
        double resource_share = p->resource_share / total_share;
        int num_results_to_skip = (int) ceil(ncpus * resource_share) - 1;
        p->work_request = 0;

        for (vector<RESULT*>::reverse_iterator iter = results.rbegin();
             iter != results.rend(); iter++
        ) {
            RESULT *rp = *iter;
            if (rp->project != p) continue;
            if (num_results_to_skip--) continue;
            if (rp->wup) {
                double cpu_time =
                    estimate_cpu_time(*rp->wup) * (1.0 - get_fraction_done(rp));
                if (cpu_time > 0) {
                    work_remaining += cpu_time * resource_share;
                }
            }
        }

        if (work_remaining < work_min_period) {
            if (work_remaining == 0) {
                urgency = NEED_WORK_IMMEDIATELY;
            }
            p->work_request =
                (2 * work_min_period - work_remaining) * resource_share;
            urgency = NEED_WORK < urgency ? urgency : NEED_WORK;
        }
    }

    return urgency;
}

// called from the client's polling loop.
// initiate scheduler RPC activity if needed and possible
//
bool CLIENT_STATE::scheduler_rpc_poll() {
    double work_secs, work_buf_days;
    int nactive_results;
    PROJECT* p;
    bool action=false, below_work_buf_min, should_get_work;

    switch(scheduler_op->state) {
    case SCHEDULER_OP_STATE_IDLE:
        if (activities_suspended || network_suspended) break;
        if (exit_when_idle && contacted_sched_server) {
            should_get_work = false;
        } else {
            current_work_buf_days(work_buf_days, nactive_results);
            below_work_buf_min = (work_buf_days < global_prefs.work_buf_min_days) || 
                                 (nactive_results < host_info.p_ncpus);
            should_get_work = below_work_buf_min && some_project_rpc_ok();
        }
        if (should_get_work) {
            if (nactive_results < host_info.p_ncpus) {
                msg_printf(NULL, MSG_INFO, "Fewer active results than CPUs; requesting more work");
            } else {
                msg_printf(NULL, MSG_INFO, "Cache low-water mark hit; requesting more work");
            }
            compute_resource_debts();
            scheduler_op->init_get_work();
            action = true;
        } else if ((p=next_project_master_pending())) {
            scheduler_op->init_get_work();
            action = true;
        } else if ((p=next_project_sched_rpc_pending())) {
            scheduler_op->init_return_results(p, 0);
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
    if (strlen(sr.user_name)) {
        safe_strcpy(project->user_name, sr.user_name);
    }
    safe_strcpy(project->team_name, sr.team_name);
    project->user_total_credit = sr.user_total_credit;
    project->user_expavg_credit = sr.user_expavg_credit;
    project->user_create_time = sr.user_create_time;

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

    project->host_total_credit = sr.host_total_credit;
    project->host_expavg_credit = sr.host_expavg_credit;
    if (sr.hostid) {
        project->hostid = sr.hostid;
        project->host_create_time = sr.host_create_time;
        project->rpc_seqno = 0;
    }

	if (strcmp(host_venue, sr.host_venue)) {
        safe_strcpy(host_venue, sr.host_venue);
        need_to_install_prefs = true;
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
		need_to_install_prefs = true;
	}


	if (need_to_install_prefs) {
        bool found_venue;
        retval = global_prefs.parse_file(
            GLOBAL_PREFS_FILE_NAME, host_venue, found_venue
        );
        if (retval) {
            msg_printf(NULL, MSG_ERROR, "Can't parse general preferences");
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
            if (retval) return retval;
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
        if (!app) {
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
        APP_VERSION* avp = lookup_app_version(app, sr.app_versions[i].version_num);
        if (!avp) {
            avp = new APP_VERSION;
            *avp = sr.app_versions[i];
            retval = link_app_version(project, avp);
            if (!retval) app_versions.push_back(avp);
        }
    }
    for (i=0; i<sr.workunits.size(); i++) {
        if (!lookup_workunit(project, sr.workunits[i].name)) {
            WORKUNIT* wup = new WORKUNIT;
            *wup = sr.workunits[i];
            wup->version_num = choose_version_num(wup->app_name, sr);
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
            nresults++;
        } else {
            sprintf(buf, "Already have result %s\n", sr.results[i].name);
            show_message(project, buf, MSG_ERROR);
        }
    }

    // update records for ack'ed results
    //
    for (i=0; i<sr.result_acks.size(); i++) {
        RESULT* rp = lookup_result(project, sr.result_acks[i].name);
        scope_messages.printf("CLIENT_STATE::handle_scheduler_reply(): got ack for result %s\n", sr.result_acks[i].name);
        if (rp) {
            rp->got_server_ack = true;
        } else {
            sprintf(buf, "Got ack for result %s, can't find\n",
                sr.result_acks[i].name
            );
            show_message(project, buf, MSG_ERROR);
        }
    }

    // remove acked trickle files
    //
    if (sr.message_ack) {
        remove_trickle_files(project);
    }
    project->sched_rpc_pending = false;
    set_client_state_dirty("handle_scheduler_reply");
    scope_messages.printf("CLIENT_STATE::handle_scheduler_reply(): State after handle_scheduler_reply():\n");
    print_summary();
    return 0;
}

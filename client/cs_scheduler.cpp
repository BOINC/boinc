// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// High-level logic for communicating with scheduling servers,
// and for merging the reply of a scheduler RPC into the client state
// The scheduler RPC mechanism is in scheduler_op.cpp

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstring>
#include <map>
#include <set>
#endif

#include "crypt.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "url.h"
#include "util.h"

#include "client_msgs.h"
#include "cs_notice.h"
#include "cs_trickle.h"
#include "project.h"
#include "result.h"
#include "scheduler_op.h"
#include "sandbox.h"

#include "client_state.h"

using std::max;
using std::vector;
using std::string;

// quantities like avg CPU time decay by a factor of e every week
//
#define EXP_DECAY_RATE  (1./(SECONDS_PER_DAY*7))

// try to report results this much before their deadline
//
#define REPORT_DEADLINE_CUSHION ((double)SECONDS_PER_DAY)

// report results within this time after completion
//
#define MAX_REPORT_DELAY    3600

#ifndef SIM

// Write a scheduler request to a disk file,
// to be sent to a scheduling server
//
int CLIENT_STATE::make_scheduler_request(PROJECT* p) {
    char buf[1024];
    MIOFILE mf;
    unsigned int i;
    RESULT* rp;

    get_sched_request_filename(*p, buf, sizeof(buf));
    FILE* f = boinc_fopen(buf, "wb");
    if (!f) return ERR_FOPEN;

    double trs = total_resource_share();
    double rrs = runnable_resource_share(RSC_TYPE_ANY);
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

    mf.init_file(f);
    fprintf(f,
        "<scheduler_request>\n"
        "    <authenticator>%s</authenticator>\n"
        "    <hostid>%d</hostid>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <core_client_major_version>%d</core_client_major_version>\n"
        "    <core_client_minor_version>%d</core_client_minor_version>\n"
        "    <core_client_release>%d</core_client_release>\n"
        "    <resource_share_fraction>%f</resource_share_fraction>\n"
        "    <rrs_fraction>%f</rrs_fraction>\n"
        "    <prrs_fraction>%f</prrs_fraction>\n"
        "    <duration_correction_factor>%f</duration_correction_factor>\n"
        "    <allow_multiple_clients>%d</allow_multiple_clients>\n"
        "    <sandbox>%d</sandbox>\n"
        "    <dont_send_work>%d</dont_send_work>\n",
        p->authenticator,
        p->hostid,
        p->rpc_seqno,
        core_client_version.major,
        core_client_version.minor,
        core_client_version.release,
        resource_share_fraction,
        rrs_fraction,
        prrs_fraction,
        p->duration_correction_factor,
        cc_config.allow_multiple_clients?1:0,
        g_use_sandbox?1:0,
        p->dont_request_more_work?1:0
    );
    if (cc_config.dont_use_docker) {
        fprintf(f, "    <dont_use_docker/>\n");
    }
    work_fetch.write_request(f, p);

    // write client capabilities
    //
    fprintf(f,
        "    <client_cap_plan_class>1</client_cap_plan_class>\n"
    );

    write_platforms(p, mf.f);

    if (strlen(p->code_sign_key)) {
        fprintf(f, "    <code_sign_key>\n%s\n</code_sign_key>\n", p->code_sign_key);
    }

    // send working prefs
    //
    fprintf(f, "<working_global_preferences>\n");
    global_prefs.write(mf);
    fprintf(f, "</working_global_preferences>\n");

    // send the oldest CPID with email hash
    //
    USER_CPID* ucp = user_cpids.lookup(p->email_hash);
    if (ucp) {
        fprintf(f,
            "<cross_project_id>%s</cross_project_id>\n",
            ucp->cpid
        );
    }

    time_stats.write(mf, true);
    net_stats.write(mf);
    if (global_prefs.daily_xfer_period_days) {
        daily_xfer_history.write_scheduler_request(
            mf, global_prefs.daily_xfer_period_days
        );
    }

    // update hardware info, and write host info
    //
    host_info.get_host_info(false);
    set_n_usable_cpus();
    host_info.write(mf, !cc_config.suppress_net_info, false);

    // get and write disk usage
    //
    if (!cc_config.no_disk_usage) {
        get_disk_usages();
        get_disk_shares();
        fprintf(f,
            "    <disk_usage>\n"
            "        <d_boinc_used_total>%f</d_boinc_used_total>\n"
            "        <d_boinc_used_project>%f</d_boinc_used_project>\n"
            "        <d_project_share>%f</d_project_share>\n"
            "    </disk_usage>\n",
            total_disk_usage, p->disk_usage, p->disk_share
        );
    }

    if (coprocs.n_rsc > 1) {
        work_fetch.copy_requests();
        coprocs.write_xml(mf, true);
    }

    // report completed jobs
    //
    unsigned int last_reported_index = 0;
    p->nresults_returned = 0;
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project == p && rp->ready_to_report) {
            p->nresults_returned++;
            rp->write(mf, true);
        }
        if (cc_config.max_tasks_reported
            && (p->nresults_returned >= cc_config.max_tasks_reported)
        ) {
            last_reported_index = i;
            break;
        }
    }

    read_trickle_files(p, f);

    // report sticky files as needed
    //
    for (i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if (fip->project != p) continue;
        if (!fip->sticky) continue;
        fprintf(f,
            "    <file_info>\n"
            "        <name>%s</name>\n"
            "        <nbytes>%f</nbytes>\n"
            "        <status>%d</status>\n"
            "    </file_info>\n",
            fip->name, fip->nbytes, fip->status
        );
    }

    if (p->send_time_stats_log) {
        fprintf(f, "<time_stats_log>\n");
        time_stats.get_log_after(p->send_time_stats_log, mf);
        fprintf(f, "</time_stats_log>\n");
    }
    if (p->send_job_log) {
        fprintf(f, "<job_log>\n");
        job_log_filename(*p, buf, sizeof(buf));
        send_log_after(buf, p->send_job_log, mf);
        fprintf(f, "</job_log>\n");
    }

    // send descriptions of app versions
    //
    fprintf(f, "<app_versions>\n");
    int j=0;
    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION* avp = app_versions[i];
        if (avp->project != p) continue;
        avp->write(mf, false);
        avp->index = j++;
    }
    fprintf(f, "</app_versions>\n");

    // send descriptions of jobs in progress for this project
    //
    fprintf(f, "<other_results>\n");
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project != p) continue;
        if ((last_reported_index && (i > last_reported_index)) || !rp->ready_to_report) {
            fprintf(f,
                "    <other_result>\n"
                "        <name>%s</name>\n"
                "        <app_version>%d</app_version>\n",
                rp->name,
                rp->avp->index
            );
            // the following is for backwards compatibility w/ old schedulers
            //
            if (strlen(rp->avp->plan_class)) {
                fprintf(f,
                    "        <plan_class>%s</plan_class>\n",
                    rp->avp->plan_class
                );
            }
            fprintf(f,
                "    </other_result>\n"
            );
        }
    }
    fprintf(f, "</other_results>\n");

    // if requested by project, send summary of all in-progress results
    // (for EDF simulation by scheduler)
    //
    if (p->send_full_workload) {
        fprintf(f, "<in_progress_results>\n");
        for (i=0; i<results.size(); i++) {
            rp = results[i];
            double x = rp->estimated_runtime_remaining();
            if (x == 0) continue;
            safe_strcpy(buf, "");
            int rt = rp->resource_usage.rsc_type;
            if (rt) {
                if (rt == rsc_index(GPU_TYPE_NVIDIA)) {
                    snprintf(buf, sizeof(buf),
                        "        <ncudas>%f</ncudas>\n",
                        rp->resource_usage.coproc_usage
                    );
                } else if (rt == rsc_index(GPU_TYPE_ATI)) {
                    snprintf(buf, sizeof(buf),
                        "        <natis>%f</natis>\n",
                        rp->resource_usage.coproc_usage
                    );
                } else if (rt == rsc_index(GPU_TYPE_INTEL)) {
                    snprintf(buf, sizeof(buf),
                        "        <nintel_gpus>%f</nintel_gpus>\n",
                        rp->resource_usage.coproc_usage
                    );
                }
            }
            fprintf(f,
                "    <ip_result>\n"
                "        <name>%s</name>\n"
                "        <report_deadline>%.0f</report_deadline>\n"
                "        <time_remaining>%.2f</time_remaining>\n"
                "        <avg_ncpus>%f</avg_ncpus>\n"
                "%s"
                "    </ip_result>\n",
                rp->name,
                rp->report_deadline,
                x,
                rp->resource_usage.avg_ncpus,
                buf
            );
        }
        fprintf(f, "</in_progress_results>\n");
    }
    FILE* cof = boinc_fopen(CLIENT_OPAQUE_FILENAME, "r");
    if (cof) {
        fprintf(f, "<client_opaque>\n<![CDATA[\n");
        copy_stream(cof, f);
        fprintf(f, "\n]]>\n</client_opaque>\n");
        fclose(cof);
    }

    if (strlen(client_brand)) {
        fprintf(f, "    <client_brand>%s</client_brand>\n", client_brand);
    }

    if (acct_mgr_info.using_am()) {
        acct_mgr_info.user_keywords.write(f);
    }

    fprintf(f, "</scheduler_request>\n");

    fclose(f);
    return 0;
}

// the project is uploading, and it started recently
//
static inline bool actively_uploading(PROJECT* p) {
    for (unsigned int i=0; i<gstate.file_xfers->file_xfers.size(); i++) {
        FILE_XFER* fxp = gstate.file_xfers->file_xfers[i];
        if (fxp->fip->project != p) continue;
        if (!fxp->is_upload) continue;
        if (gstate.now - fxp->start_time > WF_UPLOAD_DEFER_INTERVAL) continue;
        //msg_printf(p, MSG_INFO, "actively uploading");
        return true;
    }
    //msg_printf(p, MSG_INFO, "not actively uploading");
    return false;
}

// If there is a request for an idle instance, return true.
// Clear other requests
//
static inline bool idle_request() {
    bool found = false;
    for (int i=0; i<coprocs.n_rsc; i++) {
        RSC_WORK_FETCH &rwf = rsc_work_fetch[i];
        if (rwf.req_instances) {
            found = true;
        } else {
            rwf.req_secs = 0;
        }
    }
    return found;
}

// Called once/sec.
// Initiate scheduler RPC activity if needed and possible
//
bool CLIENT_STATE::scheduler_rpc_poll() {
    PROJECT *p;
    static double last_time=0;
    static double last_work_fetch_time = 0;
    double elapsed_time;

    // are we currently doing a scheduler RPC?
    // If so, see if it's finished
    //
    if (scheduler_op->state != SCHEDULER_OP_STATE_IDLE) {
        last_time = now;
        scheduler_op->poll();
        return (scheduler_op->state == SCHEDULER_OP_STATE_IDLE);
    }

    if (network_suspended) return false;

    // check only every 5 sec
    //
    if (!clock_change && now - last_time < SCHEDULER_RPC_POLL_PERIOD) return false;
    last_time = now;

    if (scheduler_op->check_master_fetch_start()) {
        return true;
    }

    // If we haven't run benchmarks yet, don't do a scheduler RPC.
    // We need to know CPU speed to handle app versions
    //
    if (!host_info.p_calculated) return false;

    // check for various reasons to contact particular projects.
    // If we need to contact a project,
    // see if we should ask it for work as well.
    //
    p = next_project_sched_rpc_pending();
    if (p) {
        if (log_flags.sched_op_debug) {
            msg_printf(p, MSG_INFO, "[sched_op] sched RPC pending: %s",
                rpc_reason_string(p->sched_rpc_pending)
            );
        }
        // if the user requested the RPC,
        // clear backoffs to allow work requests
        //
        if (p->sched_rpc_pending == RPC_REASON_USER_REQ) {
            for (int i=0; i<coprocs.n_rsc; i++) {
                p->rsc_pwf[i].clear_backoff();
            }
        }
        work_fetch.piggyback_work_request(p);
        scheduler_op->init_op_project(p, p->sched_rpc_pending);
        return true;
    }
    p = next_project_trickle_up_pending();
    if (p) {
        work_fetch.piggyback_work_request(p);
        scheduler_op->init_op_project(p, RPC_REASON_TRICKLE_UP);
        return true;
    }

    // stuff from here on is checked only once/minute,
    // or if work fetch was requested.
    //

    if (must_check_work_fetch) {
        last_work_fetch_time = 0;
    }
    elapsed_time = now - last_work_fetch_time;
    if (!clock_change && elapsed_time < WORK_FETCH_PERIOD) return false;
    must_check_work_fetch = false;
    last_work_fetch_time = now;

    // check if we should report finished results
    //
    bool suspend_soon = global_prefs.net_times.suspended(now + 1800);
    suspend_soon |= global_prefs.cpu_times.suspended(now + 1800);
    p = find_project_with_overdue_results(suspend_soon);
    if (p) {
        work_fetch.piggyback_work_request(p);
        scheduler_op->init_op_project(p, RPC_REASON_RESULTS_DUE);
        return true;
    }

    // check if we should fetch work (do this last)
    //

    switch (suspend_reason) {
    case 0:
    case SUSPEND_REASON_CPU_THROTTLE:
        break;
    default:
        return false;
    }
    if (cc_config.fetch_minimal_work && had_or_requested_work) {
        return false;
    }

    p = work_fetch.choose_project();
    if (p) {
        if (actively_uploading(p)) {
            bool dont_request = true;
            if (p->pwf.request_if_idle_and_uploading) {
                if (idle_request()) {
                    dont_request = false;
                }
            }
            if (dont_request) {
                if (log_flags.work_fetch_debug) {
                    msg_printf(p, MSG_INFO,
                        "[work_fetch] deferring work fetch; upload active"
                    );
                }
                p->sched_rpc_pending = 0;
                return false;
            }
        }
        scheduler_op->init_op_project(p, RPC_REASON_NEED_WORK);
        return true;
    }
    return false;
}

// Handle the reply from a scheduler
//
int CLIENT_STATE::handle_scheduler_reply(
    PROJECT* project, char* scheduler_url
) {
    SCHEDULER_REPLY sr;
    FILE* f;
    int retval;
    unsigned int i;
    bool signature_valid, update_global_prefs=false, update_project_prefs=false;
    char buf[1024], filename[256];
    string old_gui_urls = project->gui_urls;
    PROJECT* p2;
    vector<RESULT*>new_results;

    project->last_rpc_time = now;

    if (work_fetch.requested_work()) {
        had_or_requested_work = true;
    }

    get_sched_reply_filename(*project, filename, sizeof(filename));

    f = fopen(filename, "rb");
    if (!f) return ERR_FOPEN;
    retval = sr.parse(f, project);
    fclose(f);
    if (retval) return retval;

    if (log_flags.sched_ops) {
        if (work_fetch.requested_work()) {
            snprintf(buf, sizeof(buf), ": got %d new tasks", (int)sr.results.size());
        } else {
            safe_strcpy(buf, "");
        }
        msg_printf(project, MSG_INFO, "Scheduler request completed%s", buf);
    }
    if (log_flags.sched_op_debug) {
        if (sr.scheduler_version) {
            msg_printf(project, MSG_INFO,
                "[sched_op] Server version %d",
                sr.scheduler_version
            );
        }
    }

    // compare our URL for this project with the one returned in the reply
    // (which comes from the project's config.xml).
    // - if http -> https transition, use the https: one from now on
    // - if https -> http transition, keep using the https: one
    // - otherwise switch to the new master URL:
    //      rename and rewrite account file
    //      rename project dir
    //
    if (strlen(sr.master_url)) {
        canonicalize_master_url(sr.master_url, sizeof(sr.master_url));
        string reply_url = sr.master_url;
        string current_url = project->master_url;
        downcase_string(reply_url);
        downcase_string(current_url);
        if (reply_url != current_url) {
            if (is_https_transition(current_url.c_str(), reply_url.c_str())) {
                strcpy(project->master_url, reply_url.c_str());
                project->write_account_file();
                msg_printf(project, MSG_INFO,
                    "Project URL changed from http:// to https://"
                );
            } else if (is_https_transition(reply_url.c_str(), current_url.c_str())) {
                // project is advertising http://, but https:// works.
                // keep using https://
            } else {
                msg_printf(project, MSG_USER_ALERT,
                    _("Master URL changed from %s to %s"),
                    current_url.c_str(), reply_url.c_str()
                );
                char path[MAXPATHLEN], path2[MAXPATHLEN], old_project_dir[MAXPATHLEN];

                // rename statistics file
                //
                get_statistics_filename(
                    (char*)current_url.c_str(), path, sizeof(path)
                );
                get_statistics_filename(
                    (char*)reply_url.c_str(), path2, sizeof(path2)
                );
                boinc_rename(path, path2);

                strcpy(old_project_dir, project->project_dir());

                // delete account file and write new one
                //
                get_account_filename(project->master_url, path, sizeof(path));
                boinc_delete_file(path);
                strcpy(project->master_url, reply_url.c_str());
                project->write_account_file();

                // rename project dir
                //
                strcpy(project->_project_dir, "");
                strcpy(path2, project->project_dir());
                retval = boinc_rename(old_project_dir, path2);
                if (retval) {
                    msg_printf(project, MSG_USER_ALERT,
                        "Can't rename project dir from %s to %s",
                        old_project_dir, path2
                    );
                    return retval;
                }

                // reset the project (clear jobs etc.).
                // If any jobs are running, their soft links
                // point to the old project dir
                //
                reset_project(project, false);
            }
        }
    }

    // make sure we don't already have a project of same name
    //
    bool dup_name = false;
    for (i=0; i<projects.size(); i++) {
        p2 = projects[i];
        if (project == p2) continue;
        if (!strcmp(p2->project_name, project->project_name)) {
            dup_name = true;
            break;
        }
    }
    if (dup_name) {
        msg_printf(project, MSG_INFO,
            "Already attached to a project named %s (possibly with wrong URL)",
            project->project_name
        );
        msg_printf(project, MSG_INFO,
            "Consider detaching this project, then trying again"
        );
    }

    // update user CPID list
    //
    if (strlen(project->cross_project_id) && strlen(project->email_hash)) {
        USER_CPID *ucp = user_cpids.lookup(project->email_hash);
        if (ucp) {
            if (project->cpid_time < ucp->time) {
                strcpy(ucp->cpid, project->cross_project_id);
                ucp->time = project->cpid_time;
            }
        } else {
            USER_CPID uc;
            strcpy(uc.email_hash, project->email_hash);
            strcpy(uc.cpid, project->cross_project_id);
            uc.time = project->cpid_time;
            user_cpids.cpids.push_back(uc);
        }
    }

    // show messages from server
    //
    bool got_notice = false;
    for (i=0; i<sr.messages.size(); i++) {
        USER_MESSAGE& um = sr.messages[i];
        int prio = MSG_INFO;
        if (!strcmp(um.priority.c_str(), "notice")) {
            prio = MSG_SCHEDULER_ALERT;
            got_notice = true;
        }
        msg_printf(project, prio, "%s", um.message.c_str());
    }

    // if we requested work and didn't get notices,
    // clear scheduler notices from this project
    //
    if (work_fetch.requested_work() && !got_notice) {
        notices.remove_notices(project, REMOVE_SCHEDULER_MSG);
    }

    if (log_flags.sched_ops && sr.request_delay) {
        msg_printf(project, MSG_INFO,
            "Project requested delay of %.0f seconds", sr.request_delay
        );
    }

    // if project is down, return error (so that we back off)
    // and don't do anything else
    //
    if (sr.project_is_down) {
        if (sr.request_delay) {
            double x = now + sr.request_delay;
            project->set_min_rpc_time(x, "project requested delay");
        }
        return ERR_PROJECT_DOWN;
    }

    // if the scheduler reply includes global preferences,
    // insert extra elements, write to disk, and parse
    //
    double mod_time = sr.global_prefs_xml?GLOBAL_PREFS::parse_mod_time(sr.global_prefs_xml):0;
    if (sr.global_prefs_xml && mod_time > gstate.global_prefs.mod_time) {
        // ignore prefs if we're using prefs from account mgr
        // BAM! currently has mixed http, https; trim off
        char* p = strchr(global_prefs.source_project, '/');
        char* q = strchr(gstate.acct_mgr_info.master_url, '/');
        if (gstate.acct_mgr_info.using_am() && p && q && !strcmp(p, q)) {
            if (log_flags.sched_op_debug) {
                msg_printf(project, MSG_INFO,
                    "[sched_op] ignoring prefs from project; using prefs from AM"
                );
            }
        } else if (!global_prefs.host_specific || sr.scheduler_version >= 507) {
            // ignore prefs if we have host-specific prefs
            // and we're talking to an old scheduler
            //
            retval = save_global_prefs(
                sr.global_prefs_xml, project->master_url, scheduler_url
            );
            if (retval) {
                return retval;
            }
            update_global_prefs = true;
        } else {
            if (log_flags.sched_op_debug) {
                msg_printf(project, MSG_INFO,
                    "[sched_op] ignoring prefs from old server; we have host-specific prefs"
                );
            }
        }
    }

    // see if we have a new venue from this project
    // (this must go AFTER the above, since otherwise
    // global_prefs_source_project() is meaningless)
    //
    if (strcmp(project->host_venue, sr.host_venue)) {
        safe_strcpy(project->host_venue, sr.host_venue);
        msg_printf(project, MSG_INFO, "New computer location: %s", sr.host_venue);
        update_project_prefs = true;
#ifdef USE_NET_PREFS
        if (project == global_prefs_source_project()) {
            safe_strcpy(main_host_venue, sr.host_venue);
            update_global_prefs = true;
        }
#endif
    }

    if (update_global_prefs) {
        read_global_prefs();
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
            msg_printf(project, MSG_INTERNAL_ERROR,
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

    // notices here serve no purpose.
    // The only thing that may have changed is project prefs,
    // and there's no reason to tell the user what they just did.
    //
    //project->show_no_work_notice();

    // if the scheduler reply includes a code-signing key,
    // accept it if we don't already have one from the project.
    // Otherwise verify its signature, using the key we already have.
    //

    if (sr.code_sign_key) {
        if (!strlen(project->code_sign_key)) {
            safe_strcpy(project->code_sign_key, sr.code_sign_key);
        } else {
            if (sr.code_sign_key_signature) {
                retval = check_string_signature2(
                    sr.code_sign_key, sr.code_sign_key_signature,
                    project->code_sign_key, signature_valid
                );
                if (!retval && signature_valid) {
                    safe_strcpy(project->code_sign_key, sr.code_sign_key);
                } else {
                    msg_printf(project, MSG_INTERNAL_ERROR,
                        "New code signing key doesn't validate"
                    );
                }
            } else {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Missing code sign key signature"
                );
            }
        }
    }

    // copy new entities to client state
    //
    for (i=0; i<sr.apps.size(); i++) {
        APP& checked_app = sr.apps[i];
        APP* app = lookup_app(project, checked_app.name);
        if (app) {
            // update app attributes; they may have changed on server
            //
            safe_strcpy(app->user_friendly_name, checked_app.user_friendly_name);
            app->non_cpu_intensive = checked_app.non_cpu_intensive;
            app->sporadic = checked_app.sporadic;
            app->fraction_done_exact = checked_app.fraction_done_exact;
        } else {
            app = new APP;
            *app = checked_app;
            retval = link_app(project, app);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
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
            if (fip->sticky_lifetime) {
                fip->sticky_expire_time = now + fip->sticky_lifetime;
            }
            retval = link_file_info(project, fip);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
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
            if (log_flags.file_xfer_debug) {
                msg_printf(project, MSG_INFO,
                    "[file_xfer_debug] Got server request to delete file %s",
                    fip->name
                );
            }
            fip->sticky = false;
        }
    }
    for (i=0; i<sr.app_versions.size(); i++) {
        if (project->anonymous_platform) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "App version returned from anonymous platform project; ignoring"
            );
            continue;
        }
        APP_VERSION& avpp = sr.app_versions[i];
        if (strlen(avpp.platform) == 0) {
            safe_strcpy(avpp.platform, get_primary_platform());
        } else {
            if (!is_supported_platform(avpp.platform)) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "App version has unsupported platform %s", avpp.platform
                );
                continue;
            }
        }
        if (avpp.resource_usage.missing_coproc) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "App version uses non-existent %s GPU",
                avpp.resource_usage.missing_coproc_name
            );
        }
        APP* app = lookup_app(project, avpp.app_name);
        if (!app) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Missing app %s", avpp.app_name
            );
            continue;
        }
        APP_VERSION* avp = lookup_app_version(
            app, avpp.platform, avpp.version_num, avpp.plan_class
        );
        if (avp) {
            // don't copy resource usage info from avpp to avp.
            // That would undo app_config.xml.
            // App versions are immutable;
            // if a project wants to change something, create a new one

            // if we had download failures, clear them
            //
            avp->clear_errors();

            continue;
        }
        avp = new APP_VERSION;
        *avp = avpp;
        retval = link_app_version(project, avp);
        if (retval) {
             delete avp;
             continue;
        }
        app_versions.push_back(avp);
    }
    for (i=0; i<sr.workunits.size(); i++) {
        if (lookup_workunit(project, sr.workunits[i].name)) continue;
        WORKUNIT* wup = new WORKUNIT;
        *wup = sr.workunits[i];
        wup->project = project;
        retval = link_workunit(project, wup);
        if (retval) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Can't handle task %s in scheduler reply", wup->name
            );
            delete wup;
            continue;
        }
        wup->clear_errors();
        workunits.push_back(wup);
    }
    double est_rsc_runtime[MAX_RSC];
    bool got_work_for_rsc[MAX_RSC];
    for (int j=0; j<coprocs.n_rsc; j++) {
        est_rsc_runtime[j] = 0;
        got_work_for_rsc[j] = false;
    }
    for (i=0; i<sr.results.size(); i++) {
        RESULT& checked_result = sr.results[i];
        RESULT* rp2 = lookup_result(project, checked_result.name);
        if (rp2) {
            // see if project wants to change the job's deadline
            //
            if (checked_result.report_deadline != rp2->report_deadline) {
                rp2->report_deadline = checked_result.report_deadline;
            } else {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Already have task %s\n", checked_result.name
                );
            }
            continue;
        }
        RESULT* rp = new RESULT;
        *rp = checked_result;
        retval = link_result(project, rp);
        if (retval) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Can't handle task %s in scheduler reply", rp->name
            );
            delete rp;
            continue;
        }
        if (strlen(rp->platform) == 0) {
            safe_strcpy(rp->platform, get_primary_platform());
            rp->version_num = latest_version(rp->wup->app, rp->platform);
        }
        rp->avp = lookup_app_version(
            rp->wup->app, rp->platform, rp->version_num, rp->plan_class
        );
        if (!rp->avp) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "No app version found for app %s platform %s ver %d class %s; discarding %s",
                rp->wup->app->name, rp->platform, rp->version_num, rp->plan_class, rp->name
            );
            delete rp;
            continue;
        }
        rp->init_resource_usage();
        if (rp->resource_usage.missing_coproc) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Missing coprocessor for task %s; aborting", rp->name
            );
            rp->abort_inactive(EXIT_MISSING_COPROC);
        } else {
            rp->set_state(RESULT_NEW, "handle_scheduler_reply");
            got_work_for_rsc[0] = true;
            int rt = rp->resource_usage.rsc_type;
            if (rt > 0) {
                est_rsc_runtime[rt] += rp->estimated_runtime();
                got_work_for_rsc[rt] = true;
                gpus_usable = true;
                    // trigger a check of whether GPU is actually usable
            } else {
                est_rsc_runtime[0] += rp->estimated_runtime();
            }
        }
        rp->wup->version_num = rp->version_num;
        rp->received_time = now;
        new_results.push_back(rp);
        results.push_back(rp);
    }

    // find the resources for which we requested work and didn't get any
    // This is currently used for AM starvation mechanism.
    //
    if (!sr.too_recent) {
        for (int j=0; j<coprocs.n_rsc; j++) {
            RSC_WORK_FETCH& rwf = rsc_work_fetch[j];
            if (got_work_for_rsc[j]) {
                project->sched_req_no_work[j] = false;
            } else if (rwf.req_secs>0 || rwf.req_instances>0) {
                project->sched_req_no_work[j] = true;
            }
        }
    }

    sort_results();

    if (log_flags.sched_op_debug) {
        if (sr.results.size()) {
            for (int j=0; j<coprocs.n_rsc; j++) {
                msg_printf(project, MSG_INFO,
                    "[sched_op] estimated total %s task duration: %.0f seconds",
                    rsc_name_long(j),
                    est_rsc_runtime[j]/time_stats.availability_frac(j)
                );
            }
        }
    }

    // update records for ack'ed results
    //
    for (i=0; i<sr.result_acks.size(); i++) {
        if (log_flags.sched_op_debug) {
            msg_printf(project, MSG_INFO,
                "[sched_op] handle_scheduler_reply(): got ack for task %s\n",
                sr.result_acks[i].name
            );
        }
        RESULT* rp = lookup_result(project, sr.result_acks[i].name);
        if (rp) {
            rp->got_server_ack = true;
        } else {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Got ack for task %s, but can't find it", sr.result_acks[i].name
            );
        }
    }

    // handle result abort requests
    //
    for (i=0; i<sr.result_abort.size(); i++) {
        RESULT* rp = lookup_result(project, sr.result_abort[i].name);
        if (rp) {
            ACTIVE_TASK* atp = lookup_active_task_by_result(rp);
            if (atp) {
                atp->abort_task(EXIT_ABORTED_BY_PROJECT,
                    "aborted by project - no longer usable"
                );
            } else {
                rp->abort_inactive(EXIT_ABORTED_BY_PROJECT);
            }
        } else {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Server requested abort of unknown task %s",
                sr.result_abort[i].name
            );
        }
    }
    for (i=0; i<sr.result_abort_if_not_started.size(); i++) {
        RESULT* rp = lookup_result(project, sr.result_abort_if_not_started[i].name);
        if (!rp) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Server requested conditional abort of unknown task %s",
                sr.result_abort_if_not_started[i].name
            );
            continue;
        }
        if (rp->not_started) {
            rp->abort_inactive(EXIT_ABORTED_BY_PROJECT);
        }
    }

    // remove acked trickle files
    //
    if (sr.message_ack) {
        remove_trickle_files(project);
    }
    if (sr.send_full_workload) {
        project->send_full_workload = true;
    }
    project->dont_use_dcf = sr.dont_use_dcf;
    project->send_time_stats_log = sr.send_time_stats_log;
    project->send_job_log = sr.send_job_log;
    project->trickle_up_pending = false;

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
                "Generated new computer cross-project ID: %s",
                host_info.host_cpid
            );
        }
        //msg_printf(project, MSG_INFO, "Changing host ID from %d to %d", project->hostid, sr.hostid);
        project->hostid = sr.hostid;
        project->rpc_seqno = 0;
    }

#ifdef ENABLE_AUTO_UPDATE
    if (sr.auto_update.present) {
        if (!sr.auto_update.validate_and_link(project)) {
            auto_update = sr.auto_update;
        }
    }
#endif

    project->project_files = sr.project_files;
    project->link_project_files();
    project->create_project_file_symlinks();

    if (log_flags.state_debug) {
        msg_printf(project, MSG_INFO,
            "[state] handle_scheduler_reply(): State after handle_scheduler_reply():"
        );
        print_summary();
    }

    // the following must precede the backoff and request_delay checks,
    // since it overrides them
    //
    if (sr.next_rpc_delay) {
        project->next_rpc_time = now + sr.next_rpc_delay;
    } else {
        project->next_rpc_time = 0;
    }

    work_fetch.handle_reply(project, &sr, new_results);

    project->nrpc_failures = 0;
    project->min_rpc_time = 0;

    if (sr.request_delay) {
        double x = now + sr.request_delay;
        project->set_min_rpc_time(x, "requested by project");
    }

    if (sr.got_rss_feeds) {
        handle_sr_feeds(sr.sr_feeds, project);
    }

    update_trickle_up_urls(project, sr.trickle_up_urls);

    // garbage collect in case the project sent us some irrelevant FILE_INFOs;
    // avoid starting transfers for them
    //
    gstate.garbage_collect_always();

    // if the user provided app_config.xml for this project,
    // apply it to any app versions we just got
    //
    project->app_configs.config_app_versions(project, false);

    // make sure we don't set no_rsc_apps[] for all processor types
    //
    if (!project->anonymous_platform) {
        project->check_no_rsc_apps();
    }

    return 0;
}

#endif // SIM

void CLIENT_STATE::check_project_timeout() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->possibly_backed_off && now > p->min_rpc_time) {
            p->possibly_backed_off = false;
            char buf[1024];
            snprintf(buf, sizeof(buf), "Backoff ended for %s", p->get_project_name());
            request_work_fetch(buf);
        }
    }
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

// find a project for which a scheduler RPC has been requested
// - by user
// - by an account manager
// - by the project
// - because the project was just attached (for verification)
//
PROJECT* CLIENT_STATE::next_project_sched_rpc_pending() {
    unsigned int i;
    PROJECT* p;

    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        bool honor_backoff = true;
        bool honor_suspend = true;

        // is a scheduler-requested RPC due?
        //
        if (!p->sched_rpc_pending && p->next_rpc_time && p->next_rpc_time<now) {
            // don't do it if project is set to no new work
            // and has no jobs currently
            //
            if (!p->dont_request_more_work || p->has_results()) {
                p->sched_rpc_pending = RPC_REASON_PROJECT_REQ;
            }
        }

        switch (p->sched_rpc_pending) {
        case RPC_REASON_USER_REQ:
            honor_backoff = false;
            honor_suspend = false;
            break;
        case RPC_REASON_ACCT_MGR_REQ:
            // This is critical for acct mgrs, to propagate new host CPIDs
            honor_suspend = false;
            break;
        case RPC_REASON_INIT:
            // always do the initial RPC so we can get project name etc.
            honor_suspend = false;
            break;
        case RPC_REASON_PROJECT_REQ:
            break;
        default:
            continue;
        }
        if (honor_backoff && p->waiting_until_min_rpc_time()) {
            continue;
        }
        if (honor_suspend && p->suspended_via_gui) {
            continue;
        }
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

// find a project with finished results that should be reported.
// This means:
//    - we're not backing off contacting the project
//    - no upload for that project is active
//    - the result is ready_to_report (compute done; files uploaded)
//    - we're within a day of the report deadline,
//      or at least a day has elapsed since the result was completed,
//      or we have a sporadic connection
//      or the project is in "don't request more work" state
//      or a network suspend period is coming up soon
//      or the project has > RESULT_REPORT_IF_AT_LEAST_N results ready to report
//
PROJECT* CLIENT_STATE::find_project_with_overdue_results(
    bool network_suspend_soon
) {
    unsigned int i;
    RESULT* r;

    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->n_ready = 0;
        p->dont_contact = false;
        if (p->waiting_until_min_rpc_time()) p->dont_contact = true;
        if (p->suspended_via_gui) p->dont_contact = true;
#ifndef SIM
        if (actively_uploading(p)) p->dont_contact = true;
#endif
    }

    for (i=0; i<results.size(); i++) {
        r = results[i];
        if (!r->ready_to_report) continue;

        PROJECT* p = r->project;
        if (p->dont_contact) continue;

        if (p->dont_request_more_work) {
            return p;
        }

        if (r->report_immediately) {
            return p;
        }

        if (cc_config.report_results_immediately) {
            return p;
        }

        if (p->report_results_immediately) {
            return p;
        }

        if (r->app->report_results_immediately) {
            return p;
        }

        if (net_status.have_sporadic_connection) {
            return p;
        }

        if (network_suspend_soon) {
            return p;
        }

        double cushion = std::max(REPORT_DEADLINE_CUSHION, work_buf_min());
        if (gstate.now > r->report_deadline - cushion) {
            return p;
        }

        if (gstate.now > r->completed_time + MAX_REPORT_DELAY) {
            return p;
        }

        p->n_ready++;
        if (p->n_ready >= RESULT_REPORT_IF_AT_LEAST_N) {
            return p;
        }
    }
    return 0;
}

// trigger work fetch
//
void CLIENT_STATE::request_work_fetch(const char* where) {
    if (log_flags.work_fetch_debug) {
        msg_printf(0, MSG_INFO, "[work_fetch] Request work fetch: %s", where);
    }
    must_check_work_fetch = true;
}


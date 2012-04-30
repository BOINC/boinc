// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

#include <string.h>

#include "str_replace.h"

#include "client_msgs.h"
#include "client_state.h"
#include "log_flags.h"
#include "result.h"
#include "sandbox.h"

#include "project.h"

PROJECT::PROJECT() {
    init();
}

void PROJECT::init() {
    strcpy(master_url, "");
    strcpy(authenticator, "");
    project_specific_prefs = "";
    gui_urls = "";
    resource_share = 100;
    for (int i=0; i<MAX_RSC; i++) {
        no_rsc_pref[i] = false;
        no_rsc_config[i] = false;
        no_rsc_apps[i] = false;
        no_rsc_ams[i] = false;
        rsc_defer_sched[i] = false;
    }
    strcpy(host_venue, "");
    using_venue_specific_prefs = false;
    scheduler_urls.clear();
    strcpy(project_name, "");
    strcpy(symstore, "");
    strcpy(user_name, "");
    strcpy(team_name, "");
    strcpy(email_hash, "");
    strcpy(cross_project_id, "");
    cpid_time = 0;
    user_total_credit = 0;
    user_expavg_credit = 0;
    user_create_time = 0;
    ams_resource_share = -1;
    rpc_seqno = 0;
    userid = 0;
    teamid = 0;
    hostid = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    host_create_time = 0;
    nrpc_failures = 0;
    master_fetch_failures = 0;
    min_rpc_time = 0;
    possibly_backed_off = true;
    master_url_fetch_pending = false;
    sched_rpc_pending = 0;
    next_rpc_time = 0;
    last_rpc_time = 0;
    trickle_up_pending = false;
    anonymous_platform = false;
    non_cpu_intensive = false;
    verify_files_on_app_start = false;
    pwf.reset(this);
    send_time_stats_log = 0;
    send_job_log = 0;
    send_full_workload = false;
    dont_use_dcf = false;
    suspended_via_gui = false;
    dont_request_more_work = false;
    detach_when_done = false;
    attached_via_acct_mgr = false;
    ended = false;
    strcpy(code_sign_key, "");
    user_files.clear();
    project_files.clear();
    next_runnable_result = NULL;
    duration_correction_factor = 1;
    project_files_downloaded_time = 0;
    use_symlinks = false;
    possibly_backed_off = false;
    last_upload_start = 0;
    nuploading_results = 0;
    too_many_uploading_results = false;

#ifdef SIM
    idle_time = 0;
    idle_time_sumsq = 0;
    completed_task_count = 0;
    completions_ratio_mean = 0.0;
    completions_ratio_s = 0.0;
    completions_ratio_stdev = 0.1;  // for the first couple of completions - guess.
    completions_required_stdevs = 3.0;
    result_index = 0;
#endif
}

static void handle_no_rsc_ams(PROJECT* p, const char* name) {
    int i = rsc_index(name);
    if (i < 0) return;
    p->no_rsc_ams[i] = true;
}

static void handle_no_rsc_pref(PROJECT* p, const char* name) {
    int i = rsc_index(name);
    if (i<0) return;
    p->no_rsc_pref[i] = true;
}

static void handle_no_rsc_apps(PROJECT* p, const char* name) {
    int i = rsc_index(name);
    if (i < 0) return;
    p->no_rsc_apps[i] = true;
}

static bool parse_rsc_param(XML_PARSER& xp, const char* end_tag, int& rsc_type, double& value) {
    char name[256];
    bool val_found = false;

    rsc_type = -1;
    while (!xp.get_tag()) {
        if (xp.match_tag(end_tag)) {
            return (rsc_type > 0 && val_found);
        }
        if (xp.parse_str("name", name, sizeof(name))) {
            rsc_type = rsc_index(name);
            continue;
        }
        if (xp.parse_double("rsc_type", value)) {
            val_found = true;
        }
    }
    return false;
}
// parse project fields from client_state.xml
//
int PROJECT::parse_state(XML_PARSER& xp) {
    char buf[256];
    std::string sched_url, stemp;
    string str1, str2;
    int retval, rt;
    double x;
    bool btemp;

    init();
    while (!xp.get_tag()) {
        if (xp.match_tag("/project")) {
            if (cpid_time == 0) {
                cpid_time = user_create_time;
            }
            return 0;
        }
        if (xp.parse_string("scheduler_url", sched_url)) {
            scheduler_urls.push_back(sched_url);
            continue;
        }
        if (xp.parse_str("master_url", master_url, sizeof(master_url))) continue;
        if (xp.parse_str("project_name", project_name, sizeof(project_name))) continue;
        if (xp.parse_str("symstore", symstore, sizeof(symstore))) continue;
        if (xp.parse_str("user_name", user_name, sizeof(user_name))) continue;
        if (xp.parse_str("team_name", team_name, sizeof(team_name))) continue;
        if (xp.parse_str("host_venue", host_venue, sizeof(host_venue))) continue;
        if (xp.parse_str("email_hash", email_hash, sizeof(email_hash))) continue;
        if (xp.parse_str("cross_project_id", cross_project_id, sizeof(cross_project_id))) continue;
        if (xp.parse_double("cpid_time", cpid_time)) continue;
        if (xp.parse_double("user_total_credit", user_total_credit)) continue;
        if (xp.parse_double("user_expavg_credit", user_expavg_credit)) continue;
        if (xp.parse_double("user_create_time", user_create_time)) continue;
        if (xp.parse_int("rpc_seqno", rpc_seqno)) continue;
        if (xp.parse_int("userid", userid)) continue;
        if (xp.parse_int("teamid", teamid)) continue;
        if (xp.parse_int("hostid", hostid)) continue;
        if (xp.parse_double("host_total_credit", host_total_credit)) continue;
        if (xp.parse_double("host_expavg_credit", host_expavg_credit)) continue;
        if (xp.parse_double("host_create_time", host_create_time)) continue;
        if (xp.match_tag("code_sign_key")) {
            retval = copy_element_contents(
                xp.f->f,
                "</code_sign_key>",
                code_sign_key,
                sizeof(code_sign_key)
            );
            if (retval) return retval;
            strip_whitespace(code_sign_key);
            continue;
        }
        if (xp.parse_int("nrpc_failures", nrpc_failures)) continue;
        if (xp.parse_int("master_fetch_failures", master_fetch_failures)) continue;
        if (xp.parse_double("min_rpc_time", min_rpc_time)) continue;
        if (xp.parse_bool("master_url_fetch_pending", master_url_fetch_pending)) continue;
        if (xp.parse_int("sched_rpc_pending", sched_rpc_pending)) continue;
        if (xp.parse_double("next_rpc_time", next_rpc_time)) continue;
        if (xp.parse_bool("trickle_up_pending", trickle_up_pending)) continue;
        if (xp.parse_int("send_time_stats_log", send_time_stats_log)) continue;
        if (xp.parse_int("send_job_log", send_job_log)) continue;
        if (xp.parse_bool("send_full_workload", send_full_workload)) continue;
        if (xp.parse_bool("dont_use_dcf", dont_use_dcf)) continue;
        if (xp.parse_bool("non_cpu_intensive", non_cpu_intensive)) continue;
        if (xp.parse_bool("verify_files_on_app_start", verify_files_on_app_start)) continue;
        if (xp.parse_bool("suspended_via_gui", suspended_via_gui)) continue;
        if (xp.parse_bool("dont_request_more_work", dont_request_more_work)) continue;
        if (xp.parse_bool("detach_when_done", detach_when_done)) continue;
        if (xp.parse_bool("ended", ended)) continue;
        if (xp.parse_double("rec", pwf.rec)) continue;
        if (xp.parse_double("rec_time", pwf.rec_time)) continue;
        if (xp.parse_double("cpu_backoff_interval", rsc_pwf[0].backoff_interval)) continue;
        if (xp.parse_double("cpu_backoff_time", rsc_pwf[0].backoff_time)) {
            if (rsc_pwf[0].backoff_time > gstate.now + 28*SECONDS_PER_DAY) {
                rsc_pwf[0].backoff_time = gstate.now + 28*SECONDS_PER_DAY;
            }
            continue;
        }
        if (xp.match_tag("rsc_backoff_interval")) {
            if (parse_rsc_param(xp, "/rsc_backoff_interval", rt, x)) {
                rsc_pwf[rt].backoff_interval = x;
            }
            continue;
        }
        if (xp.match_tag("rsc_backoff_time")) {
            if (parse_rsc_param(xp, "/rsc_backoff_time", rt, x)) {
                rsc_pwf[rt].backoff_time = x;
            }
            continue;
        }
        if (xp.parse_double("resource_share", resource_share)) continue;
            // not authoritative
        if (xp.parse_double("duration_correction_factor", duration_correction_factor)) continue;
        if (xp.parse_bool("attached_via_acct_mgr", attached_via_acct_mgr)) continue;
        if (xp.parse_bool("no_cpu_apps", btemp)) {
            if (btemp) handle_no_rsc_apps(this, "CPU");
            continue;
        }
        if (xp.parse_bool("no_cuda_apps", btemp)) {
            if (btemp) handle_no_rsc_apps(this, GPU_TYPE_NVIDIA);
            continue;
        }
        if (xp.parse_bool("no_ati_apps", btemp)) {
            if (btemp) handle_no_rsc_apps(this, GPU_TYPE_ATI);
            continue;
        }
        if (xp.parse_str("no_rsc_apps", buf, sizeof(buf))) {
            handle_no_rsc_apps(this, buf);
            continue;
        }
        if (xp.parse_bool("no_cpu_ams", btemp)) {
            if (btemp) handle_no_rsc_ams(this, "CPU");
            continue;
        }
        if (xp.parse_bool("no_cuda_ams", btemp)) {
            if (btemp) handle_no_rsc_ams(this, GPU_TYPE_NVIDIA);
            continue;
        }
        if (xp.parse_bool("no_ati_ams", btemp)) {
            if (btemp) handle_no_rsc_ams(this, GPU_TYPE_ATI);
            continue;
        }
        if (xp.parse_str("no_rsc_ams", buf, sizeof(buf))) {
            handle_no_rsc_ams(this, buf);
            continue;
        }
        if (xp.parse_str("no_rsc_pref", buf, sizeof(buf))) {
            handle_no_rsc_pref(this, buf);
            continue;
        }

            // backwards compat - old state files had ams_resource_share = 0
        if (xp.parse_double("ams_resource_share_new", ams_resource_share)) continue;
        if (xp.parse_double("ams_resource_share", x)) {
            if (x > 0) ams_resource_share = x;
            continue;
        }
        if (xp.parse_bool("scheduler_rpc_in_progress", btemp)) continue;
        if (xp.parse_bool("use_symlinks", use_symlinks)) continue;
        if (xp.parse_bool("anonymous_platform", btemp)) continue;
        if (xp.parse_string("trickle_up_url", stemp)) {
            trickle_up_ops.push_back(new TRICKLE_UP_OP(stemp));
            continue;
        }
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] PROJECT::parse_state(): unrecognized: %s",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

// Write project information to client state file or GUI RPC reply
//
int PROJECT::write_state(MIOFILE& out, bool gui_rpc) {
    unsigned int i;
    char un[2048], tn[2048];

    out.printf(
        "<project>\n"
    );

    xml_escape(user_name, un, sizeof(un));
    xml_escape(team_name, tn, sizeof(tn));
    out.printf(
        "    <master_url>%s</master_url>\n"
        "    <project_name>%s</project_name>\n"
        "    <symstore>%s</symstore>\n"
        "    <user_name>%s</user_name>\n"
        "    <team_name>%s</team_name>\n"
        "    <host_venue>%s</host_venue>\n"
        "    <email_hash>%s</email_hash>\n"
        "    <cross_project_id>%s</cross_project_id>\n"
        "    <cpid_time>%f</cpid_time>\n"
        "    <user_total_credit>%f</user_total_credit>\n"
        "    <user_expavg_credit>%f</user_expavg_credit>\n"
        "    <user_create_time>%f</user_create_time>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <userid>%d</userid>\n"
        "    <teamid>%d</teamid>\n"
        "    <hostid>%d</hostid>\n"
        "    <host_total_credit>%f</host_total_credit>\n"
        "    <host_expavg_credit>%f</host_expavg_credit>\n"
        "    <host_create_time>%f</host_create_time>\n"
        "    <nrpc_failures>%d</nrpc_failures>\n"
        "    <master_fetch_failures>%d</master_fetch_failures>\n"
        "    <min_rpc_time>%f</min_rpc_time>\n"
        "    <next_rpc_time>%f</next_rpc_time>\n"
        "    <rec>%f</rec>\n"
        "    <rec_time>%f</rec_time>\n"

        "    <resource_share>%f</resource_share>\n"
        "    <duration_correction_factor>%f</duration_correction_factor>\n"
        "    <sched_rpc_pending>%d</sched_rpc_pending>\n"
        "    <send_time_stats_log>%d</send_time_stats_log>\n"
        "    <send_job_log>%d</send_job_log>\n"
        "%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
        master_url,
        project_name,
        symstore,
        un,
        tn,
        host_venue,
        email_hash,
        cross_project_id,
        cpid_time,
        user_total_credit,
        user_expavg_credit,
        user_create_time,
        rpc_seqno,
        userid,
        teamid,
        hostid,
        host_total_credit,
        host_expavg_credit,
        host_create_time,
        nrpc_failures,
        master_fetch_failures,
        min_rpc_time,
        next_rpc_time,
        pwf.rec,
        pwf.rec_time,
        resource_share,
        duration_correction_factor,
        sched_rpc_pending,
        send_time_stats_log,
        send_job_log,
        anonymous_platform?"    <anonymous_platform/>\n":"",
        master_url_fetch_pending?"    <master_url_fetch_pending/>\n":"",
        trickle_up_pending?"    <trickle_up_pending/>\n":"",
        send_full_workload?"    <send_full_workload/>\n":"",
        dont_use_dcf?"    <dont_use_dcf/>\n":"",
        non_cpu_intensive?"    <non_cpu_intensive/>\n":"",
        verify_files_on_app_start?"    <verify_files_on_app_start/>\n":"",
        suspended_via_gui?"    <suspended_via_gui/>\n":"",
        dont_request_more_work?"    <dont_request_more_work/>\n":"",
        detach_when_done?"    <detach_when_done/>\n":"",
        ended?"    <ended/>\n":"",
        attached_via_acct_mgr?"    <attached_via_acct_mgr/>\n":"",
        (this == gstate.scheduler_op->cur_proj)?"   <scheduler_rpc_in_progress/>\n":"",
        use_symlinks?"    <use_symlinks/>\n":""
    );
    for (int j=0; j<coprocs.n_rsc; j++) {
        out.printf(
            "    <rsc_backoff_time>\n"
            "        <name>%s</name>\n"
            "        <value>%f</value>\n"
            "    </rsc_backoff_time>\n"
            "    <rsc_backoff_interval>\n"
            "        <name>%s</name>\n"
            "        <value>%f</value>\n"
            "    </rsc_backoff_interval>\n",
            rsc_name(j), rsc_pwf[j].backoff_time,
            rsc_name(j), rsc_pwf[j].backoff_interval
        );
        if (no_rsc_ams[j]) {
            out.printf("    <no_rsc_ams>%s</no_rsc_ams>\n", rsc_name(j));
        }
        if (no_rsc_apps[j]) {
            out.printf("    <no_rsc_apps>%s</no_rsc_apps>\n", rsc_name(j));
        }
        if (no_rsc_pref[j]) {
            out.printf("    <no_rsc_pref>%s</no_rsc_pref>\n", rsc_name(j));
        }
        if (j>0 && gui_rpc && (ncoprocs_excluded[j] == rsc_work_fetch[j].ninstances)) {
            out.printf("    <no_rsc_config>%s</no_rsc_config>\n", rsc_name(j));
        }
    }
    if (ams_resource_share >= 0) {
        out.printf("    <ams_resource_share_new>%f</ams_resource_share_new>\n",
            ams_resource_share
        );
    }
    if (gui_rpc) {
        out.printf(
            "%s"
            "    <sched_priority>%f</sched_priority>\n"
            "    <last_rpc_time>%f</last_rpc_time>\n"
            "    <project_files_downloaded_time>%f</project_files_downloaded_time>\n",
            gui_urls.c_str(),
            sched_priority,
            last_rpc_time,
            project_files_downloaded_time
        );
        if (download_backoff.next_xfer_time > gstate.now) {
            out.printf(
                "    <download_backoff>%f</download_backoff>\n",
                download_backoff.next_xfer_time - gstate.now
            );
        }
        if (upload_backoff.next_xfer_time > gstate.now) {
            out.printf(
                "    <upload_backoff>%f</upload_backoff>\n",
                upload_backoff.next_xfer_time - gstate.now
            );
        }
        if (strlen(host_venue)) {
            out.printf("    <venue>%s</venue>\n", host_venue);
        }
    } else {
       for (i=0; i<scheduler_urls.size(); i++) {
            out.printf(
                "    <scheduler_url>%s</scheduler_url>\n",
                scheduler_urls[i].c_str()
            );
        }
        if (strlen(code_sign_key)) {
            out.printf(
                "    <code_sign_key>\n%s\n</code_sign_key>\n", code_sign_key
            );
        }
        for (i=0; i<trickle_up_ops.size(); i++) {
            TRICKLE_UP_OP* t = trickle_up_ops[i];
            out.printf(
                "    <trickle_up_url>%s</trickle_up_url>\n",
                t->url.c_str()
            );
        }
    }
    out.printf(
        "</project>\n"
    );
    return 0;
}

// Some project data is stored in account file, other in client_state.xml
// Copy fields that are stored in client_state.xml from "p" into "this"
//
void PROJECT::copy_state_fields(PROJECT& p) {
    scheduler_urls = p.scheduler_urls;
    safe_strcpy(project_name, p.project_name);
    safe_strcpy(user_name, p.user_name);
    safe_strcpy(team_name, p.team_name);
    safe_strcpy(host_venue, p.host_venue);
    safe_strcpy(email_hash, p.email_hash);
    safe_strcpy(cross_project_id, p.cross_project_id);
    user_total_credit = p.user_total_credit;
    user_expavg_credit = p.user_expavg_credit;
    user_create_time = p.user_create_time;
    cpid_time = p.cpid_time;
    rpc_seqno = p.rpc_seqno;
    userid = p.userid;
    teamid = p.teamid;
    hostid = p.hostid;
    host_total_credit = p.host_total_credit;
    host_expavg_credit = p.host_expavg_credit;
    host_create_time = p.host_create_time;
    nrpc_failures = p.nrpc_failures;
    master_fetch_failures = p.master_fetch_failures;
    min_rpc_time = p.min_rpc_time;
    next_rpc_time = p.next_rpc_time;
    master_url_fetch_pending = p.master_url_fetch_pending;
    sched_rpc_pending = p.sched_rpc_pending;
    trickle_up_pending = p.trickle_up_pending;
    safe_strcpy(code_sign_key, p.code_sign_key);
    for (int i=0; i<MAX_RSC; i++) {
        rsc_pwf[i] = p.rsc_pwf[i];
        no_rsc_pref[i] = p.no_rsc_pref[i];
        no_rsc_apps[i] = p.no_rsc_apps[i];
        no_rsc_ams[i] = p.no_rsc_ams[i];
    }
    pwf = p.pwf;
    send_full_workload = p.send_full_workload;
    dont_use_dcf = p.dont_use_dcf;
    send_time_stats_log = p.send_time_stats_log;
    send_job_log = p.send_job_log;
    non_cpu_intensive = p.non_cpu_intensive;
    verify_files_on_app_start = p.verify_files_on_app_start;
    suspended_via_gui = p.suspended_via_gui;
    dont_request_more_work = p.dont_request_more_work;
    detach_when_done = p.detach_when_done;
    attached_via_acct_mgr = p.attached_via_acct_mgr;
    ended = p.ended;
    duration_correction_factor = p.duration_correction_factor;
    ams_resource_share = p.ams_resource_share;
    if (ams_resource_share >= 0) {
        resource_share = ams_resource_share;
    }
    use_symlinks = p.use_symlinks;
}

// Write project statistic to project statistics file
//
int PROJECT::write_statistics(MIOFILE& out, bool /*gui_rpc*/) {
    out.printf(
        "<project_statistics>\n"
        "    <master_url>%s</master_url>\n",
        master_url
    );

    for (std::vector<DAILY_STATS>::iterator i=statistics.begin();
        i!=statistics.end(); ++i
    ) {
        out.printf(
            "    <daily_statistics>\n"
            "        <day>%f</day>\n"
            "        <user_total_credit>%f</user_total_credit>\n"
            "        <user_expavg_credit>%f</user_expavg_credit>\n"
            "        <host_total_credit>%f</host_total_credit>\n"
            "        <host_expavg_credit>%f</host_expavg_credit>\n"
            "    </daily_statistics>\n",
            i->day,
            i->user_total_credit,
            i->user_expavg_credit,
            i->host_total_credit,
            i->host_expavg_credit
        );
    }
    out.printf(
        "</project_statistics>\n"
    );
    return 0;
}

void PROJECT::suspend() {
    suspended_via_gui = true;
    gstate.request_schedule_cpus("project suspended");
    gstate.request_work_fetch("project suspended");
}
void PROJECT::resume() {
    suspended_via_gui = false;
    gstate.request_schedule_cpus("project resumed");
    gstate.request_work_fetch("project resumed");
}

void PROJECT::abort_not_started() {
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rp->is_not_started()) {
            rp->abort_inactive(EXIT_ABORTED_VIA_GUI);
        }
    }
}

void PROJECT::get_task_durs(double& not_started_dur, double& in_progress_dur) {
    not_started_dur = 0;
    in_progress_dur = 0;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        double d = rp->estimated_runtime_remaining();
        d /= gstate.time_stats.availability_frac(rp->avp->gpu_usage.rsc_type);
        if (rp->is_not_started()) {
            not_started_dur += d;
        } else {
            in_progress_dur += d;
        }
    }
}

const char* PROJECT::get_scheduler_url(int index, double r) {
    int n = (int) scheduler_urls.size();
    int ir = (int)(r*n);
    int i = (index + ir)%n;
    return scheduler_urls[i].c_str();
}

// delete current sym links.
// This is done when parsing scheduler reply,
// to ensure that we get rid of sym links for
// project files no longer in use
//
void PROJECT::delete_project_file_symlinks() {
    unsigned int i;
    char project_dir[256], path[256];

    get_project_dir(this, project_dir, sizeof(project_dir));
    for (i=0; i<project_files.size(); i++) {
        FILE_REF& fref = project_files[i];
        sprintf(path, "%s/%s", project_dir, fref.open_name);
        delete_project_owned_file(path, false);
    }
}

// install pointers from FILE_REFs to FILE_INFOs for project files,
// and flag FILE_INFOs as being project files.
//
void PROJECT::link_project_files() {
    FILE_INFO* fip;
    vector<FILE_REF>::iterator fref_iter;
    fref_iter = project_files.begin();
    while (fref_iter != project_files.end()) {
        FILE_REF& fref = *fref_iter;
        fip = gstate.lookup_file_info(this, fref.file_name);
        if (!fip) {
            msg_printf(this, MSG_INTERNAL_ERROR,
                "project file refers to non-existent %s", fref.file_name
            );
            fref_iter = project_files.erase(fref_iter);
            continue;
        }
        fref.file_info = fip;
        fip->is_project_file = true;
        fref_iter++;
    }
}

void PROJECT::create_project_file_symlinks() {
    for (unsigned i=0; i<gstate.file_infos.size(); i++) {
        FILE_INFO* fip = gstate.file_infos[i];
        if (fip->project == this && fip->is_project_file && fip->status == FILE_PRESENT) {
            write_symlink_for_project_file(fip);
        }
    }
}

void PROJECT::write_project_files(MIOFILE& f) {
    unsigned int i;

    if (!project_files.size()) return;
    f.printf("<project_files>\n");
    for (i=0; i<project_files.size(); i++) {
        FILE_REF& fref = project_files[i];
        fref.write(f);
    }
    f.printf("</project_files>\n");
}

// write symlinks for project files.
// Note: it's conceivable that one physical file
// has several logical names, so try them all
//
int PROJECT::write_symlink_for_project_file(FILE_INFO* fip) {
    char project_dir[256], link_path[256], file_path[256];
    unsigned int i;

    get_project_dir(this, project_dir, sizeof(project_dir));
    for (i=0; i<project_files.size(); i++) {
        FILE_REF& fref = project_files[i];
        if (fref.file_info != fip) continue;
        sprintf(link_path, "%s/%s", project_dir, fref.open_name);
        sprintf(file_path, "%s/%s", project_dir, fip->name);
        make_soft_link(this, link_path, file_path);
    }
    return 0;
}

// a project file download just finished.
// If it's the last one, update project_files_downloaded_time
//
void PROJECT::update_project_files_downloaded_time() {
    unsigned int i;
    for (i=0; i<project_files.size(); i++) {
        FILE_REF& fref = project_files[i];
        FILE_INFO* fip = fref.file_info;
        if (fip->status != FILE_PRESENT) continue;
    }
    project_files_downloaded_time = gstate.now;
}

bool PROJECT::some_download_stalled() {
#ifndef SIM
    unsigned int i;

    if (!download_backoff.ok_to_transfer()) return true;

    for (i=0; i<gstate.pers_file_xfers->pers_file_xfers.size(); i++) {
        PERS_FILE_XFER* pfx = gstate.pers_file_xfers->pers_file_xfers[i];
        if (pfx->fip->project != this) continue;
        if (pfx->is_upload) continue;
        if (pfx->next_request_time > gstate.now) return true;
    }
#endif
    return false;
}

bool PROJECT::runnable(int rsc_type) {
    if (suspended_via_gui) return false;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rsc_type != RSC_TYPE_ANY) {
            if (rp->avp->gpu_usage.rsc_type != rsc_type) {
                continue;
            }
        }
        if (rp->runnable()) return true;
    }
    return false;
}

bool PROJECT::uploading() {
    for (unsigned int i=0; i<gstate.file_xfers->file_xfers.size(); i++) {
        FILE_XFER& fx = *gstate.file_xfers->file_xfers[i];
        if (fx.fip->project == this && fx.is_upload) {
            return true;
        }
    }
    return false;
}

bool PROJECT::downloading() {
    if (suspended_via_gui) return false;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rp->downloading()) return true;
    }
    return false;
}

bool PROJECT::has_results() {
    for (unsigned i=0; i<gstate.results.size(); i++) {
        RESULT *rp = gstate.results[i];
        if (rp->project == this) return true;
    }
    return false;
}

bool PROJECT::some_result_suspended() {
    unsigned int i;
    for (i=0; i<gstate.results.size(); i++) {
        RESULT *rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rp->suspended_via_gui) return true;
    }
    return false;
}

bool PROJECT::can_request_work() {
    if (suspended_via_gui) return false;
    if (master_url_fetch_pending) return false;
    if (min_rpc_time > gstate.now) return false;
    if (dont_request_more_work) return false;
    if (gstate.in_abort_sequence) return false;
    return true;
}

bool PROJECT::potentially_runnable() {
    if (runnable(RSC_TYPE_ANY)) return true;
    if (can_request_work()) return true;
    if (downloading()) return true;
    return false;
}

bool PROJECT::nearly_runnable() {
    if (runnable(RSC_TYPE_ANY)) return true;
    if (downloading()) return true;
    return false;
}


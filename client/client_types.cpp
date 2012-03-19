// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#include "zlib.h"
#else
#include "config.h"
// Somehow having config.h define _FILE_OFFSET_BITS or _LARGE_FILES is
// causing open to be redefined to open64 which somehow, in some versions
// of zlib.h causes gzopen to be redefined as gzopen64 which subsequently gets
// reported as a linker error.  So for this file, we compile in small files
// mode, regardless of these settings
#undef _FILE_OFFSET_BITS
#undef _LARGE_FILES
#undef _LARGEFILE_SOURCE
#undef _LARGEFILE64_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>
#include <cstring>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "log_flags.h"
#include "md5.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "async_file.h"
#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "pers_file_xfer.h"
#include "sandbox.h"

#include "client_types.h"

using std::string;
using std::vector;

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
        "%s%s%s%s%s%s%s%s%s%s%s%s%s",
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
            rp->abort_inactive(ERR_ABORTED_VIA_GUI);
        }
    }
}

void PROJECT::get_task_durs(double& not_started_dur, double& in_progress_dur) {
    not_started_dur = 0;
    in_progress_dur = 0;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        double d = rp->estimated_time_remaining();
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

bool FILE_XFER_BACKOFF::ok_to_transfer() {
    double dt = next_xfer_time - gstate.now;
    if (dt > gstate.pers_retry_delay_max) {
        // must have changed the system clock
        //
        dt = 0;
    }
    return (dt <= 0);
}

void FILE_XFER_BACKOFF::file_xfer_failed(PROJECT* p) {
    file_xfer_failures++;
    if (file_xfer_failures < FILE_XFER_FAILURE_LIMIT) {
        next_xfer_time = 0;
    } else {
        double backoff = calculate_exponential_backoff(
            file_xfer_failures,
            gstate.pers_retry_delay_min,
            gstate.pers_retry_delay_max
        );
        if (log_flags.file_xfer_debug) {
            msg_printf(p, MSG_INFO,
                "[file_xfer] project-wide xfer delay for %f sec",
                backoff
            );
        }
        next_xfer_time = gstate.now + backoff;
    }
}

void FILE_XFER_BACKOFF::file_xfer_succeeded() {
    file_xfer_failures = 0;
    next_xfer_time  = 0;
}

int PROJECT::parse_project_files(XML_PARSER& xp, bool delete_existing_symlinks) {
    unsigned int i;
    char project_dir[256], path[256];
    int retval;

    if (delete_existing_symlinks) {
        // delete current sym links.
        // This is done when parsing scheduler reply,
        // to ensure that we get rid of sym links for
        // project files no longer in use
        //
        get_project_dir(this, project_dir, sizeof(project_dir));
        for (i=0; i<project_files.size(); i++) {
            FILE_REF& fref = project_files[i];
            sprintf(path, "%s/%s", project_dir, fref.open_name);
            delete_project_owned_file(path, false);
        }
    }

    project_files.clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/project_files")) return 0;
        if (xp.match_tag("file_ref")) {
            FILE_REF file_ref;
            retval = file_ref.parse(xp);
            if (!retval) {
                project_files.push_back(file_ref);
            }
        } else {
            if (log_flags.unparsed_xml) {
                msg_printf(0, MSG_INFO,
                    "[unparsed_xml] parse_project_files(): unrecognized: %s\n",
                    xp.parsed_tag
                );
            }
            xp.skip_unexpected();
        }
    }
    return ERR_XML_PARSE;
}

// install pointers from FILE_REFs to FILE_INFOs for project files,
// and flag FILE_INFOs as being project files.
//
void PROJECT::link_project_files(bool recreate_symlink_files) {
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

    if (recreate_symlink_files) {
        for (unsigned i=0; i<gstate.file_infos.size(); i++) {
            fip = gstate.file_infos[i];
            if (fip->project == this && fip->is_project_file && fip->status == FILE_PRESENT) {
                write_symlink_for_project_file(fip);
            }
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

int APP::parse(XML_PARSER& xp) {
    strcpy(name, "");
    strcpy(user_friendly_name, "");
    project = NULL;
    non_cpu_intensive = false;
    while (!xp.get_tag()) {
        if (xp.match_tag("/app")) {
            if (!strlen(user_friendly_name)) {
                strcpy(user_friendly_name, name);
            }
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("user_friendly_name", user_friendly_name, sizeof(user_friendly_name))) continue;
        if (xp.parse_bool("non_cpu_intensive", non_cpu_intensive)) continue;
#ifdef SIM
        if (xp.parse_double("latency_bound", latency_bound)) continue;
        if (xp.parse_double("fpops_est", fpops_est)) continue;
        if (xp.parse_double("weight", weight)) continue;
        if (xp.parse_double("working_set", working_set)) continue;
        if (xp.match_tag("fpops")) {
            fpops.parse(xp, "/fpops");
            continue;
        }
        if (xp.match_tag("checkpoint_period")) {
            checkpoint_period.parse(xp, "/checkpoint_period");
            continue;
        }
#endif
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] APP::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int APP::write(MIOFILE& out) {
    out.printf(
        "<app>\n"
        "    <name>%s</name>\n"
        "    <user_friendly_name>%s</user_friendly_name>\n"
        "    <non_cpu_intensive>%d</non_cpu_intensive>\n"
        "</app>\n",
        name, user_friendly_name,
        non_cpu_intensive?1:0
    );
    return 0;
}

FILE_INFO::FILE_INFO() {
    strcpy(name, "");
    strcpy(md5_cksum, "");
    max_nbytes = 0;
    nbytes = 0;
    gzipped_nbytes = 0;
    upload_offset = -1;
    status = FILE_NOT_PRESENT;
    executable = false;
    uploaded = false;
    sticky = false;
    gzip_when_done = false;
    download_gzipped = false;
    signature_required = false;
    is_user_file = false;
    is_project_file = false;
    is_auto_update_file = false;
    anonymous_platform_file = false;
    pers_file_xfer = NULL;
    result = NULL;
    project = NULL;
    download_urls.clear();
    upload_urls.clear();
    strcpy(xml_signature, "");
    strcpy(file_signature, "");
    cert_sigs = 0;
    async_verify = NULL;
}

FILE_INFO::~FILE_INFO() {
    if (async_verify) {
        remove_async_verify(async_verify);
    }
}

void FILE_INFO::reset() {
    status = FILE_NOT_PRESENT;
    delete_file();
    error_msg = "";
}

// Set the appropriate permissions depending on whether
// it's an executable file
// This doesn't seem to exist in Windows
//
int FILE_INFO::set_permissions() {
#ifdef _WIN32
    return 0;
#else
    int retval;
    char pathname[256];
    get_pathname(this, pathname, sizeof(pathname));

    if (g_use_sandbox) {
        // give exec permissions for user, group and others but give
        // read permissions only for user and group to protect account keys
        retval = set_to_project_group(pathname);
        if (retval) return retval;
        if (executable) {
            retval = chmod(pathname,
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IWGRP|S_IXGRP
                |S_IXOTH
            );
        } else {
            retval = chmod(pathname,
                S_IRUSR|S_IWUSR
                |S_IRGRP|S_IWGRP
            );
        }
    } else {
        // give read/exec permissions for user, group and others
        // in case someone runs BOINC from different user
        if (executable) {
            retval = chmod(pathname,
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IXGRP
                |S_IROTH|S_IXOTH
            );
        } else {
            retval = chmod(pathname,
                S_IRUSR|S_IWUSR
                |S_IRGRP
                |S_IROTH
            );
        }
    }
    return retval;
#endif
}

int FILE_INFO::parse(XML_PARSER& xp) {
    char buf2[1024];
    std::string url;
    PERS_FILE_XFER *pfxp;
    int retval;
    bool btemp;
    vector<string>gzipped_urls;

    while (!xp.get_tag()) {
        if (xp.match_tag("/file_info") || xp.match_tag("/file")) {
            if (!strlen(name)) return ERR_BAD_FILENAME;
            if (strstr(name, "..")) return ERR_BAD_FILENAME;
            if (strstr(name, "%")) return ERR_BAD_FILENAME;
            if (gzipped_urls.size() > 0) {
                download_urls.clear();
                download_urls.urls = gzipped_urls;
                download_gzipped = true;
            }
            return 0;
        }
        if (xp.match_tag("xml_signature")) {
            retval = copy_element_contents(
                xp.f->f,
                "</xml_signature>",
                xml_signature,
                sizeof(xml_signature)
            );
            if (retval) return retval;
            strip_whitespace(xml_signature);
            continue;
        }
        if (xp.match_tag("file_signature")) {
            retval = copy_element_contents(
                xp.f->f,
                "</file_signature>",
                file_signature,
                sizeof(file_signature)
            );
            if (retval) return retval;
            strip_whitespace(file_signature);
            continue;
        }
        if (xp.match_tag("signatures")) {
            if (!cert_sigs->parse(xp)) {
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "FILE_INFO::parse(): cannot parse <signatures>\n"
                );
                return ERR_XML_PARSE;
            }
            continue;
        }

        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_string("url", url)) {
            if (strstr(url.c_str(), "file_upload_handler")) {
                upload_urls.urls.push_back(url);
            } else {
                download_urls.urls.push_back(url);
            }
            continue;
        }
        if (xp.parse_string("download_url", url)) {
            download_urls.urls.push_back(url);
            continue;
        }
        if (xp.parse_string("upload_url", url)) {
            upload_urls.urls.push_back(url);
            continue;
        }
        if (xp.parse_string("gzipped_url", url)) {
            gzipped_urls.push_back(url);
            continue;
        }
        if (xp.parse_str("md5_cksum", md5_cksum, sizeof(md5_cksum))) continue;
        if (xp.parse_double("nbytes", nbytes)) continue;
        if (xp.parse_double("gzipped_nbytes", gzipped_nbytes)) continue;
        if (xp.parse_double("max_nbytes", max_nbytes)) continue;
        if (xp.parse_int("status", status)) {
            // on startup, VERIFY_PENDING is meaningless
            if (status == FILE_VERIFY_PENDING) {
                status = FILE_NOT_PRESENT;
            }
            continue;
        }
        if (xp.parse_bool("executable", executable)) continue;
        if (xp.parse_bool("uploaded", uploaded)) continue;
        if (xp.parse_bool("sticky", sticky)) continue;
        if (xp.parse_bool("gzip_when_done", gzip_when_done)) continue;
        if (xp.parse_bool("download_gzipped", download_gzipped)) continue;
        if (xp.parse_bool("signature_required", signature_required)) continue;
        if (xp.parse_bool("is_project_file", is_project_file)) continue;
        if (xp.parse_bool("no_delete", btemp)) continue;
        if (xp.match_tag("persistent_file_xfer")) {
            pfxp = new PERS_FILE_XFER;
            retval = pfxp->parse(xp);
#ifdef SIM
            delete pfxp;
            continue;
#endif
            if (!retval) {
                pers_file_xfer = pfxp;
            } else {
                delete pfxp;
            }
            continue;
        }
        if (xp.match_tag("file_xfer")) {
            while (!xp.get_tag()) {
                if (xp.match_tag("/file_xfer")) break;
            }
            continue;
        }
        if (xp.match_tag("error_msg")) {
            retval = copy_element_contents(
                xp.f->f,
                "</error_msg>", buf2, sizeof(buf2)
            );
            if (retval) return retval;
            error_msg = buf2;
            continue;
        }
        // deprecated tags
        if (xp.parse_bool("generated_locally", btemp)) continue;
        if (xp.parse_bool("upload_when_present", btemp)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] FILE_INFO::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int FILE_INFO::write(MIOFILE& out, bool to_server) {
    unsigned int i;
    int retval;
    char buf[1024];

    if (to_server) {
        out.printf("<file_info>\n");
    } else {
        out.printf("<file>\n");
    }
    out.printf(
        "    <name>%s</name>\n"
        "    <nbytes>%f</nbytes>\n"
        "    <max_nbytes>%f</max_nbytes>\n",
        name, nbytes, max_nbytes
    );
    if (strlen(md5_cksum)) {
        out.printf(
            "    <md5_cksum>%s</md5_cksum>\n",
            md5_cksum
        );
    }
    if (!to_server) {
        out.printf("    <status>%d</status>\n", status);
        if (executable) out.printf("    <executable/>\n");
        if (uploaded) out.printf("    <uploaded/>\n");
        if (sticky) out.printf("    <sticky/>\n");
        if (gzip_when_done) out.printf("    <gzip_when_done/>\n");
        if (download_gzipped) {
            out.printf("    <download_gzipped/>\n");
            out.printf("    <gzipped_nbytes>%.0f</gzipped_nbytes>\n", gzipped_nbytes);
        }
        if (signature_required) out.printf("    <signature_required/>\n");
        if (is_user_file) out.printf("    <is_user_file/>\n");
        if (strlen(file_signature)) out.printf("    <file_signature>\n%s\n</file_signature>\n", file_signature);
    }
    for (i=0; i<download_urls.urls.size(); i++) {
        xml_escape(download_urls.urls[i].c_str(), buf, sizeof(buf));
        out.printf("    <download_url>%s</download_url>\n", buf);
    }
    for (i=0; i<upload_urls.urls.size(); i++) {
        xml_escape(upload_urls.urls[i].c_str(), buf, sizeof(buf));
        out.printf("    <upload_url>%s</upload_url>\n", buf);
    }
    if (!to_server && pers_file_xfer) {
        retval = pers_file_xfer->write(out);
        if (retval) return retval;
    }
    if (!to_server) {
        if (strlen(xml_signature)) {
            out.printf(
                "    <xml_signature>\n%s    </xml_signature>\n",
                xml_signature
            );
        }
    }
    if (!error_msg.empty()) {
        strip_whitespace(error_msg);
        out.printf("    <error_msg>\n%s\n</error_msg>\n", error_msg.c_str());
    }
    if (to_server) {
        out.printf("</file_info>\n");
    } else {
        out.printf("</file>\n");
    }
    return 0;
}

// called only for files with a PERS_FILE_XFER
//
int FILE_INFO::write_gui(MIOFILE& out) {
    out.printf(
        "<file_transfer>\n"
        "    <project_url>%s</project_url>\n"
        "    <project_name>%s</project_name>\n"
        "    <name>%s</name>\n"
        "    <nbytes>%f</nbytes>\n"
        "    <max_nbytes>%f</max_nbytes>\n"
        "    <status>%d</status>\n",
        project->master_url,
        project->project_name,
        name,
        download_gzipped?gzipped_nbytes:nbytes,
        max_nbytes,
        status
    );

    pers_file_xfer->write(out);

    FILE_XFER_BACKOFF& fxb = project->file_xfer_backoff(pers_file_xfer->is_upload);
    if (fxb.next_xfer_time > gstate.now) {
        out.printf("    <project_backoff>%f</project_backoff>\n",
            fxb.next_xfer_time - gstate.now
        );
    }
    out.printf("</file_transfer>\n");
    return 0;
}

// delete physical underlying file associated with FILE_INFO
//
int FILE_INFO::delete_file() {
    char path[256];

    get_pathname(this, path, sizeof(path));
    int retval = delete_project_owned_file(path, true);

    // files with download_gzipped set may exist
    // in temporary or compressed form
    //
    strcat(path, ".gz");
    delete_project_owned_file(path, true);
    strcat(path, "t");
    delete_project_owned_file(path, true);

    if (retval && status != FILE_NOT_PRESENT) {
        msg_printf(project, MSG_INTERNAL_ERROR, "Couldn't delete file %s", path);
    }
    status = FILE_NOT_PRESENT;
    return retval;
}

const char* URL_LIST::get_init_url() {
    if (!urls.size()) {
        return NULL;
    }

// if a project supplies multiple URLs, try them in order
// (e.g. in Einstein@home they're ordered by proximity to client).
//
    current_index = 0;
    start_index = current_index;
    return urls[current_index].c_str();
}

// Call this to get the next URL.
// NULL return means you've tried them all.
//
const char* URL_LIST::get_next_url() {
    if (!urls.size()) return NULL;
    while(1) {
        current_index = (current_index + 1)%((int)urls.size());
        if (current_index == start_index) {
            return NULL;
        }
        return urls[current_index].c_str();
    }
}

const char* URL_LIST::get_current_url(FILE_INFO& fi) {
    if (current_index < 0) {
        return get_init_url();
    }
    if (current_index >= (int)urls.size()) {
        msg_printf(fi.project, MSG_INTERNAL_ERROR,
            "File %s has no URL", fi.name
        );
        return NULL;
    }
    return urls[current_index].c_str();
}

// merges information from a new FILE_INFO that has the same name as one
// that is already present in the client state file.
//
int FILE_INFO::merge_info(FILE_INFO& new_info) {
    char buf[256];

    if (max_nbytes <= 0 && new_info.max_nbytes) {
        max_nbytes = new_info.max_nbytes;
        sprintf(buf, "    <max_nbytes>%.0f</max_nbytes>\n", new_info.max_nbytes);
    }

    // replace existing URLs with new ones
    //

    download_urls.replace(new_info.download_urls);
    upload_urls.replace(new_info.upload_urls);
    download_gzipped = new_info.download_gzipped;

    // replace signatures
    //
    if (strlen(new_info.file_signature)) {
        strcpy(file_signature, new_info.file_signature);
    }
    if (strlen(new_info.xml_signature)) {
        strcpy(xml_signature, new_info.xml_signature);
    }

    // If the file is supposed to be executable and is PRESENT,
    // make sure it's actually executable.
    // This deals with cases where somehow a file didn't
    // get protected right when it was initially downloaded.
    //
    if (status == FILE_PRESENT && new_info.executable) {
        int retval = set_permissions();
        if (retval) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "merge_info(): failed to change permissions of %s", name
            );
        }
        return retval;
    }

    return 0;
}

// Returns true if the file had an unrecoverable error
// (couldn't download, RSA/MD5 check failed, etc)
//
bool FILE_INFO::had_failure(int& failnum) {
    switch (status) {
    case FILE_NOT_PRESENT:
    case FILE_PRESENT:
    case FILE_VERIFY_PENDING:
        return false;
    }
    failnum = status;
    return true;
}

void FILE_INFO::failure_message(string& s) {
    char buf[1024];
    sprintf(buf,
        "<file_xfer_error>\n"
        "  <file_name>%s</file_name>\n"
        "  <error_code>%d</error_code>\n",
        name,
        status
    );
    s = buf;
    if (error_msg.size()) {
        sprintf(buf,
            "  <error_message>%s</error_message>\n",
            error_msg.c_str()
            );
        s = s + buf;
    }
    s = s + "</file_xfer_error>\n";
}

#define BUFSIZE 16384
int FILE_INFO::gzip() {
    char buf[BUFSIZE];
    char inpath[256], outpath[256];

    get_pathname(this, inpath, sizeof(inpath));
    strcpy(outpath, inpath);
    strcat(outpath, ".gz");
    FILE* in = boinc_fopen(inpath, "rb");
    if (!in) return ERR_FOPEN;
    gzFile out = gzopen(outpath, "wb");
    while (1) {
        int n = (int)fread(buf, 1, BUFSIZE, in);
        if (n <= 0) break;
        int m = gzwrite(out, buf, n);
        if (m != n) {
            fclose(in);
            gzclose(out);
            return ERR_WRITE;
        }
    }
    fclose(in);
    gzclose(out);
    delete_project_owned_file(inpath, true);
    boinc_rename(outpath, inpath);
    return 0;
}

// unzip a file, and compute the uncompressed MD5 at the same time
//
int FILE_INFO::gunzip(char* md5_buf) {
    unsigned char buf[BUFSIZE];
    char inpath[256], outpath[256], tmppath[256];
    md5_state_t md5_state;

    md5_init(&md5_state);
    get_pathname(this, outpath, sizeof(outpath));
    strcpy(inpath, outpath);
    strcat(inpath, ".gz");
    strcpy(tmppath, outpath);
    char* p = strrchr(tmppath, '/');
    strcpy(p+1, "decompress_temp");
    FILE* out = boinc_fopen(tmppath, "wb");
    if (!out) return ERR_FOPEN;
    gzFile in = gzopen(inpath, "rb");
    while (1) {
        int n = gzread(in, buf, BUFSIZE);
        if (n <= 0) break;
        int m = (int)fwrite(buf, 1, n, out);
        if (m != n) {
            gzclose(in);
            fclose(out);
            return ERR_WRITE;
        }
        md5_append(&md5_state, buf, n);
    }
    unsigned char binout[16];
    md5_finish(&md5_state, binout);
    for (int i=0; i<16; i++) {
        sprintf(md5_buf+2*i, "%02x", binout[i]);
    }
    md5_buf[32] = 0;

    gzclose(in);
    fclose(out);
    boinc_rename(tmppath, outpath);
    delete_project_owned_file(inpath, true);
    return 0;
}

int APP_VERSION::parse(XML_PARSER& xp) {
    FILE_REF file_ref;
    double dtemp;

    strcpy(app_name, "");
    strcpy(api_version, "");
    version_num = 0;
    strcpy(platform, "");
    strcpy(plan_class, "");
    strcpy(cmdline, "");
    strcpy(file_prefix, "");
    avg_ncpus = 1;
    max_ncpus = 1;
    gpu_usage.rsc_type = 0;
    gpu_usage.usage = 0;
    gpu_ram = 0;
    app = NULL;
    project = NULL;
    flops = gstate.host_info.p_fpops;
    missing_coproc = false;
    strcpy(missing_coproc_name, "");
    dont_throttle = false;
    needs_network = false;

    while (!xp.get_tag()) {
        if (xp.match_tag("/app_version")) return 0;
        if (xp.parse_str("app_name", app_name, sizeof(app_name))) continue;
        if (xp.match_tag("file_ref")) {
            file_ref.parse(xp);
            app_files.push_back(file_ref);
            continue;
        }
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.parse_str("api_version", api_version, sizeof(api_version))) continue;
        if (xp.parse_str("platform", platform, sizeof(platform))) continue;
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) continue;
        if (xp.parse_double("avg_ncpus", avg_ncpus)) continue;
        if (xp.parse_double("max_ncpus", max_ncpus)) continue;
        if (xp.parse_double("flops", dtemp)) {
            if (dtemp <= 0) {
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "non-positive FLOPS in app version"
                );
            } else {
                flops = dtemp;
            }
            continue;
        }
        if (xp.parse_str("cmdline", cmdline, sizeof(cmdline))) continue;
        if (xp.parse_str("file_prefix", file_prefix, sizeof(file_prefix))) continue;
        if (xp.parse_double("gpu_ram", gpu_ram)) continue;
        if (xp.match_tag("coproc")) {
            COPROC_REQ cp;
            int retval = cp.parse(xp);
            if (!retval) {
                int rt = rsc_index(cp.type);
                if (rt > 0) {
                    gpu_usage.rsc_type = rt;
                    gpu_usage.usage = cp.count;
                } else {
                    missing_coproc = true;
                    missing_coproc_usage = cp.count;
                    strcpy(missing_coproc_name, cp.type);
                }
            } else {
                msg_printf(0, MSG_INTERNAL_ERROR, "Error parsing <coproc>");
            }
            continue;
        }
        if (xp.parse_bool("dont_throttle", dont_throttle)) continue;
        if (xp.parse_bool("needs_network", needs_network)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] APP_VERSION::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int APP_VERSION::write(MIOFILE& out, bool write_file_info) {
    unsigned int i;
    int retval;

    out.printf(
        "<app_version>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n"
        "    <platform>%s</platform>\n"
        "    <avg_ncpus>%f</avg_ncpus>\n"
        "    <max_ncpus>%f</max_ncpus>\n"
        "    <flops>%f</flops>\n",
        app_name,
        version_num,
        platform,
        avg_ncpus,
        max_ncpus,
        flops
    );
    if (strlen(plan_class)) {
        out.printf("    <plan_class>%s</plan_class>\n", plan_class);
    }
    if (strlen(api_version)) {
        out.printf("    <api_version>%s</api_version>\n", api_version);
    }
    if (strlen(cmdline)) {
        out.printf("    <cmdline>%s</cmdline>\n", cmdline);
    }
    if (strlen(file_prefix)) {
        out.printf("    <file_prefix>%s</file_prefix>\n", file_prefix);
    }
    if (write_file_info) {
        for (i=0; i<app_files.size(); i++) {
            retval = app_files[i].write(out);
            if (retval) return retval;
        }
    }
    if (gpu_usage.rsc_type) {
        const char* p = rsc_name(gpu_usage.rsc_type);
        if (!strcmp(p, "NVIDIA")) p = "CUDA";
        out.printf(
            "    <coproc>\n"
            "        <type>%s</type>\n"
            "        <count>%f</count>\n"
            "    </coproc>\n",
            p,
            gpu_usage.usage
        );
    }
    if (missing_coproc && strlen(missing_coproc_name)) {
        out.printf(
            "    <coproc>\n"
            "        <type>%s</type>\n"
            "        <count>%f</count>\n"
            "    </coproc>\n",
            missing_coproc_name,
            missing_coproc_usage
        );
    }
    if (gpu_ram) {
        out.printf(
            "    <gpu_ram>%f</gpu_ram>\n",
            gpu_ram
        );
    }
    if (dont_throttle) {
        out.printf(
            "    <dont_throttle/>\n"
        );
    }
    if (needs_network) {
        out.printf(
            "    <needs_network/>\n"
        );
    }

    out.printf(
        "</app_version>\n"
    );
    return 0;
}

bool APP_VERSION::had_download_failure(int& failnum) {
    unsigned int i;

    for (i=0; i<app_files.size();i++) {
        if (app_files[i].file_info->had_failure(failnum)) {
            return true;
        }
    }
    return false;
}

void APP_VERSION::get_file_errors(string& str) {
    int errnum;
    unsigned int i;
    FILE_INFO* fip;
    string msg;

    str = "couldn't get input files:\n";
    for (i=0; i<app_files.size();i++) {
        fip = app_files[i].file_info;
        if (fip->had_failure(errnum)) {
            fip->failure_message(msg);
            str = str + msg;
        }
    }
}

void APP_VERSION::clear_errors() {
    int x;
    unsigned int i;
    for (i=0; i<app_files.size();i++) {
        FILE_INFO* fip = app_files[i].file_info;
        if (fip->had_failure(x)) {
            fip->reset();
        }
    }
}

int APP_VERSION::api_major_version() {
    int v, n;
    n = sscanf(api_version, "%d", &v);
    if (n != 1) return 0;
    return v;
}

int FILE_REF::parse(XML_PARSER& xp) {
    bool temp;

    strcpy(file_name, "");
    strcpy(open_name, "");
    main_program = false;
    copy_file = false;
    optional = false;
    while (!xp.get_tag()) {
        if (xp.match_tag("/file_ref")) return 0;
        if (xp.parse_str("file_name", file_name, sizeof(file_name))) continue;
        if (xp.parse_str("open_name", open_name, sizeof(open_name))) continue;
        if (xp.parse_bool("main_program", main_program)) continue;
        if (xp.parse_bool("copy_file", copy_file)) continue;
        if (xp.parse_bool("optional", optional)) continue;
        if (xp.parse_bool("no_validate", temp)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] FILE_REF::parse(): unrecognized: '%s'\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int FILE_REF::write(MIOFILE& out) {

    out.printf(
        "    <file_ref>\n"
        "        <file_name>%s</file_name>\n",
        file_name
    );
    if (strlen(open_name)) {
        out.printf("        <open_name>%s</open_name>\n", open_name);
    }
    if (main_program) {
        out.printf("        <main_program/>\n");
    }
    if (copy_file) {
        out.printf("        <copy_file/>\n");
    }
    if (optional) {
        out.printf("        <optional/>\n");
    }
    out.printf("    </file_ref>\n");
    return 0;
}

int WORKUNIT::parse(XML_PARSER& xp) {
    FILE_REF file_ref;
    double dtemp;

    strcpy(name, "");
    strcpy(app_name, "");
    version_num = 0;
    command_line = "";
    //strcpy(env_vars, "");
    app = NULL;
    project = NULL;
    // Default these to very large values (1 week on a 1 cobblestone machine)
    // so we don't keep asking the server for more work
    rsc_fpops_est = 1e9*SECONDS_PER_DAY*7;
    rsc_fpops_bound = 4e9*SECONDS_PER_DAY*7;
    rsc_memory_bound = 1e8;
    rsc_disk_bound = 1e9;
    while (!xp.get_tag()) {
        if (xp.match_tag("/workunit")) return 0;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("app_name", app_name, sizeof(app_name))) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.parse_string("command_line", command_line)) {
            strip_whitespace(command_line);
            continue;
        }
        //if (xp.parse_str("env_vars", env_vars, sizeof(env_vars))) continue;
        if (xp.parse_double("rsc_fpops_est", rsc_fpops_est)) continue;
        if (xp.parse_double("rsc_fpops_bound", rsc_fpops_bound)) continue;
        if (xp.parse_double("rsc_memory_bound", rsc_memory_bound)) continue;
        if (xp.parse_double("rsc_disk_bound", rsc_disk_bound)) continue;
        if (xp.match_tag("file_ref")) {
            file_ref.parse(xp);
#ifndef SIM
            input_files.push_back(file_ref);
#endif
            continue;
        }
        // unused stuff
        if (xp.parse_double("credit", dtemp)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] WORKUNIT::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int WORKUNIT::write(MIOFILE& out) {
    unsigned int i;

    out.printf(
        "<workunit>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n"
        //"    <env_vars>%s</env_vars>\n"
        "    <rsc_fpops_est>%f</rsc_fpops_est>\n"
        "    <rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "    <rsc_memory_bound>%f</rsc_memory_bound>\n"
        "    <rsc_disk_bound>%f</rsc_disk_bound>\n",
        name,
        app_name,
        version_num,
        //env_vars,
        rsc_fpops_est,
        rsc_fpops_bound,
        rsc_memory_bound,
        rsc_disk_bound
    );
    if (command_line.size()) {
        out.printf(
            "    <command_line>\n"
            "%s\n"
            "    </command_line>\n",
            command_line.c_str()
        );
    }
    for (i=0; i<input_files.size(); i++) {
        input_files[i].write(out);
    }
    out.printf("</workunit>\n");
    return 0;
}

bool WORKUNIT::had_download_failure(int& failnum) {
    unsigned int i;

    for (i=0;i<input_files.size();i++) {
        if (input_files[i].file_info->had_failure(failnum)) {
            return true;
        }
    }
    return false;
}

void WORKUNIT::get_file_errors(string& str) {
    int x;
    unsigned int i;
    FILE_INFO* fip;
    string msg;

    str = "couldn't get input files:\n";
    for (i=0;i<input_files.size();i++) {
        fip = input_files[i].file_info;
        if (fip->had_failure(x)) {
            fip->failure_message(msg);
            str = str + msg;
        }
    }
}

// if any input files had download error from previous WU,
// reset them to try download again
//
void WORKUNIT::clear_errors() {
    int x;
    unsigned int i;
    for (i=0; i<input_files.size();i++) {
        FILE_INFO* fip = input_files[i].file_info;
        if (fip->had_failure(x)) {
            fip->reset();
        }
    }
}

int RESULT::parse_name(XML_PARSER& xp, const char* end_tag) {
    strcpy(name, "");
    while (!xp.get_tag()) {
        if (xp.match_tag(end_tag)) return 0;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] RESULT::parse_name(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

void RESULT::clear() {
    strcpy(name, "");
    strcpy(wu_name, "");
    report_deadline = 0;
    received_time = 0;
    output_files.clear();
    _state = RESULT_NEW;
    ready_to_report = false;
    completed_time = 0;
    got_server_ack = false;
    final_cpu_time = 0;
    final_elapsed_time = 0;
#ifdef SIM
    peak_flop_count = 0;
#endif
    exit_status = 0;
    stderr_out = "";
    suspended_via_gui = false;
    rr_sim_misses_deadline = false;
    fpops_per_cpu_sec = 0;
    fpops_cumulative = 0;
    intops_per_cpu_sec = 0;
    intops_cumulative = 0;
    app = NULL;
    wup = NULL;
    project = NULL;
    version_num = 0;
    strcpy(platform, "");
    strcpy(plan_class, "");
    strcpy(resources, "");
    coproc_missing = false;
    report_immediately = false;
    schedule_backoff = 0;
    strcpy(schedule_backoff_reason, "");
}

// parse a <result> element from scheduling server.
//
int RESULT::parse_server(XML_PARSER& xp) {
    FILE_REF file_ref;

    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/result")) return 0;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("wu_name", wu_name, sizeof(wu_name))) continue;
        if (xp.parse_double("report_deadline", report_deadline)) continue;
        if (xp.parse_str("platform", platform, sizeof(platform))) continue;
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.match_tag("file_ref")) {
            file_ref.parse(xp);
            output_files.push_back(file_ref);
            continue;
        }
        if (xp.parse_bool("report_immediately", report_immediately)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] RESULT::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

// parse a <result> element from state file
//
int RESULT::parse_state(XML_PARSER& xp) {
    FILE_REF file_ref;

    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/result")) {
            // set state to something reasonable in case of bad state file
            //
            if (got_server_ack || ready_to_report) {
                switch (state()) {
                case RESULT_NEW:
                case RESULT_FILES_DOWNLOADING:
                case RESULT_FILES_DOWNLOADED:
                case RESULT_FILES_UPLOADING:
                    set_state(RESULT_FILES_UPLOADED, "RESULT::parse_state");
                    break;
                }
            }
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("wu_name", wu_name, sizeof(wu_name))) continue;
        if (xp.parse_double("received_time", received_time)) continue;
        if (xp.parse_double("report_deadline", report_deadline)) {
            continue;
        }
        if (xp.match_tag("file_ref")) {
            file_ref.parse(xp);
#ifndef SIM
            output_files.push_back(file_ref);
#endif
            continue;
        }
        if (xp.parse_double("final_cpu_time", final_cpu_time)) continue;
        if (xp.parse_double("final_elapsed_time", final_elapsed_time)) continue;
        if (xp.parse_int("exit_status", exit_status)) continue;
        if (xp.parse_bool("got_server_ack", got_server_ack)) continue;
        if (xp.parse_bool("ready_to_report", ready_to_report)) continue;
        if (xp.parse_double("completed_time", completed_time)) continue;
        if (xp.parse_bool("suspended_via_gui", suspended_via_gui)) continue;
        if (xp.parse_bool("report_immediately", report_immediately)) continue;
        if (xp.parse_int("state", _state)) continue;
        if (xp.parse_string("stderr_out", stderr_out)) continue;
        if (xp.parse_double("fpops_per_cpu_sec", fpops_per_cpu_sec)) continue;
        if (xp.parse_double("fpops_cumulative", fpops_cumulative)) continue;
        if (xp.parse_double("intops_per_cpu_sec", intops_per_cpu_sec)) continue;
        if (xp.parse_double("intops_cumulative", intops_cumulative)) continue;
        if (xp.parse_str("platform", platform, sizeof(platform))) continue;
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] RESULT::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int RESULT::write(MIOFILE& out, bool to_server) {
    unsigned int i;
    FILE_INFO* fip;
    int n, retval;

    out.printf(
        "<result>\n"
        "    <name>%s</name>\n"
        "    <final_cpu_time>%f</final_cpu_time>\n"
        "    <final_elapsed_time>%f</final_elapsed_time>\n"
        "    <exit_status>%d</exit_status>\n"
        "    <state>%d</state>\n"
        "    <platform>%s</platform>\n"
        "    <version_num>%d</version_num>\n",
        name,
        final_cpu_time,
        final_elapsed_time,
        exit_status,
        state(),
        platform,
        version_num
    );
    if (strlen(plan_class)) {
        out.printf("    <plan_class>%s</plan_class>\n", plan_class);
    }
    if (fpops_per_cpu_sec) {
        out.printf("    <fpops_per_cpu_sec>%f</fpops_per_cpu_sec>\n", fpops_per_cpu_sec);
    }
    if (fpops_cumulative) {
        out.printf("    <fpops_cumulative>%f</fpops_cumulative>\n", fpops_cumulative);
    }
    if (intops_per_cpu_sec) {
        out.printf("    <intops_per_cpu_sec>%f</intops_per_cpu_sec>\n", intops_per_cpu_sec);
    }
    if (intops_cumulative) {
        out.printf("    <intops_cumulative>%f</intops_cumulative>\n", intops_cumulative);
    }
    if (to_server) {
        out.printf(
            "    <app_version_num>%d</app_version_num>\n",
            wup->version_num
        );
    }
    n = (int)stderr_out.length();
    if (n || to_server) {
        out.printf("<stderr_out>\n");

        // the following is here so that it gets recorded on server
        // (there's no core_client_version field of result table)
        //
        if (to_server) {
            out.printf(
                "<core_client_version>%d.%d.%d</core_client_version>\n",
                gstate.core_client_version.major,
                gstate.core_client_version.minor,
                gstate.core_client_version.release
            );
        }
        if (n) {
            out.printf("<![CDATA[\n");
            out.printf("%s",stderr_out.c_str());
            if (stderr_out[n-1] != '\n') {
                out.printf("\n");
            }
            out.printf("]]>\n");
        }
        out.printf("</stderr_out>\n");
    }
    if (to_server) {
        for (i=0; i<output_files.size(); i++) {
            fip = output_files[i].file_info;
            if (fip->uploaded) {
                retval = fip->write(out, true);
                if (retval) return retval;
            }
        }
    } else {
        if (got_server_ack) out.printf("    <got_server_ack/>\n");
        if (ready_to_report) out.printf("    <ready_to_report/>\n");
        if (completed_time) out.printf("    <completed_time>%f</completed_time>\n", completed_time);
        if (suspended_via_gui) out.printf("    <suspended_via_gui/>\n");
        if (report_immediately) out.printf("    <report_immediately/>\n");
        out.printf(
            "    <wu_name>%s</wu_name>\n"
            "    <report_deadline>%f</report_deadline>\n"
            "    <received_time>%f</received_time>\n",
            wu_name,
            report_deadline,
            received_time
        );
        for (i=0; i<output_files.size(); i++) {
            retval = output_files[i].write(out);
            if (retval) return retval;
        }
    }
    out.printf("</result>\n");
    return 0;
}

#ifndef SIM

int RESULT::write_gui(MIOFILE& out) {
    out.printf(
        "<result>\n"
        "    <name>%s</name>\n"
        "    <wu_name>%s</wu_name>\n"
        "    <version_num>%d</version_num>\n"
        "    <plan_class>%s</plan_class>\n"
        "    <project_url>%s</project_url>\n"
        "    <final_cpu_time>%f</final_cpu_time>\n"
        "    <final_elapsed_time>%f</final_elapsed_time>\n"
        "    <exit_status>%d</exit_status>\n"
        "    <state>%d</state>\n"
        "    <report_deadline>%f</report_deadline>\n"
        "    <received_time>%f</received_time>\n"
        "    <estimated_cpu_time_remaining>%f</estimated_cpu_time_remaining>\n",
        name,
        wu_name,
        version_num,
        plan_class,
        project->master_url,
        final_cpu_time,
        final_elapsed_time,
        exit_status,
        state(),
        report_deadline,
        received_time,
        estimated_time_remaining()
    );
    if (got_server_ack) out.printf("    <got_server_ack/>\n");
    if (ready_to_report) out.printf("    <ready_to_report/>\n");
    if (completed_time) out.printf("    <completed_time>%f</completed_time>\n", completed_time);
    if (suspended_via_gui) out.printf("    <suspended_via_gui/>\n");
    if (project->suspended_via_gui) out.printf("    <project_suspended_via_gui/>\n");
    if (report_immediately) out.printf("    <report_immediately/>\n");
    if (edf_scheduled) out.printf("    <edf_scheduled/>\n");
    if (coproc_missing) out.printf("    <coproc_missing/>\n");
    if (schedule_backoff > gstate.now) {
        out.printf("    <scheduler_wait/>\n");
        if (strlen(schedule_backoff_reason)) {
            out.printf(
                "    <scheduler_wait_reason>%s</scheduler_wait_reason>\n",
                schedule_backoff_reason
            );
        }
    }
    if (avp->needs_network && gstate.network_suspended) out.printf("    <network_wait/>\n");
    ACTIVE_TASK* atp = gstate.active_tasks.lookup_result(this);
    if (atp) {
        atp->write_gui(out);
    }
    if (!strlen(resources)) {
        // only need to compute this string once
        //
        if (avp->gpu_usage.rsc_type) {
            if (avp->gpu_usage.usage == 1) {
                sprintf(resources,
                    "%.3g CPUs + 1 %s GPU",
                    avp->avg_ncpus,
                    rsc_name(avp->gpu_usage.rsc_type)
                );
            } else {
                sprintf(resources,
                    "%.3g CPUs + %.3g %s GPUs",
                    avp->avg_ncpus,
                    avp->gpu_usage.usage,
                    rsc_name(avp->gpu_usage.rsc_type)
                );
            }
        } else if (avp->missing_coproc) {
            sprintf(resources, "%.3g CPUs + %s GPU (missing)",
                avp->avg_ncpus, avp->missing_coproc_name
            );
        } else if (!project->non_cpu_intensive && (avp->avg_ncpus != 1)) {
            sprintf(resources, "%.3g CPUs", avp->avg_ncpus);
        } else {
            strcpy(resources, " ");
        }
    }
    if (strlen(resources)>1) {
        char buf[256];
        strcpy(buf, "");
        if (atp && atp->task_state() == PROCESS_EXECUTING) {
            if (avp->gpu_usage.rsc_type) {
                COPROC& cp = coprocs.coprocs[avp->gpu_usage.rsc_type];
                if (cp.count > 1) {
                    sprintf(buf, " (device %d)",
                        cp.device_nums[coproc_indices[0]]
                    );
                }
            }
        }
        out.printf(
            "    <resources>%s%s</resources>\n", resources, buf
        );
    }
    out.printf("</result>\n");
    return 0;
}

#endif

// Returns true if the result's output files are all either
// successfully uploaded or have unrecoverable errors
//
bool RESULT::is_upload_done() {
    unsigned int i;
    FILE_INFO* fip;
    int retval;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        if (fip->uploadable()) {
            if (fip->had_failure(retval)) continue;
            if (!fip->uploaded) {
                return false;
            }
        }
    }
    return true;
}

// resets all FILE_INFO's in result to uploaded = false
//
void RESULT::clear_uploaded_flags() {
    unsigned int i;
    FILE_INFO* fip;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        fip->uploaded = false;
    }
}

bool RESULT::is_not_started() {
    if (computing_done()) return false;
    if (gstate.active_tasks.lookup_result(this)) return false;
    return true;
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

// return true if some file needed by this result (input or application)
// is downloading and backed off
//
bool RESULT::some_download_stalled() {
#ifndef SIM
    unsigned int i;
    FILE_INFO* fip;
    PERS_FILE_XFER* pfx;
    bool some_file_missing = false;

    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        if (fip->status != FILE_PRESENT) some_file_missing = true;
        pfx = fip->pers_file_xfer;
        if (pfx && pfx->next_request_time > gstate.now) {
            return true;
        }
    }
    for (i=0; i<avp->app_files.size(); i++) {
        fip = avp->app_files[i].file_info;
        if (fip->status != FILE_PRESENT) some_file_missing = true;
        pfx = fip->pers_file_xfer;
        if (pfx && pfx->next_request_time > gstate.now) {
            return true;
        }
    }

    if (some_file_missing && !project->download_backoff.ok_to_transfer()) {
        return true;
    }
#endif
    return false;
}

FILE_REF* RESULT::lookup_file(FILE_INFO* fip) {
    for (unsigned int i=0; i<output_files.size(); i++) {
        FILE_REF& fr = output_files[i];
        if (fr.file_info == fip) return &fr;
    }
    return 0;
}

FILE_INFO* RESULT::lookup_file_logical(const char* lname) {
    for (unsigned int i=0; i<output_files.size(); i++) {
        FILE_REF& fr = output_files[i];
        if (!strcmp(lname, fr.open_name)) {
            return fr.file_info;
        }
    }
    return 0;
}

void RESULT::append_log_record() {
    char filename[256];
    job_log_filename(*project, filename, sizeof(filename));
    FILE* f = fopen(filename, "ab");
    if (!f) return;
    fprintf(f, "%.0f ue %f ct %f fe %.0f nm %s et %f\n",
        gstate.now, estimated_duration_uncorrected(), final_cpu_time,
        wup->rsc_fpops_est, name, final_elapsed_time
    );
    fclose(f);
}

// abort a result that's not currently running
//
void RESULT::abort_inactive(int status) {
    if (state() >= RESULT_COMPUTE_ERROR) return;
    set_state(RESULT_ABORTED, "RESULT::abort_inactive");
    exit_status = status;
}

RUN_MODE::RUN_MODE() {
    perm_mode = 0;
    temp_mode = 0;
    prev_mode = 0;
    temp_timeout = 0;
}

void RUN_MODE::set(int mode, double duration) {
    if (mode == 0) mode = RUN_MODE_AUTO;
    if (mode == RUN_MODE_RESTORE) {
        temp_timeout = 0;
        if (temp_mode == perm_mode) {
            perm_mode = prev_mode;
        }
        temp_mode = perm_mode;
        return;
    }
    prev_mode = temp_mode;
    if (duration) {
        temp_mode = mode;
        temp_timeout = gstate.now + duration;
    } else {
        temp_timeout = 0;
        temp_mode = mode;
        perm_mode = mode;
        gstate.set_client_state_dirty("Set mode");
    }

    // In case we read older state file with no prev_mode
    if (prev_mode == 0) prev_mode = temp_mode;
}

void RUN_MODE::set_prev(int mode) {
    prev_mode = mode;
}

int RUN_MODE::get_perm() {
    return perm_mode;
}

int RUN_MODE::get_prev() {
    return prev_mode;
}

int RUN_MODE::get_current() {
    if (temp_timeout > gstate.now) {
        return temp_mode;
    } else {
        return perm_mode;
    }
}

double RUN_MODE::delay() {
    if (temp_timeout > gstate.now) {
        return temp_timeout - gstate.now;
    } else {
        return 0;
    }
}

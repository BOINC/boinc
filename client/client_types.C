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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "client_msgs.h"
#include "log_flags.h"
#include "parse.h"
#include "util.h"
#include "client_state.h"
#include "pers_file_xfer.h"

#include "client_types.h"

using std::string;
using std::vector;

PROJECT::PROJECT() {
    init();
}

void PROJECT::init() {
    strcpy(master_url, "");
    strcpy(authenticator, "");
#if 0
    deletion_policy_priority = false;
    deletion_policy_expire = false;
    share_size = 0;
    size = 0;
#endif
    project_specific_prefs = "";
    gui_urls = "";
    resource_share = 100;
    strcpy(host_venue, "");
    scheduler_urls.clear();
    strcpy(project_name, "");
    strcpy(user_name, "");
    strcpy(team_name, "");
    strcpy(email_hash, "");
    strcpy(cross_project_id, "");
    user_total_credit = 0;
    user_expavg_credit = 0;
    user_create_time = 0;
    rpc_seqno = 0;
    hostid = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    host_create_time = 0;
    nrpc_failures = 0;
    master_fetch_failures = 0;
    min_rpc_time = 0;
    min_report_min_rpc_time = 0;
    master_url_fetch_pending = false;
    sched_rpc_pending = false;
    trickle_up_pending = false;
    tentative = false;
    anonymous_platform = false;
    non_cpu_intensive = false;
    short_term_debt = 0;
    long_term_debt = 0;
    send_file_list = false;
    suspended_via_gui = false;
    dont_request_more_work = false;
    attached_via_acct_mgr = false;
    strcpy(code_sign_key, "");
    user_files.clear();
    anticipated_debt = 0;
    wall_cpu_time_this_period = 0;
    next_runnable_result = NULL;
    work_request = 0;
    work_request_urgency = WORK_FETCH_DONT_NEED;
    duration_correction_factor = 1;
}

// parse project fields from client_state.xml
//
int PROJECT::parse_state(MIOFILE& in) {
    char buf[256];
    std::string sched_url;
    string str1, str2;
    int retval;
    double x;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    init();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</project>")) return 0;
        else if (parse_str(buf, "<scheduler_url>", sched_url)) {
            scheduler_urls.push_back(sched_url);
            continue;
        }
        else if (parse_str(buf, "<master_url>", master_url, sizeof(master_url))) continue;
        else if (parse_str(buf, "<project_name>", project_name, sizeof(project_name))) continue;
#if 0
        else if (parse_double(buf, "<share_size>", share_size)) continue;
        else if (parse_double(buf, "<size>", size)) continue;
#endif
        else if (parse_str(buf, "<user_name>", user_name, sizeof(user_name))) continue;
        else if (parse_str(buf, "<team_name>", team_name, sizeof(team_name))) continue;
        else if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) continue;
        else if (parse_str(buf, "<email_hash>", email_hash, sizeof(email_hash))) continue;
        else if (parse_str(buf, "<cross_project_id>", cross_project_id, sizeof(cross_project_id))) continue;
        else if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        else if (parse_double(buf, "<user_create_time>", user_create_time)) {
            validate_time(user_create_time);
            continue;
        }
        else if (parse_int(buf, "<rpc_seqno>", rpc_seqno)) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
        else if (parse_double(buf, "<host_create_time>", host_create_time)) {
            validate_time(user_create_time);
            continue;
        }
        else if (match_tag(buf, "<code_sign_key>")) {
            retval = copy_element_contents(
                in,
                "</code_sign_key>",
                code_sign_key,
                sizeof(code_sign_key)
            );
            if (retval) return retval;
        }
        else if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        else if (parse_int(buf, "<master_fetch_failures>", master_fetch_failures)) continue;
        else if (parse_double(buf, "<min_rpc_time>", min_rpc_time)) {
            validate_time(min_rpc_time);
            continue;
        }
        else if (match_tag(buf, "<master_url_fetch_pending/>")) master_url_fetch_pending = true;
        else if (match_tag(buf, "<sched_rpc_pending/>")) sched_rpc_pending = true;
        else if (match_tag(buf, "<trickle_up_pending/>")) trickle_up_pending = true;
        else if (match_tag(buf, "<send_file_list/>")) send_file_list = true;
        else if (match_tag(buf, "<non_cpu_intensive/>")) non_cpu_intensive = true;
        else if (match_tag(buf, "<suspended_via_gui/>")) suspended_via_gui = true;
        else if (match_tag(buf, "<dont_request_more_work/>")) dont_request_more_work = true;
#if 0
        else if (match_tag(buf, "<deletion_policy_priority/>")) deletion_policy_priority = true;
        else if (match_tag(buf, "<deletion_policy_expire/>")) deletion_policy_expire = true;
#endif
        else if (parse_double(buf, "<short_term_debt>", short_term_debt)) continue;
        else if (parse_double(buf, "<long_term_debt>", long_term_debt)) continue;
        else if (parse_double(buf, "<resource_share>", x)) continue;    // not authoritative
        else if (parse_double(buf, "<duration_correction_factor>", duration_correction_factor)) continue;
        else if (match_tag(buf, "<attached_via_acct_mgr/>")) attached_via_acct_mgr = true;
        else scope_messages.printf("PROJECT::parse_state(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// Write project information to client state file or GUI RPC reply
//
int PROJECT::write_state(MIOFILE& out, bool gui_rpc) {
    unsigned int i;
    string u1, u2, t1, t2;

    out.printf(
        "<project>\n"
    );

    u1 = user_name;
    xml_escape(u1, u2);
    t1 = team_name;
    xml_escape(t1, t2);
    out.printf(
        "    <master_url>%s</master_url>\n"
        "    <project_name>%s</project_name>\n"
        "    <user_name>%s</user_name>\n"
        "    <team_name>%s</team_name>\n"
        "    <email_hash>%s</email_hash>\n"
        "    <cross_project_id>%s</cross_project_id>\n"
        "    <user_total_credit>%f</user_total_credit>\n"
        "    <user_expavg_credit>%f</user_expavg_credit>\n"
        "    <user_create_time>%f</user_create_time>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <hostid>%d</hostid>\n"
        "    <host_total_credit>%f</host_total_credit>\n"
        "    <host_expavg_credit>%f</host_expavg_credit>\n"
        "    <host_create_time>%f</host_create_time>\n"
        "    <nrpc_failures>%d</nrpc_failures>\n"
        "    <master_fetch_failures>%d</master_fetch_failures>\n"
        "    <min_rpc_time>%f</min_rpc_time>\n"
        "    <short_term_debt>%f</short_term_debt>\n"
        "    <long_term_debt>%f</long_term_debt>\n"
        "    <resource_share>%f</resource_share>\n"
        "    <duration_correction_factor>%f</duration_correction_factor>\n"
        "%s%s%s%s%s%s%s%s%s",
        master_url,
        project_name,
        u2.c_str(),
        t2.c_str(),
        email_hash,
        cross_project_id,
        user_total_credit,
        user_expavg_credit,
        user_create_time,
        rpc_seqno,
        hostid,
        host_total_credit,
        host_expavg_credit,
        host_create_time,
        nrpc_failures,
        master_fetch_failures,
        min_rpc_time,
        short_term_debt,
        long_term_debt,
        resource_share,
        duration_correction_factor,
        master_url_fetch_pending?"    <master_url_fetch_pending/>\n":"",
        sched_rpc_pending?"    <sched_rpc_pending/>\n":"",
        trickle_up_pending?"    <trickle_up_pending/>\n":"",
        send_file_list?"    <send_file_list/>\n":"",
        non_cpu_intensive?"    <non_cpu_intensive/>\n":"",
        suspended_via_gui?"    <suspended_via_gui/>\n":"",
        dont_request_more_work?"    <dont_request_more_work/>\n":"",
        attached_via_acct_mgr?"    <attached_via_acct_mgr/>\n":"",
        (this == gstate.scheduler_op->cur_proj)?"   <scheduler_rpc_in_progress/>\n":""
    );
    if (gui_rpc) {
        out.printf("%s", gui_urls.c_str());
    } else {
       for (i=0; i<scheduler_urls.size(); i++) {
            out.printf(
                "    <scheduler_url>%s</scheduler_url>\n",
                scheduler_urls[i].c_str()
            );
        }
        if (strlen(code_sign_key)) {
            out.printf(
                "    <code_sign_key>\n%s</code_sign_key>\n", code_sign_key
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
#if 0
    share_size = p.share_size;
    size = p.size;
#endif
    safe_strcpy(user_name, p.user_name);
    safe_strcpy(team_name, p.team_name);
    safe_strcpy(email_hash, p.email_hash);
    safe_strcpy(cross_project_id, p.cross_project_id);
    user_total_credit = p.user_total_credit;
    user_expavg_credit = p.user_expavg_credit;
    user_create_time = p.user_create_time;
    rpc_seqno = p.rpc_seqno;
    hostid = p.hostid;
    host_total_credit = p.host_total_credit;
    host_expavg_credit = p.host_expavg_credit;
    host_create_time = p.host_create_time;
    nrpc_failures = p.nrpc_failures;
    master_fetch_failures = p.master_fetch_failures;
    min_rpc_time = p.min_rpc_time;
    master_url_fetch_pending = p.master_url_fetch_pending;
    sched_rpc_pending = p.sched_rpc_pending;
    trickle_up_pending = p.trickle_up_pending;
    safe_strcpy(code_sign_key, p.code_sign_key);
    short_term_debt = p.short_term_debt;
    long_term_debt = p.long_term_debt;
    send_file_list = p.send_file_list;
    non_cpu_intensive = p.non_cpu_intensive;
    suspended_via_gui = p.suspended_via_gui;
    dont_request_more_work = p.dont_request_more_work;
    attached_via_acct_mgr = p.attached_via_acct_mgr;
#if 0
    deletion_policy_priority = p.deletion_policy_priority;
    deletion_policy_expire = p.deletion_policy_expire;
#endif
    duration_correction_factor = p.duration_correction_factor;
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

char* PROJECT::get_project_name() {
    if (strlen(project_name)) {
        return project_name;
    } else {
        return master_url;
    }
}

const char* PROJECT::get_scheduler_url(int index, double r) {
    int n = (int) scheduler_urls.size();
    int ir = (int)(r*n);
    int i = (index + ir)%n;
    return scheduler_urls[i].c_str();
}

#if 0
// comment?  what does this do?
// Does it do a lot of disk access to do it??
//
bool PROJECT::associate_file(FILE_INFO* fip) {
    return 0;
    double space_made = 0;
    if (gstate.get_more_disk_space(this, fip->nbytes)) {
        size += fip->nbytes;
        return true;
    }
    gstate.calc_proj_size(this);
    gstate.anything_free(space_made);
    space_made += gstate.select_delete(this, fip->nbytes - space_made, P_HIGH);
    if (space_made > fip->nbytes) {
        size += fip->nbytes;
        return true;
    } else {
        return false;
    }
}
#endif

bool PROJECT::runnable() {
    if (non_cpu_intensive) return false;
    if (suspended_via_gui) return false;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rp->runnable()) return true;
    }
    return false;
}

bool PROJECT::downloading() {
    if (non_cpu_intensive) return false;
    if (suspended_via_gui) return false;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rp->state == RESULT_FILES_DOWNLOADING) return true;
    }
    return false;
}


bool PROJECT::contactable() {
    if (suspended_via_gui) return false;
    if (master_url_fetch_pending) return false;
    if (min_rpc_time > gstate.now) return false;
    if (dont_request_more_work) return false;
    return true;
}

bool PROJECT::potentially_runnable() {
    if (runnable()) return true;
    if (contactable()) return true;
    if (downloading()) return true;
    return false;
}

bool PROJECT::debt_adjust_allowed() {
    if (non_cpu_intensive) return false;
    if (suspended_via_gui) return false;
    if (dont_request_more_work && !runnable()) return false;
    return true;
}

double PROJECT::next_file_xfer_time(const bool is_upload) {
    return (is_upload ? next_file_xfer_up : next_file_xfer_down);
}

void PROJECT::file_xfer_failed(const bool is_upload) {
    if (is_upload) {
        file_xfer_failures_up++;
        if (file_xfer_failures_up < FILE_XFER_FAILURE_LIMIT) {
            next_file_xfer_up = 0;
        } else {
            next_file_xfer_up = gstate.now + calculate_exponential_backoff(
                file_xfer_failures_up,
                gstate.pers_retry_delay_min,
                gstate.pers_retry_delay_max
            );
        }
    } else {
        file_xfer_failures_down++;
        if (file_xfer_failures_down < FILE_XFER_FAILURE_LIMIT) {
            next_file_xfer_down = 0;
        } else {
            next_file_xfer_down = gstate.now + calculate_exponential_backoff(
                file_xfer_failures_down,
                gstate.pers_retry_delay_min,
                gstate.pers_retry_delay_max
            );
        }
    }
}

void PROJECT::file_xfer_succeeded(const bool is_upload) {
    if (is_upload) {
        file_xfer_failures_up = 0;
        next_file_xfer_up  = 0;
    } else {
        file_xfer_failures_down = 0;
        next_file_xfer_down = 0;
    }
}

int APP::parse(MIOFILE& in) {
    char buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(name, "");
    project = NULL;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app>")) return 0;
        else if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        else scope_messages.printf("APP::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int APP::write(MIOFILE& out) {
    out.printf(
        "<app>\n"
        "    <name>%s</name>\n"
        "</app>\n",
        name
    );
    return 0;
}

FILE_INFO::FILE_INFO() {
    strcpy(name, "");
    strcpy(md5_cksum, "");
    max_nbytes = 0;
    nbytes = 0;
    upload_offset = -1;
    generated_locally = false;
    status = FILE_NOT_PRESENT;
    executable = false;
    uploaded = false;
    upload_when_present = false;
    sticky = false;
    marked_for_delete = false;
    report_on_rpc = false;
    signature_required = false;
    is_user_file = false;
    pers_file_xfer = NULL;
    result = NULL;
    project = NULL;
    urls.clear();
    start_url = -1;
    current_url = -1;
    strcpy(signed_xml, "");
    strcpy(xml_signature, "");
    strcpy(file_signature, "");
#if 0
    priority = P_LOW;
    time_last_used = gstate.now;
    exp_date = gstate.now + 60*SECONDS_PER_DAY;
#endif
}

FILE_INFO::~FILE_INFO() {
    if (pers_file_xfer) {
        msg_printf(NULL, MSG_ERROR,
            "Deleting file %s while in use",
            name
        );
        pers_file_xfer->fip = NULL;
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
    get_pathname(this, pathname);

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
    return retval;
#endif
}

// If from server, make an exact copy of everything
// except the start/end tags and the <xml_signature> element.
//
int FILE_INFO::parse(MIOFILE& in, bool from_server) {
    char buf[256], buf2[1024];
    std::string url;
    PERS_FILE_XFER *pfxp;
    int retval;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</file_info>")) return 0;
        else if (match_tag(buf, "<xml_signature>")) {
            retval = copy_element_contents(
                in,
                "</xml_signature>",
                xml_signature,
                sizeof(xml_signature)
            );
            if (retval) return retval;
            continue;
        }
        else if (match_tag(buf, "<file_signature>")) {
            retval = copy_element_contents(
                in,
                "</file_signature>",
                file_signature,
                sizeof(file_signature)
            );
            if (retval) return retval;
            if (from_server) {
                strcat(signed_xml, "<file_signature>\n");
                strcat(signed_xml, file_signature);
                strcat(signed_xml, "</file_signature>\n");
            }
            continue;
        }
        strcat(signed_xml, buf);
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        else if (parse_str(buf, "<url>", url)) {
            urls.push_back(url);
            continue;
        }
        else if (parse_str(buf, "<md5_cksum>", md5_cksum, sizeof(md5_cksum))) continue;
        else if (parse_double(buf, "<nbytes>", nbytes)) continue;
        else if (parse_double(buf, "<max_nbytes>", max_nbytes)) continue;
        else if (match_tag(buf, "<generated_locally/>")) generated_locally = true;
        else if (parse_int(buf, "<status>", status)) continue;
        else if (match_tag(buf, "<executable/>")) executable = true;
        else if (match_tag(buf, "<uploaded/>")) uploaded = true;
        else if (match_tag(buf, "<upload_when_present/>")) upload_when_present = true;
        else if (match_tag(buf, "<sticky/>")) sticky = true;
        else if (match_tag(buf, "<marked_for_delete/>")) marked_for_delete = true;
        else if (match_tag(buf, "<report_on_rpc/>")) report_on_rpc = true;
        else if (match_tag(buf, "<signature_required/>")) signature_required = true;
#if 0
        else if (parse_int(buf, "<time_last_used>", (int&)time_last_used)) continue;
        else if (parse_int(buf, "<priority>", priority)) continue;
        else if (parse_double(buf, "<exp_date>", exp_date)) continue;
        else if (parse_double(buf, "<exp_days>", exp_days)) {
            exp_date = gstate.now + exp_days*SECONDS_PER_DAY;
        }
#endif
        else if (match_tag(buf, "<persistent_file_xfer>")) {
            pfxp = new PERS_FILE_XFER;
            retval = pfxp->parse(in);
            if (!retval) {
                pers_file_xfer = pfxp;
            } else {
                delete pfxp;
            }
        } else if (!from_server && match_tag(buf, "<signed_xml>")) {
            retval = copy_element_contents(
                in,
                "</signed_xml>",
                signed_xml,
                sizeof(signed_xml)
            );
            if (retval) return retval;
            continue;
        } else if (match_tag(buf, "<file_xfer>")) {
            while (in.fgets(buf, 256)) {
                if (match_tag(buf, "</file_xfer>")) break;
            }
            continue;
        } else if (match_tag(buf, "<error_msg>")) {
            retval = copy_element_contents(
                in, "</error_msg>", buf2, sizeof(buf2)
            );
            if (retval) return retval;
            error_msg = buf2;
        } else scope_messages.printf("FILE_INFO::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int FILE_INFO::write(MIOFILE& out, bool to_server) {
    unsigned int i;
    int retval;

    out.printf(
        "<file_info>\n"
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
        if (generated_locally) out.printf("    <generated_locally/>\n");
        out.printf("    <status>%d</status>\n", status);
        if (executable) out.printf("    <executable/>\n");
        if (uploaded) out.printf("    <uploaded/>\n");
        if (upload_when_present) out.printf("    <upload_when_present/>\n");
        if (sticky) out.printf("    <sticky/>\n");
        if (marked_for_delete) out.printf("    <marked_for_delete/>\n");
        if (report_on_rpc) out.printf("    <report_on_rpc/>\n");
        if (signature_required) out.printf("    <signature_required/>\n");
        if (strlen(file_signature)) out.printf("    <file_signature>\n%s</file_signature>\n", file_signature);
#if 0
        if (time_last_used) out.printf("    <time_last_used>%d</time_last_used>\n", time_last_used);
        if (priority) out.printf("    <priority>%d</priority>\n", priority);
        if (exp_date) out.printf("    <exp_date>%ld</exp_date>\n", exp_date);
#endif
    }
    for (i=0; i<urls.size(); i++) {
        out.printf("    <url>%s</url>\n", urls[i].c_str());
    }
    if (!to_server && pers_file_xfer) {
        retval = pers_file_xfer->write(out);
        if (retval) return retval;
    }
    if (!to_server) {
        if (strlen(signed_xml) && strlen(xml_signature)) {
            out.printf(
                "    <signed_xml>\n%s    </signed_xml>\n"
                "    <xml_signature>\n%s    </xml_signature>\n",
                signed_xml, xml_signature
            );
        }
    }
    if (!error_msg.empty()) {
        out.printf("    <error_msg>\n%s</error_msg>\n", error_msg.c_str());
    }
    out.printf("</file_info>\n");
#if 0
    if (to_server)
        update_time();      // huh??
#endif
    return 0;
}

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
        nbytes,
        max_nbytes,
        status
    );
    if (generated_locally) out.printf("    <generated_locally/>\n");
    if (uploaded) out.printf("    <uploaded/>\n");
    if (upload_when_present) out.printf("    <upload_when_present/>\n");
    if (sticky) out.printf("    <sticky/>\n");
    if (marked_for_delete) out.printf("    <marked_for_delete/>\n");

    if (pers_file_xfer) {
        pers_file_xfer->write(out);
    }
    out.printf("</file_transfer>\n");
    return 0;
}

// delete physical underlying file associated with FILE_INFO
//
int FILE_INFO::delete_file() {
    char path[256];

    get_pathname(this, path);
    int retval = boinc_delete_file(path);
    if (retval && status != FILE_NOT_PRESENT) {
        msg_printf(project, MSG_ERROR, "Couldn't delete file %s", path);
    }
    status = FILE_NOT_PRESENT;
    return retval;
}

// Files may have URLs for both upload and download.
// Call this to get the initial url,
// The is_upload arg says which kind you want.
// NULL return means there is no URL of the requested type
//
const char* FILE_INFO::get_init_url(bool is_upload) {

// if a project supplies multiple URLs, try them in order
// (e.g. in Einstein@home they're ordered by proximity to client).
// The commented-out code tries them starting from random place.
// This is appropriate if replication is for load-balancing.
// TODO: add a flag saying which mode to use.
//
#if 1
    current_url = 0;
#else
    double temp;
    temp = rand();
    temp *= urls.size();
    temp /= RAND_MAX;
    current_url = (int)temp;
#endif
    start_url = current_url;
    while(1) {
        if (!is_correct_url_type(is_upload, urls[current_url])) {
            current_url = (current_url + 1)%urls.size();
            if (current_url == start_url) {
                msg_printf(project, MSG_ERROR,
                    "Couldn't find suitable URL for %s", name);
                return NULL;
            }
        } else {
            start_url = current_url;
            return urls[current_url].c_str();
        }
    }
}

// Call this to get the next URL of the indicated type.
// NULL return means you've tried them all.
//
const char* FILE_INFO::get_next_url(bool is_upload) {
    while(1) {
        current_url = (current_url + 1)%urls.size();
        if (current_url == start_url) {
            return NULL;
        }
        if (is_correct_url_type(is_upload, urls[current_url])) {
            return urls[current_url].c_str();
        }
    }
}

const char* FILE_INFO::get_current_url(bool is_upload) {
    if (current_url < 0) {
        return get_init_url(is_upload);
    }
    return urls[current_url].c_str();
}

// Checks if the URL includes the phrase "file_upload_handler"
// This indicates the URL is an upload url
// 
bool FILE_INFO::is_correct_url_type(bool is_upload, std::string& url) {
    const char* has_str = strstr(url.c_str(), "file_upload_handler");
    if ((is_upload && !has_str) || (!is_upload && has_str)) {
        return false;
    } else {
        return true;
    }
}

// merges information from a new FILE_INFO that has the same name as one
// that is already present in the client state file.
// Potentially changes upload_when_present, max_nbytes, and signed_xml
//
int FILE_INFO::merge_info(FILE_INFO& new_info) {
    char buf[256];
    unsigned int i;

    upload_when_present = new_info.upload_when_present;

#if 0
    if (new_info.priority > priority) {
        priority = new_info.priority;
    }

    if (new_info.exp_date > exp_date) {
        exp_date = new_info.exp_date;
    }
#endif
    if (max_nbytes <= 0 && new_info.max_nbytes) {
        max_nbytes = new_info.max_nbytes;
        sprintf(buf, "    <max_nbytes>%.0f</max_nbytes>\n", new_info.max_nbytes);
        strcat(signed_xml, buf);
    }

    // replace existing URLs with new ones
    //
    urls.clear();
    for (i=0; i<new_info.urls.size(); i++) {
        urls.push_back(new_info.urls[i]);
    }

    // replace signature
    //
    strcpy(file_signature, new_info.file_signature);

    return 0;
}

// Returns true if the file had an unrecoverable error
// (couldn't download, RSA/MD5 check failed, etc)
//
bool FILE_INFO::had_failure(int& failnum, char* buf) {
    if (status != FILE_NOT_PRESENT && status != FILE_PRESENT) {
        failnum = status;
        if (buf) {
            sprintf(buf,
                "<file_xfer_error>\n"
                "  <file_name>%s</file_name>\n"
                "  <error_code>%d</error_code>\n"
                "  <error_message>%s</error_message>\n"
                "</file_xfer_error>\n",
                name,
                status,
                error_msg.c_str()
            );
        }
        return true;
    }
    return false;
}

#if 0
// Sets the time_last_used to be equal to the current time
int FILE_INFO::update_time() {
    time_last_used = gstate.now;
    return 0;
}
#endif

// Parse XML based app_version information, usually from client_state.xml
//
int APP_VERSION::parse(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(app_name, "");
    version_num = 0;
    app = NULL;
    project = NULL;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app_version>")) return 0;
        else if (parse_str(buf, "<app_name>", app_name, sizeof(app_name))) continue;
        else if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            app_files.push_back(file_ref);
            continue;
        }
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else scope_messages.printf("APP_VERSION::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int APP_VERSION::write(MIOFILE& out) {
    unsigned int i;
    int retval;

    out.printf(
        "<app_version>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n",
        app_name,
        version_num
    );
    for (i=0; i<app_files.size(); i++) {
        retval = app_files[i].write(out);
        if (retval) return retval;
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
    char buf[1024];

    str = "couldn't get input files:\n";
    for (i=0; i<app_files.size();i++) {
        fip = app_files[i].file_info;
        if (fip->had_failure(errnum, buf)) {
            str = str + buf;
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


int FILE_REF::parse(MIOFILE& in) {
    char buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(file_name, "");
    strcpy(open_name, "");
    fd = -1;
    main_program = false;
    copy_file = false;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</file_ref>")) return 0;
        else if (parse_str(buf, "<file_name>", file_name, sizeof(file_name))) continue;
        else if (parse_str(buf, "<open_name>", open_name, sizeof(open_name))) continue;
        else if (parse_int(buf, "<fd>", fd)) continue;
        else if (match_tag(buf, "<main_program/>")) main_program = true;
        else if (match_tag(buf, "<copy_file/>")) copy_file = true;
        else scope_messages.printf("FILE_REF::parse(): unrecognized: %s\n", buf);
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
    if (fd >= 0) {
        out.printf("        <fd>%d</fd>\n", fd);
    }
    if (main_program) {
        out.printf("        <main_program/>\n");
    }
    if (copy_file) {
        out.printf("        <copy_file/>\n");
    }
    out.printf("    </file_ref>\n");
    return 0;
}

int WORKUNIT::parse(MIOFILE& in) {
    char buf[4096];
    FILE_REF file_ref;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

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
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</workunit>")) return 0;
        else if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        else if (parse_str(buf, "<app_name>", app_name, sizeof(app_name))) continue;
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else if (match_tag(buf, "<command_line>")) {
            if (strstr(buf, "</command_line>")) {
                parse_str(buf, "<command_line>", command_line);
            } else {
                bool found=false;
                while (in.fgets(buf, sizeof(buf))) {
                    if (strstr(buf, "</command_line")) {
                        found = true;
                        break;
                    }
                    command_line += buf;
                }
                if (!found) {
                    msg_printf(NULL, MSG_ERROR,
                        "Task %s: bad command line",
                        name
                    );
                    return ERR_XML_PARSE;
                }
            }
            strip_whitespace(command_line);
            continue;
        }
        //else if (parse_str(buf, "<env_vars>", env_vars, sizeof(env_vars))) continue;
        else if (parse_double(buf, "<rsc_fpops_est>", rsc_fpops_est)) continue;
        else if (parse_double(buf, "<rsc_fpops_bound>", rsc_fpops_bound)) continue;
        else if (parse_double(buf, "<rsc_memory_bound>", rsc_memory_bound)) continue;
        else if (parse_double(buf, "<rsc_disk_bound>", rsc_disk_bound)) continue;
        else if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            input_files.push_back(file_ref);
            continue;
        }
        else scope_messages.printf("WORKUNIT::parse(): unrecognized: %s\n", buf);
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
    char buf[1024];

    str = "couldn't get input files:\n";
    for (i=0;i<input_files.size();i++) {
        fip = input_files[i].file_info;
        if (fip->had_failure(x, buf)) {
            str = str + buf;
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

int RESULT::parse_ack(FILE* in) {
    char buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(name, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</result_ack>")) return 0;
        else if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        else scope_messages.printf("RESULT::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

void RESULT::clear() {
    strcpy(name, "");
    strcpy(wu_name, "");
    report_deadline = 0;
    output_files.clear();
    state = RESULT_NEW;
    ready_to_report = false;
    completed_time = 0;
    got_server_ack = false;
    final_cpu_time = 0;
    exit_status = 0;
    stderr_out = "";
    suspended_via_gui = false;
    aborted_via_gui = false;
    fpops_per_cpu_sec = 0;
    fpops_cumulative = 0;
    intops_per_cpu_sec = 0;
    intops_cumulative = 0;
    app = NULL;
    wup = NULL;
    project = NULL;
}

// parse a <result> element from scheduling server.
//
int RESULT::parse_server(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</result>")) return 0;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<wu_name>", wu_name, sizeof(wu_name))) continue;
        if (parse_double(buf, "<report_deadline>", report_deadline)) {
            validate_time(report_deadline);
            continue;
        }
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            output_files.push_back(file_ref);
            continue;
        }
        else scope_messages.printf("RESULT::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// parse a <result> element from state file
//
int RESULT::parse_state(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</result>")) {
            // restore some invariants in case of bad state file
            //
            if (got_server_ack || ready_to_report) {
                state = RESULT_FILES_UPLOADED;
            }
            return 0;
        }
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<wu_name>", wu_name, sizeof(wu_name))) continue;
        if (parse_double(buf, "<report_deadline>", report_deadline)) {
            validate_time(report_deadline);
            continue;
        }
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            output_files.push_back(file_ref);
            continue;
        }
        else if (parse_double(buf, "<final_cpu_time>", final_cpu_time)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (match_tag(buf, "<got_server_ack/>")) got_server_ack = true;
        else if (match_tag(buf, "<ready_to_report/>")) ready_to_report = true;
        else if (parse_double(buf, "<completed_time>", completed_time)) continue;
        else if (match_tag(buf, "<suspended_via_gui/>")) suspended_via_gui = true;
        else if (match_tag(buf, "<aborted_via_gui/>")) aborted_via_gui = true;
        else if (parse_int(buf, "<state>", state)) continue;
        else if (match_tag(buf, "<stderr_out>")) {
            while (in.fgets(buf, 256)) {
                if (match_tag(buf, "</stderr_out>")) break;
                stderr_out.append(buf);
            }
            continue;
        }
        else if (parse_double(buf, "<fpops_per_cpu_sec>", fpops_per_cpu_sec)) continue;
        else if (parse_double(buf, "<fpops_cumulative>", fpops_cumulative)) continue;
        else if (parse_double(buf, "<intops_per_cpu_sec>", intops_per_cpu_sec)) continue;
        else if (parse_double(buf, "<intops_cumulative>", intops_cumulative)) continue;
        else scope_messages.printf("RESULT::parse(): unrecognized: %s\n", buf);
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
        "    <exit_status>%d</exit_status>\n"
        "    <state>%d</state>\n",
        name,
        final_cpu_time,
        exit_status,
        state
    );
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
    n = stderr_out.length();
    if (n) {
        out.printf("<stderr_out>\n");
        if (to_server) {
            out.printf(
                "<core_client_version>%d.%d.%d</core_client_version>\n",
                gstate.core_client_major_version,
                gstate.core_client_minor_version,
                gstate.core_client_release
            );
        }
        out.printf(stderr_out.c_str());
        if (stderr_out[n-1] != '\n') {
            out.printf("\n");
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
        if (aborted_via_gui) out.printf("    <aborted_via_gui/>\n");
        out.printf(
            "    <wu_name>%s</wu_name>\n"
            "    <report_deadline>%f</report_deadline>\n",
            wu_name,
            report_deadline
        );
        for (i=0; i<output_files.size(); i++) {
            retval = output_files[i].write(out);
            if (retval) return retval;
        }
    }
    out.printf("</result>\n");
    return 0;
}

int RESULT::write_gui(MIOFILE& out) {
    out.printf(
        "<result>\n"
        "    <name>%s</name>\n"
        "    <wu_name>%s</wu_name>\n"
        "    <project_url>%s</project_url>\n"
        "    <final_cpu_time>%f</final_cpu_time>\n"
        "    <exit_status>%d</exit_status>\n"
        "    <state>%d</state>\n"
        "    <report_deadline>%f</report_deadline>\n"
        "    <estimated_cpu_time_remaining>%f</estimated_cpu_time_remaining>\n",
        name,
        wu_name,
        project->master_url,
        final_cpu_time,
        exit_status,
        state,
        report_deadline,
        estimated_cpu_time_remaining()
    );
    if (got_server_ack) out.printf("    <got_server_ack/>\n");
    if (ready_to_report) out.printf("    <ready_to_report/>\n");
    if (completed_time) out.printf("    <completed_time>%f</completed_time>\n", completed_time);
    if (suspended_via_gui) out.printf("    <suspended_via_gui/>\n");
    if (aborted_via_gui) out.printf("    <aborted_via_gui/>\n");
    ACTIVE_TASK* atp = gstate.active_tasks.lookup_result(this);
    if (atp) {
        atp->write(out);
    }
    out.printf("</result>\n");
    return 0;
}

// Returns true if the result's output files are all either
// successfully uploaded or have unrecoverable errors
//
bool RESULT::is_upload_done() {
    unsigned int i;
    FILE_INFO* fip;
    int retval;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        if (fip->upload_when_present) {
            if (fip->had_failure(retval)) continue;
            if (!fip->uploaded) {
                return false;
            }
        }
    }
    return true;
}

// resets all FILE_INFO's in result to uploaded = false 
// if upload_when_present is true.
// Also updates the last time the input files were used

void RESULT::reset_files() {
    unsigned int i;
    FILE_INFO* fip;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        if (fip->upload_when_present) {
            fip->uploaded = false;
        }
#if 0
        fip->update_time();
#endif
    }
    for (i=0; i < wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
#if 0
        fip->update_time();
#endif
    }
}

bool RESULT::computing_done() {
    return (state >= RESULT_COMPUTE_ERROR || ready_to_report);
}

double RESULT::estimated_cpu_time_uncorrected() {
    return wup->rsc_fpops_est/gstate.host_info.p_fpops;
}

// estimate how long a result will take on this host
//
double RESULT::estimated_cpu_time() {
    return estimated_cpu_time_uncorrected()*project->duration_correction_factor;
}

double RESULT::estimated_cpu_time_remaining() {
    if (computing_done()) return 0;
    ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(this);
    if (atp) {
        return atp->est_cpu_time_to_completion();
    }
    return estimated_cpu_time();
}

// The given result has just completed successfully.
// Update the correction factor used to predict
// completion time for this project's results
//
void PROJECT::update_duration_correction_factor(RESULT* rp) {
    double ratio = rp->final_cpu_time / rp->estimated_cpu_time();

    // it's OK to overestimate completion time,
    // but bad to underestimate it.
    // So make it easy for the factor to increase,
    // but decrease it with caution
    //
    if (ratio > 1) {
        duration_correction_factor *= ratio;
    } else {
        // in particular, don't give much weight to results
        // that completed a lot earlier than expected
        //
        if (ratio < 0.1) {
            duration_correction_factor *= (0.99 + 0.01*ratio);
        } else {
            duration_correction_factor *= (0.9 + 0.1*ratio);
        }
    }
}

bool RESULT::runnable() {
    if (suspended_via_gui) return false;
    if (project->suspended_via_gui) return false;
    if (state != RESULT_FILES_DOWNLOADED) return false;
    return true;
}

bool RESULT::runnable_soon() {
    if (suspended_via_gui) return false;
    if (project->suspended_via_gui) return false;
    if (computing_done()) return false;
    return true;
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

const char *BOINC_RCSID_b81ff9a584 = "$Id$";

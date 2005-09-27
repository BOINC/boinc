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

// This file contains:
// 1) functions to clear and parse the various structs
// 2) actual GUI RPCs

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "version.h"
#else
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#endif

#include "diagnostics.h"
#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "miofile.h"
#include "md5_file.h"
#include "network.h"
#include "gui_rpc_client.h"

using std::string;
using std::vector;

DISPLAY_INFO::DISPLAY_INFO() {
    memset(this, 0, sizeof(DISPLAY_INFO));
}

int GUI_URL::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</gui_url>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<description>", description)) continue;
        else if (parse_str(buf, "<url>", url)) continue;
    }
    return ERR_XML_PARSE;
}

PROJECT::PROJECT() {
    clear();
}

PROJECT::~PROJECT() {
    clear();
}

void PROJECT::get_name(std::string& s) {
    if (project_name.length() == 0) {
        s = master_url;
    } else {
        s = project_name;
    }
}

int PROJECT::parse(MIOFILE& in) {
    char buf[256];
    int retval;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</project>")) return 0;
        else if (parse_str(buf, "<master_url>", master_url)) continue;
        else if (parse_double(buf, "<resource_share>", resource_share)) continue;
        else if (parse_str(buf, "<project_name>", project_name)) continue;
        else if (parse_str(buf, "<user_name>", user_name)) continue;
        else if (parse_str(buf, "<team_name>", team_name)) continue;
        else if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        else if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
        else if (parse_double(buf, "<disk_usage>", disk_usage)) continue;
        else if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        else if (parse_int(buf, "<master_fetch_failures>", master_fetch_failures)) continue;
        else if (parse_double(buf, "<min_rpc_time>", min_rpc_time)) continue;
        else if (match_tag(buf, "<master_url_fetch_pending/>")) {
            master_url_fetch_pending = true;
            continue;
        }
        else if (match_tag(buf, "<sched_rpc_pending/>")) {
            sched_rpc_pending = true;
            continue;
        }
        else if (match_tag(buf, "<non_cpu_intensive/>")) {
            non_cpu_intensive = true;
            continue;
        }
        else if (match_tag(buf, "<suspended_via_gui/>")) {
            suspended_via_gui = true;
            continue;
        }
        else if (match_tag(buf, "<dont_request_more_work/>")) {
            dont_request_more_work = true;
            continue;
        }
        else if (match_tag(buf, "<tentative/>")) {
            tentative = true;
            continue;
        }
        else if (match_tag(buf, "<gui_urls>")) {
            while (in.fgets(buf, 256)) {
                if (match_tag(buf, "</gui_urls>")) break;
                else if (match_tag(buf, "<gui_url>")) {
                    GUI_URL gu;
                    retval = gu.parse(in);
                    if (!retval) {
                        gui_urls.push_back(gu);
                    }
                }
            }
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void PROJECT::clear() {
    master_url.clear();
    resource_share = 0.0;
    project_name.clear();
    user_name.clear();
    team_name.clear();
    user_total_credit = 0.0;
    user_expavg_credit = 0.0;
    host_total_credit = 0.0;
    host_expavg_credit = 0.0;
    disk_usage = 0.0;
    nrpc_failures = 0;
    master_fetch_failures = 0;
    min_rpc_time = 0;
    master_url_fetch_pending = false;
    sched_rpc_pending = false;
    tentative = false;
    non_cpu_intensive = false;
    suspended_via_gui = false;
    dont_request_more_work = false;
    gui_urls.clear();
    statistics.clear();
}

APP::APP() {
    clear();
}

APP::~APP() {
    clear();
}

int APP::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
    }
    return ERR_XML_PARSE;
}

void APP::clear() {
    name.clear();
    project = NULL;
}

APP_VERSION::APP_VERSION() {
    clear();
}

APP_VERSION::~APP_VERSION() {
    clear();
}

int APP_VERSION::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app_version>")) return 0;
        else if (parse_str(buf, "<app_name>", app_name)) continue;
        else if (parse_int(buf, "<version_num>", version_num)) continue;
    }
    return ERR_XML_PARSE;
}

void APP_VERSION::clear() {
    app_name.clear();
    version_num = 0;
    app = NULL;
    project = NULL;
}

WORKUNIT::WORKUNIT() {
    clear();
}

WORKUNIT::~WORKUNIT() {
    clear();
}

int WORKUNIT::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</workunit>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<app_name>", app_name)) continue;
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else if (parse_double(buf, "<rsc_fpops_est>", rsc_fpops_est)) continue;
        else if (parse_double(buf, "<rsc_fpops_bound>", rsc_fpops_bound)) continue;
        else if (parse_double(buf, "<rsc_memory_bound>", rsc_memory_bound)) continue;
        else if (parse_double(buf, "<rsc_disk_bound>", rsc_disk_bound)) continue;
    }
    return ERR_XML_PARSE;
}

void WORKUNIT::clear() {
    name.clear();
    app_name.clear();
    version_num = 0;
    rsc_fpops_est = 0.0;
    rsc_fpops_bound = 0.0;
    rsc_memory_bound = 0.0;
    rsc_disk_bound = 0.0;
    project = NULL;
    app = NULL;
    avp = NULL;
}

RESULT::RESULT() {
    clear();
}

RESULT::~RESULT() {
    clear();
}

int RESULT::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</result>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<wu_name>", wu_name)) continue;
        else if (parse_str(buf, "<project_url>", project_url)) continue;
        else if (parse_int(buf, "<report_deadline>", report_deadline)) continue;
        else if (match_tag(buf, "<ready_to_report/>")) {
            ready_to_report = true;
            continue;
        }
        else if (match_tag(buf, "<got_server_ack/>")) {
            got_server_ack = true;
            continue;
        }
        else if (match_tag(buf, "<suspended_via_gui/>")) {
            suspended_via_gui = true;
            continue;
        }
        else if (match_tag(buf, "<aborted_via_gui/>")) {
            aborted_via_gui = true;
            continue;
        }
        else if (match_tag(buf, "<active_task>")) {
            active_task = true;
            continue;
        }
        else if (match_tag(buf, "<supports_graphics/>")) {
            supports_graphics = true;
            continue;
        }
        else if (parse_double(buf, "<final_cpu_time>", final_cpu_time)) continue;
        else if (parse_int(buf, "<state>", state)) continue;
        else if (parse_int(buf, "<scheduler_state>", scheduler_state)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (parse_int(buf, "<signal>", signal)) continue;
        else if (parse_int(buf, "<active_task_state>", active_task_state)) continue;
        else if (match_tag(buf, "<stderr_out>")) {
            copy_element_contents(in, "</stderr_out>", stderr_out);
        }
        else if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        else if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        else if (parse_double(buf, "<current_cpu_time>", current_cpu_time)) continue;
        else if (parse_double(buf, "<vm_bytes>", vm_bytes)) continue;
        else if (parse_double(buf, "<rss_bytes>", rss_bytes)) continue;
        else if (parse_double(buf, "<fraction_done>", fraction_done)) continue;
        else if (parse_double(buf, "<estimated_cpu_time_remaining>", estimated_cpu_time_remaining)) continue;
    }
    return ERR_XML_PARSE;
}

void RESULT::clear() {
    name.clear();
    wu_name.clear();
    project_url.clear();
    report_deadline = 0;
    ready_to_report = false;
    got_server_ack = false;
    final_cpu_time = 0.0;
    state = 0;
    scheduler_state = 0;
    exit_status = 0;
    signal = 0;
    active_task_state = 0;
    active_task = false;
    stderr_out.clear();
    app_version_num = 0;
    checkpoint_cpu_time = 0.0;
    current_cpu_time = 0.0;
    fraction_done = 0.0;
    estimated_cpu_time_remaining = 0.0;
    suspended_via_gui = false;
    aborted_via_gui = false;
    supports_graphics = false;
}

FILE_TRANSFER::FILE_TRANSFER() {
    clear();
}

FILE_TRANSFER::~FILE_TRANSFER() {
    clear();
}

int FILE_TRANSFER::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</file_transfer>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<project_url>", project_url)) continue;
        else if (parse_str(buf, "<project_name>", project_name)) continue;
        else if (parse_double(buf, "<nbytes>", nbytes)) continue;
        else if (match_tag(buf, "<generated_locally/>")) {
            generated_locally = true;
            continue;
        }
        else if (match_tag(buf, "<uploaded/>")) {
            uploaded = true;
            continue;
        }
        else if (match_tag(buf, "<upload_when_present/>")) {
            upload_when_present = true;
            continue;
        }
        else if (match_tag(buf, "<sticky/>")) {
            sticky = true;
            continue;
        }
        else if (match_tag(buf, "<persistent_file_xfer>")) {
            pers_xfer_active = true;
            continue;
        }
        else if (match_tag(buf, "<file_xfer>")) {
            xfer_active = true;
            continue;
        }
        else if (parse_int(buf, "<num_retries>", num_retries)) continue;
        else if (parse_int(buf, "<first_request_time>", first_request_time)) continue;
        else if (parse_int(buf, "<next_request_time>", next_request_time)) continue;
        else if (parse_int(buf, "<status>", status)) continue;
        else if (parse_double(buf, "<time_so_far>", time_so_far)) continue;
        else if (parse_double(buf, "<bytes_xferred>", bytes_xferred)) continue;
        else if (parse_double(buf, "<file_offset>", file_offset)) continue;
        else if (parse_double(buf, "<xfer_speed>", xfer_speed)) continue;
        else if (parse_str(buf, "<hostname>", hostname)) continue;
    }
    return ERR_XML_PARSE;
}

void FILE_TRANSFER::clear() {
    name.clear();
    project_url.clear();
    project_name.clear();
    nbytes = 0.0;
    generated_locally = false;
    uploaded = false;
    upload_when_present = false;
    sticky = false;
    pers_xfer_active = false;
    xfer_active = false;
    num_retries = 0;
    first_request_time = 0;
    next_request_time = 0;
    status = 0;
    time_so_far = 0.0;
    bytes_xferred = 0.0;
    file_offset = 0.0;
    xfer_speed = 0.0;
    hostname.clear();
    project = NULL;
}

MESSAGE::MESSAGE() {
    clear();
}

MESSAGE::~MESSAGE() {
    clear();
}

int MESSAGE::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</msg>")) return 0;
        else if (parse_str(buf, "<project>", project)) continue;
        else if (match_tag(buf, "<body>" )) {
            copy_element_contents(in, "</body>", body);
            continue;
        }
        else if (parse_int(buf, "<pri>", priority)) continue;
        else if (parse_int(buf, "<time>", timestamp)) continue;
        else if (parse_int(buf, "<seqno>", seqno)) continue;
    }
    return ERR_XML_PARSE;
}

void MESSAGE::clear() {
    project.clear();
    priority = 0;
    timestamp = 0;
    body.clear();
}

GR_PROXY_INFO::GR_PROXY_INFO() {
    clear();
}

GR_PROXY_INFO::~GR_PROXY_INFO() {
    clear();
}

int GR_PROXY_INFO::parse(MIOFILE& in) {
    char buf[4096];
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_authentication = false;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</proxy_info>")) return 0;
        else if (parse_int(buf, "<socks_version>", socks_version)) continue;
        else if (parse_str(buf, "<socks_server_name>", socks_server_name)) continue;
        else if (parse_int(buf, "<socks_server_port>", socks_server_port)) continue;
        else if (parse_str(buf, "<socks5_user_name>", socks5_user_name)) continue;
        else if (parse_str(buf, "<socks5_user_passwd>", socks5_user_passwd)) continue;
        else if (parse_str(buf, "<http_server_name>", http_server_name)) continue;
        else if (parse_int(buf, "<http_server_port>", http_server_port)) continue;
        else if (parse_str(buf, "<http_user_name>", http_user_name)) continue;
        else if (parse_str(buf, "<http_user_passwd>", http_user_passwd)) continue;
        else if (match_tag(buf, "<use_http_proxy/>")) {
            use_http_proxy = true;
            continue;
        }
        else if (match_tag(buf, "<use_socks_proxy/>")) {
            use_socks_proxy = true;
            continue;
        }
        else if (match_tag(buf, "<use_http_auth/>")) {
            use_http_authentication = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void GR_PROXY_INFO::clear() {
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_authentication = false;
    socks_version = 0;
    socks_server_name.clear();
    http_server_name.clear();
    socks_server_port = 0;
    http_server_port = 0;
    http_user_name.clear();
    http_user_passwd.clear();
    socks5_user_name.clear();
    socks5_user_passwd.clear();
}

#if 0
HOST_INFO::HOST_INFO() {
    clear();
}

HOST_INFO::~HOST_INFO() {
    clear();
}

int HOST_INFO::parse(MIOFILE& in) {
    char buf[256];

    memset(this, 0, sizeof(HOST_INFO));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</host_info>")) return 0;
        else if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name, sizeof(domain_name))) continue;
        else if (parse_str(buf, "<ip_addr>", ip_addr, sizeof(ip_addr))) continue;
        else if (parse_int(buf, "<p_ncpus>", p_ncpus)) continue;
        else if (parse_str(buf, "<p_vendor>", p_vendor, sizeof(p_vendor))) continue;
        else if (parse_str(buf, "<p_model>", p_model, sizeof(p_model))) continue;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) {
            // fix foolishness that could result in negative value here
            //
            if (p_fpops < 0) p_fpops = -p_fpops;
            continue;
        }
        else if (parse_double(buf, "<p_iops>", p_iops)) {
            if (p_iops < 0) p_iops = -p_iops;
            continue;
        }
        else if (parse_double(buf, "<p_membw>", p_membw)) {
            if (p_membw < 0) p_membw = -p_membw;
            continue;
        }
        else if (parse_int(buf, "<p_fpop_err>", p_fpop_err)) continue;
        else if (parse_int(buf, "<p_iop_err>", p_iop_err)) continue;
        else if (parse_int(buf, "<p_membw_err>", p_membw_err)) continue;
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_str(buf, "<os_name>", os_name, sizeof(os_name))) continue;
        else if (parse_str(buf, "<os_version>", os_version, sizeof(os_version))) continue;
        else if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else if (parse_double(buf, "<m_swap>", m_swap)) continue;
        else if (parse_double(buf, "<d_total>", d_total)) continue;
        else if (parse_double(buf, "<d_free>", d_free)) continue;
    }
    return 0;
}

void HOST_INFO::clear() {
    timezone = 0;
    strcpy(domain_name, "");
    strcpy(serialnum, "");
    strcpy(ip_addr, "");
    p_ncpus = 0;
    strcpy(p_vendor, "");
    strcpy(p_model, "");
    p_fpops = 0.0;
    p_iops = 0.0;
    p_membw = 0.0;
    p_fpop_err = 0;
    p_iop_err = 0;
    p_membw_err = 0;
    p_calculated = 0.0;
    strcpy(os_name, "");
    strcpy(os_version, "");
    m_nbytes = 0.0;
    m_cache = 0.0;
    m_swap = 0.0;
    d_total = 0.0;
    d_free = 0.0;
}

#endif

CC_STATE::CC_STATE() {
    clear();
}

CC_STATE::~CC_STATE() {
    clear();
}

void CC_STATE::clear() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        delete projects[i];
    }
    projects.clear();
    for (i=0; i<apps.size(); i++) {
        delete apps[i];
    }
    apps.clear();
    for (i=0; i<app_versions.size(); i++) {
        delete app_versions[i];
    }
    app_versions.clear();
    for (i=0; i<wus.size(); i++) {
        delete wus[i];
    }
    wus.clear();
    for (i=0; i<results.size(); i++) {
        delete results[i];
    }
    results.clear();
}

PROJECT* CC_STATE::lookup_project(string& str) {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        if (projects[i]->master_url == str) return projects[i];
    }
    BOINCTRACE("CAN'T FIND PROJECT %s\n", str.c_str());
    return 0;
}

APP* CC_STATE::lookup_app(string& project_url, string& str) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        if (apps[i]->project->master_url != project_url) continue;
        if (apps[i]->name == str) return apps[i];
    }
    BOINCTRACE("CAN'T FIND APP %s\n", str.c_str());
    return 0;
}

APP* CC_STATE::lookup_app(PROJECT* project, string& str) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        if (apps[i]->project != project) continue;
        if (apps[i]->name == str) return apps[i];
    }
    BOINCTRACE("CAN'T FIND APP %s\n", str.c_str());
    return 0;
}

APP_VERSION* CC_STATE::lookup_app_version(
    string& project_url, string& str, int version_num
) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (app_versions[i]->project->master_url != project_url) continue;
        if (app_versions[i]->app_name == str && app_versions[i]->version_num == version_num) {
            return app_versions[i];
        }
    }
    return 0;
}

APP_VERSION* CC_STATE::lookup_app_version(
    PROJECT* project, string& str, int version_num
) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (app_versions[i]->project != project) continue;
        if (app_versions[i]->app_name == str && app_versions[i]->version_num == version_num) {
            return app_versions[i];
        }
    }
    return 0;
}

WORKUNIT* CC_STATE::lookup_wu(string& project_url, string& str) {
    unsigned int i;
    for (i=0; i<wus.size(); i++) {
        if (wus[i]->project->master_url != project_url) continue;
        if (wus[i]->name == str) return wus[i];
    }
    BOINCTRACE("CAN'T FIND WU %s\n", str.c_str());
    return 0;
}

WORKUNIT* CC_STATE::lookup_wu(PROJECT* project, string& str) {
    unsigned int i;
    for (i=0; i<wus.size(); i++) {
        if (wus[i]->project != project) continue;
        if (wus[i]->name == str) return wus[i];
    }
    BOINCTRACE("CAN'T FIND WU %s\n", str.c_str());
    return 0;
}

RESULT* CC_STATE::lookup_result(string& project_url, string& str) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (results[i]->project->master_url != project_url) continue;
        if (results[i]->name == str) return results[i];
    }
    BOINCTRACE("CAN'T FIND RESULT %s\n", str.c_str());
    return 0;
}

RESULT* CC_STATE::lookup_result(PROJECT* project, string& str) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (results[i]->project != project) continue;
        if (results[i]->name == str) return results[i];
    }
    BOINCTRACE("CAN'T FIND RESULT %s\n", str.c_str());
    return 0;
}

PROJECTS::PROJECTS() {
    clear();
}

PROJECTS::~PROJECTS() {
    clear();
}

void PROJECTS::clear() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        delete projects[i];
    }
    projects.clear();
}

RESULTS::RESULTS() {
    clear();
}

RESULTS::~RESULTS() {
    clear();
}

void RESULTS::clear() {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        delete results[i];
    }
    results.clear();
}

FILE_TRANSFERS::FILE_TRANSFERS() {
    clear();
}

FILE_TRANSFERS::~FILE_TRANSFERS() {
    clear();
}

void FILE_TRANSFERS::clear() {
    unsigned int i;
    for (i=0; i<file_transfers.size(); i++) {
        delete file_transfers[i];
    }
    file_transfers.clear();
}

MESSAGES::MESSAGES() {
    clear();
}

MESSAGES::~MESSAGES() {
    clear();
}

void MESSAGES::clear() {
    unsigned int i;
    for (i=0; i<messages.size(); i++) {
        delete messages[i];
    }
    messages.clear();
}

ACCT_MGR_INFO::ACCT_MGR_INFO() {
    clear();
}

ACCT_MGR_INFO::~ACCT_MGR_INFO() {
    clear();
}

int ACCT_MGR_INFO::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</acct_mgr_info>")) return 0;
        else if (parse_str(buf, "<acct_mgr_name>", acct_mgr_name)) continue;
        else if (parse_str(buf, "<acct_mgr_url>", acct_mgr_url)) continue;
        else if (match_tag(buf, "<cached_credentials/>")) {
            cached_credentials = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void ACCT_MGR_INFO::clear() {
    acct_mgr_name = "";
    acct_mgr_url = "";
    login_name = "";
    password = "";
    cached_credentials = false;
}

ACCT_MGR_RPC_REPLY::ACCT_MGR_RPC_REPLY() {
    clear();
}

ACCT_MGR_RPC_REPLY::~ACCT_MGR_RPC_REPLY() {
    clear();
}

int ACCT_MGR_RPC_REPLY::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</acct_mgr_rpc_reply>")) return 0;
        else if (parse_int(buf, "<error_num>", error_num)) return error_num;
    }
    return ERR_XML_PARSE;
}

void ACCT_MGR_RPC_REPLY::clear() {
    error_num = ERR_XML_PARSE;
}

PROJECT_INIT_STATUS::PROJECT_INIT_STATUS() {
    clear();
}

PROJECT_INIT_STATUS::~PROJECT_INIT_STATUS() {
    clear();
}

int PROJECT_INIT_STATUS::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</get_project_init_status>")) return 0;
        else if (parse_str(buf, "<url>", url)) continue;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (match_tag(buf, "<has_account_key/>")) {
            has_account_key = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void PROJECT_INIT_STATUS::clear() {
    url.clear();
    name.clear();
    has_account_key = false;
}

PROJECT_CONFIG::PROJECT_CONFIG() {
    clear();
}

PROJECT_CONFIG::~PROJECT_CONFIG() {
    clear();
}

int PROJECT_CONFIG::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</project_config>")) return 0;
        else if (parse_int(buf, "<error_num>", error_num)) return error_num;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_int(buf, "<min_passwd_length>", min_passwd_length)) continue;
        else if (match_tag(buf, "<account_manager/>")) {
            account_manager = true;
            continue;
        } else if (match_tag(buf, "<uses_username/>")) {
            uses_username = true;
            continue;
        } else if (match_tag(buf, "<account_creation_disabled")) {
            account_creation_disabled = true;
            continue;
        } else if (match_tag(buf, "<client_account_creation_disabled")) {
            client_account_creation_disabled = true;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void PROJECT_CONFIG::clear() {
    error_num = -1;
    name.clear();
    min_passwd_length = 6;
    account_manager = false;
    uses_username = false;
    account_creation_disabled = false;
    client_account_creation_disabled = false;
}

ACCOUNT_IN::ACCOUNT_IN() {
    clear();
}

ACCOUNT_IN::~ACCOUNT_IN() {
    clear();
}

void ACCOUNT_IN::clear() {
    url.clear();
    email_addr.clear();
    user_name.clear();
    passwd.clear();
}

ACCOUNT_OUT::ACCOUNT_OUT() {
    clear();
}

ACCOUNT_OUT::~ACCOUNT_OUT() {
    clear();
}

int ACCOUNT_OUT::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</account_out>")) return 0;
        else if (parse_int(buf, "<error_num>", error_num)) return error_num;
        else if (parse_str(buf, "<authenticator>", authenticator)) continue;
    }
    return ERR_XML_PARSE;
}

void ACCOUNT_OUT::clear() {
    error_num = -1;
    authenticator.clear();
}

LOOKUP_WEBSITE::LOOKUP_WEBSITE() {
    clear();
}

LOOKUP_WEBSITE::~LOOKUP_WEBSITE() {
    clear();
}

int LOOKUP_WEBSITE::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</lookup_website>")) return 0;
        else if (parse_int(buf, "<error_num>", error_num)) return error_num;
    }
    return ERR_XML_PARSE;
}

void LOOKUP_WEBSITE::clear() {
    error_num = 0;
}

/////////// END OF PARSING FUNCTIONS.  RPCS START HERE ////////////////

int RPC_CLIENT::get_state(CC_STATE& state) {
    char buf[256];
    PROJECT* project = NULL;
    RPC rpc(this);
    int retval;

    state.clear();
    client_major_version = 0;
    client_minor_version = 0;
    client_release = 0;

    retval = rpc.do_rpc("<get_state/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "<unauthorized")) {
            return ERR_AUTHENTICATOR;
        }
        if (match_tag(buf, "</client_state>")) break;
        else if (parse_int(buf, "<client_major_version>", client_major_version)) continue;
        else if (parse_int(buf, "<client_minor_version>", client_minor_version)) continue;
        else if (parse_int(buf, "<client_release>", client_release)) continue;
        else if (match_tag(buf, "<project>")) {
            project = new PROJECT();
            project->parse(rpc.fin);
            state.projects.push_back(project);
            continue;
        }
        else if (match_tag(buf, "<app>")) {
            APP* app = new APP();
            app->parse(rpc.fin);
            app->project = project;
            state.apps.push_back(app);
            continue;
        }
        else if (match_tag(buf, "<app_version>")) {
            APP_VERSION* app_version = new APP_VERSION();
            app_version->parse(rpc.fin);
            app_version->project = project;
            app_version->app = state.lookup_app(project, app_version->app_name);
            state.app_versions.push_back(app_version);
            continue;
        }
        else if (match_tag(buf, "<workunit>")) {
            WORKUNIT* wu = new WORKUNIT();
            wu->parse(rpc.fin);
            wu->project = project;
            wu->app = state.lookup_app(project, wu->app_name);
            wu->avp = state.lookup_app_version(project, wu->app_name, wu->version_num);
            state.wus.push_back(wu);
            continue;
        }
        else if (match_tag(buf, "<result>")) {
            RESULT* result = new RESULT();
            result->parse(rpc.fin);
            result->project = project;
            result->wup = state.lookup_wu(project, result->wu_name);
            result->app = result->wup->app;
            state.results.push_back(result);
            continue;
        }
        else if (match_tag(buf, "<global_preferences>")) {
            bool flag = false;
            state.global_prefs.parse(rpc.fin, "", flag);
            continue;
        }
    }
    return 0;
}

int RPC_CLIENT::get_results(RESULTS& t) {
    char buf[256];
    int retval;
    RPC rpc(this);

    t.clear();

    retval = rpc.do_rpc("<get_results/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</results>")) break;
        else if (match_tag(buf, "<result>")) {
            RESULT* rp = new RESULT();
            rp->parse(rpc.fin);
            t.results.push_back(rp);
            continue;
        }
    }
    return 0;
}

int RPC_CLIENT::get_file_transfers(FILE_TRANSFERS& t) {
    char buf[256];
    int retval;
    RPC rpc(this);

    t.clear();

    retval = rpc.do_rpc("<get_file_transfers/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</file_transfers>")) break;
        else if (match_tag(buf, "<file_transfer>")) {
            FILE_TRANSFER* fip = new FILE_TRANSFER();
            fip->parse(rpc.fin);
            t.file_transfers.push_back(fip);
            continue;
        }
    }
    return 0;
}

int RPC_CLIENT::get_project_status(PROJECTS& p) {
    char buf[256];
    RPC rpc(this);
    int retval;

    p.clear();

    retval = rpc.do_rpc("<get_project_status/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</projects>")) break;
        else if (match_tag(buf, "<project>")) {
            PROJECT* project = new PROJECT();
            project->parse(rpc.fin);
            p.projects.push_back(project);
            continue;
        }
    }
    return 0;
}

int RPC_CLIENT::get_disk_usage(PROJECTS& p) {
    char buf[256];
    RPC rpc(this);
    int retval;

    p.clear();

    retval = rpc.do_rpc("<get_disk_usage/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</projects>")) break;
        else if (match_tag(buf, "<project>")) {
            PROJECT* project = new PROJECT();
            project->parse(rpc.fin);
            p.projects.push_back(project);
            continue;
        }
    }
    return 0;
}

int DAILY_STATS::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</daily_statistics>")) return 0;
        else if (parse_double(buf, "<day>", day)) continue;
        else if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        else if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
    }
    return ERR_XML_PARSE;
}

int RPC_CLIENT::get_statistics(PROJECTS& p) {
    char buf[256];
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_statistics/>\n");
    if (retval) return retval;

    p.clear();

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</statistics>")) break;
        else if (match_tag(buf, "<project_statistics>")) {
            PROJECT* project = new PROJECT();
            p.projects.push_back(project);

            while (rpc.fin.fgets(buf, 256)) {
                if (match_tag(buf, "</project_statistics>")) break;
                else if (parse_str(buf, "<master_url>", p.projects.back()->master_url)) continue;
                else if (match_tag(buf, "<daily_statistics>")) {
                    DAILY_STATS ds;
                    retval = ds.parse(rpc.fin);
                    if (retval) return retval;
                    p.projects.back()->statistics.push_back(ds);
                }
            }
        }
    }
    return 0;
}

int RPC_CLIENT::network_query(int& want_network) {
    RPC rpc(this);
    int retval;
    char buf[256];

    retval = rpc.do_rpc("<network_query/>\n");
    if (retval) return retval;
    while (rpc.fin.fgets(buf, 256)) {
        if (parse_int(buf, "<want_network>", want_network)) {
            return 0;
        }
    }
    return -1;
}

int RPC_CLIENT::network_available() {
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<network_available/>\n");
    if (retval) return retval;
    return 0;
}

void DISPLAY_INFO::print_str(char* p) {
    char buf[256];
    if (strlen(window_station)) {
        sprintf(buf,
            "   <window_station>%s</window_station>\n", window_station
        );
        strcat(p, buf);
    }
    if (strlen(desktop)) {
        sprintf(buf,
            "   <desktop>%s</desktop>\n", desktop
        );
        strcat(p, buf);
    }
    if (strlen(display)) {
        sprintf(buf,
            "   <display>%s</display>\n", display
        );
        strcat(p, buf);
    }
}

int RPC_CLIENT::show_graphics(
    const char* project_url, const char* result_name, bool full_screen,
    DISPLAY_INFO& di
) {
    char buf[1024];
    RPC rpc(this);

    sprintf(buf, 
        "<result_show_graphics>\n"
        "   <project_url>%s</project_url>\n"
        "   <result_name>%s</result_name>\n"
        "%s",
        project_url,
        result_name,
        full_screen?"   <full_screen/>\n":""
    );
    di.print_str(buf);
    strcat(buf, "</result_show_graphics>\n");
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::project_op(PROJECT& project, const char* op) {
    char buf[256];
    const char *tag;
    int retval;
    RPC rpc(this);

    if (!strcmp(op, "reset")) {
        tag = "project_reset";
    } else if (!strcmp(op, "detach")) {
        tag = "project_detach";
    } else if (!strcmp(op, "update")) {
        tag = "project_update";
    } else if (!strcmp(op, "suspend")) {
        tag = "project_suspend";
    } else if (!strcmp(op, "resume")) {
        tag = "project_resume";
    } else if (!strcmp(op, "allowmorework")) {
         tag = "project_allowmorework";
    } else if (!strcmp(op, "nomorework")) {
         tag = "project_nomorework";
    } else {
        return -1;
    }
    sprintf(buf,
        "<%s>\n"
        "  <project_url>%s</project_url>\n"
        "</%s>\n",
        tag,
        project.master_url.c_str(),
        tag
    );
    retval =  rpc.do_rpc(buf);
    if (retval) return retval;
    return rpc.parse_reply();
}

int RPC_CLIENT::project_attach(const char* url, const char* auth, bool use_cached_credentials) {
    char buf[256];
    int retval;
    RPC rpc(this);

    if (use_cached_credentials) {
        sprintf(buf,
            "<project_attach>\n"
            "  <use_cached_credentials/>\n"
            "</project_attach>\n"
        );
    } else {
        sprintf(buf,
            "<project_attach>\n"
            "  <project_url>%s</project_url>\n"
            "  <authenticator>%s</authenticator>\n"
            "</project_attach>\n",
            url, auth
        );
    }

    retval =  rpc.do_rpc(buf);
    if (retval) return retval;
    return rpc.parse_reply();
}

int RPC_CLIENT::project_attach_poll() {
    RPC rpc(this);
    char buf[256];
    int retval;

    retval = rpc.do_rpc("<project_attach_poll/>\n");
    if (retval) return retval;
    retval = ERR_XML_PARSE;
    while (rpc.fin.fgets(buf, 256)) {
        parse_int(buf, "<error_num>", retval);
    }
    return retval;
}

const char* RPC_CLIENT::mode_name(int mode) {
    const char* p = NULL;
    switch (mode) {
    case RUN_MODE_ALWAYS: p="<always/>"; break;
    case RUN_MODE_NEVER: p="<never/>"; break;
    case RUN_MODE_AUTO: p="<auto/>"; break;
    }
    return p;
}

int RPC_CLIENT::set_run_mode(int mode) {
    char buf[256];
    RPC rpc(this);

    sprintf(buf, "<set_run_mode>\n%s\n</set_run_mode>\n", mode_name(mode));
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::get_run_mode(int& mode) {
    char buf[256];
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_run_mode/>\n");
    if (retval) return retval;

    mode = -1;
    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</run_mode>")) break;
        else if (match_tag(buf, mode_name(RUN_MODE_ALWAYS))) mode = RUN_MODE_ALWAYS;
        else if (match_tag(buf, mode_name(RUN_MODE_NEVER))) mode = RUN_MODE_NEVER;
        else if (match_tag(buf, mode_name(RUN_MODE_AUTO))) mode = RUN_MODE_AUTO;
    }
    return 0;
}

int RPC_CLIENT::set_network_mode(int mode) {
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<set_network_mode>\n"
        "%s"
        "</set_network_mode>\n",
        mode_name(mode)
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::get_network_mode(int& mode) {
    char buf[256];
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_network_mode/>\n");
    if (retval) return retval;

    mode = -1;
    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</network_mode>")) break;
        if (match_tag(buf, mode_name(RUN_MODE_ALWAYS))) mode = RUN_MODE_ALWAYS;
        if (match_tag(buf, mode_name(RUN_MODE_NEVER))) mode = RUN_MODE_NEVER;
        if (match_tag(buf, mode_name(RUN_MODE_AUTO))) mode = RUN_MODE_AUTO;
    }
    return 0;
}

int RPC_CLIENT::get_activity_state(bool& activities_suspended, bool& network_suspended) {
    char buf[256];
    RPC rpc(this);
    int retval;

    activities_suspended = false;
    network_suspended = false;

    retval = rpc.do_rpc("<get_activity_state/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</activity_state>")) break;
        else if (match_tag(buf, "<activities_suspended/>")) {
            activities_suspended = true;
            continue;
        }
        else if (match_tag(buf, "<network_suspended/>")) {
            network_suspended = true;
            continue;
        }
    }

    return 0;
}

int RPC_CLIENT::get_screensaver_mode(int& status) {
    char buf[256];
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_screensaver_mode/>\n");
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</screensaver_mode>")) break;
        else if (parse_int(buf, "<status>", status)) continue;
    }
    BOINCTRACE("Receiving: get_screensaver_mode\nstatus = %d\n", status);
    return 0;
}

int RPC_CLIENT::set_screensaver_mode(
    bool enabled, double blank_time,
    DISPLAY_INFO& di
) {
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<set_screensaver_mode>\n"
        "     %s\n"
        "     <blank_time>%f</blank_time>\n",
        enabled ? "<enabled/>" : "",
        blank_time
    );
    di.print_str(buf);
    strcat(buf, "</set_screensaver_mode>\n");

    BOINCTRACE("Sending: set_screensaver_mode\n%s\n", buf);
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::run_benchmarks() {
    RPC rpc(this);
    return rpc.do_rpc("<run_benchmarks/>\n");
}

int RPC_CLIENT::set_proxy_settings(GR_PROXY_INFO& pi) {
    char buf[1024];
    RPC rpc(this);

    sprintf(buf,
        "<set_proxy_settings>\n%s%s%s"
        "    <proxy_info>\n"
        "        <http_server_name>%s</http_server_name>\n"
        "        <http_server_port>%d</http_server_port>\n"
        "        <http_user_name>%s</http_user_name>\n"
        "        <http_user_passwd>%s</http_user_passwd>\n"
        "        <socks_server_name>%s</socks_server_name>\n"
        "        <socks_server_port>%d</socks_server_port>\n"
        "        <socks_version>%d</socks_version>\n"
        "        <socks5_user_name>%s</socks5_user_name>\n"
        "        <socks5_user_passwd>%s</socks5_user_passwd>\n"
        "    </proxy_info>\n"
        "</set_proxy_settings>\n",
        pi.use_http_proxy?"   <use_http_proxy/>\n":"",
        pi.use_socks_proxy?"   <use_socks_proxy/>\n":"",
        pi.use_http_authentication?"   <use_http_auth/>\n":"",
        pi.http_server_name.c_str(),
        pi.http_server_port,
        pi.http_user_name.c_str(),
        pi.http_user_passwd.c_str(),
        pi.socks_server_name.c_str(),
        pi.socks_server_port,
        pi.socks_version,
        pi.socks5_user_name.c_str(),
        pi.socks5_user_passwd.c_str()
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::get_proxy_settings(GR_PROXY_INFO& p) {
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_proxy_settings/>");
    if (retval) return retval;

    return p.parse(rpc.fin);
}

int RPC_CLIENT::get_messages(int seqno, MESSAGES& msgs) {
    char buf[4096];
    RPC rpc(this);
    int retval;

    sprintf(buf,
        "<get_messages>\n"
        "  <seqno>%d</seqno>\n"
        "</get_messages>\n",
        seqno
    );
    retval = rpc.do_rpc(buf);
    if (retval) return retval;

    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "</msgs>")) break;
        else if (match_tag(buf, "<msg>")) {
            MESSAGE* message = new MESSAGE();
            message->parse(rpc.fin);
            msgs.messages.push_back(message);
            continue;
        }
    }

    return 0;
}

int RPC_CLIENT::file_transfer_op(FILE_TRANSFER& ft, const char* op) {
    char buf[4096];
    const char *tag;
    RPC rpc(this);

    if (!strcmp(op, "retry")) {
        tag = "retry_file_transfer";
    } else if (!strcmp(op, "abort")) {
        tag = "abort_file_transfer";
    } else {
        return -1;
    }
    sprintf(buf,
        "<%s>\n"
        "   <project_url>%s</project_url>\n"
        "   <filename>%s</filename>\n"
        "</%s>\n",
        tag,
        ft.project_url.c_str(),
        ft.name.c_str(),
        tag
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::result_op(RESULT& result, const char* op) {
    char buf[4096];
    const char *tag;
    RPC rpc(this);

    if (!strcmp(op, "abort")) {
        tag = "abort_result";
    } else if (!strcmp(op, "suspend")) {
        tag = "suspend_result";
    } else if (!strcmp(op, "resume")) {
        tag = "resume_result";
    } else {
        return -1;
    }
    sprintf(buf,
        "<%s>\n"
        "   <project_url>%s</project_url>\n"
        "   <name>%s</name>\n"
        "</%s>\n",
        tag,
        result.project_url.c_str(),
        result.name.c_str(),
        tag
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::get_host_info(HOST_INFO& h) {
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_host_info/>");
    if (retval) return retval;

    return h.parse(rpc.fin);
}


int RPC_CLIENT::quit() {
    RPC rpc(this);
    return rpc.do_rpc("<quit/>\n");
}

int RPC_CLIENT::acct_mgr_rpc(const char* url, const char* name, const char* password, bool use_cached_credentials) {
    char buf[4096];
    RPC rpc(this);
    if (use_cached_credentials) {
        sprintf(buf,
            "<acct_mgr_rpc>\n"
            "  <use_cached_credentials/>\n"
            "</acct_mgr_rpc>\n"
        );
    }else {
        sprintf(buf,
            "<acct_mgr_rpc>\n"
            "  <url>%s</url>\n"
            "  <name>%s</name>\n"
            "  <password>%s</password>\n"
            "</acct_mgr_rpc>\n",
            url, name, password
        );
    }
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::acct_mgr_rpc_poll() {
    ACCT_MGR_RPC_REPLY r;
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<acct_mgr_rpc_poll/>\n");
    if (retval) return retval;
    return r.parse(rpc.fin);
}

int RPC_CLIENT::acct_mgr_info(ACCT_MGR_INFO& ami) {
    int retval;
    RPC rpc(this);
    retval = rpc.do_rpc("<acct_mgr_info/>\n");
    if (retval) return retval;
    return ami.parse(rpc.fin);
}

int RPC_CLIENT::get_project_init_status(PROJECT_INIT_STATUS& pis) {
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_project_init_status/>\n");
    if (retval) return retval;
    return pis.parse(rpc.fin);
}


int RPC_CLIENT::get_project_config(std::string url) {
    char buf[4096];
    int retval;
    RPC rpc(this);

    sprintf(buf,
        "<get_project_config>\n"
        "   <url>%s</url>\n"
        "</get_project_config>\n",
        url.c_str()
    );

    retval =  rpc.do_rpc(buf);
    if (retval) return retval;
    return rpc.parse_reply();
}

int RPC_CLIENT::get_project_config_poll(PROJECT_CONFIG& pc) {
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_project_config_poll/>\n");
    if (retval) return retval;
    return pc.parse(rpc.fin);
}

static string get_passwd_hash(string passwd, string email_addr) {
    return md5_string(passwd+email_addr);

}
int RPC_CLIENT::lookup_account(ACCOUNT_IN& ai) {
    char buf[4096];
    int retval;
    RPC rpc(this);

    downcase_string(ai.email_addr);
    string passwd_hash = get_passwd_hash(ai.passwd, ai.email_addr);
    sprintf(buf,
        "<lookup_account>\n"
        "   <url>%s</url>\n"
        "   <email_addr>%s</email_addr>\n"
        "   <passwd_hash>%s</passwd_hash>\n"
        "</lookup_account>\n",
        ai.url.c_str(),
        ai.email_addr.c_str(),
        passwd_hash.c_str()
    );

    retval =  rpc.do_rpc(buf);
    if (retval) return retval;
    return rpc.parse_reply();
}

int RPC_CLIENT::lookup_account_poll(ACCOUNT_OUT& ao) {
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<lookup_account_poll/>\n");
    if (retval) return retval;
    return ao.parse(rpc.fin);
}

int RPC_CLIENT::create_account(ACCOUNT_IN& ai) {
    char buf[4096];
    int retval;
    RPC rpc(this);

    downcase_string(ai.email_addr);
    string passwd_hash = get_passwd_hash(ai.passwd, ai.email_addr);
    sprintf(buf,
        "<create_account>\n"
        "   <url>%s</url>\n"
        "   <email_addr>%s</email_addr>\n"
        "   <passwd_hash>%s</passwd_hash>\n"
        "   <user_name>%s</user_name>\n"
        "</create_account>\n",
        ai.url.c_str(),
        ai.email_addr.c_str(),
        passwd_hash.c_str(),
        ai.user_name.c_str()
    );

    retval =  rpc.do_rpc(buf);
    if (retval) return retval;
    return rpc.parse_reply();
}

int RPC_CLIENT::create_account_poll(ACCOUNT_OUT& ao) {
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<create_account_poll/>\n");
    if (retval) return retval;
    return ao.parse(rpc.fin);
}

int RPC_CLIENT::lookup_website(int website_id) {
    char buf[4096];
    RPC rpc(this);
    switch (website_id) {
    case LOOKUP_GOOGLE:
    case LOOKUP_YAHOO:
        break;
    default:
        return ERR_INVALID_PARAM;
    }
    sprintf(buf,
        "<lookup_website>\n"
        "    %s%s\n"
        "</lookup_website>\n",
        (LOOKUP_GOOGLE == website_id)?"<google/>":"",
        (LOOKUP_YAHOO == website_id)?"<yahoo/>":""
    );
    return rpc.do_rpc(buf);
}

int RPC_CLIENT::lookup_website_poll() {
    RPC rpc(this);
    LOOKUP_WEBSITE lw;
    int retval;

    retval = rpc.do_rpc("<lookup_website_poll/>\n");
    if (retval) return retval;
    return lw.parse(rpc.fin);
}

const char *BOINC_RCSID_90e8b8d168="$Id$";

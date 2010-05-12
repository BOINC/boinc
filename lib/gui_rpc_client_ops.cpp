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

// This file contains:
// 1) functions to clear and parse the various structs
// 2) actual GUI RPCs

// The core client expects all data to be formatted in the "C" locale,
// so each GUI RPC should get the current locale, then switch to the
// "C" locale before formatting messages or parsing results.
// After all work is completed, revert back to the original locale.
//
// Template:
//
// int RPC_CLIENT::template_function( args ) {
//     int retval;
//     SET_LOCALE sl;
//     char buf[256];
//     RPC rpc(this);
//
//     <do something useful>
//
//     return retval;
// }
//
// NOTE: Failing to revert back to the original locale will cause
//   formatting failures for any software that has been localized or
//   displays localized data.


#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "../version.h"
#else
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <locale>
#endif

#include "diagnostics.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "miofile.h"
#include "md5_file.h"
#include "network.h"
#include "common_defs.h"
#include "gui_rpc_client.h"

using std::string;
using std::vector;
using std::sort;

DISPLAY_INFO::DISPLAY_INFO() {
    memset(this, 0, sizeof(DISPLAY_INFO));
}

int GUI_URL::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</gui_url>")) return 0;
        if (match_tag(buf, "</gui_urls>")) break;
        if (parse_str(buf, "<name>", name)) continue;
        if (parse_str(buf, "<description>", description)) continue;
        if (parse_str(buf, "<url>", url)) continue;
    }
    return ERR_XML_PARSE;
}


PROJECT_LIST_ENTRY::PROJECT_LIST_ENTRY() {
    clear();
}

PROJECT_LIST_ENTRY::~PROJECT_LIST_ENTRY() {
    clear();
}

int PROJECT_LIST_ENTRY::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    string platform;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!strcmp(tag, "/project")) return 0;
        if (xp.parse_string(tag, "name", name)) continue;
        if (xp.parse_string(tag, "url", url)) continue;
        if (xp.parse_string(tag, "general_area", general_area)) continue;
        if (xp.parse_string(tag, "specific_area", specific_area)) continue;
        if (xp.parse_string(tag, "description", description)) continue;
        if (xp.parse_string(tag, "home", home)) continue;
        if (xp.parse_string(tag, "image", image)) continue;
        if (!strcmp(tag, "platforms")) {
            while (!xp.get(tag, sizeof(tag), is_tag)) {
                if (!strcmp(tag, "/platforms")) break;
                if (xp.parse_string(tag, "name", platform)) {
                    platforms.push_back(platform);
                }
            }
        }
        xp.skip_unexpected(tag, false, "");
    }
    return ERR_XML_PARSE;
}

void PROJECT_LIST_ENTRY::clear() {
    name.clear();
    url.clear();
    general_area.clear();
    specific_area.clear();
    description.clear();
    platforms.clear();
    home.clear();
    image.clear();
}

AM_LIST_ENTRY::AM_LIST_ENTRY() {
    clear();
}

AM_LIST_ENTRY::~AM_LIST_ENTRY() {
    clear();
}

int AM_LIST_ENTRY::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!strcmp(tag, "/account_manager")) return 0;
        if (xp.parse_string(tag, "name", name)) continue;
        if (xp.parse_string(tag, "url", url)) continue;
        if (xp.parse_string(tag, "description", description)) continue;
        if (xp.parse_string(tag, "image", image)) continue;
    }
    return 0;
}

void AM_LIST_ENTRY::clear() {
    name.clear();
    url.clear();
    description.clear();
    image.clear();
}

ALL_PROJECTS_LIST::ALL_PROJECTS_LIST() {
}

ALL_PROJECTS_LIST::~ALL_PROJECTS_LIST() {
    clear();
}

bool compare_project_list_entry(const PROJECT_LIST_ENTRY* a, const PROJECT_LIST_ENTRY* b) 
{
    return a->name < b->name;
}

bool compare_am_list_entry(const AM_LIST_ENTRY* a, const AM_LIST_ENTRY* b) 
{
    return a->name < b->name;
}

void ALL_PROJECTS_LIST::shuffle() {
    sort(projects.begin(), projects.end(), compare_project_list_entry);
    sort(account_managers.begin(), account_managers.end(), compare_am_list_entry);
}

void ALL_PROJECTS_LIST::clear() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        delete projects[i];
    }
    for (i=0; i<account_managers.size(); i++) {
        delete account_managers[i];
    }
    projects.clear();
    account_managers.clear();
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
        if (parse_str(buf, "<master_url>", master_url, sizeof(master_url))) continue;
        if (parse_double(buf, "<resource_share>", resource_share)) continue;
        if (parse_str(buf, "<project_name>", project_name)) continue;
        if (parse_str(buf, "<user_name>", user_name)) {
            xml_unescape(user_name);
            continue;
        }
        if (parse_str(buf, "<team_name>", team_name)) {
            xml_unescape(team_name);
            continue;
        }
        if (parse_int(buf, "<hostid>", hostid)) continue;
        if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
        if (parse_double(buf, "<disk_usage>", disk_usage)) continue;
        if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        if (parse_int(buf, "<master_fetch_failures>", master_fetch_failures)) continue;
        if (parse_double(buf, "<min_rpc_time>", min_rpc_time)) continue;
        if (parse_double(buf, "<download_backoff>", download_backoff)) continue;
        if (parse_double(buf, "<upload_backoff>", upload_backoff)) continue;
        if (parse_double(buf, "<short_term_debt>", cpu_short_term_debt)) continue;
        if (parse_double(buf, "<long_term_debt>", cpu_long_term_debt)) continue;
        if (parse_double(buf, "<cpu_backoff_time>", cpu_backoff_time)) continue;
        if (parse_double(buf, "<cpu_backoff_interval>", cpu_backoff_interval)) continue;
        if (parse_double(buf, "<cuda_debt>", cuda_debt)) continue;
        if (parse_double(buf, "<cuda_short_term_debt>", cuda_short_term_debt)) continue;
        if (parse_double(buf, "<cuda_backoff_time>", cuda_backoff_time)) continue;
        if (parse_double(buf, "<cuda_backoff_interval>", cuda_backoff_interval)) continue;
        if (parse_double(buf, "<ati_debt>", ati_debt)) continue;
        if (parse_double(buf, "<ati_short_term_debt>", ati_short_term_debt)) continue;
        if (parse_double(buf, "<ati_backoff_time>", ati_backoff_time)) continue;
        if (parse_double(buf, "<ati_backoff_interval>", ati_backoff_interval)) continue;
        if (parse_double(buf, "<duration_correction_factor>", duration_correction_factor)) continue;
        if (parse_bool(buf, "anonymous_platform", anonymous_platform)) continue;
        if (parse_bool(buf, "master_url_fetch_pending", master_url_fetch_pending)) continue;
        if (parse_int(buf, "<sched_rpc_pending>", sched_rpc_pending)) continue;
        if (parse_bool(buf, "non_cpu_intensive", non_cpu_intensive)) continue;
        if (parse_bool(buf, "suspended_via_gui", suspended_via_gui)) continue;
        if (parse_bool(buf, "dont_request_more_work", dont_request_more_work)) continue;
        if (parse_bool(buf, "ended", ended)) continue;
        if (parse_bool(buf, "scheduler_rpc_in_progress", scheduler_rpc_in_progress)) continue;
        if (parse_bool(buf, "attached_via_acct_mgr", attached_via_acct_mgr)) continue;
        if (parse_bool(buf, "detach_when_done", detach_when_done)) continue;
        if (parse_bool(buf, "trickle_up_pending", trickle_up_pending)) continue;
        if (match_tag(buf, "<gui_urls>")) {
            while (in.fgets(buf, 256)) {
                if (match_tag(buf, "</gui_urls>")) break;
                if (match_tag(buf, "<gui_url>")) {
                    GUI_URL gu;
                    retval = gu.parse(in);
                    if (retval) break;
                    gui_urls.push_back(gu);
                    continue;
                }
            }
            continue;
        }
        if (parse_double(buf, "<project_files_downloaded_time>", project_files_downloaded_time)) continue;
        if (parse_double(buf, "<last_rpc_time>", last_rpc_time)) continue;
        if (parse_bool(buf, "no_cpu_pref", no_cpu_pref)) continue;
        if (parse_bool(buf, "no_cuda_pref", no_cuda_pref)) continue;
        if (parse_bool(buf, "no_ati_pref", no_ati_pref)) continue;
    }
    return ERR_XML_PARSE;
}

void PROJECT::clear() {
    strcpy(master_url, "");
    resource_share = 0;
    project_name.clear();
    user_name.clear();
    team_name.clear();
    user_total_credit = 0;
    user_expavg_credit = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    disk_usage = 0;
    nrpc_failures = 0;
    master_fetch_failures = 0;
    min_rpc_time = 0;
    download_backoff = 0;
    upload_backoff = 0;
    cpu_short_term_debt = 0;
    cpu_long_term_debt = 0;
    cpu_backoff_time = 0;
    cpu_backoff_interval = 0;
    cuda_debt = 0;
    cuda_backoff_time = 0;
    cuda_backoff_interval = 0;
    ati_debt = 0;
    ati_backoff_time = 0;
    ati_backoff_interval = 0;
    duration_correction_factor = 0;
    anonymous_platform = false;
    master_url_fetch_pending = false;
    sched_rpc_pending = 0;
    ended = false;
    non_cpu_intensive = false;
    suspended_via_gui = false;
    dont_request_more_work = false;
    scheduler_rpc_in_progress = false;
    attached_via_acct_mgr = false;
    detach_when_done = false;
    trickle_up_pending = false;
    project_files_downloaded_time = 0;
    last_rpc_time = 0;
    gui_urls.clear();
    statistics.clear();
    no_cpu_pref = false;
    no_cuda_pref = false;
    no_ati_pref = false;
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
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<user_friendly_name>", user_friendly_name, sizeof(user_friendly_name))) continue;
    }
    return ERR_XML_PARSE;
}

void APP::clear() {
    strcpy(name, "");
    strcpy(user_friendly_name, "");
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
        if (parse_str(buf, "<app_name>", app_name, sizeof(app_name))) continue;
        if (parse_str(buf, "<plan_class>", plan_class, sizeof(plan_class))) continue;
        if (parse_int(buf, "<version_num>", version_num)) continue;
    }
    return ERR_XML_PARSE;
}

void APP_VERSION::clear() {
    strcpy(app_name, "");
    strcpy(plan_class, "");
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
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<app_name>", app_name, sizeof(app_name))) continue;
        if (parse_int(buf, "<version_num>", version_num)) continue;
        if (parse_double(buf, "<rsc_fpops_est>", rsc_fpops_est)) continue;
        if (parse_double(buf, "<rsc_fpops_bound>", rsc_fpops_bound)) continue;
        if (parse_double(buf, "<rsc_memory_bound>", rsc_memory_bound)) continue;
        if (parse_double(buf, "<rsc_disk_bound>", rsc_disk_bound)) continue;
    }
    return ERR_XML_PARSE;
}

void WORKUNIT::clear() {
    strcpy(name, "");
    strcpy(app_name, "");
    version_num = 0;
    rsc_fpops_est = 0;
    rsc_fpops_bound = 0;
    rsc_memory_bound = 0;
    rsc_disk_bound = 0;
    project = NULL;
    app = NULL;
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
        if (match_tag(buf, "</result>")) {
            // if CPU time is nonzero but elapsed time is zero,
            // we must be talking to an old client.
            // Set elapsed = CPU
            // (easier to deal with this here than in the manager)
            //
            if (current_cpu_time && !elapsed_time) {
                elapsed_time = current_cpu_time;
            }
            if (final_cpu_time && !final_elapsed_time) {
                final_elapsed_time = final_cpu_time;
            }
            return 0;
        }
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<wu_name>", wu_name, sizeof(wu_name))) continue;
        if (parse_int(buf, "<version_num>", version_num)) continue;
        if (parse_str(buf, "<plan_class>", plan_class, sizeof(plan_class))) continue;
        if (parse_str(buf, "<project_url>", project_url, sizeof(project_url))) continue;
        if (parse_double(buf, "<report_deadline>", report_deadline)) continue;
        if (parse_double(buf, "<received_time>", received_time)) continue;
        if (parse_bool(buf, "ready_to_report", ready_to_report)) continue;
        if (parse_bool(buf, "got_server_ack", got_server_ack)) continue;
        if (parse_bool(buf, "suspended_via_gui", suspended_via_gui)) continue;
        if (parse_bool(buf, "project_suspended_via_gui", project_suspended_via_gui)) continue;
        if (parse_bool(buf, "coproc_missing", coproc_missing)) continue;
        if (parse_bool(buf, "gpu_mem_wait", gpu_mem_wait)) continue;
        if (match_tag(buf, "<active_task>")) {
            active_task = true;
            continue;
        }
        if (parse_bool(buf, "supports_graphics", supports_graphics)) continue;
        if (parse_int(buf, "<graphics_mode_acked>", graphics_mode_acked)) continue;
        if (parse_double(buf, "<final_cpu_time>", final_cpu_time)) continue;
        if (parse_double(buf, "<final_elapsed_time>", final_elapsed_time)) continue;
        if (parse_int(buf, "<state>", state)) continue;
        if (parse_int(buf, "<scheduler_state>", scheduler_state)) continue;
        if (parse_int(buf, "<exit_status>", exit_status)) continue;
        if (parse_int(buf, "<signal>", signal)) continue;
        if (parse_int(buf, "<active_task_state>", active_task_state)) continue;
#if 0
        if (match_tag(buf, "<stderr_out>")) {
            copy_element_contents(in, "</stderr_out>", stderr_out);
            continue;
        }
#endif
        if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        if (parse_int(buf, "<slot>", slot)) continue;
        if (parse_int(buf, "<pid>", pid)) continue;
        if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu_time)) continue;
        if (parse_double(buf, "<current_cpu_time>", current_cpu_time)) continue;
        if (parse_double(buf, "<elapsed_time>", elapsed_time)) continue;
        if (parse_double(buf, "<swap_size>", swap_size)) continue;
        if (parse_double(buf, "<working_set_size_smoothed>", working_set_size_smoothed)) continue;
        if (parse_double(buf, "<fraction_done>", fraction_done)) continue;
        if (parse_double(buf, "<estimated_cpu_time_remaining>", estimated_cpu_time_remaining)) continue;
        if (parse_bool(buf, "too_large", too_large)) continue;
        if (parse_bool(buf, "needs_shmem", needs_shmem)) continue;
        if (parse_bool(buf, "edf_scheduled", edf_scheduled)) continue;
        if (parse_str(buf, "graphics_exec_path", graphics_exec_path, sizeof(graphics_exec_path))) continue;
        if (parse_str(buf, "slot_path", slot_path, sizeof(slot_path))) continue;
        if (parse_str(buf, "resources", resources, sizeof(resources))) continue;
    }
    return ERR_XML_PARSE;
}

void RESULT::clear() {
    strcpy(name, "");
    strcpy(wu_name, "");
    version_num = 0;
    strcpy(plan_class, "");
    strcpy(project_url, "");
    strcpy(graphics_exec_path, "");
    strcpy(slot_path, "");
    strcpy(resources, "");
    report_deadline = 0;
    received_time = 0;
    ready_to_report = false;
    got_server_ack = false;
    final_cpu_time = 0;
    final_elapsed_time = 0;
    state = 0;
    scheduler_state = 0;
    exit_status = 0;
    signal = 0;
    //stderr_out.clear();
    suspended_via_gui = false;
    project_suspended_via_gui = false;
    coproc_missing = false;
    gpu_mem_wait = false;

    active_task = false;
    active_task_state = 0;
    app_version_num = 0;
    slot = -1;
    pid = 0;
    checkpoint_cpu_time = 0;
    current_cpu_time = 0;
    fraction_done = 0;
    elapsed_time = 0;
    swap_size = 0;
    working_set_size_smoothed = 0;
    estimated_cpu_time_remaining = 0;
    supports_graphics = false;
    graphics_mode_acked = 0;
    too_large = false;
    needs_shmem = false;
    edf_scheduled = false;

    app = NULL;
    wup = NULL;
    project = NULL;
    avp = NULL;
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
        if (parse_str(buf, "<name>", name)) continue;
        if (parse_str(buf, "<project_url>", project_url)) continue;
        if (parse_str(buf, "<project_name>", project_name)) continue;
        if (parse_double(buf, "<nbytes>", nbytes)) continue;
        if (parse_bool(buf, "generated_locally", generated_locally)) continue;
        if (parse_bool(buf, "uploaded", uploaded)) continue;
        if (parse_bool(buf, "upload_when_present", upload_when_present)) continue;
        if (parse_bool(buf, "sticky", sticky)) continue;
        if (match_tag(buf, "<persistent_file_xfer>")) {
            pers_xfer_active = true;
            continue;
        }
        if (match_tag(buf, "<file_xfer>")) {
            xfer_active = true;
            continue;
        }
        if (parse_int(buf, "<num_retries>", num_retries)) continue;
        if (parse_int(buf, "<first_request_time>", first_request_time)) continue;
        if (parse_int(buf, "<next_request_time>", next_request_time)) continue;
        if (parse_int(buf, "<status>", status)) continue;
        if (parse_double(buf, "<time_so_far>", time_so_far)) continue;
        if (parse_double(buf, "<last_bytes_xferred>", bytes_xferred)) continue;
        if (parse_double(buf, "<file_offset>", file_offset)) continue;
        if (parse_double(buf, "<xfer_speed>", xfer_speed)) continue;
        if (parse_str(buf, "<hostname>", hostname)) continue;
        if (parse_double(buf, "<project_backoff>", project_backoff)) continue;
    }
    return ERR_XML_PARSE;
}

void FILE_TRANSFER::clear() {
    name.clear();
    project_url.clear();
    project_name.clear();
    nbytes = 0;
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
    time_so_far = 0;
    bytes_xferred = 0;
    file_offset = 0;
    xfer_speed = 0;
    hostname.clear();
    project = NULL;
    project_backoff = 0;
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
        if (parse_str(buf, "<project>", project)) continue;
        if (match_tag(buf, "<body>" )) {
            copy_element_contents(in, "</body>", body);
            continue;
        }
        if (parse_int(buf, "<pri>", priority)) continue;
        if (parse_int(buf, "<time>", timestamp)) continue;
        if (parse_int(buf, "<seqno>", seqno)) continue;
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
	std::string noproxy;
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_authentication = false;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</proxy_info>")) return 0;
        if (parse_int(buf, "<socks_version>", socks_version)) continue;
        if (parse_str(buf, "<socks_server_name>", socks_server_name)) continue;
        if (parse_int(buf, "<socks_server_port>", socks_server_port)) continue;
        if (parse_str(buf, "<socks5_user_name>", socks5_user_name)) continue;
        if (parse_str(buf, "<socks5_user_passwd>", socks5_user_passwd)) continue;
        if (parse_str(buf, "<http_server_name>", http_server_name)) continue;
        if (parse_int(buf, "<http_server_port>", http_server_port)) continue;
        if (parse_str(buf, "<http_user_name>", http_user_name)) continue;
        if (parse_str(buf, "<http_user_passwd>", http_user_passwd)) continue;
        if (parse_bool(buf, "use_http_proxy", use_http_proxy)) continue;
        if (parse_bool(buf, "use_socks_proxy", use_socks_proxy)) continue;
        if (parse_bool(buf, "use_http_auth", use_http_authentication)) continue;
		if (parse_str(buf, "<no_proxy>", noproxy_hosts)) continue;
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
	noproxy_hosts.clear();
}

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
    platforms.clear();
    executing_as_daemon = false;
    have_cuda = false;
    have_ati = false;
}

PROJECT* CC_STATE::lookup_project(char* url) {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        if (!strcmp(projects[i]->master_url, url)) return projects[i];
    }
    return 0;
}

APP* CC_STATE::lookup_app(PROJECT* project, char* name) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        if (apps[i]->project != project) continue;
        if (!strcmp(apps[i]->name, name)) return apps[i];
    }
    return 0;
}

APP_VERSION* CC_STATE::lookup_app_version(
    PROJECT* project, APP* app, int version_num, char* plan_class
) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (app_versions[i]->project != project) continue;
        if (app_versions[i]->app != app) continue;
        if (app_versions[i]->version_num != version_num) continue;
        if (strcmp(app_versions[i]->plan_class, plan_class)) continue;
        return app_versions[i];
    }
    return 0;
}

APP_VERSION* CC_STATE::lookup_app_version_old(
    PROJECT* project, APP* app, int version_num
) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (app_versions[i]->project != project) continue;
        if (app_versions[i]->app != app) continue;
        if (app_versions[i]->version_num != version_num) continue;
        return app_versions[i];
    }
    return 0;
}

WORKUNIT* CC_STATE::lookup_wu(PROJECT* project, char* name) {
    unsigned int i;
    for (i=0; i<wus.size(); i++) {
        if (wus[i]->project != project) continue;
        if (!strcmp(wus[i]->name, name)) return wus[i];
    }
    return 0;
}

RESULT* CC_STATE::lookup_result(PROJECT* project, char* name) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (results[i]->project != project) continue;
        if (!strcmp(results[i]->name, name)) return results[i];
    }
    return 0;
}

RESULT* CC_STATE::lookup_result(char* url, char* name) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (strcmp(results[i]->project->master_url, url)) continue;
        if (!strcmp(results[i]->name, name)) return results[i];
    }
    return 0;
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

DISK_USAGE::~DISK_USAGE() {
    clear();
}

void DISK_USAGE::clear() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        delete projects[i];
    }
    projects.clear();
    d_free = 0;
    d_total = 0;
    d_boinc = 0;
    d_allowed = 0;
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

NOTICES::NOTICES() {
    clear();
}

NOTICES::~NOTICES() {
    clear();
}

void NOTICES::clear() {
    unsigned int i;
    for (i=0; i<notices.size(); i++) {
        delete notices[i];
    }
    notices.clear();
}

ACCT_MGR_INFO::ACCT_MGR_INFO() {
    clear();
}

int ACCT_MGR_INFO::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</acct_mgr_info>")) return 0;
        if (parse_str(buf, "<acct_mgr_name>", acct_mgr_name)) continue;
        if (parse_str(buf, "<acct_mgr_url>", acct_mgr_url)) continue;
        if (parse_bool(buf, "have_credentials", have_credentials)) continue;
        if (parse_bool(buf, "cookie_required", cookie_required)) continue;
        if (parse_str(buf, "<cookie_failure_url>", cookie_failure_url)) continue;
    }
    return ERR_XML_PARSE;
}

void ACCT_MGR_INFO::clear() {
    acct_mgr_name = "";
    acct_mgr_url = "";
    have_credentials = false;
    cookie_required = false;
    cookie_failure_url = "";
}

ACCT_MGR_RPC_REPLY::ACCT_MGR_RPC_REPLY() {
    clear();
}

int ACCT_MGR_RPC_REPLY::parse(MIOFILE& in) {
    char buf[256];
    std::string msg;
    clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</acct_mgr_rpc_reply>")) return 0;
        if (parse_int(buf, "<error_num>", error_num)) continue;
        if (parse_str(buf, "<message>", msg)) {
            messages.push_back(msg);
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void ACCT_MGR_RPC_REPLY::clear() {
    messages.clear();
    error_num = 0;
}

PROJECT_ATTACH_REPLY::PROJECT_ATTACH_REPLY() {
    clear();
}

int PROJECT_ATTACH_REPLY::parse(MIOFILE& in) {
    char buf[256];
    std::string msg;
    clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</project_attach_reply>")) return 0;
        if (parse_int(buf, "<error_num>", error_num)) continue;
        if (parse_str(buf, "<message>", msg)) {
            messages.push_back(msg);
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void PROJECT_ATTACH_REPLY::clear() {
    messages.clear();
    error_num = 0;
}

PROJECT_INIT_STATUS::PROJECT_INIT_STATUS() {
    clear();
}

int PROJECT_INIT_STATUS::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</get_project_init_status>")) return 0;
        if (parse_str(buf, "<url>", url)) continue;
        if (parse_str(buf, "<name>", name)) continue;
        if (parse_str(buf, "<team_name>", team_name)) continue;
        if (parse_bool(buf, "has_account_key", has_account_key)) continue;
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
    std::string msg;
    clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</project_config>")) return 0;
        if (parse_int(buf, "<error_num>", error_num)) continue;
        if (parse_str(buf, "<name>", name)) continue;
        if (parse_str(buf, "<master_url>", master_url)) continue;
        if (parse_int(buf, "<local_revision>", local_revision)) continue;
        if (parse_int(buf, "<min_passwd_length>", min_passwd_length)) continue;
        if (parse_bool(buf, "account_manager", account_manager)) continue;
        if (parse_bool(buf, "uses_username", uses_username)) continue;
        if (parse_bool(buf, "account_creation_disabled", account_creation_disabled)) continue;
        if (parse_bool(buf, "client_account_creation_disabled", client_account_creation_disabled)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
        if (match_tag(buf, "<terms_of_use>")) {
            while (in.fgets(buf, 256)) {
                if (match_tag(buf, "</terms_of_use>")) break;
                terms_of_use += buf;
            }
            continue;
        }
        if (parse_int(buf, "<min_client_version>", min_client_version)) continue;
        if (parse_bool(buf, "web_stopped", web_stopped)) continue;
        if (parse_bool(buf, "sched_stopped", sched_stopped)) continue;
        if (parse_str(buf, "platform_name", msg)) {
            platforms.push_back(msg);
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void PROJECT_CONFIG::clear() {
    error_num = 0;
    name.clear();
    master_url.clear();
    error_msg.clear();
    terms_of_use.clear();
    min_passwd_length = 6;
    account_manager = false;
    uses_username = false;
    account_creation_disabled = false;
    client_account_creation_disabled = false;
    platforms.clear();
    sched_stopped = false;
    web_stopped = false;
    min_client_version = 0;
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
    team_name.clear();
}

ACCOUNT_OUT::ACCOUNT_OUT() {
    clear();
}

ACCOUNT_OUT::~ACCOUNT_OUT() {
    clear();
}

int ACCOUNT_OUT::parse(MIOFILE& in) {
    char buf[256];
    clear();
    while (in.fgets(buf, 256)) {
        if (parse_int(buf, "<error_num>", error_num)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
        if (parse_str(buf, "<authenticator>", authenticator)) continue;
    }
    return 0;
}

void ACCOUNT_OUT::clear() {
    error_num = 0;
	error_msg = "";
    authenticator.clear();
}

CC_STATUS::CC_STATUS() {
    clear();
}

CC_STATUS::~CC_STATUS() {
    clear();
}

int CC_STATUS::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</cc_status>")) return 0; 
        if (parse_int(buf, "<network_status>", network_status)) continue;
        if (parse_bool(buf, "ams_password_error", ams_password_error)) continue;
        if (parse_bool(buf, "manager_must_quit", manager_must_quit)) continue;
        if (parse_int(buf, "<task_suspend_reason>", task_suspend_reason)) continue;
        if (parse_int(buf, "<task_mode>", task_mode)) continue;
        if (parse_int(buf, "<task_mode_perm>", task_mode_perm)) continue;
		if (parse_double(buf, "<task_mode_delay>", task_mode_delay)) continue;
        if (parse_int(buf, "<gpu_suspend_reason>", gpu_suspend_reason)) continue;
        if (parse_int(buf, "<gpu_mode>", gpu_mode)) continue;
        if (parse_int(buf, "<gpu_mode_perm>", gpu_mode_perm)) continue;
		if (parse_double(buf, "<gpu_mode_delay>", gpu_mode_delay)) continue;
        if (parse_int(buf, "<network_suspend_reason>", network_suspend_reason)) continue;
        if (parse_int(buf, "<network_mode>", network_mode)) continue;
        if (parse_int(buf, "<network_mode_perm>", network_mode_perm)) continue;
		if (parse_double(buf, "<network_mode_delay>", network_mode_delay)) continue;
        if (parse_bool(buf, "disallow_attach", disallow_attach)) continue;
        if (parse_bool(buf, "simple_gui_only", simple_gui_only)) continue;
    }
    return ERR_XML_PARSE;
}

void CC_STATUS::clear() {
    network_status = -1;
    ams_password_error = false;
    manager_must_quit = false;
    task_suspend_reason = -1;
    task_mode = -1;
    task_mode_perm = -1;
	task_mode_delay = 0;
    network_suspend_reason = -1;
    network_mode = -1;
    network_mode_perm = -1;
	network_mode_delay = 0;
    gpu_suspend_reason = -1;
    gpu_mode = -1;
    gpu_mode_perm = -1;
	gpu_mode_delay = 0;
    disallow_attach = false;
    simple_gui_only = false;
}

/////////// END OF PARSING FUNCTIONS.  RPCS START HERE ////////////////

int RPC_CLIENT::exchange_versions(VERSION_INFO& server) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<exchange_versions>\n"
        "   <major>%d</major>\n"
        "   <minor>%d</minor>\n"
        "   <release>%d</release>\n"
        "</exchange_versions>\n",
        BOINC_MAJOR_VERSION,
        BOINC_MINOR_VERSION,
        BOINC_RELEASE
    );

    retval = rpc.do_rpc(buf);
    if (!retval) {
        memset(&server, 0, sizeof(server));
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "</server_version>")) break;
            else if (parse_int(buf, "<major>", server.major)) continue;
            else if (parse_int(buf, "<minor>", server.minor)) continue;
            else if (parse_int(buf, "<release>", server.release)) continue;
        }
    }
    return retval;
}

int RPC_CLIENT::get_state(CC_STATE& state) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    PROJECT* project = NULL;
    RPC rpc(this);
    string platform;

    state.clear();

    retval = rpc.do_rpc("<get_state/>\n");
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "<unauthorized")) {
                retval = ERR_AUTHENTICATOR;
                break;
            }
            if (match_tag(buf, "</client_state>")) break;

            if (parse_bool(buf, "executing_as_daemon", state.executing_as_daemon)) continue;
            if (parse_bool(buf, "have_cuda", state.have_cuda)) continue;
            if (parse_bool(buf, "have_ati", state.have_ati)) continue;
            if (match_tag(buf, "<project>")) {
                project = new PROJECT();
                project->parse(rpc.fin);
                state.projects.push_back(project);
                continue;
            }
            if (match_tag(buf, "<app>")) {
                APP* app = new APP();
                app->parse(rpc.fin);
                app->project = project;
                state.apps.push_back(app);
                continue;
            }
            if (match_tag(buf, "<app_version>")) {
                APP_VERSION* app_version = new APP_VERSION();
                app_version->parse(rpc.fin);
                app_version->project = project;
                app_version->app = state.lookup_app(project, app_version->app_name);
                state.app_versions.push_back(app_version);
                continue;
            }
            if (match_tag(buf, "<workunit>")) {
                WORKUNIT* wu = new WORKUNIT();
                wu->parse(rpc.fin);
                wu->project = project;
                wu->app = state.lookup_app(project, wu->app_name);
                state.wus.push_back(wu);
                continue;
            }
            if (match_tag(buf, "<result>")) {
                RESULT* result = new RESULT();
                result->parse(rpc.fin);
                result->project = project;
                result->wup = state.lookup_wu(project, result->wu_name);
                result->app = result->wup->app;
                APP_VERSION* avp;
                if (result->version_num) {
                    avp = state.lookup_app_version(
                        project, result->app, result->version_num,
                        result->plan_class
                    );
                } else {
                    avp = state.lookup_app_version_old(
                        project, result->app, result->wup->version_num
                    );
                }
                result->avp = avp;
                state.results.push_back(result);
                continue;
            }
            if (match_tag(buf, "<global_preferences>")) {
                bool flag = false;
                GLOBAL_PREFS_MASK mask;
                XML_PARSER xp(&rpc.fin);
                state.global_prefs.parse(xp, "", flag, mask);
                continue;
            }
            if (parse_str(buf, "<platform>", platform)) {
                state.platforms.push_back(platform);
            }
        }
    }
    return retval;
}

int RPC_CLIENT::get_results(RESULTS& t, bool active_only) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    t.clear();

    sprintf(buf, "<get_results>\n<active_only>%d</active_only>\n</get_results>\n",
        active_only?1:0
    );
    retval = rpc.do_rpc(buf);
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "</results>")) break;
            else if (match_tag(buf, "<result>")) {
                RESULT* rp = new RESULT();
                rp->parse(rpc.fin);
                t.results.push_back(rp);
                continue;
            }
        }
    }
    return retval;
}

int RPC_CLIENT::get_file_transfers(FILE_TRANSFERS& t) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    t.clear();

    retval = rpc.do_rpc("<get_file_transfers/>\n");
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "</file_transfers>")) break;
            else if (match_tag(buf, "<file_transfer>")) {
                FILE_TRANSFER* fip = new FILE_TRANSFER();
                fip->parse(rpc.fin);
                t.file_transfers.push_back(fip);
                continue;
            }
        }
    }
    return retval;
}

int RPC_CLIENT::get_simple_gui_info(SIMPLE_GUI_INFO& info) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    info.projects.clear();
    info.results.clear();

    retval = rpc.do_rpc("<get_simple_gui_info/>\n");
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "</simple_gui_info>")) break;
            else if (match_tag(buf, "<project>")) {
                PROJECT* project = new PROJECT();
                project->parse(rpc.fin);
                info.projects.push_back(project);
                continue;
            }
            else if (match_tag(buf, "<result>")) {
                RESULT* result = new RESULT();
                result->parse(rpc.fin);
                info.results.push_back(result);
                continue;
            }
        }
    }
    return retval;
}


// creates new array of PROJECTs
//
int RPC_CLIENT::get_project_status(PROJECTS& p) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    p.clear();

    retval = rpc.do_rpc("<get_project_status/>\n");
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "</projects>")) break;
            else if (match_tag(buf, "<project>")) {
                PROJECT* project = new PROJECT();
                project->parse(rpc.fin);
                p.projects.push_back(project);
                continue;
            }
        }
    }
    return retval;
}

int RPC_CLIENT::get_all_projects_list(ALL_PROJECTS_LIST& pl) {
    int retval = 0;
    SET_LOCALE sl;
    char tag[256];
    bool is_tag;
    MIOFILE mf;
    PROJECT_LIST_ENTRY* project;
    AM_LIST_ENTRY* am;
    RPC rpc(this);

    pl.clear();

    retval = rpc.do_rpc("<get_all_projects_list/>\n");
    if (retval) return retval;
    XML_PARSER xp(&rpc.fin);
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!strcmp(tag, "/projects")) break;
        else if (!strcmp(tag, "project")) {
            project = new PROJECT_LIST_ENTRY();
            retval = project->parse(xp);
            if (!retval) {
                pl.projects.push_back(project);
            } else {
                delete project;
            }
            continue;
        } else if (!strcmp(tag, "account_manager")) {
            am = new AM_LIST_ENTRY();
            retval = am->parse(xp);
            if (!retval) {
                pl.account_managers.push_back(am);
            } else {
                delete am;
            }
            continue;
        }
    }

    pl.shuffle();

    return 0;
}

int RPC_CLIENT::get_disk_usage(DISK_USAGE& du) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    du.clear();

    retval = rpc.do_rpc("<get_disk_usage/>\n");
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "</disk_usage_summary>")) break;
            if (match_tag(buf, "<project>")) {
                PROJECT* project = new PROJECT();
                project->parse(rpc.fin);
                du.projects.push_back(project);
                continue;
            }
            if (parse_double(buf, "<d_total>", du.d_total)) continue;
            if (parse_double(buf, "<d_free>", du.d_free)) continue;
            if (parse_double(buf, "<d_boinc>", du.d_boinc)) continue;
            if (parse_double(buf, "<d_allowed>", du.d_allowed)) continue;
        }
    }
    return retval;
}

int DAILY_STATS::parse(MIOFILE& in) {
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</daily_statistics>")) return 0;
        if (parse_double(buf, "<day>", day)) continue;
        if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
    }
    return ERR_XML_PARSE;
}

int RPC_CLIENT::get_statistics(PROJECTS& p) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    retval = rpc.do_rpc("<get_statistics/>\n");
    if (!retval) {
        p.clear();

        while (rpc.fin.fgets(buf, 256)) {
            if (retval) break;
            if (match_tag(buf, "</statistics>")) break;
            if (match_tag(buf, "<project_statistics>")) {
                PROJECT* project = new PROJECT();
                p.projects.push_back(project);

                while (rpc.fin.fgets(buf, 256)) {
                    if (match_tag(buf, "</project_statistics>")) break;
                    if (parse_str(buf, "<master_url>", p.projects.back()->master_url, sizeof(project->master_url))) continue;
                    if (match_tag(buf, "<daily_statistics>")) {
                        DAILY_STATS ds;
                        retval = ds.parse(rpc.fin);
                        if (retval) break;
                        p.projects.back()->statistics.push_back(ds);
                        continue;
                    }
                }
                continue;
            }
        }
    }
    return retval;
}

int RPC_CLIENT::get_cc_status(CC_STATUS& status) {
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    int retval = rpc.do_rpc("<get_cc_status/>\n");
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "<cc_status>")) {
                retval = status.parse(rpc.fin);
                if (retval) break;
            }
        }
    }
    return retval;
}

int RPC_CLIENT::network_available() {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<network_available/>\n");

    return retval;
}

void DISPLAY_INFO::print_str(char* p) {
    char buf[768];
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
    const char* project_url, const char* result_name, int graphics_mode,
    DISPLAY_INFO& di
) {
    int retval;
    SET_LOCALE sl;
    char buf[1536];
    RPC rpc(this);

    sprintf(buf, 
        "<result_show_graphics>\n"
        "   <project_url>%s</project_url>\n"
        "   <result_name>%s</result_name>\n"
        "%s%s%s",
        project_url,
        result_name,
        graphics_mode == MODE_HIDE_GRAPHICS?"   <hide/>\n":"",
        graphics_mode == MODE_WINDOW       ?"   <window/>\n":"",
        graphics_mode == MODE_FULLSCREEN   ?"   <full_screen/>\n":""
    );
    di.print_str(buf);
    strcat(buf, "</result_show_graphics>\n");

    retval = rpc.do_rpc(buf);
    return retval;
}

int RPC_CLIENT::project_op(PROJECT& project, const char* op) {
    int retval;
    SET_LOCALE sl;
    char buf[512];
    const char *tag;
    RPC rpc(this);

    if (!strcmp(op, "reset")) {
        tag = "project_reset";
    } else if (!strcmp(op, "detach")) {
        tag = "project_detach";
    } else if (!strcmp(op, "update")) {
        tag = "project_update";
    } else if (!strcmp(op, "suspend")) {
        tag = "project_suspend";
        project.suspended_via_gui = true;
    } else if (!strcmp(op, "resume")) {
        tag = "project_resume";
        project.suspended_via_gui = false;
    } else if (!strcmp(op, "allowmorework")) {
        tag = "project_allowmorework";
        project.dont_request_more_work = false;
    } else if (!strcmp(op, "nomorework")) {
         tag = "project_nomorework";
        project.dont_request_more_work = true;
    } else if (!strcmp(op, "detach_when_done")) {
         tag = "project_detach_when_done";
    } else if (!strcmp(op, "dont_detach_when_done")) {
         tag = "project_dont_detach_when_done";
    } else {
        return -1;
    }
    sprintf(buf,
        "<%s>\n"
        "  <project_url>%s</project_url>\n"
        "</%s>\n",
        tag,
        project.master_url,
        tag
    );
    retval = rpc.do_rpc(buf);
    if (!retval) {
        retval = rpc.parse_reply();
    }

    return retval;
}

int RPC_CLIENT::project_attach_from_file() {
    int retval;
    SET_LOCALE sl;
    char buf[768];
    RPC rpc(this);

    sprintf(buf,
        "<project_attach>\n"
        "  <use_config_file/>\n"
        "</project_attach>\n"
    );
    retval = rpc.do_rpc(buf);
    if (!retval) {
        retval = rpc.parse_reply();
    }
    return retval;
}

int RPC_CLIENT::project_attach(const char* url, const char* auth, const char* name) {
    int retval;
    SET_LOCALE sl;
    char buf[768];
    RPC rpc(this);

    sprintf(buf,
        "<project_attach>\n"
        "  <project_url>%s</project_url>\n"
        "  <authenticator>%s</authenticator>\n"
        "  <project_name>%s</project_name>\n"
        "</project_attach>\n",
        url, auth, name
    );

    retval = rpc.do_rpc(buf);
    if (!retval) {
        retval = rpc.parse_reply();
    }
    return retval;
}

int RPC_CLIENT::project_attach_poll(PROJECT_ATTACH_REPLY& reply) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<project_attach_poll/>\n");
    if (!retval) {
        retval = reply.parse(rpc.fin);
    }
    return retval;
}

const char* RPC_CLIENT::mode_name(int mode) {
    const char* p = NULL;
    switch (mode) {
    case RUN_MODE_ALWAYS: p="<always/>"; break;
    case RUN_MODE_NEVER: p="<never/>"; break;
    case RUN_MODE_AUTO: p="<auto/>"; break;
    case RUN_MODE_RESTORE: p="<restore/>"; break;
    }
    return p;
}

int RPC_CLIENT::set_run_mode(int mode, double duration) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    sprintf(buf, 
        "<set_run_mode>\n"
        "%s\n"
        "  <duration>%f</duration>\n"
        "</set_run_mode>\n",
        mode_name(mode), duration
    );

    retval = rpc.do_rpc(buf);
    return retval;
}

int RPC_CLIENT::set_gpu_mode(int mode, double duration) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    sprintf(buf, 
        "<set_gpu_mode>\n"
        "%s\n"
        "  <duration>%f</duration>\n"
        "</set_gpu_mode>\n",
        mode_name(mode), duration
    );

    retval = rpc.do_rpc(buf);
    return retval;
}

int RPC_CLIENT::set_network_mode(int mode, double duration) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<set_network_mode>\n"
        "%s\n"
        "  <duration>%f</duration>\n"
        "</set_network_mode>\n",
        mode_name(mode), duration
    );
    retval = rpc.do_rpc(buf);
    return retval;
}

int RPC_CLIENT::get_screensaver_tasks(int& suspend_reason, RESULTS& t) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    t.clear();

    retval = rpc.do_rpc("<get_screensaver_tasks/>\n");
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "</get_screensaver_tasks>")) break;
            if (parse_int(buf, "<suspend_reason>", suspend_reason)) continue;
            if (match_tag(buf, "<result>")) {
                RESULT* rp = new RESULT();
                rp->parse(rpc.fin);
                t.results.push_back(rp);
                continue;
            }
        }
    }
    return retval;
}

int RPC_CLIENT::run_benchmarks() {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<run_benchmarks/>\n");
    return retval;
}

int RPC_CLIENT::set_proxy_settings(GR_PROXY_INFO& pi) {
    int retval;
    SET_LOCALE sl;
    char buf[1792];
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
		"        <no_proxy>%s</no_proxy\n"
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
        pi.socks5_user_passwd.c_str(),
		pi.noproxy_hosts.c_str()
    );
    retval = rpc.do_rpc(buf);
    return retval;
}

int RPC_CLIENT::get_proxy_settings(GR_PROXY_INFO& p) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<get_proxy_settings/>");
    if (!retval) {
        retval = p.parse(rpc.fin);
    }
    return retval;
}

int RPC_CLIENT::get_message_count(int& seqno) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<get_message_count/>\n"
    );
    retval = rpc.do_rpc(buf);
    if (retval) return retval;
    while (rpc.fin.fgets(buf, 256)) {
        if (parse_int(buf, "<seqno>", seqno)) {
            return 0;
        }
    }
    return ERR_XML_PARSE;
}

int RPC_CLIENT::get_messages(int seqno, MESSAGES& msgs) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<get_messages>\n"
        "  <seqno>%d</seqno>\n"
        "</get_messages>\n",
        seqno
    );

    retval = rpc.do_rpc(buf);
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            if (match_tag(buf, "</msgs>")) {
                return 0;
            }
            if (match_tag(buf, "<msg>")) {
                MESSAGE* message = new MESSAGE();
                message->parse(rpc.fin);
                msgs.messages.push_back(message);
                continue;
            }
            if (match_tag(buf, "<boinc_gui_rpc_reply>")) continue;
            if (match_tag(buf, "<msgs>")) continue;
            fprintf(stderr, "bad tag %s\n", buf);
        }
    }
    return retval;
}

int RPC_CLIENT::file_transfer_op(FILE_TRANSFER& ft, const char* op) {
    int retval;
    SET_LOCALE sl;
    char buf[768];
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
    retval = rpc.do_rpc(buf);
    return retval;
}

int RPC_CLIENT::result_op(RESULT& result, const char* op) {
    int retval;
    SET_LOCALE sl;
    char buf[768];
    const char *tag;
    RPC rpc(this);

    if (!strcmp(op, "abort")) {
        tag = "abort_result";
    } else if (!strcmp(op, "suspend")) {
        tag = "suspend_result";
        result.suspended_via_gui = true;
    } else if (!strcmp(op, "resume")) {
        tag = "resume_result";
        result.suspended_via_gui = false;
    } else {
        return -1;
    }

    sprintf(buf,
        "<%s>\n"
        "   <project_url>%s</project_url>\n"
        "   <name>%s</name>\n"
        "</%s>\n",
        tag,
        result.project_url,
        result.name,
        tag
    );
    retval = rpc.do_rpc(buf);
    return retval;
}

int RPC_CLIENT::get_host_info(HOST_INFO& h) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<get_host_info/>");
    if (!retval) {
        retval = h.parse(rpc.fin);
    }
    return retval;
}


int RPC_CLIENT::quit() {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<quit/>\n");
    return retval;
}

int RPC_CLIENT::acct_mgr_rpc(const char* url, const char* name, const char* password, bool use_config_file) {
    int retval;
    SET_LOCALE sl;
    char buf[1024];
    RPC rpc(this);

    if (use_config_file) {
        sprintf(buf,
            "<acct_mgr_rpc>\n"
            "  <use_config_file/>\n"
            "</acct_mgr_rpc>\n"
        );
    } else {
        sprintf(buf,
            "<acct_mgr_rpc>\n"
            "  <url>%s</url>\n"
            "  <name>%s</name>\n"
            "  <password>%s</password>\n"
            "</acct_mgr_rpc>\n",
            url, name, password
        );
    }
    retval = rpc.do_rpc(buf);
    return retval;
}

int RPC_CLIENT::acct_mgr_rpc_poll(ACCT_MGR_RPC_REPLY& r) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<acct_mgr_rpc_poll/>\n");
    if (!retval) {
        retval = r.parse(rpc.fin);
    }
    return retval;
}

int RPC_CLIENT::acct_mgr_info(ACCT_MGR_INFO& ami) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<acct_mgr_info/>\n");
    if (!retval) {
        retval = ami.parse(rpc.fin);
    }
    return retval;
}

int RPC_CLIENT::get_project_init_status(PROJECT_INIT_STATUS& pis) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<get_project_init_status/>\n");
    if (!retval) {
        retval = pis.parse(rpc.fin);
    }
    return retval;
}


int RPC_CLIENT::get_project_config(std::string url) {
    int retval;
    char buf[512];
    SET_LOCALE sl;
    RPC rpc(this);

    sprintf(buf,
        "<get_project_config>\n"
        "   <url>%s</url>\n"
        "</get_project_config>\n",
        url.c_str()
    );

    retval =  rpc.do_rpc(buf);
    if (!retval) {
        retval = rpc.parse_reply();
    }

    return retval;
}

int RPC_CLIENT::get_project_config_poll(PROJECT_CONFIG& pc) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<get_project_config_poll/>\n");
    if (!retval) {
        retval = pc.parse(rpc.fin);
    }
    return retval;
}

static string get_passwd_hash(string passwd, string email_addr) {
    return md5_string(passwd+email_addr);

}
int RPC_CLIENT::lookup_account(ACCOUNT_IN& ai) {
    int retval;
    SET_LOCALE sl;
    char buf[1024];
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
    if (!retval) {
        retval = rpc.parse_reply();
    }
    return retval;
}

int RPC_CLIENT::lookup_account_poll(ACCOUNT_OUT& ao) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<lookup_account_poll/>\n");
    if (!retval) {
        retval = ao.parse(rpc.fin);
    }
    return retval;
}

int RPC_CLIENT::create_account(ACCOUNT_IN& ai) {
    int retval;
    SET_LOCALE sl;
    char buf[1280];
    RPC rpc(this);

    downcase_string(ai.email_addr);
    string passwd_hash = get_passwd_hash(ai.passwd, ai.email_addr);
    sprintf(buf,
        "<create_account>\n"
        "   <url>%s</url>\n"
        "   <email_addr>%s</email_addr>\n"
        "   <passwd_hash>%s</passwd_hash>\n"
        "   <user_name>%s</user_name>\n"
        "   <team_name>%s</team_name>\n"
        "</create_account>\n",
        ai.url.c_str(),
        ai.email_addr.c_str(),
        passwd_hash.c_str(),
        ai.user_name.c_str(),
        ai.team_name.c_str()
    );

    retval =  rpc.do_rpc(buf);
    if (!retval) {
        retval = rpc.parse_reply();
    }
    return retval;
}

int RPC_CLIENT::create_account_poll(ACCOUNT_OUT& ao) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<create_account_poll/>\n");
    if (!retval) {
        retval = ao.parse(rpc.fin);
    }
    return retval;
}

int RPC_CLIENT::get_newer_version(std::string& version, std::string& version_download_url) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    version = "";
    retval = rpc.do_rpc("<get_newer_version/>\n");
    if (!retval) {
        while (rpc.fin.fgets(buf, 256)) {
            parse_str(buf, "<newer_version>", version);
            parse_str(buf, "<download_url>", version_download_url);
        }
    }
    return retval;
}

int RPC_CLIENT::read_global_prefs_override() {
    SET_LOCALE sl;
    RPC rpc(this);
    return rpc.do_rpc("<read_global_prefs_override/>");
}

int RPC_CLIENT::get_global_prefs_file(string& s) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);
    char buf[1024];
    bool found = false;
    bool in_prefs = false;

    s = "";
    retval = rpc.do_rpc("<get_global_prefs_file/>");
    if (retval) return retval;
    while (rpc.fin.fgets(buf, 256)) {
        if (in_prefs) {
            s += buf;
            if (match_tag(buf, "</global_preferences>")) {
                in_prefs = false;
            }
        } else {
            if (match_tag(buf, "<global_preferences>")) {
                s += buf;
                in_prefs = true;
                found = true;
            }
        }
    }
    if (!found) return ERR_NOT_FOUND;
    return 0;
}

int RPC_CLIENT::get_global_prefs_working(string& s) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);
    char buf[1024];
    bool found = false;
    bool in_prefs = false;

    s = "";
    retval = rpc.do_rpc("<get_global_prefs_working/>");
    if (retval) return retval;
    while (rpc.fin.fgets(buf, 256)) {
        if (in_prefs) {
            s += buf;
            if (match_tag(buf, "</global_preferences>")) {
                in_prefs = false;
            }
        } else {
            if (match_tag(buf, "<global_preferences>")) {
                s += buf;
                in_prefs = true;
                found = true;
            }
        }
    }
    if (!found) return ERR_NOT_FOUND;
    return 0;
}


int RPC_CLIENT::get_global_prefs_working_struct(GLOBAL_PREFS& prefs, GLOBAL_PREFS_MASK& mask) {
    int retval;
    SET_LOCALE sl;
    string s;
    MIOFILE mf;
    bool found_venue;

    retval = get_global_prefs_working(s);
    if (retval) return retval;
    mf.init_buf_read(s.c_str());
    XML_PARSER xp(&mf);
    prefs.parse(xp, "", found_venue, mask);

    if (!mask.are_prefs_set()) {
        return ERR_FILE_NOT_FOUND;
    }
    return 0;
}

int RPC_CLIENT::get_global_prefs_override(string& s) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);
    char buf[1024];
    bool found = false;
    bool in_prefs = false;

    s = "";
    retval = rpc.do_rpc("<get_global_prefs_override/>");
    if (retval) return retval;
    while (rpc.fin.fgets(buf, 256)) {
        if (in_prefs) {
            s += buf;
            if (match_tag(buf, "</global_preferences>")) {
                in_prefs = false;
            }
        } else {
            if (match_tag(buf, "<global_preferences>")) {
                s += buf;
                in_prefs = true;
                found = true;
            }
        }
    }
    if (!found) return ERR_NOT_FOUND;
    return 0;
}

int RPC_CLIENT::set_global_prefs_override(string& s) {
    int retval;
    RPC rpc(this);
    char buf[64000];

    sprintf(buf,
        "<set_global_prefs_override>\n"
        "%s\n"
        "</set_global_prefs_override>\n",
        s.c_str()
    );
    retval = rpc.do_rpc(buf);
    return retval;
}

int RPC_CLIENT::get_global_prefs_override_struct(GLOBAL_PREFS& prefs, GLOBAL_PREFS_MASK& mask) {
    int retval;
    SET_LOCALE sl;
    string s;
    MIOFILE mf;
    bool found_venue;

    retval = get_global_prefs_override(s);
    if (retval) return retval;
    mf.init_buf_read(s.c_str());
    XML_PARSER xp(&mf);
    prefs.parse(xp, "", found_venue, mask);

    if (!mask.are_prefs_set()) {
        return ERR_FILE_NOT_FOUND;
    }
    return 0;
}

int RPC_CLIENT::set_global_prefs_override_struct(GLOBAL_PREFS& prefs, GLOBAL_PREFS_MASK& mask) {
    SET_LOCALE sl;
    char buf[64000];
    MIOFILE mf;
    string s;

    mf.init_buf_write(buf, sizeof(buf));
    prefs.write_subset(mf, mask);
    s = buf;
    return set_global_prefs_override(s);
}

int RPC_CLIENT::read_cc_config() {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<read_cc_config/>");
    return retval;
}

int RPC_CLIENT::set_debts(vector<PROJECT> projects) {
    int retval;
    SET_LOCALE sl;
    char buf[1024];
    RPC rpc(this);
    string s;

    s = "<set_debts>\n";
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT& p = projects[i];
        sprintf(buf,
            "    <project>\n"
            "        <master_url>%s</master_url>\n"
            "        <short_term_debt>%f</short_term_debt>\n"
            "        <long_term_debt>%f</long_term_debt>\n"
            "    </project>\n",
            p.master_url,
            p.cpu_short_term_debt,
            p.cpu_long_term_debt
        );
        s += string(buf);
    }
    s += "</set_debts>\n";
    retval = rpc.do_rpc(s.c_str());
    return retval;
}

static int parse_notices(MIOFILE& fin, NOTICES& notices) {
    XML_PARSER xp(&fin);
    char tag[256];
    bool is_tag;
    int retval;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "notice")) {
            NOTICE* np = new NOTICE();
            retval = np->parse(xp);
            if (!retval) {
                if (np->seqno == -1) {
                    notices.notices.clear();
                } else {
                    notices.notices.insert(notices.notices.begin(), np);
                }
            } else {
                delete np;
            }
        }
    }
    return 0;
}

int RPC_CLIENT::get_notices(int seqno, NOTICES& notices) {
    SET_LOCALE sl;
    char buf[1024];
    RPC rpc(this);
    int retval;

    sprintf(buf,
        "<get_notices>\n"
        "   <seqno>%d</seqno>\n"
        "</get_notices>\n",
        seqno
    );
    retval = rpc.do_rpc(buf);
    if (retval) return retval;
    return parse_notices(rpc.fin, notices);
}

int RPC_CLIENT::get_notices_public(int seqno, NOTICES& notices) {
    SET_LOCALE sl;
    char buf[1024];
    RPC rpc(this);
    int retval;

    sprintf(buf,
        "<get_notices_public>\n"
        "   <seqno>%d</seqno>\n"
        "</get_notices_public>\n",
        seqno
    );
    retval = rpc.do_rpc(buf);
    if (retval) return retval;
    return parse_notices(rpc.fin, notices);
}


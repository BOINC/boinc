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
#include <algorithm>
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

int TIME_STATS::parse(XML_PARSER& xp) {
    memset(this, 0, sizeof(TIME_STATS));
    while (!xp.get_tag()) {
        if (xp.match_tag("/time_stats")) return 0;
        if (xp.parse_double("now", now)) continue;
        if (xp.parse_double("on_frac", on_frac)) continue;
        if (xp.parse_double("connected_frac", connected_frac)) continue;
        if (xp.parse_double("cpu_and_network_available_frac", cpu_and_network_available_frac)) continue;
        if (xp.parse_double("active_frac", active_frac)) continue;
        if (xp.parse_double("gpu_active_frac", gpu_active_frac)) continue;
        if (xp.parse_double("client_start_time", client_start_time)) continue;
        if (xp.parse_double("previous_uptime", previous_uptime)) continue;
    }
    return ERR_XML_PARSE;
}

int DAILY_XFER::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/dx")) return 0;
        if (xp.parse_int("when", when)) continue;
        if (xp.parse_double("up", up)) continue;
        if (xp.parse_double("down", down)) continue;
    }
    return ERR_XML_PARSE;
}

int DAILY_XFER_HISTORY::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("dx")) {
            DAILY_XFER dx;
            int retval = dx.parse(xp);
            if (!retval) {
                daily_xfers.push_back(dx);
            }
        }
    }
    return 0;
}

int GUI_URL::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/gui_url")) return 0;
        if (xp.match_tag("/gui_urls")) break;
        if (xp.parse_string("name", name)) continue;
        if (xp.parse_string("description", description)) continue;
        if (xp.parse_string("url", url)) continue;
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
    string platform;

    while (!xp.get_tag()) {
        if (xp.match_tag("/project")) return 0;
        if (xp.parse_string("name", name)) continue;
        if (xp.parse_string("url", url)) {
            continue;
        }
        if (xp.parse_string("general_area", general_area)) continue;
        if (xp.parse_string("specific_area", specific_area)) continue;
        if (xp.parse_string("description", description)) {
            continue;
        }
        if (xp.parse_string("home", home)) continue;
        if (xp.parse_string("image", image)) continue;
        if (xp.match_tag("platforms")) {
            while (!xp.get_tag()) {
                if (xp.match_tag("/platforms")) break;
                if (xp.parse_string("name", platform)) {
                    platforms.push_back(platform);
                }
            }
        }
        xp.skip_unexpected(false, "");
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
    while (!xp.get_tag()) {
        if (xp.match_tag("/account_manager")) return 0;
        if (xp.parse_string("name", name)) continue;
        if (xp.parse_string("url", url)) continue;
        if (xp.parse_string("description", description)) continue;
        if (xp.parse_string("image", image)) continue;
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
#ifdef _WIN32
    return _stricmp(a->name.c_str(), b->name.c_str()) < 0;
#else
    return strcasecmp(a->name.c_str(), b->name.c_str()) < 0;
#endif
}

bool compare_am_list_entry(const AM_LIST_ENTRY* a, const AM_LIST_ENTRY* b) 
{
#ifdef _WIN32
    return _stricmp(a->name.c_str(), b->name.c_str()) < 0;
#else
    return strcasecmp(a->name.c_str(), b->name.c_str()) < 0;
#endif
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

int PROJECT::parse(XML_PARSER& xp) {
    int retval;
    char buf[256];

    while (!xp.get_tag()) {
        if (xp.match_tag("/project")) return 0;
        if (xp.parse_str("master_url", master_url, sizeof(master_url))) continue;
        if (xp.parse_double("resource_share", resource_share)) continue;
        if (xp.parse_string("project_name", project_name)) continue;
        if (xp.parse_string("user_name", user_name)) {
            xml_unescape(user_name);
            continue;
        }
        if (xp.parse_string("team_name", team_name)) {
            xml_unescape(team_name);
            continue;
        }
        if (xp.parse_int("hostid", hostid)) continue;
        if (xp.parse_double("user_total_credit", user_total_credit)) continue;
        if (xp.parse_double("user_expavg_credit", user_expavg_credit)) continue;
        if (xp.parse_double("host_total_credit", host_total_credit)) continue;
        if (xp.parse_double("host_expavg_credit", host_expavg_credit)) continue;
        if (xp.parse_double("disk_usage", disk_usage)) continue;
        if (xp.parse_int("nrpc_failures", nrpc_failures)) continue;
        if (xp.parse_int("master_fetch_failures", master_fetch_failures)) continue;
        if (xp.parse_double("min_rpc_time", min_rpc_time)) continue;
        if (xp.parse_double("download_backoff", download_backoff)) continue;
        if (xp.parse_double("upload_backoff", upload_backoff)) continue;
        if (xp.parse_double("sched_priority", sched_priority)) continue;

        // resource-specific stuff, old format
        //
        if (xp.parse_double("cpu_backoff_time", rsc_desc_cpu.backoff_time)) continue;
        if (xp.parse_double("cpu_backoff_interval", rsc_desc_cpu.backoff_interval)) continue;
        if (xp.parse_double("cuda_backoff_time", rsc_desc_nvidia.backoff_time)) continue;
        if (xp.parse_double("cuda_backoff_interval", rsc_desc_nvidia.backoff_interval)) continue;
        if (xp.parse_double("ati_backoff_time", rsc_desc_ati.backoff_time)) continue;
        if (xp.parse_double("ati_backoff_interval", rsc_desc_ati.backoff_interval)) continue;
        if (xp.parse_double("last_rpc_time", last_rpc_time)) continue;
        if (xp.parse_bool("no_cpu_pref", rsc_desc_cpu.no_rsc_pref)) continue;
        if (xp.parse_bool("no_cuda_pref", rsc_desc_cpu.no_rsc_pref)) continue;

        // resource-specific stuff, new format
        //
        if (xp.match_tag("rsc_backoff_time")) {
            double value = 0;
            while (!xp.get_tag()) {
                if (xp.match_tag("/rsc_backoff_time")) {
                    if (!strcmp(buf, "CPU")) {
                        rsc_desc_cpu.backoff_time = value;
                    } else if (!strcmp(buf, "NVIDIA")) {
                        rsc_desc_nvidia.backoff_time = value;
                    } else if (!strcmp(buf, "ATI")) {
                        rsc_desc_ati.backoff_time = value;
                    }
                    break;
                }
                if (xp.parse_str("name", buf, sizeof(buf))) continue;
                if (xp.parse_double("value", value)) continue;
            }
            continue;
        }
        if (xp.match_tag("rsc_backoff_interval")) {
            double value = 0;
            while (!xp.get_tag()) {
                if (xp.match_tag("/rsc_backoff_interval")) {
                    if (!strcmp(buf, "CPU")) {
                        rsc_desc_cpu.backoff_interval = value;
                    } else if (!strcmp(buf, "NVIDIA")) {
                        rsc_desc_nvidia.backoff_interval = value;
                    } else if (!strcmp(buf, "ATI")) {
                        rsc_desc_ati.backoff_interval = value;
                    }
                    break;
                }
                if (xp.parse_str("name", buf, sizeof(buf))) continue;
                if (xp.parse_double("value", value)) continue;
            }
            continue;
        }
        if (xp.parse_str("no_rsc_ams", buf, sizeof(buf))) {
            if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_CPU))) {
                rsc_desc_cpu.no_rsc_ams = true;
            } else if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_NVIDIA_GPU))) {
                rsc_desc_nvidia.no_rsc_ams = true;
            } else if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_AMD_GPU))) {
                rsc_desc_ati.no_rsc_ams = true;
            }
            continue;
        }
        if (xp.parse_str("no_rsc_apps", buf, sizeof(buf))) {
            if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_CPU))) {
                rsc_desc_cpu.no_rsc_apps = true;
            } else if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_NVIDIA_GPU))) {
                rsc_desc_nvidia.no_rsc_apps = true;
            } else if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_AMD_GPU))) {
                rsc_desc_ati.no_rsc_apps = true;
            }
            continue;
        }
        if (xp.parse_str("no_rsc_pref", buf, sizeof(buf))) {
            if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_CPU))) {
                rsc_desc_cpu.no_rsc_pref = true;
            } else if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_NVIDIA_GPU))) {
                rsc_desc_nvidia.no_rsc_pref = true;
            } else if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_AMD_GPU))) {
                rsc_desc_ati.no_rsc_pref = true;
            }
            continue;
        }
        if (xp.parse_str("no_rsc_config", buf, sizeof(buf))) {
            if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_CPU))) {
                rsc_desc_cpu.no_rsc_config = true;
            } else if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_NVIDIA_GPU))) {
                rsc_desc_nvidia.no_rsc_config = true;
            } else if (!strcmp(buf, proc_type_name_xml(PROC_TYPE_AMD_GPU))) {
                rsc_desc_ati.no_rsc_config = true;
            }
            continue;
        }

        if (xp.parse_double("duration_correction_factor", duration_correction_factor)) continue;
        if (xp.parse_bool("anonymous_platform", anonymous_platform)) continue;
        if (xp.parse_bool("master_url_fetch_pending", master_url_fetch_pending)) continue;
        if (xp.parse_int("sched_rpc_pending", sched_rpc_pending)) continue;
        if (xp.parse_bool("non_cpu_intensive", non_cpu_intensive)) continue;
        if (xp.parse_bool("suspended_via_gui", suspended_via_gui)) continue;
        if (xp.parse_bool("dont_request_more_work", dont_request_more_work)) continue;
        if (xp.parse_bool("ended", ended)) continue;
        if (xp.parse_bool("scheduler_rpc_in_progress", scheduler_rpc_in_progress)) continue;
        if (xp.parse_bool("attached_via_acct_mgr", attached_via_acct_mgr)) continue;
        if (xp.parse_bool("detach_when_done", detach_when_done)) continue;
        if (xp.parse_bool("trickle_up_pending", trickle_up_pending)) continue;
        if (xp.match_tag("gui_urls")) {
            while (!xp.get_tag()) {
                if (xp.match_tag("/gui_urls")) break;
                if (xp.match_tag("gui_url")) {
                    GUI_URL gu;
                    retval = gu.parse(xp);
                    if (retval) break;
                    gui_urls.push_back(gu);
                    continue;
                }
            }
            continue;
        }
        if (xp.parse_double("project_files_downloaded_time", project_files_downloaded_time)) continue;
        if (xp.parse_bool("no_ati_pref", rsc_desc_cpu.no_rsc_pref)) continue;
        if (xp.parse_str("venue", venue, sizeof(venue))) continue;
    }
    return ERR_XML_PARSE;
}

void RSC_DESC::clear() {
    backoff_time = 0;
    backoff_interval = 0;
    no_rsc_ams = false;
    no_rsc_apps = false;
    no_rsc_pref = false;
    no_rsc_config = false;
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
    rsc_desc_cpu.clear();
    rsc_desc_nvidia.clear();
    rsc_desc_ati.clear();
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
    strcpy(venue, "");
}

APP::APP() {
    clear();
}

APP::~APP() {
    clear();
}

int APP::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/app")) return 0;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("user_friendly_name", user_friendly_name, sizeof(user_friendly_name))) continue;
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

int APP_VERSION::parse_coproc(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/coproc")) {
            return 0;
        }
        if (xp.parse_int("gpu_type", gpu_type)) continue;
        if (xp.parse_double("gpu_usage", gpu_usage)) continue;
    }
    return ERR_XML_PARSE;
}

int APP_VERSION::parse(XML_PARSER& xp) {
    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/app_version")) return 0;
        if (xp.parse_str("app_name", app_name, sizeof(app_name))) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) continue;
        if (xp.parse_str("platform", platform, sizeof(platform))) continue;
        if (xp.parse_double("avg_ncpus", avg_ncpus)) continue;
        if (xp.parse_double("gpu_ram", gpu_ram)) continue;
        if (xp.parse_double("flops", flops)) continue;
        if (xp.match_tag("coproc")) {
            parse_coproc(xp);
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void APP_VERSION::clear() {
    memset(this, 0, sizeof(*this));
}

WORKUNIT::WORKUNIT() {
    clear();
}

WORKUNIT::~WORKUNIT() {
    clear();
}

int WORKUNIT::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/workunit")) return 0;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("app_name", app_name, sizeof(app_name))) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.parse_double("rsc_fpops_est", rsc_fpops_est)) continue;
        if (xp.parse_double("rsc_fpops_bound", rsc_fpops_bound)) continue;
        if (xp.parse_double("rsc_memory_bound", rsc_memory_bound)) continue;
        if (xp.parse_double("rsc_disk_bound", rsc_disk_bound)) continue;
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

int RESULT::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/result")) {
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
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("wu_name", wu_name, sizeof(wu_name))) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) continue;
        if (xp.parse_str("project_url", project_url, sizeof(project_url))) continue;
        if (xp.parse_double("report_deadline", report_deadline)) continue;
        if (xp.parse_double("received_time", received_time)) continue;
        if (xp.parse_bool("ready_to_report", ready_to_report)) continue;
        if (xp.parse_bool("got_server_ack", got_server_ack)) continue;
        if (xp.parse_bool("suspended_via_gui", suspended_via_gui)) continue;
        if (xp.parse_bool("project_suspended_via_gui", project_suspended_via_gui)) continue;
        if (xp.parse_bool("coproc_missing", coproc_missing)) continue;
        if (xp.parse_bool("scheduler_wait", scheduler_wait)) continue;
        if (xp.parse_str("scheduler_wait_reason", scheduler_wait_reason, sizeof(scheduler_wait_reason))) continue;
        if (xp.parse_bool("network_wait", network_wait)) continue;
        if (xp.match_tag("active_task")) {
            active_task = true;
            continue;
        }
        if (xp.parse_double("final_cpu_time", final_cpu_time)) continue;
        if (xp.parse_double("final_elapsed_time", final_elapsed_time)) continue;
        if (xp.parse_int("state", state)) continue;
        if (xp.parse_int("scheduler_state", scheduler_state)) continue;
        if (xp.parse_int("exit_status", exit_status)) continue;
        if (xp.parse_int("signal", signal)) continue;
        if (xp.parse_int("active_task_state", active_task_state)) continue;
#if 0
        if (xp.match_tag("stderr_out")) {
            char buf[65536];
            xp.element_contents("</stderr_out>", buf);
            stderr_out = buf;
            continue;
        }
#endif
        if (xp.parse_int("app_version_num", app_version_num)) continue;
        if (xp.parse_int("slot", slot)) continue;
        if (xp.parse_int("pid", pid)) continue;
        if (xp.parse_double("checkpoint_cpu_time", checkpoint_cpu_time)) continue;
        if (xp.parse_double("current_cpu_time", current_cpu_time)) continue;
        if (xp.parse_double("elapsed_time", elapsed_time)) continue;
        if (xp.parse_double("swap_size", swap_size)) continue;
        if (xp.parse_double("working_set_size_smoothed", working_set_size_smoothed)) continue;
        if (xp.parse_double("fraction_done", fraction_done)) continue;
        if (xp.parse_double("estimated_cpu_time_remaining", estimated_cpu_time_remaining)) continue;
        if (xp.parse_bool("too_large", too_large)) continue;
        if (xp.parse_bool("needs_shmem", needs_shmem)) continue;
        if (xp.parse_bool("edf_scheduled", edf_scheduled)) continue;
        if (xp.parse_str("graphics_exec_path", graphics_exec_path, sizeof(graphics_exec_path))) continue;
        if (xp.parse_str("web_graphics_url", web_graphics_url, sizeof(web_graphics_url))) continue;
        if (xp.parse_str("remote_desktop_addr", remote_desktop_addr, sizeof(remote_desktop_addr))) continue;
        if (xp.parse_str("slot_path", slot_path, sizeof(slot_path))) continue;
        if (xp.parse_str("resources", resources, sizeof(resources))) continue;
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
    strcpy(web_graphics_url, "");
    strcpy(remote_desktop_addr, "");
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
    scheduler_wait = false;
    strcpy(scheduler_wait_reason, "");
    network_wait = false;

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

int FILE_TRANSFER::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/file_transfer")) return 0;
        if (xp.parse_string("name", name)) continue;
        if (xp.parse_string("project_url", project_url)) continue;
        if (xp.parse_string("project_name", project_name)) continue;
        if (xp.parse_double("nbytes", nbytes)) continue;
        if (xp.parse_bool("sticky", sticky)) continue;
        if (xp.match_tag("persistent_file_xfer")) {
            pers_xfer_active = true;
            continue;
        }
        if (xp.match_tag("file_xfer")) {
            xfer_active = true;
            continue;
        }
        if (xp.parse_bool("is_upload", is_upload)) {
            generated_locally = is_upload;
            continue;
        }
        if (xp.parse_bool("generated_locally", generated_locally)) {
            is_upload = generated_locally;
        }
        if (xp.parse_int("num_retries", num_retries)) continue;
        if (xp.parse_double("first_request_time", first_request_time)) continue;
        if (xp.parse_double("next_request_time", next_request_time)) continue;
        if (xp.parse_int("status", status)) continue;
        if (xp.parse_double("time_so_far", time_so_far)) continue;
        if (xp.parse_double("last_bytes_xferred", bytes_xferred)) continue;
        if (xp.parse_double("file_offset", file_offset)) continue;
        if (xp.parse_double("xfer_speed", xfer_speed)) continue;
        if (xp.parse_string("hostname", hostname)) continue;
        if (xp.parse_double("project_backoff", project_backoff)) continue;
    }
    return ERR_XML_PARSE;
}

void FILE_TRANSFER::clear() {
    name.clear();
    project_url.clear();
    project_name.clear();
    nbytes = 0;
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

int MESSAGE::parse(XML_PARSER& xp) {
    char buf[1024];
    while (!xp.get_tag()) {
        if (xp.match_tag("/msg")) return 0;
        if (xp.parse_string("project", project)) continue;
        if (xp.match_tag("body")) {
            xp.element_contents("</body>", buf, sizeof(buf));
            body = buf;
            continue;
        }
        if (xp.parse_int("pri", priority)) continue;
        if (xp.parse_int("time", timestamp)) continue;
        if (xp.parse_int("seqno", seqno)) continue;
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

int GR_PROXY_INFO::parse(XML_PARSER& xp) {
	std::string noproxy;
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_authentication = false;
    while (!xp.get_tag()) {
        if (xp.match_tag("/proxy_info")) return 0;
        if (xp.parse_string("socks_server_name", socks_server_name)) continue;
        if (xp.parse_int("socks_server_port", socks_server_port)) continue;
        if (xp.parse_string("socks5_user_name", socks5_user_name)) continue;
        if (xp.parse_string("socks5_user_passwd", socks5_user_passwd)) continue;
        if (xp.parse_string("http_server_name", http_server_name)) continue;
        if (xp.parse_int("http_server_port", http_server_port)) continue;
        if (xp.parse_string("http_user_name", http_user_name)) continue;
        if (xp.parse_string("http_user_passwd", http_user_passwd)) continue;
        if (xp.parse_bool("use_http_proxy", use_http_proxy)) continue;
        if (xp.parse_bool("use_socks_proxy", use_socks_proxy)) continue;
        if (xp.parse_bool("use_http_auth", use_http_authentication)) continue;
		if (xp.parse_string("no_proxy", noproxy_hosts)) continue;
    }
    return ERR_XML_PARSE;
}

void GR_PROXY_INFO::clear() {
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_authentication = false;
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

int CC_STATE::parse(XML_PARSER& xp) {
    string platform;
    PROJECT* project = NULL;
    int retval;

    while (!xp.get_tag()) {
        if (xp.match_tag("unauthorized")) {
            return ERR_AUTHENTICATOR;
        }
        if (xp.match_tag("/client_state")) break;

        if (xp.parse_bool("executing_as_daemon", executing_as_daemon)) continue;
        if (xp.match_tag("project")) {
            project = new PROJECT();
            retval = project->parse(xp);
            if (retval) {
                // should never happen
                delete project;
                project = NULL;
                continue;
            }
            projects.push_back(project);
            continue;
        }
        if (xp.match_tag("app")) {
            APP* app = new APP();
            retval = app->parse(xp);
            if (retval || !project) {
                delete app;
                continue;
            }
            app->project = project;
            apps.push_back(app);
            continue;
        }
        if (xp.match_tag("app_version")) {
            APP_VERSION* app_version = new APP_VERSION();
            retval = app_version->parse(xp);
            if (retval || !project) {
                delete app_version;
                continue;
            }
            app_version->project = project;
            app_version->app = lookup_app(project, app_version->app_name);
            if (!app_version->app) {
                delete app_version;
                continue;
            }
            app_versions.push_back(app_version);
            continue;
        }
        if (xp.match_tag("workunit")) {
            WORKUNIT* wu = new WORKUNIT();
            retval = wu->parse(xp);
            if (retval || !project) {
                delete wu;
                continue;
            }
            wu->project = project;
            wu->app = lookup_app(project, wu->app_name);
            if (!wu->app) {
                delete wu;
                continue;
            }
            wus.push_back(wu);
            continue;
        }
        if (xp.match_tag("result")) {
            RESULT* result = new RESULT();
            retval = result->parse(xp);
            if (retval || !project) {
                delete result;
                continue;
            }
            result->project = project;
            result->wup = lookup_wu(project, result->wu_name);
            if (!result->wup) {
                delete result;
                continue;
            }
            result->app = result->wup->app;
            APP_VERSION* avp;
            if (result->version_num) {
                avp = lookup_app_version(
                    project, result->app, result->version_num,
                    result->plan_class
                );
            } else {
                avp = lookup_app_version_old(
                    project, result->app, result->wup->version_num
                );
            }
            if (!avp) {
                delete result;
                continue;
            }
            result->avp = avp;
            results.push_back(result);
            continue;
        }
        if (xp.match_tag("global_preferences")) {
            bool flag = false;
            GLOBAL_PREFS_MASK mask;
            global_prefs.parse(xp, "", flag, mask);
            continue;
        }
        if (xp.parse_string("platform", platform)) {
            platforms.push_back(platform);
            continue;
        }
        if (xp.match_tag("host_info")) {
            host_info.parse(xp);
            continue;
        }
        if (xp.match_tag("time_stats")) {
            time_stats.parse(xp);
            continue;
        }
        if (xp.parse_bool("have_cuda", have_nvidia)) continue;
        if (xp.parse_bool("have_ati", have_ati)) continue;
    }
    return 0;
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
    host_info.clear_host_info();
    have_nvidia = false;
    have_ati = false;
}

PROJECT* CC_STATE::lookup_project(const char* url) {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        if (!strcmp(projects[i]->master_url, url)) return projects[i];
    }
    return 0;
}

APP* CC_STATE::lookup_app(PROJECT* project, const char* name) {
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

WORKUNIT* CC_STATE::lookup_wu(PROJECT* project, const char* name) {
    unsigned int i;
    for (i=0; i<wus.size(); i++) {
        if (wus[i]->project != project) continue;
        if (!strcmp(wus[i]->name, name)) return wus[i];
    }
    return 0;
}

RESULT* CC_STATE::lookup_result(PROJECT* project, const char* name) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (results[i]->project != project) continue;
        if (!strcmp(results[i]->name, name)) return results[i];
    }
    return 0;
}

RESULT* CC_STATE::lookup_result(const char* url, const char* name) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (strcmp(results[i]->project_url, url)) continue;
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
    complete = false;
    unsigned int i;
    for (i=0; i<notices.size(); i++) {
        delete notices[i];
    }
    notices.clear();
}

ACCT_MGR_INFO::ACCT_MGR_INFO() {
    clear();
}

int ACCT_MGR_INFO::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/acct_mgr_info")) return 0;
        if (xp.parse_string("acct_mgr_name", acct_mgr_name)) continue;
        if (xp.parse_string("acct_mgr_url", acct_mgr_url)) continue;
        if (xp.parse_bool("have_credentials", have_credentials)) continue;
        if (xp.parse_bool("cookie_required", cookie_required)) continue;
        if (xp.parse_string("cookie_failure_url", cookie_failure_url)) continue;
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

int ACCT_MGR_RPC_REPLY::parse(XML_PARSER& xp) {
    clear();
    string msg;
    while (!xp.get_tag()) {
        if (xp.match_tag("/acct_mgr_rpc_reply")) return 0;
        if (xp.parse_int("error_num", error_num)) continue;
        if (xp.parse_string("message", msg)) {
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

int PROJECT_ATTACH_REPLY::parse(XML_PARSER& xp) {
    std::string msg;
    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/project_attach_reply")) return 0;
        if (xp.parse_int("error_num", error_num)) continue;
        if (xp.parse_string("message", msg)) {
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

int PROJECT_INIT_STATUS::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/get_project_init_status")) return 0;
        if (xp.parse_string("url", url)) continue;
        if (xp.parse_string("name", name)) continue;
        if (xp.parse_string("team_name", team_name)) continue;
        if (xp.parse_bool("has_account_key", has_account_key)) continue;
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

int PROJECT_CONFIG::parse(XML_PARSER& xp) {
    std::string msg;
    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/project_config")) return 0;
        if (xp.parse_int("error_num", error_num)) continue;
        if (xp.parse_string("name", name)) continue;
        if (xp.parse_string("master_url", master_url)) continue;
        if (xp.parse_int("local_revision", local_revision)) continue;
        if (xp.parse_int("min_passwd_length", min_passwd_length)) continue;
        if (xp.parse_bool("account_manager", account_manager)) continue;
        if (xp.parse_bool("uses_username", uses_username)) continue;
        if (xp.parse_bool("account_creation_disabled", account_creation_disabled)) continue;
        if (xp.parse_bool("client_account_creation_disabled", client_account_creation_disabled)) continue;
        if (xp.parse_string("error_msg", error_msg)) continue;
        if (xp.match_tag("terms_of_use")) {
            char buf[65536];
            xp.element_contents("</terms_of_use>", buf, sizeof(buf));
            terms_of_use = buf;
            continue;
        }
        if (xp.parse_int("min_client_version", min_client_version)) continue;
        if (xp.parse_bool("web_stopped", web_stopped)) continue;
        if (xp.parse_bool("sched_stopped", sched_stopped)) continue;
        if (xp.parse_string("platform_name", msg)) {
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

int ACCOUNT_OUT::parse(XML_PARSER& xp) {
    clear();
    while (!xp.get_tag()) {
        if (xp.parse_int("error_num", error_num)) continue;
        if (xp.parse_string("error_msg", error_msg)) continue;
        if (xp.parse_string("authenticator", authenticator)) continue;
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

int CC_STATUS::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/cc_status")) return 0; 
        if (xp.parse_int("network_status", network_status)) continue;
        if (xp.parse_bool("ams_password_error", ams_password_error)) continue;
        if (xp.parse_bool("manager_must_quit", manager_must_quit)) continue;
        if (xp.parse_int("task_suspend_reason", task_suspend_reason)) continue;
        if (xp.parse_int("task_mode", task_mode)) continue;
        if (xp.parse_int("task_mode_perm", task_mode_perm)) continue;
		if (xp.parse_double("task_mode_delay", task_mode_delay)) continue;
        if (xp.parse_int("gpu_suspend_reason", gpu_suspend_reason)) continue;
        if (xp.parse_int("gpu_mode", gpu_mode)) continue;
        if (xp.parse_int("gpu_mode_perm", gpu_mode_perm)) continue;
		if (xp.parse_double("gpu_mode_delay", gpu_mode_delay)) continue;
        if (xp.parse_int("network_suspend_reason", network_suspend_reason)) continue;
        if (xp.parse_int("network_mode", network_mode)) continue;
        if (xp.parse_int("network_mode_perm", network_mode_perm)) continue;
		if (xp.parse_double("network_mode_delay", network_mode_delay)) continue;
        if (xp.parse_bool("disallow_attach", disallow_attach)) continue;
        if (xp.parse_bool("simple_gui_only", simple_gui_only)) continue;
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
    RPC rpc(this);

    state.clear();

    retval = rpc.do_rpc("<get_state/>\n");
    if (retval) return retval;
    return state.parse(rpc.xp);
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
                rp->parse(rpc.xp);
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
                fip->parse(rpc.xp);
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
                project->parse(rpc.xp);
                info.projects.push_back(project);
                continue;
            }
            else if (match_tag(buf, "<result>")) {
                RESULT* result = new RESULT();
                result->parse(rpc.xp);
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
                project->parse(rpc.xp);
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
    MIOFILE mf;
    PROJECT_LIST_ENTRY* project;
    AM_LIST_ENTRY* am;
    RPC rpc(this);

    pl.clear();

    retval = rpc.do_rpc("<get_all_projects_list/>\n");
    if (retval) return retval;
    while (!rpc.xp.get_tag()) {
        if (rpc.xp.match_tag("/projects")) break;
        else if (rpc.xp.match_tag("project")) {
            project = new PROJECT_LIST_ENTRY();
            retval = project->parse(rpc.xp);
            if (!retval) {
                pl.projects.push_back(project);
            } else {
                delete project;
            }
            continue;
        } else if (rpc.xp.match_tag("account_manager")) {
            am = new AM_LIST_ENTRY();
            retval = am->parse(rpc.xp);
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
                project->parse(rpc.xp);
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

int DAILY_STATS::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/daily_statistics")) return 0;
        if (xp.parse_double("day", day)) continue;
        if (xp.parse_double("user_total_credit", user_total_credit)) continue;
        if (xp.parse_double("user_expavg_credit", user_expavg_credit)) continue;
        if (xp.parse_double("host_total_credit", host_total_credit)) continue;
        if (xp.parse_double("host_expavg_credit", host_expavg_credit)) continue;
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
                        retval = ds.parse(rpc.xp);
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
                retval = status.parse(rpc.xp);
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
        retval = reply.parse(rpc.xp);
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
                rp->parse(rpc.xp);
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
        "<set_proxy_settings>\n"
        "    <proxy_info>\n"
        "%s%s%s"
        "        <http_server_name>%s</http_server_name>\n"
        "        <http_server_port>%d</http_server_port>\n"
        "        <http_user_name>%s</http_user_name>\n"
        "        <http_user_passwd>%s</http_user_passwd>\n"
        "        <socks_server_name>%s</socks_server_name>\n"
        "        <socks_server_port>%d</socks_server_port>\n"
        "        <socks5_user_name>%s</socks5_user_name>\n"
        "        <socks5_user_passwd>%s</socks5_user_passwd>\n"		
		"        <no_proxy>%s</no_proxy>\n"
        "    </proxy_info>\n"
        "</set_proxy_settings>\n",
        pi.use_http_proxy?"        <use_http_proxy/>\n":"",
        pi.use_socks_proxy?"        <use_socks_proxy/>\n":"",
        pi.use_http_authentication?"        <use_http_auth/>\n":"",
        pi.http_server_name.c_str(),
        pi.http_server_port,
        pi.http_user_name.c_str(),
        pi.http_user_passwd.c_str(),
        pi.socks_server_name.c_str(),
        pi.socks_server_port,
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
        retval = p.parse(rpc.xp);
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

int RPC_CLIENT::get_messages(int seqno, MESSAGES& msgs, bool translatable) {
    int retval;
    SET_LOCALE sl;
    char buf[256];
    RPC rpc(this);

    sprintf(buf,
        "<get_messages>\n"
        "  <seqno>%d</seqno>\n"
        "%s"
        "</get_messages>\n",
        seqno,
        translatable?"  <translatable/>\n":""
    );

    retval = rpc.do_rpc(buf);
    if (!retval) {
        while (!rpc.xp.get_tag()) {
            if (rpc.xp.match_tag("/msgs")) {
                return 0;
            }
            if (rpc.xp.match_tag("msg")) {
                MESSAGE* message = new MESSAGE();
                message->parse(rpc.xp);
                msgs.messages.push_back(message);
                continue;
            }
            if (rpc.xp.match_tag("boinc_gui_rpc_reply")) continue;
            if (rpc.xp.match_tag("msgs")) continue;
            //fprintf(stderr, "bad tag %s\n", buf);
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
    char buf[256];
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<get_host_info/>");
    if (retval) return retval;
    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "<host_info>")) {
            return h.parse(rpc.xp);
        }
        if (match_tag(buf, "<unauthorized")) {
            return ERR_AUTHENTICATOR;
        }
    }
    return ERR_XML_PARSE;
}


int RPC_CLIENT::quit() {
    int retval;
    char buf[256];
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<quit/>\n");
    if (retval) return retval;
    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "success")) {
            return 0;
        }
    }
    return ERR_XML_PARSE;
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
        retval = r.parse(rpc.xp);
    }
    return retval;
}

int RPC_CLIENT::acct_mgr_info(ACCT_MGR_INFO& ami) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<acct_mgr_info/>\n");
    if (!retval) {
        retval = ami.parse(rpc.xp);
    }
    return retval;
}

int RPC_CLIENT::get_project_init_status(PROJECT_INIT_STATUS& pis) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<get_project_init_status/>\n");
    if (!retval) {
        retval = pis.parse(rpc.xp);
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
        retval = pc.parse(rpc.xp);
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
        retval = ao.parse(rpc.xp);
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
        retval = ao.parse(rpc.xp);
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
        return ERR_NOT_FOUND;
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
        return ERR_NOT_FOUND;
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

int RPC_CLIENT::get_cc_config(CONFIG& config, LOG_FLAGS& log_flags) {
    int retval;
    SET_LOCALE sl;
    RPC rpc(this);

    retval = rpc.do_rpc("<get_cc_config/>");
    if (retval) return retval;

    return config.parse(rpc.xp, log_flags);
}

int RPC_CLIENT::set_cc_config(CONFIG& config, LOG_FLAGS& log_flags) {
    SET_LOCALE sl;
    char buf[64000];
    MIOFILE mf;
    int retval;
    RPC rpc(this);
    
    mf.init_buf_write(buf, sizeof(buf));
    config.write(mf, log_flags);

    retval = rpc.do_rpc(buf);
    return retval;
}

static int parse_notices(XML_PARSER& xp, NOTICES& notices) {
    int retval;

    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("notice")) {
            NOTICE* np = new NOTICE();
            retval = np->parse(xp);
            if (!retval) {
                if (np->seqno == -1) {
                    notices.notices.clear();
                    notices.complete = true;
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
    return parse_notices(rpc.xp, notices);
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
    return parse_notices(rpc.xp, notices);
}

int RPC_CLIENT::get_daily_xfer_history(DAILY_XFER_HISTORY& dxh) {
    SET_LOCALE sl;
    RPC rpc(this);
    int retval;

    retval = rpc.do_rpc("<get_daily_xfer_history/>\n");
    if (retval) return retval;
    return dxh.parse(rpc.xp);
}

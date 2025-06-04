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

#if defined(_WIN32)
#include "boinc_win.h"
#else
#include "config.h"
#include <cstring>
#include <string>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "miofile.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "url.h"
#include "util.h"

#include "app_ipc.h"

using std::string;

APP_INIT_DATA::APP_INIT_DATA() {
    clear();
}

APP_INIT_DATA::~APP_INIT_DATA() {
    if (project_preferences) {
        free(project_preferences);
        project_preferences=0;      // paranoia
    }
}

APP_INIT_DATA::APP_INIT_DATA(const APP_INIT_DATA& a) {
    copy(a);
}

APP_INIT_DATA &APP_INIT_DATA::operator=(const APP_INIT_DATA& a) {
    if (this != &a) {
        copy(a);
    }
    return *this;
}

void APP_INIT_DATA::copy(const APP_INIT_DATA& a) {
    safe_strcpy(app_name, a.app_name);
    safe_strcpy(plan_class, a.plan_class);
    safe_strcpy(symstore, a.symstore);
    safe_strcpy(acct_mgr_url, a.acct_mgr_url);
    safe_strcpy(user_name, a.user_name);
    safe_strcpy(team_name, a.team_name);
    safe_strcpy(project_dir, a.project_dir);
    safe_strcpy(boinc_dir, a.boinc_dir);
    safe_strcpy(wu_name, a.wu_name);
    safe_strcpy(result_name, a.result_name);
    safe_strcpy(authenticator, a.authenticator);
    memcpy(&shmem_seg_name, &a.shmem_seg_name, sizeof(SHMEM_SEG_NAME));
    safe_strcpy(gpu_type, a.gpu_type);

    // use assignment for the rest, especially the classes
    // (so that the overloaded operators are called!)
    major_version               = a.major_version;
    minor_version               = a.minor_version;
    release                     = a.release;
    app_version                 = a.app_version;
    userid                      = a.userid;
    teamid                      = a.teamid;
    hostid                      = a.hostid;
    slot                        = a.slot;
    client_pid                  = a.client_pid;
    user_total_credit           = a.user_total_credit;
    user_expavg_credit          = a.user_expavg_credit;
    host_total_credit           = a.host_total_credit;
    host_expavg_credit          = a.host_expavg_credit;
    resource_share_fraction     = a.resource_share_fraction;
    host_info                   = a.host_info;
    proxy_info                  = a.proxy_info;
    global_prefs                = a.global_prefs;
    starting_elapsed_time       = a.starting_elapsed_time;
    using_sandbox               = a.using_sandbox;
    vm_extensions_disabled      = a.vm_extensions_disabled;
    rsc_fpops_est               = a.rsc_fpops_est;
    rsc_fpops_bound             = a.rsc_fpops_bound;
    rsc_memory_bound            = a.rsc_memory_bound;
    rsc_disk_bound              = a.rsc_disk_bound;
    computation_deadline        = a.computation_deadline;
    fraction_done_start         = a.fraction_done_start;
    fraction_done_end           = a.fraction_done_end;
    gpu_device_num              = a.gpu_device_num;
    gpu_opencl_dev_index        = a.gpu_opencl_dev_index;
    gpu_usage                   = a.gpu_usage;
    ncpus                       = a.ncpus;
    checkpoint_period           = a.checkpoint_period;
    wu_cpu_time                 = a.wu_cpu_time;
    if (a.project_preferences) {
        project_preferences = strdup(a.project_preferences);
    } else {
        project_preferences = NULL;
    }
    vbox_window                 = a.vbox_window;
    no_priority_change          = a.no_priority_change;
    process_priority            = a.process_priority;
    process_priority_special    = a.process_priority_special;
    app_files                   = a.app_files;
}

int write_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    char buf[2048];
    fprintf(f,
        "<app_init_data>\n"
        "<major_version>%d</major_version>\n"
        "<minor_version>%d</minor_version>\n"
        "<release>%d</release>\n"
        "<app_version>%d</app_version>\n"
        "<userid>%d</userid>\n"
        "<teamid>%d</teamid>\n"
        "<hostid>%d</hostid>\n",
        ai.major_version,
        ai.minor_version,
        ai.release,
        ai.app_version,
        ai.userid,
        ai.teamid,
        ai.hostid
    );
    // some strings are always present;
    // for others, print only if present

    fprintf(f, "<app_name>%s</app_name>\n", ai.app_name);
    if (strlen(ai.plan_class)) {
        fprintf(f, "<plan_class>%s</plan_class>\n", ai.plan_class);
    }
    if (strlen(ai.symstore)) {
        fprintf(f, "<symstore>%s</symstore>\n", ai.symstore);
    }
    if (strlen(ai.acct_mgr_url)) {
        fprintf(f, "<acct_mgr_url>%s</acct_mgr_url>\n", ai.acct_mgr_url);
    }
    if (ai.project_preferences && strlen(ai.project_preferences)) {
        fprintf(f, "<project_preferences>\n%s</project_preferences>\n", ai.project_preferences);
    }
    if (strlen(ai.team_name)) {
        xml_escape(ai.team_name, buf, sizeof(buf));
        fprintf(f, "<team_name>%s</team_name>\n", buf);
    }
    if (strlen(ai.user_name)) {
        xml_escape(ai.user_name, buf, sizeof(buf));
        fprintf(f, "<user_name>%s</user_name>\n", buf);
    }
    fprintf(f, "<project_dir>%s</project_dir>\n", ai.project_dir);
    fprintf(f, "<boinc_dir>%s</boinc_dir>\n", ai.boinc_dir);
    fprintf(f, "<authenticator>%s</authenticator>\n", ai.authenticator);
    fprintf(f, "<wu_name>%s</wu_name>\n", ai.wu_name);
    fprintf(f, "<result_name>%s</result_name>\n", ai.result_name);
#ifdef _WIN32
    if (strlen(ai.shmem_seg_name)) {
        fprintf(f, "<comm_obj_name>%s</comm_obj_name>\n", ai.shmem_seg_name);
    }
#else
    fprintf(f, "<shm_key>%d</shm_key>\n", ai.shmem_seg_name);
#endif
    fprintf(f,
        "<slot>%d</slot>\n"
        "<client_pid>%d</client_pid>\n"
        "<wu_cpu_time>%f</wu_cpu_time>\n"
        "<starting_elapsed_time>%f</starting_elapsed_time>\n"
        "<using_sandbox>%d</using_sandbox>\n"
        "<vm_extensions_disabled>%d</vm_extensions_disabled>\n"
        "<user_total_credit>%f</user_total_credit>\n"
        "<user_expavg_credit>%f</user_expavg_credit>\n"
        "<host_total_credit>%f</host_total_credit>\n"
        "<host_expavg_credit>%f</host_expavg_credit>\n"
        "<resource_share_fraction>%f</resource_share_fraction>\n"
        "<checkpoint_period>%f</checkpoint_period>\n"
        "<fraction_done_start>%f</fraction_done_start>\n"
        "<fraction_done_end>%f</fraction_done_end>\n"
        "<gpu_type>%s</gpu_type>\n"
        "<gpu_device_num>%d</gpu_device_num>\n"
        "<gpu_opencl_dev_index>%d</gpu_opencl_dev_index>\n"
        "<gpu_usage>%f</gpu_usage>\n"
        "<ncpus>%f</ncpus>\n"
        "<rsc_fpops_est>%f</rsc_fpops_est>\n"
        "<rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "<rsc_memory_bound>%f</rsc_memory_bound>\n"
        "<rsc_disk_bound>%f</rsc_disk_bound>\n"
        "<computation_deadline>%f</computation_deadline>\n"
        "<vbox_window>%d</vbox_window>\n"
        "<no_priority_change>%d</no_priority_change>\n"
        "<process_priority>%d</process_priority>\n"
        "<process_priority_special>%d</process_priority_special>\n",
        ai.slot,
        ai.client_pid,
        ai.wu_cpu_time,
        ai.starting_elapsed_time,
        ai.using_sandbox?1:0,
        ai.vm_extensions_disabled?1:0,
        ai.user_total_credit,
        ai.user_expavg_credit,
        ai.host_total_credit,
        ai.host_expavg_credit,
        ai.resource_share_fraction,
        ai.checkpoint_period,
        ai.fraction_done_start,
        ai.fraction_done_end,
        ai.gpu_type,
        ai.gpu_device_num,
        ai.gpu_opencl_dev_index,
        ai.gpu_usage,
        ai.ncpus,
        ai.rsc_fpops_est,
        ai.rsc_fpops_bound,
        ai.rsc_memory_bound,
        ai.rsc_disk_bound,
        ai.computation_deadline,
        ai.vbox_window,
        ai.no_priority_change?1:0,
        ai.process_priority,
        ai.process_priority_special
    );
    MIOFILE mf;
    mf.init_file(f);
    ai.host_info.write(mf, true, true);
    if (ai.proxy_info.using_proxy()) {
        ai.proxy_info.write(mf);
    }
    ai.global_prefs.write(mf);
    for (unsigned int i=0; i<ai.app_files.size(); i++) {
        fprintf(f, "<app_file>%s</app_file>\n", ai.app_files[i].c_str());
    }
    fprintf(f, "</app_init_data>\n");
    return 0;
}

void APP_INIT_DATA::clear() {
    major_version = 0;
    minor_version = 0;
    release = 0;
    app_version = 0;
    app_name[0] = 0;
    plan_class[0] = 0;
    symstore[0] = 0;
    acct_mgr_url[0] = 0;
    project_preferences = NULL;
    userid = 0;
    teamid = 0;
    hostid = 0;
    user_name[0] = 0;
    team_name[0] = 0;
    project_dir[0] = 0;
    boinc_dir[0] = 0;
    wu_name[0] = 0;
    result_name[0] = 0;
    authenticator[0] = 0;
    slot = 0;
    client_pid = 0;
    user_total_credit = 0;
    user_expavg_credit = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    resource_share_fraction = 0;
    host_info.clear_host_info();
    proxy_info.clear();
    global_prefs.defaults();
    starting_elapsed_time = 0;
    using_sandbox = false;
    vm_extensions_disabled = false;
    rsc_fpops_est = 0;
    rsc_fpops_bound = 0;
    rsc_memory_bound = 0;
    rsc_disk_bound = 0;
    computation_deadline = 0;
    fraction_done_start = 0;
    fraction_done_end = 0;
    checkpoint_period = 0;
    // gpu_type is an empty string for client versions before 6.13.3 without this
    // field or (on newer clients) if BOINC did not assign an OpenCL GPU to task.
    gpu_type[0] = 0;
    // gpu_device_num < 0 for client versions before 6.13.3 without this field
    // or (on newer clients) if BOINC did not assign an OpenCL GPU to task.
    gpu_device_num = -1;
    // gpu_opencl_dev_index < 0 for client versions before 7.0.12 without this
    // field or (on newer clients) if BOINC did not assign any GPU to task.
    gpu_opencl_dev_index = -1;
    gpu_usage = 0;
    ncpus = 0;
    memset(&shmem_seg_name, 0, sizeof(shmem_seg_name));
    wu_cpu_time = 0;
    vbox_window = false;
    no_priority_change = false;
    process_priority = -1;
    process_priority_special = -1;
}

int parse_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    int retval;
    bool flag;

    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("app_init_data")) {
        fprintf(stderr, "%s: no start tag in app init data\n",
            time_to_string(dtime())
        );
        return ERR_XML_PARSE;
    }

    if (ai.project_preferences) {
        free(ai.project_preferences);
        ai.project_preferences = 0;
    }
    ai.clear();
    ai.fraction_done_start = 0;
    ai.fraction_done_end = 1;

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            fprintf(stderr,
                "%s: unexpected text in init_data.xml: %s\n",
                time_to_string(dtime()), xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/app_init_data")) return 0;
        if (xp.match_tag("project_preferences")) {
            retval = dup_element(f, "project_preferences", &ai.project_preferences);
            if (retval) return retval;
            continue;
        }
        if (xp.match_tag("global_preferences")) {
            GLOBAL_PREFS_MASK mask;
            retval = ai.global_prefs.parse(xp, "", flag, mask);
            if (retval) return retval;
            continue;
        }
        if (xp.match_tag("host_info")) {
            ai.host_info.parse(xp);
            continue;
        }
        if (xp.match_tag("proxy_info")) {
            ai.proxy_info.parse(xp);
            continue;
        }
        if (xp.parse_int("major_version", ai.major_version)) continue;
        if (xp.parse_int("minor_version", ai.minor_version)) continue;
        if (xp.parse_int("release", ai.release)) continue;
        if (xp.parse_int("app_version", ai.app_version)) continue;
        if (xp.parse_str("app_name", ai.app_name, sizeof(ai.app_name))) continue;
        if (xp.parse_str("plan_class", ai.plan_class, sizeof(ai.plan_class))) continue;
        if (xp.parse_str("symstore", ai.symstore, sizeof(ai.symstore))) continue;
        if (xp.parse_str("acct_mgr_url", ai.acct_mgr_url, sizeof(ai.acct_mgr_url))) continue;
        if (xp.parse_int("userid", ai.userid)) continue;
        if (xp.parse_int("teamid", ai.teamid)) continue;
        if (xp.parse_int("hostid", ai.hostid)) continue;
        if (xp.parse_str("user_name", ai.user_name, sizeof(ai.user_name))) {
            xml_unescape(ai.user_name);
            continue;
        }
        if (xp.parse_str("team_name", ai.team_name, sizeof(ai.team_name))) {
            xml_unescape(ai.team_name);
            continue;
        }
        if (xp.parse_str("project_dir", ai.project_dir, sizeof(ai.project_dir))) continue;
        if (xp.parse_str("boinc_dir", ai.boinc_dir, sizeof(ai.boinc_dir))) continue;
        if (xp.parse_str("authenticator", ai.authenticator, sizeof(ai.authenticator))) continue;
        if (xp.parse_str("wu_name", ai.wu_name, sizeof(ai.wu_name))) continue;
        if (xp.parse_str("result_name", ai.result_name, sizeof(ai.result_name))) continue;
#ifdef _WIN32
        if (xp.parse_str("comm_obj_name", ai.shmem_seg_name, sizeof(ai.shmem_seg_name))) continue;
#else
        if (xp.parse_int("shm_key", ai.shmem_seg_name)) continue;
#endif
        if (xp.parse_int("slot", ai.slot)) continue;
        if (xp.parse_int("client_pid", ai.client_pid)) continue;
        if (xp.parse_double("user_total_credit", ai.user_total_credit)) continue;
        if (xp.parse_double("user_expavg_credit", ai.user_expavg_credit)) continue;
        if (xp.parse_double("host_total_credit", ai.host_total_credit)) continue;
        if (xp.parse_double("host_expavg_credit", ai.host_expavg_credit)) continue;
        if (xp.parse_double("resource_share_fraction", ai.resource_share_fraction)) continue;
        if (xp.parse_double("rsc_fpops_est", ai.rsc_fpops_est)) continue;
        if (xp.parse_double("rsc_fpops_bound", ai.rsc_fpops_bound)) continue;
        if (xp.parse_double("rsc_memory_bound", ai.rsc_memory_bound)) continue;
        if (xp.parse_double("rsc_disk_bound", ai.rsc_disk_bound)) continue;
        if (xp.parse_double("computation_deadline", ai.computation_deadline)) continue;
        if (xp.parse_double("wu_cpu_time", ai.wu_cpu_time)) continue;
        if (xp.parse_double("starting_elapsed_time", ai.starting_elapsed_time)) continue;
        if (xp.parse_bool("using_sandbox", ai.using_sandbox)) continue;
        if (xp.parse_bool("vm_extensions_disabled", ai.vm_extensions_disabled)) continue;
        if (xp.parse_double("checkpoint_period", ai.checkpoint_period)) continue;
        if (xp.parse_str("gpu_type", ai.gpu_type, sizeof(ai.gpu_type))) continue;
        if (xp.parse_int("gpu_device_num", ai.gpu_device_num)) continue;
        if (xp.parse_int("gpu_opencl_dev_index", ai.gpu_opencl_dev_index)) continue;
        if (xp.parse_double("gpu_usage", ai.gpu_usage)) continue;
        if (xp.parse_double("ncpus", ai.ncpus)) continue;
        if (xp.parse_double("fraction_done_start", ai.fraction_done_start)) continue;
        if (xp.parse_double("fraction_done_end", ai.fraction_done_end)) continue;
        if (xp.parse_bool("vbox_window", ai.vbox_window)) continue;
        if (xp.parse_bool("no_priority_change", ai.no_priority_change)) continue;
        if (xp.parse_int("process_priority", ai.process_priority)) continue;
        if (xp.parse_int("process_priority_special", ai.process_priority_special)) continue;
        xp.skip_unexpected(false, "parse_init_data_file");
    }
    fprintf(stderr, "%s: parse_init_data_file: no end tag\n",
        time_to_string(dtime())
    );
    return ERR_XML_PARSE;
}

APP_CLIENT_SHM::APP_CLIENT_SHM() : shm(NULL) {
}

bool MSG_CHANNEL::get_msg(char *msg) {
    if (!buf[0]) return false;
    strlcpy(msg, buf+1, MSG_CHANNEL_SIZE-1);
    buf[0] = 0;
    return true;
}

bool MSG_CHANNEL::send_msg(const char *msg) {
    if (has_msg()) return false;
    strlcpy(buf+1, msg, MSG_CHANNEL_SIZE-1);
    buf[0] = 1;
    return true;
}

void MSG_CHANNEL::send_msg_overwrite(const char* msg) {
    strlcpy(buf+1, msg, MSG_CHANNEL_SIZE-1);
    buf[0] = 1;
}

void APP_CLIENT_SHM::reset_msgs() {
    memset(shm, 0, sizeof(SHARED_MEM));
}

void url_to_project_dir(char* url, char* dir, int dirsize) {
    char buf[256];
    escape_project_url(url, buf);
    snprintf(dir, dirsize, "%s/%s", PROJECT_DIR, buf);
}

// this is here because it's called (once) in the Manager
//
int resolve_soft_link(
    const char *virtual_name, char *physical_name, int len
) {
    FILE *fp;
    char buf[512], *p;

    if (!virtual_name) return ERR_NULL;
    strlcpy(physical_name, virtual_name, len);

#ifndef _WIN32
    if (is_symlink(virtual_name)) {
        return 0;
    }
#endif

    // Open the link file and read the first line
    //
    fp = boinc_fopen(virtual_name, "r");
    if (!fp) return 0;

    // must initialize buf since fgets() on an empty file won't do anything
    //
    buf[0] = 0;
    p = fgets(buf, sizeof(buf), fp);
    fclose(fp);

    // If it's the <soft_link> XML tag, return its value,
    // otherwise, return the original file name
    //
    if (p) parse_str(buf, "<soft_link>", physical_name, len);
    return 0;
}

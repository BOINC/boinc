// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#endif

#include "common_defs.h"
#include "diagnostics.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "str_util.h"
#include "url.h"

#include "client_state.h"
#include "client_msgs.h"
#include "cs_proxy.h"
#include "file_names.h"
#include "project.h"
#include "result.h"
#include "sandbox.h"

using std::string;

LOG_FLAGS log_flags;
CC_CONFIG cc_config;

static void show_flag(char* buf, int len, bool flag, const char* flag_name) {
    if (!flag) return;
    int n = (int)strlen(buf);
    if (!n) {
        strlcpy(buf, flag_name, len);
        return;
    }
    strlcat(buf, ", ", len);
    strlcat(buf, flag_name, len);
    if (strlen(buf) > 60) {
        msg_printf(NULL, MSG_INFO, "log flags: %s", buf);
        strlcpy(buf, "", len);
    }
}

void LOG_FLAGS::show() {
    char buf[256];
    safe_strcpy(buf, "");
    show_flag(buf, sizeof(buf), file_xfer, "file_xfer");
    show_flag(buf, sizeof(buf), sched_ops, "sched_ops");
    show_flag(buf, sizeof(buf), task, "task");

    show_flag(buf, sizeof(buf), app_msg_receive, "app_msg_receive");
    show_flag(buf, sizeof(buf), app_msg_send, "app_msg_send");
    show_flag(buf, sizeof(buf), async_file_debug, "async_file_debug");
    show_flag(buf, sizeof(buf), benchmark_debug, "benchmark_debug");
    show_flag(buf, sizeof(buf), checkpoint_debug, "checkpoint_debug");
    show_flag(buf, sizeof(buf), coproc_debug, "coproc_debug");
    show_flag(buf, sizeof(buf), cpu_sched, "cpu_sched");
    show_flag(buf, sizeof(buf), cpu_sched_debug, "cpu_sched_debug");
    show_flag(buf, sizeof(buf), cpu_sched_status, "cpu_sched_status");
    show_flag(buf, sizeof(buf), dcf_debug, "dcf_debug");
    show_flag(buf, sizeof(buf), file_xfer_debug, "file_xfer_debug");
    show_flag(buf, sizeof(buf), gui_rpc_debug, "gui_rpc_debug");
    show_flag(buf, sizeof(buf), heartbeat_debug, "heartbeat_debug");
    show_flag(buf, sizeof(buf), http_debug, "http_debug");
    show_flag(buf, sizeof(buf), http_xfer_debug, "http_xfer_debug");
    show_flag(buf, sizeof(buf), idle_detection_debug, "idle_detection_debug");
    show_flag(buf, sizeof(buf), mem_usage_debug, "mem_usage_debug");
    show_flag(buf, sizeof(buf), network_status_debug, "network_status_debug");
    show_flag(buf, sizeof(buf), notice_debug, "notice_debug");
    show_flag(buf, sizeof(buf), poll_debug, "poll_debug");
    show_flag(buf, sizeof(buf), priority_debug, "priority_debug");
    show_flag(buf, sizeof(buf), proxy_debug, "proxy_debug");
    show_flag(buf, sizeof(buf), rr_simulation, "rr_simulation");
    show_flag(buf, sizeof(buf), sched_op_debug, "sched_op_debug");
    show_flag(buf, sizeof(buf), scrsave_debug, "scrsave_debug");
    show_flag(buf, sizeof(buf), slot_debug, "slot_debug");
    show_flag(buf, sizeof(buf), sporadic_debug, "sporadic_debug");
    show_flag(buf, sizeof(buf), state_debug, "state_debug");
    show_flag(buf, sizeof(buf), statefile_debug, "statefile_debug");
    show_flag(buf, sizeof(buf), task_debug, "task_debug");
    show_flag(buf, sizeof(buf), time_debug, "time_debug");
    show_flag(buf, sizeof(buf), unparsed_xml, "unparsed_xml");
    show_flag(buf, sizeof(buf), work_fetch_debug, "work_fetch_debug");

    if (strlen(buf)) {
        msg_printf(NULL, MSG_INFO, "log flags: %s", buf);
    }
}

static void show_gpu_ignore(vector<int>& devs, int rt) {
    for (unsigned int i=0; i<devs.size(); i++) {
        msg_printf(NULL, MSG_INFO,
            "Config: ignoring %s %d", proc_type_name(rt), devs[i]
        );
    }
}

// Show GPU exclusions in event log.
// Don't show errors - they were already shown when we parsed the config file
//
static void show_exclude_gpu(EXCLUDE_GPU& e) {
    char t[256], app_name[256], dev[256];
    PROJECT *p = gstate.lookup_project(e.url.c_str());
    if (!p) return;
    if (e.type.empty()) {
        safe_strcpy(t, "all");
    } else {
        safe_strcpy(t, e.type.c_str());
    }
    if (e.appname.empty()) {
        safe_strcpy(app_name, "all");
    } else {
        safe_strcpy(app_name, e.appname.c_str());
    }
    if (e.device_num < 0) {
        safe_strcpy(dev, "all");
    } else {
        snprintf(dev, sizeof(dev), "%d", e.device_num);
    }
    msg_printf(p, MSG_INFO,
        "Config: excluded GPU.  Type: %s.  App: %s.  Device: %s",
        t, app_name, dev
    );
}

// Print config info.
// This is called during startup (after client_state.xml has been read)
// and also from the handle_read_cc_config GUI RPC.
//
// Keep these in alpha order
//
// TODO: show all config options
//
void CC_CONFIG::show() {
    unsigned int i;
    if (abort_jobs_on_exit) {
        msg_printf(NULL, MSG_INFO, "Config: abort jobs on exit");
    }
    if (allow_gui_rpc_get) {
        msg_printf(NULL, MSG_INFO, "Config: allow web file fetch");
    }
    if (allow_multiple_clients) {
        msg_printf(NULL, MSG_INFO, "Config: allow multiple clients");
    }
    if (allow_remote_gui_rpc) {
        msg_printf(NULL, MSG_INFO, "Config: GUI RPC allowed from any host");
    }
    FILE* f = fopen(REMOTEHOST_FILE_NAME, "r");
    if (f) {
        msg_printf(NULL, MSG_INFO,
            "Config: GUI RPCs allowed from:"
        );
        char buf[256];
        while (fgets(buf, 256, f)) {
            strip_whitespace(buf);
            if (!(buf[0] =='#' || buf[0] == ';') && strlen(buf) > 0 ) {
                msg_printf(NULL, MSG_INFO,
                    "    %s", buf
                );
            }
        }
        fclose(f);
    }
    if (disallow_attach) {
        msg_printf(NULL, MSG_INFO, "Config: disallow project attach");
    }
    if (dont_check_file_sizes) {
        msg_printf(NULL, MSG_INFO, "Config: don't check file sizes");
    }
    if (dont_suspend_nci) {
        msg_printf(NULL, MSG_INFO, "Config: don't suspend NCI tasks");
    }
    if (dont_use_vbox) {
        msg_printf(NULL, MSG_INFO, "Config: don't use VirtualBox");
    }
    if (dont_use_docker) {
        msg_printf(NULL, MSG_INFO, "Config: don't use Docker");
    }
    if (dont_use_wsl) {
        msg_printf(NULL, MSG_INFO, "Config: don't use Windows Subsystem for Linux");
    }
    for (string s: disallowed_wsls) {
        msg_printf(NULL, MSG_INFO,
            "Config: disallowed WSL distro: %s", s.c_str()
        );
    }
    for (i=0; i<alt_platforms.size(); i++) {
        msg_printf(NULL, MSG_INFO,
            "Config: alternate platform: %s", alt_platforms[i].c_str()
        );
    }
    for (i=0; i<exclude_gpus.size(); i++) {
        show_exclude_gpu(exclude_gpus[i]);
    }
    for (i=0; i<exclusive_apps.size(); i++) {
        msg_printf(NULL, MSG_INFO,
            "Config: don't compute while %s is running",
            exclusive_apps[i].c_str()
        );
    }
    for (i=0; i<exclusive_gpu_apps.size(); i++) {
        msg_printf(NULL, MSG_INFO,
            "Config: don't use GPUs while %s is running",
            exclusive_gpu_apps[i].c_str()
        );
    }
    if (exit_after_finish) {
        msg_printf(NULL, MSG_INFO, "Config: exit after finish");
    }
    if (exit_before_start) {
        msg_printf(NULL, MSG_INFO, "Config: exit before start task");
    }
    if (exit_when_idle) {
        msg_printf(NULL, MSG_INFO, "Config: exit when idle");
    }
    if (fetch_minimal_work) {
        msg_printf(NULL, MSG_INFO, "Config: fetch minimal work");
    }
    if (fetch_on_update) {
        msg_printf(NULL, MSG_INFO, "Config: fetch on update");
    }
    if (http_1_0) {
        msg_printf(NULL, MSG_INFO, "Config: use HTTP 1.0");
    }
    for (int j=1; j<NPROC_TYPES; j++) {
        show_gpu_ignore(ignore_gpu_instance[j], j);
    }
    if (lower_client_priority) {
        msg_printf(NULL, MSG_INFO, "Config: lower client priority");
    }
    if (max_event_log_lines != DEFAULT_MAX_EVENT_LOG_LINES) {
        if (max_event_log_lines) {
            msg_printf(NULL, MSG_INFO,
                "Config: event log limit %d lines", max_event_log_lines
            );
        } else {
            msg_printf(NULL, MSG_INFO, "Config: event log limit disabled");
        }
    }
    if (max_overdue_days >= 0) {
        msg_printf(NULL, MSG_INFO,
            "Config: abort tasks overdue by > %.2f days",
            max_overdue_days
        );
    }
    if (ncpus>0) {
        msg_printf(NULL, MSG_INFO, "Config: simulate %d CPUs", cc_config.ncpus);
    }
    if (no_gpus) {
        msg_printf(NULL, MSG_INFO, "Config: don't use coprocessors");
    }
    if (no_info_fetch) {
        msg_printf(NULL, MSG_INFO, "Config: don't fetch project list or client version info");
    }
    if (no_opencl) {
        msg_printf(NULL, MSG_INFO, "Config: don't use OpenCL");
    }
    if (no_priority_change) {
        msg_printf(NULL, MSG_INFO, "Config: run apps at regular priority");
    }
    if (no_rdp_check) {
        msg_printf(NULL, MSG_INFO, "Config: allow GPU apps when using remote desktop");
    }
    if (report_results_immediately) {
        msg_printf(NULL, MSG_INFO, "Config: report completed tasks immediately");
    }
    if (unsigned_apps_ok) {
        msg_printf(NULL, MSG_INFO, "Config: unsigned apps OK");
    }
    if (use_all_gpus) {
        msg_printf(NULL, MSG_INFO, "Config: use all coprocessors");
    }
    if (vbox_window) {
        msg_printf(NULL, MSG_INFO,
            "Config: open console window for VirtualBox applications"
        );
        if (g_use_sandbox) {
            msg_printf(NULL, MSG_INFO,
                "    NOTE: the client is running in protected mode,"
            );
            msg_printf(NULL, MSG_INFO,
                "    so VirtualBox console windows cannot be opened."
            );
        }
    }
    for (i=0; i<ignore_tty.size(); i++) {
        msg_printf(NULL, MSG_INFO,
            "Config: ignore tty: %s", ignore_tty[i].c_str()
        );
    }
    if (!device_name.empty()) {
        msg_printf(NULL, MSG_INFO, "Config: device name is %s", device_name.c_str());
    }
}

// This is used by the BOINC client.
// KEEP IN SYNCH WITH CC_CONFIG::parse_options()!!
// (It's separate so that we can write messages in it)

int CC_CONFIG::parse_options_client(XML_PARSER& xp) {
    string s;
    int n, retval;

    //clear();
    // don't do this here because some options are set by cmdline args,
    // which are parsed first
    // but do clear these, which aren't accessable via cmdline:
    //
    alt_platforms.clear();
    exclusive_apps.clear();
    exclusive_gpu_apps.clear();
    for (int i=1; i<NPROC_TYPES; i++) {
        ignore_gpu_instance[i].clear();
    }
    ignore_tty.clear();

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            msg_printf_notice(NULL, false,
                "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
                "%s: %s",
                _("Unexpected text in cc_config.xml"),
                xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/options")) {
            return 0;
        }
        if (xp.parse_bool("abort_jobs_on_exit", abort_jobs_on_exit)) continue;
        if (xp.parse_bool("allow_gui_rpc_get", allow_gui_rpc_get)) continue;
        if (xp.parse_bool("allow_multiple_clients", allow_multiple_clients)) continue;
        if (xp.parse_bool("allow_remote_gui_rpc", allow_remote_gui_rpc)) continue;
        if (xp.parse_string("alt_platform", s)) {
            alt_platforms.push_back(s);
            continue;
        }
        if (xp.match_tag("coproc")) {
            COPROC c;
            retval = c.parse(xp);
            if (retval) {
                msg_printf_notice(NULL, false, NULL,
                    "Can't parse <coproc> element in cc_config.xml"
                );
                continue;
            }
            retval = config_coprocs.add(c);
            if (retval) {
                msg_printf_notice(NULL, false, NULL,
                    "Duplicate <coproc> element in cc_config.xml"
                );
            }
            continue;
        }
        if (xp.parse_bool("disallow_attach", disallow_attach)) continue;
        if (xp.parse_bool("dont_check_file_sizes", dont_check_file_sizes)) continue;
        if (xp.parse_bool("dont_contact_ref_site", dont_contact_ref_site)) continue;
        if (xp.parse_bool("lower_client_priority", lower_client_priority)) continue;
        if (xp.parse_bool("dont_suspend_nci", dont_suspend_nci)) continue;
        if (xp.parse_bool("dont_use_vbox", dont_use_vbox)) continue;
        if (xp.parse_bool("dont_use_wsl", dont_use_wsl)) continue;
        if (xp.parse_string("disallowed_wsl", s)) {
            disallowed_wsls.push_back(s);
            continue;
        }
        if (xp.parse_bool("dont_use_docker", dont_use_docker)) continue;
        if (xp.match_tag("exclude_gpu")) {
            EXCLUDE_GPU eg;
            retval = eg.parse(xp);
            if (retval) {
                msg_printf_notice(NULL, false, NULL,
                    "Can't parse <exclude_gpu> element in cc_config.xml"
                );
            } else {
                exclude_gpus.push_back(eg);
            }
            continue;
        }
        if (xp.parse_string("exclusive_app", s)) {
            if (!strstr(s.c_str(), "boinc")) {
                exclusive_apps.push_back(s);
            }
            continue;
        }
        if (xp.parse_string("exclusive_gpu_app", s)) {
            if (!strstr(s.c_str(), "boinc")) {
                exclusive_gpu_apps.push_back(s);
            }
            continue;
        }
        if (xp.parse_bool("exit_after_finish", exit_after_finish)) continue;
        if (xp.parse_bool("exit_before_start", exit_before_start)) continue;
        if (xp.parse_bool("exit_when_idle", exit_when_idle)) {
            if (exit_when_idle) {
                report_results_immediately = true;
            }
            continue;
        }
        if (xp.parse_bool("fetch_minimal_work", fetch_minimal_work)) continue;
        if (xp.parse_bool("fetch_on_update", fetch_on_update)) continue;
        if (xp.parse_string("force_auth", force_auth)) {
            downcase_string(force_auth);
            continue;
        }
        if (xp.parse_bool("http_1_0", http_1_0)) continue;
        if (xp.parse_int("http_transfer_timeout", http_transfer_timeout)) continue;
        if (xp.parse_int("http_transfer_timeout_bps", http_transfer_timeout_bps)) continue;
        if (xp.parse_int("ignore_cuda_dev", n)||xp.parse_int("ignore_nvidia_dev", n)) {
            ignore_gpu_instance[PROC_TYPE_NVIDIA_GPU].push_back(n);
            continue;
        }
        if (xp.parse_int("ignore_ati_dev", n)) {
            ignore_gpu_instance[PROC_TYPE_AMD_GPU].push_back(n);
            continue;
        }
        if (xp.parse_int("ignore_intel_dev", n)) {
            ignore_gpu_instance[PROC_TYPE_INTEL_GPU].push_back(n);
            continue;
        }
        if (xp.parse_int("ignore_apple_dev", n)) {
            ignore_gpu_instance[PROC_TYPE_APPLE_GPU].push_back(n);
            continue;
        }
        if (xp.parse_int("max_event_log_lines", max_event_log_lines)) continue;
        if (xp.parse_int("max_file_xfers", max_file_xfers)) continue;
        if (xp.parse_int("max_file_xfers_per_project", max_file_xfers_per_project)) continue;
        if (xp.parse_double("max_overdue_days", max_overdue_days)) continue;
        if (xp.parse_double("max_stderr_file_size", max_stderr_file_size)) continue;
        if (xp.parse_double("max_stdout_file_size", max_stdout_file_size)) continue;
        if (xp.parse_int("max_tasks_reported", max_tasks_reported)) continue;
        if (xp.parse_int("ncpus", ncpus)) continue;
        if (xp.parse_bool("no_alt_platform", no_alt_platform)) continue;
        if (xp.parse_bool("no_disk_usage", no_disk_usage)) continue;
        if (xp.parse_bool("no_gpus", no_gpus)) continue;
        if (xp.parse_bool("no_info_fetch", no_info_fetch)) continue;
        if (xp.parse_bool("no_opencl", no_opencl)) continue;
        if (xp.parse_bool("no_priority_change", no_priority_change)) continue;
        if (xp.parse_bool("no_rdp_check", no_rdp_check)) continue;
        if (xp.parse_bool("os_random_only", os_random_only)) continue;
        if (xp.parse_int("process_priority", process_priority)) continue;
        if (xp.parse_int("process_priority_special", process_priority_special)) continue;
        if (xp.match_tag("proxy_info")) {
            retval = proxy_info.parse_config(xp);
            if (retval) {
                msg_printf_notice(NULL, false, NULL,
                    "Can't parse <proxy_info> element in cc_config.xml"
                );
            }
            continue;
        }
        if (xp.parse_double("rec_half_life_days", rec_half_life)) {
            if (rec_half_life <= 0) rec_half_life = 10;
            rec_half_life *= 86400;
            continue;
        }
        if (xp.parse_bool("report_results_immediately", report_results_immediately)) continue;
        if (xp.parse_bool("run_apps_manually", run_apps_manually)) continue;
        if (xp.parse_int("save_stats_days", save_stats_days)) continue;
        if (xp.parse_bool("simple_gui_only", simple_gui_only)) continue;
        if (xp.parse_bool("skip_cpu_benchmarks", skip_cpu_benchmarks)) continue;
        if (xp.parse_double("start_delay", start_delay)) continue;
        if (xp.parse_bool("stderr_head", stderr_head)) continue;
        if (xp.parse_bool("suppress_net_info", suppress_net_info)) continue;
        if (xp.parse_bool("unsigned_apps_ok", unsigned_apps_ok)) continue;
        if (xp.parse_bool("use_all_gpus", use_all_gpus)) continue;
        if (xp.parse_bool("use_certs", use_certs)) continue;
        if (xp.parse_bool("use_certs_only", use_certs_only)) continue;
        if (xp.parse_bool("vbox_window", vbox_window)) continue;
        if (xp.parse_string("ignore_tty", s)) {
            ignore_tty.push_back(s);
            continue;
        }
        if (xp.parse_string("device_name", device_name)) continue;

        // The following tags have been moved to nvc_config and NVC_CONFIG_FILE,
        // but CC_CONFIG::write() in older clients
        // may have written their default values to CONFIG_FILE.
        // Silently skip them if present.
        //
        if (xp.parse_string("client_download_url", s)) continue;
        if (xp.parse_string("client_new_version_text", s)) continue;
        if (xp.parse_string("client_version_check_url", s)) continue;
        if (xp.parse_string("network_test_url", s)) continue;

        msg_printf_notice(NULL, false,
            "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
            "%s: <%s>",
            _("Unrecognized tag in cc_config.xml"),
            xp.parsed_tag
        );
        xp.skip_unexpected(true, "CC_CONFIG::parse_options");
    }
    return ERR_XML_PARSE;
}

int CC_CONFIG::parse_client(FILE* f) {
    MIOFILE mf;
    XML_PARSER xp(&mf);

    mf.init_file(f);
    if (!xp.parse_start("cc_config")) {
        msg_printf_notice(NULL, false,
            "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
            "%s",
            _("Missing start tag in cc_config.xml")
        );
        return ERR_XML_PARSE;
    }
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            msg_printf_notice(NULL, false,
                "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
                "%s: %s",
                _("Unexpected text in cc_config.xml"),
                xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/cc_config")) {
            notices.remove_notices(NULL, REMOVE_CONFIG_MSG);
            return 0;
        }
        if (xp.match_tag("log_flags")) {
            log_flags.parse(xp);
            continue;
        }
        if (xp.match_tag("options")) {
            int retval = parse_options_client(xp);
            if (retval) {
                msg_printf_notice(NULL, false,
                    "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
                    "%s",
                    _("Error in cc_config.xml options")
                );
            }
            continue;
        }
        if (xp.match_tag("options/")) continue;
        if (xp.match_tag("log_flags/")) continue;
        msg_printf_notice(NULL, false,
            "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
            "%s: <%s>",
            _("Unrecognized tag in cc_config.xml"),
            xp.parsed_tag
        );
        xp.skip_unexpected(true, "CC_CONFIG.parse");
    }
    msg_printf_notice(NULL, false,
        "https://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
        "%s",
        _("Missing end tag in cc_config.xml")
    );
    return ERR_XML_PARSE;
}

int CC_CONFIG::parse(FILE* f) {
    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);

    return parse(xp, log_flags);
}

// read config file, e.g. cc_config.xml
// Called on startup and in response to GUI RPC requesting reread
//
int read_config_file(bool init, const char* fname) {
    if (!init) {
        msg_printf(NULL, MSG_INFO, "Re-reading %s", fname);
        cc_config.defaults();
        log_flags.init();
    }
    FILE* f = boinc_fopen(fname, "r");
    if (!f) {
        msg_printf(NULL, MSG_INFO, "cc_config.xml not found - using defaults");
        return ERR_FOPEN;
    }
    cc_config.parse_client(f);
    fclose(f);
#ifndef SIM
    diagnostics_set_max_file_sizes(
        cc_config.max_stdout_file_size, cc_config.max_stderr_file_size
    );
#endif
    config_proxy_info = cc_config.proxy_info;

    if (init) {
        coprocs = cc_config.config_coprocs;
    } else {
        select_proxy_info();        // in case added or removed proxy info
    }
    return 0;
}

// Do stuff involving GPU exclusions.
// - check syntax
// - set APP::non_excluded_instances[rsc_type]
//   (used in RR sim)
// - set PROJECT::rsc_pwf[rsc_type].non_excluded_instances
//   (used in work fetch)
// - set PROJECT::rsc_pwf[rsc_type].ncoprocs_excluded
//   (used in RR sim and work fetch)
// - set APP_VERSION::missing_coproc for app versions where
//   all instances are excluded
// - set RESULT::missing_coproc for results of these app versions
//
void process_gpu_exclusions() {
    unsigned int i, j, a;
    PROJECT *p;

    // check the syntactic validity of the exclusions
    //
    for (i=0; i<cc_config.exclude_gpus.size(); i++) {
        EXCLUDE_GPU& eg = cc_config.exclude_gpus[i];
        p = gstate.lookup_project(eg.url.c_str());
        if (!p) {
            msg_printf(0, MSG_USER_ALERT,
                "cc_config.xml: bad URL in GPU exclusion: %s", eg.url.c_str()
            );
            continue;
        }
        if (!eg.appname.empty()) {
            APP* app = gstate.lookup_app(p, eg.appname.c_str());
            if (!app) {
                msg_printf(p, MSG_USER_ALERT,
                    "cc_config.xml: a GPU exclusion refers to an unknown application '%s'.  Known applications: %s",
                    eg.appname.c_str(),
                    app_list_string(p).c_str()
                );
                continue;
            }
        }
        if (!eg.type.empty()) {
            bool found = false;
            string types;
            for (int k=1; k<coprocs.n_rsc; k++) {
                COPROC& cp = coprocs.coprocs[k];
                if (eg.type == cp.type) {
                    found = true;

                    // skip exclusions of non-existent devices
                    //
                    if (eg.device_num && (cp.device_num_index(eg.device_num) < 0)) {
                        break;
                    }
                    rsc_work_fetch[k].has_exclusions = true;
                    break;
                }
                types += " " + string(cp.type);
            }
            if (!found) {
                msg_printf(p, MSG_USER_ALERT,
                    "cc_config.xml: bad type '%s' in GPU exclusion; valid types:%s",
                    eg.type.c_str(), types.c_str()
                );
                continue;
            }
        } else {
            for (int k=1; k<coprocs.n_rsc; k++) {
                rsc_work_fetch[k].has_exclusions = true;
            }
        }
    }

    for (i=0; i<gstate.apps.size(); i++) {
        APP* app = gstate.apps[i];
        for (int k=1; k<coprocs.n_rsc; k++) {
            COPROC& cp = coprocs.coprocs[k];
            for (int h=0; h<cp.count; h++) {
                app->non_excluded_instances[k] |= ((COPROC_INSTANCE_BITMAP)1)<<h;
            }
        }
    }

    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        for (int k=1; k<coprocs.n_rsc; k++) {
            COPROC& cp = coprocs.coprocs[k];
            COPROC_INSTANCE_BITMAP all_instances = 0;
            // bitmap of 1 for all instances
            //
            for (int h=0; h<cp.count; h++) {
                all_instances |= ((COPROC_INSTANCE_BITMAP)1)<<h;
            }
            for (j=0; j<cc_config.exclude_gpus.size(); j++) {
                EXCLUDE_GPU& eg = cc_config.exclude_gpus[j];
                if (!eg.type.empty() && (eg.type != cp.type)) continue;
                if (!urls_match(eg.url.c_str(), p->master_url)) continue;
                COPROC_INSTANCE_BITMAP mask;
                if (eg.device_num >= 0) {
                    int index = cp.device_num_index(eg.device_num);
                    // exclusion may refer to nonexistent GPU
                    //
                    if (index < 0) continue;
                    mask = ((COPROC_INSTANCE_BITMAP)1)<<index;
                } else {
                    mask = all_instances;
                }
                if (eg.appname.empty()) {
                    // exclusion applies to all apps
                    //
                    for (a=0; a<gstate.apps.size(); a++) {
                        APP* app = gstate.apps[a];
                        if (app->project != p) continue;
                        app->non_excluded_instances[k] &= ~mask;
                    }
                } else {
                    // exclusion applies to a particular app
                    //
                    APP* app = gstate.lookup_app(p, eg.appname.c_str());
                    if (!app) continue;
                    app->non_excluded_instances[k] &= ~mask;
                }
            }

            bool found = false;
            p->rsc_pwf[k].non_excluded_instances = 0;
            for (a=0; a<gstate.apps.size(); a++) {
                APP* app = gstate.apps[a];
                if (app->project != p) continue;
                found = true;
                p->rsc_pwf[k].non_excluded_instances |= app->non_excluded_instances[k];
            }
            // if project has no apps yet (for some reason)
            // assume it can use all instances
            //
            if (!found) {
                p->rsc_pwf[k].non_excluded_instances = all_instances;
            }

            // compute ncoprocs_excluded as the number of instances
            // excluded for at least 1 app
            //
            p->rsc_pwf[k].ncoprocs_excluded = 0;
            for (int b=0; b<cp.count; b++) {
                COPROC_INSTANCE_BITMAP mask = ((COPROC_INSTANCE_BITMAP)1)<<b;
                for (a=0; a<gstate.apps.size(); a++) {
                    APP* app = gstate.apps[a];
                    if (app->project != p) continue;
                    if (!(app->non_excluded_instances[k] & mask)) {
                        p->rsc_pwf[k].ncoprocs_excluded++;
                        break;
                    }
                }
            }
        }
    }

    for (i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        if (avp->resource_usage.missing_coproc) continue;
        int rt = avp->resource_usage.rsc_type;
        if (!rt) continue;
        COPROC& cp = coprocs.coprocs[rt];
        bool found = false;
        for (int k=0; k<cp.count; k++) {
            if (!gpu_excluded(avp->app, cp, k)) {
                found = true;
                break;
            }
        }
        if (found) continue;
        avp->resource_usage.missing_coproc = true;
        avp->resource_usage.missing_coproc_name[0] = 0;
        for (j=0; j<gstate.results.size(); j++) {
            RESULT* rp = gstate.results[j];
            if (rp->avp != avp) continue;
            msg_printf(avp->project, MSG_INFO,
                "marking %s as coproc missing",
                rp->name
            );
            rp->resource_usage.missing_coproc = true;
            rp->resource_usage.missing_coproc_name[0] = 0;
        }
    }
}

bool gpu_excluded(APP* app, COPROC& cp, int ind) {
    if (cc_config.no_gpus) return true;
    PROJECT* p = app->project;
    for (unsigned int i=0; i<cc_config.exclude_gpus.size(); i++) {
        EXCLUDE_GPU& eg = cc_config.exclude_gpus[i];
        if (!urls_match(eg.url.c_str(), p->master_url)) continue;
        if (!eg.type.empty() && (eg.type != cp.type)) continue;
        if (!eg.appname.empty() && (eg.appname != app->name)) continue;
        if (eg.device_num >= 0 && eg.device_num != cp.device_nums[ind]) continue;
        return true;
    }
    return false;
}

// if the configuration file disallows the use of a GPU type
// for a project, set a flag to that effect
//
void set_no_rsc_config() {
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT& p = *gstate.projects[i];
        for (int j=1; j<coprocs.n_rsc; j++) {
            bool allowed[MAX_COPROC_INSTANCES];
            memset(allowed, 0, sizeof(allowed));
            COPROC& c = coprocs.coprocs[j];
            for (int k=0; k<c.count; k++) {
                allowed[c.device_nums[k]] = true;
            }
            for (unsigned int k=0; k<cc_config.exclude_gpus.size(); k++) {
                EXCLUDE_GPU& e = cc_config.exclude_gpus[k];
                if (strcmp(e.url.c_str(), p.master_url)) continue;
                if (!e.type.empty() && strcmp(e.type.c_str(), c.type)) continue;
                if (!e.appname.empty()) continue;
                if (e.device_num < 0) {
                    memset(allowed, 0, sizeof(allowed));
                    break;
                }
                allowed[e.device_num] = false;
            }
            p.no_rsc_config[j] = true;
            for (int k=0; k<c.count; k++) {
                if (allowed[c.device_nums[k]]) {
                    p.no_rsc_config[j] = false;
                    break;
                }
            }
        }
    }
}

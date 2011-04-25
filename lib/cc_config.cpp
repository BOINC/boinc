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
#include "str_util.h"

#include "cc_config.h"

using std::string;

LOG_FLAGS::LOG_FLAGS() {
    init();
}

void LOG_FLAGS::init() {
    memset(this, 0, sizeof(LOG_FLAGS));
    // on by default (others are off by default)
    //
    task = true;
    file_xfer = true;
    sched_ops = true;
}

// Parse log flag preferences
//
int LOG_FLAGS::parse(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            continue;
        }
        if (!strcmp(tag, "/log_flags")) return 0;
        if (xp.parse_bool(tag, "file_xfer", file_xfer)) continue;
        if (xp.parse_bool(tag, "sched_ops", sched_ops)) continue;
        if (xp.parse_bool(tag, "task", task)) continue;

        if (xp.parse_bool(tag, "app_msg_receive", app_msg_receive)) continue;
        if (xp.parse_bool(tag, "app_msg_send", app_msg_send)) continue;
        if (xp.parse_bool(tag, "benchmark_debug", benchmark_debug)) continue;
        if (xp.parse_bool(tag, "checkpoint_debug", checkpoint_debug)) continue;
        if (xp.parse_bool(tag, "coproc_debug", coproc_debug)) continue;
        if (xp.parse_bool(tag, "cpu_sched", cpu_sched)) continue;
        if (xp.parse_bool(tag, "cpu_sched_debug", cpu_sched_debug)) continue;
        if (xp.parse_bool(tag, "cpu_sched_status", cpu_sched_status)) continue;
        if (xp.parse_bool(tag, "dcf_debug", dcf_debug)) continue;
        if (xp.parse_bool(tag, "debt_debug", debt_debug)) continue;
        if (xp.parse_bool(tag, "std_debug", std_debug)) continue;
        if (xp.parse_bool(tag, "file_xfer_debug", file_xfer_debug)) continue;
        if (xp.parse_bool(tag, "gui_rpc_debug", gui_rpc_debug)) continue;
        if (xp.parse_bool(tag, "heartbeat_debug", heartbeat_debug)) continue;
        if (xp.parse_bool(tag, "http_debug", http_debug)) continue;
        if (xp.parse_bool(tag, "http_xfer_debug", http_xfer_debug)) continue;
        if (xp.parse_bool(tag, "mem_usage_debug", mem_usage_debug)) continue;
        if (xp.parse_bool(tag, "network_status_debug", network_status_debug)) continue;
        if (xp.parse_bool(tag, "poll_debug", poll_debug)) continue;
        if (xp.parse_bool(tag, "proxy_debug", proxy_debug)) continue;
        if (xp.parse_bool(tag, "rr_simulation", rr_simulation)) continue;
        if (xp.parse_bool(tag, "sched_op_debug", sched_op_debug)) continue;
        if (xp.parse_bool(tag, "scrsave_debug", scrsave_debug)) continue;
        if (xp.parse_bool(tag, "slot_debug", slot_debug)) continue;
        if (xp.parse_bool(tag, "state_debug", state_debug)) continue;
        if (xp.parse_bool(tag, "statefile_debug", statefile_debug)) continue;
        if (xp.parse_bool(tag, "task_debug", task_debug)) continue;
        if (xp.parse_bool(tag, "time_debug", time_debug)) continue;
        if (xp.parse_bool(tag, "unparsed_xml", unparsed_xml)) continue;
        if (xp.parse_bool(tag, "work_fetch_debug", work_fetch_debug)) continue;
        if (xp.parse_bool(tag, "notice_debug", notice_debug)) continue;
        xp.skip_unexpected(tag, true, "LOG_FLAGS::parse");
    }
    return ERR_XML_PARSE;
}

CONFIG::CONFIG() {
}

int CONFIG::parse_options(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;
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
    ignore_cuda_dev.clear();
    ignore_ati_dev.clear();

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            continue;
        }
        if (!strcmp(tag, "/options")) {
            return 0;
        }
        if (xp.parse_bool(tag, "abort_jobs_on_exit", abort_jobs_on_exit)) continue;
        if (xp.parse_bool(tag, "allow_multiple_clients", allow_multiple_clients)) continue;
        if (xp.parse_bool(tag, "allow_remote_gui_rpc", allow_remote_gui_rpc)) continue;
        if (xp.parse_string(tag, "alt_platform", s)) {
            alt_platforms.push_back(s);
            continue;
        }
        if (xp.parse_string(tag, "client_download_url", client_download_url)) {
            downcase_string(client_download_url);
            continue;
        }
        if (xp.parse_string(tag, "client_version_check_url", client_version_check_url)) {
            downcase_string(client_version_check_url);
            continue;
        }
        if (!strcmp(tag, "coproc")) {
            COPROC c;
            retval = c.parse(xp);
            if (retval) return retval;
            retval = config_coprocs.add(c);
            continue;
        }
        if (xp.parse_str(tag, "data_dir", data_dir, sizeof(data_dir))) {
            continue;
        }
        if (xp.parse_bool(tag, "disallow_attach", disallow_attach)) continue;
        if (xp.parse_bool(tag, "dont_check_file_sizes", dont_check_file_sizes)) continue;
        if (xp.parse_bool(tag, "dont_contact_ref_site", dont_contact_ref_site)) continue;
        if (xp.parse_string(tag, "exclusive_app", s)) {
            if (!strstr(s.c_str(), "boinc")) {
                exclusive_apps.push_back(s);
            }
            continue;
        }
        if (xp.parse_string(tag, "exclusive_gpu_app", s)) {
            if (!strstr(s.c_str(), "boinc")) {
                exclusive_gpu_apps.push_back(s);
            }
            continue;
        }
        if (xp.parse_bool(tag, "exit_after_finish", exit_after_finish)) continue;
        if (xp.parse_bool(tag, "exit_when_idle", exit_when_idle)) {
            if (exit_when_idle) {
                report_results_immediately = true;
            }
            continue;
        }
        if (xp.parse_bool(tag, "fetch_minimal_work", fetch_minimal_work)) continue;
        if (xp.parse_string(tag, "force_auth", force_auth)) {
            downcase_string(force_auth);
            continue;
        }
        if (xp.parse_bool(tag, "http_1_0", http_1_0)) continue;
        if (xp.parse_int(tag, "ignore_cuda_dev", n)) {
            ignore_cuda_dev.push_back(n);
            continue;
        }
        if (xp.parse_int(tag, "ignore_ati_dev", n)) {
            ignore_ati_dev.push_back(n);
            continue;
        }
        if (xp.parse_int(tag, "max_file_xfers", max_file_xfers)) continue;
        if (xp.parse_int(tag, "max_file_xfers_per_project", max_file_xfers_per_project)) continue;
        if (xp.parse_int(tag, "max_stderr_file_size", max_stderr_file_size)) continue;
        if (xp.parse_int(tag, "max_stdout_file_size", max_stdout_file_size)) continue;
        if (xp.parse_int(tag, "max_tasks_reported", max_tasks_reported)) continue;
        if (xp.parse_int(tag, "ncpus", ncpus)) continue;
        if (xp.parse_string(tag, "network_test_url", network_test_url)) {
            downcase_string(network_test_url);
            continue;
        }
        if (xp.parse_bool(tag, "no_alt_platform", no_alt_platform)) continue;
        if (xp.parse_bool(tag, "no_gpus", no_gpus)) continue;
        if (xp.parse_bool(tag, "no_info_fetch", no_info_fetch)) continue;
        if (xp.parse_bool(tag, "no_priority_change", no_priority_change)) continue;
        if (xp.parse_bool(tag, "os_random_only", os_random_only)) continue;
#ifndef SIM
        if (!strcmp(tag, "proxy_info")) {
            int retval = proxy_info.parse_config(*xp.f);
            if (retval) return retval;
            continue;
        }
#endif
        if (xp.parse_bool(tag, "report_results_immediately", report_results_immediately)) continue;
        if (xp.parse_bool(tag, "run_apps_manually", run_apps_manually)) continue;
        if (xp.parse_int(tag, "save_stats_days", save_stats_days)) continue;
        if (xp.parse_bool(tag, "simple_gui_only", simple_gui_only)) continue;
        if (xp.parse_bool(tag, "skip_cpu_benchmarks", skip_cpu_benchmarks)) continue;
        if (xp.parse_double(tag, "start_delay", start_delay)) continue;
        if (xp.parse_bool(tag, "stderr_head", stderr_head)) continue;
        if (xp.parse_bool(tag, "suppress_net_info", suppress_net_info)) continue;
        if (xp.parse_bool(tag, "unsigned_apps_ok", unsigned_apps_ok)) continue;
        if (xp.parse_bool(tag, "use_all_gpus", use_all_gpus)) continue;
        if (xp.parse_bool(tag, "use_certs", use_certs)) continue;
        if (xp.parse_bool(tag, "use_certs_only", use_certs_only)) continue;
        if (xp.parse_bool(tag, "zero_debts", zero_debts)) continue;

        xp.skip_unexpected(tag, true, "CONFIG::parse_options");
    }
    return ERR_XML_PARSE;
}

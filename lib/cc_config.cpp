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

#include "cc_config.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <algorithm>
#endif

#include "common_defs.h"
#include "diagnostics.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"
#include "url.h"

#include "cc_config.h"

using std::string;

void LOG_FLAGS::init() {
    static const LOG_FLAGS x;
    *this = x;
}

// Parse log flag preferences
//
int LOG_FLAGS::parse(XML_PARSER& xp) {
    init();
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/log_flags")) return 0;
        if (xp.parse_bool("file_xfer", file_xfer)) continue;
        if (xp.parse_bool("sched_ops", sched_ops)) continue;
        if (xp.parse_bool("task", task)) continue;

#ifdef ANDROID
        if (xp.parse_bool("android_debug", android_debug)) continue;
#endif
        if (xp.parse_bool("app_msg_receive", app_msg_receive)) continue;
        if (xp.parse_bool("app_msg_send", app_msg_send)) continue;
        if (xp.parse_bool("async_file_debug", async_file_debug)) continue;
        if (xp.parse_bool("benchmark_debug", benchmark_debug)) continue;
        if (xp.parse_bool("checkpoint_debug", checkpoint_debug)) continue;
        if (xp.parse_bool("coproc_debug", coproc_debug)) continue;
        if (xp.parse_bool("cpu_sched", cpu_sched)) continue;
        if (xp.parse_bool("cpu_sched_debug", cpu_sched_debug)) continue;
        if (xp.parse_bool("cpu_sched_status", cpu_sched_status)) continue;
        if (xp.parse_bool("dcf_debug", dcf_debug)) continue;
        if (xp.parse_bool("disk_usage_debug", disk_usage_debug)) continue;
        if (xp.parse_bool("file_xfer_debug", file_xfer_debug)) continue;
        if (xp.parse_bool("gui_rpc_debug", gui_rpc_debug)) continue;
        if (xp.parse_bool("heartbeat_debug", heartbeat_debug)) continue;
        if (xp.parse_bool("http_debug", http_debug)) continue;
        if (xp.parse_bool("http_xfer_debug", http_xfer_debug)) continue;
        if (xp.parse_bool("idle_detection_debug", idle_detection_debug)) continue;
        if (xp.parse_bool("mem_usage_debug", mem_usage_debug)) continue;
        if (xp.parse_bool("network_status_debug", network_status_debug)) continue;
        if (xp.parse_bool("notice_debug", notice_debug)) continue;
        if (xp.parse_bool("poll_debug", poll_debug)) continue;
        if (xp.parse_bool("priority_debug", priority_debug)) continue;
        if (xp.parse_bool("proxy_debug", proxy_debug)) continue;
        if (xp.parse_bool("rr_simulation", rr_simulation)) continue;
        if (xp.parse_bool("rrsim_detail", rrsim_detail)) continue;
        if (xp.parse_bool("sched_op_debug", sched_op_debug)) continue;
        if (xp.parse_bool("scrsave_debug", scrsave_debug)) continue;
        if (xp.parse_bool("slot_debug", slot_debug)) continue;
        if (xp.parse_bool("sporadic_debug", sporadic_debug)) continue;
        if (xp.parse_bool("state_debug", state_debug)) continue;
        if (xp.parse_bool("statefile_debug", statefile_debug)) continue;
        if (xp.parse_bool("suspend_debug", suspend_debug)) continue;
        if (xp.parse_bool("task_debug", task_debug)) continue;
        if (xp.parse_bool("time_debug", time_debug)) continue;
        if (xp.parse_bool("trickle_debug", trickle_debug)) continue;
        if (xp.parse_bool("unparsed_xml", unparsed_xml)) continue;
        if (xp.parse_bool("work_fetch_debug", work_fetch_debug)) continue;
        xp.skip_unexpected(true, "LOG_FLAGS::parse");
    }
    return ERR_XML_PARSE;
}

int LOG_FLAGS::write(MIOFILE& out) {
    out.printf(
        "    <log_flags>\n"
        "        <file_xfer>%d</file_xfer>\n"
        "        <sched_ops>%d</sched_ops>\n"
        "        <task>%d</task>\n"
#ifdef ANDROID
        "        <android_debug>%d</android_debug>\n"
#endif
        "        <app_msg_receive>%d</app_msg_receive>\n"
        "        <app_msg_send>%d</app_msg_send>\n"
        "        <async_file_debug>%d</async_file_debug>\n"
        "        <benchmark_debug>%d</benchmark_debug>\n"
        "        <checkpoint_debug>%d</checkpoint_debug>\n"
        "        <coproc_debug>%d</coproc_debug>\n"
        "        <cpu_sched>%d</cpu_sched>\n"
        "        <cpu_sched_debug>%d</cpu_sched_debug>\n"
        "        <cpu_sched_status>%d</cpu_sched_status>\n"
        "        <dcf_debug>%d</dcf_debug>\n"
        "        <disk_usage_debug>%d</disk_usage_debug>\n"
        "        <file_xfer_debug>%d</file_xfer_debug>\n"
        "        <gui_rpc_debug>%d</gui_rpc_debug>\n"
        "        <heartbeat_debug>%d</heartbeat_debug>\n"
        "        <http_debug>%d</http_debug>\n"
        "        <http_xfer_debug>%d</http_xfer_debug>\n"
        "        <idle_detection_debug>%d</idle_detection_debug>\n"
        "        <mem_usage_debug>%d</mem_usage_debug>\n"
        "        <network_status_debug>%d</network_status_debug>\n"
        "        <notice_debug>%d</notice_debug>\n"
        "        <poll_debug>%d</poll_debug>\n"
        "        <priority_debug>%d</priority_debug>\n"
        "        <proxy_debug>%d</proxy_debug>\n"
        "        <rr_simulation>%d</rr_simulation>\n"
        "        <rrsim_detail>%d</rrsim_detail>\n"
        "        <sched_op_debug>%d</sched_op_debug>\n"
        "        <scrsave_debug>%d</scrsave_debug>\n"
        "        <slot_debug>%d</slot_debug>\n"
        "        <sporadic_debug>%d</sporadic_debug>\n"
        "        <state_debug>%d</state_debug>\n"
        "        <statefile_debug>%d</statefile_debug>\n"
        "        <suspend_debug>%d</suspend_debug>\n"
        "        <task_debug>%d</task_debug>\n"
        "        <time_debug>%d</time_debug>\n"
        "        <trickle_debug>%d</trickle_debug>\n"
        "        <unparsed_xml>%d</unparsed_xml>\n"
        "        <work_fetch_debug>%d</work_fetch_debug>\n"
        "    </log_flags>\n",
        file_xfer ? 1 : 0,
        sched_ops ? 1 : 0,
        task ? 1 : 0,
#ifdef ANDROID
        android_debug ? 1 : 0,
#endif
        app_msg_receive ? 1 : 0,
        app_msg_send ? 1 : 0,
        async_file_debug ? 1 : 0,
        benchmark_debug ? 1 : 0,
        checkpoint_debug  ? 1 : 0,
        coproc_debug ? 1 : 0,
        cpu_sched ? 1 : 0,
        cpu_sched_debug ? 1 : 0,
        cpu_sched_status ? 1 : 0,
        dcf_debug ? 1 : 0,
        disk_usage_debug ? 1 : 0,
        file_xfer_debug ? 1 : 0,
        gui_rpc_debug ? 1 : 0,
        heartbeat_debug ? 1 : 0,
        http_debug ? 1 : 0,
        http_xfer_debug ? 1 : 0,
        idle_detection_debug ? 1 : 0,
        mem_usage_debug ? 1 : 0,
        network_status_debug ? 1 : 0,
        notice_debug ? 1 : 0,
        poll_debug ? 1 : 0,
        priority_debug ? 1 : 0,
        proxy_debug ? 1 : 0,
        rr_simulation ? 1 : 0,
        rrsim_detail ? 1 : 0,
        sched_op_debug ? 1 : 0,
        scrsave_debug ? 1 : 0,
        slot_debug ? 1 : 0,
        sporadic_debug ? 1 : 0,
        state_debug ? 1 : 0,
        statefile_debug ? 1 : 0,
        suspend_debug ? 1 : 0,
        task_debug ? 1 : 0,
        time_debug ? 1 : 0,
        trickle_debug ? 1 : 0,
        unparsed_xml ? 1 : 0,
        work_fetch_debug ? 1 : 0
    );
    return 0;
}

CC_CONFIG::CC_CONFIG() {
    defaults();
}

// this is called first thing by client
//
void CC_CONFIG::defaults() {
    abort_jobs_on_exit = false;
    allow_gui_rpc_get = false;
    allow_multiple_clients = false;
    allow_remote_gui_rpc = false;
    alt_platforms.clear();
    config_coprocs.clear();
    disallow_attach = false;
    dont_check_file_sizes = false;
    dont_contact_ref_site = false;
    dont_suspend_nci = false;
    dont_use_vbox = false;
    dont_use_wsl = false;
    disallowed_wsls.clear();
    dont_use_docker = false;
    exclude_gpus.clear();
    exclusive_apps.clear();
    exclusive_gpu_apps.clear();
    exit_after_finish = false;
    exit_before_start = false;
    exit_when_idle = false;
    fetch_minimal_work = false;
    fetch_on_update = false;
    force_auth = "default";
    http_1_0 = false;
    http_transfer_timeout = 300;
    http_transfer_timeout_bps = 10;
    for (int i=1; i<NPROC_TYPES; i++) {
        ignore_gpu_instance[i].clear();
    }
    ignore_tty.clear();
    lower_client_priority = false;
    max_event_log_lines = DEFAULT_MAX_EVENT_LOG_LINES;
    max_file_xfers = 8;
    max_file_xfers_per_project = 2;
    max_overdue_days = -1;
    max_stderr_file_size = 0;
    max_stdout_file_size = 0;
    max_tasks_reported = 0;
    ncpus = -1;
    no_alt_platform = false;
    no_disk_usage = false;
    no_gpus = false;
    no_info_fetch = false;
    no_opencl = false;
    no_priority_change = false;
    no_rdp_check = false;
    os_random_only = false;
    process_priority = CONFIG_PRIORITY_UNSPECIFIED;
    process_priority_special = CONFIG_PRIORITY_UNSPECIFIED;
    proxy_info.clear();
    rec_half_life = 10*86400;
#ifdef ANDROID
    report_results_immediately = true;
#else
    report_results_immediately = false;
#endif
    run_apps_manually = false;
    save_stats_days = 30;
    simple_gui_only = false;
    skip_cpu_benchmarks = false;
    start_delay = 0;
    stderr_head = false;
    suppress_net_info = false;
    unsigned_apps_ok = false;
    use_all_gpus = false;
    use_certs = false;
    use_certs_only = false;
    vbox_window = false;
}

int EXCLUDE_GPU::parse(XML_PARSER& xp) {
    bool found_url = false;
    type = "";
    appname = "";
    device_num = -1;
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/exclude_gpu")) {
            if (!found_url) return ERR_XML_PARSE;
            if (device_num >= MAX_COPROC_INSTANCES) {
                return ERR_XML_PARSE;
            }
            return 0;
        }
        if (xp.parse_string("url", url)) {
            canonicalize_master_url(url);
            found_url = true;
            continue;
        }
        if (xp.parse_int("device_num", device_num)) continue;
        if (xp.parse_string("type", type)) continue;
        if (xp.parse_string("app", appname)) continue;
    }
    return ERR_XML_PARSE;
}

// This is used by GUI RPC clients, NOT by the BOINC client
// KEEP IN SYNCH WITH CC_CONFIG::parse_options_client()!!
//
int CC_CONFIG::parse_options(XML_PARSER& xp) {
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
    exclude_gpus.clear();
    ignore_tty.clear();

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
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
            if (retval) return retval;
            c.specified_in_config = true;
            if (!strcmp(c.type, "CPU")) continue;
            config_coprocs.add(c);
            continue;
        }
        if (xp.parse_bool("disallow_attach", disallow_attach)) continue;
        if (xp.parse_bool("dont_check_file_sizes", dont_check_file_sizes)) continue;
        if (xp.parse_bool("dont_contact_ref_site", dont_contact_ref_site)) continue;
        if (xp.parse_bool("lower_client_priority", lower_client_priority)) continue;
        if (xp.parse_bool("dont_suspend_nci", dont_suspend_nci)) continue;
        if (xp.parse_bool("dont_use_vbox", dont_use_vbox)) continue;
        if (xp.parse_bool("dont_use_docker", dont_use_docker)) continue;
        if (xp.parse_bool("dont_use_wsl", dont_use_wsl)) continue;
        if (xp.parse_string("disallowed_wsl", s)) {
            disallowed_wsls.push_back(s);
            continue;
        }
        if (xp.match_tag("exclude_gpu")) {
            EXCLUDE_GPU eg;
            retval = eg.parse(xp);
            if (retval) return retval;
            exclude_gpus.push_back(eg);
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
        if (xp.parse_int("ignore_cuda_dev", n) || xp.parse_int("ignore_nvidia_dev", n)) {
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
#ifndef SIM
        if (xp.match_tag("proxy_info")) {
            proxy_info.parse_config(xp);
            continue;
        }
#endif
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

        xp.skip_unexpected(true, "CC_CONFIG::parse_options");
    }
    return ERR_XML_PARSE;
}

int CC_CONFIG::parse(XML_PARSER& xp, LOG_FLAGS& log_flags) {
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/cc_config")) return 0;
        if (xp.match_tag("log_flags")) {
            log_flags.parse(xp);
            continue;
        }
        if (xp.match_tag("options")) {
            parse_options(xp);
            continue;
        }
        if (xp.match_tag("options/")) continue;
        if (xp.match_tag("log_flags/")) continue;
    }
    return ERR_XML_PARSE;
}

void EXCLUDE_GPU::write(MIOFILE& out) {
    out.printf(
        "    <exclude_gpu>\n"
        "        <url>%s</url>\n"
        "        <device_num>%d</device_num>\n",
        url.c_str(),
        device_num
    );
    if (type.length()) {
        out.printf(
            "        <type>%s</type>\n",
            type.c_str()
        );
    }
    if (appname.length()) {
        out.printf(
            "        <app>%s</app>\n",
            appname.c_str()
        );
    }
    out.printf(
        "    </exclude_gpu>\n"
    );
}

int CC_CONFIG::write(MIOFILE& out, LOG_FLAGS& log_flags) {
    int j;
    unsigned int i;

    out.printf("<cc_config>\n");

    log_flags.write(out);

    out.printf(
        "    <options>\n"
        "        <abort_jobs_on_exit>%d</abort_jobs_on_exit>\n"
        "        <allow_gui_rpc_get>%d</allow_gui_rpc_get>\n"
        "        <allow_multiple_clients>%d</allow_multiple_clients>\n"
        "        <allow_remote_gui_rpc>%d</allow_remote_gui_rpc>\n",
        abort_jobs_on_exit ? 1 : 0,
        allow_gui_rpc_get ? 1 : 0,
        allow_multiple_clients ? 1 : 0,
        allow_remote_gui_rpc ? 1 : 0
    );

    for (i=0; i<alt_platforms.size(); ++i) {
        out.printf(
            "        <alt_platform>%s</alt_platform>\n",
            alt_platforms[i].c_str()
        );
    }

    for (int k=1; k<config_coprocs.n_rsc; k++) {
        if (!config_coprocs.coprocs[k].specified_in_config) continue;
        out.printf(
            "        <coproc>\n"
            "            <type>%s</type>\n"
            "            <count>%d</count>\n"
            "            <peak_flops>%f</peak_flops>\n"
            "            <device_nums>",
            config_coprocs.coprocs[k].type,
            config_coprocs.coprocs[k].count,
            config_coprocs.coprocs[k].peak_flops
        );
        for (j=0; j<config_coprocs.coprocs[k].count; j++) {
            out.printf("%d", config_coprocs.coprocs[k].device_nums[j]);
            if (j < (config_coprocs.coprocs[k].count - 1)) {
                out.printf(" ");
            }
        }
        out.printf(
            "</device_nums>\n"
            "        </coproc>\n"
        );
    }

    out.printf(
        "        <disallow_attach>%d</disallow_attach>\n"
        "        <dont_check_file_sizes>%d</dont_check_file_sizes>\n"
        "        <dont_contact_ref_site>%d</dont_contact_ref_site>\n"
        "        <lower_client_priority>%d</lower_client_priority>\n"
        "        <dont_suspend_nci>%d</dont_suspend_nci>\n"
        "        <dont_use_vbox>%d</dont_use_vbox>\n"
        "        <dont_use_wsl>%d</dont_use_wsl>\n"
        "        <dont_use_docker>%d</dont_use_docker>\n",
        disallow_attach,
        dont_check_file_sizes,
        dont_contact_ref_site,
        lower_client_priority,
        dont_suspend_nci,
        dont_use_vbox,
        dont_use_wsl,
        dont_use_docker
    );

    for (i=0; i<disallowed_wsls.size(); ++i) {
        out.printf(
            "        <disallowed_wsl>%s</disallowed_wsl>\n",
            disallowed_wsls[i].c_str()
        );
    }

    for (i=0; i<exclude_gpus.size(); i++) {
        exclude_gpus[i].write(out);
    }

    for (i=0; i<exclusive_apps.size(); ++i) {
        out.printf(
            "        <exclusive_app>%s</exclusive_app>\n",
            exclusive_apps[i].c_str()
        );
    }

    for (i=0; i<exclusive_gpu_apps.size(); ++i) {
        out.printf(
            "        <exclusive_gpu_app>%s</exclusive_gpu_app>\n",
            exclusive_gpu_apps[i].c_str()
        );
    }

    out.printf(
        "        <exit_after_finish>%d</exit_after_finish>\n"
        "        <exit_before_start>%d</exit_before_start>\n"
        "        <exit_when_idle>%d</exit_when_idle>\n"
        "        <fetch_minimal_work>%d</fetch_minimal_work>\n"
        "        <fetch_on_update>%d</fetch_on_update>\n"
        "        <force_auth>%s</force_auth>\n"
        "        <http_1_0>%d</http_1_0>\n"
        "        <http_transfer_timeout>%d</http_transfer_timeout>\n"
        "        <http_transfer_timeout_bps>%d</http_transfer_timeout_bps>\n",
        exit_after_finish,
        exit_before_start,
        exit_when_idle,
        fetch_minimal_work,
        fetch_on_update,
        force_auth.c_str(),
        http_1_0,
        http_transfer_timeout,
        http_transfer_timeout_bps
    );

    for (i=0; i<ignore_gpu_instance[PROC_TYPE_NVIDIA_GPU].size(); ++i) {
        out.printf(
            "        <ignore_nvidia_dev>%d</ignore_nvidia_dev>\n",
            ignore_gpu_instance[PROC_TYPE_NVIDIA_GPU][i]
        );
    }

    for (i=0; i<ignore_gpu_instance[PROC_TYPE_AMD_GPU].size(); ++i) {
        out.printf(
            "        <ignore_ati_dev>%d</ignore_ati_dev>\n",
            ignore_gpu_instance[PROC_TYPE_AMD_GPU][i]
        );
    }

    for (i=0; i<ignore_gpu_instance[PROC_TYPE_INTEL_GPU].size(); ++i) {
        out.printf(
            "        <ignore_intel_dev>%d</ignore_intel_dev>\n",
            ignore_gpu_instance[PROC_TYPE_INTEL_GPU][i]
        );
    }

    out.printf(
        "        <max_event_log_lines>%d</max_event_log_lines>\n"
        "        <max_file_xfers>%d</max_file_xfers>\n"
        "        <max_file_xfers_per_project>%d</max_file_xfers_per_project>\n"
        "        <max_overdue_days>%f</max_overdue_days>\n"
        "        <max_stderr_file_size>%f</max_stderr_file_size>\n"
        "        <max_stdout_file_size>%f</max_stdout_file_size>\n"
        "        <max_tasks_reported>%d</max_tasks_reported>\n"
        "        <ncpus>%d</ncpus>\n"
        "        <no_alt_platform>%d</no_alt_platform>\n"
        "        <no_disk_usage>%d</no_disk_usage>\n"
        "        <no_gpus>%d</no_gpus>\n"
        "        <no_info_fetch>%d</no_info_fetch>\n"
        "        <no_opencl>%d</no_opencl>\n"
        "        <no_priority_change>%d</no_priority_change>\n"
        "        <no_rdp_check>%d</no_rdp_check>\n"
        "        <os_random_only>%d</os_random_only>\n"
        "        <process_priority>%d</process_priority>\n"
        "        <process_priority_special>%d</process_priority_special>\n",
        max_event_log_lines,
        max_file_xfers,
        max_file_xfers_per_project,
        max_overdue_days,
        max_stderr_file_size,
        max_stdout_file_size,
        max_tasks_reported,
        ncpus,
        no_alt_platform,
        no_disk_usage,
        no_gpus,
        no_info_fetch,
        no_opencl,
        no_priority_change,
        no_rdp_check,
        os_random_only,
        process_priority,
        process_priority_special
    );

    proxy_info.write(out);

    out.printf(
        "        <rec_half_life_days>%f</rec_half_life_days>\n"
        "        <report_results_immediately>%d</report_results_immediately>\n"
        "        <run_apps_manually>%d</run_apps_manually>\n"
        "        <save_stats_days>%d</save_stats_days>\n"
        "        <skip_cpu_benchmarks>%d</skip_cpu_benchmarks>\n"
        "        <simple_gui_only>%d</simple_gui_only>\n"
        "        <start_delay>%f</start_delay>\n"
        "        <stderr_head>%d</stderr_head>\n"
        "        <suppress_net_info>%d</suppress_net_info>\n"
        "        <unsigned_apps_ok>%d</unsigned_apps_ok>\n"
        "        <use_all_gpus>%d</use_all_gpus>\n"
        "        <use_certs>%d</use_certs>\n"
        "        <use_certs_only>%d</use_certs_only>\n"
        "        <vbox_window>%d</vbox_window>\n",
        rec_half_life/86400,
        report_results_immediately,
        run_apps_manually,
        save_stats_days,
        skip_cpu_benchmarks,
        simple_gui_only,
        start_delay,
        stderr_head,
        suppress_net_info,
        unsigned_apps_ok,
        use_all_gpus,
        use_certs,
        use_certs_only,
        vbox_window
    );
    for (i=0; i<ignore_tty.size(); ++i) {
        out.printf(
            "        <ignore_tty>%s</ignore_tty>\n",
            ignore_tty[i].c_str()
        );
    }

    out.printf("    </options>\n</cc_config>\n");
    return 0;
}

// app_config.xml stuff

bool have_max_concurrent = false;
    // does any project have a max concurrent restriction?

int APP_CONFIG::parse_gpu_versions(
    XML_PARSER& xp, MSG_VEC& mv, LOG_FLAGS& log_flags
) {
    double x;
    char buf[1024];
    while (!xp.get_tag()) {
        if (xp.match_tag("/gpu_versions")) return 0;
        else if (xp.parse_double("gpu_usage", x)) {
            if (x <= 0) {
                mv.push_back(string("gpu_usage must be positive in app_config.xml"));
            } else {
                gpu_gpu_usage = x;
            }
            continue;
        }
        else if (xp.parse_double("cpu_usage", x)) {
            if (x < 0) {
                mv.push_back(string("cpu_usage must be non-negative in app_config.xml"));
            } else {
                gpu_cpu_usage = x;
            }
            continue;
        }
        if (log_flags.unparsed_xml) {
            snprintf(buf, sizeof(buf), "Unparsed line in app_config.xml: %.128s", xp.parsed_tag);
            mv.push_back(string(buf));
        }
    }
    mv.push_back(string("Missing </gpu_versions> in app_config.xml"));
    return ERR_XML_PARSE;
}

int APP_CONFIG::parse(XML_PARSER& xp, MSG_VEC& mv, LOG_FLAGS& log_flags) {
    char buf[1024];
    static const APP_CONFIG init;
    *this = init;

    while (!xp.get_tag()) {
        if (xp.match_tag("/app")) return 0;
        if (xp.parse_str("name", name, 256)) continue;
        if (xp.parse_int("max_concurrent", max_concurrent)) {
            if (max_concurrent) {
                have_max_concurrent = true;
            }
            continue;
        }
        if (xp.match_tag("gpu_versions")) {
            int retval = parse_gpu_versions(xp, mv, log_flags);
            if (retval) return retval;
            continue;
        }
        if (xp.parse_bool("fraction_done_exact", fraction_done_exact)) {
            continue;
        }
        if (xp.parse_bool("report_results_immediately", report_results_immediately)) {
            continue;
        }

        // unparsed XML not considered an error; maybe it should be?
        //
        if (log_flags.unparsed_xml) {
            snprintf(buf, sizeof(buf), "Unparsed line in app_config.xml: %.128s", xp.parsed_tag);
            mv.push_back(string(buf));
        }
        xp.skip_unexpected(log_flags.unparsed_xml, "APP_CONFIG::parse");
    }
    mv.push_back(string("Missing </app> in app_config.xml"));
    return ERR_XML_PARSE;
}

int APP_VERSION_CONFIG::parse(
    XML_PARSER& xp, MSG_VEC& mv, LOG_FLAGS& log_flags
) {
    char buf[1024];
    static const APP_VERSION_CONFIG init;
    *this = init;

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            snprintf(buf, sizeof(buf), "unexpected text '%.128s' in app_config.xml", xp.parsed_tag);
            mv.push_back(string(buf));
            return ERR_XML_PARSE;
        }
        if (xp.match_tag("/app_version")) return 0;
        if (xp.parse_str("app_name", app_name, 256)) continue;
        if (xp.parse_str("plan_class", plan_class, 256)) continue;
        if (xp.parse_str("cmdline", cmdline, 256)) continue;
        if (xp.parse_double("avg_ncpus", avg_ncpus)) continue;
        if (xp.parse_double("ngpus", ngpus)) continue;
        if (log_flags.unparsed_xml) {
            snprintf(buf, sizeof(buf), "Unparsed line in app_config.xml: %.128s", xp.parsed_tag);
            mv.push_back(string(buf));
        }
        xp.skip_unexpected(log_flags.unparsed_xml, "APP_VERSION_CONFIG::parse");
    }
    mv.push_back(string("Missing </app_version> in app_config.xml"));
    return ERR_XML_PARSE;
}

int APP_CONFIGS::parse(XML_PARSER& xp, MSG_VEC& mv, LOG_FLAGS& log_flags) {
    char buf[1024];
    int n;
    clear();
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            snprintf(buf, sizeof(buf), "unexpected text '%.128s' in app_config.xml", xp.parsed_tag);
            mv.push_back(string(buf));
            return ERR_XML_PARSE;
        }
        if (xp.match_tag("/app_config")) return 0;
        if (xp.match_tag("app")) {
            APP_CONFIG ac;
            int retval = ac.parse(xp, mv, log_flags);
            if (retval) return retval;
            app_configs.push_back(ac);
            if (ac.max_concurrent) {
                project_has_mc = true;
                project_min_mc = project_min_mc?std::min(project_min_mc, ac.max_concurrent):ac.max_concurrent;
            }
            continue;
        }
        if (xp.match_tag("app_version")) {
            APP_VERSION_CONFIG avc;
            int retval = avc.parse(xp, mv, log_flags);
            if (retval) return retval;
            app_version_configs.push_back(avc);
            continue;
        }
        if (xp.parse_int("project_max_concurrent", n)) {
            if (n > 0) {
                have_max_concurrent = true;
                project_has_mc = true;
                project_max_concurrent = n;
                project_min_mc = project_min_mc?std::min(project_min_mc, n):n;
            }
            continue;
        }
        if (xp.parse_bool("report_results_immediately", report_results_immediately)) {
            continue;
        }
        snprintf(buf, sizeof(buf), "Unknown tag in app_config.xml: %.128s", xp.parsed_tag);
        mv.push_back(string(buf));

        xp.skip_unexpected(log_flags.unparsed_xml, "APP_CONFIGS::parse");
    }
    mv.push_back(string("Missing </app_config> in app_config.xml"));
    return ERR_XML_PARSE;
}

int APP_CONFIGS::parse_file(FILE* f, MSG_VEC& mv, LOG_FLAGS& log_flags) {
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(f);
    if (!xp.parse_start("app_config")) {
        mv.push_back(string("Missing <app_config> in app_config.xml"));
        return ERR_XML_PARSE;
    }
    return parse(xp, mv, log_flags);
}

void APP_CONFIGS::write(MIOFILE& out) {
    out.printf(
        "<app_config>\n"
    );
    for (unsigned int i=0; i<app_configs.size(); i++) {
        APP_CONFIG& ac = app_configs[i];
        out.printf(
            "    <app>\n"
            "        <name>%s</name>\n"
            "        <max_concurrent>%d</max_concurrent>\n"
            "        <gpu_versions>\n"
            "            <gpu_usage>%f</gpu_usage>\n"
            "            <cpu_usage>%f</cpu_usage>\n"
            "        </gpu_versions>\n"
            "        <fraction_done_exact>%d</fraction_done_exact>\n"
            "        <report_results_immediately>%d</report_results_immediately>\n"
            "    </app>\n",
            ac.name,
            ac.max_concurrent,
            ac.gpu_gpu_usage,
            ac.gpu_cpu_usage,
            ac.fraction_done_exact?1:0,
            ac.report_results_immediately?1:0
        );
    }
    for (unsigned int i=0; i<app_version_configs.size(); i++) {
        APP_VERSION_CONFIG& avc = app_version_configs[i];
        out.printf(
            "    <app_version>\n"
            "        <app_name>%s</app_name>\n"
            "        <plan_class>%s</plan_class>\n"
            "        <cmdline>%s</cmdline>\n"
            "        <avg_ncpus>%f</avg_ncpus>\n"
            "        <ngpus>%f</ngpus>\n"
            "    </app_version>\n",
            avc.app_name,
            avc.plan_class,
            avc.cmdline,
            avc.avg_ncpus,
            avc.ngpus
        );
    }
    out.printf(
        "    <project_max_concurrent>%d</project_max_concurrent>\n"
        "    <report_results_immediately>%d</report_results_immediately>\n"
        "</app_config>\n",
        project_max_concurrent,
        report_results_immediately?1:0
    );
}

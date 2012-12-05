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

#include "cc_config.h"

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
#include "url.h"

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
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/log_flags")) return 0;
        if (xp.parse_bool("file_xfer", file_xfer)) continue;
        if (xp.parse_bool("sched_ops", sched_ops)) continue;
        if (xp.parse_bool("task", task)) continue;

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
        if (xp.parse_bool("priority_debug", priority_debug)) continue;
        if (xp.parse_bool("file_xfer_debug", file_xfer_debug)) continue;
        if (xp.parse_bool("gui_rpc_debug", gui_rpc_debug)) continue;
        if (xp.parse_bool("heartbeat_debug", heartbeat_debug)) continue;
        if (xp.parse_bool("http_debug", http_debug)) continue;
        if (xp.parse_bool("http_xfer_debug", http_xfer_debug)) continue;
        if (xp.parse_bool("mem_usage_debug", mem_usage_debug)) continue;
        if (xp.parse_bool("network_status_debug", network_status_debug)) continue;
        if (xp.parse_bool("poll_debug", poll_debug)) continue;
        if (xp.parse_bool("proxy_debug", proxy_debug)) continue;
        if (xp.parse_bool("rr_simulation", rr_simulation)) continue;
        if (xp.parse_bool("rrsim_detail", rrsim_detail)) continue;
        if (xp.parse_bool("sched_op_debug", sched_op_debug)) continue;
        if (xp.parse_bool("scrsave_debug", scrsave_debug)) continue;
        if (xp.parse_bool("slot_debug", slot_debug)) continue;
        if (xp.parse_bool("state_debug", state_debug)) continue;
        if (xp.parse_bool("statefile_debug", statefile_debug)) continue;
        if (xp.parse_bool("suspend_debug", suspend_debug)) continue;
        if (xp.parse_bool("task_debug", task_debug)) continue;
        if (xp.parse_bool("time_debug", time_debug)) continue;
        if (xp.parse_bool("trickle_debug", trickle_debug)) continue;
        if (xp.parse_bool("unparsed_xml", unparsed_xml)) continue;
        if (xp.parse_bool("work_fetch_debug", work_fetch_debug)) continue;
        if (xp.parse_bool("notice_debug", notice_debug)) continue;
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
        "        <priority_debug>%d</priority_debug>\n"
        "        <file_xfer_debug>%d</file_xfer_debug>\n"
        "        <gui_rpc_debug>%d</gui_rpc_debug>\n"
        "        <heartbeat_debug>%d</heartbeat_debug>\n"
        "        <http_debug>%d</http_debug>\n"
        "        <http_xfer_debug>%d</http_xfer_debug>\n"
        "        <mem_usage_debug>%d</mem_usage_debug>\n"
        "        <network_status_debug>%d</network_status_debug>\n"
        "        <poll_debug>%d</poll_debug>\n"
        "        <proxy_debug>%d</proxy_debug>\n"
        "        <rr_simulation>%d</rr_simulation>\n"
        "        <rrsim_detail>%d</rrsim_detail>\n"
        "        <sched_op_debug>%d</sched_op_debug>\n"
        "        <scrsave_debug>%d</scrsave_debug>\n"
        "        <slot_debug>%d</slot_debug>\n"
        "        <state_debug>%d</state_debug>\n"
        "        <statefile_debug>%d</statefile_debug>\n"
        "        <suspend_debug>%d</suspend_debug>\n"
        "        <task_debug>%d</task_debug>\n"
        "        <time_debug>%d</time_debug>\n"
        "        <trickle_debug>%d</trickle_debug>\n"
        "        <unparsed_xml>%d</unparsed_xml>\n"
        "        <work_fetch_debug>%d</work_fetch_debug>\n"
        "        <notice_debug>%d</notice_debug>\n"
        "    </log_flags>\n",
        file_xfer ? 1 : 0,
        sched_ops ? 1 : 0,
        task ? 1 : 0,
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
        priority_debug ? 1 : 0,
        file_xfer_debug ? 1 : 0,
        gui_rpc_debug ? 1 : 0,
        heartbeat_debug ? 1 : 0,
        http_debug ? 1 : 0,
        http_xfer_debug ? 1 : 0,
        mem_usage_debug ? 1 : 0,
        network_status_debug ? 1 : 0,
        poll_debug ? 1 : 0,
        proxy_debug ? 1 : 0,
        rr_simulation ? 1 : 0,
        rrsim_detail ? 1 : 0,
        sched_op_debug ? 1 : 0,
        scrsave_debug ? 1 : 0,
        slot_debug ? 1 : 0,
        state_debug ? 1 : 0,
        statefile_debug ? 1 : 0,
        suspend_debug ? 1 : 0,
        task_debug ? 1 : 0,
        time_debug ? 1 : 0,
        trickle_debug ? 1 : 0,
        unparsed_xml ? 1 : 0,
        work_fetch_debug ? 1 : 0,
        notice_debug ? 1 : 0
    );
    
    return 0;
}

CONFIG::CONFIG() {
}

// this is called first thing by client
//
void CONFIG::defaults() {
    abort_jobs_on_exit = false;
    allow_multiple_clients = false;
    allow_remote_gui_rpc = false;
    alt_platforms.clear();
    client_version_check_url = "http://boinc.berkeley.edu/download.php?xml=1";
    client_download_url = "http://boinc.berkeley.edu/download.php";
    config_coprocs.clear();
    data_dir[0] = 0;
    disallow_attach = false;
    dont_check_file_sizes = false;
    dont_contact_ref_site = false;
    exclude_gpus.clear();
    exclusive_apps.clear();
    exclusive_gpu_apps.clear();
    exit_after_finish = false;
    exit_before_start = false;
    exit_when_idle = false;
    fetch_minimal_work = false;
    force_auth = "default";
    http_1_0 = false;
    http_transfer_timeout = 300;
    http_transfer_timeout_bps = 10;
    ignore_nvidia_dev.clear();
    ignore_ati_dev.clear();
    ignore_intel_gpu_dev.clear();
    max_file_xfers = 8;
    max_file_xfers_per_project = 2;
    max_stderr_file_size = 0;
    max_stdout_file_size = 0;
    max_tasks_reported = 0;
    ncpus = -1;
    network_test_url = "http://www.google.com/";
    no_alt_platform = false;
    no_gpus = false;
    no_info_fetch = false;
    no_priority_change = false;
    os_random_only = false;
    proxy_info.clear();
    rec_half_life = 10*86400;
    report_results_immediately = false;
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
// KEEP IN SYNCH WITH CONFIG::parse_options_client()!!
//
int CONFIG::parse_options(XML_PARSER& xp) {
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
    ignore_nvidia_dev.clear();
    ignore_ati_dev.clear();
    ignore_intel_gpu_dev.clear();
    exclude_gpus.clear();

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/options")) {
            return 0;
        }
        if (xp.parse_bool("abort_jobs_on_exit", abort_jobs_on_exit)) continue;
        if (xp.parse_bool("allow_multiple_clients", allow_multiple_clients)) continue;
        if (xp.parse_bool("allow_remote_gui_rpc", allow_remote_gui_rpc)) continue;
        if (xp.parse_string("alt_platform", s)) {
            alt_platforms.push_back(s);
            continue;
        }
        if (xp.parse_string("client_download_url", client_download_url)) {
            downcase_string(client_download_url);
            continue;
        }
        if (xp.parse_string("client_version_check_url", client_version_check_url)) {
            downcase_string(client_version_check_url);
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
        if (xp.parse_str("data_dir", data_dir, sizeof(data_dir))) {
            continue;
        }
        if (xp.parse_bool("disallow_attach", disallow_attach)) continue;
        if (xp.parse_bool("dont_check_file_sizes", dont_check_file_sizes)) continue;
        if (xp.parse_bool("dont_contact_ref_site", dont_contact_ref_site)) continue;
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
        if (xp.parse_string("force_auth", force_auth)) {
            downcase_string(force_auth);
            continue;
        }
        if (xp.parse_bool("http_1_0", http_1_0)) continue;
        if (xp.parse_int("http_transfer_timeout", http_transfer_timeout)) continue;
        if (xp.parse_int("http_transfer_timeout_bps", http_transfer_timeout_bps)) continue;
        if (xp.parse_int("ignore_cuda_dev", n) || xp.parse_int("ignore_nvidia_dev", n)) {
            ignore_nvidia_dev.push_back(n);
            continue;
        }
        if (xp.parse_int("ignore_ati_dev", n)) {
            ignore_ati_dev.push_back(n);
            continue;
        }
        if (xp.parse_int("ignore_intel_gpu_dev", n)) {
            ignore_intel_gpu_dev.push_back(n);
            continue;
        }
        if (xp.parse_int("max_file_xfers", max_file_xfers)) continue;
        if (xp.parse_int("max_file_xfers_per_project", max_file_xfers_per_project)) continue;
        if (xp.parse_int("max_stderr_file_size", max_stderr_file_size)) continue;
        if (xp.parse_int("max_stdout_file_size", max_stdout_file_size)) continue;
        if (xp.parse_int("max_tasks_reported", max_tasks_reported)) continue;
        if (xp.parse_int("ncpus", ncpus)) continue;
        if (xp.parse_string("network_test_url", network_test_url)) {
            downcase_string(network_test_url);
            continue;
        }
        if (xp.parse_bool("no_alt_platform", no_alt_platform)) continue;
        if (xp.parse_bool("no_gpus", no_gpus)) continue;
        if (xp.parse_bool("no_info_fetch", no_info_fetch)) continue;
        if (xp.parse_bool("no_priority_change", no_priority_change)) continue;
        if (xp.parse_bool("os_random_only", os_random_only)) continue;
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

        xp.skip_unexpected(true, "CONFIG::parse_options");
    }
    return ERR_XML_PARSE;
}

int CONFIG::parse(XML_PARSER& xp, LOG_FLAGS& log_flags) {
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

int CONFIG::write(MIOFILE& out, LOG_FLAGS& log_flags) {
    int j;
    unsigned int i;

    out.printf("<set_cc_config>\n");
    out.printf("<cc_config>\n");

    log_flags.write(out);

    out.printf(
        "    <options>\n"
        "        <abort_jobs_on_exit>%d</abort_jobs_on_exit>\n"
        "        <allow_multiple_clients>%d</allow_multiple_clients>\n"
        "        <allow_remote_gui_rpc>%d</allow_remote_gui_rpc>\n",
        abort_jobs_on_exit ? 1 : 0,
        allow_multiple_clients ? 1 : 0,
        allow_remote_gui_rpc ? 1 : 0
    );

    for (i=0; i<alt_platforms.size(); ++i) {
        out.printf(
            "        <alt_platform>%s</alt_platform>\n",
            alt_platforms[i].c_str()
        );
    }
    
    out.printf(
        "        <client_version_check_url>%s</client_version_check_url>\n"
        "        <client_download_url>%s</client_download_url>\n",
        client_version_check_url.c_str(),
        client_download_url.c_str()
    );
    
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
    
    // Older versions of BOINC choke on empty data_dir string 
    if (strlen(data_dir)) {
        out.printf("        <data_dir>%s</data_dir>\n", data_dir);
    }
    
    out.printf(
        "        <disallow_attach>%d</disallow_attach>\n"
        "        <dont_check_file_sizes>%d</dont_check_file_sizes>\n"
        "        <dont_contact_ref_site>%d</dont_contact_ref_site>\n",
        disallow_attach,
        dont_check_file_sizes,
        dont_contact_ref_site
    );
    
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
        "        <force_auth>%s</force_auth>\n"
        "        <http_1_0>%d</http_1_0>\n"
        "        <http_transfer_timeout>%d</http_transfer_timeout>\n"
        "        <http_transfer_timeout_bps>%d</http_transfer_timeout_bps>\n",
        exit_after_finish,
        exit_before_start,
        exit_when_idle,
        fetch_minimal_work,
        force_auth.c_str(),
        http_1_0,
        http_transfer_timeout,
        http_transfer_timeout_bps
    );
        
    for (i=0; i<ignore_nvidia_dev.size(); ++i) {
        out.printf(
            "        <ignore_nvidia_dev>%d</ignore_nvidia_dev>\n",
            ignore_nvidia_dev[i]
        );
    }

    for (i=0; i<ignore_ati_dev.size(); ++i) {
        out.printf(
            "        <ignore_ati_dev>%d</ignore_ati_dev>\n",
            ignore_ati_dev[i]
        );
    }

    for (i=0; i<ignore_intel_gpu_dev.size(); ++i) {
        out.printf(
            "        <ignore_intel_gpu_dev>%d</ignore_intel_gpu_dev>\n",
            ignore_intel_gpu_dev[i]
        );
    }
        
    out.printf(
        "        <max_file_xfers>%d</max_file_xfers>\n"
        "        <max_file_xfers_per_project>%d</max_file_xfers_per_project>\n"
        "        <max_stderr_file_size>%d</max_stderr_file_size>\n"
        "        <max_stdout_file_size>%d</max_stdout_file_size>\n"
        "        <max_tasks_reported>%d</max_tasks_reported>\n"
        "        <ncpus>%d</ncpus>\n"
        "        <network_test_url>%s</network_test_url>\n"
        "        <no_alt_platform>%d</no_alt_platform>\n"
        "        <no_gpus>%d</no_gpus>\n"
        "        <no_info_fetch>%d</no_info_fetch>\n"
        "        <no_priority_change>%d</no_priority_change>\n"
        "        <os_random_only>%d</os_random_only>\n",
        max_file_xfers,
        max_file_xfers_per_project,
        max_stderr_file_size,
        max_stdout_file_size,
        max_tasks_reported,
        ncpus,
        network_test_url.c_str(),
        no_alt_platform,
        no_gpus,
        no_info_fetch,
        no_priority_change,
        os_random_only
    );
    
    proxy_info.write(out);
    
    out.printf(
        "        <rec_half_life_days>%f</rec_half_life_days>\n"
        "        <report_results_immediately>%d</report_results_immediately>\n"
        "        <run_apps_manually>%d</run_apps_manually>\n"
        "        <save_stats_days>%d</save_stats_days>\n"
        "        <skip_cpu_benchmarks>%d</skip_cpu_benchmarks>\n"
        "        <simple_gui_only>%d</simple_gui_only>\n"
        "        <start_delay>%d</start_delay>\n"
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

    out.printf("    </options>\n</cc_config>\n");
    out.printf("</set_cc_config>\n");
    return 0;
}

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
#ifdef _MSC_VER
#define chdir _chdir
#endif
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#endif

#include "error_numbers.h"
#include "common_defs.h"
#include "parse.h"
#include "str_util.h"
#include "filesys.h"

#include "file_names.h"
#include "client_state.h"
#include "client_msgs.h"
#include "cs_proxy.h"

using std::string;

LOG_FLAGS log_flags;
CONFIG config;

LOG_FLAGS::LOG_FLAGS() {
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
            msg_printf(NULL, MSG_USER_ALERT,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
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

        msg_printf(NULL, MSG_USER_ALERT, "Unrecognized tag in %s: <%s>\n",
            CONFIG_FILE, tag
        );
        xp.skip_unexpected(tag, true, "LOG_FLAGS::parse");
    }
    return ERR_XML_PARSE;
}

static void show_flag(char* buf, bool flag, const char* flag_name) {
    if (!flag) return;
    int n = (int)strlen(buf);
    if (!n) {
        strcpy(buf, flag_name);
        return;
    }
    strcat(buf, ", ");
    strcat(buf, flag_name);
    if (strlen(buf) > 60) {
        msg_printf(NULL, MSG_INFO, "log flags: %s", buf);
        strcpy(buf, "");
    }
}

void LOG_FLAGS::show() {
    char buf[256];
    strcpy(buf, "");
    show_flag(buf, file_xfer, "file_xfer");
    show_flag(buf, sched_ops, "sched_ops");
    show_flag(buf, task, "task");

    show_flag(buf, app_msg_receive, "app_msg_receive");
    show_flag(buf, app_msg_send, "app_msg_send");
    show_flag(buf, benchmark_debug, "benchmark_debug");
    show_flag(buf, checkpoint_debug, "checkpoint_debug");
    show_flag(buf, coproc_debug, "coproc_debug");
    show_flag(buf, cpu_sched, "cpu_sched");
    show_flag(buf, cpu_sched_debug, "cpu_sched_debug");
    show_flag(buf, cpu_sched_status, "cpu_sched_status");
    show_flag(buf, dcf_debug, "dcf_debug");
    show_flag(buf, debt_debug, "debt_debug");
    show_flag(buf, file_xfer_debug, "file_xfer_debug");
    show_flag(buf, gui_rpc_debug, "gui_rpc_debug");
    show_flag(buf, http_debug, "http_debug");
    show_flag(buf, http_xfer_debug, "http_xfer_debug");
    show_flag(buf, mem_usage_debug, "mem_usage_debug");
    show_flag(buf, network_status_debug, "network_status_debug");
    show_flag(buf, poll_debug, "poll_debug");
    show_flag(buf, proxy_debug, "proxy_debug");
    show_flag(buf, rr_simulation, "rr_simulation");
    show_flag(buf, sched_op_debug, "sched_op_debug");
    show_flag(buf, scrsave_debug, "scrsave_debug");
    show_flag(buf, slot_debug, "slot_debug");
    show_flag(buf, state_debug, "state_debug");
    show_flag(buf, statefile_debug, "statefile_debug");
    show_flag(buf, std_debug, "std_debug");
    show_flag(buf, task_debug, "task_debug");
    show_flag(buf, time_debug, "time_debug");
    show_flag(buf, unparsed_xml, "unparsed_xml");
    show_flag(buf, work_fetch_debug, "work_fetch_debug");
    show_flag(buf, notice_debug, "notice_debug");

    if (strlen(buf)) {
        msg_printf(NULL, MSG_INFO, "log flags: %s", buf);
    }
}

static void show_gpu_ignore(vector<int>& devs, const char* name) {
    for (unsigned int i=0; i<devs.size(); i++) {
        msg_printf(NULL, MSG_INFO, "Config: ignoring %s GPU %d", name, devs[i]);
    }
}

// TODO: show other config options
//
void CONFIG::show() {
    unsigned int i;
    if (ncpus>0) {
        msg_printf(NULL, MSG_INFO, "Config: simulate %d CPUs", config.ncpus);
    }
    if (no_gpus) {
        msg_printf(NULL, MSG_INFO, "Config: don't use coprocessors");
    }
    if (no_priority_change) {
        msg_printf(NULL, MSG_INFO, "Config: run apps at regular priority");
    }
    if (report_results_immediately) {
        msg_printf(NULL, MSG_INFO, "Config: report completed tasks immediately");
    }
    if (use_all_gpus) {
        msg_printf(NULL, MSG_INFO, "Config: use all coprocessors");
    }
    if (zero_debts) {
        msg_printf(NULL, MSG_INFO, "Config: zero long-term debts on startup");
    }
    show_gpu_ignore(ignore_cuda_dev, "NVIDIA");
    show_gpu_ignore(ignore_ati_dev, "ATI");
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
    if (allow_remote_gui_rpc) {
        msg_printf(NULL, MSG_INFO,
            "Config: GUI RPC allowed from any host"
        );
    }
    FILE* f = fopen(REMOTEHOST_FILE_NAME, "r");
    if (f) {
        msg_printf(NULL, MSG_INFO,
            "Config: GUI RPC allowed from:"
        );
        char buf[256];
        while (fgets(buf, 256, f)) {
            strip_whitespace(buf);
            if (!(buf[0] =='#' || buf[0] == ';') && strlen(buf) > 0 ) {
                msg_printf(NULL, MSG_INFO,
                    "Config:   %s", buf
                );
            }
        }
        fclose(f);
    }
}

CONFIG::CONFIG() {
    clear();
}

// this is called first thing by client
//
void CONFIG::clear() {
    abort_jobs_on_exit = false;
    allow_multiple_clients = false;
    allow_remote_gui_rpc = false;
    alt_platforms.clear();
    client_version_check_url = "http://boinc.berkeley.edu/download.php?xml=1";
    client_download_url = "http://boinc.berkeley.edu/download.php";
    disallow_attach = false;
    dont_check_file_sizes = false;
    dont_contact_ref_site = false;
    exclusive_apps.clear();
    exclusive_gpu_apps.clear();
    force_auth = "default";
    http_1_0 = false;
    ignore_cuda_dev.clear();
    ignore_ati_dev.clear();
    max_file_xfers = MAX_FILE_XFERS;
    max_file_xfers_per_project = MAX_FILE_XFERS_PER_PROJECT;
    max_stderr_file_size = 0;
    max_stdout_file_size = 0;
    ncpus = -1;
    network_test_url = "http://www.google.com/";
    no_alt_platform = false;
    no_gpus = false;
    no_priority_change = false;
    os_random_only = false;
    report_results_immediately = false;
    run_apps_manually = false;
    save_stats_days = 30;
    simple_gui_only = false;
    start_delay = 0;
    stderr_head = false;
    suppress_net_info = false;
    use_all_gpus = false;
    use_certs = false;
    use_certs_only = false;
    zero_debts = false;
}

int CONFIG::parse_options(XML_PARSER& xp) {
    char tag[1024], path[256];
    bool is_tag, btemp;
    string s;
    int n;

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
            msg_printf(NULL, MSG_USER_ALERT,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
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
        if (xp.parse_str(tag, "data_dir", path, sizeof(path))) {
            if (chdir(path)) {
                perror("chdir");
                exit(1);
            }
            continue;
        }
        if (xp.parse_bool(tag, "disallow_attach", disallow_attach)) continue;
        if (xp.parse_bool(tag, "dont_check_file_sizes", dont_check_file_sizes)) continue;
        if (xp.parse_bool(tag, "dont_contact_ref_site", dont_contact_ref_site)) continue;
        if (xp.parse_string(tag, "exclusive_app", s)) {
            exclusive_apps.push_back(s);
            continue;
        }
        if (xp.parse_string(tag, "exclusive_gpu_app", s)) {
            exclusive_gpu_apps.push_back(s);
            continue;
        }
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
        if (xp.parse_int(tag, "ncpus", ncpus)) continue;
        if (xp.parse_string(tag, "network_test_url", network_test_url)) {
            downcase_string(network_test_url);
            continue;
        }
        if (xp.parse_bool(tag, "no_alt_platform", no_alt_platform)) continue;
        if (xp.parse_bool(tag, "no_gpus", no_gpus)) continue;
        if (xp.parse_bool(tag, "no_priority_change", no_priority_change)) continue;
        if (xp.parse_bool(tag, "os_random_only", os_random_only)) continue;
#ifndef SIM
        if (!strcmp(tag, "proxy_info")) {
            int retval = config_proxy_info.parse_config(*xp.f);
            if (retval) return retval;
            continue;
        }
#endif
        if (xp.parse_bool(tag, "report_results_immediately", report_results_immediately)) continue;
        if (xp.parse_bool(tag, "run_apps_manually", run_apps_manually)) continue;
        if (xp.parse_int(tag, "save_stats_days", save_stats_days)) continue;
        if (xp.parse_bool(tag, "simple_gui_only", simple_gui_only)) continue;
        if (xp.parse_double(tag, "start_delay", start_delay)) continue;
        if (xp.parse_bool(tag, "stderr_head", stderr_head)) continue;
        if (xp.parse_bool(tag, "suppress_net_info", suppress_net_info)) continue;
        if (xp.parse_bool(tag, "use_all_gpus", use_all_gpus)) continue;
        if (xp.parse_bool(tag, "use_certs", use_certs)) continue;
        if (xp.parse_bool(tag, "use_certs_only", use_certs_only)) continue;
        if (xp.parse_bool(tag, "zero_debts", zero_debts)) continue;
        if (xp.parse_bool(tag, "skip_cpu_benchmarks", btemp)) {
            gstate.skip_cpu_benchmarks = btemp;
            continue;
        }
        if (xp.parse_bool(tag, "unsigned_apps_ok", btemp)) {
            gstate.unsigned_apps_ok = btemp;
            continue;
        }
        if (xp.parse_bool(tag, "exit_after_finish", btemp)) {
            gstate.exit_after_finish = btemp;
            continue;
        }

        msg_printf(NULL, MSG_USER_ALERT, "Unrecognized tag in %s: <%s>\n",
            CONFIG_FILE, tag
        );
        xp.skip_unexpected(tag, true, "CONFIG::parse_options");
    }
    return ERR_XML_PARSE;
}

int CONFIG::parse(FILE* f) {
    char tag[256];
    MIOFILE mf;
    XML_PARSER xp(&mf);
    bool is_tag;

    mf.init_file(f);
    if (!xp.parse_start("cc_config")) {
        msg_printf(NULL, MSG_USER_ALERT, "Missing start tag in %s", CONFIG_FILE);
        return ERR_XML_PARSE;
    }
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_USER_ALERT,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
            continue;
        }
        if (!strcmp(tag, "/cc_config")) return 0;
        if (!strcmp(tag, "log_flags")) {
            log_flags.parse(xp);
            continue;
        }
        if (!strcmp(tag, "options")) {
            parse_options(xp);
            continue;
        }
        msg_printf(NULL, MSG_USER_ALERT, "Unparsed tag in %s: <%s>\n",
            CONFIG_FILE, tag
        );
        xp.skip_unexpected(tag, true, "CONFIG.parse");
    }
    msg_printf(NULL, MSG_USER_ALERT, "Missing end tag in %s", CONFIG_FILE);
    return ERR_XML_PARSE;
}

int read_config_file(bool init) {
    FILE* f;

    if (!init) {
        msg_printf(NULL, MSG_INFO, "Re-reading cc_config.xml");
        config.clear();
    }
    f = boinc_fopen(CONFIG_FILE, "r");
    if (!f) return ERR_FOPEN;
    config.parse(f);
    fclose(f);
    return 0;
}


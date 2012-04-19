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

#include "common_defs.h"
#include "diagnostics.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"

#include "file_names.h"
#include "client_state.h"
#include "client_msgs.h"
#include "cs_proxy.h"

using std::string;

LOG_FLAGS log_flags;
CONFIG config;

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
    show_flag(buf, async_file_debug, "async_file_debug");
    show_flag(buf, benchmark_debug, "benchmark_debug");
    show_flag(buf, checkpoint_debug, "checkpoint_debug");
    show_flag(buf, coproc_debug, "coproc_debug");
    show_flag(buf, cpu_sched, "cpu_sched");
    show_flag(buf, cpu_sched_debug, "cpu_sched_debug");
    show_flag(buf, cpu_sched_status, "cpu_sched_status");
    show_flag(buf, dcf_debug, "dcf_debug");
    show_flag(buf, priority_debug, "priority_debug");
    show_flag(buf, file_xfer_debug, "file_xfer_debug");
    show_flag(buf, gui_rpc_debug, "gui_rpc_debug");
    show_flag(buf, heartbeat_debug, "heartbeat_debug");
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

static void show_exclude_gpu(EXCLUDE_GPU& e) {
    char t[256], app_name[256], dev[256];
    PROJECT *p = gstate.lookup_project(e.url.c_str());
    if (!p) return;
    if (e.type.empty()) {
        strcpy(t, "all");
    } else {
        strcpy(t, e.type.c_str());
    }
    if (e.appname.empty()) {
        strcpy(app_name, "all");
    } else {
        strcpy(app_name, e.appname.c_str());
    }
    if (e.device_num < 0) {
        strcpy(dev, "all");
    } else {
        sprintf(dev, "%d", e.device_num);
    }
    msg_printf(p, MSG_INFO,
        "Config: excluded GPU.  Type: %s.  App: %s.  Device: %s",
        t, app_name, dev
    );
    if (!e.appname.empty()) {
        APP* app = gstate.lookup_app(p, e.appname.c_str());
        if (!app) {
            string app_list;
            for (unsigned int i=0; i<gstate.apps.size(); i++) {
                app = gstate.apps[i];
                if (app->project != p) continue;
				if (!app_list.empty()) {
					app_list += ", ";
				}
                app_list += "'";
                app_list += app->name;
                app_list += "'";
            }
            msg_printf(p, MSG_USER_ALERT,
                "A GPU exclusion in your cc_config.xml file specifies a non-existent application '%s'.  Existing applications: %s",
                e.appname.c_str(),
                app_list.c_str()
            );
        }
    }
}

// Print config info.
// This is called during startup (after client_state.xml has been read)
// and also from the handle_read_cc_config GUI RPC.
//
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
    if (no_info_fetch) {
        msg_printf(NULL, MSG_INFO, "Config: don't fetch project list or client version info");
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
    if (fetch_minimal_work) {
        msg_printf(NULL, MSG_INFO, "Config: fetch minimal work");
    }
    show_gpu_ignore(ignore_nvidia_dev, GPU_TYPE_NVIDIA);
    show_gpu_ignore(ignore_ati_dev, GPU_TYPE_ATI);
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

// This is used by the BOINC client.
// KEEP IN SYNCH WITH CONFIG::parse_options()!!

int CONFIG::parse_options_client(XML_PARSER& xp) {
    char path[256];
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

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            msg_printf_notice(NULL, false,
                "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
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
            if (retval) {
                msg_printf_notice(NULL, false, NULL,
                    "Can't parse <coproc> element in cc_config.xml"
                );
            }
            retval = coprocs.add(c);
            if (retval) {
                msg_printf_notice(NULL, false, NULL,
                    "Duplicate <coproc> element in cc_config.xml"
                );
            }
            continue;
        }
        if (xp.parse_str("data_dir", path, sizeof(path))) {
            if (chdir(path)) {
                perror("chdir");
                exit(1);
            }
            continue;
        }
        if (xp.parse_bool("disallow_attach", disallow_attach)) continue;
        if (xp.parse_bool("dont_check_file_sizes", dont_check_file_sizes)) continue;
        if (xp.parse_bool("dont_contact_ref_site", dont_contact_ref_site)) continue;
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
        if (xp.parse_string("force_auth", force_auth)) {
            downcase_string(force_auth);
            continue;
        }
        if (xp.parse_bool("http_1_0", http_1_0)) continue;
        if (xp.parse_int("http_transfer_timeout", http_transfer_timeout)) continue;
        if (xp.parse_int("http_transfer_timeout_bps", http_transfer_timeout_bps)) continue;
        if (xp.parse_int("ignore_cuda_dev", n)||xp.parse_int("ignore_nvidia_dev", n)) {
            ignore_nvidia_dev.push_back(n);
            continue;
        }
        if (xp.parse_int("ignore_ati_dev", n)) {
            ignore_ati_dev.push_back(n);
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
            retval = config_proxy_info.parse_config(xp);
            if (retval) {
                msg_printf_notice(NULL, false, NULL,
                    "Can't parse <proxy_info> element in cc_config.xml"
                );
            }
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

        msg_printf_notice(NULL, false,
            "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
            "%s: <%s>",
            _("Unrecognized tag in cc_config.xml"),
            xp.parsed_tag
        );
        xp.skip_unexpected(true, "CONFIG::parse_options");
    }
    return ERR_XML_PARSE;
}

int CONFIG::parse_client(FILE* f) {
    MIOFILE mf;
    XML_PARSER xp(&mf);

    mf.init_file(f);
    if (!xp.parse_start("cc_config")) {
        msg_printf_notice(NULL, false,
            "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
            "%s",
            _("Missing start tag in cc_config.xml")
        );
        return ERR_XML_PARSE;
    }
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            msg_printf_notice(NULL, false,
                "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
                "%s: %s",
                _("Unexpected text in cc_config.xml"),
                xp.parsed_tag
            );
            continue;
        }
        if (xp.match_tag("/cc_config")) return 0;
        if (xp.match_tag("log_flags")) {
            log_flags.parse(xp);
            continue;
        }
        if (xp.match_tag("options")) {
            int retval = parse_options_client(xp);
            if (retval) {
                msg_printf_notice(NULL, false,
                    "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
                    "%s",
                    _("Error in cc_config.xml options")
                );
            }
            continue;
        }
        if (xp.match_tag("options/")) continue;
        if (xp.match_tag("log_flags/")) continue;
        msg_printf_notice(NULL, false,
            "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
            "%s: <%s>",
            _("Unrecognized tag in cc_config.xml"),
            xp.parsed_tag
        );
        xp.skip_unexpected(true, "CONFIG.parse");
    }
    msg_printf_notice(NULL, false,
        "http://boinc.berkeley.edu/manager_links.php?target=notice&controlid=config",
        "%s",
        _("Missing end tag in cc_config.xml")
    );
    return ERR_XML_PARSE;
}

int CONFIG::parse(FILE* f) {
    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);

    return parse(xp, log_flags);
}

int read_config_file(bool init, const char* fname) {
    if (!init) {
        msg_printf(NULL, MSG_INFO, "Re-reading %s", fname);
        config.defaults();
        log_flags.init();
    }
    FILE* f = boinc_fopen(fname, "r");
    if (!f) {
        msg_printf(NULL, MSG_INFO, "No config file found - using defaults");
        return ERR_FOPEN;
    }
    config.parse_client(f);
    fclose(f);
#ifndef SIM
    diagnostics_set_max_file_sizes(
        config.max_stdout_file_size, config.max_stderr_file_size
    );
#endif
    if (init) {
        coprocs = config.config_coprocs;
        config_proxy_info = config.proxy_info;
        if (strlen(config.data_dir)) {
#ifdef _WIN32
            _chdir(config.data_dir);
#else
            chdir(config.data_dir);
#endif
        }
    }
    return 0;
}

// Do stuff involving GPU exclusions.
// - Count excluded GPUS per project.
//   NOTE: this is currently done just on the project level.
//   Could do it at the app level also.
// - Flag app versions and results for which all GPUs are excluded
//
void process_gpu_exclusions() {
    unsigned int i, j;
    PROJECT *p;

    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        for (int k=1; k<coprocs.n_rsc; k++) {
            int n=0;
            COPROC& cp = coprocs.coprocs[k];
            for (j=0; j<config.exclude_gpus.size(); j++) {
                EXCLUDE_GPU& eg = config.exclude_gpus[j];
                if (strcmp(eg.url.c_str(), p->master_url)) continue;
                if (!eg.appname.empty()) continue;
                if (!eg.type.empty() && (eg.type != cp.type)) continue;
                if (eg.device_num >= 0) {
                    // exclusion may refer to nonexistent GPU
                    //
                    if (cp.device_num_exists(eg.device_num)) {
                        n++;
                    }
                } else {
                    n = cp.count;
                }
            }
            p->ncoprocs_excluded[k] = n;
        }
    }

    for (i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        if (avp->missing_coproc) continue;
        int rt = avp->gpu_usage.rsc_type;
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
        avp->missing_coproc = true;
        strcpy(avp->missing_coproc_name, "");
        for (j=0; j<gstate.results.size(); j++) {
            RESULT* rp = gstate.results[j];
            if (rp->avp != avp) continue;
            rp->coproc_missing = true;
            msg_printf(avp->project, MSG_INFO,
                "marking %s as coproc missing",
                rp->name
            );
        }
    }
}

bool gpu_excluded(APP* app, COPROC& cp, int ind) {
    PROJECT* p = app->project;
    for (unsigned int i=0; i<config.exclude_gpus.size(); i++) {
        EXCLUDE_GPU& eg = config.exclude_gpus[i];
        if (strcmp(eg.url.c_str(), p->master_url)) continue;
        if (!eg.type.empty() && (eg.type != cp.type)) continue;
        if (!eg.appname.empty() && (eg.appname != app->name)) continue;
        if (eg.device_num >= 0 && eg.device_num != cp.device_nums[ind]) continue;
        return true;
    }
    return false;
}

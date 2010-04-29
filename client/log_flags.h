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

// flags determining what is written to standard out.
// (errors go to stderr)
//
// NOTE: all writes to stdout should have an if (log_flags.*) {} around them.
//

#ifndef _LOGFLAGS_H_
#define _LOGFLAGS_H_

#include <vector>
#include <string>

#ifndef _WIN32
#include <cstdio>
#endif

#define MAX_FILE_XFERS_PER_PROJECT      2
#define MAX_FILE_XFERS                  8
    // kind of arbitrary

class XML_PARSER;

struct LOG_FLAGS {
    // on by default; intended for all users
    //
    bool file_xfer;
        // file transfer start and finish
    bool sched_ops;
        // interactions with schedulers
    bool task;
        // task start and finish

    // off by default; intended for developers and testers
    //
    bool app_msg_receive;
        // show shared-mem message from apps
    bool app_msg_send;
        // show shared-mem message to apps
    bool benchmark_debug;
        // debug CPU benchmarks
    bool checkpoint_debug;
    bool coproc_debug;
        // show coproc reserve/free and startup msgs
    bool cpu_sched;
        // preemption and resumption
    bool cpu_sched_debug;
        // explain scheduler decisions
    bool cpu_sched_status;
        // show what's running
    bool dcf_debug;
        // show changes to duration correction factors
    bool debt_debug;
        // changes to long-term debt
    bool file_xfer_debug;
        // show completion of FILE_XFER
    bool gui_rpc_debug;
    bool http_debug;
    bool http_xfer_debug;
    bool mem_usage_debug;
        // memory usage
    bool network_status_debug;
    bool poll_debug;
        // show what polls are responding
    bool proxy_debug;
    bool rr_simulation;
        // results of rr simulator
    bool sched_op_debug;
    bool scrsave_debug;
    bool slot_debug;
        // allocation of slots
    bool state_debug;
        // print textual summary of CLIENT_STATE initially
        // and after each scheduler RPC and garbage collect
        // also show actions of garbage collector
    bool statefile_debug;
        // show when and why state file is written
    bool std_debug;
        // changes to short-term debt
    bool task_debug;
        // task start and control details, and when apps checkpoint
    bool time_debug;
        // changes in on_frac, active_frac, connected_frac
    bool unparsed_xml;
        // show unparsed XML lines
    bool work_fetch_debug;
        // work fetch policy 
    bool notice_debug;

    LOG_FLAGS();
    int parse(XML_PARSER&);
    void show();
};

struct CONFIG {
    bool abort_jobs_on_exit;
    bool allow_multiple_clients;
    bool allow_remote_gui_rpc;
    std::vector<std::string> alt_platforms;
    std::string client_version_check_url;
    std::string client_download_url;
    bool disallow_attach;
    bool dont_check_file_sizes;
    bool dont_contact_ref_site;
    std::vector<std::string> exclusive_apps;
    std::vector<std::string> exclusive_gpu_apps;
    std::string force_auth;
    bool http_1_0;
    std::vector<int> ignore_cuda_dev;
    std::vector<int> ignore_ati_dev;
    int max_file_xfers;
    int max_file_xfers_per_project;
    int max_stderr_file_size;
    int max_stdout_file_size;
    int ncpus;
    std::string network_test_url;
    bool no_alt_platform;
    bool no_gpus;
    bool no_priority_change;
    bool os_random_only;
    bool report_results_immediately;
    bool run_apps_manually;
    int save_stats_days;
    bool simple_gui_only;
    double start_delay;
    bool stderr_head;
    bool suppress_net_info;
    bool use_all_gpus;
    bool use_certs;
    bool use_certs_only;
        // overrides use_certs
    bool zero_debts;

    CONFIG();
    void clear();
    int parse(FILE*);
    int parse_options(XML_PARSER&);
    void show();
};

extern LOG_FLAGS log_flags;
extern CONFIG config;
extern int read_config_file(bool init);

#endif

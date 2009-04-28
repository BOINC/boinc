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
#include <stdio.h>
#endif

#define MAX_FILE_XFERS_PER_PROJECT      2
#define MAX_FILE_XFERS                  8
    // kind of arbitrary

class XML_PARSER;

struct LOG_FLAGS {
    // on by default, user-readable
    //
        /// task start and finish
    bool task;
        /// file transfer start and finish
    bool file_xfer;
        /// interactions with schedulers
    bool sched_ops;

    // off by default; intended for developers and testers
    //
        /// preemption and resumption
    bool cpu_sched;
        /// explain scheduler decisions
    bool cpu_sched_debug;
        /// results of rr simulator
    bool rr_simulation;
        /// changes to debt
    bool debt_debug;
        /// task start and control details, and when apps checkpoint
    bool task_debug;
        /// work fetch policy 
    bool work_fetch_debug;

        /// show unparsed XML lines
    bool unparsed_xml;
        /// print textual summary of CLIENT_STATE initially
        /// and after each scheduler RPC and garbage collect
        /// also show actions of garbage collector
    bool state_debug;
        /// show when and why state file is written
    bool statefile_debug;
        /// show completion of FILE_XFER
    bool file_xfer_debug;
    bool sched_op_debug;
    bool http_debug;
    bool proxy_debug;
        /// changes in on_frac, active_frac, connected_frac
    bool time_debug;
    bool http_xfer_debug;
        /// debug CPU benchmarks
    bool benchmark_debug;
        /// show what polls are responding
    bool poll_debug;
    bool guirpc_debug;
    bool scrsave_debug;
        /// show shared-mem message to apps
    bool app_msg_send;
        /// show shared-mem message from apps
    bool app_msg_receive;
        /// memory usage
	bool mem_usage_debug;
	bool network_status_debug;
    bool checkpoint_debug;
        /// show coproc reserve/free
	bool coproc_debug;
        /// show changes to duration correction factors
    bool dcf_debug;

    LOG_FLAGS();
    int parse(XML_PARSER&);
    void show();
};

struct CONFIG {
    bool dont_check_file_sizes;
	bool http_1_0;
    int save_stats_days;
    int ncpus;
    int max_file_xfers;
    int max_file_xfers_per_project;
    bool suppress_net_info;
    bool disallow_attach;
    bool os_random_only;
    bool no_alt_platform;
    bool simple_gui_only;
    bool dont_contact_ref_site;
    std::vector<std::string> alt_platforms;
    int max_stdout_file_size;
    int max_stderr_file_size;
    bool report_results_immediately;
    double start_delay;
    bool run_apps_manually;
    std::string force_auth;
    bool allow_multiple_clients;
    bool use_certs;
    bool use_certs_only;
        // overrides use_certs
    std::vector<std::string> exclusive_apps;
    std::string client_version_check_url;
    std::string client_download_url;
    std::string network_test_url;
    bool no_gpus;
    bool zero_debts;
    bool no_priority_change;
    bool use_all_gpus;

    CONFIG();
    int parse(FILE*);
    int parse_options(XML_PARSER&);
    void show();
};

extern LOG_FLAGS log_flags;
extern CONFIG config;
extern int read_config_file(bool init);

#endif

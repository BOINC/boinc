// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
    bool task;              // task start and finish
    bool file_xfer;         // file transfer start and finish
    bool sched_ops;         // interactions with schedulers

    // off by default; intended for developers and testers
    //
    bool cpu_sched;         // preemption and resumption
    bool cpu_sched_debug;   // explain scheduler decisions
    bool rr_simulation;     // results of rr simulator
    bool debt_debug;        // changes to debt
    bool task_debug;        // task start and control details
                            // also prints when apps checkpoint
    bool work_fetch_debug;  // work fetch policy 

    bool unparsed_xml;      // show unparsed XML lines
    bool state_debug;       // print textual summary of CLIENT_STATE initially
                            // and after each scheduler RPC and garbage collect
                            // also show actions of garbage collector
                            // also show when state file is read or written
    bool file_xfer_debug;   // show completion of FILE_XFER
    bool sched_op_debug;
    bool http_debug;
    bool proxy_debug;
    bool time_debug;        // changes in on_frac, active_frac, connected_frac
    bool http_xfer_debug;
    bool benchmark_debug;   // debug CPU benchmarks
    bool poll_debug;        // show what polls are responding
    bool guirpc_debug;
    bool scrsave_debug;
    bool app_msg_send;       // show shared-mem message to apps
    bool app_msg_receive;    // show shared-mem message from apps
	bool mem_usage_debug;	 // memory usage
	bool network_status_debug;
    bool checkpoint_debug;

    LOG_FLAGS();
    void defaults();
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

    CONFIG();
    void defaults();
    int parse(FILE*);
    int parse_options(XML_PARSER&);
};

extern LOG_FLAGS log_flags;
extern CONFIG config;
extern int read_config_file();

#endif

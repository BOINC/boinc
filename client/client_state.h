// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#ifndef _CLIENT_STATE_
#define _CLIENT_STATE_

#include <vector>
#include <time.h>

#include "app.h"
#include "client_types.h"
#include "file_xfer.h"
#include "hostinfo.h"
#include "http.h"
#include "net_stats.h"
#include "net_xfer.h"
#include "pers_file_xfer.h"
#include "prefs.h"
#include "scheduler_op.h"
#include "time_stats.h"

// CLIENT_STATE is the global variables of the core client
// Most of the state is saved to and restored from "client_state.xml"
//
class CLIENT_STATE {
public:
    vector<PROJECT*> projects;
    vector<APP*> apps;
    vector<FILE_INFO*> file_infos;
    vector<APP_VERSION*> app_versions;
    vector<WORKUNIT*> workunits;
    vector<RESULT*> results;

    NET_XFER_SET* net_xfers;
    PERS_FILE_XFER_SET* pers_xfers;
    HTTP_OP_SET* http_ops;
    FILE_XFER_SET* file_xfers;
    ACTIVE_TASK_SET active_tasks;
    HOST_INFO host_info;
    PREFS* prefs;
    NET_STATS net_stats;

    CLIENT_STATE();
    int init();
    int restart_tasks();
    int exit();
    bool do_something();
    void parse_cmdline(int argc, char** argv);
    bool time_to_exit();
    bool run_time_tests();
    int time_tests();
    double current_disk_usage();
    double allowed_disk_usage();
    unsigned int giveup_after;

private:
    bool client_state_dirty;
    TIME_STATS time_stats;
    int core_client_version;
    char* platform_name;
    unsigned int nslots;
    bool exit_when_idle;
    bool run_time_test;
    bool activities_suspended;
    int exit_after;
    time_t app_started;

    int parse_state_file();
    int write_state_file();
    int write_state_file_if_needed();
    int link_app(PROJECT*, APP*);
    int link_file_info(PROJECT*, FILE_INFO*);
    int link_file_ref(PROJECT*, FILE_REF*);
    int link_app_version(PROJECT*, APP_VERSION*);
    int link_workunit(PROJECT*, WORKUNIT*);
    int link_result(PROJECT*, RESULT*);
    int latest_version_num(char*);
    int check_suspend_activities();
    int make_project_dirs();
    int make_slot_dirs();
    int exit_tasks();
    bool input_files_available(RESULT*);
    int app_finished(ACTIVE_TASK&);
    bool start_apps();
    bool handle_running_apps();
    bool handle_pers_file_xfers();
    void print_counts();
    bool garbage_collect();
    bool update_results();

    // stuff related to scheduler RPCs
    //
    SCHEDULER_OP* scheduler_op;
    bool contacted_sched_server;
    void compute_resource_debts();
public:
    bool start_new_file_xfer();
    PROJECT* next_project(PROJECT*);
    PROJECT* next_project_master_pending();
    double work_needed_secs();
    int make_scheduler_request(PROJECT*, double);
    void handle_scheduler_reply(PROJECT*, char* scheduler_url);
    void set_client_state_dirty(char*);
private:
    PROJECT* find_project_with_overdue_results();
    bool some_project_rpc_ok();
    bool scheduler_rpc_poll();
    void update_avg_cpu(PROJECT*);
    double estimate_duration(WORKUNIT*);
    double current_water_days();

    // the following could be eliminated by using map instead of vector
    //
public:
    PROJECT* lookup_project(char*);
    APP* lookup_app(PROJECT*, char*);
    FILE_INFO* lookup_file_info(PROJECT*, char* name);
    RESULT* lookup_result(PROJECT*, char*);
    WORKUNIT* lookup_workunit(PROJECT*, char*);
    APP_VERSION* lookup_app_version(APP*, int);
};

extern CLIENT_STATE gstate;

#endif

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

#include <vector>

#include "app.h"
#include "client_types.h"
#include "file_xfer.h"
#include "hostinfo.h"
#include "http.h"
#include "net_stats.h"
#include "net_xfer.h"
#include "prefs.h"
#include "scheduler_op.h"
#include "time_stats.h"

class CLIENT_STATE {
public:
    CLIENT_STATE();
    int init(PREFS*);
    int restart_tasks();
    int exit_tasks();
    bool do_something();
    void parse_cmdline(int argc, char** argv);
    bool time_to_exit();
    bool run_time_tests();
    int time_tests();

private:
    vector<PROJECT*> projects;
    vector<APP*> apps;
    vector<FILE_INFO*> file_infos;
    vector<APP_VERSION*> app_versions;
    vector<WORKUNIT*> workunits;
    vector<RESULT*> results;

    int version;
    char* platform_name;
    NET_XFER_SET* net_xfers;
    HTTP_OP_SET* http_ops;
    FILE_XFER_SET* file_xfers;
    SCHEDULER_OP* scheduler_op;
    ACTIVE_TASK_SET active_tasks;
    HOST_INFO host_info;
    PREFS* prefs;
    TIME_STATS time_stats;
    NET_STATS net_stats;
    unsigned int nslots;
    bool client_state_dirty;
    bool exit_when_idle;
    bool run_time_test;
    bool contacted_sched_server;
    bool activities_suspended;
    int exit_after;

    int parse_state_file();
    int write_state_file();
    int write_state_file_if_needed();
    int link_app(PROJECT*, APP*);
    int link_file_info(PROJECT*, FILE_INFO*);
    int link_file_ref(PROJECT*, FILE_REF*);
    int link_app_version(PROJECT*, APP_VERSION*);
    int link_workunit(PROJECT*, WORKUNIT*);
    int link_result(PROJECT*, RESULT*);
    int check_suspend_activities();
    bool need_work();
    void update_avg_cpu(PROJECT*);
    PROJECT* choose_project();
    int make_project_dirs();
    int make_slot_dirs();
    bool get_work();
    bool input_files_available(RESULT*);
    int app_finished(ACTIVE_TASK&);
    bool start_apps();
    bool handle_running_apps();
    bool start_file_xfers();
    void print_counts();
    bool garbage_collect();
    int make_scheduler_request(PROJECT*, int);
    void handle_scheduler_reply(SCHEDULER_OP&);
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

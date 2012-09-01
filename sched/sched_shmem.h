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

// The structure of the memory segment shared between
// the feeder and schedulers
// This is essentially a cache of DB contents:
// small static tables like app_version,
// and a queue of results waiting to be sent.

#ifndef _SCHED_SHMEM_H_
#define _SCHED_SHMEM_H_

#include "boinc_db.h"
#include "sched_util.h"
#include "sched_types.h"
#include "hr_info.h"

// the following must be at least as large as DB tables
// (counting only non-deprecated entries for the current major version)
// Increase as needed
//
#define MAX_PLATFORMS       50
#define MAX_APPS            10
#define MAX_APP_VERSIONS    50
#define MAX_ASSIGNMENTS     10

// Default number of work items in shared mem.
// You can configure this in config.xml (<shmem_work_items>)
// If you increase this above 100,
// you may exceed the max shared-memory segment size
// on some operating systems.
//
#define MAX_WU_RESULTS      100

// values of WU_RESULT.state
#define WR_STATE_EMPTY   0
#define WR_STATE_PRESENT 1
// If neither of the above, the value is the PID of a scheduler process
// that has this item reserved

// a workunit/result pair
struct WU_RESULT {
    int state;
        // EMPTY, PRESENT, or PID of locking process
    int infeasible_count;
    bool need_reliable;        // try to send to a reliable host
    WORKUNIT workunit;
    int resultid;
    int time_added_to_shared_memory;
    int res_priority;
    int res_server_state;
    double res_report_deadline;
    double fpops_size;      // measured in stdevs
};

// this struct is followed in memory by an array of WU_RESULTS
//
struct SCHED_SHMEM {
    bool ready;             // feeder sets to true when init done
        // the following fields let the scheduler make sure
        // that the shared mem has the right format
    int ss_size;            // size of this struct, including WU_RESULT array
    int platform_size;      // sizeof(PLATFORM)
    int app_size;           // sizeof(APP)
    int app_version_size;   // sizeof(APP_VERSION)
    int assignment_size;    // sizeof(ASSIGNMENT))
    int wu_result_size;     // sizeof(WU_RESULT)
    int nplatforms;
    int napps;
    double app_weight_sum;
    int napp_versions;
    int nassignments;
    int max_platforms;
    int max_apps;
    int max_app_versions;
    int max_assignments;
    int max_wu_results;
    bool locality_sched_lite;   // some app uses locality sched Lite
    bool have_nci_app;
    bool have_apps_for_proc_type[NPROC_TYPES];
    PERF_INFO perf_info;
    PLATFORM platforms[MAX_PLATFORMS];
    APP apps[MAX_APPS];
    APP_VERSION app_versions[MAX_APP_VERSIONS];
    ASSIGNMENT assignments[MAX_ASSIGNMENTS];
    WU_RESULT wu_results[0];

    void init(int nwu_results);
    int verify();
    int scan_tables();
    bool no_work(int pid);
    void restore_work(int pid);
#ifndef _USING_FCGI_
    void show(FILE*);
#else
    void show(FCGI_FILE*);
#endif

    APP* lookup_app(int);
    APP* lookup_app_name(char*);
    APP_VERSION* lookup_app_version(int);
    APP_VERSION* lookup_app_version_platform_plan_class(
        int platform, char* plan_class
    );
    PLATFORM* lookup_platform_id(int);
    PLATFORM* lookup_platform(char*);
};

#endif

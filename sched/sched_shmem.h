// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#include "boinc_db.h"

// the following must be at least as large as DB tables
// Increase as needed
//
#define MAX_PLATFORMS       50
#define MAX_APPS            10
#define MAX_APP_VERSIONS    1000
#define MAX_WU_RESULTS      1000
#define MAX_INFEASIBLE      500
    // if # of elements in work array that were infeasible for some host
    // exceeds this, classify some of them as COULDNT_SEND

// a workunit/result pair
struct WU_RESULT {
    bool present;
    int infeasible_count;
    WORKUNIT workunit;
    RESULT result;
};

struct SCHED_SHMEM {
    bool ready;             // feeder sets to true when init done
        // the following fields let the scheduler make sure
        // that the shared mem has the right format
    int ss_size;            // sizeof(SCHED_SHMEM)
    int platform_size;      // sizeof(PLATFORM)
    int app_size;           // sizeof(APP)
    int app_version_size;   // sizeof(APP_VERSION)
    int wu_result_size;     // sizeof(WU_RESULT)
    int nplatforms;
    int napps;
    int napp_versions;
    int nwu_results;
    int max_platforms;
    int max_apps;
    int max_app_versions;
    int max_wu_results;
    PLATFORM platforms[MAX_PLATFORMS];
    APP apps[MAX_APPS];
    APP_VERSION app_versions[MAX_APP_VERSIONS];
    WU_RESULT wu_results[MAX_WU_RESULTS];

    void init();
    int verify();
    int scan_tables();

    APP* lookup_app(int);
    APP_VERSION* lookup_app_version(int appid, int platform, int version);
    PLATFORM* lookup_platform(char*);
};


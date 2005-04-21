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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// The structure of the memory segment shared between
// the feeder and schedulers
// This is essentially a cache of DB contents:
// small static tables like app_version,
// and a queue of results waiting to be sent.

#include "boinc_db.h"

// the following must be at least as large as DB tables
// (counting only non-deprecated entries for the current major version)
// Increase as needed
//
#define MAX_PLATFORMS       50
#define MAX_APPS            10
#define MAX_APP_VERSIONS    50

// If you increase this above 100,
// you may exceed the max shared-memory segment size
// on some operating systems.
#define MAX_WU_RESULTS      100
//#define MAX_WU_RESULTS      500

// values of state field
#define WR_STATE_EMPTY   0
#define WR_STATE_PRESENT 1
#define WR_STATE_CHECKED_OUT    2
    // some server process is considering this result
// If none of the above, the value is a PID of a server
// that has this item reserved

// a workunit/result pair
struct WU_RESULT {
    int state;
    int infeasible_count;
    WORKUNIT workunit;
    int resultid;
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
    int ncore_versions;
    int nwu_results;
    int max_platforms;
    int max_apps;
    int max_app_versions;
    int max_core_versions;
    int max_wu_results;
    PLATFORM platforms[MAX_PLATFORMS];
    APP apps[MAX_APPS];
    APP_VERSION app_versions[MAX_APP_VERSIONS];
    WU_RESULT wu_results[MAX_WU_RESULTS];

    void init();
    int verify();
    int scan_tables();
    bool have_app(int);
    bool no_work(int pid);
    void restore_work(int pid);

    APP* lookup_app(int);
    APP_VERSION* lookup_app_version(int appid, int platform, int version);
    PLATFORM* lookup_platform(char*);
};


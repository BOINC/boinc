#include "db.h"

#ifndef BOINC_KEY
#define BOINC_KEY   0xdadacafe
#endif

#define MAX_PLATFORMS       50
#define MAX_APPS            10
#define MAX_APP_VERSIONS    100
#define MAX_WU_RESULTS      1000

// a workunit/result pair
struct WU_RESULT {
    bool present;
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


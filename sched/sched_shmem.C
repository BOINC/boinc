#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "db.h"

#include "sched_shmem.h"

void SCHED_SHMEM::init() {
    memset(this, 0, sizeof(SCHED_SHMEM));
    ss_size = sizeof(SCHED_SHMEM);
    platform_size = sizeof(PLATFORM);
    app_size = sizeof(APP);
    app_version_size = sizeof(APP_VERSION);
    wu_result_size = sizeof(WU_RESULT);
    max_platforms = MAX_PLATFORMS;
    max_apps = MAX_APPS;
    max_app_versions = MAX_APP_VERSIONS;
    max_wu_results = MAX_WU_RESULTS;
    nwu_results = MAX_WU_RESULTS;
}

int SCHED_SHMEM::verify() {
    if (ss_size != sizeof(SCHED_SHMEM)) return -1;
    if (platform_size != sizeof(PLATFORM)) return -1;
    if (app_size != sizeof(APP)) return -1;
    if (app_version_size != sizeof(APP_VERSION)) return -1;
    if (wu_result_size != sizeof(WU_RESULT)) return -1;
    if (max_platforms != MAX_PLATFORMS) return -1;
    if (max_apps != MAX_APPS) return -1;
    if (max_app_versions != MAX_APP_VERSIONS) return -1;
    if (max_wu_results != MAX_WU_RESULTS) return -1;
    return 0;
}

static void overflow(char* table) {
    assert(table!=NULL);
    fprintf(stderr,
        "The SCHED_SHMEM structure is too small for table %s.\n"
        "Increase the size and restart feeder and fcgi.\n",
        table
    );
}

int SCHED_SHMEM::scan_tables() {
    PLATFORM platform;
    APP app;
    APP_VERSION app_version;
    int n;

    n = 0;
    while (!db_platform_enum(platform)) {
        platforms[n++] = platform;
        if (n == MAX_PLATFORMS) overflow("platforms");
    }
    nplatforms = n;


    n = 0;
    while (!db_app_enum(app)) {
        apps[n++] = app;
        if (n == MAX_APPS) overflow("apps");
    }
    napps = n;

    n = 0;
    while (!db_app_version_enum(app_version)) {
        app_versions[n++] = app_version;
        if (n == MAX_APP_VERSIONS) overflow("app_versions");
    }
    napp_versions = n;
    return 0;
}

PLATFORM* SCHED_SHMEM::lookup_platform(char* name) {
    int i;
    assert(name!=NULL);
    for (i=0; i<nplatforms; i++) {
        if (!strcmp(platforms[i].name, name)) {
            return &platforms[i];
        }
    }
    return 0;
}

APP* SCHED_SHMEM::lookup_app(int id) {
    int i;

    for (i=0; i<napps; i++) {
        if (apps[i].id == id) {
            return &apps[i];
        }
    }
    return 0;
}

APP_VERSION* SCHED_SHMEM::lookup_app_version(
    int appid, int platformid, int version
) {
    int i;
    APP_VERSION* avp;
    assert(version>=0);
    for (i=0; i<napp_versions; i++) {
        avp = &app_versions[i];
        if (avp->appid == appid && avp->platformid == platformid && avp->version_num == version) {
            return avp;
        }
    }
    return 0;
}

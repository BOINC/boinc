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

// SCHED_SHMEM is the structure of a chunk of memory shared between
// the feeder (which reads from the database)
// and instances of the scheduling server

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "boinc_db.h"
#include "error_numbers.h"

#include "sched_shmem.h"
#include "sched_util.h"

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
    if (ss_size != sizeof(SCHED_SHMEM)) return ERR_SCHED_SHMEM;
    if (platform_size != sizeof(PLATFORM)) return ERR_SCHED_SHMEM;
    if (app_size != sizeof(APP)) return ERR_SCHED_SHMEM;
    if (app_version_size != sizeof(APP_VERSION)) return ERR_SCHED_SHMEM;
    if (wu_result_size != sizeof(WU_RESULT)) return ERR_SCHED_SHMEM;
    if (max_platforms != MAX_PLATFORMS) return ERR_SCHED_SHMEM;
    if (max_apps != MAX_APPS) return ERR_SCHED_SHMEM;
    if (max_app_versions != MAX_APP_VERSIONS) return ERR_SCHED_SHMEM;
    if (max_wu_results != MAX_WU_RESULTS) return ERR_SCHED_SHMEM;
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
    DB_PLATFORM platform;
    DB_APP app;
    DB_APP_VERSION app_version;
    int n;

    n = 0;
    while (!platform.enumerate()) {
        if (platform.deprecated) continue;
        platforms[n++] = platform;
        if (n == MAX_PLATFORMS) overflow("platforms");
    }
    nplatforms = n;

    n = 0;
    while (!app.enumerate()) {
        apps[n++] = app;
        if (n == MAX_APPS) overflow("apps");
    }
    napps = n;

    n = 0;
    while (!app_version.enumerate()) {
        if (app_version.version_num/100 != MAJOR_VERSION) continue;
        if (app_version.deprecated) continue;
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

// find the latest version for a given platform
//
APP_VERSION* SCHED_SHMEM::lookup_app_version(
    int appid, int platformid, int min_version
) {
    int i, best_version=-1;
    APP_VERSION* avp, *best_avp = 0;
    assert(min_version>=0);
    for (i=0; i<napp_versions; i++) {
        avp = &app_versions[i];
        if (avp->appid == appid && avp->platformid == platformid) {
            if (avp->version_num >= min_version && avp->version_num > best_version) {
                best_avp = avp;
                best_version = avp->version_num;
            }
        }
    }

    // log_messages.printf(
    //     SchedMessages::DEBUG,
    //     "SCHED_SHMEM::lookup_app_version(appid=%d, platformid=%d, min_version=%d): best_avp=%d\n",
    //     appid, platformid, min_version,
    //     best_version);

    return best_avp;
}

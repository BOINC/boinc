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

// SCHED_SHMEM is the structure of a chunk of memory shared between
// the feeder (which reads from the database)
// and instances of the scheduling server

#include <cstdio>
#include <cstring>
#include <cassert>

#include "boinc_db.h"
#include "error_numbers.h"

#include "sched_shmem.h"
#include "sched_util.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

void SCHED_SHMEM::init() {
    memset(this, 0, sizeof(SCHED_SHMEM));
    ss_size = sizeof(SCHED_SHMEM);
    platform_size = sizeof(PLATFORM);
    app_size = sizeof(APP);
    app_version_size = sizeof(APP_VERSION);
    core_version_size = sizeof(CORE_VERSION);
    wu_result_size = sizeof(WU_RESULT);
    max_platforms = MAX_PLATFORMS;
    max_apps = MAX_APPS;
    max_app_versions = MAX_APP_VERSIONS;
    max_core_versions = MAX_CORE_VERSIONS;
    max_wu_results = MAX_WU_RESULTS;
    nwu_results = MAX_WU_RESULTS;
}

int SCHED_SHMEM::verify() {
    if (ss_size != sizeof(SCHED_SHMEM)) return ERR_SCHED_SHMEM;
    if (platform_size != sizeof(PLATFORM)) return ERR_SCHED_SHMEM;
    if (app_size != sizeof(APP)) return ERR_SCHED_SHMEM;
    if (app_version_size != sizeof(APP_VERSION)) return ERR_SCHED_SHMEM;
    if (core_version_size != sizeof(CORE_VERSION)) return ERR_SCHED_SHMEM;
    if (wu_result_size != sizeof(WU_RESULT)) return ERR_SCHED_SHMEM;
    if (max_platforms != MAX_PLATFORMS) return ERR_SCHED_SHMEM;
    if (max_apps != MAX_APPS) return ERR_SCHED_SHMEM;
    if (max_app_versions != MAX_APP_VERSIONS) return ERR_SCHED_SHMEM;
    if (max_core_versions != MAX_CORE_VERSIONS) return ERR_SCHED_SHMEM;
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

bool SCHED_SHMEM::have_app(int appid) {
    int i;

    for (i=0; i<napps; i++) {
        if (apps[i].id == appid) return true;
    }
    return false;
}

int SCHED_SHMEM::scan_tables() {
    DB_PLATFORM platform;
    DB_APP app;
    DB_APP_VERSION app_version;
    DB_CORE_VERSION core_version;
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
        if (app.deprecated) continue;
        apps[n++] = app;
        if (n == MAX_APPS) overflow("apps");
    }
    napps = n;

    n = 0;
    while (!app_version.enumerate()) {
        if (app_version.version_num/100 != BOINC_MAJOR_VERSION) continue;
        if (app_version.deprecated) continue;
        if (!have_app(app_version.appid)) continue;
        app_versions[n++] = app_version;
        if (n == MAX_APP_VERSIONS) overflow("app_versions");
    }
    napp_versions = n;

    n = 0;
    while (!core_version.enumerate()) {
        if (core_version.version_num/100 != BOINC_MAJOR_VERSION) continue;
        if (core_version.deprecated) continue;
        core_versions[n++] = core_version;
        if (n == MAX_CORE_VERSIONS) overflow("core_versions");
    }
    ncore_versions = n;
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

// find the latest app version for a given platform
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

    return best_avp;
}

// find the latest core version for a given platform
//
CORE_VERSION* SCHED_SHMEM::lookup_core_version(int platformid) {
    CORE_VERSION* cvp, *best=0;
    int i;

    for (i=0; i<ncore_versions; i++) {
        cvp = &core_versions[i];
        if (!best || cvp->version_num > best->version_num) {
            best = cvp;
        }
    }
    return best;
}

bool SCHED_SHMEM::no_work(int pid) {
    int i;

    if (!ready) return true;
    for (i=0; i<max_wu_results; i++) {
        if (wu_results[i].state == WR_STATE_PRESENT) {
            wu_results[i].state = pid;
            return false;
        }
    }
    return true;
}

void SCHED_SHMEM::restore_work(int pid) {
    int i;

    for (i=0; i<max_wu_results; i++) {
        if (wu_results[i].state == pid) {
            wu_results[i].state = WR_STATE_PRESENT;
            return;
        }
    }
}

const char *BOINC_RCSID_e548c94703 = "$Id$";

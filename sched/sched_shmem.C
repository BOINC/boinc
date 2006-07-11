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

#include "config.h"
#include <cstdio>
#include <cstring>
#include <cassert>

#include "boinc_db.h"
#include "error_numbers.h"

#include "sched_shmem.h"
#include "sched_util.h"
#include "sched_msgs.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

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

static int error_return(const char* p) {
    fprintf(stderr, "Error in structure: %s\n", p);
    return ERR_SCHED_SHMEM;
}

int SCHED_SHMEM::verify() {
    if (ss_size != sizeof(SCHED_SHMEM)) return error_return("shmem");
    if (platform_size != sizeof(PLATFORM)) return error_return("platform");
    if (app_size != sizeof(APP)) return error_return("app");
    if (app_version_size != sizeof(APP_VERSION)) return error_return("app_version");
    if (wu_result_size != sizeof(WU_RESULT)) return error_return("wu_result");
    if (max_platforms != MAX_PLATFORMS) return error_return("max platform");
    if (max_apps != MAX_APPS) return error_return("max apps");
    if (max_app_versions != MAX_APP_VERSIONS) return error_return("max app version");
    if (max_wu_results != MAX_WU_RESULTS) return error_return("max wu_result");
    return 0;
}

static void overflow(const char* table, const char* param_name) {
    log_messages.printf(
        SCHED_MSG_LOG::MSG_CRITICAL,
        "The SCHED_SHMEM structure is too small for the %s table.\n"
        "Either increase the %s parameter in sched_shmem.h and recompile,\n"
        "or prune old rows from the table.\n"
        "Then restart the project.\n",
        table, param_name
    );
    exit(1);
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
    int n;

    n = 0;
    while (!platform.enumerate()) {
        if (platform.deprecated) continue;
        platforms[n++] = platform;
        if (n == MAX_PLATFORMS) {
            overflow("platforms", "MAX_PLATFORMS");
        }
    }
    nplatforms = n;

    n = 0;
    app_weights = 0;
    while (!app.enumerate()) {
        if (app.deprecated) continue;
        apps[n++] = app;
        if (n == MAX_APPS) {
            overflow("apps", "MAX_APPS");
        }
        app_weights += app.weight;
    }
    napps = n;

    n = 0;
    while (!app_version.enumerate()) {
        if (app_version.deprecated) continue;
        if (!have_app(app_version.appid)) continue;
        app_versions[n++] = app_version;
        if (n == MAX_APP_VERSIONS) {
            overflow("app_versions", "MAX_APP_VERSIONS");
        }
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

// see if there's any work.
// If there is, reserve it for this process
// (if we don't do this, there's a race condition where lots
// of servers try to get a single work item)
//
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

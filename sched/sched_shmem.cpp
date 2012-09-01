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

// SCHED_SHMEM is the structure of a chunk of memory shared between
// the feeder (which reads from the database)
// and instances of the scheduling server

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using std::vector;

#include "boinc_db.h"
#include "error_numbers.h"

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_types.h"
#include "sched_util.h"

#include "sched_shmem.h"


void SCHED_SHMEM::init(int nwu_results) {
    int size = sizeof(SCHED_SHMEM) + nwu_results*sizeof(WU_RESULT);
    memset(this, 0, size);
    ss_size = size;
    platform_size = sizeof(PLATFORM);
    app_size = sizeof(APP);
    app_version_size = sizeof(APP_VERSION);
    assignment_size = sizeof(ASSIGNMENT);
    wu_result_size = sizeof(WU_RESULT);
    max_platforms = MAX_PLATFORMS;
    max_apps = MAX_APPS;
    max_app_versions = MAX_APP_VERSIONS;
    max_assignments = MAX_ASSIGNMENTS;
    max_wu_results = nwu_results;
}

static int error_return(const char* p) {
    fprintf(stderr, "Error in structure: %s\n", p);
    return ERR_SCHED_SHMEM;
}

int SCHED_SHMEM::verify() {
    int size = sizeof(SCHED_SHMEM) + max_wu_results*sizeof(WU_RESULT);
    if (ss_size != size) return error_return("shmem");
    if (platform_size != sizeof(PLATFORM)) return error_return("platform");
    if (app_size != sizeof(APP)) return error_return("app");
    if (app_version_size != sizeof(APP_VERSION)) return error_return("app_version");
    if (assignment_size != sizeof(ASSIGNMENT)) return error_return("assignment");
    if (wu_result_size != sizeof(WU_RESULT)) return error_return("wu_result");
    if (max_platforms != MAX_PLATFORMS) return error_return("max platform");
    if (max_apps != MAX_APPS) return error_return("max apps");
    if (max_app_versions != MAX_APP_VERSIONS) return error_return("max app versions");
    if (max_assignments != MAX_ASSIGNMENTS) return error_return("max assignments");
    return 0;
}

static void overflow(const char* table, const char* param_name) {
    log_messages.printf(MSG_CRITICAL,
        "The SCHED_SHMEM structure is too small for the %s table.\n"
        "Either increase the %s parameter in sched_shmem.h and recompile,\n"
        "or prune old rows from the table.\n"
        "Then restart the project.\n",
        table, param_name
    );
    exit(1);
}

int SCHED_SHMEM::scan_tables() {
    DB_PLATFORM platform;
    DB_APP app;
    DB_APP_VERSION app_version;
    DB_ASSIGNMENT assignment;
    int i, j, n;

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
    app_weight_sum = 0;
    while (!app.enumerate()) {
        if (app.deprecated) continue;
        apps[n++] = app;
        if (n == MAX_APPS) {
            overflow("apps", "MAX_APPS");
        }
        app_weight_sum += app.weight;
        if (app.locality_scheduling == LOCALITY_SCHED_LITE) {
            locality_sched_lite = true;
        }
        if (app.non_cpu_intensive) {
            have_nci_app = true;
        }
        if (config.non_cpu_intensive) {
            have_nci_app = true;
            app.non_cpu_intensive = true;
        }
    }
    napps = n;

    n = 0;

    // for each (app, platform) pair,
    // get all versions with numbers maximal in their plan class.
    //
    for (i=0; i<nplatforms; i++) {
        PLATFORM& splatform = platforms[i];
        for (j=0; j<napps; j++) {
            APP& sapp = apps[j];
            vector<APP_VERSION> avs;
            char query[1024];
            sprintf(query,
                "where appid=%d and platformid=%d and deprecated=0",
                sapp.id, splatform.id
            );
            while (!app_version.enumerate(query)) {
                avs.push_back(app_version);
            }
            for (unsigned int k=0; k<avs.size(); k++) {
                APP_VERSION& av1 = avs[k];
                for (unsigned int kk=0; kk<avs.size(); kk++) {
                    if (k == kk) continue;
                    APP_VERSION& av2 = avs[kk];
                    if (!strcmp(av1.plan_class, av2.plan_class) && av1.version_num > av2.version_num) {
                        av2.deprecated = 1;
                    }
                }
            }
            for (unsigned int k=0; k<avs.size(); k++) {
                APP_VERSION& av1 = avs[k];
                if (av1.deprecated) continue;
                if (av1.min_core_version && av1.min_core_version < 10000) {
                    fprintf(stderr, "min core version too small - multiplying by 100\n");
                    av1.min_core_version *= 100;
                }
                if (av1.max_core_version && av1.max_core_version < 10000) {
                    fprintf(stderr, "max core version too small - multiplying by 100\n");
                    av1.max_core_version *= 100;
                }

                app_versions[n++] = av1;
                if (n == MAX_APP_VERSIONS) {
                    overflow("app_versions", "MAX_APP_VERSIONS");
                }
            }
        }
    }
    napp_versions = n;

    // see which resources we have app versions for
    //
    for (i=0; i<NPROC_TYPES; i++) {
        have_apps_for_proc_type[i] = false;
    }
    for (i=0; i<napp_versions; i++) {
        APP_VERSION& av = app_versions[i];
        if (strstr(av.plan_class, "cuda") || strstr(av.plan_class, "nvidia")) {
            have_apps_for_proc_type[PROC_TYPE_NVIDIA_GPU] = true;
        } else if (strstr(av.plan_class, "ati")) {
            have_apps_for_proc_type[PROC_TYPE_AMD_GPU] = true;
        } else if (strstr(av.plan_class, "intel_gpu")) {
            have_apps_for_proc_type[PROC_TYPE_INTEL_GPU] = true;
        } else {
            have_apps_for_proc_type[PROC_TYPE_CPU] = true;
        }
    }

    n = 0;
    while (!assignment.enumerate("where multi <> 0")) {
        assignments[n++] = assignment;
        if (n == MAX_ASSIGNMENTS) {
            overflow("assignments", "MAX_ASSIGNMENTS");
        }
    }
    nassignments = n;

    return 0;
}

PLATFORM* SCHED_SHMEM::lookup_platform(char* name) {
    for (int i=0; i<nplatforms; i++) {
        if (!strcmp(platforms[i].name, name)) {
            return &platforms[i];
        }
    }
    return NULL;
}

PLATFORM* SCHED_SHMEM::lookup_platform_id(int id) {
    for (int i=0; i<nplatforms; i++) {
        if (platforms[i].id == id) return &platforms[i];
    }
    return NULL;
}

APP* SCHED_SHMEM::lookup_app(int id) {
    for (int i=0; i<napps; i++) {
        if (apps[i].id == id) return &apps[i];
    }
    return NULL;
}

APP* SCHED_SHMEM::lookup_app_name(char* name) {
    for (int i=0; i<napps; i++) {
        if (!strcmp(name, apps[i].name)) return &apps[i];
    }
    return NULL;
}

APP_VERSION* SCHED_SHMEM::lookup_app_version(int id) {
    APP_VERSION* avp;
    for (int i=0; i<napp_versions; i++) {
        avp = &app_versions[i];
        if (avp->id == id) {
            return avp;
        }
    }
    return NULL;
}

APP_VERSION* SCHED_SHMEM::lookup_app_version_platform_plan_class(
    int platformid, char* plan_class
) {
    APP_VERSION* avp;
    for (int i=0; i<napp_versions; i++) {
        avp = &app_versions[i];
        if (avp->platformid == platformid && !strcmp(avp->plan_class, plan_class)) {
            return avp;
        }
    }
    return NULL;
}

// see if there's any work.
// If there is, reserve it for this process
// (if we don't do this, there's a race condition where lots
// of servers try to get a single work item)
//
bool SCHED_SHMEM::no_work(int pid) {
    if (!ready) return true;
    for (int i=0; i<max_wu_results; i++) {
        if (wu_results[i].state == WR_STATE_PRESENT) {
            wu_results[i].state = pid;
            return false;
        }
    }
    return true;
}

void SCHED_SHMEM::restore_work(int pid) {
    for (int i=0; i<max_wu_results; i++) {
        if (wu_results[i].state == pid) {
            wu_results[i].state = WR_STATE_PRESENT;
            return;
        }
    }
}

void SCHED_SHMEM::show(FILE* f) {
    fprintf(f, "app versions:\n");
    for (int i=0; i<napp_versions; i++) {
        APP_VERSION av = app_versions[i];
        fprintf(f, "appid: %d platformid: %d version_num: %d plan_class: %s\n",
            av.appid, av.platformid, av.version_num, av.plan_class
        );
    }
    for (int i=0; i<NPROC_TYPES; i++) {
        fprintf(f,
            "have %s apps: %s\n",
            proc_type_name(i),
            have_apps_for_proc_type[i]?"yes":"no"
        );
    }
    fprintf(f,
        "Jobs; key:\n"
        "ap: app ID\n"
        "ic: infeasible count\n"
        "wu: workunit ID\n"
        "rs: result ID\n"
        "hr: HR class\n"
        "nr: need reliable\n"
    );
    fprintf(f,
        "host fpops mean %f stddev %f\n",
        perf_info.host_fpops_mean, perf_info.host_fpops_stddev
    );
    fprintf(f,
        "host fpops 50th pctile %f 95th pctile %f\n",
        perf_info.host_fpops_50_percentile, perf_info.host_fpops_95_percentile
    );
    fprintf(f, "ready: %d\n", ready);
    fprintf(f, "max_wu_results: %d\n", max_wu_results);
    for (int i=0; i<max_wu_results; i++) {
        WU_RESULT& wu_result = wu_results[i];
        switch(wu_result.state) {
        case WR_STATE_PRESENT:
            fprintf(f, "%4d: ap %d ic %d wu %d rs %u hr %d nr %d\n",
                i, wu_result.workunit.appid, wu_result.infeasible_count,
                wu_result.workunit.id, wu_result.resultid,
                wu_result.workunit.hr_class, wu_result.need_reliable
            );
            break;
        case WR_STATE_EMPTY:
            fprintf(f, "%4d: ---\n", i);
            break;
        default:
            fprintf(f, "%4d: PID %d: result %u\n", i, wu_result.state, wu_result.resultid);
        }
    }
}

const char *BOINC_RCSID_e548c94703 = "$Id$";

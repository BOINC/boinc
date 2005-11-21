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

// -------------------------------
//
// feeder
//  [ -asynch ]           fork and run in a separate process
//  [ -d x ]              debug level x
//  [ -random_order ]     order by "random" field of result
//  [ -priority_order ]   order by "priority" field of result
//  [ -mod n i ]          handle only results with (id mod n) == i
//  [ -sleep_interval x ]   sleep x seconds if nothing to do
//  [ -allapps ]  gets workunits for all apps in the app table
//
// Creates a shared memory segment containing DB info,
// including the work array (results/workunits to send).
//
// Try to keep the work array filled.
// This is a little tricky.
// We use an enumerator.
// The inner loop scans the wu_result table,
// looking for empty slots and trying to fill them in.
// When the enumerator reaches the end, it is restarted;
// hopefully there will be some new workunits.
// There are two complications:
//
//  - An enumeration may return results already in the array.
//    So, for each result, we scan the entire array to make sure
//    it's not there already.  Can this be streamlined?
//
//  - We must avoid excessive re-enumeration,
//    especially when the number of results is less than the array size.
//    Crude approach: if a "collision" (as above) occurred on
//    a pass through the array, wait a long time (5 sec)
//
// Checking for infeasible results (i.e. can't sent to any host):
//
// - the "infeasible_count" field of WU_RESULT keeps track of
//   how many times the WU_RESULT was infeasible for a host
//
// - the scheduler gives priority to results that have infeasible_count > 0
//
// - If the infeasible_count of any result exceeds MAX_INFEASIBLE_COUNT,
//   the feeder flags the result as OVER with outcome COULDNT_SEND,
//   and flags the WU for the transitioner.
//
// - the feeder tries to ensure that the number of WU_RESULTs
//   with infeasible_count  > MAX_INFEASIBLE_THRESHOLD
//   doesn't exceed MAX_INFEASIBLE (defined in sched_shmem.h)
//   If it does, then the feeder picks the WU_RESULT with
//   the largest infeasible_count, marks if COULDNT_SEND as above,
//   and repeats this until the infeasible count is low enough again

// Trigger files:
// The feeder program periodically checks for two trigger files:
//
// stop_server:  destroy shmem and exit
//               leave trigger file there (for other daemons)
// reread_db:    update DB contents in existing shmem
//               delete trigger file

// If you get an "Invalid argument" error when trying to run the feeder,
// it is likely that you aren't able to allocate enough shared memory.
// Either increase the maximum shared memory segment size in the kernel
// configuration, or decrease the MAX_PLATFORMS, MAX_APPS
// MAX_APP_VERSIONS, and MAX_WU_RESULTS in sched_shmem.h

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "boinc_db.h"
#include "shmem.h"
#include "synch.h"
#include "util.h"
#include "sched_config.h"
#include "sched_shmem.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define DEFAULT_SLEEP_INTERVAL  5
#define ENUM_LIMIT 1000

// The following parameters determine the feeder's policy
// for purging "infeasible" results,
// i.e. those that are hard to send to any client.
// TODO: remove these from the source code,
// make them config.xml parameters

#define MAX_INFEASIBLE_THRESHOLD    2000
    // if a result's infeasible_count exceeds this,
    // count it as "possibly infeasible" (see the following)
    // TODO: lower this to 20 or so
#define MAX_INFEASIBLE      500
    // if # of possibly infeasibly results exceeds this,
    // classify some of them as COULDNT_SEND and remove from array
#define MAX_INFEASIBLE_COUNT    5000
    // a result's infeasible_count exceeds this,
    // classify as COULDNT_SEND and remove it from array
    // TODO: lower this to 50 or so

// Uncomment the following to enable this purging.
//
//#define REMOVE_INFEASIBLE_ENTRIES

#define REREAD_DB_FILENAME      "../reread_db"

SCHED_CONFIG config;
SCHED_SHMEM* ssp;
key_t sema_key;
const char* order_clause="";
char select_clause[256];
double sleep_interval = DEFAULT_SLEEP_INTERVAL;
bool all_apps = false;

void cleanup_shmem() {
    detach_shmem((void*)ssp);
    destroy_shmem(config.shmem_key);
}

int check_reread_trigger() {
    FILE* f;
    f = fopen(REREAD_DB_FILENAME, "r");
    if (f) {
        fclose(f);
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "Found trigger file %s; re-scanning database tables.\n",
            REREAD_DB_FILENAME
        );
        ssp->init();
        ssp->scan_tables();
        unlink(REREAD_DB_FILENAME);
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "Done re-scanning: trigger file removed.\n"
        );
    }
    return 0;
}

#ifdef REMOVE_INFEASIBLE_ENTRIES
static int remove_infeasible(int i) {
    char buf[256]; 
    int retval;
    DB_RESULT result;
    DB_WORKUNIT wu;

    WU_RESULT& wu_result = ssp->wu_results[i];
    wu_result.state = WR_STATE_EMPTY;
    result = wu_result.result;
    wu = wu_result.workunit;

    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
        "[%s] declaring result as unsendable; infeasible count %d\n",
        result.name, wu_result.infeasible_count
    );

    result.server_state = RESULT_SERVER_STATE_OVER;
    result.outcome = RESULT_OUTCOME_COULDNT_SEND;
    sprintf(
        buf, "server_state=%d, outcome=%d",
        result.server_state, result.outcome
    ); 
    retval = result.update_field(buf);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[%s]: can't update: %d\n",
            result.name, retval
        );
        return retval;
    }
    wu.transition_time = time(0);
    sprintf(buf, "transition_time=%d", wu.transition_time); 
    retval = wu.update_field(buf);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[%s]: can't update: %d\n",
            wu.name, retval
        );
        return retval;
    }

    return 0;
}
#endif

static void scan_work_array(
    DB_WORK_ITEM& wi,
    int& nadditions, int& ncollisions, int& /*ninfeasible*/,
    bool& no_wus
) {
    int i, j, retval;
    bool collision, restarted_enum = false;

    for (i=0; i<ssp->nwu_results; i++) {
        WU_RESULT& wu_result = ssp->wu_results[i];
        switch (wu_result.state) {
        case WR_STATE_CHECKED_OUT:
            break;
#ifdef REMOVE_INFEASIBLE_ENTRIES
        case WR_STATE_PRESENT:
            if (wu_result.infeasible_count > MAX_INFEASIBLE_COUNT) {
                remove_infeasible(i);
            } else if (wu_result.infeasible_count > MAX_INFEASIBLE_THRESHOLD) {
                ninfeasible++;
            }
            break;
#endif
        case WR_STATE_EMPTY:
            retval = wi.enumerate(ENUM_LIMIT, select_clause, order_clause, all_apps);
            if (retval) {

                // if we already restarted the enum on this array scan,
                // there's no point in doing it again.
                //
                if (restarted_enum) {
                    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                        "already restarted enum on this array scan\n"
                    );
                    return;
                }

                // restart the enumeration
                //
                restarted_enum = true;
                retval = wi.enumerate(ENUM_LIMIT, select_clause, order_clause, all_apps);
                log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                    "restarting enumeration\n"
                );
                if (retval) {
                    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                        "enumeration restart returned nothing\n"
                    );
                    no_wus = true;
                    return;
                }
            }

            // at this point "wi" has an item

            if (!ssp->have_app(wi.wu.appid)) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
                    "result [RESULT#%d] has bad appid %d; clean up your DB!\n",
                    wi.res_id, wi.wu.appid
                );
                exit(1);
            }
            collision = false;
            for (j=0; j<ssp->nwu_results; j++) {
                if (ssp->wu_results[j].state != WR_STATE_EMPTY
                    && ssp->wu_results[j].resultid == wi.res_id
                ) {
                    ncollisions++;
                    collision = true;
                    break;
                }
            }
            if (!collision) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "adding result [RESULT#%d] in slot %d\n",
                    wi.res_id, i
                );
                wu_result.resultid = wi.res_id;
                wu_result.workunit = wi.wu;
                wu_result.state = WR_STATE_PRESENT;
                wu_result.infeasible_count = 0;
                nadditions++;
            }
            break;
        default:
            // here the state is a PID; see if it's still alive
            //
            int pid = wu_result.state;
            struct stat s;
            char buf[256];
            sprintf(buf, "/proc/%d", pid);
            if (stat(buf, &s)) {
                wu_result.state = WR_STATE_PRESENT;
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "Result reserved by non-existent process PID %d; resetting\n",
                    pid
                );
            }
        }
    }
}

#ifdef REMOVE_INFEASIBLE_ENTRIES
static int remove_most_infeasible() {
    int i, max, imax=-1;

    max = 0;
    for (i=0; i<ssp->nwu_results; i++) {
        WU_RESULT& wu_result = ssp->wu_results[i];
        if (wu_result.state == WR_STATE_PRESENT && wu_result.infeasible_count > max) {
            imax = i;
            max = wu_result.infeasible_count;
        }
    }
    if (max == 0) return -1;        // nothing is infeasible

    return remove_infeasible(imax);
}
#endif

void feeder_loop() {
    int nadditions, ncollisions, ninfeasible;
    DB_WORK_ITEM wi;
    bool no_wus;

    while (1) {
        nadditions = 0;
        ncollisions = 0;
        ninfeasible = 0;
        no_wus = false;

        scan_work_array(
            wi, nadditions, ncollisions, ninfeasible, no_wus
        );

        ssp->ready = true;

#ifdef REMOVE_INFEASIBLE_ENTRIES
        int i, n, retval;
        if (ninfeasible > MAX_INFEASIBLE) {
            n = ninfeasible - MAX_INFEASIBLE;
            for (i=0; i<n; i++ ) {
                retval = remove_most_infeasible();
                if (retval) break;
            }
        }
#endif
        if (nadditions == 0) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG, "No results added; sleeping %.2f sec\n", sleep_interval);
            boinc_sleep(sleep_interval);
        } else {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG, "Added %d results to array\n", nadditions);
        }
        if (no_wus) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG, "No results available; sleeping %.2f sec\n", sleep_interval);
            boinc_sleep(sleep_interval);
        }
        if (ncollisions) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG, "Some results already in array - sleeping %.2f sec\n", sleep_interval);
            boinc_sleep(sleep_interval);
        }
        fflush(stdout);
        check_stop_daemons();
        check_reread_trigger();
    }
}

int main(int argc, char** argv) {
    int i, retval;
    bool asynch = false;
    void* p;
    char path[256];

    unlink(REREAD_DB_FILENAME);

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't parse config file\n");
        exit(1);
    }

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-random_order")) {
            order_clause = "order by random ";
        } else if (!strcmp(argv[i], "-allapps")) {
            all_apps = true;
        } else if (!strcmp(argv[i], "-priority_order")) {
            order_clause = "order by priority desc ";
        } else if (!strcmp(argv[i], "-mod")) {
            int n = atoi(argv[++i]);
            int j = atoi(argv[++i]);
            sprintf(select_clause, "and result.id %% %d = %d ", n, j);
        } else if (!strcmp(argv[i], "-sleep_interval")) {
            sleep_interval = atof(argv[++i]);
        } else {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "bad cmdline arg: %s\n", argv[i]
            );
            exit(1);
        }
    }

    if (asynch) {
        if (fork()!=0) {
            exit(0);
        }
    }

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "Starting\n");

    get_project_dir(path, sizeof(path));
    get_key(path, 'a', sema_key);
    destroy_semaphore(sema_key);
    create_semaphore(sema_key);

    retval = destroy_shmem(config.shmem_key);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't destroy shmem\n");
        exit(1);
    }
    retval = create_shmem(config.shmem_key, sizeof(SCHED_SHMEM), &p);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't create shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    ssp->init();

    atexit(cleanup_shmem);
    install_stop_signal_handler();

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "boinc_db.open: %d; %s\n", retval, boinc_db.error_string());
        exit(1);
    }
    ssp->scan_tables();

    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
        "feeder: read "
        "%d platforms, "
        "%d apps, "
        "%d app_versions\n",
        ssp->nplatforms,
        ssp->napps,
        ssp->napp_versions
    );

    feeder_loop();
}

const char *BOINC_RCSID_57c87aa242 = "$Id$";

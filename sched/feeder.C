// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002, 2003
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

// feeder [-asynch]
//
// Creates a shared memory segment containing DB info,
// including results/workunits to send.
// This means that the scheduler CGI program doesn't have to
// access the DB to get this info.
//
// -asynch      fork and run in a separate process

// TODO:
// - check for wu/results that don't get sent for a long time;
//   generate a warning message

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "boinc_db.h"
#include "shmem.h"
#include "util.h"
#include "config.h"
#include "sched_shmem.h"
#include "sched_util.h"

#define RESULTS_PER_ENUM    100
#define REREAD_DB_FILENAME      "reread_db"
#define LOCKFILE                "feeder.out"
#define PIDFILE                 "feeder.pid"

CONFIG config;

SCHED_SHMEM* ssp;

void cleanup_shmem()
{
    detach_shmem((void*)ssp);
    destroy_shmem(config.shmem_key);
}

int check_reread_trigger() {
    FILE* f;
    f = fopen(REREAD_DB_FILENAME, "r");
    if (f) {
        fclose(f);
        ssp->init();
        ssp->scan_tables();
        unlink(REREAD_DB_FILENAME);
    }

    return 0;
}

// Try keep the wu_results array filled.
// This is actually a little tricky.
// We use an enumerator.
// The inner loop scans the wu_result table,
// looking for empty slots and trying to fill them in.
// When the enumerator reaches the end, it is restarted;
// hopefully there will be some new workunits.
// There are two complications:
//  - An enumeration may return results already in the array.
//    So, for each result, we scan the entire array to make sure
//    it's not there already.  Can this be streamlined?
//  - We must avoid excessive re-enumeration,
//    especially when the number of results is less than the array size.
//    Crude approach: if a "collision" (as above) occurred on
//    a pass through the array, wait a long time (5 sec)
//
void feeder_loop() {
    int i, j, nadditions, ncollisions, retval;
    DB_RESULT result;
    DB_WORKUNIT wu;
    bool no_wus, collision, restarted_enum;
    char clause[256];

    sprintf(clause, "where server_state=%d order by random limit %d",
        RESULT_SERVER_STATE_UNSENT, RESULTS_PER_ENUM
    );

    while (1) {
        nadditions = 0;
        ncollisions = 0;
        no_wus = false;
        restarted_enum = false;
        for (i=0; i<ssp->nwu_results; i++) {
            if (!ssp->wu_results[i].present) {
try_again:
                retval = result.enumerate(clause);
                if (retval) {

                    // if we already restarted the enum on this pass,
                    // there's no point in doing it again.
                    //
                    if (restarted_enum) {
                        write_log(MSG_DEBUG, "already restarted enum on this pass\n");
                        break;
                    }

                    // restart the enumeration
                    //
                    restarted_enum = true;
                    retval = result.enumerate(clause);
                    write_log(MSG_DEBUG, "restarting enumeration\n");
                    if (retval) {
                        write_log(MSG_NORMAL, "enumeration restart returned nothing\n");
                        no_wus = true;
                        break;
                    }
                }

                // there's a chance this result was sent out
                // after the enumeration started.
                // So read it from the DB again
                //
                retval = result.lookup_id(result.id);
                if (retval) {
                    write_log(MSG_NORMAL, "can't reread result %s\n", result.name);
                    goto try_again;
                }
                if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
                    write_log(MSG_NORMAL, "RESULT STATE CHANGED: %s\n", result.name);
                    goto try_again;
                }
                collision = false;
                for (j=0; j<ssp->nwu_results; j++) {
                    if (ssp->wu_results[j].present
                        && ssp->wu_results[j].result.id == result.id
                    ) {
                        ncollisions++;
                        collision = true;
                        break;
                    }
                }
                if (!collision) {
                    write_log(MSG_DEBUG, "adding result %d in slot %d\n", result.id, i);
                    retval = wu.lookup_id(result.workunitid);
                    if (retval) {
                        write_log(MSG_CRITICAL, "can't read workunit %d: %d\n", result.workunitid, retval);
                        continue;
                    }
                    ssp->wu_results[i].result = result;
                    ssp->wu_results[i].workunit = wu;
                    ssp->wu_results[i].present = true;
                    nadditions++;
                }
            }
        }
        ssp->ready = true;
        if (nadditions == 0) {
            write_log(MSG_DEBUG, "no results added\n");
            sleep(1);
        } else {
            write_log(MSG_DEBUG, "added %d results to array\n", nadditions);
        }
        if (no_wus) {
            write_log(MSG_DEBUG, "feeder: no results available\n");
            sleep(5);
        }
        if (ncollisions) {
            write_log(MSG_DEBUG, "feeder: some results already in array - sleeping\n");
            sleep(5);
        }
        fflush(stdout);
        check_stop_trigger();
        check_reread_trigger();
    }
}

int main(int argc, char** argv) {
    int i, retval;
    bool asynch = false;
    void* p;

    unlink(REREAD_DB_FILENAME);

    retval = config.parse_file();
    if (retval) {
        write_log(MSG_CRITICAL, "can't parse config file\n");
        exit(1);
    }

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-d")) {
            set_debug_level(atoi(argv[++i]));
        }
    }

    if (asynch) {
        if (fork()!=0) {
            exit(0);
        }
    }

    // Call lock_file after fork(), because file locks are not always inherited
    if (lock_file(LOCKFILE)) {
        write_log(MSG_NORMAL, "Another copy of feeder is already running\n");
        exit(1);
    }
    write_pid_file(PIDFILE);
    write_log(MSG_NORMAL, "Starting\n");

    retval = destroy_shmem(config.shmem_key);
    if (retval) {
        write_log(MSG_CRITICAL, "can't destroy shmem\n");
        exit(1);
    }
    retval = create_shmem(config.shmem_key, sizeof(SCHED_SHMEM), &p);
    if (retval) {
        write_log(MSG_CRITICAL, "can't create shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    ssp->init();

    atexit(cleanup_shmem);
    install_sigint_handler();

    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        write_log(MSG_CRITICAL, "boinc_db_open: %d\n", retval);
        exit(1);
    }
    ssp->scan_tables();

    write_log(MSG_NORMAL,
              "feeder: read "
              "%d platforms, "
              "%d apps, "
              "%d app_versions\n",
              ssp->nplatforms,
              ssp->napps,
              ssp->napp_versions);

    feeder_loop();
}

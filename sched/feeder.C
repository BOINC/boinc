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
// Portions created by the SETI@home project are Copyright (C) 2002
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
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "db.h"
#include "shmem.h"
#include "config.h"
#include "sched_shmem.h"

#define RESULTS_PER_ENUM    100
#define STOP_SERVER_FILENAME    "stop_server"
#define REREAD_DB_FILENAME      "reread_db"

CONFIG config;

int check_triggers(SCHED_SHMEM* ssp) {
    FILE* f;

    f = fopen(STOP_SERVER_FILENAME, "r");
    if (f) {
        fclose(f);
        detach_shmem((void*)ssp);
        destroy_shmem(config.shmem_key);
        exit(0);
    }
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
//    a pass through the array, wait a long time (60 sec)
//
void feeder_loop(SCHED_SHMEM* ssp) {
    int i, j, nadditions, ncollisions, retval;
    RESULT result;
    WORKUNIT wu;
    bool no_wus, collision, restarted_enum;

    while (1) {
        nadditions = 0;
        ncollisions = 0;
        no_wus = false;
        restarted_enum = false;
        for (i=0; i<ssp->nwu_results; i++) {
            if (!ssp->wu_results[i].present) {
                result.state = RESULT_STATE_UNSENT;
                retval = db_result_enum_state(result, RESULTS_PER_ENUM);
                if (retval) {

                    // if we already restarted the enum on this pass,
                    // there's no point in doing it again.
                    //
                    if (restarted_enum) {
                        printf("feeder: already restarted enum\n");
                        break;
                    }

                    // restart the enumeration
                    //
                    restarted_enum = true;
                    result.state = RESULT_STATE_UNSENT;
                    retval = db_result_enum_state(result, RESULTS_PER_ENUM);
                    printf("feeder: restarting enumeration: %d\n", retval);
                    if (retval) {
                        printf("feeder: enumeration returned nothing\n");
                        no_wus = true;
                        break;
                    }
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
                    printf("feeder: adding result %d in slot %d\n", result.id, i);
                    retval = db_workunit(result.workunitid, wu);
                    if (retval) {
                        printf("feeder: can't read workunit %d: %d\n", result.workunitid, retval);
                        continue;
                    }
                    ssp->wu_results[i].result = result;
                    ssp->wu_results[i].workunit = wu;
                    ssp->wu_results[i].present = true;
                    nadditions++;
                }
            }
        }
        if (nadditions == 0) {
            printf("feeder: no results added\n");
            sleep(1);
        } else {
            printf("feeder: added %d results to array\n", nadditions);
        }
        if (no_wus) {
            printf("feeder: no results available\n");
            sleep(5);
        }
        if (ncollisions) {
            printf("feeder: some results already in array - sleeping\n");
            sleep(5);
        }
        fflush(stdout);
        check_triggers(ssp);
        ssp->ready = true;
    }
}

int main(int argc, char** argv) {
    SCHED_SHMEM* ssp;
    int i, retval;
    bool asynch = false;
    void* p;

    unlink(STOP_SERVER_FILENAME);
    unlink(REREAD_DB_FILENAME);

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "feeder: can't parse config file\n");
        exit(1);
    }

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        }
    }

    if (asynch) {
        if (fork()!=0) {
            exit(0);
        }
    }

    retval = destroy_shmem(config.shmem_key);
    if (retval) {
        fprintf(stderr, "feeder: can't destroy shmem\n");
        exit(1);
    }
    retval = create_shmem(config.shmem_key, sizeof(SCHED_SHMEM), &p);
    if (retval) {
        fprintf(stderr, "feeder: can't create shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    ssp->init();
    retval = db_open(config.db_name, config.db_passwd);
    if (retval) {
        fprintf(stderr, "feeder: db_open: %d\n", retval);
        exit(1);
    }
    ssp->scan_tables();

    printf(
        "feeder: read\n"
        "%d platforms\n"
        "%d apps\n"
        "%d app_versions\n",
        ssp->nplatforms,
        ssp->napps,
        ssp->napp_versions
    );

    feeder_loop(ssp);
}

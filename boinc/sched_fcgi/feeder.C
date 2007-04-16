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

// feeder.C [-asynch]
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
// - mechanism for rereading static tables (trigger file? signal?)

// Trigger file mechanism:
// The feeder program periodically checks for a file "feeder_trigger".
// It this file exists it contains a command to the feeder:
//
// <quit/>          destroy shmem and exit
// <reread_db/>     reread DB contents into existing shmem
//
// The feeder deletes the trigger file to indicate that it
// has completed the request.

#include <stdio.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _USING_FCGI_
#include "/usr/local/include/fcgi_stdio.h"
#endif

#include "db.h"
#include "shmem.h"
#include "sched_shmem.h"

#define RESULTS_PER_ENUM    100
#define TRIGGER_FILENAME    "feeder_trigger"

int check_trigger(SCHED_SHMEM* ssp) {
    FILE* f;
    char buf[256];

    f = fopen(TRIGGER_FILENAME, "r");
    if (!f) return 0;
    fread(buf, 1, 256, f);
    fclose(f);
    if (!strcmp(buf, "<quit/>\n")) {
        detach_shmem((void*)ssp);
        destroy_shmem(BOINC_KEY);
        unlink(TRIGGER_FILENAME);
        exit(0);
    } else if (!strcmp(buf, "<reread_db/>\n")) {
        ssp->init();
        ssp->scan_tables();
    } else {
        fprintf(stderr, "feeder: unknown command in trigger file: %s\n", buf);
    }
    unlink(TRIGGER_FILENAME);
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
                retval = db_result_enum_to_send(result, RESULTS_PER_ENUM);
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
                    retval = db_result_enum_to_send(result, RESULTS_PER_ENUM);
                    printf("feeder: restarting enumeration: %d\n", retval);
                    if (retval) {
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
                    printf("feeder: adding result %d\n", result.id);
                    retval = db_workunit(result.workunitid, wu);
                    if (retval) continue;
                    ssp->wu_results[i].result = result;
                    ssp->wu_results[i].workunit = wu;
                    ssp->wu_results[i].present = true;
                    nadditions++;
                }
            }
        }
        if (nadditions == 0) {
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
        check_trigger(ssp);
        ssp->ready = true;
    }
}

int main(int argc, char** argv) {
    SCHED_SHMEM* ssp;
    int i, retval;
    bool asynch = false;
    void* p;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        }
    }

    retval = destroy_shmem(BOINC_KEY);
    if (retval) {
        fprintf(stderr, "feeder: can't destroy shmem\n");
        exit(1);
    }
    retval = create_shmem(BOINC_KEY, sizeof(SCHED_SHMEM), &p);
    if (retval) {
        fprintf(stderr, "feeder: can't create shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    ssp->init();
    retval = db_open("boinc");
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

    if (asynch) {
        if (fork()==0) {
            feeder_loop(ssp);
        }
    } else {
        feeder_loop(ssp);
    }
}

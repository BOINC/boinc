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

// show_shmem: display WU part of shared-memory structure

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "shmem.h"
#include "sched_config.h"
#include "sched_shmem.h"

int main() {
    SCHED_SHMEM* ssp;
    int retval, i;
    void* p;
    SCHED_CONFIG config;

    retval = config.parse_file("..");
    if (retval) {
        printf("can't parse config\n");
        exit(1);
    }
    retval = attach_shmem(config.shmem_key, &p);
    if (retval) {
        printf("can't attach shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    retval = ssp->verify();
    printf("ready: %d\n", ssp->ready);
    printf("nwu_results: %d\n", ssp->nwu_results);
    printf("max_wu_results: %d\n", ssp->max_wu_results);
    for (i=0; i<ssp->max_wu_results; i++) {
        WU_RESULT& wu_result = ssp->wu_results[i];
        switch(wu_result.state) {
        case WR_STATE_PRESENT:
            printf("%d: present; infeasible_count %d; result %d\n",
                i, wu_result.infeasible_count, wu_result.resultid
            );
            break;
        case WR_STATE_EMPTY:
            printf("%d: absent\n", i);
            break;
        case WR_STATE_CHECKED_OUT:
            printf("%d: checked out: result %d\n", i, wu_result.resultid);
            break;
        default:
            printf("%d: PID %d: result %d\n", i, wu_result.state, wu_result.resultid);
        }
    }
}

#ifdef __GNUC__
static volatile const char  __attribute__((unused)) *BOINCrcsid="$Id$";
#else
static volatile const char *BOINCrcsid="$Id$";
#endif

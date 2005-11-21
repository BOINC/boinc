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

// show_shmem: display WU part of shared-memory structure
#include "config.h"
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

const char *BOINC_RCSID_a370415aab = "$Id$";

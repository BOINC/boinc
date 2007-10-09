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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// show_shmem: display work_item part of shared-memory structure

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

    retval = config.parse_file(".");
    if (retval) {
        printf("can't parse config: %d\n", retval);
        exit(1);
    }
    retval = attach_shmem(config.shmem_key, &p);
    if (retval) {
        printf("can't attach shmem: key %x\n", config.shmem_key);
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    retval = ssp->verify();
    ssp->show(stdout);
}

const char *BOINC_RCSID_a370415aab = "$Id$";

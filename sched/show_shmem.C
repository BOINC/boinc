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

// show_shmem: display WU part of shared-memory structure

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shmem.h"
#include "config.h"
#include "sched_shmem.h"

int main() {
    SCHED_SHMEM* ssp;
    int retval, i;
    void* p;
    CONFIG config;

    retval = config.parse_file();
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
    for (i=0; i<ssp->max_wu_results; i++) {
        printf("%d. %s\n", i, ssp->wu_results[i].present?"present":"absent");
    }
}

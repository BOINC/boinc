#include <stdio.h>

#include "/usr/local/include/fcgi_stdio.h"

#include "shmem.h"
#include "sched_shmem.h"

int main() {
    SCHED_SHMEM* ssp;
    int retval;
    void* p;

    retval = attach_shmem(BOINC_KEY, &p);
    if (retval) {
        printf("can't attach shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    retval = ssp->verify();
    printf("ready: %d\n", ssp->ready);
}

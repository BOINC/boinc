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

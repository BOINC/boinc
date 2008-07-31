#include "boinc_api.h"

struct UC_SHMEM {
    double update_time;
    double fraction_done;
    double cpu_time;
    BOINC_STATUS status;
    int countdown;
        // graphics app sets this to 5 repeatedly,
        // main program decrements it once/sec.
        // If it's zero, don't bother updating shmem
};

// test program for shmem functions

// -a       attach and sleep
// -d       destroy
// -c       create

#define KEY 0xbeefcafe

#include <cstring>
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

#include "shmem.h"

int main(int argc, char** argv) {
    void* p;
    int retval;

    if (!strcmp(argv[1], "-a")) {
        retval = attach_shmem(KEY, &p);
        sleep(60);
    } else if (!strcmp(argv[1], "-d")) {
        destroy_shmem(KEY);
    } else if (!strcmp(argv[1], "-c")) {
        create_shmem(KEY, 100, &p);
    }

    return 0;
}

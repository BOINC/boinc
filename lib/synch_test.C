static volatile const char *BOINCrcsid="$Id$";
// -c       create semaphore
// -d       destroy semaphore
// -l       lock semaphore, sleep 10 secs, unlock

#include <unistd.h>

#include "synch.h"

#define KEY 0xdeadbeef

int main(int argc, char** argv) {
    if (!strcmp(argv[1], "-c")) {
        create_semaphore(KEY);
    } else if (!strcmp(argv[1], "-d")) {
        destroy_semaphore(KEY);
    } else if (!strcmp(argv[1], "-l")) {
        lock_semaphore(KEY);
        sleep(10);
        unlock_semaphore(KEY);
    }

    return 0;
}

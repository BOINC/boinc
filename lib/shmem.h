// platform-independent interface to shared memory

#include <sys/shm.h>

// create a shared-memory segment of the given size.
//
extern int create_shmem(key_t, int size, void**);

// Destroy a shared-memory segment.
// If there are attachments to it,
// print a message in a loop until the attachments are gone
//
extern int destroy_shmem(key_t);

// attach to a shared-memory segment
//
extern int attach_shmem(key_t, void**);

// detach from a shared-mem segment
//
extern int detach_shmem(void*);

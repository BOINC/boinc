// platform-independent interface to shared memory

#if HAVE_SYS_SHM_H
#include <sys/shm.h>
#endif

#ifdef _WIN32

// create a shared-memory segment of the given size.
//
HANDLE create_shmem(LPCTSTR seg_name, int size, void** pp);

// attach to a shared-memory segment
//
HANDLE attach_shmem(LPCTSTR seg_name, void** pp);

// detach from a shared-mem segment.  Once all processes have
// detached, the segment is destroyed
//
int detach_shmem(HANDLE hSharedMem, void* p);

#else

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

extern int shmem_info(key_t key);

#endif

// platform-independent interface to shared memory

#ifndef BOINC_SHMEM_H
#define BOINC_SHMEM_H

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

#define SHM_SEG_SIZE            1024
#define NUM_SEGS            4
#define CORE_APP_WORKER_SEG 0
#define APP_CORE_WORKER_SEG 1
#define CORE_APP_GFX_SEG    2
#define APP_CORE_GFX_SEG    3

/* Shared memory is arranged as follows:
   4 1K segments
   First byte of each segment indicates whether
   segment contains unread data, remaining 1023
   bytes contain data
*/

class APP_CLIENT_SHM {
public:
    char *shm;

    bool pending_msg(int);    // returns true a message is waiting
                              // in the specified segment
	bool get_msg(char *,int);  // returns the message from the specified
                              // segment and resets the message flag
	bool send_msg(char *,int); // if there is not already a message in the segment,
                              // writes specified message and sets message flag
	void reset_msgs();        // resets all messages and clears their flags
};

#endif
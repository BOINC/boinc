// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

// platform-independent interface to shared memory

#ifndef BOINC_SHMEM_H
#define BOINC_SHMEM_H

#ifndef _WIN32
#if HAVE_SYS_SHM_H
#include <sys/shm.h>
#endif
#endif

#ifdef _WIN32

// create a shared-memory segment of the given size.
//
HANDLE create_shmem(LPCTSTR seg_name, int size, void** pp, bool disable_mapview);

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
#endif

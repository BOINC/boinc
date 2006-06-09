// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

// detach from a shared-mem segment.
// Once all processes have detached, the segment is destroyed
//
int detach_shmem(HANDLE hSharedMem, void* p);

#else

// create a shared-memory segment of the given size.
//
extern int create_shmem(key_t, int size, gid_t gid, void**);

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

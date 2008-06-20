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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// platform-independent interface to shared memory

#ifndef BOINC_SHMEM_H
#define BOINC_SHMEM_H

#ifndef _WIN32
#include <sys/types.h>
#include <sys/shm.h>
#endif

// create_shmem(): create a shared-memory segment of the given size.
// attach_shmem(): attach to a shared-memory segment
// detach_shmem(): detach from a shared-mem segment.
//    Once all processes have detached, the segment is destroyed
// The above with _mmap: V6 mmap() shared memory for Unix/Linux/Mac

#ifdef _WIN32
HANDLE create_shmem(
    LPCTSTR seg_name, int size, void** pp, bool try_global=true
);
HANDLE attach_shmem(LPCTSTR seg_name, void** pp);
int detach_shmem(HANDLE hSharedMem, void* p);
#else
#ifndef __EMX__
#define MMAPPED_FILE_NAME    "boinc_mmap_file"
extern int create_shmem_mmap(char *path, size_t size, void** pp);
extern int attach_shmem_mmap(char *path, void** pp);
extern int detach_shmem_mmap(void* p, size_t size);
#endif
extern int create_shmem(key_t, int size, gid_t gid, void**);
extern int attach_shmem(key_t, void**);
extern int detach_shmem(void*);
extern int shmem_info(key_t key);
// Destroy a shared-memory segment.
// If there are attachments to it,
// print a message in a loop until the attachments are gone
//
extern int destroy_shmem(key_t);
#endif      // !defined(_WIN32)

#endif      // BOINC_SHMEM_H

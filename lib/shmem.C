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

// interfaces for accessing shared memory segments

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef __EMX__
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
extern "C" int debug_printf(const char *fmt, ...);
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <assert.h>

#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#if HAVE_SYS_SHM_H
#if __FreeBSD__
#include <sys/param.h>
#endif
#include <sys/shm.h>
#endif
#endif

#ifdef USE_FILE_MAPPED_SHMEM
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#include "error_numbers.h"
#include "shmem.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

#ifdef _WIN32

HANDLE create_shmem(LPCTSTR seg_name, int size, void** pp, bool disable_mapview) {
    SECURITY_ATTRIBUTES security;
    HANDLE hMap;
    DWORD  dwError = 0;

    security.nLength = sizeof(security);
    security.lpSecurityDescriptor = NULL;
    security.bInheritHandle = TRUE;

    hMap = CreateFileMapping(INVALID_HANDLE_VALUE, &security, PAGE_READWRITE, 0, size, seg_name);
    dwError = GetLastError();
    if (disable_mapview && (NULL != hMap) && (ERROR_ALREADY_EXISTS == dwError)) {
        CloseHandle(hMap);
        hMap = NULL;
    }

    if (!disable_mapview && (NULL != hMap) && pp) {
        *pp = MapViewOfFile( hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
    }

    return hMap;
}

HANDLE attach_shmem(LPCTSTR seg_name, void** pp) {
    HANDLE hMap;

    hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, seg_name);
    if (!hMap) return NULL;

    if (pp) *pp = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    return hMap;
}

int detach_shmem(HANDLE hMap, void* p) {
    if (p) UnmapViewOfFile(p);
    CloseHandle(hMap);

    return 0;
}

#elif defined(__EMX__)

int create_shmem(key_t key, int size, void** pp) {
    APIRET rc;
    char   buf[256];

    sprintf(buf, "\\SHAREMEM\\BOINC\\%d", key);
    //debug_printf( "create_shmem %s, %d, %p\n", buf, size, pp);
    rc = DosAllocSharedMem(pp, (PSZ)buf, size, PAG_READ | PAG_WRITE | PAG_EXECUTE | PAG_COMMIT | OBJ_ANY);
    if (rc == ERROR_ALREADY_EXISTS)
       return attach_shmem( key, pp);
    if (rc)
       rc = DosAllocSharedMem(pp, (PSZ)buf, size, PAG_READ | PAG_WRITE | PAG_EXECUTE | PAG_COMMIT);

    if (rc) {
        //debug_printf( "DosAllocSharedMem %s failed, rc=%d\n", buf, rc);
        return ERR_SHMGET;
    }
    //debug_printf( "create_shmem %p\n", *pp);
    return 0;

}

int destroy_shmem(key_t key){
    APIRET rc;
    void*  pp;

    //debug_printf( "destroy_shmem %d\n", key);
    attach_shmem( key, &pp);
    rc = DosFreeMem(pp);
    if (rc) {
        //debug_printf( "DosFreeMem %d failed, rc=%d\n", key, rc);
        return ERR_SHMCTL;
    }
    //debug_printf( "destroy_shmem %d done\n", key);
    return 0;
}

int attach_shmem(key_t key, void** pp){
    APIRET rc;
    char   buf[256];

    sprintf(buf, "\\SHAREMEM\\BOINC\\%d", key);
    //debug_printf( "attach_shmem %s, %p\n", buf, pp);
    rc = DosGetNamedSharedMem(pp, (PSZ) buf, PAG_READ | PAG_WRITE);
    if (rc) {
        //debug_printf( "DosGetNamedSharedMem %s failed, rc=%d\n", buf, rc);
        return ERR_SHMAT;
    }
    //debug_printf( "attach_shmem %p\n", *pp);
    return 0;
}

int detach_shmem(void* p) {
    /* dummy */
    //debug_printf( "detach_shmem %p not supported\n", p);
    return 0;
}

#else

#ifdef USE_FILE_MAPPED_SHMEM

int create_shmem(char *path, size_t size, void** pp) {
    int fd, retval;
    struct stat sbuf;
    
    // NOTE: in principle it should be 0660, not 0666
    // (i.e. Apache should belong to the same group as the
    // project admin user, and should therefore be able to access the seg.
    // However, this doesn't seem to work on some Linux systems.
    // I don't have time to figure this out (31 July 07)
    // it's a big headache for anyone it affects,
    // and it's not a significant security issue.
    //
    fd = open(path, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
        return ERR_SHMGET;

    retval = fstat(fd, &sbuf);
    if (retval == 0) {
        if (sbuf.st_size < size) {
            // The following 2 lines extend the file and clear its new 
            // area to all zeros because they write beyond the old EOF. 
            // See the lseek man page for details.
            lseek(fd, size-1, SEEK_SET);
            write(fd, "\0", 1);
        }
    }
    
    if (retval) {
        perror("fcntl returned ");
       return ERR_SHMGET;
    }

    // mmap willl return NULL if size is 0
    *pp = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
    
    // Now close the file. The kernel doesnÕt use our file descriptor.
    close(fd);

    if (*pp == MAP_FAILED)
        return ERR_SHMGET;

    return 0;
}

int destroy_shmem(key_t key){
    return 0;
}

int detach_shmem(void* p, size_t size) {
    return munmap(p, size);
}

#else   // !defined(USE_FILE_MAPPED_SHMEM)

int create_shmem(key_t key, int size, gid_t gid, void** pp) {
    int id;
    
    // try 0666, then SHM_R|SHM_W
    // seems like some platforms require one or the other
    // (this may be superstition)
    //
    // NOTE: in principle it should be 0660, not 0666
    // (i.e. Apache should belong to the same group as the
    // project admin user, and should therefore be able to access the seg.
    // However, this doesn't seem to work on some Linux systems.
    // I don't have time to figure this out (31 July 07)
    // it's a big headache for anyone it affects,
    // and it's not a significant security issue.
    //
    id = shmget(key, size, IPC_CREAT|0666);
    if (id < 0) {
        id = shmget(key, size, IPC_CREAT|SHM_R|SHM_W);
    }
    if (id < 0) {
        perror("shmget");
        return ERR_SHMGET;
    }

    // set group ownership if requested
    //
    if (gid) {
        int retval;
        struct shmid_ds buf;
        // Set the shmem segment's group ID
        retval = shmctl(id, IPC_STAT, &buf);
        if (retval) {
            perror("shmget: shmctl STAT");
            return ERR_SHMGET;
        }
        buf.shm_perm.gid = gid;
        retval = shmctl(id, IPC_SET, &buf);
        if (retval) {
            perror("shmget: shmctl IPC_SET");
            return ERR_SHMGET;
        }
    }
    return attach_shmem(key, pp);
}

// Mark the shared memory segment so it will be released after 
// the last attached process detaches or exits.
// On Mac OS X and some other systems, not doing this causes 
// shared memory leaks if BOINC crashes or exits suddenly.
// On Mac OS X and some other systems, this command also 
// prevents any more processes from attaching (by clearing 
// the key in the shared memory structure), so BOINC does it 
// only after we are completey done with the segment.
int destroy_shmem(key_t key){
    struct shmid_ds buf;
    int id, retval;

    id = shmget(key, 0, 0);
    if (id < 0) return 0;           // assume it doesn't exist
    retval = shmctl(id, IPC_STAT, &buf);
    if (retval) {
        perror("shmctl STAT");
        return ERR_SHMCTL;
    }
    retval = shmctl(id, IPC_RMID, 0);
    if (retval) {
        perror("shmctl RMID");
        return ERR_SHMCTL;
    }
    return 0;
}

int attach_shmem(key_t key, void** pp){
    void* p;
    int id;

    id = shmget(key, 0, 0);
    if (id < 0) {
        perror("shmget");
        return ERR_SHMGET;
    }
    p = shmat(id, 0, 0);
    if ((long)p == -1) {
        perror("shmat");
        return ERR_SHMAT;
    }
    *pp = p;
    return 0;
}

int detach_shmem(void* p) {
    int retval;
    retval = shmdt((char *)p);
    return retval;
}

int print_shmem_info(key_t key) {
    int id;
    struct shmid_ds buf;

    id = shmget(key, 0, 0);
    if (id < 0) {
        return ERR_SHMGET;
    }
    shmctl(id, IPC_STAT, &buf);
    fprintf(
        stderr, "shmem key: %x\t\tid: %d, size: %d, nattach: %d\n",
        (unsigned int)key, id, (int)buf.shm_segsz, (int)buf.shm_nattch
    );

    return 0;
}

#endif  // USE_FILE_MAPPED_SHMEM

// For debugging shared memory logic
// For testing on Apple, Linux, UNIX systems with limited number 
// of shared memory segments per process and / or system-wide
// Mac OS X has a default limit of 8 segments per process, 32 system-wide
// If 
void stress_shmem(short reduce_by) {
#ifndef USE_FILE_MAPPED_SHMEM
    int retval;
    void * shmaddr[16];
    key_t key[] = {
        'BNC0', 'BNC1', 'BNC2', 'BNC3', 'BNC4', 'BNC5', 'BNC6', 'BNC7',
        'BNC8', 'BNC9', 'BNCA', 'BNCB', 'BNCC', 'BNCD', 'BNCE', 'BNCF' 
    };
    int i, id;
    
    if (reduce_by > 16) reduce_by = 16;
    
    // Tie up 5 of the 8 shared memory segments each process may have
    for (i=0; i<reduce_by; i++) {
        retval = create_shmem(key[i], 1024, 0, &shmaddr[i]);
        id = shmget(key[i], 0, 0);
        // Mark it for automatic destruction when BOINC exits
        if (id >= 0) {
            retval = shmctl(id, IPC_RMID, 0);
        }
    }
#endif  // !defined(USE_FILE_MAPPED_SHMEM)
}

#endif  // !defined(_WIN32) && !defined(__EMX__)

const char *BOINC_RCSID_f835f078de = "$Id$";

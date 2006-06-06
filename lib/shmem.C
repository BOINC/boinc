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

int create_shmem(key_t key, int size, void** pp) {
    int id;
    id = shmget(key, size, IPC_CREAT|0666);
    if (id < 0) {
        id = shmget(key, size, IPC_CREAT|SHM_R|SHM_W);
    }
    if (id < 0) {
        perror("shmget");
        return ERR_SHMGET;
    }
    return attach_shmem(key, pp);

}

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

#endif

const char *BOINC_RCSID_f835f078de = "$Id$";

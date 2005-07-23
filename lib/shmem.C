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

#ifndef _WIN32
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

#else

int create_shmem(key_t key, int size, void** pp) {
    int id;
    id = shmget(key, size, IPC_CREAT|0666);
    if (id < 0) {
        id = shmget(key, size, IPC_CREAT|SHM_R|SHM_W);
    }
    if (id < 0) {
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
    if (retval) return ERR_SHMCTL;
    retval = shmctl(id, IPC_RMID, 0);
    if (retval) {
        return ERR_SHMCTL;
    }
    return 0;
}

int attach_shmem(key_t key, void** pp){
    void* p;
    int id;

    id = shmget(key, 0, 0);
    if (id < 0) {
        return ERR_SHMGET;
    }
    p = shmat(id, 0, 0);
    if ((int)p == -1) {
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
        (unsigned int)key, id, buf.shm_segsz, (int)buf.shm_nattch
    );

    return 0;
}

#endif

const char *BOINC_RCSID_f835f078de = "$Id$";

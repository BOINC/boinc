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

// interfaces for accessing shared memory segments

#ifdef _WIN32
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

HANDLE create_shmem(LPCTSTR seg_name, int size, void** pp) {
    SECURITY_ATTRIBUTES security;
    HANDLE hSharedMem;
    LPVOID pMemPtr;

    security.nLength = sizeof(security);
    security.lpSecurityDescriptor = NULL;
    security.bInheritHandle = TRUE;

    hSharedMem = CreateFileMapping(INVALID_HANDLE_VALUE, &security,
        PAGE_READWRITE, 0, size, seg_name);
    if (!hSharedMem) return NULL;
    if (hSharedMem && (ERROR_ALREADY_EXISTS == GetLastError())) return NULL;

    pMemPtr = MapViewOfFile( hSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
    if (pp) *pp = pMemPtr;

    return hSharedMem;
}

HANDLE attach_shmem(LPCTSTR seg_name, void** pp) {
    HANDLE hSharedMem;
    LPVOID pMemPtr;

    hSharedMem = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, seg_name);
    if (!hSharedMem) return NULL;

    pMemPtr = MapViewOfFile( hSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
    *pp = pMemPtr;

    return hSharedMem;
}


int detach_shmem(HANDLE hSharedMem, void* p) {
    UnmapViewOfFile(p);
    CloseHandle(hSharedMem);

    return 0;
}

#else

int create_shmem(key_t key, int size, void** pp) {
    int id;
    char buf[256];
    assert(pp!=NULL);
    id = shmget(key, size, IPC_CREAT|0777);
    if (id < 0) {
        sprintf(buf, "create_shmem: shmget: key: %x size: %d", (unsigned int)key, size);
        perror(buf);
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
    if (buf.shm_nattch > 0) {
        fprintf(stderr,
            "destroy_shmem: %d attachments\n",
            (int)buf.shm_nattch
        );
    }
    retval = shmctl(id, IPC_RMID, 0);
    if (retval) {
        fprintf(stderr, "destroy_shmem: remove failed %d\n", retval);
        return ERR_SHMCTL;
    }
    return 0;
}

int attach_shmem(key_t key, void** pp){
    void* p;
    char buf[256];
    int id;
    assert(pp!=NULL);
    id = shmget(key, 0, 0);
    if (id < 0) {
        sprintf(buf, "attach_shmem: shmget: key: %x mem_addr: %p", (unsigned int)key, (void*)pp);
        perror(buf);
        return ERR_SHMGET;
    }
    p = shmat(id, 0, 0);
    if ((long)p == ERR_SHMAT) {
        sprintf(buf, "attach_shmem: shmat: key: %x mem_addr: %p", (unsigned int)key, (void*)pp);
        perror(buf);
        return ERR_SHMAT;
    }
    *pp = p;
    return 0;
}

int detach_shmem(void* p) {
    int retval;
    assert(p!=NULL);
    retval = shmdt((char *)p);
    if (retval) perror("detach_shmem: shmdt");
    return retval;
}

int shmem_info(key_t key) {
    int id;
    struct shmid_ds buf;
    char buf2[256];

    id = shmget(key, 0, 0);
    if (id < 0) {
        sprintf(buf2, "shmem_info: shmget: key: %x", (unsigned int)key);
        perror(buf2);
        return ERR_SHMGET;
    }
    shmctl(id, IPC_STAT, &buf);
    fprintf( stderr, "shmem key: %x\t\tid: %d, size: %d, nattach: %d\n",
            (unsigned int)key, id, buf.shm_segsz, (int)buf.shm_nattch );

    return 0;
}

#endif

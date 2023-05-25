// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// interfaces for accessing shared memory segments

#if defined(_WIN32)
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

#if(!defined (_WIN32) && !defined (__EMX__))
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "error_numbers.h"

#include "shmem.h"

// MAP_FILE isn't defined on most operating systems, and even then, it
// is often defined just for the sake of compatibility.  On those that
// don't define it, we will....
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#endif

#include "boinc_stdio.h"

#ifdef _WIN32

HANDLE create_shmem(LPCTSTR seg_name, int size, void** pp, bool try_global) {
    HANDLE hMap = NULL;
    DWORD dwError = 0;
    DWORD dwRes = 0;
    PSID pEveryoneSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS ea;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    SECURITY_ATTRIBUTES sa;
    OSVERSIONINFO osvi;
    char global_seg_name[256];

    // Win9X doesn't like any reference to a security descriptor.
    // So if we detect that we are running on the Win9X platform pass
    // a NULL value for it.
    //
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {

        hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, seg_name);
        dwError = GetLastError();

    } else {
        // Create a well-known SID for the Everyone group.
        if(!AllocateAndInitializeSid(&SIDAuthWorld, 1,
             SECURITY_WORLD_RID,
             0, 0, 0, 0, 0, 0, 0,
             &pEveryoneSID)
        ) {
            fprintf(stderr, "AllocateAndInitializeSid Error %u\n", GetLastError());
            goto Cleanup;
        }

        // Initialize an EXPLICIT_ACCESS structure for an ACE.
        // The ACE will allow Everyone all access to the shared memory object.
        ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
        ea.grfAccessPermissions = FILE_MAP_ALL_ACCESS;
        ea.grfAccessMode = SET_ACCESS;
        ea.grfInheritance= NO_INHERITANCE;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea.Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

        // Create a new ACL that contains the new ACEs.
        dwRes = SetEntriesInAcl(1, &ea, NULL, &pACL);
        if (ERROR_SUCCESS != dwRes) {
            fprintf(stderr, "SetEntriesInAcl Error %u\n", GetLastError());
            goto Cleanup;
        }

        // Initialize a security descriptor.
        pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (NULL == pSD) {
            fprintf(stderr, "LocalAlloc Error %u\n", GetLastError());
            goto Cleanup;
        }

        if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
            fprintf(stderr, "InitializeSecurityDescriptor Error %u\n", GetLastError());
            goto Cleanup;
        }

        // Add the ACL to the security descriptor.
        if (!SetSecurityDescriptorDacl(pSD,
                TRUE,     // bDaclPresent flag
                pACL,
                FALSE) // not a default DACL
        ) {
            fprintf(stderr,
                "SetSecurityDescriptorDacl Error %u\n", GetLastError()
            );
            goto Cleanup;
        }

        // Initialize a security attributes structure.
        sa.nLength = sizeof (SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;

        // Use the security attributes to set the security descriptor
        // when you create a shared file mapping.

        // Try using 'Global' so that it can cross terminal server sessions
        // The 'Global' prefix must be included in the shared memory
        // name if the shared memory segment is going to cross
        // terminal server session boundaries.
        //
        if (try_global) {
            snprintf(global_seg_name, sizeof(global_seg_name), "Global\\%s", seg_name);
            hMap = CreateFileMapping(
                INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0,
                size, global_seg_name
            );
            dwError = GetLastError();
            if (!hMap && (ERROR_ACCESS_DENIED == dwError)) {
                // Couldn't use the 'Global' tag, so try the original name.
                try_global = false;
            }
        }
        if (!try_global) {
            hMap = CreateFileMapping(
                INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, size, seg_name
            );
            dwError = GetLastError();
        }
    }

    if (hMap) {
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            CloseHandle(hMap);
            hMap = NULL;
        } else {
            *pp = MapViewOfFile( hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
        }
    }

Cleanup:

    if (osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
        if (pEveryoneSID)
            FreeSid(pEveryoneSID);
        if (pACL)
            LocalFree(pACL);
        if (pSD)
            LocalFree(pSD);
    }

    return hMap;
}

HANDLE attach_shmem(LPCTSTR seg_name, void** pp) {
    HANDLE hMap;
    char global_seg_name[256];

    // The 'Global' prefix must be included in the shared memory
    // name if the shared memory segment is going to cross
    // terminal server session boundaries.
    //
    snprintf(global_seg_name, sizeof(global_seg_name), "Global\\%s", seg_name);

    // Try using 'Global' so that it can cross terminal server sessions
    //
    hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, global_seg_name);
    if (!hMap) {
        // Couldn't use the 'Global' tag, so just attempt to use the original
        // name.
        //
        hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, seg_name);
    }
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

    snprintf(buf, sizeof(buf), "\\SHAREMEM\\BOINC\\%d", key);
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

    snprintf(buf, sizeof(buf), "\\SHAREMEM\\BOINC\\%d", key);
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

// V6 mmap() shared memory for Unix/Linux/Mac
//
int create_shmem_mmap(const char *path, size_t size, void** pp) {
    int fd, retval;
    struct stat sbuf;

    // Return NULL pointer if create_shmem fails
    *pp = 0;
    if (size == 0) return ERR_SHMGET;

    // NOTE: in principle it should be 0660, not 0666
    // (i.e. Apache should belong to the same group as the
    // project admin user, and should therefore be able to access the seg.
    // However, this doesn't seem to work on some Linux systems.
    // I don't have time to figure this out (31 July 07)
    // it's a big headache for anyone it affects,
    // and it's not a significant security issue.
    //
    fd = open(path, O_RDWR | O_CREAT, 0666);
    if (fd < 0) return ERR_SHMGET;

    retval = fstat(fd, &sbuf);
    if (retval) {
        close(fd);
        return ERR_SHMGET;
    }
    if (sbuf.st_size < (long)size) {
        // The following 2 lines extend the file and clear its new
        // area to all zeros because they write beyond the old EOF.
        // See the lseek man page for details.
        lseek(fd, size-1, SEEK_SET);
        if (1 != write(fd, "\0", 1)) {
	    close(fd);
	    return ERR_SHMGET;
	}
    }

    *pp = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);

    close(fd);

    if (*pp == MAP_FAILED) {
        *pp = 0;
        return ERR_SHMGET;
    }

    return 0;
}

int destroy_shmem_mmap(key_t /*key*/){
    return 0;
}


int attach_shmem_mmap(const char *path, void** pp) {
    int fd, retval;
    struct stat sbuf;

    // Return NULL pointer if attach_shmem fails
    *pp = 0;
    fd = open(path, O_RDWR);
    if (fd < 0) return ERR_SHMGET;

    retval = fstat(fd, &sbuf);
    if (retval) {
        close(fd);
        return ERR_SHMGET;
    }
    if (sbuf.st_size == 0) {
        close(fd);
        return ERR_SHMGET;
    }

    *pp = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);

    close(fd);

    if (*pp == MAP_FAILED) {
        *pp = 0;
        return ERR_SHMGET;
    }

    return 0;
}


int detach_shmem_mmap(void* p, size_t size) {
    return munmap((char *)p, size);
}

#if HAVE_SYS_SHM_H && !defined(ANDROID)

// Compatibility routines for Unix/Linux/Mac V5 applications
//
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
        boinc::perror("shmget");
        boinc::fprintf(stderr, "shmem size: %d\n", size);
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
            boinc::perror("shmget: shmctl STAT");
            return ERR_SHMGET;
        }
        buf.shm_perm.gid = gid;
        retval = shmctl(id, IPC_SET, &buf);
        if (retval) {
            boinc::perror("shmget: shmctl IPC_SET");
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
// only after we are completely done with the segment.
//
int destroy_shmem(key_t key){
    struct shmid_ds buf;
    int id, retval;

    id = shmget(key, 0, 0);
    if (id < 0) return 0;           // assume it doesn't exist
    retval = shmctl(id, IPC_STAT, &buf);
    if (retval) {
        boinc::perror("shmctl STAT");
        return ERR_SHMCTL;
    }
    retval = shmctl(id, IPC_RMID, 0);
    if (retval) {
        boinc::perror("shmctl RMID");
        return ERR_SHMCTL;
    }
    return 0;
}

int attach_shmem(key_t key, void** pp){
    void* p;
    int id;

    id = shmget(key, 0, 0);
    if (id < 0) {
        boinc::perror("shmget in attach_shmem");
        return ERR_SHMGET;
    }
    p = shmat(id, 0, 0);
    if ((long)p == -1) {
        boinc::perror("shmat");
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
    boinc::fprintf(
        stderr, "shmem key: %x\t\tid: %d, size: %d, nattach: %d\n",
        (unsigned int)key, id, (int)buf.shm_segsz, (int)buf.shm_nattch
    );

    return 0;
}

#else  // HAVE_SYS_SHM_H

// Platforms that don't have sys/shm.h will need stubs,
// or alternate implementations

int create_shmem(key_t, int size, gid_t gid, void**) {
   boinc::perror("create_shmem: not supported on this platform");
   return ERR_SHMGET;
}
int attach_shmem(key_t, void**) {
   boinc::perror("attach_shmem: not supported on this platform");
   return ERR_SHMGET;
}
int detach_shmem(void*) {
   boinc::perror("detach_shmem: not supported on this platform");
   return ERR_SHMGET;
}
int destroy_shmem(key_t) {
   boinc::perror("destroy_shmem: not supported on this platform");
   return ERR_SHMCTL;
}

#endif  // !HAVE_SYS_SHM_H

#endif  // !defined(_WIN32) && !defined(__EMX__)


#ifndef _WIN32
#ifndef HAVE_FTOK

key_t ftok(const char *path, int id) {
	struct stat st;
	if (stat(path, &st) < 0) return -1;
	return ((st.st_ino & 0xffff) | ((st.st_dev & 0xff) << 16) | ((id & 0xff) << 24));
}

#endif
#endif

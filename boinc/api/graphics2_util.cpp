// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// graphics-related utilities, used both from main applications
// (set create shared mem) and by the graphics app

#ifdef _WIN32
#include "boinc_win.h"
#elif (!defined(__EMX__))
#include <sys/stat.h>
#else
#include "config.h"
#endif

#include <cstring>
#include "shmem.h"
#include "filesys.h"
#include "app_ipc.h"
#include "boinc_api.h"
#include "graphics2.h"

#ifdef __EMX__
static key_t get_shmem_name(const char* prog_name) {
    char cwd[256], path[256];
    boinc_getcwd(cwd);
    sprintf(path, "%s/init_data.xml", cwd);
    return ftok(path, 2);
}
#else
// Unix/Linux/Mac applications always use mmap() for gfx communication
//
static void get_shmem_name(const char* prog_name, char* shmem_name) {
    APP_INIT_DATA aid;
    int retval = boinc_get_init_data(aid);
    if (retval) aid.slot = 0;
    sprintf(shmem_name, "boinc_%s_%d", prog_name, aid.slot);
}
#endif

void* boinc_graphics_make_shmem(const char* prog_name, int size) {
#ifdef _WIN32
    HANDLE shmem_handle;
    char shmem_name[256];
    void* p;
    get_shmem_name(prog_name, shmem_name);
    shmem_handle = create_shmem(shmem_name, size, &p);
    if (shmem_handle == NULL) return 0;
    return p;
#else
    void* p;
#ifdef __EMX__
    key_t key = get_shmem_name(prog_name);
    int retval = create_shmem(key, size, 0, &p);
#else
    // V6 Unix/Linux/Mac applications always use mmap() shared memory for graphics communication
    char shmem_name[256];
    get_shmem_name(prog_name, shmem_name);
    int retval = create_shmem_mmap(shmem_name, size, &p);
    // Graphics app may be run by a different user & group than worker app
    // Although create_shmem passed 0666 to open(), it was modified by umask
    if (retval == 0) chmod(shmem_name, 0666);
#endif
    if (retval) return 0;
    return p;
#endif
}

#ifdef _WIN32
void* boinc_graphics_get_shmem(const char* prog_name) {
    HANDLE shmem_handle;
    char shmem_name[256];
    void* p;
    get_shmem_name(prog_name, shmem_name);
    shmem_handle = attach_shmem(shmem_name, &p);
    if (shmem_handle == NULL) {
        return 0;
    }
    return p;
}
#else
void* boinc_graphics_get_shmem(const char* prog_name) {
    void* p;
    int retval;
#ifdef __EMX__
    key_t key = get_shmem_name(prog_name);
    retval = attach_shmem(key, &p);
#else
    // V6 Unix/Linux/Mac applications always use mmap() shared memory for graphics communication
    char shmem_name[256];
    get_shmem_name(prog_name, shmem_name);
    retval = attach_shmem_mmap(shmem_name, &p);
#endif
    if (retval) return 0;
    return p;
}
#endif

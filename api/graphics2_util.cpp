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
#include "app_ipc.h"
#include "boinc_api.h"
#include "filesys.h"
#include "util.h"

#include "graphics2.h"

#ifdef __EMX__
static key_t get_shmem_name(const char* prog_name) {
    char cwd[MAXPATHLEN+1], path[MAXPATHLEN+1];
    boinc_getcwd(cwd);
    snprintf(path, sizeof(path), "%s/init_data.xml", cwd);
    return ftok(path, 2);
}
#else
// Unix/Linux/Mac applications always use mmap() for gfx communication
//
static void get_shmem_name(const char* prog_name, char* shmem_name) {
    APP_INIT_DATA aid;
    int retval = boinc_get_init_data(aid);
    if (retval) aid.slot = 0;
    snprintf(shmem_name, MAXPATHLEN+1, "boinc_%s_%d", prog_name, aid.slot);
}
#endif

void* boinc_graphics_make_shmem(const char* prog_name, int size) {
#ifdef _WIN32
    HANDLE shmem_handle;
    char shmem_name[MAXPATHLEN+1];
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
    char shmem_name[MAXPATHLEN+1];
    get_shmem_name(prog_name, shmem_name);
    int retval = create_shmem_mmap(shmem_name, size, &p);
    // make sure user/group RW permissions are set, but not other.
    //
    if (retval == 0) chmod(shmem_name, 0660);
#endif
    if (retval) return 0;
    return p;
#endif
}

#ifdef _WIN32
void* boinc_graphics_get_shmem(const char* prog_name) {
    HANDLE shmem_handle;
    char shmem_name[MAXPATHLEN+1];
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
    char shmem_name[MAXPATHLEN+1];
    get_shmem_name(prog_name, shmem_name);
    retval = attach_shmem_mmap(shmem_name, &p);
#endif
    if (retval) return 0;
    return p;
}
#endif

#define GRAPHICS_STATUS_FILENAME "graphics_status.xml"

int boinc_write_graphics_status(
    double cpu_time, double elapsed_time,
    double fraction_done
){
    MIOFILE mf;
    FILE* f = boinc_fopen(GRAPHICS_STATUS_FILENAME, "w");
    mf.init_file(f);
    mf.printf(
        "<graphics_status>\n"
        "    <updated_time>%f</updated_time>\n"
        "    <cpu_time>%f</cpu_time>\n"
        "    <elapsed_time>%f</elapsed_time>\n"
        "    <fraction_done>%f</fraction_done>\n"
        "    <boinc_status>\n"
        "        <no_heartbeat>%d</no_heartbeat>\n"
        "        <suspended>%d</suspended>\n"
        "        <quit_request>%d</quit_request>\n"
        "        <abort_request>%d</abort_request>\n"
        "        <network_suspended>%d</network_suspended>\n"
        "    </boinc_status>\n"
        "</graphics_status>\n",
        dtime(),
        cpu_time,
        elapsed_time,
        fraction_done,
        boinc_status.no_heartbeat,
        boinc_status.suspended,
        boinc_status.quit_request,
        boinc_status.abort_request,
        boinc_status.network_suspended
    );
    fclose(f);
    return 0;
}

int boinc_parse_graphics_status(
    double* updated_time, double* cpu_time,
    double* elapsed_time, double* fraction_done, BOINC_STATUS* status
){
    MIOFILE mf;
    FILE* f = boinc_fopen(GRAPHICS_STATUS_FILENAME, "r");
    if (!f) {
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    *updated_time = 0;
    *cpu_time = 0;
    *elapsed_time = 0;
    *fraction_done = 0;
    memset(status, 0, sizeof(BOINC_STATUS));

    if (!xp.parse_start("graphics_status")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/graphics_status")) {
            fclose(f);
            return 0;
        }
        if (xp.match_tag("boinc_status")) {
            while (!xp.get_tag()) {
                if (!xp.is_tag) {
                    continue;
                }
                if (xp.match_tag("/boinc_status")) {
                    break;
                }
                else if (xp.parse_int("no_heartbeat", status->no_heartbeat)) continue;
                else if (xp.parse_int("suspended", status->suspended)) continue;
                else if (xp.parse_int("quit_request", status->quit_request)) continue;
                else if (xp.parse_int("abort_request", status->abort_request)) continue;
                else if (xp.parse_int("network_suspended", status->network_suspended)) continue;
            }
        }
        else if (xp.parse_double("updated_time", *updated_time)) continue;
        else if (xp.parse_double("cpu_time", *cpu_time)) continue;
        else if (xp.parse_double("elapsed_time", *elapsed_time)) continue;
        else if (xp.parse_double("fraction_done", *fraction_done)) continue;
    }
    fclose(f);
    return ERR_XML_PARSE;
}

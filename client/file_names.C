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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#endif

#ifndef _WIN32
#include <cstdio>
#include <sys/stat.h>
#include <cctype>
#endif

#include "filesys.h"
#include "error_numbers.h"
#include "client_msgs.h"
#include "util.h"

#include "file_names.h"

void escape_project_url(char *in, char* out) {
    escape_url_readable(in, out);
    char& last = out[strlen(out)-1];
    // remove trailing _
    if (last == '_') {
        last = '\0';
    }
}

void get_project_dir(PROJECT* p, char* path) {
    char buf[256];
    escape_project_url(p->master_url, buf);
    sprintf(path, "%s%s%s", PROJECTS_DIR, PATH_SEPARATOR, buf);
}

// Gets the pathname of a file
//
void get_pathname(FILE_INFO* fip, char* path) {
    PROJECT* p = fip->project;
    char buf[256];

    // for testing purposes, it's handy to allow a FILE_INFO without
    // an associated PROJECT.
    //
    if (p) {
        get_project_dir(p, buf);
        sprintf(path, "%s%s%s", buf, PATH_SEPARATOR, fip->name);
    } else {
        strcpy(path, fip->name);
    }
}

void get_sched_request_filename(PROJECT& project, char* buf) {
    char url[256];

    escape_project_url(project.master_url, url);
    sprintf(buf, "%s%s.xml", SCHED_OP_REQUEST_BASE, url);
}

void get_sched_reply_filename(PROJECT& project, char* buf) {
    char url[256];

    escape_project_url(project.master_url, url);
    sprintf(buf, "%s%s.xml", SCHED_OP_REPLY_BASE, url);
}

// Returns the location of a numbered slot directory
//
void get_slot_dir(int slot, char* path) {
    sprintf(path, "%s%s%d", SLOTS_DIR, PATH_SEPARATOR, slot);
}

// Create the directory for the project p
//
int make_project_dir(PROJECT& p) {
    char buf[256];
    int retval;

    boinc_mkdir(PROJECTS_DIR);
    get_project_dir(&p, buf);
    retval = boinc_mkdir(buf);
    return retval;
}

int remove_project_dir(PROJECT& p) {
    char buf[256];
    int retval;

    get_project_dir(&p, buf);
    retval = clean_out_dir(buf);
    if (retval) {
        msg_printf(&p, MSG_ERROR, "Can't delete file %s\n", boinc_failed_file);
        return retval;
    }
    return boinc_rmdir(buf);
}

// Create the slot directory for the specified slot #
//
int make_slot_dir(int slot) {
    char buf[256];
    if (slot<0) {
        msg_printf(NULL, MSG_ERROR, "make_slot_dir(): negative slot\n");
        return ERR_NEG;
    }
    boinc_mkdir(SLOTS_DIR);
    get_slot_dir(slot, buf);
    return boinc_mkdir(buf);
}

void get_account_filename(char* master_url, char* path) {
    char buf[256];
    escape_project_url(master_url, buf);
    sprintf(path, "account_%s.xml", buf);
}

bool is_account_file(char* filename) {
    return (strstr(filename, "account_") == filename);
}

int check_unique_instance() {
#ifdef _WIN32
    // on Windows, we also set a mutex so that the screensaver
    // can find out that the core client is running
    //
    HANDLE h = CreateMutex(NULL, true, RUN_MUTEX);
    if ((h==0) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
        return ERR_ALREADY_RUNNING;
    }
#else
    if (lock_file(LOCK_FILE_NAME)) {
        return ERR_ALREADY_RUNNING;
    }
#endif
    return 0;
}

const char *BOINC_RCSID_7d362a6a52 = "$Id$";

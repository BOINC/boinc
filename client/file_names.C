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
#if 1
    if (lock_file(LOCK_FILE_NAME)) {
        return ERR_ALREADY_RUNNING;
    }
#ifdef _WIN32
    else
    {
        HANDLE h = CreateMutex(NULL, true, RUN_MUTEX);
        if ((h==0) || (GetLastError() == ERROR_ALREADY_EXISTS)) {
            UINT nShowMsg = RegisterWindowMessage(SHOW_WIN_MSG);
            PostMessage(HWND_BROADCAST, nShowMsg, 0, 0);
            return FALSE;
        }
    }
#endif
#else
    key_t key;
    char path[256];
    getcwd(path, 256);
    retval = get_key(path, 'a', key);
    if (!retval) retval = create_semaphore(key);
    if (!retval) retval = lock_semaphore(key);
    if (retval) {
        return ERR_ALREADY_RUNNING;
    }
#endif
    return 0;
}

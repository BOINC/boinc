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
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <sys/stat.h>
#include <cctype>
#endif

#include "filesys.h"
#include "error_numbers.h"
#include "client_msgs.h"
#include "client_state.h"
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
    sprintf(path, "%s/%s", PROJECTS_DIR, buf);
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
        sprintf(path, "%s/%s", buf, fip->name);
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

void get_master_filename(PROJECT& project, char* buf) {
    char url[256];

    escape_project_url(project.master_url, url);
    sprintf(buf, "%s%s.xml", MASTER_BASE, url);
}

// Returns the location of a numbered slot directory
//
void get_slot_dir(int slot, char* path) {
    sprintf(path, "%s/%d", SLOTS_DIR, slot);
}

// Create the directory for the project p
//
int make_project_dir(PROJECT& p) {
    char buf[256];
    int retval;

    boinc_mkdir(PROJECTS_DIR);
#ifdef SANDBOX
    chmod(PROJECTS_DIR,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
#endif
    get_project_dir(&p, buf);
    retval = boinc_mkdir(buf);
#ifdef SANDBOX
    boinc_chown(buf, gstate.boinc_project_gid);
#endif
    return retval;
}

int remove_project_dir(PROJECT& p) {
    char buf[256];
    int retval;

    get_project_dir(&p, buf);
    retval = clean_out_dir(buf);
    if (retval) {
        msg_printf(&p, MSG_ERROR, "Can't delete file %s", boinc_failed_file);
        return retval;
    }
    return boinc_rmdir(buf);
}

// Create the slot directory for the specified slot #
//
int make_slot_dir(int slot) {
    char buf[256];
    if (slot<0) {
        msg_printf(NULL, MSG_ERROR, "Bad slot number %d", slot);
        return ERR_NEG;
    }
    boinc_mkdir(SLOTS_DIR);
#ifdef SANDBOX
    chmod(SLOTS_DIR,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP||S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
#endif
    get_slot_dir(slot, buf);
    int retval = boinc_mkdir(buf);
#ifdef SANDBOX
    boinc_chown(buf, gstate.boinc_project_gid);
#endif
    return retval;
}

// A slot dir can't be cleaned out
// (e.g. a virus-checker has a file locked).
// Rename it to DELETE_ME_x
//
int rename_slot_dir(int slot) {
    char oldname[256], newname[256];
    sprintf(oldname, "%s/%d", SLOTS_DIR, slot);
    sprintf(newname, "%s/DELETE_ME_%d_%d", SLOTS_DIR, slot, (int)gstate.now);
    int retval = rename(oldname, newname);
    if (retval) return ERR_RENAME;
    return 0;
}

// delete directories created by the above
//
void delete_old_slot_dirs() {
    char filename[256], path[256];
    DIRREF dirp;
    int retval;

    dirp = dir_open(SLOTS_DIR);
    if (!dirp) return;
    while (1) {
        strcpy(filename, "");
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;
        if (strstr(filename, "DELETE_ME")) {
            sprintf(path, "%s/%s", SLOTS_DIR, filename);
            clean_out_dir(path);
            boinc_rmdir(path);
        }
    }
}

void get_account_filename(char* master_url, char* path) {
    char buf[256];
    escape_project_url(master_url, buf);
    sprintf(path, "account_%s.xml", buf);
}

static bool bad_account_filename(const char* filename) {
    msg_printf(NULL, MSG_ERROR, "Invalid account filename: %s", filename);
    return false;
}

// account filenames are of the form
// account_URL.xml
// where URL is master URL with slashes replaced by underscores
//
bool is_account_file(const char* filename) {
    const char* p, *q;
    p = strstr(filename, "account_");
    if (p != filename) return false;

    q = filename + strlen("account_");
    p = strstr(q, ".xml");
    if (!p) return bad_account_filename(filename);
    if (p == q) return bad_account_filename(filename);

    q = p + strlen(".xml");
    if (strlen(q)) return bad_account_filename(filename);
    return true;
}

// statistics filenames are of the form
// statistics_URL.xml
// where URL is master URL with slashes replaced by underscores
//
bool is_statistics_file(const char* filename) {
    const char* p, *q;
    p = strstr(filename, "statistics_");
    if (p != filename) return false;
    q = filename + strlen("statistics_");

    p = strstr(q, ".");
    if (!p) return bad_account_filename(filename);
    if (p == q) return bad_account_filename(filename);

    q = p+1;
    p = strstr(q, ".xml");
    if (!p) return bad_account_filename(filename);
    if (p == q) return bad_account_filename(filename);

    q = p + strlen(".xml");
    if (strlen(q)) return bad_account_filename(filename);
    return true;
}

void get_statistics_filename(char* master_url, char* path) {
    char buf[256];
    escape_project_url(master_url, buf);
    sprintf(path, "statistics_%s.xml", buf);
}

bool is_image_file(const char* filename) {
    std::string fn = filename;
    downcase_string(fn);
    if (ends_with(fn, std::string(".jpg"))) return true;
    if (ends_with(fn, std::string(".jpeg"))) return true;
    if (ends_with(fn, std::string(".png"))) return true;
    return false;
}

const char *BOINC_RCSID_7d362a6a52 = "$Id$";

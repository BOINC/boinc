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
#include "str_util.h"
#include "util.h"
#include "client_msgs.h"
#include "client_state.h"

#include "file_names.h"

void get_project_dir(PROJECT* p, char* path, int len) {
    char buf[1024];
    escape_project_url(p->master_url, buf);
    snprintf(path, len, "%s/%s", PROJECTS_DIR, buf);
}

// Gets the pathname of a file
//
void get_pathname(FILE_INFO* fip, char* path, int len) {
    PROJECT* p = fip->project;
    char buf[1024];

    // for testing purposes, it's handy to allow a FILE_INFO without
    // an associated PROJECT.
    //
    if (p) {
		if (fip->is_auto_update_file) {
			boinc_version_dir(*p, gstate.auto_update.version, buf);
		} else {
            get_project_dir(p, buf, sizeof(buf));
		}
	    snprintf(path, len, "%s/%s", buf, fip->name);
	} else {
        strlcpy(path, fip->name, len);
    }
}

void get_sched_request_filename(PROJECT& project, char* buf, int len) {
    char url[1024];

    escape_project_url(project.master_url, url);
    snprintf(buf, len, "%s%s.xml", SCHED_OP_REQUEST_BASE, url);
}

void get_sched_reply_filename(PROJECT& project, char* buf, int len) {
    char url[1024];

    escape_project_url(project.master_url, url);
    snprintf(buf, len, "%s%s.xml", SCHED_OP_REPLY_BASE, url);
}

void get_master_filename(PROJECT& project, char* buf, int len) {
    char url[1024];

    escape_project_url(project.master_url, url);
    snprintf(buf, len, "%s%s.xml", MASTER_BASE, url);
}

void job_log_filename(PROJECT& project, char* buf, int len) {
    char url[1024];

    escape_project_url(project.master_url, url);
    snprintf(buf, len, "%s%s.txt", JOB_LOG_BASE, url);
}

// Returns the location of a numbered slot directory
//
void get_slot_dir(int slot, char* path, int len) {
    snprintf(path, len, "%s/%d", SLOTS_DIR, slot);
}

// Create the directory for the project p
//
int make_project_dir(PROJECT& p) {
    char buf[1024];
    int retval;

    boinc_mkdir(PROJECTS_DIR);
#ifndef _WIN32
    if (g_use_sandbox) {
        chmod(PROJECTS_DIR,
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IWGRP|S_IXGRP
                |S_IROTH|S_IXOTH
            );
    }
#endif
    get_project_dir(&p, buf, sizeof(buf));
    retval = boinc_mkdir(buf);
#ifndef _WIN32
    if (g_use_sandbox) {
        chmod(buf,
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IWGRP|S_IXGRP
                |S_IROTH|S_IXOTH
            );
        set_to_project_group(buf);
    }
#endif
    return retval;
}

int remove_project_dir(PROJECT& p) {
    char buf[1024];
    int retval;

    get_project_dir(&p, buf, sizeof(buf));
    retval = clean_out_dir(buf);
    if (retval) {
        msg_printf(&p, MSG_INTERNAL_ERROR, "Can't delete file %s", boinc_failed_file);
        return retval;
    }
    return boinc_rmdir(buf);
}

// Create the slot directory for the specified slot #
//
int make_slot_dir(int slot) {
    char buf[1024];
    if (slot<0) {
        msg_printf(NULL, MSG_INTERNAL_ERROR, "Bad slot number %d", slot);
        return ERR_NEG;
    }
    boinc_mkdir(SLOTS_DIR);
#ifndef _WIN32
    if (g_use_sandbox) {
        chmod(SLOTS_DIR,
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IWGRP|S_IXGRP
                |S_IROTH|S_IXOTH
            );
    }
#endif
    get_slot_dir(slot, buf, sizeof(buf));
    int retval = boinc_mkdir(buf);
#ifndef _WIN32
    if (g_use_sandbox) {
        chmod(buf,
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IWGRP|S_IXGRP
                |S_IROTH|S_IXOTH
            );
        set_to_project_group(buf);
    }
#endif
    return retval;
}

// delete unused stuff in the slots/ directory
//
void delete_old_slot_dirs() {
    char filename[1024], path[1024];
    DIRREF dirp;
    int retval;

    dirp = dir_open(SLOTS_DIR);
    if (!dirp) return;
    while (1) {
        strcpy(filename, "");
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;
        sprintf(path, "%s/%s", SLOTS_DIR, filename);
        if (is_dir(path)) {
            if (!gstate.active_tasks.is_slot_dir_in_use(path)) {
                clean_out_dir(path);
                boinc_rmdir(path);
            }
        } else {
            boinc_delete_file(path);
        }
    }
    dir_close(dirp);
}

void get_account_filename(char* master_url, char* path) {
    char buf[1024];
    escape_project_url(master_url, buf);
    sprintf(path, "account_%s.xml", buf);
}

static bool bad_account_filename(const char* filename) {
    msg_printf(NULL, MSG_INTERNAL_ERROR, "Invalid account filename: %s", filename);
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

int set_to_project_group(const char* path) {
    char buf[1024];
    
    if (g_use_sandbox) {
        sprintf(buf, "%s/%s %s", SWITCHER_DIR, SETPROJECTGRP_FILE_NAME, path);
        if (system(buf))
            return ERR_CHOWN;
    }
        return 0;
}

void boinc_version_dir(PROJECT& p, VERSION_INFO& vi, char* buf) {
	char projdir[1024];
	get_project_dir(&p, projdir, sizeof(projdir));
    sprintf(buf, "%s/boinc_version_%d_%d_%d", projdir, vi.major, vi.minor, vi.release);
}

bool is_version_dir(char* buf, VERSION_INFO& vi) {
    int n = sscanf(buf, "boinc_version_%d_%d_%d", &vi.major, &vi.minor, &vi.release);
    return (n==3);
}

const char *BOINC_RCSID_7d362a6a52 = "$Id$";

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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
#else
#include "config.h"
#include <cstdio>
#include <sys/stat.h>
#include <cctype>
#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#include "shmem.h"
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"
#include "url.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "project.h"
#include "sandbox.h"

#include "file_names.h"

int make_soft_link(PROJECT* project, char* link_path, char* rel_file_path) {
    FILE *fp = boinc_fopen(link_path, "w");
    if (!fp) {
        msg_printf(project, MSG_INTERNAL_ERROR,
            "Can't create link file %s", link_path
        );
        return ERR_FOPEN;
    }
    fprintf(fp, "<soft_link>%s</soft_link>\n", rel_file_path);
    fclose(fp);
    if (log_flags.slot_debug) {
        msg_printf(project, MSG_INFO,
            "[slot] linked %s to %s", rel_file_path, link_path
        );
    }
    return 0;
}

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
#ifdef ENABLE_AUTO_UPDATE
        if (fip->is_auto_update_file) {
            boinc_version_dir(*p, gstate.auto_update.version, buf);
        } else {
            get_project_dir(p, buf, sizeof(buf));
        }
#else
        get_project_dir(p, buf, sizeof(buf));
#endif
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
    mode_t old_mask;
    if (g_use_sandbox) {
        old_mask = umask(2);        // Allow writing by group
         chmod(PROJECTS_DIR,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
        );
        umask(old_mask);
        // Only user boinc_master and group boinc_project can access
        // project directories, to keep authenticators private
        set_to_project_group(PROJECTS_DIR);
    }
#endif
    get_project_dir(&p, buf, sizeof(buf));
    retval = boinc_mkdir(buf);
#ifndef _WIN32
    if (g_use_sandbox) {
        old_mask = umask(2);
        // Contents of projects directory must be world-readable so BOINC Client can read
        // files written by projects which have user boinc_project and group boinc_project
        chmod(buf,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
        umask(old_mask);
        set_to_project_group(buf);
    }
#endif
    return retval;
}

int remove_project_dir(PROJECT& p) {
    char buf[1024];
    int retval;

    get_project_dir(&p, buf, sizeof(buf));
    retval = client_clean_out_dir(buf, "remove project dir");
    if (retval) {
        msg_printf(&p, MSG_INTERNAL_ERROR, "Can't delete file %s", boinc_failed_file);
        return retval;
    }
    return remove_project_owned_dir(buf);
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
    mode_t old_mask;
    if (g_use_sandbox) {
        old_mask = umask(2);        // Allow writing by group
        chmod(SLOTS_DIR,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
        );
        umask(old_mask);
        // Only user boinc_master and group boinc_project can
        // access slot directories, to keep authenticators private
        set_to_project_group(SLOTS_DIR);
    }

#endif
    get_slot_dir(slot, buf, sizeof(buf));
    int retval = boinc_mkdir(buf);
#ifndef _WIN32
    if (g_use_sandbox) {
        old_mask = umask(2);
        // Contents of slots directory must be world-readable so BOINC Client can read
        // files written by projects which have user boinc_project and group boinc_project
        chmod(buf,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
        umask(old_mask);
        set_to_project_group(buf);
    }
#endif
    return retval;
}

// delete unused stuff in the slots/ directory
//
void delete_old_slot_dirs() {
    char filename[1024], path[MAXPATHLEN];
    DIRREF dirp;
    int retval;

    dirp = dir_open(SLOTS_DIR);
    if (!dirp) return;
    while (1) {
        strcpy(filename, "");
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;
        snprintf(path, sizeof(path), "%s/%s", SLOTS_DIR, filename);
        if (is_dir(path)) {
#ifndef _WIN32
            char init_data_path[MAXPATHLEN];
            SHMEM_SEG_NAME shmem_seg_name;

            // If BOINC crashes or exits suddenly (e.g., due to
            // being called with --exit_after_finish) it may leave
            // orphan shared memory segments in the system.
            // Clean these up here. (We must do this before deleting the
            // INIT_DATA_FILE, if any, from each slot directory.)
            //
            snprintf(init_data_path, sizeof(init_data_path), "%s/%s", path, INIT_DATA_FILE);
            shmem_seg_name = ftok(init_data_path, 1);
            if (shmem_seg_name != -1) {
                destroy_shmem(shmem_seg_name);
            }
#endif
            if (!gstate.active_tasks.is_slot_dir_in_use(path)) {
                client_clean_out_dir(path, "delete old slot dirs");
                remove_project_owned_dir(path);
            }
        } else {
            delete_project_owned_file(path, false);
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

void boinc_version_dir(PROJECT& p, VERSION_INFO& vi, char* buf) {
    char projdir[1024];
    get_project_dir(&p, projdir, sizeof(projdir));
    sprintf(buf, "%s/boinc_version_%d_%d_%d", projdir, vi.major, vi.minor, vi.release);
}

bool is_version_dir(char* buf, VERSION_INFO& vi) {
    int n = sscanf(buf, "boinc_version_%d_%d_%d", &vi.major, &vi.minor, &vi.release);
    return (n==3);
}


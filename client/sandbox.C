// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <errno.h>
#include "client_state.h"
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "util.h"
#include "str_util.h"
#include "filesys.h"

#include "sandbox.h"

bool g_use_sandbox = false;

#ifndef _WIN32
#ifndef _DEBUG
static int lookup_group(char* name, gid_t& gid) {
    struct group* gp = getgrnam(name);
    if (!gp) return ERR_GETGRNAM;
    gid = gp->gr_gid;
    return 0;
}
#endif

void kill_via_switcher(int pid) {
    char cmd[1024];
    
    if (!g_use_sandbox) return;

    // if project application is running as user boinc_project and 
    // core client is running as user boinc_master, we cannot send
    // a signal directly, so use switcher.
    sprintf(cmd, "/bin/kill kill -s KILL %d", pid);
    switcher_exec(SWITCHER_FILE_NAME, cmd);
}

int get_project_gid() {
    if (g_use_sandbox) {
#ifdef _DEBUG
        gstate.boinc_project_gid = getegid();
#else
        return lookup_group(BOINC_PROJECT_GROUP_NAME, gstate.boinc_project_gid);
#endif  // _DEBUG
    } else {
        gstate.boinc_project_gid = 0;
    }
    return 0;
}

int set_to_project_group(const char* path) {
    if (g_use_sandbox) {
        if (switcher_exec(SETPROJECTGRP_FILE_NAME, (char*)path)) {
            return ERR_CHOWN;
        }
    }
    return 0;
}

// POSIX requires that shells run from an application will use the 
// real UID and GID if different from the effective UID and GID.  
// Mac OS 10.4 did not enforce this, but OS 10.5 does.  Since 
// system() invokes a shell, we can't use it to run the switcher 
// or setprojectgrp utilities, so we must do a fork() and execv().
//
int switcher_exec(char *util_filename, char* cmdline) {
    char* argv[100];
    char util_path[1024];

    sprintf(util_path, "%s/%s", SWITCHER_DIR, util_filename);
    argv[0] = util_filename;
    parse_command_line(cmdline, argv+1);
    int pid = fork();
    if (pid == -1) {
        perror("fork() failed in switcher_exec");
        return ERR_FORK;
    }
    if (pid == 0) {
        // This is the new (forked) process
        execv(util_path, argv);
        perror("execv failed in switcher_exec");
        return ERR_EXEC;
    }
    // Wait for command to complete, like system() does.
    waitpid(pid, 0, 0); 
    return BOINC_SUCCESS;
}

int remove_project_owned_file_or_dir(const char* path) {
    char cmd[1024];

    if (g_use_sandbox) {
        sprintf(cmd, "/bin/rm rm -fR \"%s\"", path);
        if (switcher_exec(SWITCHER_FILE_NAME, cmd)) {
            return ERR_UNLINK;
        } else {
            return 0;
        }
    }
    return ERR_UNLINK;
}

#endif // ! _WIN32

static int delete_project_owned_file_aux(const char* path) {
#ifdef _WIN32
    if (DeleteFile(path)) return 0;
    return GetLastError();
#else
    int retval = unlink(path);
    if (retval && g_use_sandbox && (errno == EACCES)) {
        // We may not have permission to read subdirectories created by projects
        return remove_project_owned_file_or_dir(path);
    }
    return retval;
#endif
}

// Delete the file located at path
//
int delete_project_owned_file(const char* path) {
    int retval = 0;

    if (!boinc_file_exists(path)) {
        return 0;
    }
    retval = delete_project_owned_file_aux(path);
    if (retval) {
        double start = dtime();
        do {
            boinc_sleep(drand()*2);       // avoid lockstep
            retval = delete_project_owned_file_aux(path);
            if (!retval) break;
        } while (dtime() < start + FILE_RETRY_INTERVAL);
    }
    if (retval) {
        safe_strcpy(boinc_failed_file, path);
        return ERR_UNLINK;
    }
    return 0;
}

// recursively delete everything in the specified directory
// (but not the directory itself).
// If an error occurs, delete as much as you can.
//
int clean_out_dir(const char* dirpath) {
    char filename[256], path[256];
    int retval, final_retval = 0;
    DIRREF dirp;

    dirp = dir_open(dirpath);
    if (!dirp) {
#ifndef _WIN32
        if (g_use_sandbox && (errno == EACCES)) {
            // dir may be owned by boinc_apps
            return remove_project_owned_file_or_dir(dirpath);
        }
#endif
        return 0;    // if dir doesn't exist, it's empty
    }

    while (1) {
        strcpy(filename, "");
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) break;
        sprintf(path, "%s/%s", dirpath,  filename);
        if (is_dir(path)) {
            retval = clean_out_dir(path);
            if (retval) final_retval = retval;
            retval = remove_project_owned_dir(path);
            if (retval) final_retval = retval;
        } else {
            retval = delete_project_owned_file(path);
            if (retval) final_retval = retval;
        }
    }
    dir_close(dirp);
    return final_retval;
}

int remove_project_owned_dir(const char* name) {
#ifdef _WIN32
    if (!RemoveDirectory(name)) {
        return GetLastError();
    }
    return 0;
#else
    int retval;
    retval = rmdir(name);
    // We may not have permission to read subdirectories created by projects
    if (retval && g_use_sandbox && (errno == EACCES)) {
        retval = remove_project_owned_file_or_dir(name);
    }
    return retval;
#endif
}

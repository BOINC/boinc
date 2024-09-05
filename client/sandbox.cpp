// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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
#else
#include "config.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "util.h"
#include "str_util.h"
#include "str_replace.h"
#include "filesys.h"
#include "parse.h"
#include "client_msgs.h"
#include "client_state.h"

#include "sandbox.h"

bool g_use_sandbox = false;

#ifndef _WIN32

// POSIX requires that shells run from an application will use the
// real UID and GID if different from the effective UID and GID.
// Mac OS 10.4 did not enforce this, but OS 10.5 does.  Since
// system() invokes a shell, we can't use it to run the switcher
// or setprojectgrp utilities, so we must do a fork() and execv().
//
int switcher_exec(const char *util_filename, const char* cmdline) {
    char* argv[100];
    char util_path[MAXPATHLEN];
    char command [1024];
    char buffer[1024];
    int fds_out[2], fds_err[2];
    int stat;
    int retval;
    string output_out, output_err;

    snprintf(util_path, sizeof(util_path), "%s/%s", SWITCHER_DIR, util_filename);
    argv[0] = const_cast<char*>(util_filename);
    // Make a copy of cmdline because parse_command_line modifies it
    safe_strcpy(command, cmdline);
    parse_command_line(const_cast<char*>(cmdline), argv+1);

    // Create the output pipes
    if (pipe(fds_out) == -1) {
        perror("pipe() for fds_out failed in switcher_exec");
        return ERR_PIPE;
    }

    if (pipe(fds_err) == -1) {
        perror("pipe() for fds_err failed in switcher_exec");
        return ERR_PIPE;
    }

    int pid = fork();
    if (pid == -1) {
        perror("fork() failed in switcher_exec");
        return ERR_FORK;
    }
    if (pid == 0) {
        // This is the new (forked) process

        // Setup pipe redirects
        while ((dup2(fds_out[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        while ((dup2(fds_err[1], STDERR_FILENO) == -1) && (errno == EINTR)) {}
        // Child only needs one-way (write) pipes so close read pipes
        close(fds_out[0]);
        close(fds_err[0]);

        execv(util_path, argv);
        fprintf(stderr, "execv failed in switcher_exec(%s, %s): %s", util_path, cmdline, strerror(errno));

        _exit(EXIT_FAILURE);
    }
    // Parent only needs one-way (read) pipes so close write pipes
    close(fds_out[1]);
    close(fds_err[1]);

    // Capture stdout output
    while (1) {
        ssize_t count = read(fds_out[0], buffer, sizeof(buffer)-1);
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        } else if (count == 0) {
            break;
        } else {
            buffer[count] = '\0';
            output_out += buffer;
        }
    }

    // Capture stderr output
    while (1) {
        ssize_t count = read(fds_err[0], buffer, sizeof(buffer)-1);
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        } else if (count == 0) {
            break;
        } else {
            buffer[count] = '\0';
            output_err += buffer;
        }
    }

    // Wait for command to complete, like system() does.
    waitpid(pid, &stat, 0);

    // Close pipe descriptors
    close(fds_out[0]);
    close(fds_err[0]);

    if (WIFEXITED(stat)) {
        retval = WEXITSTATUS(stat);

        if (retval) {
            if (log_flags.task_debug) {
                msg_printf(0, MSG_INTERNAL_ERROR, "[task_debug] failure in switcher_exec");
                msg_printf(0, MSG_INTERNAL_ERROR, "[task_debug]   switcher: %s", util_path);
                msg_printf(0, MSG_INTERNAL_ERROR, "[task_debug]    command: %s", command);
                msg_printf(0, MSG_INTERNAL_ERROR, "[task_debug]  exit code: %d", retval);
                msg_printf(0, MSG_INTERNAL_ERROR, "[task_debug]     stdout: %s", output_out.c_str());
                msg_printf(0, MSG_INTERNAL_ERROR, "[task_debug]     stderr: %s", output_err.c_str());
            }
        }
        return retval;
    }

    return 0;
}

int kill_via_switcher(int pid) {
    char cmd[1024];

    if (!g_use_sandbox) return 0;

    // if project application is running as user boinc_project and
    // client is running as user boinc_master,
    // we cannot send a signal directly, so use switcher.
    //
    snprintf(cmd, sizeof(cmd), "/bin/kill kill -s KILL %d", pid);
    return switcher_exec(SWITCHER_FILE_NAME, cmd);
}

static int lookup_group(const char* name, gid_t& gid) {
    struct group* gp = getgrnam(name);
    if (!gp) return ERR_GETGRNAM;
    gid = gp->gr_gid;
    return 0;
}

int remove_project_owned_file_or_dir(const char* path) {
    char cmd[5120];

    if (g_use_sandbox) {
        snprintf(cmd, sizeof(cmd), "/bin/rm rm -fR \"%s\"", path);
        if (switcher_exec(SWITCHER_FILE_NAME, cmd)) {
            return ERR_UNLINK;
        } else {
            return 0;
        }
    }
    return ERR_UNLINK;
}

int get_project_gid() {
    if (g_use_sandbox) {
#ifdef _DEBUG
        // GDB can't attach to applications which are running as a different user
        //  or group, so fix up data with current user and group during debugging
        gstate.boinc_project_gid = getegid();
#else
        return lookup_group(BOINC_PROJECT_GROUP_NAME, gstate.boinc_project_gid);
#endif  // _DEBUG
    } else {
        gstate.boinc_project_gid = 0;
    }
    return 0;
}

// Graphics apps called by screensaver or Manager (via Show
// Graphics button) now write files in their slot directory as
// the logged in user, not boinc_master. This ugly hack uses
// setprojectgrp to fix all ownerships in this slot directory.
#ifdef __APPLE__
int fix_slot_owners(const int slot){
    char relative_path[100];
    char full_path[MAXPATHLEN];

    if (g_use_sandbox) {
        snprintf(relative_path, sizeof(relative_path), "slots/%d", slot);
        realpath(relative_path, full_path);
        fix_owners_in_directory(full_path);
    }
    return 0;
}
#else
int fix_slot_owners(const int){
    return 0;
}
#endif

int fix_owners_in_directory(char* dir_path) {
    char item_path[MAXPATHLEN];
    char quoted_item_path[MAXPATHLEN+2];
    DIR* dirp;
    struct stat sbuf;
    int retval = 0;
    bool isDirectory = false;
    passwd              *pw;
    uid_t boinc_master_uid = -1;
    uid_t boinc_project_uid = -1;
    gid_t boinc_project_gid = -1;

    pw = getpwnam(BOINC_MASTER_USER_NAME);
    if (pw == NULL) return -1;
    boinc_master_uid = pw->pw_uid;

    pw = getpwnam(BOINC_PROJECT_USER_NAME);
    if (pw == NULL) return -1;
    boinc_project_uid = pw->pw_uid;

    lookup_group(BOINC_PROJECT_GROUP_NAME, boinc_project_gid);

    dirp = opendir(dir_path);
    if (!dirp) return ERR_READDIR;
    while (1) {
        dirent* dp = readdir(dirp);
        if (!dp) break;
        if (dp->d_name[0] == '.') continue;
        snprintf(item_path, sizeof(item_path), "%s/%s", dir_path, dp->d_name);
        retval = lstat(item_path, &sbuf);
        if (retval)
            break;              // Should never happen

        isDirectory = S_ISDIR(sbuf.st_mode);
        if (isDirectory) {
            fix_owners_in_directory(item_path);
        }

       if ( (sbuf.st_uid == boinc_master_uid) || (sbuf.st_uid == boinc_project_uid) ) {
            if (sbuf.st_gid == boinc_project_gid) {
                continue;
            }
        }
        snprintf(quoted_item_path, sizeof(quoted_item_path),"\"%s\"", item_path);
        set_to_project_group(quoted_item_path);
    }
    closedir(dirp);
    return retval;
}

int set_to_project_group(const char* path) {
    if (g_use_sandbox) {
        if (switcher_exec(SETPROJECTGRP_FILE_NAME, path)) {
            return ERR_CHOWN;
        }
    }
    return 0;
}

#else
int get_project_gid() {
    return 0;
}
int set_to_project_group(const char*) {
    return 0;
}
#endif // ! _WIN32

// delete a file.
// return success if we deleted it or it didn't exist in the first place
//
static int delete_project_owned_file_aux(const char* path) {
#ifdef _WIN32
    if (DeleteFile(path)) return 0;
    int error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND) {
        return 0;
    }
    if (error == ERROR_ACCESS_DENIED) {
        SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);
        if (DeleteFile(path)) return 0;
    }
    return error;
#else
    int retval = unlink(path);
    if (retval == 0) return 0;
    if (errno == ENOENT) {
        return 0;
    }
    if (g_use_sandbox && (errno == EACCES)) {
        // We may not have permission to read subdirectories created by projects
        //
        return remove_project_owned_file_or_dir(path);
    }
    return ERR_UNLINK;
#endif
}

// Delete the file located at path.
// If "retry" is set, do retries for 5 sec in case some
// other program (e.g. virus checker) has the file locked.
// Don't do this if deleting directories - it can lock up the Manager.
//
int delete_project_owned_file(const char* path, bool retry) {
    int retval = 0;

    retval = delete_project_owned_file_aux(path);
    if (retval && retry) {
        if (log_flags.slot_debug) {
            msg_printf(0, MSG_INFO,
                "[slot] delete of %s failed (%d); retrying", path, retval
            );
        }
        double start = dtime();
        do {
            boinc_sleep(drand()*2);       // avoid lockstep
            retval = delete_project_owned_file_aux(path);
            if (!retval) break;
        } while (dtime() < start + FILE_RETRY_INTERVAL);
    }
    if (retval) {
        if (log_flags.slot_debug) {
            msg_printf(0, MSG_INFO,
                "[slot] failed to remove file %s: %s",
                path, boincerror(retval)
            );
        }
        safe_strcpy(boinc_failed_file, path);
        return ERR_UNLINK;
    }
    if (log_flags.slot_debug) {
        msg_printf(0, MSG_INFO, "[slot] removed file %s", path);
    }
    return 0;
}

// recursively delete everything in the specified directory
// (but not the directory itself).
// If an error occurs, delete as much as possible.
//
int client_clean_out_dir(
    const char* dirpath, const char* reason, const char* except
) {
    char filename[MAXPATHLEN], path[MAXPATHLEN];
    int retval, final_retval = 0;
    DIRREF dirp;

    if (reason && log_flags.slot_debug) {
        msg_printf(0, MSG_INFO, "[slot] cleaning out %s: %s", dirpath, reason);
    }
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
        safe_strcpy(filename, "");
        retval = dir_scan(filename, dirp, sizeof(filename));
        if (retval) {
            if (retval != ERR_NOT_FOUND) {
                if (log_flags.slot_debug) {
                    msg_printf(0, MSG_INFO,
                        "[slot] dir_scan(%s) failed: %s",
                        dirpath, boincerror(retval)
                    );
                }
                final_retval = retval;
            }
            break;
        }
        if (except && !strcmp(except, filename)) {
            continue;
        }
        snprintf(path, sizeof(path), "%.*s/%.*s", DIR_LEN, dirpath,  FILE_LEN, filename);
        if (is_dir(path)) {
            retval = client_clean_out_dir(path, NULL);
            if (retval) final_retval = retval;
            retval = remove_project_owned_dir(path);
            if (retval) final_retval = retval;
        } else {
            retval = delete_project_owned_file(path, false);
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

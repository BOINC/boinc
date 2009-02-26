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

// Program to install files as part of auto-update.
// Run in a directory that contains the new files.
// Arguments:
//
// --install_dir X      copy files to X (required)
// --run_manager        when done, run Manager
// --run_as_service     when done, run core client as service
//
// What it does:
// 1) wait for a mutex, to ensure that the core client has exited
// 2) create or empty out a "previous-version" dir
// 3) copy files from install dir to previous-version dir
// 4) copy files from current dir to install dir
// 5) run the new core client and/or manager
//
// If we get an error in 2) or 3):
// 6) run the (old) core client and/or manager
//    We pass it a "--run_from_updater" option; this causes it to
//    mark the update as failed, so it won't try again.
// If we get an error in 4) or 5):
// 7) copy files from previous-version dir back to install dir
// 8) run the old core client and/or manager


#include <stdio.h>
#ifdef _WIN32
#else
#include <unistd.h>
#include <cerrno>
#endif

#include <vector>

#include "filesys.h"
#include "util.h"
#include "sandbox.h"
using std::vector;

#ifdef _WIN32
#define CORE_NAME "boinc.exe"
#define MANAGER_NAME "boincmgr.exe"
#else
#define CORE_NAME "boinc_client"
#define MANAGER_NAME "boinc_mgr"
#endif

char* install_dir;
char prev_dir[1024];

int prepare_prev_dir() {
    int retval;

    if (is_dir(prev_dir)) {
        retval = clean_out_dir(prev_dir);
        if (retval) return retval;
    } else {
        retval = boinc_mkdir(prev_dir);
        if (retval) return retval;
    }
    return 0;
}

int move_file(const char* file, char* old_dir, char* new_dir) {
    char old_path[1024], new_path[1024];
    sprintf(old_path, "%s/%s", old_dir, file);
    sprintf(new_path, "%s/%s", new_dir, file);
    int retval = boinc_rename(old_path, new_path);
    fprintf(stderr, "rename %s to %s\n", old_path, new_path);
    if (retval) {
        fprintf(stderr, "couldn't rename %s to %s\n", old_path, new_path);
    }
    return retval;
}

// try to move all; return 0 if moved all
//
int move_files(vector<const char*> files, char* old_dir, char* new_dir) {
    int retval = 0;
    for (unsigned int i=0; i<files.size(); i++) {
        int ret = move_file(files[i], old_dir, new_dir);
        if (ret) retval = ret;
    }
    return retval;
}

// NOTE: this program must always (re)start the core client before exiting

int main(int argc, char** argv) {
    int i, retval, argc2;
    char filepath[512];
#ifdef _WIN32
    HANDLE core_pid, mgr_pid;
#else
    int core_pid, mgr_pid;
#endif
    bool run_as_service = false;
    bool run_manager = false;
    char* argv2[10];
    char cur_dir[1024];
    bool new_version_installed = false;
    vector<const char*> files;

    install_dir = 0;
    for (i=1; i<argc; i++) {
        printf("updater: argv[%d] is %s\n", i, argv[i]);
        if (!strcmp(argv[i], "--run_as_service")) {
            run_as_service = true;
        } else if (!strcmp(argv[i], "--run_manager")) {
            run_manager = true;
        } else if (!strcmp(argv[i], "--install_dir")) {
            install_dir = argv[++i];
        }
    }
    if (!install_dir) {
        fprintf(stderr, "updater: install dir not specified\n");
        retval = 1;
        goto restart;
    }
    sprintf(prev_dir, "%s/prev_version", install_dir);
    boinc_getcwd(cur_dir);
    printf("%s\n", cur_dir);

    wait_client_mutex(install_dir, 30);
    retval = prepare_prev_dir();
    if (retval) {
        fprintf(stderr, "couldn't prepare prev_dir %s: %d\n", prev_dir, retval);
        goto restart;
    }

    files.push_back(CORE_NAME);
    files.push_back(MANAGER_NAME);

    // save existing files to "previous version" dir
    //
    retval = move_files(files, install_dir, prev_dir);
    if (retval) {
        move_files(files, prev_dir, install_dir);
        goto restart;
    }

    retval = move_files(files, cur_dir, install_dir);
    if (retval) {
        move_files(files, prev_dir, install_dir);
        goto restart;
    }

    new_version_installed = true;
    retval = 0;

restart:
    argv2[0] = CORE_NAME;
    argv2[1] = "--run_by_updater";
    argv2[2] = 0;
    argc2 = 2;
    sprintf(filepath, "%s/%s", install_dir, CORE_NAME);
    retval = run_program(install_dir, filepath, argc2, argv2, 5, core_pid);
    if (retval) {
        fprintf(stderr, "failed to run core client (%d); backing out\n", retval);
        if (new_version_installed) {
            retval = move_files(files, prev_dir, install_dir);
            if (retval) exit(retval);   // abandon ship
            new_version_installed = false;
            goto restart;
        }
        exit(retval);
    }
    if (run_manager) {
        argv2[0] = MANAGER_NAME;
        argv2[1] = 0;
        argc2 = 1;
        sprintf(filepath, "%s/%s", install_dir, MANAGER_NAME);
        retval = run_program(install_dir, filepath, argc2, argv2, 5, mgr_pid);
        if (retval) {
            fprintf(stderr, "failed to run manager (%d); backing out\n", retval);
            if (new_version_installed) {
                kill_program(core_pid);
                retval = move_files(files, prev_dir, install_dir);
                if (retval) exit(retval);
                new_version_installed = false;
                goto restart;
            }
            exit(retval);
        }
    }
}

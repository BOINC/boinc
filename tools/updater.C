// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
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

// Program to install files as part of auto-update.
// Run in a version directory.
// Arguments:
//
// --install_dir X      copy files to X (required)
// --run_manager        when done, run Manager
// --run_core           when done, run core client
// --run_as_service     when done, run core client as service

#include <stdio.h>
#ifdef _WIN32
#else
#include <unistd.h>
#include <errno.h>
#endif

#include "filesys.h"
#include "util.h"

#ifdef _WIN32
#define CORE_NAME "boinc.exe"
#define MANAGER_NAME "boincmgr.exe"
#else
#define CORE_NAME "boinc_client"
#define MANAGER_NAME "boinc_mgr"
#endif

char* install_dir;

int prepare_prev_dir() {
    char prev_dir[256];
    int retval;

    sprintf(prev_dir, "%s/prev_version", install_dir);
    if (is_dir(prev_dir)) {
        retval = clean_out_dir(prev_dir);
        if (retval) return retval;
    } else {
        retval = boinc_mkdir(prev_dir);
        if (retval) return retval;
    }
    return 0;
}

int move_to_prev(char* file) {
    char oldname[1024], newname[1024];
    sprintf(oldname, "%s/%s", install_dir, file);
    sprintf(newname, "%s/prev_version/%s", install_dir, file);
    return boinc_rename(oldname, newname);
}

int copy_to_main(char* file) {
    char newname[1024];
    sprintf(newname, "%s/%s", install_dir, file);
    return boinc_copy(file, newname);
}

int main(int argc, char** argv) {
    int i, retval, argc2;
    bool run_as_service = false;
    bool run_core = false;
    bool run_manager = false;
    char* argv2[10];
    char path[1024];

    install_dir = 0;
    for (i=1; i<argc; i++) {
        printf("updater: argv[%d] is %s\n", i, argv[i]);
        if (!strcmp(argv[i], "--run_as_service")) {
            run_as_service = true;
        } else if (!strcmp(argv[i], "--run_manager")) {
            run_manager = true;
        } else if (!strcmp(argv[i], "--run_core")) {
            run_core = true;
        } else if (!strcmp(argv[i], "--install_dir")) {
            install_dir = argv[++i];
        }
    }
    if (!install_dir) {
        fprintf(stderr, "updater: install dir not specified\n");
        exit(1);
    }

    wait_client_mutex(install_dir, 30);
    retval = prepare_prev_dir();
    if (retval) exit(retval);
    move_to_prev(CORE_NAME);
    //move_to_prev(MANAGER_NAME);
    copy_to_main(CORE_NAME);
    //copy_to_main(MANAGER_NAME);
    if (run_core) {
        argv2[0] = CORE_NAME;
        argv2[1] = 0;
        argc2 = 1;
        run_program(install_dir, CORE_NAME, argc2, argv2);
    }
    if (run_manager) {
        argv2[0] = MANAGER_NAME;
        argv2[1] = 0;
        argc2 = 1;
        run_program(install_dir, MANAGER_NAME, argc2, argv2);
    }
}

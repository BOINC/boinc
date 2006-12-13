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

// This program is run in a version directory.
// The main BOINC directory is ../../..

#include <stdio.h>
#ifdef _WIN32
#else
#include <unistd.h>
#include <errno.h>
#endif

#include "filesys.h"

#define MAIN_DIR "../../.."
#ifdef _WIN32
#define CORE_NAME "boinc.exe"
#define MANAGER_NAME "boincmgr.exe"
#else
#define CORE_NAME "boinc"
#define MANAGER_NAME "boinc_mgr"
#endif

int run_program(char* file, char** argv) {
#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
             
    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
             
    retval = CreateProcess(
        file,
        NULL,
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_PROCESS_GROUP,
        NULL,
        MAIN_DIR,
        &startupinfo
        $processinfo
    );
#else
    int pid = fork();
    if (pid == 0) {
        chdir(MAIN_DIR);
        execv(file, argv);
        perror("execv");
    }
#endif
}

int prepare_prev_dir() {
    char prev_dir[256];
    int retval;

    sprintf(prev_dir, "%s/prev_version", MAIN_DIR);
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
    sprintf(oldname, "%s/%s", MAIN_DIR, file);
    sprintf(newname, "%s/prev_version/%s", MAIN_DIR, file);
    return boinc_rename(oldname, newname);
}

int copy_to_main(char* file) {
    char newname[1024];
    sprintf(newname, "%s/%s", MAIN_DIR, file);
    return boinc_copy(file, newname);
}

int main(int argc, char** argv) {
    int i, retval;
    bool run_as_service = false;
    bool run_core = false;
    bool run_manager = false;
    char* argv2[10];

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--run_as_service")) {
            run_as_service = true;
        } else if (!strcmp(argv[i], "--run_manager")) {
            run_manager = true;
        } else if (!strcmp(argv[i], "--run_core")) {
            run_core = true;
        }
    }

    retval = prepare_prev_dir();
    if (retval) exit(retval);
    move_to_prev(CORE_NAME);
    move_to_prev(MANAGER_NAME);
    copy_to_main(CORE_NAME);
    copy_to_main(MANAGER_NAME);
    if (run_core) {
        argv2[0] = CORE_NAME;
        argv2[1] = 0;
        run_program(CORE_NAME, argv2);
    }
    if (run_manager) {
        argv2[0] = MANAGER_NAME;
        argv2[1] = 0;
        run_program(MANAGER_NAME, argv2);
    }
}

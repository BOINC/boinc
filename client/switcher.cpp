// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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

// switcher.C
//
// When run as
// switcher Full-Path Executable-Name X1 ... Xn
// runs program at Full-Path with args X1. ... Xn
// note that the executable name must be specified twice:
//  once as part of the Full_Path and again as just the name

#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <pwd.h>    // getpwuid
#include <grp.h>
#include <sys/stat.h>  // for chmod

#include "app_ipc.h"
#include "filesys.h"
#include "str_replace.h"
#include "mac_spawn.h"

using std::strcpy;

int main(int /*argc*/, char** argv) {
    passwd          *pw;
    group           *grp;
    char            boinc_project_user_name[256], boinc_project_group_name[256];
    char            boinc_master_user_name[256];
    APP_INIT_DATA   aid;
    FILE            *f;
    int             retval = -1;
    int             i;
    char            libpath[8192];
    char            newlibs[256];
    char            *projectDirName;
    const char      *screensaverLoginUser = NULL;
    bool            launching_gfx=false;
    char            current_dir[MAXPATHLEN];


    strcpy(boinc_project_user_name, "boinc_project");
    strcpy(boinc_project_group_name, "boinc_project");
    strcpy(boinc_master_user_name, "boinc_master");

#if 0           // For debugging only
    fprintf(stderr, "\n\nEntered switcher with euid %d, egid %d, uid %d and gid %d\n", geteuid(), getegid(), getuid(), getgid());       
    getcwd( current_dir, sizeof(current_dir));
    fprintf(stderr, "current directory = %s\n", current_dir);
    fflush(stderr);
    
    i = 0;
    while(argv[i]) {
        fprintf(stderr, "switcher arg %d: %s\n", i, argv[i]);
        fflush(stderr);
        ++i;
    }
#endif

#if 0       // For debugging only
    // Allow debugging without running as user or group boinc_project
    pw = getpwuid(getuid());
    if (pw) {
        strcpy(boinc_project_user_name, pw->pw_name);
        strcpy(boinc_master_user_name, pw->pw_name);
    }
    grp = getgrgid(getgid());
    if (grp) {
        strcpy(boinc_project_group_name, grp->gr_gid);
    }

#endif

    // Under fast user switching, the BOINC client may be running under a
    // different login than the screensaver 
    //
    // If we need to join a different process group, it must be the last argument.
    // This is currently used for OS 10.15+
    i = 0;
    while(argv[i]) {
        if (!strcmp(argv[i], "--ScreensaverLoginUser")) {
            screensaverLoginUser = argv[i+1];
            break;
        }
        ++i;
    }
 
    if (!screensaverLoginUser) {
        // Satisfy an error / warning from rpmlint: ensure that
        // we drop any supplementary groups associated with root
        setgroups(0, NULL);

        // We are running setuid root, so setgid() sets real group ID,
        // effective group ID and saved set_group-ID for this process
        grp = getgrnam(boinc_project_group_name);
        if (grp) {
            (void) setgid(grp->gr_gid);
        }

        // We are running setuid root, so setuid() sets real user ID,
        // effective user ID and saved set_user-ID for this process
        pw = getpwnam(boinc_project_user_name);
        if (pw) {
            (void) setuid(pw->pw_uid);
        }
    }
    // For unknown reasons, the LD_LIBRARY_PATH and DYLD_LIBRARY_PATH
    // environment variables are not passed in to switcher, though all
    // other environment variables do get propagated.  So we recreate
    // LD_LIBRARY_PATH and DYLD_LIBRARY_PATH here.
    f = fopen(INIT_DATA_FILE, "r");
    if (f) {
        retval = parse_init_data_file(f, aid);
        fclose(f);
    }

    if (!retval) {
        // Get project name without leading path
        projectDirName = strrchr(aid.project_dir, '/');
        if (projectDirName) {
            ++projectDirName;
        } else {
            projectDirName = aid.project_dir;
        }
        sprintf(newlibs, "../../%s:.:../..", projectDirName);
#ifdef __APPLE__
        strcat(newlibs, ":/usr/local/cuda/lib/");
#endif
        char* p = getenv("LD_LIBRARY_PATH");
        if (p) {
            snprintf(libpath, sizeof(libpath), "%s:%s", newlibs, p);
        } else {
            safe_strcpy(libpath, newlibs);
        }
        setenv("LD_LIBRARY_PATH", libpath, 1);

        // On the Mac, do the same for DYLD_LIBRARY_PATH
        //
#ifdef __APPLE__
        p = getenv("DYLD_LIBRARY_PATH");
        if (p) {
            sprintf(libpath, "%s:%s", newlibs, p);
        } else {
            safe_strcpy(libpath, newlibs);
        }
        setenv("DYLD_LIBRARY_PATH", libpath, 1);
#endif
    }

   if (screensaverLoginUser) {
       // Used under OS 10.15 Catalina and later to launch screensaver graphics apps
       //
       // BOINC screensaver plugin BOINCSaver.saver (BOINC Screensaver Coordinator)
       // sends a run_graphics_app RPC to the BOINC client. The BOINC client then 
       // launches switcher, which submits a script to launchd as a LaunchAgent
       // for the user that invoked the screensaver (the currently logged in user.)
       //
       // We must go through launchd to establish a connection to the windowserver 
       // in the currently logged in user's space for use by the project graphics
       // app. This script then launches gfx_switcher, which uses fork and execv to 
       // launch the project graphics app. gfx_switcher writes the graphics app's 
       // process ID to shared memory, to be read by the Screensaver Coordinator. 
       // gfx_switcher waits for the graphics app to exit and notifies then notifies 
       // the Screensaver Coordinator by writing 0 to the shared memory.
       //
       // This Rube Goldberg process is necessary due to limitations on screensavers
       // introduced in OS 10.15 Catalina.
        char cmd[1024];

        // We are running setuid root, so setuid() sets real user ID, 
        // effective user ID and saved set_user-ID for this process
        setuid(geteuid());
        // We are running setuid root, so setgid() sets real group ID, 
        // effective group ID and saved set_group-ID for this process
        setgid(getegid());
        
        getcwd(current_dir, sizeof(current_dir));

        i = 0;
        while(argv[i]) {
            if (strcmp(argv[i], "/bin/kill")) {
                launching_gfx = true;   // not KILL command
                break;
            }
            ++i;
        }

        if (!strcmp(argv[2], "-kill_gfx")) {
            snprintf(cmd, sizeof(cmd), "\"/Library/Screen Savers/%s.saver/Contents/Resources/gfx_switcher\" %s %s", argv[1], argv[2], argv[3]);
            retval = callPosixSpawn((const char*)cmd);
            return retval;
       } else {
            // A new submit of edu.berkeley.boinc-ss_helper will be ignored if for some reason 
            // edu.berkeley.boinc-ss_helper is still loaded, so ensure it is removed.
            snprintf(cmd, sizeof(cmd), "su -l \"%s\" -c 'launchctl remove edu.berkeley.boinc-ss_helper'", screensaverLoginUser);
            retval = callPosixSpawn((const char*)cmd);

            snprintf(cmd, sizeof(cmd), "su -l \"%s\" -c 'launchctl submit -l edu.berkeley.boinc-ss_helper -- \"/Library/Screen Savers/%s.saver/Contents/Resources/boinc_ss_helper.sh\" \"%s\"", screensaverLoginUser, argv[2], argv[1]);
            i = 2;
            while(argv[i]) {
                safe_strcat(cmd, " ");
                safe_strcat(cmd, argv[i++]);
            }
        }
        safe_strcat(cmd, "\'");
#if 0       // For debugging only
        fprintf(stderr, "About to call Posix Spawn (%s)\n", cmd);
        fflush(stderr);
#endif
        if (launching_gfx) {
            // Suppress "killed" message from bash in stderrdae.txt
            freopen("/dev/null", "a", stderr);
        }
        retval = callPosixSpawn((const char*)cmd);
        return retval;
    }


    retval = execv(argv[1], argv+2);
    if (retval == -1) {
        // If we got here execv failed
        fprintf(stderr, "Process creation (%s) failed: %s (errno = %d)\n", argv[1], strerror(errno), retval);
    }

    return retval;
}

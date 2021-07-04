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

// gfx_switcher.C
//
// Used by screensaver to:
//  - launch graphics application at given slot number as user & group boinc_project
//  - launch default graphics application as user & group boinc_project
//  - kill graphics application with given process ID as user & group boinc_project
//

// Special logic used only under OS 10.15 Catalina and later:
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
//


#include <SystemConfiguration/SystemConfiguration.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#if HAVE_SYS_PARAM_H
#include <sys/param.h>  // for MAXPATHLEN
#endif
#include <pwd.h>	// getpwuid
#include <grp.h>
#include <signal.h> // For kill()
#include <pthread.h>

#include "boinc_api.h"
#include "common_defs.h"
#include "util.h"
#include "mac_util.h"
#include "shmem.h"

#define VERBOSE 0
#define CREATE_LOG VERBOSE

#if VERBOSE
#ifdef __cplusplus
extern "C" {
#endif
static void print_to_log_file(const char *format, ...);
#ifdef __cplusplus
}

static void strip_cr(char *buf);
#endif
#else
#define print_to_log_file(...)
#endif

void * MonitorScreenSaverEngine(void* param);

pid_t* pid_for_shmem = NULL;

int main(int argc, char** argv) {
    passwd      *pw;
    group       *grp;
    char        user_name[256], group_name[256];
    char	    gfx_app_path[MAXPATHLEN], resolved_path[MAXPATHLEN];
    char        *BOINCDatSlotsPath = "/Library/Application Support/BOINC Data/slots/";
    int         retval;
    int         pid;
    int         i;
    const char  *screensaverLoginUser = NULL;
    pthread_t   monitorScreenSaverEngineThread = 0;

    if (argc < 2) return EINVAL;

    CFStringRef cf_gUserName = SCDynamicStoreCopyConsoleUser(NULL, NULL, NULL);
    CFStringGetCString(cf_gUserName, user_name, sizeof(user_name), kCFStringEncodingUTF8);
//    strlcpy(user_name, getlogin(), sizeof(user_name));
    strlcpy(group_name, "boinc_project", sizeof(group_name));

    // Under fast user switching, the BOINC client may be running under a
    // different login than the screensaver 
    //
    // If we need to join a different process group, it must be the last argument.
    i = 0;
    while(argv[i]) {
        if (!strcmp(argv[i], "--ScreensaverLoginUser")) {
           screensaverLoginUser = argv[i+1];
 //          strlcpy(user_name, screensaverLoginUser, sizeof(user_name));
            argv[i] = 0;    // Strip off the --ScreensaverLoginUser argument
            argc -= 2;
#if VERBOSE           // For debugging only
            print_to_log_file("\n\ngfx_switcher: screensaverLoginUser = %s", screensaverLoginUser);
#endif
            break;
        }
        ++i;
    }

#if 0       // For debugging only
    // Allow debugging without running as user or group boinc_project
    pw = getpwuid(getuid());
    if (pw) strlcpy(user_name, pw->pw_name, sizeof(user_name));
    grp = getgrgid(getgid());
    if (grp) strlcpy(group_name, grp->gr_gid, sizeof(group_name));

#endif
    // We are running setuid root, so setgid() sets real group ID, 
    // effective group ID and saved set_group-ID for this process
    grp = getgrnam(group_name);
    if (grp) setgid(grp->gr_gid);

    // We are running setuid root, so setuid() sets real user ID, 
    // effective user ID and saved set_user-ID for this process
    strlcpy(user_name, "boinc_project", sizeof(user_name));
    pw = getpwnam(user_name);
    if (pw) setuid(pw->pw_uid);

    // NOTE: call print_to_log_file only after switching user and group
#if VERBOSE           // For debugging only
    char	current_dir[MAXPATHLEN];

    getcwd( current_dir, sizeof(current_dir));
    print_to_log_file("current directory = %s", current_dir);
    print_to_log_file("user_name is %s, euid=%d, uid=%d, egid=%d, gid=%d", user_name, geteuid(), getuid(), getegid(), getgid());
    
    for (int i=0; i<argc; i++) {
         print_to_log_file("gfx_switcher arg %d: %s", i, argv[i]);
    }
#endif

    if (strcmp(argv[1], "-default_gfx") == 0) {
        strlcpy(resolved_path, "/Library/Application Support/BOINC Data/boincscr", sizeof(resolved_path));
        argv[2] = resolved_path;
        
#if VERBOSE           // For debugging only
        for (int i=2; i<argc; i++) {
            print_to_log_file("gfx_switcher calling execv with arg %d: %s", i-2, argv[i]);
        }
#endif

        // For unknown reasons, the graphics application exits with 
        // "RegisterProcess failed (error = -50)" unless we pass its 
        // full path twice in the argument list to execv.
        if (! screensaverLoginUser) {
            execv(resolved_path, argv+2);
            // If we got here execv failed
            fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
            return errno;
        } else {   // if screensaverLoginUser
#if VERBOSE           // For debugging only
            print_to_log_file("gfx_switcher using fork()");
#endif
            int pid = fork();
            if (pid == 0) {
               // For unknown reasons, the graphics application exits with 
                // "RegisterProcess failed (error = -50)" unless we pass its 
                // full path twice in the argument list to execv.
                execv(resolved_path, argv+2);
                // If we got here execv failed
#if VERBOSE           // For debugging only
                print_to_log_file("gfx_switcher: Process creation (%s) failed: errno=%d\n", resolved_path, errno);
#endif
                fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
                return errno;
            } else {
                char shmem_name[MAXPATHLEN];
#if VERBOSE           // For debugging only
                print_to_log_file("gfx_switcher: Child PID=%d", pid);
#endif
                snprintf(shmem_name, sizeof(shmem_name), "/tmp/boinc_ss_%s", screensaverLoginUser);
                retval = attach_shmem_mmap(shmem_name, (void**)&pid_for_shmem);
                if (pid_for_shmem != 0) {
                    *pid_for_shmem = pid;
                }
                pthread_create(&monitorScreenSaverEngineThread, NULL, MonitorScreenSaverEngine, &pid);
                waitpid(pid, 0, 0);
                pthread_cancel(monitorScreenSaverEngineThread);
                if (pid_for_shmem != 0) {
                    *pid_for_shmem = 0;
                }
                return 0;
            }
        }
    }
    
    if (strcmp(argv[1], "-launch_gfx") == 0) {
        strlcpy(gfx_app_path, BOINCDatSlotsPath, sizeof(gfx_app_path));
        strlcat(gfx_app_path, argv[2], sizeof(gfx_app_path));
        strlcat(gfx_app_path, "/", sizeof(gfx_app_path));
        strlcat(gfx_app_path, GRAPHICS_APP_FILENAME, sizeof(gfx_app_path));
        retval = boinc_resolve_filename(gfx_app_path, resolved_path, sizeof(resolved_path));
        if (retval) return retval;
        
        argv[2] = resolved_path;
        
#if VERBOSE           // For debugging only
        for (int i=2; i<argc; i++) {
             print_to_log_file("gfx_switcher calling execv with arg %d: %s", i-2, argv[i]);
        }
#endif

        if (! screensaverLoginUser) {
            // For unknown reasons, the graphics application exits with 
            // "RegisterProcess failed (error = -50)" unless we pass its 
            // full path twice in the argument list to execv.
            execv(resolved_path, argv+2);
            // If we got here execv failed
            fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
            return errno;
         } else {   // if useScreenSaverLaunchAgent            
#if VERBOSE           // For debugging only
            print_to_log_file("gfx_switcher using fork()");;
#endif
            int pid = fork();
            if (pid == 0) {
               // For unknown reasons, the graphics application exits with 
                // "RegisterProcess failed (error = -50)" unless we pass its 
                // full path twice in the argument list to execv.
                execv(resolved_path, argv+2);
                // If we got here execv failed
                fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
                return errno;
            } else {
                char shmem_name[MAXPATHLEN];
                snprintf(shmem_name, sizeof(shmem_name), "/tmp/boinc_ss_%s", screensaverLoginUser);
                retval = attach_shmem_mmap(shmem_name, (void**)&pid_for_shmem);
                if (pid_for_shmem != 0) {
                    *pid_for_shmem = pid;
                }
                pthread_create(&monitorScreenSaverEngineThread, NULL, MonitorScreenSaverEngine, &pid);
                waitpid(pid, 0, 0);
                pthread_cancel(monitorScreenSaverEngineThread);
                if (pid_for_shmem != 0) {
                    *pid_for_shmem = 0;
                }
                return 0;
            }
        }
    }

    if (strcmp(argv[1], "-kill_gfx") == 0) {
        pid = atoi(argv[2]);
        if (! pid) return EINVAL;
        if ( kill(pid, SIGKILL)) {
#if VERBOSE           // For debugging only
     print_to_log_file("kill(%d, SIGKILL) returned error %d", pid, errno);
#endif
            return errno;
        }
        return 0;
    }
    
    return EINVAL;  // Unknown command
}

// For extra safety, kill our graphics app if ScreenSaverEngine has exited
void * MonitorScreenSaverEngine(void* param) {
    pid_t ScreenSaverEngine_Pid = 0;
    pid_t graphics_Pid = *(pid_t*)param;
    
    while (true) {
        boinc_sleep(1.0);  // Test every second
        ScreenSaverEngine_Pid = getPidIfRunning("com.apple.ScreenSaver.Engine");
#if VERBOSE           // For debugging only
        print_to_log_file("MonitorScreenSaverEngine: ScreenSaverEngine_Pid=%d", ScreenSaverEngine_Pid);
#endif
        if (ScreenSaverEngine_Pid == 0) {
#ifdef __x86_64__
            ScreenSaverEngine_Pid = getPidIfRunning("com.apple.ScreenSaver.Engine.legacyScreenSaver.x86_64");
#elif defined(__arm64__)
            ScreenSaverEngine_Pid = getPidIfRunning("com.apple.ScreenSaver.Engine.legacyScreenSaver.arm64");
#endif
#if VERBOSE           // For debugging only
        print_to_log_file("MonitorScreenSaverEngine: ScreenSaverEngine_legacyScreenSaver_Pid=%d", ScreenSaverEngine_Pid);
#endif
        }
     
    if (ScreenSaverEngine_Pid == 0) {
        kill(graphics_Pid, SIGKILL);
#if VERBOSE           // For debugging only
        print_to_log_file("MonitorScreenSaverEngine calling kill(%d, SIGKILL", graphics_Pid);
#endif
            return 0;
        }
    }
}

#if CREATE_LOG

#include <sys/stat.h>

static void print_to_log_file(const char *format, ...) {
    FILE *f;
    va_list args;
    char buf[256];
    time_t t;
    
    f = fopen("/Users/Shared/test_log_gfx_switcher.txt", "a");
    if (!f) return;

//  freopen(buf, "a", stdout);
//  freopen(buf, "a", stderr);

    time(&t);
    safe_strcpy(buf, asctime(localtime(&t)));
    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    
    fputs("\n", f);
    fflush(f);
    fclose(f);
    chmod("/Users/Shared/test_log_gfx_switcher.txt", 0666);
}

static void strip_cr(char *buf)
{
    char *theCR;

    theCR = strrchr(buf, '\n');
    if (theCR)
        *theCR = '\0';
    theCR = strrchr(buf, '\r');
    if (theCR)
        *theCR = '\0';
}
#endif	// CREATE_LOG

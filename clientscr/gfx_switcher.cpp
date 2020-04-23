// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#if HAVE_SYS_PARAM_H
#include <sys/param.h>  // for MAXPATHLEN
#endif
#include <pwd.h>	    // getpwuid
#include <grp.h>
#include <signal.h>     // For kill()
#include <sys/stat.h>   // for chmod
#include <pthread.h>

#include "boinc_api.h"
#include "common_defs.h"
#include "util.h"
#include "mac_util.h"

#define CREATE_LOG 0
#define VERBOSE_DEBUG 0 

#if CREATE_LOG
#ifdef __cplusplus
extern "C" {
#endif
static void print_to_log_file(const char *format, ...);
#ifdef __cplusplus
}

static void strip_cr(char *buf);
#endif
#endif  // if CREATE_LOG

void * MonitorScreenSaverEngine(void* param);

#define MAXARGS 16

int main(int argc, char** argv) {
    char        *args[MAXARGS];
    char        argString[MAXARGS][MAXPATHLEN];
    Boolean     useScreenSaverLaunchAgent = false;
    FILE        *f = NULL;
    char        helper_app_dir[MAXPATHLEN], helper_app_path[MAXPATHLEN];
    char        *helper_app_base_path;
    char        *ptr;
    int         argsCount = 0;
    passwd      *pw;
    group       *grp;
    char        user_name[256], group_name[256];
    char        gfx_app_path[MAXPATHLEN], resolved_path[MAXPATHLEN];
    char        *BOINCDatSlotsPath = "/Library/Application Support/BOINC Data/slots/";
    char        *BOINCPidFilePath = "/Users/Shared/BOINC/BOINCGfxPid.txt";
    int         retval = 0;
    int         pid;
    pthread_t   monitorScreenSaverEngineThread = 0;

    // As of OS 10.15 (Catalina) screensavers can no longer:
    //  - launch apps that run setuid or setgid
    //  - launch apps downloaded from the Internet which have not been 
    //    specifically approved by the user via Gatekeeper.
    // So instead of launching graphics apps via gfx_switcher, we write 
    // a file containing the information. The file is detected by 
    // a LaunchAgent which then launches gfx_switcher for us. Though we
    // confirmed it works on OS 10.13 High Sierra, we don't use it there.
    //
    // MIN_OS_TO_USE_SCREENSAVER_LAUNCH_AGENT is defined in mac_util.h
    useScreenSaverLaunchAgent = compareOSVersionTo(10, MIN_OS_TO_USE_SCREENSAVER_LAUNCH_AGENT) >= 0;
 
    if (useScreenSaverLaunchAgent) {
        if (compareOSVersionTo(10, 15) >= 0) {
            helper_app_base_path = "Containers/com.apple.ScreenSaver.Engine.legacyScreenSaver/Data/Library/";
        } else {
            helper_app_base_path = "";
        }

        pw = getpwuid(getuid());
        if (pw) {
            snprintf(helper_app_dir, sizeof(helper_app_dir), "/Users/%s/Library/%sApplication Support/BOINC/", pw->pw_name, helper_app_base_path);
        }
#if 0
        safe_strcpy(helper_app_path, BOINCPidFilePath);
        if (boinc_file_exists(helper_app_path)) {
            boinc_delete_file(helper_app_path);
        }
#endif
        safe_strcpy(helper_app_path, helper_app_dir);
        safe_strcat(helper_app_path, "BOINCSSHelper.txt");
        f = fopen(helper_app_path, "r");
        if (f) {
            for (argsCount=0; argsCount<MAXARGS; argsCount++) {
                args[argsCount] = fgets(argString[argsCount], sizeof(argString[argsCount]), f);
                if (args[argsCount]) {
                    ptr = strrchr(args[argsCount], '\n');
                    if (ptr) *ptr = '\0';  // Remove newline character if present
                    ptr = strrchr(args[argsCount], '\r');
                    if (ptr) *ptr = '\0';  // Remove return character if present
                }
                if (argsCount) chdir(argString[0]);
            }
            fclose(f);
            f = NULL;
        } else {
            retval = -1;
        }
        boinc_delete_file(helper_app_path);
        if (retval) return EINVAL;
    } else {
        for (argsCount=0; argsCount<=argc; argsCount++) {
            args[argsCount] = argv[argsCount];
        }
    }
    
    strlcpy(user_name, "boinc_project", sizeof(user_name));
    strlcpy(group_name, user_name, sizeof(group_name));

#if 0       // For debugging only
    // Allow debugging without running as user or group boinc_project
    pw = getpwuid(getuid());
    if (pw) strlcpy(user_name, pw->pw_name, sizeof(user_name));
    grp = getgrgid(getgid());
    if (grp) strlcpy(group_name, grp->gr_name, sizeof(group_name));
#endif

    // We are running setuid root, so setgid() sets real group ID, 
    // effective group ID and saved set_group-ID for this process
    grp = getgrnam(group_name);
    if (grp) setgid(grp->gr_gid);

    // We are running setuid root, so setuid() sets real user ID, 
    // effective user ID and saved set_user-ID for this process
    pw = getpwnam(user_name);
    if (pw) setuid(pw->pw_uid);

    // NOTE: call print_to_log_file only after switching user and group
#if VERBOSE_DEBUG           // For debugging only
    char	current_dir[MAXPATHLEN];

    getcwd( current_dir, sizeof(current_dir));
    print_to_log_file( "current directory = %s", current_dir);
    
    for (int i=0; i<argsCount; i++) {
         print_to_log_file("gfx_switcher arg %d: %s", i, args[i]);
         if (!args[i]) break;
    }
#endif

    if (strcmp(args[1], "-default_gfx") == 0) {
        strlcpy(resolved_path, "/Library/Application Support/BOINC Data/boincscr", sizeof(resolved_path));

        args[2] = resolved_path;
        
#if VERBOSE_DEBUG           // For debugging only
        for (int i=2; i<argc; i++) {
            print_to_log_file("calling execv with arg %d: %s", i-2, args[i]);
        }
#endif
        if (! useScreenSaverLaunchAgent) {
            // For unknown reasons, the graphics application exits with 
            // "RegisterProcess failed (error = -50)" unless we pass its 
            // full path twice in the argument list to execv.
            execv(resolved_path, args+2);
            // If we got here execv failed
            fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
            return errno;
        } else {   // if useScreenSaverLaunchAgent
            int pid = fork();
            if (pid == 0) {
               // For unknown reasons, the graphics application exits with 
                // "RegisterProcess failed (error = -50)" unless we pass its 
                // full path twice in the argument list to execv.
                execv(resolved_path, args+2);
                // If we got here execv failed
                fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
                return errno;
            } else {
                if (!boinc_file_exists("/Users/Shared/BOINC")) {
                    boinc_mkdir("/Users/Shared/BOINC");
                    chmod("/Users/Shared/BOINC", 0777);
                }
                safe_strcpy(helper_app_path, BOINCPidFilePath);
                f = fopen(helper_app_path, "w");
                if (f) {
                    fprintf(f, "%d\n", pid);
                    fclose(f);
                    f = NULL;
                    chmod(helper_app_path, 0644);
                }

                pthread_create(&monitorScreenSaverEngineThread, NULL, MonitorScreenSaverEngine, &pid);
                waitpid(pid, 0, 0);
                boinc_delete_file(helper_app_path);
                pthread_cancel(monitorScreenSaverEngineThread);
                return 0;
            }
        }
    }
    
    if (strcmp(args[1], "-launch_gfx") == 0) {
        strlcpy(gfx_app_path, BOINCDatSlotsPath, sizeof(gfx_app_path));
        strlcat(gfx_app_path, args[2], sizeof(gfx_app_path));
        strlcat(gfx_app_path, "/", sizeof(gfx_app_path));
        strlcat(gfx_app_path, GRAPHICS_APP_FILENAME, sizeof(gfx_app_path));
        retval = boinc_resolve_filename(gfx_app_path, resolved_path, sizeof(resolved_path));
        if (retval) return retval;
        
        args[2] = resolved_path;

#if VERBOSE_DEBUG   // For debugging only
            for (int i=2; i<argc; i++) {
                print_to_log_file("calling execv with arg %d: %s", i-2, args[i]);
            }
#endif
        if (! useScreenSaverLaunchAgent) {
            // For unknown reasons, the graphics application exits with 
            // "RegisterProcess failed (error = -50)" unless we pass its 
            // full path twice in the argument list to execv.
            execv(resolved_path, args+2);
            // If we got here execv failed
            fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
            return errno;
        } else {   // if useScreenSaverLaunchAgent            
            int pid = fork();
            if (pid == 0) {
                   // For unknown reasons, the graphics application exits with 
                    // "RegisterProcess failed (error = -50)" unless we pass its 
                    // full path twice in the argument list to execv.
                    execv(resolved_path, args+2);
                    // If we got here execv failed
                    fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
                    return errno;
                } else {
                    if (!boinc_file_exists("/Users/Shared/BOINC")) {
                        boinc_mkdir("/Users/Shared/BOINC");
                        chmod("/Users/Shared/BOINC", 0777);
                    }
                    safe_strcpy(helper_app_path, BOINCPidFilePath);
                    f = fopen(helper_app_path, "w");
                    if (f) {
                        fprintf(f, "%d\n", pid);
                        fclose(f);
                        f = NULL;
                        chmod(helper_app_path, 0644);
                    }
                
                    pthread_create(&monitorScreenSaverEngineThread, NULL, MonitorScreenSaverEngine, &pid);

                    waitpid(pid, 0, 0);
                    boinc_delete_file(helper_app_path);
                    pthread_cancel(monitorScreenSaverEngineThread);
                    return 0;
                }
            }
    }
    
    if (strcmp(args[1], "-kill_gfx") == 0) {
        pid = atoi(args[2]);
        if (! pid) return EINVAL;
        if ( kill(pid, SIGKILL)) {
#if VERBOSE_DEBUG           // For debugging only
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
        if (ScreenSaverEngine_Pid == 0) {
            kill(graphics_Pid, SIGKILL);
#if VERBOSE_DEBUG           // For debugging only
            print_to_log_file("MonitorScreenSaverEngine calling kill(%d, SIGKILL", graphics_Pid);
#endif
            return 0;
        }
    }
}

#if CREATE_LOG

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

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// gfx_switcher.cpp
//
// Used by screensaver to:
//  - launch graphics application at given slot number
//  - launch default graphics application
//  - kill graphics application with given process ID
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
// gfx_switcher waits for the graphics app to exit and then notifies the
// Screensaver Coordinator by writing 0 to the shared memory.
//
// This Rube Goldberg process is necessary due to limitations on screensavers
// introduced in OS 10.15 Catalina and OS 14.0 Sonoma.
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
#include <spawn.h>

#include "boinc_api.h"
#include "common_defs.h"
#include "util.h"
#include "mac_util.h"
#include "shmem.h"

#define VERBOSE 0
#define CREATE_LOG VERBOSE

#if CREATE_LOG
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

// struct ss_shmem_data must be kept in sync in these files:
// screensaver.cpp
// gfx_switcher.cpp
// gfx_cleanup.mm
// graphics2_unix.cpp
struct ss_shmem_data {
    pid_t gfx_pid;
    int gfx_slot;
    int major_version;
    int minor_version;
    int release;
};

static struct ss_shmem_data* ss_shmem = NULL;

int main(int argc, char** argv) {
    passwd      *pw;
    group       *grp;
    char        user_name[256], group_name[256];
    char        cmd[2048];
    char	    gfx_app_path[MAXPATHLEN], resolved_path[MAXPATHLEN];
    char        *BOINCDatSlotsPath = "/Library/Application Support/BOINC Data/slots/";
    int         retval;
    int         pid;
    int         i;
    const char  *screensaverLoginUser = NULL;
    pthread_t   monitorScreenSaverEngineThread = 0;
    char        shmem_name[MAXPATHLEN];

    if (argc < 2) return EINVAL;

    CFStringRef cf_gUserName = SCDynamicStoreCopyConsoleUser(NULL, NULL, NULL);
    CFStringGetCString(cf_gUserName, user_name, sizeof(user_name), kCFStringEncodingUTF8);
//    strlcpy(user_name, getenv("USER"), sizeof(user_name));
    strlcpy(group_name, "boinc_project", sizeof(group_name));

    // Under fast user switching, the BOINC client may be running under a
    // different login than the screensaver
    //
    // If we need to join a different process group, it must be the last argument.
    i = 0;
    while(argv[i]) {
        if (!strcmp(argv[i], "--ScreensaverLoginUser")) {
           screensaverLoginUser = argv[i+1];
//           strlcpy(user_name, screensaverLoginUser, sizeof(user_name));
            argv[i] = 0;    // Strip off the --ScreensaverLoginUser argument
            argc -= 2;
#if VERBOSE           // For debugging only
            print_to_log_file("\n\ngfx_switcher: screensaverLoginUser = %s", screensaverLoginUser);
#endif
            break;
        }
        ++i;
    }

    if (! screensaverLoginUser) {
        screensaverLoginUser = user_name;
    }

#if 0       // For debugging only
    // Allow debugging without running as group boinc_project
    grp = getgrgid(getgid());
    if (grp) strlcpy(group_name, grp->gr_name, sizeof(group_name));
#endif

    // We are running setuid root, so setgid() sets real group ID,
    // effective group ID and saved set_group-ID for this process
    grp = getgrnam(group_name);
    if (grp) setgid(grp->gr_gid);

    // As of MacOS 13.0 Ventura IOSurface cannot be used to share graphics
    // between apps unless they are running as the same user, so we no
    // longer run the graphics apps as user boinc_master.
    pw = getpwnam(user_name);
    if (pw) setuid(pw->pw_uid);

    // NOTE: call print_to_log_file only after switching user and group
#if VERBOSE           // For debugging only
    char	current_dir[MAXPATHLEN];

    getcwd( current_dir, sizeof(current_dir));
    print_to_log_file("current directory = %s", current_dir);
    print_to_log_file("user_name is %s, euid=%d, uid=%d, egid=%d, gid=%d", user_name, geteuid(), getuid(), getegid(), getgid());
    print_to_log_file("gfx_switcher pid = %d", getpid()) ;
    pid_t ScreenSaverEngine_Pid = 0;
    ScreenSaverEngine_Pid = getPidIfRunning("com.apple.ScreenSaver.Engine.legacyScreenSaver");
    print_to_log_file("gfx_switcher main: ScreenSaverEngine_legacyScreenSaver_Pid=%d", ScreenSaverEngine_Pid);

    for (int i=0; i<argc; i++) {
         print_to_log_file("gfx_switcher arg %d: %s", i, argv[i]);
    }
#endif

    // As of MacOS 14.0, the legacyScreenSaver sandbox prevents using
    // bootstrap_look_up, so we launch a bridging utility to relay Mach
    // communications between the graphics apps and the legacyScreenSaver.
    if (strcmp(argv[1], "-run_bridge") == 0) {
        char *args[4];
        strlcpy(gfx_app_path, argv[2], sizeof(gfx_app_path));
        char *slash = strrchr(gfx_app_path, '/');
        if (slash) *slash = '\0';       // Directory containing this executable
        strlcat(gfx_app_path, "/gfx_ss_bridge", sizeof(gfx_app_path));

        args[0] = gfx_app_path;
        args[1] = 0;
        execv(gfx_app_path, args);
        fprintf(stderr, "Process creation (%s) failed: errno=%d\n", gfx_app_path, errno);
        return 0;
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

    snprintf(shmem_name, sizeof(shmem_name), "/tmp/boinc_ss_%s", screensaverLoginUser);
    retval = attach_shmem_mmap(shmem_name, (void**)&ss_shmem);

    if (strcmp(argv[1], "-default_gfx") == 0) {
        strlcpy(resolved_path, "/Library/Application Support/BOINC Data/boincscr", sizeof(resolved_path));
        argv[2] = resolved_path;

#if VERBOSE           // For debugging only
        print_to_log_file("gfx_switcher using fork()");
#endif
        int pid = fork();
        // As of MacOS 13.0 Ventura IOSurface cannot be used to share graphics
        // between apps unless they are running as the same user, so we no
        // longer run the graphics apps as user boinc_master. To replace the
        // security that was provided by running as a different user, we use
        // sandbox-exec() to launch the graphics apps. Note that sandbox-exec()
        // is marked deprecated because it is an Apple private API so the syntax
        // of the security specifications is subject to change without notice.
        // But it is used widely in Apple's software, and the security profile
        // elements we use are very unlikely to change.
        //
        if (pid == 0) {
            char *args[128];
            pid_t thePid = 0;
            extern char **environ;

            args[0] = "sandbox-exec";
            args[1] = "-f";
            strlcpy(cmd, argv[0], sizeof(cmd));
            char *slash = strrchr(cmd, '/');
            if (slash) *slash = '\0';       // Directory containing this executable
            strlcat(cmd, "/mac_restrict_access.sb", sizeof(cmd)); // path to sandboxing profile
            args[2] = cmd;
            args[3] = resolved_path;
            int argcnt = 4;
            for (int i=3; i<argc; i++) {
                args[argcnt++] = argv[i];
            }
            args[argcnt] = 0;
#if VERBOSE           // For debugging only
            for (int i=0; i<argcnt; i++) {
                print_to_log_file("gfx_switcher calling posix_spawnp with args[%d]: %s", i, args[i]);
            }
#endif
            retval = posix_spawnp(&thePid, "sandbox-exec", NULL, NULL, args, environ);
#if VERBOSE           // For debugging only
            print_to_log_file("gfx_switcher posix_spawnp returned thePid = %d\n", thePid);
#endif
            if (ss_shmem != 0) {
#if VERBOSE
                print_to_log_file("gfx_switcher setting ss_shmem->gfx_pid = %d", thePid);
#endif
                ss_shmem->gfx_pid = thePid;
                ss_shmem->gfx_slot = -1;    // Default GFX has no slot number
            }
            waitpid(thePid, 0, 0);
            if (ss_shmem != 0) {
#if VERBOSE
                print_to_log_file("gfx_switcher changing ss_shmem->gfx_pid from %d to 0", thePid);
#endif
                ss_shmem->gfx_pid = 0;
                ss_shmem->major_version = 0;
                ss_shmem->minor_version = 0;
                ss_shmem->release = 0;
            }
           return 0;

    } else {
#if VERBOSE           // For debugging only
            print_to_log_file("gfx_switcher: Child PID=%d", pid);
#endif
            pthread_create(&monitorScreenSaverEngineThread, NULL, MonitorScreenSaverEngine, NULL);
            waitpid(pid, 0, 0);
            pthread_cancel(monitorScreenSaverEngineThread);
            return 0;
        }
    }

    if (strcmp(argv[1], "-launch_gfx") == 0) {
        strlcpy(gfx_app_path, BOINCDatSlotsPath, sizeof(gfx_app_path));
        strlcat(gfx_app_path, argv[2], sizeof(gfx_app_path));
        strlcat(gfx_app_path, "/", sizeof(gfx_app_path));
        strlcat(gfx_app_path, GRAPHICS_APP_FILENAME, sizeof(gfx_app_path));
        retval = resolve_soft_link(gfx_app_path, resolved_path, sizeof(resolved_path));
        if (retval) return retval;

#if VERBOSE           // For debugging only
        print_to_log_file("gfx_switcher using fork()");;
#endif
        int pid = fork();
        // As of MacOS 13.0 Ventura IOSurface cannot be used to share graphics
        // between apps unless they are running as the same user, so we no
        // longer run the graphics apps as user boinc_master. To replace the
        // security that was provided by running as a different user, we use
        // sandbox-exec() to launch the graphics apps. Note that sandbox-exec()
        // is marked deprecated because it is an Apple private API so the syntax
        // of the security specifications is subject to change without notice.
        // But it is used widely in Apple's software, and the security profile
        // elements we use are very unlikely to change.
        if (pid == 0) {
            char *args[128];
            pid_t thePid = 0;
            extern char **environ;

            args[0] = "sandbox-exec";
            args[1] = "-f";
            strlcpy(cmd, argv[0], sizeof(cmd));
            char *slash = strrchr(cmd, '/');
            if (slash) *slash = '\0';       // Directory containing this executable
            strlcat(cmd, "/mac_restrict_access.sb", sizeof(cmd)); // path to sandboxing profile
            args[2] = cmd;
            args[3] = resolved_path;
            int argcnt = 4;
            for (int i=3; i<argc; i++) {
                args[argcnt++] = argv[i];
            }
            args[argcnt] = 0;
#if VERBOSE
            for (int i=0; i<argcnt; i++) {
                print_to_log_file("gfx_switcher calling posix_spawnp with args[%d]: %s", i, args[i]);
            }
#endif
            retval = posix_spawnp(&thePid, "sandbox-exec", NULL, NULL, args, environ);
#if VERBOSE
            print_to_log_file("gfx_switcher posix_spawnp returned thePid = %d\n", thePid);
#endif
            if (ss_shmem != 0) {
#if VERBOSE
                print_to_log_file("gfx_switcher setting ss_shmem->gfx_pid = %d", thePid);
#endif
                ss_shmem->gfx_pid = thePid;
                ss_shmem->gfx_slot = atoi(argv[2]);
            }
            waitpid(thePid, 0, 0);
            if (ss_shmem != 0) {
#if VERBOSE
                print_to_log_file("gfx_switcher changing ss_shmem->gfx_pid from %d to 0", thePid);
#endif
                ss_shmem->gfx_pid = 0;
                ss_shmem->major_version = 0;
                ss_shmem->minor_version = 0;
                ss_shmem->release = 0;
            }
            return 0;
       } else {
            pthread_create(&monitorScreenSaverEngineThread, NULL, MonitorScreenSaverEngine, NULL);
            waitpid(pid, 0, 0);
            pthread_cancel(monitorScreenSaverEngineThread);
            return 0;
        }
    }

    return EINVAL;  // Unknown command
}

// For extra safety, kill our graphics app if ScreenSaverEngine has exited
void * MonitorScreenSaverEngine(void* /*param*/) {
    pid_t ScreenSaverEngine_Pid = 0;

    while (true) {
        boinc_sleep(1.0);  // Test every second
        ScreenSaverEngine_Pid = getPidIfRunning("com.apple.ScreenSaver.Engine");
#if VERBOSE           // For debugging only
        print_to_log_file("gfx_switcher MonitorScreenSaverEngine: ScreenSaverEngine_Pid=%d", ScreenSaverEngine_Pid);
#endif
        if (ScreenSaverEngine_Pid == 0) {
            // legacyScreenSaver name under MacOS 14 Sonoma
            ScreenSaverEngine_Pid = getPidIfRunning("com.apple.ScreenSaver.Engine.legacyScreenSaver");
        }
        if (ScreenSaverEngine_Pid == 0) {
#ifdef __x86_64__
            ScreenSaverEngine_Pid = getPidIfRunning("com.apple.ScreenSaver.Engine.legacyScreenSaver.x86_64");
#elif defined(__arm64__)
            ScreenSaverEngine_Pid = getPidIfRunning("com.apple.ScreenSaver.Engine.legacyScreenSaver.arm64");
#endif
        }

#if VERBOSE           // For debugging only
        print_to_log_file("gfx_switcher MonitorScreenSaverEngine: ScreenSaverEngine_legacyScreenSaver_Pid=%d", ScreenSaverEngine_Pid);
#endif

        if (ScreenSaverEngine_Pid == 0) {
            if (ss_shmem != 0) {
                if (ss_shmem->gfx_pid) {
#if VERBOSE
                    print_to_log_file("gfx_switcher MonitorScreenSaverEngine killing ss_shmem->gfx_pid = %d", ss_shmem->gfx_pid);
#endif
                    kill(ss_shmem->gfx_pid, SIGKILL);
                }
            }
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

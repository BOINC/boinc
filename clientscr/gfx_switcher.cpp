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

// gfx_switcher.C
//
// Used by screensaver to:
//  - launch graphics application at given slot number as user & owner boinc_project
//  - launch default graphics application as user & owner boinc_project
//  - kill graphics application with given process ID as user & owner boinc_project
//
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

#include "boinc_api.h"
#include "common_defs.h"

#define CREATE_LOG 0

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

int main(int argc, char** argv) {
    passwd      *pw;
    group       *grp;
    char        user_name[256], group_name[256];
    char	gfx_app_path[MAXPATHLEN], resolved_path[MAXPATHLEN];
    char        *BOINCDatSlotsPath = "/Library/Application Support/BOINC Data/slots/";
    int         retval;
    int         pid;

    if (argc < 2) return EINVAL;

    strlcpy(user_name, "boinc_project", sizeof(user_name));
    strlcpy(group_name, user_name, sizeof(group_name));

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
    pw = getpwnam(user_name);
    if (pw) setuid(pw->pw_uid);

    // NOTE: call print_to_log_file only after switching user and group
#if 0           // For debugging only
    char	current_dir[MAXPATHLEN];

    getcwd( current_dir, sizeof(current_dir));
    print_to_log_file( "current directory = %s", current_dir);
    
    for (int i=0; i<argc; i++) {
         print_to_log_file("switcher arg %d: %s", i, argv[i]);
    }
#endif

    if (strcmp(argv[1], "-default_gfx") == 0) {
        strlcpy(resolved_path, "/Library/Application Support/BOINC Data/boincscr", sizeof(resolved_path));
        argv[2] = resolved_path;
        
#if 0           // For debugging only
    for (int i=2; i<argc; i++) {
         print_to_log_file("calling execv with arg %d: %s", i-2, argv[i]);
    }
#endif

        // For unknown reasons, the graphics application exits with 
        // "RegisterProcess failed (error = -50)" unless we pass its 
        // full path twice in the argument list to execv.
        execv(resolved_path, argv+2);
        // If we got here execv failed
        fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
        return errno;
    }
    
    if (strcmp(argv[1], "-launch_gfx") == 0) {
        strlcpy(gfx_app_path, BOINCDatSlotsPath, sizeof(gfx_app_path));
        strlcat(gfx_app_path, argv[2], sizeof(gfx_app_path));
        strlcat(gfx_app_path, "/", sizeof(gfx_app_path));
        strlcat(gfx_app_path, GRAPHICS_APP_FILENAME, sizeof(gfx_app_path));
        retval = boinc_resolve_filename(gfx_app_path, resolved_path, sizeof(resolved_path));
        if (retval) return retval;
        
        argv[2] = resolved_path;
        
#if 0           // For debugging only
    for (int i=2; i<argc; i++) {
         print_to_log_file("calling execv with arg %d: %s", i-2, argv[i]);
    }
#endif

        // For unknown reasons, the graphics application exits with 
        // "RegisterProcess failed (error = -50)" unless we pass its 
        // full path twice in the argument list to execv.
        execv(resolved_path, argv+2);
        // If we got here execv failed
        fprintf(stderr, "Process creation (%s) failed: errno=%d\n", resolved_path, errno);
        return errno;
    }

    if (strcmp(argv[1], "-kill_gfx") == 0) {
        pid = atoi(argv[2]);
        if (! pid) return EINVAL;
        if ( kill(pid, SIGKILL)) {
#if 0           // For debugging only
     print_to_log_file("kill(%d, SIGKILL) returned error %d", pid, errno);
#endif
            return errno;
        }
        return 0;
    }
    
    return EINVAL;  // Unknown command
}


#if CREATE_LOG
static void print_to_log_file(const char *format, ...) {
    FILE *f;
    va_list args;
    char buf[256];
    time_t t;
    
    f = fopen("/Users/Shared/test_log.txt", "a");
    if (!f) return;

//  freopen(buf, "a", stdout);
//  freopen(buf, "a", stderr);

    time(&t);
    strcpy(buf, asctime(localtime(&t)));
    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    
    fputs("\n", f);
    fflush(f);
    fclose(f);
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

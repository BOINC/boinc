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

// setprojectgrp.C
//
// When run as
// setprojectgrp Path
// sets file at Path to user boinc_master and group boinc_project
//
// setprojectgrp runs setuid boinc_master and setgid boinc_project

#include <unistd.h>
#include <grp.h>
#include <pwd.h>	// getpwuid
#include <cstdio>
#include <cerrno>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#define VERBOSE 0

#if VERBOSE
#include <cstring>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif
static void print_to_log_file(const char *format, ...);
#ifdef __cplusplus
}
static void strip_cr(char *buf);
#endif
#endif

int main(int argc, char** argv) {
    passwd      *pw;
    group       *grp;
    uid_t       master_uid = 0;
    gid_t       project_gid = 0;
    int         retval = 0;
    struct stat sbuf;

    pw = getpwnam("boinc_master");
    if (pw) master_uid = pw->pw_uid;

    grp = getgrnam("boinc_project");
    if (grp) project_gid = grp->gr_gid;

#if VERBOSE
    print_to_log_file("setprojectgrp: current uid=%d, current euid=%d, current gid=%d, current egid=%d\n", getuid(), geteuid(), getgid(), getegid());
    print_to_log_file("setprojectgrp: argc=%d, arg[1]= %s, boinc_project gid = %d\n", argc, argv[1], project_gid);
#endif

    // chown() doesn't change ownership of symbolic links; it follows the link and
    // changes the file is not available in OS 10.3.9.
    //
    // But we don't really need to worry about this, because the system ignores
    // ownership & permissions of symbolic links anyway.
    //
    // Also, the target of a symbolic link may not be present if the slot containing
    // the link is no longer in use.
    //
    if (lstat(argv[1], &sbuf) == 0) {
        if (!S_ISLNK(sbuf.st_mode)) {
            retval = chown(argv[1], master_uid, project_gid);
//            retval = chown(argv[1], -1, project_gid);
#if VERBOSE
            if (retval)
                print_to_log_file("setprojectgrp: chown(%s, %d, %d) failed: errno=%d %s\n", argv[1], master_uid, project_gid, errno, strerror(errno));
        else
                print_to_log_file("setprojectgrp: chown(%s, %d, %d) succeeded: errno=%d\n", argv[1], master_uid, project_gid, errno);
#endif
        }
    }
    return retval;
}


#if VERBOSE

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
    strlcpy(buf, asctime(localtime(&t)), sizeof(buf));
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
#endif


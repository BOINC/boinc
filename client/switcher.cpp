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

// switcher.C
//
// When run as
// switcher Full-Path Executable-Name X1 ... Xn
// runs program at Full-Path with args X1. ... Xn
// note that the executable name nust be specified twice: 
//  once as part of the Full_Path and again as just the name

#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>  // for MAXPATHLEN
#endif
#include <pwd.h>	// getpwuid
#include <grp.h>

using std::strcpy;

int main(int argc, char** argv) {
    passwd      *pw;
    group       *grp;
    char        user_name[256], group_name[256];

    strcpy(user_name, "boinc_project");
    strcpy(group_name, "boinc_project");

#if 0           // For debugging only
    char	current_dir[MAXPATHLEN];

    getcwd( current_dir, sizeof(current_dir));
    fprintf(stderr, "current directory = %s\n", current_dir);
    
    for (int i=0; i<argc; i++) {
        fprintf(stderr, "switcher arg %d: %s\n", i, argv[i]);
    }
    fflush(stderr);
#endif

#if 0       // For debugging only
    // Allow debugging without running as user or group boinc_project
    pw = getpwuid(getuid());
    if (pw) strcpy(user_name, pw->pw_name);
    grp = getgrgid(getgid());
    if (grp) strcpy(group_name, grp->gr_gid);

#endif

    // We are running setuid root, so setgid() sets real group ID, 
    // effective group ID and saved set_group-ID for this process
    grp = getgrnam(group_name);
    if (grp) setgid(grp->gr_gid);

    // We are running setuid root, so setuid() sets real user ID, 
    // effective user ID and saved set_user-ID for this process
    pw = getpwnam(user_name);
    if (pw) setuid(pw->pw_uid);

    execv(argv[1], argv+2);
    
    // If we got here execv failed
    fprintf(stderr, "Process creation (%s) failed: errno=%d\n", argv[1], errno);

}

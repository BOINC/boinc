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

// check_security.C


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>	// getpwnam
#include <grp.h>
#include <sys/param.h>  // for MAXPATHLEN
#include "util.h"
#include "error_numbers.h"

#define boinc_master_name "boinc_master"
#define boinc_project_name "boinc_project"


// Returns FALSE (0) if owners and permissions are OK, else TRUE (1)
int check_security(int isManager) {
    passwd              *pw;
    gid_t               egid, rgid, boinc_master_gid, boinc_project_gid;
    uid_t               euid, ruid, boinc_master_uid, boinc_project_uid;
    char                *p;
    group               *grp;
    int                 i;
    char                dir_path[MAXPATHLEN];
    struct stat         sbuf;
    int                 retval;

    pw = getpwnam(boinc_master_name);
    if (pw == NULL)
        return ERR_USER_REJECTED;      // User boinc_master does not exist
    boinc_master_uid = pw->pw_uid;

    pw = getpwnam(boinc_project_name);
    if (pw == NULL)
        return ERR_USER_REJECTED;      // User boinc_project does not exist
    boinc_project_uid = pw->pw_uid;

    grp = getgrnam(boinc_master_name);
    if (grp == NULL)
        return ERR_GETGRNAM;                // Group boinc_master does not exist
    boinc_master_gid = grp->gr_gid;

    grp = getgrnam(boinc_project_name);
    if (grp == NULL)
        return ERR_GETGRNAM;                // Group boinc_project does not exist
    boinc_project_gid = grp->gr_gid;

    for (i=0; ; i++) {                      // Step through all users in group boinc_project
        p = grp->gr_mem[i];
        if (p == NULL)
            return ERR_GETGRNAM;            // User boinc_master is not a member of group boinc_project
        if (strcmp(p, boinc_master_name) == 0)
            break;
    }

    rgid = getgid();
    egid = getegid();
    if (egid != boinc_master_gid)
        return ERR_USER_PERMISSION;         // We should be running setgid boinc_master

    ruid = getuid();
    euid = geteuid();
    if (! isManager) {
        if (euid != boinc_master_uid)
            return ERR_USER_PERMISSION;     // BOINC Client should be running setuid boinc_master
    }

    getcwd(dir_path, sizeof(dir_path));
    retval = stat(dir_path, &sbuf);
    if (retval)
        return retval;
        
    // The top-level BOINC Data directory can have a different user if created by the Manager, 
    // but it should always have group boinc_master.
    if (sbuf.st_gid != boinc_master_gid)
        return ERR_USER_PERMISSION;
    
    // The top-level BOINC Data directory should have permission 775 or 575
    if ((sbuf.st_mode & 0575) != 0575)
        return ERR_USER_PERMISSION;

    getcwd(dir_path, sizeof(dir_path));
    strlcat(dir_path, "/projects", MAXPATHLEN);
    retval = stat(dir_path, &sbuf);
    if (! retval) {                 // Client can create projects directory if it does not yet exist.  
        if (sbuf.st_gid != boinc_master_gid)
            return ERR_USER_PERMISSION;

        if (sbuf.st_uid != boinc_master_uid)
            return ERR_USER_PERMISSION;

        if ((sbuf.st_mode & 0775) != 0775)
            return ERR_USER_PERMISSION;
    }

    getcwd(dir_path, sizeof(dir_path));
    strlcat(dir_path, "/slots", MAXPATHLEN);
    retval = stat(dir_path, &sbuf);
    if (! retval) {                 // Client can create slots directory if it does not yet exist.  
        if (sbuf.st_gid != boinc_master_gid)
            return ERR_USER_PERMISSION;

        if (sbuf.st_uid != boinc_master_uid)
            return ERR_USER_PERMISSION;

        if ((sbuf.st_mode & 0775) != 0775)
            return ERR_USER_PERMISSION;
    }

    return 0;
}

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
#include "file_names.h"
#include "SetupSecurity.h"


#define real_boinc_master_name "boinc_master"
#define real_boinc_project_name "boinc_project"

static char boinc_master_user_name[64];
static char boinc_master_group_name[64];
static char boinc_project_user_name[64];
static char boinc_project_group_name[64];

// Returns FALSE (0) if owners and permissions are OK, else TRUE (1)
int check_security() {
    passwd              *pw;
    group               *grp;
    gid_t               egid, boinc_master_gid;
    uid_t               euid, boinc_master_uid;
    gid_t               boinc_project_gid;
    uid_t               boinc_project_uid;
    char                dir_path[MAXPATHLEN], full_path[MAXPATHLEN];
    struct stat         sbuf;
    int                 retval;
#ifdef __WXMAC__                            // If Mac BOINC Manager
    ProcessSerialNumber ourPSN, parentPSN;
    ProcessInfoRec      pInfo;
    FSRef               ourFSRef;
    char                *p;
#endif

#ifdef _DEBUG
// GDB can't attach to applications which are running as a diferent user or group so 
//  it ignores the S_ISUID and S_ISGID permisison bits when launching an application.
// To work around this, the _DEBUG version uses the current user and group.
    boinc_master_uid = geteuid();
    pw = getpwuid(boinc_master_uid);
    if (pw == NULL)
        return ERR_USER_REJECTED;      // Should never happen
    strlcpy(boinc_master_user_name, pw->pw_name, sizeof(boinc_master_user_name));

    boinc_master_gid = getegid();
    grp = getgrgid(boinc_master_gid);
    if (grp == NULL)
        return ERR_GETGRNAM;
    strlcpy(boinc_master_group_name, grp->gr_name, sizeof(boinc_master_group_name));
    
#else   // if (! _DEBUG)
    strlcpy(boinc_master_user_name, real_boinc_master_name, sizeof(boinc_master_user_name));
    pw = getpwnam(boinc_master_user_name);
    if (pw == NULL)
        return ERR_USER_REJECTED;      // User boinc_master does not exist
    boinc_master_uid = pw->pw_uid;

    strlcpy(boinc_master_group_name, real_boinc_master_name, sizeof(boinc_master_group_name));
    grp = getgrnam(boinc_master_group_name);
    if (grp == NULL)
        return ERR_GETGRNAM;                // Group boinc_master does not exist
    boinc_master_gid = grp->gr_gid;
#endif  // ! _DEBUG

#if (defined(_DEBUG) && defined(DEBUG_WITH_FAKE_PROJECT_USER_AND_GROUP))
    // For easier debugging of project applications
    strlcpy(boinc_project_user_name, boinc_master_user_name, sizeof(boinc_project_user_name));
    strlcpy(boinc_project_group_name, boinc_master_group_name, sizeof(boinc_project_group_name));
    boinc_project_uid = boinc_master_uid;
    boinc_project_gid = boinc_master_gid;
#else
    strlcpy(boinc_project_user_name, real_boinc_project_name, sizeof(boinc_project_user_name));
    pw = getpwnam(boinc_project_user_name);
    if (pw == NULL)
        return ERR_USER_REJECTED;      // User boinc_project does not exist
    boinc_project_uid = pw->pw_uid;

    strlcpy(boinc_project_group_name, real_boinc_project_name, sizeof(boinc_project_group_name));
    grp = getgrnam(boinc_project_group_name);
    if (grp == NULL)
        return ERR_GETGRNAM;                // Group boinc_project does not exist
    boinc_project_gid = grp->gr_gid;

    for (int i=0; ; i++) {                      // Step through all users in group boinc_project
        char *p = grp->gr_mem[i];
        if (p == NULL)
            return ERR_GETGRNAM;            // User boinc_master is not a member of group boinc_project
        if (strcmp(p, boinc_master_user_name) == 0)
            break;
    }
#endif

#ifdef __WXMAC__                            // If Mac BOINC Manager
    // Get the full path to BOINC Manager application's bundle
    retval = GetCurrentProcess (&ourPSN);
    if (retval)
        return retval;          // Should never happen

    memset(&pInfo, 0, sizeof(pInfo));
    pInfo.processInfoLength = sizeof( ProcessInfoRec );
    retval = GetProcessInformation(&ourPSN, &pInfo);
    if (retval)
        return retval;          // Should never happen
    
    retval = GetProcessBundleLocation(&ourPSN, &ourFSRef);
    if (retval)
        return retval;          // Should never happen
    
    retval = FSRefMakePath (&ourFSRef, (UInt8*)dir_path, sizeof(dir_path));
    if (retval)
        return retval;          // Should never happen

    parentPSN = pInfo.processLauncher;
    memset(&pInfo, 0, sizeof(pInfo));
    pInfo.processInfoLength = sizeof( ProcessInfoRec );
    retval = GetProcessInformation(&parentPSN, &pInfo);
    if (retval)
        return retval;          // Should never happen

    // If we are running under the GDB debugger, ignore owner, 
    //  group and permissions of BOINC Manager and BOINC Client
    if (pInfo.processSignature != 'xcde') {  // Login Window app
        // Get the full path to BOINC Manager executable inside this application's bundle
        strlcpy(full_path, dir_path, sizeof(full_path));
        strlcat(full_path, "/Contents/MacOS/", sizeof(full_path));
        // To allow for branding, assume name of executable inside bundle is same as name of bundle
        p = strrchr(dir_path, '/');         // Assume name of executable inside bundle is same as name of bundle
        if (p == NULL)
            p = dir_path - 1;
        strlcat(full_path, p+1, sizeof(full_path));
        p = strrchr(full_path, '.');         // Strip off  bundle extension (".app")
        if (p)
            *p = '\0'; 

        retval = stat(full_path, &sbuf);
        if (retval)
            return retval;          // Should never happen
        
        if (sbuf.st_gid != boinc_master_gid)
            return ERR_USER_PERMISSION;

        if ((sbuf.st_mode & S_ISGID) != S_ISGID)
            return ERR_USER_PERMISSION;

        // Get the full path to BOINC Clients inside this application's bundle
        strlcpy(full_path, dir_path, sizeof(full_path));
        strlcat(full_path, "/Contents/Resources/boinc", sizeof(full_path));

        retval = stat(full_path, &sbuf);
        if (retval)
            return retval;          // Should never happen
        
        if (sbuf.st_gid != boinc_master_gid)
            return ERR_USER_PERMISSION;

        if (sbuf.st_uid != boinc_master_uid)
            return ERR_USER_PERMISSION;

        if ((sbuf.st_mode & (S_ISUID | S_ISGID)) != (S_ISUID | S_ISGID))
            return ERR_USER_PERMISSION;
    }                               // If not running under GDB debugger
#endif                                      // Mac BOINC Manager

//    rgid = getgid();
//    ruid = getuid();
    egid = getegid();
    euid = geteuid();

    if (egid != boinc_master_gid)
        return ERR_USER_PERMISSION;         // We should be running setgid boinc_master

#ifndef __WXMAC__                          // If NOT Mac BOINC Manager
    if (euid != boinc_master_uid)
        return ERR_USER_PERMISSION;     // BOINC Client should be running setuid boinc_master
#endif

    getcwd(dir_path, sizeof(dir_path));
    retval = stat(dir_path, &sbuf);
    if (retval)
        return retval;          // Should never happen
        
    // The top-level BOINC Data directory can have a different user if created by the Manager, 
    // but it should always have group boinc_master.
    if (sbuf.st_gid != boinc_master_gid)
        return ERR_USER_PERMISSION;
    
    // The top-level BOINC Data directory should have permission 775 or 575
    if ((sbuf.st_mode & 0577) != 0575)
        return ERR_USER_PERMISSION;

    strlcpy(full_path, dir_path, sizeof(full_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, PROJECTS_DIR, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (! retval) {                 // Client can create projects directory if it does not yet exist.  
        if (sbuf.st_gid != boinc_master_gid)
            return ERR_USER_PERMISSION;

        if (sbuf.st_uid != boinc_master_uid)
            return ERR_USER_PERMISSION;

        if ((sbuf.st_mode & 0777) != 0775)
            return ERR_USER_PERMISSION;
    }

    strlcpy(full_path, dir_path, sizeof(dir_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, SLOTS_DIR, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (! retval) {                 // Client can create slots directory if it does not yet exist.  
        if (sbuf.st_gid != boinc_master_gid)
            return ERR_USER_PERMISSION;

        if (sbuf.st_uid != boinc_master_uid)
            return ERR_USER_PERMISSION;

        if ((sbuf.st_mode & 0777) != 0775)
            return ERR_USER_PERMISSION;
    }

    strlcpy(full_path, dir_path, sizeof(full_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, GUI_RPC_PASSWD_FILE, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (! retval) {                 // Client can create RPC password file if it does not yet exist.  
        if (sbuf.st_gid != boinc_master_gid)
            return ERR_USER_PERMISSION;

        if (sbuf.st_uid != boinc_master_uid)
            return ERR_USER_PERMISSION;

        if ((sbuf.st_mode & 0777) != 0660)
            return ERR_USER_PERMISSION;
    }

    strlcpy(full_path, dir_path, sizeof(dir_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, SWITCHER_DIR, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (retval)
        return ERR_FILE_MISSING;
      
    if (sbuf.st_gid != boinc_master_gid)
        return ERR_USER_PERMISSION;

    if (sbuf.st_uid != boinc_master_uid)
        return ERR_USER_PERMISSION;

    if ((sbuf.st_mode & 0777) != 0770)
        return ERR_USER_PERMISSION;

    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, SWITCHER_FILE_NAME, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (retval)
        return ERR_FILE_MISSING;
    
    if (sbuf.st_gid != boinc_project_gid)
        return ERR_USER_PERMISSION;

    if (sbuf.st_uid != boinc_project_uid)
        return ERR_USER_PERMISSION;

    if ((sbuf.st_mode & 07777) != 06550)
        return ERR_USER_PERMISSION;

    return 0;
}

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


#define REAL_BOINC_MASTER_NAME "boinc_master"
#define REAL_BOINC_PROJECT_NAME "boinc_project"

static char boinc_master_user_name[64];
static char boinc_master_group_name[64];
static char boinc_project_user_name[64];
static char boinc_project_group_name[64];

// Called from BOINC Manager, BOINC Client and Installer.  
// The arguments are use only when called from the Installer

// Returns FALSE (0) if owners and permissions are OK, else TRUE (1)
int check_security(
#ifdef _MAC_INSTALLER
char *bundlePath, char *dataPath
#endif
) {
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
    ProcessSerialNumber ourPSN;
    ProcessInfoRec      pInfo;
    FSRef               ourFSRef;
    char                *p;
#endif
#ifdef _MAC_INSTALLER
    char                *p;
#else 
#endif

// GDB can't attach to applications which are running as a diferent user or group so 
//  it ignores the S_ISUID and S_ISGID permisison bits when launching an application.
// To work around this, and to allow testing the uninstalled Deployment version, we 
//  assume that the BOINC Client has the correct user and group.  
// We must get the BOINC Client's user and group differently depending on whether we 
//  were called from the Manager or from the Client
        
#ifdef __WXMAC__                            // If Mac BOINC Manager
    // Get the full path to BOINC Manager application's bundle
    retval = GetCurrentProcess (&ourPSN);
    if (retval)
        return -1000;          // Should never happen

    memset(&pInfo, 0, sizeof(pInfo));
    pInfo.processInfoLength = sizeof( ProcessInfoRec );
    retval = GetProcessInformation(&ourPSN, &pInfo);
    if (retval)
        return -1001;          // Should never happen
    
    retval = GetProcessBundleLocation(&ourPSN, &ourFSRef);
    if (retval)
        return -1002;          // Should never happen
    
    retval = FSRefMakePath (&ourFSRef, (UInt8*)dir_path, sizeof(dir_path));
    if (retval)
        return -1003;          // Should never happen
#endif

#ifdef _MAC_INSTALLER
    strlcpy(dir_path, bundlePath, sizeof(dir_path));
#endif

#if (defined(__WXMAC__) || defined(_MAC_INSTALLER)) // If Mac BOINC Manager or installer
    // Get the full path to BOINC Clients inside this application's bundle
    strlcpy(full_path, dir_path, sizeof(full_path));
    strlcat(full_path, "/Contents/Resources/boinc", sizeof(full_path));

    retval = stat(full_path, &sbuf);
    if (retval)
        return -1004;          // Should never happen

    if ((sbuf.st_mode & (S_ISUID | S_ISGID)) != (S_ISUID | S_ISGID))
        return -1005;

    boinc_master_uid = sbuf.st_gid;
    boinc_master_gid = sbuf.st_uid;
#else
    boinc_master_uid = geteuid();
    boinc_master_gid = getegid();

#endif

#ifdef _MAC_INSTALLER
   // Require absolute owner and group boinc_master:boinc_master
    strlcpy(boinc_master_user_name, REAL_BOINC_MASTER_NAME, sizeof(boinc_master_user_name));
    pw = getpwnam(boinc_master_user_name);
    if (pw == NULL)
        return -1006;      // User boinc_master does not exist
    boinc_master_uid = pw->pw_uid;

    strlcpy(boinc_master_group_name, REAL_BOINC_MASTER_NAME, sizeof(boinc_master_group_name));
    grp = getgrnam(boinc_master_group_name);
    if (grp == NULL)
        return -1007;                // Group boinc_master does not exist
    boinc_master_gid = grp->gr_gid;

#else   // Use current user and group (see comment above)

    pw = getpwuid(boinc_master_uid);
    if (pw == NULL)
        return -1008;      // Should never happen
    strlcpy(boinc_master_user_name, pw->pw_name, sizeof(boinc_master_user_name));

    grp = getgrgid(boinc_master_gid);
    if (grp == NULL)
        return -1009;
    strlcpy(boinc_master_group_name, grp->gr_name, sizeof(boinc_master_group_name));
    
#endif

#if (defined(_DEBUG) && defined(DEBUG_WITH_FAKE_PROJECT_USER_AND_GROUP))
    // For easier debugging of project applications
    strlcpy(boinc_project_user_name, boinc_master_user_name, sizeof(boinc_project_user_name));
    strlcpy(boinc_project_group_name, boinc_master_group_name, sizeof(boinc_project_group_name));
    boinc_project_uid = boinc_master_uid;
    boinc_project_gid = boinc_master_gid;
#else
    strlcpy(boinc_project_user_name, REAL_BOINC_PROJECT_NAME, sizeof(boinc_project_user_name));
    pw = getpwnam(boinc_project_user_name);
    if (pw == NULL)
        return -1010;      // User boinc_project does not exist
    boinc_project_uid = pw->pw_uid;

    strlcpy(boinc_project_group_name, REAL_BOINC_PROJECT_NAME, sizeof(boinc_project_group_name));
    grp = getgrnam(boinc_project_group_name);
    if (grp == NULL)
        return -1011;                // Group boinc_project does not exist
    boinc_project_gid = grp->gr_gid;

    for (int i=0; ; i++) {                      // Step through all users in group boinc_project
        char *p = grp->gr_mem[i];
        if (p == NULL)
            return -1012;            // User boinc_master is not a member of group boinc_project
        if (strcmp(p, boinc_master_user_name) == 0)
            break;
    }
#endif

#if (defined(__WXMAC__) || defined(_MAC_INSTALLER)) // If Mac BOINC Manager or installer
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
            return -1013;          // Should never happen
        
        if (sbuf.st_gid != boinc_master_gid)
            return -1014;

        if ((sbuf.st_mode & S_ISGID) != S_ISGID)
            return -1015;
#endif

#ifdef _MAC_INSTALLER
        // Require absolute owner and group boinc_master:boinc_master
        // Get the full path to BOINC Clients inside this application's bundle
        strlcpy(full_path, dir_path, sizeof(full_path));
        strlcat(full_path, "/Contents/Resources/boinc", sizeof(full_path));

        retval = stat(full_path, &sbuf);
        if (retval)
            return -1016;          // Should never happen
        
        if (sbuf.st_gid != boinc_master_gid)
            return -1017;

        if (sbuf.st_uid != boinc_master_uid)
            return -1018;
#endif

//    rgid = getgid();
//    ruid = getuid();
    egid = getegid();
    euid = geteuid();

#ifndef _MAC_INSTALLER
    if (egid != boinc_master_gid)
        return -1019;     // Client or Manager should be running setgid boinc_master

#ifndef __WXMAC__               // If BOINC Client
    if (euid != boinc_master_uid)
        return -1020;     // BOINC Client should be running setuid boinc_master
#endif

    getcwd(dir_path, sizeof(dir_path));             // Client or Manager
#else       // _MAC_INSTALLER
    strlcpy(dir_path, dataPath, sizeof(dir_path));  // Installer
#endif       // _MAC_INSTALLER

    retval = stat(dir_path, &sbuf);
    if (retval)
        return -1021;          // Should never happen
        
    // The top-level BOINC Data directory can have a different user if created by the Manager, 
    // but it should always have group boinc_master.
    if (sbuf.st_gid != boinc_master_gid)
        return -1022;
    
    // The top-level BOINC Data directory should have permission 775 or 575
    if ((sbuf.st_mode & 0577) != 0575)
        return -1023;

    strlcpy(full_path, dir_path, sizeof(full_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, PROJECTS_DIR, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (! retval) {                 // Client can create projects directory if it does not yet exist.  
        if (sbuf.st_gid != boinc_master_gid)
            return -1024;

        if (sbuf.st_uid != boinc_master_uid)
            return -1025;

        if ((sbuf.st_mode & 0777) != 0775)
            return -1026;
    }

    strlcpy(full_path, dir_path, sizeof(dir_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, SLOTS_DIR, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (! retval) {                 // Client can create slots directory if it does not yet exist.  
        if (sbuf.st_gid != boinc_master_gid)
            return -1027;

        if (sbuf.st_uid != boinc_master_uid)
            return -1028;

        if ((sbuf.st_mode & 0777) != 0775)
            return -1029;
    }

    strlcpy(full_path, dir_path, sizeof(full_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, GUI_RPC_PASSWD_FILE, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (! retval) {                 // Client can create RPC password file if it does not yet exist.  
        if (sbuf.st_gid != boinc_master_gid)
            return -1030;

        if (sbuf.st_uid != boinc_master_uid)
            return -1031;

        if ((sbuf.st_mode & 0777) != 0660)
            return -1032;
    }

    strlcpy(full_path, dir_path, sizeof(dir_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, SWITCHER_DIR, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (retval)
        return -1033;
      
    if (sbuf.st_gid != boinc_master_gid)
        return -1034;

    if (sbuf.st_uid != boinc_master_uid)
        return -1035;

    if ((sbuf.st_mode & 0777) != 0770)
        return -1036;

    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, SWITCHER_FILE_NAME, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (retval)
        return -1037;
    
    if (sbuf.st_gid != boinc_project_gid)
        return -1038;

    if (sbuf.st_uid != boinc_project_uid)
        return -1039;

    if ((sbuf.st_mode & 07777) != 06550)
        return -1040;

    return 0;
}

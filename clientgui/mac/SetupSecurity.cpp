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

// SetupSecurity.cpp

#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

#include <grp.h>	// getgrname, getgrgid
#include <pwd.h>	// getpwname, getpwuid, getuid
#include <unistd.h>     // usleep
#include <sys/param.h>  // for MAXPATHLEN

#include <Carbon/Carbon.h>

#include "file_names.h"
#include "SetupSecurity.h"

static OSStatus GetAuthorization(void);
OSStatus DoPrivilegedExec(const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5);
static pascal Boolean ErrorDlgFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *theItemHit);
static void SleepTicks(UInt32 ticksToSleep);
#ifdef _DEBUG
static OSStatus SetFakeMasterNames(void);
#endif
static OSStatus CreateUserAndGroup(char * user_name, char * group_name);

static AuthorizationRef        gOurAuthRef = NULL;

#define DELAY_TICKS 3
#define DELAY_TICKS_R 10


#define REAL_BOINC_MASTER_NAME "boinc_master"
#define REAL_BOINC_PROJECT_NAME "boinc_project"

#ifdef _DEBUG
// GDB can't attach to applications which are running as a diferent user or group so 
//  it ignores the S_ISUID and S_ISGID permisison bits when launching an application.
// To work around this, the _DEBUG version uses the current user and group.
//
// NOTE: The Manager and Client call these routines only "#ifdef _DEBUG" (i.e., 
// only from the DEVELOPMENT BUILD), never from the Deployment build.
//
static char boinc_master_user_name[64];
static char boinc_master_group_name[64];
static char boinc_project_user_name[64];
static char boinc_project_group_name[64];
#else
#define boinc_master_user_name REAL_BOINC_MASTER_NAME
#define boinc_master_group_name REAL_BOINC_MASTER_NAME
#define boinc_project_user_name REAL_BOINC_PROJECT_NAME
#define boinc_project_group_name REAL_BOINC_PROJECT_NAME
#endif

#define MIN_ID 25   /* Minimum user ID / Group ID to create */

static char                    dsclPath[] = "/usr/bin/dscl";
static char                    chmodPath[] = "/bin/chmod";
static char                    chownPath[] = "/usr/sbin/chown";
#define RIGHTS_COUNT 3          /* Count of the 3 above items */

int CreateBOINCUsersAndGroups() {
    char            buf1[80];
    OSStatus        err = noErr;

    err = CreateUserAndGroup(REAL_BOINC_MASTER_NAME, REAL_BOINC_MASTER_NAME);
    if (err != noErr)
        return err;
        
    SleepTicks(120);

    err = CreateUserAndGroup(REAL_BOINC_PROJECT_NAME, REAL_BOINC_PROJECT_NAME);
    if (err != noErr)
        return err;
    
    // Add user boinc_master to group boinc_project
    sprintf(buf1, "/groups/%s", boinc_project_group_name);
    // "dscl . -merge /groups/boinc_project users boinc_master"
    err = DoPrivilegedExec(dsclPath, ".", "-merge", buf1, "users", boinc_master_user_name);
    if (err != noErr)
        return err;

    system("lookupd -flushcache");
    system("memberd -r");
    
    return noErr;
}


// Pass NULL for path when calling this routine from within BOINC Manager
int SetBOINCAppOwnersGroupsAndPermissions(char *path) {
    char                    fullpath[MAXPATHLEN];
    char                    dir_path[MAXPATHLEN];
    char                    buf1[80];
    ProcessSerialNumber     ourPSN;
    FSRef                   ourFSRef;
    char                    *p;
    OSStatus                err = noErr;
    
#ifdef _DEBUG
    err = SetFakeMasterNames();
    if (err)
        return err;
#endif

    if (path == NULL) {        // NULL means we were called from within BOINC Manager
        // Get the full path to this application's bundle (BOINC Manager's bundle)
        err = GetCurrentProcess (&ourPSN);
        if (err)
            return err;          // Should never happen

        err = GetProcessBundleLocation(&ourPSN, &ourFSRef);
        if (err)
            return err;          // Should never happen
        
        err = FSRefMakePath (&ourFSRef, (UInt8*)dir_path, sizeof(dir_path));
        if (err)
            return err;          // Should never happen
    } else
        strlcpy(dir_path, path, MAXPATHLEN);    // Path to BOINC Manager's bundle was passed as argument
        
    if (strlen(fullpath) >= (MAXPATHLEN-1)) {
        ShowSecurityError("SetBOINCAppOwnersGroupsAndPermissions: path to Manager is too long");
        return -1;
    }

    strlcpy(fullpath, dir_path, sizeof(fullpath));

    // chmod -R u=rwsx,g=rwsx,o=rx path/BOINCManager.app
    // 0775 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
    //  read, write and execute permission for user & group;  read and execute permission for others
    err = DoPrivilegedExec(chmodPath, "-R", "u=rwx,g=rwx,o=rx", fullpath, NULL, NULL);
    if (err)
        return err;

    // Get the full path to BOINC Manager executable inside this application's bundle
    strlcat(fullpath, "/Contents/MacOS/", sizeof(fullpath));
    // To allow for branding, assume name of executable inside bundle is same as name of bundle
    p = strrchr(dir_path, '/');         // Assume name of executable inside bundle is same as name of bundle
    if (p == NULL)
        p = dir_path - 1;
    strlcat(fullpath, p+1, sizeof(fullpath));
    p = strrchr(fullpath, '.');         // Strip off  bundle extension (".app")
    if (p)
        *p = '\0'; 

    sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
    // chown boinc_master:boinc_master path/BOINCManager.app/Contents/MacOS/BOINCManager
    err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
    if (err)
        return err;

#ifdef _DEBUG
        // chmod u=rwx,g=rwsx,o=rx path/BOINCManager.app/Contents/MacOS/BOINCManager
        // 02775 = S_ISGID | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  setgid-on-execution plus read, write and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwsx,o=rx", fullpath, NULL, NULL, NULL);
#else
        // chmod u=rx,g=rsx,o=rx path/BOINCManager.app/Contents/MacOS/BOINCManager
        // 02555 = S_ISGID | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  setgid-on-execution plus read and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rx,g=rsx,o=rx", fullpath, NULL, NULL, NULL);
#endif
    if (err)
        return err;

    // Get the full path to BOINC Clients inside this application's bundle
    strlcpy(fullpath, dir_path, sizeof(fullpath));
    strlcat(fullpath, "/Contents/Resources/boinc", sizeof(fullpath));
    if (strlen(fullpath) >= (MAXPATHLEN-1)) {
        ShowSecurityError("SetBOINCAppOwnersGroupsAndPermissions: path to client is too long");
        return -1;
    }
    
    sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
    // chown boinc_master:boinc_master path/BOINCManager.app/Contents/Resources/boinc
    err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
    if (err)
        return err;

#ifdef _DEBUG
        // chmod u=rwsx,g=rwsx,o=rx path/BOINCManager.app/Contents/Resources/boinc
        // 06775 = S_ISUID | S_ISGID | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  setuid-on-execution, setgid-on-execution plus read, write and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rwsx,g=rwsx,o=rx", fullpath, NULL, NULL, NULL);
#else
        // chmod u=rsx,g=rsx,o=rx path/BOINCManager.app/Contents/Resources/boinc
        // 06555 = S_ISUID | S_ISGID | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  setuid-on-execution, setgid-on-execution plus read and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rsx,g=rsx,o=rx", fullpath, NULL, NULL, NULL);
#endif
    if (err)
        return err;

    return noErr;
}


int SetBOINCDataOwnersGroupsAndPermissions() {
    FSRef           ref;
    Boolean         isDirectory;
    char            fullpath[MAXPATHLEN];
    char            buf1[80];
    OSStatus        err = noErr;
    OSStatus        result;
    char            *BOINCDataDirPath = "/Library/Application Support/BOINC Data";

#ifdef _DEBUG
    err = SetFakeMasterNames();
    if (err)
        return err;
#endif
    
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);

    // Does BOINC Data directory exist?
    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result != noErr) || (! isDirectory))
        return err;                         // BOINC Data Directory does not exist

    // Set owner and group of BOINC Data directory's contents
    sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
    // chown boinc_master:boinc_master "/Library/Applications/BOINC Data"
    err = DoPrivilegedExec(chownPath, "-R", buf1, fullpath, NULL, NULL);
    if (err)
        return err;

    // Set permissions of BOINC Data directory's contents
    // chmod -R u+rw,g+rw,o= "/Library/Applications/BOINC Data"
    // 0660 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
    // set read and write permission for user and group, no access for others (leaves execute bits unchanged)
    err = DoPrivilegedExec(chmodPath, "-R", "u+rw,g+rw,o=", fullpath, NULL, NULL);
    if (err)
        return err;

#if 0   // Redundant if we already set BOINC Data directory to boinc_master:boinc_master
    // Set owner and group of BOINC Data directory itself
    sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
    // chown boinc_master:boinc_master "/Library/Applications/BOINC Data"
    err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
    if (err)
        return err;
#endif

    // Set permissions of BOINC Data directory itself
     // chmod -R u=rwsx,g=rwsx,o=rx "/Library/Applications/BOINC Data"
    // 0775 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
    //  read, write and execute permission for user & group;  read and execute permission for others
    err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwx,o=rx", fullpath, NULL, NULL, NULL);
    if (err)
        return err;

#if 0   // Redundant if we already set contents of BOINC Data directory to boinc_master:boinc_master 0660
    // Does gui_rpc_auth.cfg file exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/gui_rpc_auth.cfg", MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (! isDirectory)) {
        // Make gui_rpc_auth.cfg file readable and writable only by user boinc_master and group boinc_master

        // Set owner and group of gui_rpc_auth.cfg file
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
        // chown boinc_master:boinc_master "/Library/Applications/BOINC Data/gui_rpc_auth.cfg"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
        if (err)
            return err;

        // chmod u=rw,g=rw,o= "/Library/Applications/BOINC Data/gui_rpc_auth.cfg"
        // 0660 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
        //  read, write and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rw,g=rw,o=", fullpath, NULL, NULL, NULL);
        if (err)
            return err;
    }           // gui_rpc_auth.cfg
#endif

    // Does projects directory exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, PROJECTS_DIR, MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
        // Set owner and group of projects directory's contents
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_project_group_name);
        // chown boinc_master:boinc_project "/Library/Applications/BOINC Data/projects"
        err = DoPrivilegedExec(chownPath, "-R", buf1, fullpath, NULL, NULL);
        if (err)
            return err;

        // Set owner and group of projects directory itself
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
        // chown boinc_master:boinc_master "/Library/Applications/BOINC Data/projects"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
        if (err)
            return err;

        // Set permissions for projects directory itself (not its contents)
        // chmod -R u=rwsx,g=rwsx,o=rx "/Library/Applications/BOINC Data/projects"
        // 0775 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  read, write and execute permission for user & group;  read and execute permission for others
        err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwx,o=rx", fullpath, NULL, NULL, NULL);
        if (err)
            return err;
    }       // projects directory
    
    // Does slots directory exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SLOTS_DIR, MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
        // Set owner and group of slots directory's contents
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_project_group_name);
        // chown boinc_master:boinc_project "/Library/Applications/BOINC Data/slots"
        err = DoPrivilegedExec(chownPath, "-R", buf1, fullpath, NULL, NULL);
        if (err)
            return err;

        // Set owner and group of slots directory itself
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
        // chown boinc_master:boinc_master "/Library/Applications/BOINC Data/slots"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
        if (err)
            return err;

        // Set permissions for slots directory itself (not its contents)
        // chmod -R u=rwsx,g=rwsx,o=rx "/Library/Applications/BOINC Data/slots"
        // 0775 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  read, write and execute permission for user & group;  read and execute permission for others
        err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwx,o=rx", fullpath, NULL, NULL, NULL);
        if (err)
            return err;
    }       // slots directory

#if 0   // Redundant if we already set contents of BOINC Data directory to boinc_master:boinc_master 0660
    // Does locale directory exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/locale", MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
        // Set owner and group of locale directory and all its contents
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
        // chown boinc_master:boinc_master "/Library/Applications/BOINC Data/locale"
        err = DoPrivilegedExec(chownPath, "-R", buf1, fullpath, NULL, NULL);
        if (err)
            return err;

        // chmod -R u+rw,g+rw,o= "/Library/Applications/BOINC Data/locale"
        // 0660 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
        // set read and write permission for user and group, no access for others (leaves execute bits unchanged)
        err = DoPrivilegedExec(chmodPath, "-R", "u+rw,g+rw,o=", fullpath, NULL, NULL);
        if (err)
            return err;
    }       // locale directory
#endif
    
    // Does switcher directory exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SWITCHER_DIR, MAXPATHLEN);

#if 0   // Redundant if we already set contents of BOINC Data directory to boinc_master:boinc_master 0660
    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
        // Set owner and group of switcher directory
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
        // chown boinc_master:boinc_master "/Library/Applications/BOINC Data/switcher"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
        if (err)
            return err;

        // chmod u+rw,g+rw,o= "/Library/Applications/BOINC Data/switcher"
        // 0660 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
        // set read and write permission for user and group, no access for others (leaves execute bits unchanged)
        err = DoPrivilegedExec(chmodPath, "u+rw,g+rw,o=", fullpath, NULL, NULL, NULL);
        if (err)
            return err;
    }       // switcher directory
#endif

    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SWITCHER_FILE_NAME, MAXPATHLEN);
        result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (! isDirectory)) {
        // Set owner and group of switcher application
        sprintf(buf1, "%s:%s", boinc_project_user_name, boinc_project_group_name);
        // chown boinc_project:boinc_project "/Library/Applications/BOINC Data/switcher/switcher"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
        if (err)
            return err;

        // Set permissions of switcher application
        // chmod u=rsx,g=rsx,o= "/Library/Applications/BOINC Data/switcher/switcher"
        // 06550 = S_ISUID | S_ISGID | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP
        //  setuid-on-execution, setgid-on-execution plus read and execute permission for user and group, no access for others
        err = DoPrivilegedExec(chmodPath, "u=rsx,g=rsx,o=", fullpath, NULL, NULL, NULL);
        if (err)
            return err;
    }       // switcher application
    
    return noErr;
}


static OSStatus CreateUserAndGroup(char * user_name, char * group_name) {
    OSStatus        err = noErr;
    passwd          *pw = NULL;
    group           *grp = NULL;
    uid_t           userid = 0;
    gid_t           groupid = 0;
    gid_t           usergid = 0;
    Boolean         userExists = false;
    Boolean         groupExists = false;
    short           i;
    static short    start_id = MIN_ID;
    char            buf1[80];
    char            buf2[80];
    char            buf3[80];
    
    pw = getpwnam(user_name);
    if (pw) {
        userid = pw->pw_uid;
        userExists = true;
    }

    grp = getgrnam(group_name);
    if (grp) {
        groupid = grp->gr_gid;
        groupExists = true;
    }
    
    if ( userExists && groupExists )
        return noErr;       // User and group already exist

    // If only user or only group exists, try to use the same ID for the one we create
    if (userExists) {      // User exists but group does not
        usergid = pw->pw_gid;
        if (usergid) {
            grp = getgrgid(usergid);
            if (grp == NULL)    // Set the group ID = users existing group if this group ID is available
                groupid = usergid;
        }
        if (groupid == 0) {
            grp = getgrgid(userid);
            if (grp == NULL)    // Set the group ID = user ID if this group ID is available
                groupid = userid;
        }
    } else {
        if (groupExists) {      // Group exists but user does not
            pw = getpwuid(groupid);
            if (pw == NULL)    // Set the user ID = group ID if this user ID is available
                userid = groupid;
        }
    }
    
    // We need to find an available user ID, group ID, or both.  Find a value that is currently 
    // neither a user ID or a group ID.
    // If we need both a new user ID and a new group ID, finds a value that can be used for both.
    if ( (userid == 0) || (groupid == 0) ) {
        for(i=start_id; ; i++) {
           if ((uid_t)i != userid) {
                pw = getpwuid((uid_t)i);
                if (pw)
                    continue;               // Already exists as a user ID of a different user
            }
            
            if ((gid_t)i != groupid) {
                grp = getgrgid((gid_t)i);
                if (grp)
                    continue;               // Already exists as a group ID of a different group
            }
            
            if (! userExists)
                userid = (uid_t)i;
            if (! groupExists)
                groupid = (gid_t)i;

            start_id = i + 1;               // Start with next higher value next time
                
            break;                          // Success!
        }
    }
    
    sprintf(buf1, "/groups/%s", group_name);
    sprintf(buf2, "%d", groupid);

    if (! groupExists) {             // If we need to create group
        // Something like "dscl . -create /groups/boinc_master"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf1, NULL, NULL);
        if (err)
            return err;
 
        // Something like "dscl . -create /groups/boinc_master gid 33"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf1, "gid", buf2);
        if (err)
            return err;
    }           // if (! groupExists)

    sprintf(buf1, "/users/%s", user_name);
    sprintf(buf2, "%d", groupid);
    sprintf(buf3, "%d", userid);
        
    if (! userExists) {             // If we need to create user
        // Something like "dscl . -create /users/boinc_master"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf1, NULL, NULL);
        if (err)
            return err;

        // Something like "dscl . -create /users/boinc_master uid 33"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf1, "uid", buf3);
        if (err)
            return err;

        // Prevent a security hole by not allowing a login from this user
        // Something like "dscl . -create /users/boinc_master shell /usr/bin/false"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf1, "shell", "/usr/bin/false");
        if (err)
            return err;

        // Something like "dscl . -create /users/boinc_master home /var/empty"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf1, "home", "/var/empty");
        if (err)
            return err;
    }           // if (! userExists)

    // Always set the user gid if we created either the user or the group or both
    // Something like "dscl . -create /users/boinc_master gid 33"
    err = DoPrivilegedExec(dsclPath, ".", "-create", buf1, "gid", buf2);
    if (err)
        return err;

    system("lookupd -flushcache");
    system("memberd -r");

    return noErr;
}


int AddAdminUserToGroups(char *user_name) {        
#ifndef _DEBUG
    char            buf1[80];
    OSStatus        err = noErr;

    sprintf(buf1, "/groups/%s", boinc_master_group_name);

    // "dscl . -merge /groups/boinc_master users user_name"
    err = DoPrivilegedExec(dsclPath, ".", "-merge", buf1, "users", user_name);
    if (err)
        return err;

    sprintf(buf1, "/groups/%s", boinc_project_group_name);

    // "dscl . -merge /groups/boinc_project users user_name"
    err = DoPrivilegedExec(dsclPath, ".", "-merge", buf1, "users", user_name);
    if (err)
        return err;

    system("lookupd -flushcache");
    system("memberd -r");

#endif          // ! _DEBUG    
    return noErr;
}


#ifdef _DEBUG
// GDB can't attach to applications which are running as a diferent user or group so 
//  it ignores the S_ISUID and S_ISGID permisison bits when launching an application.
// To work around this, the _DEBUG version uses the current user and group.
static OSStatus SetFakeMasterNames() {
    passwd              *pw;
    group               *grp;
    gid_t               boinc_master_gid;
    uid_t               boinc_master_uid;

    boinc_master_uid = geteuid();
    pw = getpwuid(boinc_master_uid);
    if (pw == NULL)
        return -1;      // Should never happen
    strlcpy(boinc_master_user_name, pw->pw_name, sizeof(boinc_master_user_name));

    boinc_master_gid = getegid();
    grp = getgrgid(boinc_master_gid);
    if (grp == NULL)
        return -1;
    strlcpy(boinc_master_group_name, grp->gr_name, sizeof(boinc_master_group_name));
    
#ifdef DEBUG_WITH_FAKE_PROJECT_USER_AND_GROUP
    // For easier debugging of project applications
    strlcpy(boinc_project_user_name, pw->pw_name, sizeof(boinc_project_user_name));
    strlcpy(boinc_project_group_name, grp->gr_name, sizeof(boinc_project_group_name));
#else
    // For better debugging of SANDBOX permissions logic
    strlcpy(boinc_project_user_name, REAL_BOINC_PROJECT_NAME, sizeof(boinc_project_user_name));
    strlcpy(boinc_project_group_name, REAL_BOINC_PROJECT_NAME, sizeof(boinc_project_group_name));
#endif
    
    return noErr;
}
#endif


static OSStatus GetAuthorization (void) {
    static Boolean              sIsAuthorized = false;
    AuthorizationRights         ourAuthRights;
    AuthorizationFlags          ourAuthFlags;
    AuthorizationItem           ourAuthRightsItem[RIGHTS_COUNT];
    AuthorizationEnvironment    ourAuthEnvironment;
    AuthorizationItem           ourAuthEnvItem[1];
    char                        prompt[] = "BOINC needs to have certain permissions set up.\n\n";
    OSStatus                    err = noErr;

    if (sIsAuthorized)
        return noErr;
        
    ourAuthRights.count = 0;
    ourAuthRights.items = NULL;

    err = AuthorizationCreate (&ourAuthRights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &gOurAuthRef);
    if (err != noErr) {
        ShowSecurityError("AuthorizationCreate returned error %d", err);
        return err;
    }
     
    ourAuthRightsItem[0].name = kAuthorizationRightExecute;
    ourAuthRightsItem[0].value = dsclPath;
    ourAuthRightsItem[0].valueLength = strlen (dsclPath);
    ourAuthRightsItem[0].flags = 0;

    ourAuthRightsItem[1].name = kAuthorizationRightExecute;
    ourAuthRightsItem[1].value = chmodPath;
    ourAuthRightsItem[1].valueLength = strlen (chmodPath);
    ourAuthRightsItem[1].flags = 0;

    ourAuthRightsItem[2].name = kAuthorizationRightExecute;
    ourAuthRightsItem[2].value = chownPath;
    ourAuthRightsItem[2].valueLength = strlen (chownPath);
    ourAuthRightsItem[2].flags = 0;

#if AUTHORIZE_LOOKUPD_MEMBERD
    ourAuthRightsItem[3].name = kAuthorizationRightExecute;
    ourAuthRightsItem[3].value = lookupdPath;
    ourAuthRightsItem[3].valueLength = strlen (lookupdPath);
    ourAuthRightsItem[3].flags = 0;

    ourAuthRightsItem[4].name = kAuthorizationRightExecute;
    ourAuthRightsItem[4].value = memberdPath;
    ourAuthRightsItem[4].valueLength = strlen (memberdPath);
    ourAuthRightsItem[4].flags = 0;
#endif

    ourAuthRights.count = RIGHTS_COUNT;
    ourAuthRights.items = ourAuthRightsItem;

    ourAuthEnvItem[0].name = kAuthorizationEnvironmentPrompt;
    ourAuthEnvItem[0].value = prompt;
    ourAuthEnvItem[0].valueLength = strlen (prompt);
    ourAuthEnvItem[0].flags = 0;

    ourAuthEnvironment.count = 1;
    ourAuthEnvironment.items = ourAuthEnvItem;

    
    ourAuthFlags = kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights;
    
    // When this is called from the installer, the installer has already authenticated.  
    // In that case we are already running with full root privileges so AuthorizationCopyRights() 
    // does not request a password from the user again.
    err = AuthorizationCopyRights (gOurAuthRef, &ourAuthRights, &ourAuthEnvironment, ourAuthFlags, NULL);
    
    if (err == noErr)
        sIsAuthorized = true;
    
    return err;
}

OSStatus DoPrivilegedExec(const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5) {
    short               i;
    char                *args[8];
    OSStatus            err;
    FILE                *ioPipe;
    char                *p, junk[16];

    err = GetAuthorization();
    if (err != noErr) {
        if (err == errAuthorizationCanceled)
            return err;
        ShowSecurityError("GetAuthorization returned error %d", err);
    } else {
        for (i=0; i<5; i++) {       // Retry 5 times if error
            args[0] = arg1;
            args[1] = arg2;
            args[2] = arg3;
            args[3] = arg4;
            args[4] = arg5;
            args[5] = NULL;

            err = AuthorizationExecuteWithPrivileges (gOurAuthRef, pathToTool, 0, args, &ioPipe);
            // We use the pipe to signal us when the command has completed
            p = fgets(junk, sizeof(junk), ioPipe);
            fclose (ioPipe);
#if 0
            if (strcmp(arg2, "-R") == 0)
                SleepTicks(DELAY_TICKS_R);
            else
                SleepTicks(DELAY_TICKS);
#endif
            if (err == noErr)
                break;
        }
    }
    if (err != noErr)
        ShowSecurityError("\"%s %s %s %s %s %s\" returned error %d", pathToTool, 
                            arg1 ? arg1 : "", arg2 ? arg2 : "", arg3 ? arg3 : "", 
                            arg4 ? arg4 : "", arg5 ? arg5 : "", err);

       return err;
}



void ShowSecurityError(const char *format, ...) {
    va_list                 args;
    char                    s[1024];
    short                   itemHit;
    AlertStdAlertParamRec   alertParams;
    ModalFilterUPP          ErrorDlgFilterProcUPP;
    
    ProcessSerialNumber	ourProcess;

    va_start(args, format);
    s[0] = vsprintf(s+1, format, args);
    va_end(args);

    ErrorDlgFilterProcUPP = NewModalFilterUPP(ErrorDlgFilterProc);

    alertParams.movable = true;
    alertParams.helpButton = false;
    alertParams.filterProc = ErrorDlgFilterProcUPP;
    alertParams.defaultText = "\pOK";
    alertParams.cancelText = NULL;
    alertParams.otherText = NULL;
    alertParams.defaultButton = kAlertStdAlertOKButton;
    alertParams.cancelButton = 0;
    alertParams.position = kWindowDefaultPosition;

    ::GetCurrentProcess (&ourProcess);
    ::SetFrontProcess(&ourProcess);

    StandardAlert (kAlertStopAlert, (StringPtr)s, NULL, &alertParams, &itemHit);

    DisposeModalFilterUPP(ErrorDlgFilterProcUPP);
}


static pascal Boolean ErrorDlgFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *theItemHit) {
    // We need this because this is a command-line application so it does not get normal events
    if (Button()) {
        *theItemHit = kStdOkItemIndex;
        return true;
    }
    
    return StdFilterProc(theDialog, theEvent, theItemHit);
}

// Uses usleep to sleep for full duration even if a signal is received
static void SleepTicks(UInt32 ticksToSleep) {
    UInt32 endSleep, timeNow, ticksRemaining;

    timeNow = TickCount();
    ticksRemaining = ticksToSleep;
    endSleep = timeNow + ticksToSleep;
    while ( (timeNow < endSleep) && (ticksRemaining <= ticksToSleep) ) {
        usleep(16667 * ticksRemaining);
        timeNow = TickCount();
        ticksRemaining = endSleep - timeNow;
    } 
}

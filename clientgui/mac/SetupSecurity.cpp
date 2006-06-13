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

#include "SetupSecurity.h"

static OSStatus CreateUserAndGroup(char * name);
static OSStatus GetAuthorization(void);
OSStatus DoPrivilegedExec(const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5);
static pascal Boolean ErrorDlgFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *theItemHit);
static void SleepTicks(UInt32 ticksToSleep);


static AuthorizationRef        gOurAuthRef = NULL;

#define DELAY_TICKS 3
#define DELAY_TICKS_R 10

#define boinc_master_name "boinc_master"
#define boinc_project_name "boinc_project"

#define MIN_ID 25   /* Minimum user ID / Group ID to create */

static char                    dsclPath[] = "/usr/bin/dscl";
static char                    chmodPath[] = "/bin/chmod";
static char                    chownPath[] = "/usr/sbin/chown";
#define RIGHTS_COUNT 3          /* Count of the 3 above items */

OSStatus CreateBOINCUsersAndGroups() {
    char            buf1[80];
    OSStatus        err = noErr;

    err = CreateUserAndGroup(boinc_master_name);
    if (err != noErr)
        return err;
    
    err = CreateUserAndGroup(boinc_project_name);
    if (err != noErr)
        return err;
    
    // Add user boinc_master to group boinc_project
    sprintf(buf1, "/groups/%s", boinc_project_name);
    // "dscl . -merge /groups/boinc_project users boinc_master"
    err = DoPrivilegedExec(dsclPath, ".", "-merge", buf1, "users", boinc_master_name);
    if (err != noErr)
        return err;

    system("lookupd -flushcache");
    system("memberd -r");

    return noErr;
}


OSStatus SetBOINCAppOwnersGroupsAndPermissions(char *path, char *managerName, Boolean development) {
    char            fullpath[MAXPATHLEN];
    char            buf1[80];
    mode_t          old_mask;
    OSStatus        err = noErr;
    
    strlcpy(fullpath, path, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, managerName, MAXPATHLEN);
    strlcat(fullpath, ".app", MAXPATHLEN);
    if (strlen(fullpath) >= (MAXPATHLEN-1)) {
        ShowSecurityError("SetBOINCAppOwnersGroupsAndPermissions: path to Manager is too long");
        return -1;
    }

     // chmod -R u=rwsx,g=rwsx,o=rx path/BOINCManager.app
    // 0775 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
    //  read, write and execute permission for user, group & others
    err = DoPrivilegedExec(chmodPath, "-R", "u=rwx,g=rwx,o=rx", fullpath, NULL, NULL);
    if (err)
        return err;

    strlcat(fullpath, "/Contents/MacOS/", MAXPATHLEN);
    strlcat(fullpath, managerName, MAXPATHLEN);
    if (strlen(fullpath) >= (MAXPATHLEN-1)) {
        ShowSecurityError("SetBOINCAppOwnersGroupsAndPermissions: path to Manager is too long");
        return -1;
    }

    sprintf(buf1, "%s:%s", boinc_master_name, boinc_master_name);
    // chown boinc_master:boinc_master path/BOINCManager.app/Contents/MacOS/BOINCManager
    err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
    if (err)
        return err;

    if (development) {
        // chmod u=rwx,g=rwsx,o=rx path/BOINCManager.app/Contents/MacOS/BOINCManager
        // 02775 = S_ISGID | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  setgid-on-execution plus read, write and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwsx,o=rx", fullpath, NULL, NULL, NULL);
    } else {
        // chmod u=rx,g=rsx,o=rx path/BOINCManager.app/Contents/MacOS/BOINCManager
        // 02555 = S_ISGID | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  setgid-on-execution plus read and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rx,g=rsx,o=rx", fullpath, NULL, NULL, NULL);
    }
    if (err)
        return err;


    strlcpy(fullpath, path, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, managerName, MAXPATHLEN);
    strlcat(fullpath, ".app/Contents/Resources/boinc", MAXPATHLEN);
    if (strlen(fullpath) >= (MAXPATHLEN-1)) {
        ShowSecurityError("SetBOINCAppOwnersGroupsAndPermissions: path to client is too long");
        return -1;
    }
    
    sprintf(buf1, "%s:%s", boinc_master_name, boinc_master_name);
    // chown boinc_master:boinc_master path/BOINCManager.app/Contents/Resources/boinc
    err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
    if (err)
        return err;

    if (development) {
        // chmod u=rwsx,g=rwsx,o=rx path/BOINCManager.app/Contents/Resources/boinc
        // 06775 = S_ISUID | S_ISGID | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  setuid-on-execution, setgid-on-execution plus read, write and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rwsx,g=rwsx,o=rx", fullpath, NULL, NULL, NULL);
    } else {
        // chmod u=rsx,g=rsx,o=rx path/BOINCManager.app/Contents/Resources/boinc
        // 06555 = S_ISUID | S_ISGID | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
        //  setuid-on-execution, setgid-on-execution plus read and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rsx,g=rsx,o=rx", fullpath, NULL, NULL, NULL);
    }
    if (err)
        return err;

    return noErr;
}


OSStatus SetBOINCDataOwnersGroupsAndPermissions() {
    FSRef           ref;
    Boolean         isDirectory;
    char            fullpath[MAXPATHLEN];
    char            buf1[80];
    mode_t          old_mask;
    OSStatus        err = noErr;
    OSStatus        result;
    char            *BOINCDataDirPath = "/Library/Application Support/BOINC Data";
    
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);

    // Does BOINC Data directory exist?
    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result != noErr) || (! isDirectory))
        return noErr;                       // BOINC Data Directory does not exist

    // Set owner and group of BOINC Data directory's contents
    sprintf(buf1, "%s:%s", boinc_master_name, boinc_master_name);
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

    // Set owner and group of BOINC Data directory itself
    sprintf(buf1, "%s:%s", boinc_master_name, boinc_project_name);
    // chown boinc_master:boinc_project "/Library/Applications/BOINC Data"
    err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL);
    if (err)
        return err;

    // Set permissions of BOINC Data directory itself
    // chmod -R u+rwx,g=rx,o-rwx "/Library/Applications/BOINC Data"
    // 0750 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP| S_IXGRP
    // set read, write, execute permission for user and group, no access for others
    err = DoPrivilegedExec(chmodPath, "u+rwx,g=rx,o-rwx", fullpath, NULL, NULL, NULL);
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
        sprintf(buf1, "%s:%s", boinc_master_name, boinc_master_name);
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
    strlcat(fullpath, "/projects", MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
        // Set owner and group of projects directory and all its contents
        sprintf(buf1, "%s:%s", boinc_master_name, boinc_project_name);
        // chown boinc_master:boinc_project "/Library/Applications/BOINC Data/projects"
        err = DoPrivilegedExec(chownPath, "-R", buf1, fullpath, NULL, NULL);
        if (err)
            return err;

        // Set permissions for projects directory itself (not its contents)
        // chmod u=rwx,g=rx,o= "/Library/Applications/BOINC Data/projects"
        // 0750 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP| S_IXGRP
        // set read, write, execute permission for user, read and execute for group, no access for others
        err = DoPrivilegedExec(chmodPath, "u=rwx,g=rx,o=", fullpath, NULL, NULL, NULL);
        if (err)
            return err;
    }       // projects directory
    
    // Does slots directory exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/slots", MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
        // Set owner and group of slots directory and all its contents
        sprintf(buf1, "%s:%s", boinc_master_name, boinc_project_name);
        // chown boinc_master:boinc_project "/Library/Applications/BOINC Data/slots"
        err = DoPrivilegedExec(chownPath, "-R", buf1, fullpath, NULL, NULL);
        if (err)
            return err;

        // Set permissions for slots directory itself (not its contents)
        // chmod u=rwx,g=rx,o= "/Library/Applications/BOINC Data/slots"
        // 0750 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP| S_IXGRP
        // set read, write, execute permission for user, read and execute for group, no access for others
        err = DoPrivilegedExec(chmodPath, "u=rwx,g=rx,o=", fullpath, NULL, NULL, NULL);
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
        sprintf(buf1, "%s:%s", boinc_master_name, boinc_master_name);
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
    strlcat(fullpath, "/switcher", MAXPATHLEN);

#if 0   // Redundant if we already set contents of BOINC Data directory to boinc_master:boinc_master 0660
    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
        // Set owner and group of switcher directory and its contents
        sprintf(buf1, "%s:%s", boinc_master_name, boinc_master_name);
        // chown boinc_master:boinc_master "/Library/Applications/BOINC Data/switcher"
        err = DoPrivilegedExec(chownPath, "-R", buf1, fullpath, NULL, NULL);
        if (err)
            return err;

        // chmod -R u+rw,g+rw,o= "/Library/Applications/BOINC Data/switcher"
        // 0660 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
        // set read and write permission for user and group, no access for others (leaves execute bits unchanged)
        err = DoPrivilegedExec(chmodPath, "-R", "u+rw,g+rw,o=", fullpath, NULL, NULL);
        if (err)
            return err;
    }       // switcher directory
#endif

    strlcat(fullpath, "/switcher", MAXPATHLEN);
    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (! isDirectory)) {
        // Set permissions of switcher application
        // chmod -R u=rsx,g=rsx,o= "/Library/Applications/BOINC Data/switcher"
        // 06550 = S_ISUID | S_ISGID | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP
        //  setuid-on-execution, setgid-on-execution plus read and execute permission for user and group, no access for others
        err = DoPrivilegedExec(chmodPath, "-R", "u=rsx,g=rsx,o=", fullpath, NULL, NULL);
        if (err)
            return err;
    }       // switcher application
    
    return noErr;
}


static OSStatus CreateUserAndGroup(char * name) {
    OSStatus    err = noErr;
    passwd      *pw = NULL;
    group       *grp = NULL;
    uid_t       userid = 0;
    gid_t       groupid = 0;
    gid_t       usergid = 0;
    Boolean     userExists = false;
    Boolean     groupExists = false;
    short       i;
    char        buf1[80];
    char        buf2[80];
    char        buf3[80];
    
    pw = getpwnam(name);
    if (pw) {
        userid = pw->pw_uid;
        userExists = true;
    }

    grp = getgrnam(name);
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
        for(i=MIN_ID; ; i++) {
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
                
            break;                          // Success!
        }
    }

    sprintf(buf1, "/groups/%s", name);
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

    sprintf(buf1, "/users/%s", name);
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
    
    return noErr;
}


OSStatus AddAdminUserToGroups(char *user_name) {        
    char            buf1[80];
    OSStatus        err = noErr;

    sprintf(buf1, "/groups/%s", boinc_master_name);

    // "dscl . -merge /groups/boinc_master users user_name"
    err = DoPrivilegedExec(dsclPath, ".", "-merge", buf1, "users", user_name);
    if (err)
        return err;

    sprintf(buf1, "/groups/%s", boinc_project_name);

    // "dscl . -merge /groups/boinc_project users user_name"
    err = DoPrivilegedExec(dsclPath, ".", "-merge", buf1, "users", user_name);
    if (err)
        return err;

    system("lookupd -flushcache");
    system("memberd -r");

    return noErr;
}


static OSStatus GetAuthorization (void) {
    static Boolean          sIsAuthorized = false;
    AuthorizationRights     ourAuthRights;
    AuthorizationFlags      ourAuthFlags;
    AuthorizationItem       ourAuthItem[RIGHTS_COUNT];
    OSStatus                err = noErr;

    if (sIsAuthorized)
        return noErr;
        
    ourAuthRights.count = 0;
    ourAuthRights.items = NULL;

    err = AuthorizationCreate (&ourAuthRights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &gOurAuthRef);
    if (err != noErr) {
        ShowSecurityError("AuthorizationCreate returned error %d", err);
        return err;
    }
     
    ourAuthItem[0].name = kAuthorizationRightExecute;
    ourAuthItem[0].value = dsclPath;
    ourAuthItem[0].valueLength = strlen (dsclPath);
    ourAuthItem[0].flags = 0;

    ourAuthItem[1].name = kAuthorizationRightExecute;
    ourAuthItem[1].value = chmodPath;
    ourAuthItem[1].valueLength = strlen (chmodPath);
    ourAuthItem[1].flags = 0;

    ourAuthItem[2].name = kAuthorizationRightExecute;
    ourAuthItem[2].value = chownPath;
    ourAuthItem[2].valueLength = strlen (chownPath);
    ourAuthItem[2].flags = 0;

#if AUTHORIZE_LOOKUPD_MEMBERD
    ourAuthItem[3].name = kAuthorizationRightExecute;
    ourAuthItem[3].value = lookupdPath;
    ourAuthItem[3].valueLength = strlen (lookupdPath);
    ourAuthItem[3].flags = 0;

    ourAuthItem[4].name = kAuthorizationRightExecute;
    ourAuthItem[4].value = memberdPath;
    ourAuthItem[4].valueLength = strlen (memberdPath);
    ourAuthItem[4].flags = 0;
#endif

    ourAuthRights.count = RIGHTS_COUNT;
    ourAuthRights.items = ourAuthItem;
    
    ourAuthFlags = kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights;
    
    // When this is called from the installer, the installer has already authenticated.  
    // In that case we are already running with full root privileges so AuthorizationCopyRights() 
    // does not request a password from the user again.
    err = AuthorizationCopyRights (gOurAuthRef, &ourAuthRights, kAuthorizationEmptyEnvironment, ourAuthFlags, NULL);
    
    if (err == noErr)
        sIsAuthorized = true;
    
    return err;
}

OSStatus DoPrivilegedExec(const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5) {
    short               i;
    char                *args[8];
    OSStatus            err;

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

            err = AuthorizationExecuteWithPrivileges (gOurAuthRef, pathToTool, 0, args, NULL);

            if (strcmp(arg2, "-R") == 0)
                SleepTicks(DELAY_TICKS_R);
            else
                SleepTicks(DELAY_TICKS);
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
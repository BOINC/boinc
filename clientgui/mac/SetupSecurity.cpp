// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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
#include <unistd.h>

#include <Carbon/Carbon.h>

static OSStatus CreateUserAndGroup(char * name);
static OSStatus GetAuthorization(void);
static void ShowSecurityError(const char *format, ...);

static AuthorizationRef        gOurAuthRef = NULL;

static char                    dsclPath[] = "/usr/bin/dscl";
static char                    chmodPath[] = "/bin/chmod";
static char                    chownPath[] = "/usr/sbin/chown";
static char                    lookupdPath[] = "/usr/sbin/lookupd";
static char                    memberdPath[] = "/usr/sbin/memberd";


#define boinc_master_name "boinc_master"
#define boinc_project_name "boinc_project"

#define MIN_ID 25   /* Minimum user ID / Group ID to create */

int main(int argc, char *argv[]) {
    OSStatus                err = noErr;

    err = CreateUserAndGroup(boinc_master_name);
    if (err != noErr)
        return err;
        
    err = CreateUserAndGroup(boinc_project_name);
    return err;
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
    char        *args[8];
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

    err = GetAuthorization();
    if (err != noErr) {
        ShowSecurityError("GetAuthorization returned error %d", err);
        return err;
    }

    sprintf(buf1, "/groups/%s", name);
    sprintf(buf2, "%d", groupid);

    if (! groupExists) {             // If we need to create group
        // Something like "dscl . -create /groups/boinc_master"
	args[0] = ".";
	args[1] = "-create";
	args[2] = buf1;
	args[3] = NULL;
	err = AuthorizationExecuteWithPrivileges (gOurAuthRef, dsclPath, 0, args, NULL);
        if (err != noErr) {
            ShowSecurityError("\"dscl . -create %s\" returned error %d", buf1, err);
            return err;
        }

        // Something like "dscl . -create /groups/boinc_master gid 33"
	args[0] = ".";
	args[1] = "-create";    // "-append";
	args[2] = buf1;
	args[3] = "gid";
	args[4] = buf2;
	args[5] = NULL;
	err = AuthorizationExecuteWithPrivileges (gOurAuthRef, dsclPath, 0, args, NULL);
        if (err != noErr) {
            ShowSecurityError("\"dscl . -create %s gid %s\" returned error %d", buf1, buf2, err);
            return err;
        }
    }

    sprintf(buf1, "/users/%s", name);
    sprintf(buf2, "%d", groupid);
    sprintf(buf3, "%d", userid);
        
    if (! userExists) {             // If we need to create user
        // Something like "dscl . -create /users/boinc_master"
	args[0] = ".";
	args[1] = "-create";
	args[2] = buf1;
	args[3] = NULL;
	err = AuthorizationExecuteWithPrivileges (gOurAuthRef, dsclPath, 0, args, NULL);
        if (err != noErr) {
            ShowSecurityError("\"dscl . -create %s\" returned error %d", buf1, err);
            return err;
        }

        // Something like "dscl . -create /users/boinc_master uid 33"
	args[0] = ".";
	args[1] = "-create";    // "-append";
	args[2] = buf1;
	args[3] = "uid";
	args[4] = buf3;
	args[5] = NULL;
	err = AuthorizationExecuteWithPrivileges (gOurAuthRef, dsclPath, 0, args, NULL);
        if (err != noErr) {
            ShowSecurityError("\"dscl . -create %s uid %s\" returned error %d", buf1, buf3, err);
            return err;
        }
    }

    // Always set the user gid if we created either the user or the group or both
    // Something like "dscl . -create /users/boinc_master gid 33"
    args[0] = ".";
    args[1] = "-create";    // "-append";
    args[2] = buf1;
    args[3] = "gid";
    args[4] = buf2;
    args[5] = NULL;
    err = AuthorizationExecuteWithPrivileges (gOurAuthRef, dsclPath, 0, args, NULL);
    if (err != noErr) {
        ShowSecurityError("\"dscl . -create %s gid %s\" returned error %d", buf1, buf2, err);
        return err;
    }
    
#if 0
    // "lookupd -flushcache"
    args[0] = "-flushcache";
    args[1] = NULL;
    err = AuthorizationExecuteWithPrivileges (gOurAuthRef, lookupdPath, 0, args, NULL);
    if (err != noErr) {
        ShowSecurityError("\"lookupdPath -flushcache\" returned error %d", err);
        return err;
    }
#endif

    // "memberd -r"
    args[0] = "-r";
    args[1] = NULL;
    err = AuthorizationExecuteWithPrivileges (gOurAuthRef, memberdPath, 0, args, NULL);
    if (err != noErr) {
        ShowSecurityError("\"memberd -r\" returned error %d", err);
        return err;
    }
    
    return noErr;
}


static OSStatus GetAuthorization (void)
{
    static Boolean          sIsAuthorized = false;
    AuthorizationRights     ourAuthRights;
    AuthorizationFlags      ourAuthFlags;
    AuthorizationItem       ourAuthItem[5];
    OSErr                   err = noErr;

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

    ourAuthItem[3].name = kAuthorizationRightExecute;
    ourAuthItem[3].value = lookupdPath;
    ourAuthItem[3].valueLength = strlen (lookupdPath);
    ourAuthItem[3].flags = 0;

    ourAuthItem[4].name = kAuthorizationRightExecute;
    ourAuthItem[4].value = memberdPath;
    ourAuthItem[4].valueLength = strlen (memberdPath);
    ourAuthItem[4].flags = 0;

    ourAuthRights.count = 5;
    ourAuthRights.items = ourAuthItem;
    
    ourAuthFlags = kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights;
    
    err = AuthorizationCopyRights (gOurAuthRef, &ourAuthRights, kAuthorizationEmptyEnvironment, ourAuthFlags, NULL);
    
    if (err == noErr)
        sIsAuthorized = true;
    
    return err;
}


static void ShowSecurityError(const char *format, ...)
{
    va_list args;
    char s[1024];
    short itemHit;
    ProcessSerialNumber	ourProcess;

    va_start(args, format);
    s[0] = vsprintf(s+1, format, args);
    va_end(args);

    ::GetCurrentProcess (&ourProcess);
    ::SetFrontProcess(&ourProcess);

    StandardAlert (kAlertStopAlert, (StringPtr)s, NULL, NULL, &itemHit);
}
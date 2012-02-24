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

// SetupSecurity.cpp

#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

#include <grp.h>	// getgrname, getgrgid
#include <pwd.h>	// getpwnam, getpwuid, getuid
#include <unistd.h>     // usleep
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>
#include <dirent.h>

#include <Carbon/Carbon.h>

#include "file_names.h"
#include "SetupSecurity.h"

static OSStatus UpdateNestedDirectories(char * basepath);
static OSStatus MakeXMLFilesPrivate(char * basepath);
static OSStatus GetAuthorization(void);
OSStatus DoPrivilegedExec(const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5, char *arg6);
#ifndef __x86_64__
static pascal Boolean ErrorDlgFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *theItemHit);
#endif
static void SleepTicks(UInt32 ticksToSleep);
#ifdef _DEBUG
static OSStatus SetFakeMasterNames(void);
#endif
static OSStatus CreateUserAndGroup(char * user_name, char * group_name);
static OSStatus ResynchSystem(void);

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

#define MIN_ID 501   /* Minimum user ID / Group ID to create */

static char                    dsclPath[] = "/usr/bin/dscl";
static char                    chmodPath[] = "/bin/chmod";
static char                    chownPath[] = "/usr/sbin/chown";
#define RIGHTS_COUNT 3          /* Count of the 3 above items */

int CreateBOINCUsersAndGroups() {
    OSStatus        err = noErr;

    err = CreateUserAndGroup(REAL_BOINC_MASTER_NAME, REAL_BOINC_MASTER_NAME);
    if (err != noErr)
        return err;

    err = CreateUserAndGroup(REAL_BOINC_PROJECT_NAME, REAL_BOINC_PROJECT_NAME);
    if (err != noErr)
        return err;
        
    err = ResynchSystem();
    if (err != noErr)
        return err;
    
    return noErr;
}


// Pass NULL for path when calling this routine from within BOINC Manager
int SetBOINCAppOwnersGroupsAndPermissions(char *path) {
    char                    fullpath[MAXPATHLEN];
    char                    dir_path[MAXPATHLEN];
    char                    buf1[80];
    ProcessSerialNumber     ourPSN;
    FSRef                   ourFSRef, ref;
    char                    *p;
    Boolean                 isDirectory;
    OSStatus                err = noErr;
    
#define NUMBRANDS 3

char *saverName[NUMBRANDS];

saverName[0] = "BOINCSaver";
saverName[1] = "GridRepublic";
saverName[2] = "Progress Thru Processors";

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

#ifdef _DEBUG
    // chmod -R u=rwx,g=rwx,o=rx path/BOINCManager.app
    // 0775 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
    // Set read, write permission for user;  read and execute permission for group and others
    err = DoPrivilegedExec(chmodPath, "-R", "u=rwx,g=rwx,o=rx", fullpath, NULL, NULL, NULL);
#else
    // chmod -R u=rx,g=rx,o=rx path/BOINCManager.app
    // 0555 = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
    // Set read, write permission for user;  read and execute permission for group and others
    err = DoPrivilegedExec(chmodPath, "-R", "u=rx,g=rx,o=rx", fullpath, NULL, NULL, NULL);
#endif
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
    err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
    if (err)
        return err;

#ifdef _DEBUG
        // chmod u=rwx,g=rwx,o=rx path/BOINCManager.app/Contents/MacOS/BOINCManager
        // 0775 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
        // Set read, write and execute permission for user & group, read & execute for others
        err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwx,o=rx", fullpath, NULL, NULL, NULL, NULL);
#else
        // chmod u=rx,g=rx,o=rx path/BOINCManager.app/Contents/MacOS/BOINCManager
        // 0555 = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
        // Set read and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rx,g=rx,o=rx", fullpath, NULL, NULL, NULL, NULL);
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
    err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
    if (err)
        return err;

#ifdef _DEBUG
        // chmod u=rwsx,g=rwsx,o=rx path/BOINCManager.app/Contents/Resources/boinc
        // 06775 = S_ISUID | S_ISGID | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
        // Set setuid-on-execution, setgid-on-execution plus read, write and execute permission for user & group, read & execute for others
        err = DoPrivilegedExec(chmodPath, "u=rwsx,g=rwsx,o=rx", fullpath, NULL, NULL, NULL, NULL);
#else
        // chmod u=rsx,g=rsx,o=rx path/BOINCManager.app/Contents/Resources/boinc
        // 06555 = S_ISUID | S_ISGID | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
        // Set setuid-on-execution, setgid-on-execution plus read and execute permission for user, group & others
        err = DoPrivilegedExec(chmodPath, "u=rsx,g=rsx,o=rx", fullpath, NULL, NULL, NULL, NULL);
#endif
    if (err)
        return err;

    for (int i=0; i<NUMBRANDS; i++) {
        // Version 6 screensaver has its own embedded switcher application, but older versions don't.
        // We don't allow unauthorized users to run the switcher application in the BOINC Data directory 
        // because they could use it to run as user & group boinc_project and damage project files.
        // The screensaver's switcher application runs as user and group "nobody" to avoid this risk.

        // Does switcher exist in screensaver bundle?
        sprintf(fullpath, "/Library/Screen Savers/%s.saver/Contents/Resources/gfx_switcher", saverName[i]);
        err = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);   // Does it exist?
        if ((err == noErr) && (! isDirectory)) {
#ifdef _DEBUG
            sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
            // chown boinc_master:boinc_master "/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher"
            err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
            if (err)
                return err;

            // chmod u=rwx,g=rwx,o=rx "/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher"
            // 0775 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
            // Set read, write and execute permission for user & group;  read and execute permission for others
            err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwx,o=rx", fullpath, NULL, NULL, NULL, NULL);
            if (err)
                return err;
#else
            sprintf(buf1, "root:%s", boinc_master_group_name);
            // chown root:boinc_master "/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher"
            err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
            if (err)
                return err;

            // chmod u=rsx,g=rx,o=rx "/Library/Screen Savers/BOINCSaver.saver/Contents/Resources/gfx_switcher"
            // 04555 = S_ISUID | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
            // Set setuid-on-execution plus read and execute permission for user, group & others
            err = DoPrivilegedExec(chmodPath, "u=rsx,g=rx,o=rx", fullpath, NULL, NULL, NULL, NULL);
            if (err)
                return err;
#endif
        }
    }
    
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
    result = FSPathMakeRef((StringPtr)BOINCDataDirPath, &ref, &isDirectory);
    if ((result != noErr) || (! isDirectory))
        return dirNFErr;                    // BOINC Data Directory does not exist

    // Set owner and group of BOINC Data directory's contents
    sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
    // chown -R boinc_master:boinc_master "/Library/Application Support/BOINC Data"
    err = DoPrivilegedExec(chownPath, "-R", buf1, BOINCDataDirPath, NULL, NULL, NULL);
    if (err)
        return err;

#if 0   // Redundant if we already set BOINC Data directory to boinc_master:boinc_master
    // Set owner and group of BOINC Data directory itself
    sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
    // chown boinc_master:boinc_master "/Library/Application Support/BOINC Data"
    err = DoPrivilegedExec(chownPath, buf1, BOINCDataDirPath, NULL, NULL, NULL, NULL);
    if (err)
        return err;
#endif

    // Set permissions of BOINC Data directory's contents:
    //   ss_config.xml is world-readable so screensaver coordinator can read it
    //   all other *.xml are not world-readable to keep authenticators private
    //   gui_rpc_auth.cfg is not world-readable to keep RPC password private
    //   all other files are world-readable so default screensaver can read them
    
    // First make all files world-readable (temporarily)
    // chmod -R u+rw,g+rw,o+r-w "/Library/Application Support/BOINC Data"
    // 0661 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
    // Set read and write permission for user and group, read-only for others (leaves execute bits unchanged)
    err = DoPrivilegedExec(chmodPath, "-R", "u+rw,g+rw,o+r-w", BOINCDataDirPath, NULL, NULL, NULL);
    if (err)
        return err;
    
    // Next make gui_rpc_auth.cfg not world-readable to keep RPC password private
    // Does gui_rpc_auth.cfg file exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, GUI_RPC_PASSWD_FILE, MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (! isDirectory)) {
        // Make gui_rpc_auth.cfg file readable and writable only by user boinc_master and group boinc_master

        // Set owner and group of gui_rpc_auth.cfg file
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
        // chown boinc_master:boinc_master "/Library/Application Support/BOINC Data/gui_rpc_auth.cfg"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;

        // chmod u=rw,g=rw,o= "/Library/Application Support/BOINC Data/gui_rpc_auth.cfg"
        // 0660 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
        // Set read and write permission for user and group, no access for others
        err = DoPrivilegedExec(chmodPath, "u=rw,g=rw,o=", fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;
    }           // gui_rpc_auth.cfg

    // Next make all *.xml files not world-readable to keep authenticators private
    err = MakeXMLFilesPrivate(BOINCDataDirPath);
    if (err)
        return err;

    // Next make ss_config.xml world-readable so screensaver coordinator can read it
    // Does screensaver config file ss_config.xml exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SS_CONFIG_FILE, MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (! isDirectory)) {
        // Make ss_config.xml file world readable but writable only by user boinc_master and group boinc_master

        // Set owner and group of ss_config.xml file
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
        // chown boinc_master:boinc_master "/Library/Application Support/BOINC Data/ss_config.xml"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;

        // chmod u=rw,g=rw,o=r "/Library/Application Support/BOINC Data/ss_config.xml"
        // 0664 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
        // Set read and write permission for user and group, read-only for others
        err = DoPrivilegedExec(chmodPath, "u=rw,g=rw,o=r", fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;
    }           // ss_config.xml


    // Set permissions of BOINC Data directory itself
    // chmod u=rwx,g=rwx,o=x "/Library/Application Support/BOINC Data"
    // 0771 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IXOTH
    // Set read, write and execute permission for user & group; execute-only permission for others
    err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwx,o=x", BOINCDataDirPath, NULL, NULL, NULL, NULL);
    if (err)
        return err;


    // Does projects directory exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, PROJECTS_DIR, MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
        // Set owner and group of projects directory and it's contents
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_project_group_name);
        // chown -R boinc_master:boinc_project "/Library/Application Support/BOINC Data/projects"
        err = DoPrivilegedExec(chownPath, "-Rh", buf1, fullpath, NULL, NULL, NULL);
        if (err)
            return err;

#if 0       // Redundant if the same as projects directory's contents
        // Set owner and group of projects directory itself
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_project_group_name);
        // chown -R boinc_master:boinc_project "/Library/Application Support/BOINC Data/projects"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;
#endif

        // Set permissions of project directories' contents
        // Contents of project directories must be world-readable so BOINC Client can read 
        // files written by projects which have user boinc_project and group boinc_project
        // chmod -R u+rw,g+rw,o+r-w "/Library/Application Support/BOINC Data/projects"
        // 0664 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
        // set read and write permission for user and group, no access for others (leaves execute bits unchanged)
        err = DoPrivilegedExec(chmodPath, "-R", "u+rw,g+rw,o+r-w", fullpath, NULL, NULL, NULL);
        if (err)
            return err;

        // Set permissions for projects directory itself (not its contents)
        // chmod u=rwx,g=rwx,o= "/Library/Application Support/BOINC Data/projects"
        // 0770 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP
        // Set read, write and execute permission for user & group, no access for others
        err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwx,o=", fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;

        // Set execute permissions for project subdirectories
        err = UpdateNestedDirectories(fullpath);    // Sets execute for user, group and others
        if (err)
            return err;
    }       // projects directory
    
    // Does slots directory exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SLOTS_DIR, MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
        // Set owner and group of slots directory and it's contents
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_project_group_name);
        // chown -R boinc_master:boinc_project "/Library/Application Support/BOINC Data/slots"
        err = DoPrivilegedExec(chownPath, "-Rh", buf1, fullpath, NULL, NULL, NULL);
        if (err)
            return err;

#if 0       // Redundant if the same as slots directory's contents
        // Set owner and group of slots directory itself
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_project_group_name);
        // chown boinc_master:boinc_project "/Library/Application Support/BOINC Data/slots"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;
#endif

        // Set permissions of slot directories' contents
        // Contents of slot directories must be world-readable so BOINC Client can read 
        // files written by projects which have user boinc_project and group boinc_project
        // chmod -R u+rw,g+rw,o+r-w "/Library/Application Support/BOINC Data/slots"
        // 0664 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
        // set read and write permission for user and group, no access for others (leaves execute bits unchanged)
        err = DoPrivilegedExec(chmodPath, "-R", "u+rw,g+rw,o+r-w", fullpath, NULL, NULL, NULL);
        if (err)
            return err;

        // Set permissions for slots directory itself (not its contents)
        // chmod u=rwx,g=rwx,o= "/Library/Application Support/BOINC Data/slots"
        // 0770 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP
        // Set read, write and execute permission for user & group, no access for others
        err = DoPrivilegedExec(chmodPath, "u=rwx,g=rwx,o=", fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;

        // Set execute permissions for slot subdirectories
        err = UpdateNestedDirectories(fullpath);    // Sets execute for user, group and others
        if (err)
            return err;
    }       // slots directory

    // Does locale directory exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/locale", MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
#if 0   // Redundant if we already set contents of BOINC Data directory to boinc_master:boinc_master
        // Set owner and group of locale directory and all its contents
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
        // chown -R boinc_master:boinc_master "/Library/Application Support/BOINC Data/locale"
        err = DoPrivilegedExec(chownPath, "-R", buf1, fullpath, NULL, NULL, NULL);
        if (err)
            return err;
#endif

        // chmod -R u+r-w,g+r-w,o+r-w "/Library/Application Support/BOINC Data/locale"
        // 0550 = S_IRUSR | S_IXUSR | S_IRGRP | S_IXUSR | S_IROTH | S_IXOTH 
        // Set execute permission for user, group, and others if it was set for any
        err = DoPrivilegedExec(chmodPath, "-R", "+X", fullpath, NULL, NULL, NULL);
        // Set read-only permission for user, group, and others (leaves execute bits unchanged)
        err = DoPrivilegedExec(chmodPath, "-R", "u+r-w,g+r-w,o+r-w", fullpath, NULL, NULL, NULL);
        if (err)
            return err;
    }       // locale directory
    
    // Does switcher directory exist?
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SWITCHER_DIR, MAXPATHLEN);

    result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (isDirectory)) {
#if 0   // Redundant if we already set contents of BOINC Data directory to boinc_master:boinc_master
        // Set owner and group of switcher directory
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_master_group_name);
        // chown boinc_master:boinc_master "/Library/Application Support/BOINC Data/switcher"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;
#endif

        // chmod u=rx,g=rx,o= "/Library/Application Support/BOINC Data/switcher"
        // 0550 = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP
        // Set read and execute permission for user and group, no access for others
        err = DoPrivilegedExec(chmodPath, "u=rx,g=rx,o=", fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;
    }       // switcher directory

    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SWITCHER_FILE_NAME, MAXPATHLEN);
        result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (! isDirectory)) {
        // Set owner and group of switcher application
        sprintf(buf1, "root:%s", boinc_master_group_name);
        // chown root:boinc_master "/Library/Application Support/BOINC Data/switcher/switcher"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;

        // Set permissions of switcher application
        // chmod u=s,g=rx,o= "/Library/Application Support/BOINC Data/switcher/switcher"
        // 04050 = S_ISUID | S_IRGRP | S_IXGRP
        // Set setuid-on-execution plus read and execute permission for group boinc_master only
        err = DoPrivilegedExec(chmodPath, "u=s,g=rx,o=", fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;
    }       // switcher application
    
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SWITCHER_DIR, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SETPROJECTGRP_FILE_NAME, MAXPATHLEN);
        result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (! isDirectory)) {
        // Set owner and group of setprojectgrp application
        sprintf(buf1, "%s:%s", boinc_master_user_name, boinc_project_group_name);
        // chown boinc_master:boinc_project "/Library/Application Support/BOINC Data/switcher/setprojectgrp"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;

        // Set permissions of setprojectgrp application
        // chmod u=rx,g=s,o= "/Library/Application Support/BOINC Data/switcher/setprojectgrp"
        // 02500 = S_ISGID | S_IRUSR | S_IXUSR
        // Set setgid-on-execution plus read and execute permission for user only
        err = DoPrivilegedExec(chmodPath, "u=rx,g=s,o=", fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;
    }       // setprojectgrp application

#ifdef __APPLE__
#if 0       // AppStats is deprecated as of version 5.8.15
    strlcpy(fullpath, BOINCDataDirPath, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, SWITCHER_DIR, MAXPATHLEN);
    strlcat(fullpath, "/", MAXPATHLEN);
    strlcat(fullpath, APP_STATS_FILE_NAME, MAXPATHLEN);
        result = FSPathMakeRef((StringPtr)fullpath, &ref, &isDirectory);
    if ((result == noErr) && (! isDirectory)) {
        // Set owner and group of AppStats application (must be setuid root)
        sprintf(buf1, "root:%s", boinc_master_group_name);
        // chown root:boinc_project "/Library/Application Support/BOINC Data/switcher/AppStats"
        err = DoPrivilegedExec(chownPath, buf1, fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;

        // Set permissions of AppStats application
        // chmod u=rsx,g=rx,o= "/Library/Application Support/BOINC Data/switcher/AppStats"
        // 04550 = S_ISUID | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP
        // Set setuid-on-execution plus read and execute permission for user and group
        err = DoPrivilegedExec(chmodPath, "u=rsx,g=rx,o=", fullpath, NULL, NULL, NULL, NULL);
        if (err)
            return err;
    }       // setprojectgrp application
#endif
#endif  // __APPLE__

    return noErr;
}


// make all *.xml files not world-readable to keep authenticators private
static OSStatus MakeXMLFilesPrivate(char * basepath) {
    char            fullpath[MAXPATHLEN];
    OSStatus        retval = 0;
    DIR             *dirp;
    int             len;
    dirent          *dp;

    dirp = opendir(basepath);
    if (dirp == NULL)           // Should never happen
        return -1;
        
    while (true) {
        dp = readdir(dirp);
        if (dp == NULL)
            break;                  // End of list
        
        if (dp->d_name[0] == '.')
            continue;               // Ignore names beginning with '.'
        
        len = strlen(dp->d_name);
        if (len < 5)
            continue;
        
        if (strcmp(dp->d_name+len-4, ".xml"))
            continue;

        strlcpy(fullpath, basepath, sizeof(fullpath));
        strlcat(fullpath, "/", sizeof(fullpath));
        strlcat(fullpath, dp->d_name, sizeof(fullpath));

        // chmod u+rw,g+rw,o= "/Library/Application Support/BOINC Data/????.xml"
        // 0660 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP0
        // Set read and write permission for user and group, no access for others
        retval = DoPrivilegedExec(chmodPath, "u+rw,g+rw,o=", fullpath, NULL, NULL, NULL, NULL);
        if (retval)
            break;
    }       // End while (true)
    
    closedir(dirp);

    return retval;
}


static OSStatus UpdateNestedDirectories(char * basepath) {
    Boolean         isDirectory;
    char            fullpath[MAXPATHLEN];
    struct stat     sbuf;
    OSStatus        retval = 0;
    DIR             *dirp;
    dirent          *dp;

    dirp = opendir(basepath);
    if (dirp == NULL)           // Should never happen
        return -1;
        
    while (true) {
        dp = readdir(dirp);
        if (dp == NULL)
            break;                  // End of list
            
        if (dp->d_name[0] == '.')
            continue;               // Ignore names beginning with '.'

        strlcpy(fullpath, basepath, sizeof(fullpath));
        strlcat(fullpath, "/", sizeof(fullpath));
        strlcat(fullpath, dp->d_name, sizeof(fullpath));

        retval = stat(fullpath, &sbuf);
        if (retval) {
            if (lstat(fullpath, &sbuf) == 0) {
                // A broken symlink in a slot directory may be OK if slot is no longer in use
                if (S_ISLNK(sbuf.st_mode)) {
                    retval = 0;
                    continue;
                }
            }
            break;              // Should never happen
        }
        isDirectory = S_ISDIR(sbuf.st_mode);

        if (isDirectory) {
            // chmod u=rwx,g=rwx,o=rx fullpath
            // 0775 = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH
            // Set read, write and execute permission for user & group;  read and execute permission for others
            retval = DoPrivilegedExec(chmodPath, "u=rwx,g=rwx,o=rx", fullpath, NULL, NULL, NULL, NULL);
            if (retval)
                break;

            retval = UpdateNestedDirectories(fullpath);
            if (retval)
                break;
        } else {
            // Since we are changing ownership from boinc_project to boinc_master, 
            // make sure executable-by-group bit is set if executable-by-owner is set 
            if ((sbuf.st_mode & 0110) == 0100) {    // If executable by owner but not by group
                retval = DoPrivilegedExec(chmodPath, "g+x", fullpath, NULL, NULL, NULL, NULL);
            }
        }
            
    }       // End while (true)
    
    closedir(dirp);

    return retval;
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
    char            buf4[80];
    SInt32          response;
   
    err = Gestalt(gestaltSystemVersion, &response);
    if (err) return err;
    
    // OS 10.4 has problems with Accounts pane if we create uid or gid > 501
    if (response < 0x1050) {
        start_id = 25;
    }
    
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
    
    sprintf(buf1, "/groups/%s", group_name);
    sprintf(buf2, "/users/%s", user_name);

    if ( userExists && groupExists )
        goto setRealName;       // User and group already exist

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
    
    sprintf(buf3, "%d", groupid);
    sprintf(buf4, "%d", userid);

    if (! groupExists) {             // If we need to create group
        // Something like "dscl . -create /groups/boinc_master"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf1, NULL, NULL, NULL);
        if (err)
            return err;
 
        // Something like "dscl . -create /groups/boinc_master gid 33"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf1, "gid", buf3, NULL);
        if (err)
            return err;
    }           // if (! groupExists)
        
    if (! userExists) {             // If we need to create user
        // Something like "dscl . -create /users/boinc_master"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf2, NULL, NULL, NULL);
        if (err)
            return err;

        // Something like "dscl . -create /users/boinc_master uid 33"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf2, "uid", buf4, NULL);
        if (err)
            return err;

        // Prevent a security hole by not allowing a login from this user
        // Something like "dscl . -create /users/boinc_master shell /usr/bin/false"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf2, "shell", "/usr/bin/false", NULL);
        if (err)
            return err;

        // Something like "dscl . -create /users/boinc_master home /var/empty"
        err = DoPrivilegedExec(dsclPath, ".", "-create", buf2, "home", "/var/empty", NULL);
        if (err)
            return err;
    }           // if (! userExists)

    // Always set the user gid if we created either the user or the group or both
    // Something like "dscl . -create /users/boinc_master gid 33"
    err = DoPrivilegedExec(dsclPath, ".", "-create", buf2, "gid", buf3, NULL);
    if (err)
        return err;

setRealName:
    // Always set the RealName field to an empty string
    // Note: create RealName with empty string fails under OS 10.7, but 
    // creating it with non-empty string and changing to empty string does work.
    //
    // Something like "dscl . -create /users/boinc_master RealName tempName"
    err = DoPrivilegedExec(dsclPath, ".", "-create", buf2, "RealName", user_name, NULL);
    if (err)
        return err;

    // Something like 'dscl . -change /users/boinc_master RealName ""'
    err = DoPrivilegedExec(dsclPath, ".", "-change", buf2, "RealName", user_name, "");
    if (err)
        return err;

    err = ResynchSystem();
    if (err != noErr)
        return err;

    SleepTicks(120);

    return noErr;
}


int AddAdminUserToGroups(char *user_name, bool add_to_boinc_project) {        
#ifndef _DEBUG
    char            buf1[80];
    OSStatus        err = noErr;

    sprintf(buf1, "/groups/%s", boinc_master_group_name);

    // "dscl . -merge /groups/boinc_master users user_name"
    err = DoPrivilegedExec(dsclPath, ".", "-merge", buf1, "users", user_name, NULL);
    if (err)
        return err;

    if (add_to_boinc_project)  {
        sprintf(buf1, "/groups/%s", boinc_project_group_name);

        // "dscl . -merge /groups/boinc_project users user_name"
        err = DoPrivilegedExec(dsclPath, ".", "-merge", buf1, "users", user_name, NULL);
        if (err)
            return err;
    }
    
    err = ResynchSystem();
    if (err != noErr)
        return err;

#endif          // ! _DEBUG    
    return noErr;
}


static OSStatus ResynchSystem() {
    SInt32          response;
    OSStatus        err = noErr;
   
    err = Gestalt(gestaltSystemVersion, &response);
    if (err) return err;
    
    if (response >= 0x1050) {
        // OS 10.5
        err = system("dscacheutil -flushcache");
        err = system("dsmemberutil flushcache");
        return noErr;
    }
    
    err = system("lookupd -flushcache");

    err = Gestalt(gestaltSystemVersion, &response);
    if ((err == noErr) && (response >= 0x1040))
        err = system("memberd -r");           // Available only in OS 10.4

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
    long                response;
    OSStatus            err = noErr;

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
    
    err = Gestalt(gestaltSystemVersion, (SInt32*)&response);
#ifndef DEBUG_WITH_FAKE_PROJECT_USER_AND_GROUP
    if ((err == noErr) && (response >= 0x1040)) {
        // For better debugging of SANDBOX permissions logic
        strlcpy(boinc_project_user_name, REAL_BOINC_PROJECT_NAME, sizeof(boinc_project_user_name));
        strlcpy(boinc_project_group_name, REAL_BOINC_PROJECT_NAME, sizeof(boinc_project_group_name));
    } else 
#endif
    {
        // For easier debugging of project applications; required under OS 10.3.x
        strlcpy(boinc_project_user_name, pw->pw_name, sizeof(boinc_project_user_name));
        strlcpy(boinc_project_group_name, grp->gr_name, sizeof(boinc_project_group_name));
     }
     
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

OSStatus DoPrivilegedExec(const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5, char *arg6) {
    short               i;
    char                *args[8];
    OSStatus            err;
    FILE                *ioPipe = NULL;
    char                *p, junk[256];

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
            args[5] = arg6;
            args[6] = NULL;

            err = AuthorizationExecuteWithPrivileges (gOurAuthRef, pathToTool, 0, args, &ioPipe);
            if (ioPipe) {
                // We use the pipe to signal us when the command has completed
                do {
                    p = fgets(junk, sizeof(junk), ioPipe);
                } while (p);
                
                fclose (ioPipe);
            }

            // AuthorizationExecuteWithPrivileges() does a fork() and so 
            // leaves a zombie process.  Clear these so we don't exceed 
            // the system-imposed limit of processes per user (MAXUPRC).
            while (waitpid(-1, 0, WNOHANG) > 0);
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

#ifdef __x86_64__
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
#else
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
#endif
}


#ifndef __x86_64__
static pascal Boolean ErrorDlgFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *theItemHit) {
    // We need this because this is a command-line application so it does not get normal events
    if (Button()) {
        *theItemHit = kStdOkItemIndex;
        return true;
    }
    
    return StdFilterProc(theDialog, theEvent, theItemHit);
}
#endif

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

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
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

/* PostInstall.cpp */

// Notes on command-line installation to a remote Mac:
//
// When the installer is run from the Finder, this Postinstall.app will 
// display up to two dialogs, asking the user whether or not to:
//  [1] allow non-administrative users to run the BOINC Manager
//      (asked only if this Mac has any non-administrative users)
//  [2] set BOINC as the screensaver for all users who can run BOINC 
//      (asked only if BOINC screensaver is not already set for them)
//
// The installer can also be run from the command line.  This is useful 
//  for installation on remote Macs.  However, there is no way to respond 
//  to dialogs during a command-line install.
//
// The command-line installer sets the following environment variable: 
//     COMMAND_LINE_INSTALL=1
// The postinstall script, postupgrade script, and this Postinstall.app 
//   detect this environment variable and do the following:
//  * Redirect the Postinstall.app log output to a file 
//      /tmp/BOINCInstallLog.txt
//  * Suppress the 2 dialogs
//  * test for the existence of a file /tmp/nonadminusersok.txt; if the 
//     file exists, allow non-administrative users to run BOINC Manager
//  * test for the existence of a file /tmp/setboincsaver.txt; if the 
//     file exists, set BOINC as the screensaver for all BOINC users. 
//
// Example: To install on a remote Mac from the command line, allowing 
//   non-admin users to run the BOINC Manager and setting BOINC as the 
//   screensaver:
//  * First SCP the "BOINC Installer.pkg" to the remote Mac's /tmp 
//     directory, then SSh into the remote mac and enter the following
//  $ touch /tmp/nonadminusersok.txt
//  $ touch /tmp/setboincsaver.txt
//  $ sudo installer -pkg "/tmp/BOINC Installer.pkg" -tgt /
//  $ sudo reboot
//

#define CREATE_LOG 1    /* for debugging */

#include <Carbon/Carbon.h>
#include <grp.h>

#include <unistd.h>	// getlogin
#include <sys/types.h>	// getpwname, getpwuid, getuid
#include <pwd.h>	// getpwname, getpwuid, getuid
#include <grp.h>        // getgrnam
#include <sys/wait.h>	// waitpid
#include <dirent.h>
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>   // for chmod
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <time.h>       // for time()
#include <vector>
#include <string>

using std::vector;
using std::string;

#include "LoginItemAPI.h"

#include "SetupSecurity.h"


#define admin_group_name "admin"
#define boinc_master_user_name "boinc_master"
#define boinc_master_group_name "boinc_master"
#define boinc_project_user_name "boinc_project"
#define boinc_project_group_name "boinc_project"

void Initialize(void);	/* function prototypes */
Boolean myFilterProc(DialogRef theDialog, EventRecord *theEvent, DialogItemIndex *itemHit);
int DeleteReceipt(void);
OSStatus CheckLogoutRequirement(int *finalAction);
void CheckUserAndGroupConflicts();
Boolean SetLoginItemOSAScript(long brandID, Boolean deleteLogInItem, char *userName);
Boolean SetLoginItemAPI(long brandID, Boolean deleteLogInItem);
void SetSkinInUserPrefs(char *userName, char *skinName);
Boolean CheckDeleteFile(char *name);
void SetEUIDBackToUser (void);
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
Boolean IsUserMemberOfGroup(const char *userName, const char *groupName);
int CountGroupMembershipEntries(const char *userName, const char *groupName);
OSErr UpdateAllVisibleUsers(long brandID);
long GetBrandID(void);
int TestRPCBind(void);
static OSStatus ResynchSystem(void);
OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN);
pid_t FindProcessPID(char* name, pid_t thePID);
static void SleepTicks(UInt32 ticksToSleep);
int FindSkinName(char *name, size_t len);
static OSErr QuitOneProcess(OSType signature);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

extern int check_security(char *bundlePath, char *dataPath, 
                            int use_sandbox, int isManager, 
                            char* path_to_error = NULL
                        );

#define NUMBRANDS 4

/* globals */
static Boolean                  gCommandLineInstall = false;
static Boolean                  gQuitFlag = false;
static Boolean                  currentUserCanRunBOINC = false;
static char                     loginName[256];
static long                     OSVersion = 0;
static time_t                   waitPermissionsStartTime;

static char *saverName[NUMBRANDS];
static char *saverNameEscaped[NUMBRANDS];
static char *brandName[NUMBRANDS];
static char *appName[NUMBRANDS];
static char *appPath[NUMBRANDS];
static char *appPathEscaped[NUMBRANDS];
static char *receiptNameEscaped[NUMBRANDS];

enum { launchWhenDone,
        logoutRequired,
        restartRequired,
        nothingrequired
    };

/******************************************************************
***                                                             ***
***                         NOTE:                               ***
***                                                             ***
*** On entry, the postinstall or postupgrade script has set the ***
*** current directory to the top level of our installer package ***
***                                                             ***
******************************************************************/

int main(int argc, char *argv[])
{
    Boolean                 Success;
    ProcessSerialNumber     ourProcess, installerPSN;
    short                   itemHit;
    long                    brandID = 0;
    int                     i;
    pid_t                   installerPID = 0, coreClientPID = 0;
    FSRef                   fileRef;
    OSStatus                err, err_fsref;
    FILE                    *f;
    char                    s[256];
#ifndef SANDBOX
    group                   *grp;
#endif  // SANDBOX

    appName[0] = "BOINCManager";
    appPath[0] = "/Applications/BOINCManager.app";
    appPathEscaped[0] = "/Applications/BOINCManager.app";
    brandName[0] = "BOINC";
    saverName[0] = "BOINCSaver";
    saverNameEscaped[0] = "BOINCSaver";
    receiptNameEscaped[0] = "/Library/Receipts/BOINC\\ Installer.pkg";

    appName[1] = "GridRepublic Desktop";
    appPath[1] = "/Applications/GridRepublic Desktop.app";
    appPathEscaped[1] = "/Applications/GridRepublic\\ Desktop.app";
    brandName[1] = "GridRepublic";
    saverName[1] = "GridRepublic";
    saverNameEscaped[1] = "GridRepublic";
    receiptNameEscaped[1] = "/Library/Receipts/GridRepublic\\ Installer.pkg";

    appName[2] = "Progress Thru Processors Desktop";
    appPath[2] = "/Applications/Progress Thru Processors Desktop.app";
    appPathEscaped[2] = "/Applications/Progress\\ Thru\\ Processors\\ Desktop.app";
    brandName[2] = "Progress Thru Processors";
    saverName[2] = "Progress Thru Processors";
    saverNameEscaped[2] = "Progress\\ Thru\\ Processors";
    receiptNameEscaped[2] = "/Library/Receipts/Progress\\ Thru\\ Processors\\ Installer.pkg";

    appName[3] = "Charity Engine Desktop";
    appPath[3] = "/Applications/Charity Engine Desktop.app";
    appPathEscaped[3] = "/Applications/Charity\\ Engine\\ Desktop.app";
    brandName[3] = "Charity Engine";
    saverName[3] = "Charity Engine";
    saverNameEscaped[3] = "Charity\\ Engine";
    receiptNameEscaped[3] = "/Library/Receipts/Charity\\ Engine\\ Installer.pkg";

    ::GetCurrentProcess (&ourProcess);

    puts("Starting PostInstall app\n");
    fflush(stdout);
    // getlogin() gives unreliable results under OS 10.6.2, so use environment
    strncpy(loginName, getenv("USER"), sizeof(loginName)-1);
    printf("login name = %s\n", loginName);
    fflush(stdout);

    if (getenv("COMMAND_LINE_INSTALL") != NULL) {
        gCommandLineInstall = true;
        puts("command-line install\n");
        fflush(stdout);
    }

    err = Gestalt(gestaltSystemVersion, &OSVersion);
    if (err != noErr) {
        printf("Gestalt(gestaltSystemVersion) returned error %ld\n", err);
        fflush(stdout);
        return err;
    }
    
    for (i=0; i<argc; i++) {
        if (strcmp(argv[i], "-part2") == 0)
            return DeleteReceipt();
    }

    Initialize();

    QuitOneProcess('BNC!'); // Quit any old instance of BOINC manager
    sleep(2);

    // Core Client may still be running if it was started without Manager
    coreClientPID = FindProcessPID("boinc", 0);
    if (coreClientPID)
        kill(coreClientPID, SIGTERM);   // boinc catches SIGTERM & exits gracefully

    err = FindProcess ('APPL', 'xins', &installerPSN);
    if (err == noErr)
        err = GetProcessPID(&installerPSN , &installerPID);

    brandID = GetBrandID();
    
    if ((brandID < 0) || (brandID >= NUMBRANDS)) {       // Safety check
        brandID = 0;
    }
    
    if (OSVersion < 0x1040) {
        ::SetFrontProcess(&ourProcess);
        // Remove everything we've installed
        // "\pSorry, this version of GridRepublic requires system 10.4.0 or higher."
        s[0] = sprintf(s+1, "Sorry, this version of %s requires system 10.4.0 or higher.", brandName[brandID]);
        StandardAlert (kAlertStopAlert, (StringPtr)s, NULL, NULL, &itemHit);

        // "rm -rf /Applications/GridRepublic\\ Desktop.app"
        sprintf(s, "rm -rf %s", appPathEscaped[brandID]);
        system (s);
        
        // "rm -rf /Library/Screen\\ Savers/GridRepublic.saver"
        sprintf(s, "rm -rf /Library/Screen\\ Savers/%s.saver", saverNameEscaped[brandID]);
        system (s);
        
        // "rm -rf /Library/Receipts/GridRepublic.pkg"
        sprintf(s, "rm -rf %s", receiptNameEscaped[brandID]);
        system (s);

        // We don't customize BOINC Data directory name for branding
        system ("rm -rf /Library/Application\\ Support/BOINC\\ Data");

        err = kill(installerPID, SIGKILL);

	ExitToShell();
    }
    
    sleep (2);

    // Install all_projects_list.xml file, but only if one doesn't 
    // already exist, since a pre-existing one is probably newer.
    f = fopen("/Library/Application Support/BOINC Data/all_projects_list.xml", "r");
    if (f) {
        fclose(f);      // Already exists
    } else {
        system ("cp -fp Contents/Resources/all_projects_list.xml /Library/Application\\ Support/BOINC\\ Data/");
        system ("chmod a-x /Library/Application\\ Support/BOINC\\ Data/all_projects_list.xml");
    }
    
    Success = false;
    
#ifdef SANDBOX

    CheckUserAndGroupConflicts();
    for (i=0; i<5; ++i) {
        err = CreateBOINCUsersAndGroups();
        if (err != noErr) {
            printf("CreateBOINCUsersAndGroups returned %ld (repetition=%d)", err, i);
            fflush(stdout);
            continue;
        }
        
        // err = SetBOINCAppOwnersGroupsAndPermissions("/Applications/GridRepublic Desktop.app");
        err = SetBOINCAppOwnersGroupsAndPermissions(appPath[brandID]);
        
        if (err != noErr) {
            printf("SetBOINCAppOwnersGroupsAndPermissions returned %ld (repetition=%d)", err, i);
            fflush(stdout);
            continue;
        }

        err = SetBOINCDataOwnersGroupsAndPermissions();
        if (err != noErr) {
            printf("SetBOINCDataOwnersGroupsAndPermissions returned %ld (repetition=%d)", err, i);
            fflush(stdout);
            continue;
        }
        
        err = check_security(appPath[brandID], "/Library/Application Support/BOINC Data", true, false);
        if (err != noErr) {
            printf("check_security returned %ld (repetition=%d)", err, i);
            fflush(stdout);
        } else {
            break;
        }
    }
    
#else   // ! defined(SANDBOX)

    // The BOINC Manager and Core Client have the set-user-ID-on-execution 
    // flag set, so their ownership is important and must match the 
    // ownership of the BOINC Data directory.
    
    // Find an appropriate admin user to set as owner of installed files
    // First, try the user currently logged in
    grp = getgrnam(admin_group_name);
    i = 0;
    while ((p = grp->gr_mem[i]) != NULL) {   // Step through all users in group admin
        if (strcmp(p, loginName) == 0) {
            Success = true;     // Logged in user is a member of group admin
            break;
        }
        ++i;
    }
    
    // If currently logged in user is not admin, use first non-root admin user
    if (!Success) {
        i = 0;
        while ((p = grp->gr_mem[i]) != NULL) {   // Step through all users in group admin
            if (strcmp(p, "root") != 0)
                break;
            ++i;
        }
    }

    // Set owner of branded BOINCManager and contents, including core client
    // "chown -Rf username /Applications/GridRepublic\\ Desktop.app"
    sprintf(s, "chown -Rf %s %s", p, appPathEscaped[brandID]);
    system (s);

    // Set owner of BOINC Screen Saver
    // "chown -Rf username /Library/Screen\\ Savers/GridRepublic.saver"
    sprintf(s, "chown -Rf %s /Library/Screen\\ Savers/%s.saver", p, saverNameEscaped[brandID]);
    system (s);

    //  We don't customize BOINC Data directory name for branding
    // "chown -Rf username /Library/Application\\ Support/BOINC\\ Data"
    sprintf(s, "chown -Rf %s /Library/Application\\ Support/BOINC\\ Data", p);
    system (s);

    // "chmod -R a+s /Applications/GridRepublic\\ Desktop.app"
    sprintf(s, "chmod -R a+s %s", appPathEscaped[brandID]);
    system (s);

#endif   // ! defined(SANDBOX)

    // Remove any branded versions of BOINC other than ours (i.e., old versions) 
    for (i=0; i< NUMBRANDS; i++) {
        if (i == brandID) continue;
        
        // "rm -rf /Applications/GridRepublic\\ Desktop.app"
        sprintf(s, "rm -rf %s", appPathEscaped[i]);
        system (s);
        
        // "rm -rf /Library/Screen\\ Savers/GridRepublic.saver"
        sprintf(s, "rm -rf /Library/Screen\\ Savers/%s.saver", saverNameEscaped[i]);
        system (s);
    }
    
   if (brandID == 0) {  // Installing generic BOINC
        system ("rm -f /Library/Application\\ Support/BOINC\\ Data/Branding");
    }
    
    // err_fsref = FSPathMakeRef((StringPtr)"/Applications/GridRepublic Desktop.app", &fileRef, NULL);
    err_fsref = FSPathMakeRef((StringPtr)appPath[brandID], &fileRef, NULL);
    
    if (err_fsref == noErr)
        err = LSRegisterFSRef(&fileRef, true);
    
    err = UpdateAllVisibleUsers(brandID);
    if (err != noErr)
        return err;

#if 0   // WaitPermissions is not needed when using wrapper
#ifdef SANDBOX
    pid_t                   waitPermissionsPID = 0;
    uid_t                   saved_euid, saved_uid, b_m_uid;
    passwd                  *pw;
    int                     finalInstallAction;
    DialogRef               theWin;

    err = CheckLogoutRequirement(&finalInstallAction);
    printf("CheckLogoutRequirement returned %d\n", finalInstallAction);
    fflush(stdout);
    
    if (finalInstallAction == launchWhenDone) {

        // Wait for BOINC's RPC socket address to become available to user boinc_master, in
        // case we are upgrading from a version which did not run as user boinc_master.
        saved_uid = getuid();
        saved_euid = geteuid();
        
        pw = getpwnam(boinc_master_user_name);
        b_m_uid = pw->pw_uid;
        seteuid(b_m_uid);
        
        for (i=0; i<120; i++) {
            err = TestRPCBind();
            if (err == noErr)
                break;
            
            sleep(1);
        }
        
        seteuid(saved_euid);
            
        FSRef               theFSRef;

        err = FSPathMakeRef((StringPtr)"/Library/Application Support/BOINC Data/WaitPermissions.app", 
                &theFSRef, NULL);
        if (err) {
            printf("FSPathMakeRef(WaitPermissions) returned error %ld\n", err);
            fflush(stdout);
        }
        
        // When we first create the boinc_master group and add the current user to the 
        // new group, there is a delay before the new group membership is recognized.  
        // If we launch the BOINC Manager too soon, it will fail with a -1037 permissions 
        // error, so we wait until the current user can access the switcher application.
        // Apparently, in order to get the changed permissions / group membership, we must 
        // launch a new process belonging to the user.  It may also need to be in a new 
        // process group or new session. Neither system() nor popen() works, even after 
        // setting the uid and euid back to the logged in user, but LSOpenFSRef() does.
        // The WaitPermissions application loops until it can access the switcher 
        // application.
        err = LSOpenFSRef(&theFSRef, NULL);
        if (err) {
            printf("LSOpenFSRef(WaitPermissions) returned error %ld\n", err);
            fflush(stdout);
        }
        waitPermissionsStartTime = time(NULL);
        for (i=0; i<15; i++) {     // Show "Please wait..." alert after 15 seconds
            waitPermissionsPID = FindProcessPID("WaitPermissions", 0);
            if (waitPermissionsPID == 0) {
                return 0;
            }
            sleep(1);
        }
        
        if (gCommandLineInstall) {
            printf("Finishing install.  Please wait ...\n");
            printf("This may take a few more minutes.\n");
            fflush(stdout);
        } else {
            CreateStandardAlert(kAlertNoteAlert, CFSTR("Finishing install.  Please wait ..."), CFSTR("This may take a few more minutes."), NULL, &theWin);
            HideDialogItem(theWin, kStdOkItemIndex);
            RemoveDialogItems(theWin, kStdOkItemIndex, 1, false);
            RunStandardAlert(theWin, &myFilterProc, &itemHit);
        }
    }
#endif   // SANDBOX
#endif  // WaitPermissions is not needed when using wrapper

    
    return 0;
}


Boolean myFilterProc(DialogRef theDialog, EventRecord *theEvent, DialogItemIndex *itemHit) {
    static time_t       lastCheckTime = 0;
    time_t              now = time(NULL);
    pid_t               waitPermissionsPID = 0;

    if (now != lastCheckTime) {
            waitPermissionsPID = FindProcessPID("WaitPermissions", 0);
            if (waitPermissionsPID == 0) {
                *itemHit = kStdOkItemIndex;
                return true;
            }
        lastCheckTime = now;
        // Limit delay to 3 minutes
        if ((now - waitPermissionsStartTime) > 180) {
            *itemHit = kStdOkItemIndex;
            return true;
        }
    }

return false;
}


// After installation has completed, delete the installer receipt.  
// If we don't need to logout the user, also launch BOINC Manager.
int DeleteReceipt()
{
    ProcessSerialNumber     installerPSN;
    long                    brandID = 0;
    int                     i;
    pid_t                   installerPID = 0;
    OSStatus                err;
    int                     finalInstallAction;
    FSRef                   fileRef;
    char                    s[256];
    struct stat             sbuf;
    OSStatus                err_fsref;

    Initialize();

    err = CheckLogoutRequirement(&finalInstallAction);
//    print_to_log_file("CheckLogoutRequirement returned %d\n", finalInstallAction);
    
    brandID = GetBrandID();

    // Remove installer package receipt so we can run installer again if needed to fix permissions
    // "rm -rf /Library/Receipts/GridRepublic.pkg"
    sprintf(s, "rm -rf %s", receiptNameEscaped[brandID]);
    system (s);

    // err_fsref = FSPathMakeRef((StringPtr)"/Applications/GridRepublic Desktop.app", &fileRef, NULL);
    err_fsref = FSPathMakeRef((StringPtr)appPath[brandID], &fileRef, NULL);
    if (finalInstallAction == launchWhenDone) {

        err = FindProcess ('APPL', 'xins', &installerPSN);
        if (err == noErr) {
            err = GetProcessPID(&installerPSN , &installerPID);

           // Launch BOINC Manager when user closes installer or after 15 seconds
            for (i=0; i<15; i++) { // Wait 15 seconds max for installer to quit
                sleep (1);
                if (err == noErr)
                    if (FindProcessPID(NULL, installerPID) == 0)
                        break;
            }
        }

        // If system is set up to run BOINC Client as a daemon using launchd, launch it 
        //  as a daemon and allow time for client to start before launching BOINC Manager.
        err = stat("/Library/LaunchDaemons/edu.berkeley.boinc.plist", &sbuf);
        if (err == noErr) {
            system("launchctl unload /Library/LaunchDaemons/edu.berkeley.boinc.plist");
            i = system("launchctl load /Library/LaunchDaemons/edu.berkeley.boinc.plist");
            if (i == 0) sleep (2);
        }
        err = LSOpenFSRef(&fileRef, NULL);
    }

    return 0;
}


OSStatus CheckLogoutRequirement(int *finalAction)
{    
    *finalAction = restartRequired;

    if (OSVersion < 0x1040) {
        return noErr;   // Always require restart on OS 10.3.9
    }
    
#ifdef SANDBOX
    if (loginName[0]) {
        if (!IsUserMemberOfGroup(loginName, boinc_master_group_name)) {
            *finalAction = nothingrequired;
            return noErr;   // Logged in user is not a member of group boinc_master
        }
    }
#endif  // SANDBOX

    char                    path[MAXPATHLEN];
    FSRef                   infoPlistFileRef;
    Boolean                 isDirectory, result;
    CFURLRef                xmlURL = NULL;
    CFDataRef               xmlDataIn = NULL;
    CFPropertyListRef       propertyListRef = NULL;
    CFStringRef             restartKey = CFSTR("IFPkgFlagRestartAction");
    CFStringRef             currentValue = NULL;
//    CFStringRef             valueRestartRequired = CFSTR("RequiredRestart");
    CFStringRef             valueLogoutRequired = CFSTR("RequiredLogout");
    CFStringRef             valueNoRestart = CFSTR("NoRestart");
    CFStringRef             errorString = NULL;
    OSStatus                err = noErr;

    getcwd(path, sizeof(path));
    strlcat(path, "/Contents/Info.plist", sizeof(path));

    err = FSPathMakeRef((UInt8*)path, &infoPlistFileRef, &isDirectory);
    if (err)
        return err;
        
    xmlURL = CFURLCreateFromFSRef(NULL, &infoPlistFileRef);
    if (xmlURL == NULL)
        return -1;

    // Read XML Data from file
    result = CFURLCreateDataAndPropertiesFromResource(NULL, xmlURL, &xmlDataIn, NULL, NULL, &err);
    if (err == noErr)
        if (!result)
            err = coreFoundationUnknownErr;
	
    if (err == noErr) { // Convert XML Data to internal CFPropertyListRef / CFDictionaryRef format
        propertyListRef = CFPropertyListCreateFromXMLData(NULL, xmlDataIn, kCFPropertyListMutableContainersAndLeaves, &errorString);
        if (propertyListRef == NULL)
            err = coreFoundationUnknownErr;
    }
    
    if (err == noErr) { // Get current value for our key
        currentValue = (CFStringRef)CFDictionaryGetValue((CFDictionaryRef)propertyListRef, restartKey);
        if (currentValue == NULL)
            err = coreFoundationUnknownErr;
    }    
    
    if (err == noErr) {
        if (CFStringCompare(currentValue, valueLogoutRequired, 0) == kCFCompareEqualTo)
            *finalAction = logoutRequired;
        else if (CFStringCompare(currentValue, valueNoRestart, 0) == kCFCompareEqualTo)
            *finalAction = launchWhenDone;
    }
   
    if (xmlURL)
        CFRelease(xmlURL);
    if (xmlDataIn)
        CFRelease(xmlDataIn);
    if (propertyListRef)
        CFRelease(propertyListRef);
    return err;
}


// Some newer versions of the OS define users and groups which may conflict with 
// our previously created boinc_master or boinc_project user or group.  This could 
// also happen when the user installs new software.  So we must check for such 
// duplicate UserIDs and groupIDs; if found, we delete our user or group so that 
// the PostInstall application will create a new one that does not conflict.
//
// Older versions of the installer created our users and groups at the first
// unused IDs at or above 25.  Apple now recommends using IDs at or above 501, 
// to reduce the likelihood of conflicts with future UserIDs and groupIDs.
// If we have previously created UserIDs and / or groupIDs below 501, this code 
// now removes them so we can create new ones above 500.
void CheckUserAndGroupConflicts()
{
#ifdef SANDBOX
    passwd          *pw = NULL;
    group           *grp = NULL;
    gid_t           boinc_master_gid = 0, boinc_project_gid = 0;
    uid_t           boinc_master_uid = 0, boinc_project_uid = 0;
    
    FILE            *f;
    char            cmd[256], buf[256];
    int             entryCount;
    OSErr           err = noErr;

    if (OSVersion < 0x1050) {
        // This fails under OS 10.4, but should not be needed under OS 10.4
        return;     
    }

    printf("Checking user and group conflicts\n");
    fflush(stdout);
    
    entryCount = 0;
    grp = getgrnam(boinc_master_group_name);
    if (grp) {
        boinc_master_gid = grp->gr_gid;
        printf("boinc_master group ID = %d\n", (int)boinc_master_gid);
        fflush(stdout);
        if (boinc_master_gid > 500) {
            sprintf(cmd, "dscl . -search /Groups PrimaryGroupID %d", boinc_master_gid);
            f = popen(cmd, "r");
            if (f) {
                while (PersistentFGets(buf, sizeof(buf), f)) {
                    if (strstr(buf, "PrimaryGroupID")) {
                        ++entryCount;
                    }
                }
                pclose(f);
            }    
        }
    }
    if ((boinc_master_gid < 501) || (entryCount > 1)) {
        err = system ("dscl . -delete /groups/boinc_master");
        // User boinc_master must have group boinc_master as its primary group.
        // Since this group no longer exists, delete the user as well.
        if (err) {
            fprintf(stdout, "dscl . -delete /groups/boinc_master returned %d\n", err);
            fflush(stdout);
        }
        err = system ("dscl . -delete /users/boinc_master");
        if (err) {
            fprintf(stdout, "dscl . -delete /users/boinc_master returned %d\n", err);
            fflush(stdout);
        }
        ResynchSystem();
    }

    entryCount = 0;
    grp = getgrnam(boinc_project_group_name);
    if (grp) {
        boinc_project_gid = grp->gr_gid;
        printf("boinc_project group ID = %d\n", (int)boinc_project_gid);
        fflush(stdout);
        if (boinc_project_gid > 500) {
            sprintf(cmd, "dscl . -search /Groups PrimaryGroupID %d", boinc_project_gid);
            f = popen(cmd, "r");
            if (f) {
                while (PersistentFGets(buf, sizeof(buf), f)) {
                    if (strstr(buf, "PrimaryGroupID")) {
                        ++entryCount;
                    }
                }
                pclose(f);
            }    
        }
    }
    
    if ((boinc_project_gid < 501) || (entryCount > 1)) {
       err = system ("dscl . -delete /groups/boinc_project");
        if (err) {
            fprintf(stdout, "dscl . -delete /groups/boinc_project returned %d\n", err);
            fflush(stdout);
        }
        // User boinc_project must have group boinc_project as its primary group.
        // Since this group no longer exists, delete the user as well.
        err = system ("dscl . -delete /users/boinc_project");
        if (err) {
            fprintf(stdout, "dscl . -delete /users/boinc_project returned %d\n", err);
            fflush(stdout);
        }
        ResynchSystem();
    }

    if ((boinc_master_gid < 500) && (boinc_project_gid < 500)) {
        return;
    }

    entryCount = 0;
    pw = getpwnam(boinc_master_user_name);
    if (pw) {
        boinc_master_uid = pw->pw_uid;
        printf("boinc_master user ID = %d\n", (int)boinc_master_uid);
        fflush(stdout);
        sprintf(cmd, "dscl . -search /Users UniqueID %d", boinc_master_uid);
        f = popen(cmd, "r");
        if (f) {
            while (PersistentFGets(buf, sizeof(buf), f)) {
                if (strstr(buf, "UniqueID")) {
                    ++entryCount;
                }
            }
            pclose(f);
        }    
    }

    if (entryCount > 1) {
        err = system ("dscl . -delete /users/boinc_master");
        if (err) {
            fprintf(stdout, "dscl . -delete /users/boinc_master returned %d\n", err);
            fflush(stdout);
        }
        ResynchSystem();
    }
        
    entryCount = 0;
    pw = getpwnam(boinc_project_user_name);
    if (pw) {
        boinc_project_uid = pw->pw_uid;
        printf("boinc_project user ID = %d\n", (int)boinc_project_uid);
        fflush(stdout);
        sprintf(cmd, "dscl . -search /Users UniqueID %d", boinc_project_uid);
        f = popen(cmd, "r");
        if (f) {
            while (PersistentFGets(buf, sizeof(buf), f)) {
                if (strstr(buf, "UniqueID")) {
                    ++entryCount;
                }
            }
            pclose(f);
        }    
    }

    if (entryCount > 1) {
        system ("dscl . -delete /users/boinc_project");
        if (err) {
            fprintf(stdout, "dscl . -delete /users/boinc_project returned %d\n", err);
            fflush(stdout);
        }
        ResynchSystem();
    }
#endif  // SANDBOX
}

enum {
	kSystemEventsCreator = 'sevs'
};


Boolean SetLoginItemOSAScript(long brandID, Boolean deleteLogInItem, char *userName)
{
    int                     i;
    char                    cmd[2048];
    char                    systemEventsPath[1024];
    ProcessSerialNumber     SystemEventsPSN;
	FSRef                   appRef;
    OSErr                   err, err2;

    fprintf(stdout, "Adjusting login items for user %s\n", userName);
    fflush(stdout);

    // We must launch the System Events application for the target user

    // Find SystemEvents process.  If found, quit it in case 
    // it is running under a different user.
    fprintf(stdout, "Telling System Events to quit (at start of SetLoginItemOSAScript)\n");
    fflush(stdout);
    err = QuitOneProcess(kSystemEventsCreator);
    if (err != noErr) {
        fprintf(stdout, "QuitOneProcess(kSystemEventsCreator) returned error %d \n", (int) err);
        fflush(stdout);
    }
    // Wait for the process to be gone
    for (i=0; i<50; ++i) {      // 5 seconds max delay
        SleepTicks(6);  // 6 Ticks == 1/10 second
        err = FindProcess ('APPL', kSystemEventsCreator, &SystemEventsPSN);
        if (err != noErr) break;
    }
    if (i > 50) {
        fprintf(stdout, "Failed to make System Events quit\n");
        fflush(stdout);
    }
    
    err = LSFindApplicationForInfo(kSystemEventsCreator, NULL, NULL, &appRef, NULL);
    if (err != noErr) {
        fprintf(stdout, "LSFindApplicationForInfo(kSystemEventsCreator) returned error %d \n", (int) err);
        fflush(stdout);
    } else {
        FSRefMakePath(&appRef, (UInt8*)systemEventsPath, sizeof(systemEventsPath));
        fprintf(stdout, "SystemEvents is at %s\n", systemEventsPath);
        fprintf(stdout, "Lunching SystemEvents for user %s\n", userName);
        fflush(stdout);

        sprintf(cmd, "sudo -u %s \"%s/Contents/MacOS/System Events\" &", userName, systemEventsPath);
        err = system(cmd);
        if (err) {
            fprintf(stdout, "[2] Command: %s returned error %d\n", cmd, (int) err);
        }
    }
    // Wait for the process to start
    for (i=0; i<50; ++i) {      // 5 seconds max delay
        SleepTicks(6);  // 6 Ticks == 1/10 second
        err = FindProcess ('APPL', kSystemEventsCreator, &SystemEventsPSN);
        if (err == noErr) break;
    }
    if (i > 50) {
        fprintf(stdout, "Failed to launch System Events for user %s\n", userName);
        fflush(stdout);
    }
    
    for (i=0; i<NUMBRANDS; i++) {
        fprintf(stdout, "Deleting any login items containing %s for user %s\n", appName[i], userName);
        fflush(stdout);
        sprintf(cmd, "sudo -u %s osascript -e 'tell application \"System Events\"' -e 'delete (every login item whose path contains \"%s\")' -e 'end tell'", userName, appName[i]);
        err = system(cmd);
        if (err) {
            fprintf(stdout, "[2] Command: %s\n", cmd);
            fprintf(stdout, "[2] Delete login item containing %s returned error %d\n", appName[i], err);
            fflush(stdout);
        }
    }

    if (deleteLogInItem) {
        err = noErr;
        goto cleanupSystemEvents;
    }
    
    fprintf(stdout, "Making new login item %s for user %s\n", appName[brandID], userName);
    fflush(stdout);
    sprintf(cmd, "sudo -u %s osascript -e 'tell application \"System Events\"' -e 'make new login item at end with properties {path:\"%s\", hidden:true, name:\"%s\"}' -e 'end tell'", userName, appPath[brandID], appName[brandID]);
    err = system(cmd);
    if (err) {
        fprintf(stdout, "[2] Command: %s\n", cmd);
        printf("[2] Make login item for %s returned error %d\n", appPath[brandID], err);
    }
    fflush(stdout);

cleanupSystemEvents:
    // Clean up in case this was our last user
    fprintf(stdout, "Telling System Events to quit (at end of SetLoginItemOSAScript)\n");
    fflush(stdout);
    err2 = QuitOneProcess(kSystemEventsCreator);
    if (err2 != noErr) {
        fprintf(stdout, "QuitOneProcess(kSystemEventsCreator) returned error %d \n", (int) err2);
        fflush(stdout);
    }

    return (err == noErr);
}


Boolean SetLoginItemAPI(long brandID, Boolean deleteLogInItem)
{
    Boolean                 Success;
    int                     NumberOfLoginItems, Counter;
    char                    *p, *q;
    char                    s[256];
    int                     i;

    Success = false;
    
    NumberOfLoginItems = GetCountOfLoginItems(kCurrentUser);
    
    // Search existing login items in reverse order, deleting any duplicates of ours
    for (Counter = NumberOfLoginItems ; Counter > 0 ; Counter--)
    {
        p = ReturnLoginItemPropertyAtIndex(kCurrentUser, kApplicationNameInfo, Counter-1);
        q = p;
        while (*q)
        {
            // It is OK to modify the returned string because we "own" it
            *q = toupper(*q);	// Make it case-insensitive
            q++;
        }
    
        for (i=0; i<NUMBRANDS; i++) {
            q = strrchr(appPath[i], '/');
            if (!q) continue;       // Should never happen
            strncpy(s, q+1, sizeof(s)-1);
            q = s;
            while (*q) {
                *q = toupper(*q);
                q++;
            }

            // if (strcmp(p, "BOINCMANAGER.APP") == 0)
            // if (strcmp(p, "GRIDREPUBLIC DESKTOP.APP") == 0)
            // if (strcmp(p, "PROGRESS THRU PROCESSORS DESKTOP.APP") == 0)
            // if (strcmp(p, "CHARITY ENGINE DESKTOP.APP") == 0)
            if (strcmp(p, s) == 0) {
                Success = RemoveLoginItemAtIndex(kCurrentUser, Counter-1);
            }
        }
    }

    if (deleteLogInItem)
        return false;
        
    Success = AddLoginItemWithPropertiesToUser(kCurrentUser, appPath[brandID], kHideOnLaunch);

    return Success;
}



// Sets the skin selection in the specified user's preferences to the specified skin
void SetSkinInUserPrefs(char *userName, char *skinName)
{
    passwd              *pw;
    FILE                *oldPrefs, *newPrefs;
    char                oldFileName[MAXPATHLEN], tempFilename[MAXPATHLEN];
    char                buf[1024];
    int                 wroteSkinName;
    struct stat         sbuf;
    group               *grp;
    OSStatus            statErr;

    if (skinName[0]) {
        sprintf(oldFileName, "/Users/%s/Library/Preferences/BOINC Manager Preferences", userName);
        sprintf(tempFilename, "/Users/%s/Library/Preferences/BOINC Manager NewPrefs", userName);
        newPrefs = fopen(tempFilename, "w");
        if (newPrefs) {
            wroteSkinName = 0;
            statErr = stat(oldFileName, &sbuf);

            oldPrefs = fopen(oldFileName, "r");
            if (oldPrefs) {
                while (fgets(buf, sizeof(buf), oldPrefs)) {
                    if (strstr(buf, "Skin=")) {
                        fprintf(newPrefs, "Skin=%s\n", skinName);
                        wroteSkinName = 1;
                    } else {
                        fputs(buf, newPrefs);
                    }
                }
                fclose(oldPrefs);
            }
            
            if (! wroteSkinName)
                fprintf(newPrefs, "Skin=%s\n", skinName);
                
            fclose(newPrefs);
            rename(tempFilename, oldFileName);  // Deletes old file
            if (! statErr) {
                chown(oldFileName, sbuf.st_uid, sbuf.st_gid);
                chmod(oldFileName, sbuf.st_mode);
            } else {
                chmod(oldFileName, 0664);
                pw = getpwnam(userName);
                grp = getgrnam(userName);
                if (pw && grp)
                    chown(oldFileName, pw->pw_uid, grp->gr_gid);
            }
        }
    }
}

// Returns true if the user name is in the nologinitems.txt, else false
Boolean CheckDeleteFile(char *name)
{
    FILE        *f;
    char        buf[64];
    size_t      len;
    
    f = fopen("/Library/Application Support/BOINC Data/nologinitems.txt", "r");
    if (!f)
        return false;
    
    while (true) {
        *buf = '\0';
        len = sizeof(buf);
        fgets(buf, len, f);
        if (feof(f)) break;
        strip_cr(buf);
        if (strcmp(buf, name) == 0) {
            fclose(f);
            return true;
        }
    }
    
    fclose(f);
    return false;
}

void SetEUIDBackToUser (void)
{
    uid_t login_uid;
    passwd *pw;

    pw = getpwnam(loginName);
    login_uid = pw->pw_uid;

    setuid(login_uid);
    seteuid(login_uid);
}


static char * PersistentFGets(char *buf, size_t buflen, FILE *f) {
    char *p = buf;
    size_t len = buflen;
    size_t datalen = 0;

    *buf = '\0';
    while (datalen < (buflen - 1)) {
        fgets(p, len, f);
        if (feof(f)) break;
        if (ferror(f) && (errno != EINTR)) break;
        if (strchr(buf, '\n')) break;
        datalen = strlen(buf);
        p = buf + datalen;
        len -= datalen;
    }
    return (buf[0] ? buf : NULL);
}


static Boolean ShowMessage(Boolean allowCancel, const char *format, ...) {
    va_list                 args;
    char                    s[1024];
    short                   itemHit;
    AlertStdAlertParamRec   alertParams;
    
    ProcessSerialNumber	ourProcess;

    va_start(args, format);
    s[0] = vsprintf(s+1, format, args);
    va_end(args);

    alertParams.movable = true;
    alertParams.helpButton = false;
    alertParams.filterProc = NULL;
    alertParams.defaultText = (StringPtr)"\pYes";
    alertParams.cancelText = allowCancel ? (StringPtr)"\pNo" : NULL;
    alertParams.otherText = NULL;
    alertParams.defaultButton = kAlertStdAlertOKButton;
    alertParams.cancelButton = allowCancel ? kAlertStdAlertCancelButton : 0;
    alertParams.position = kWindowDefaultPosition;

    ::GetCurrentProcess (&ourProcess);
    ::SetFrontProcess(&ourProcess);

    StandardAlert (kAlertNoteAlert, (StringPtr)s, NULL, &alertParams, &itemHit);
    
    return (itemHit == kAlertStdAlertOKButton);
}


Boolean IsUserMemberOfGroup(const char *userName, const char *groupName) {
    group               *grp;
    short               i = 0;
    char                *p;

    grp = getgrnam(groupName);
    if (!grp) {
        printf("getgrnam(%s) failed\n", groupName);
        fflush(stdout);
        return false;  // Group not found
    }

    while ((p = grp->gr_mem[i]) != NULL) {  // Step through all users in group admin
        if (strcmp(p, userName) == 0) {
            return true;
        }
        ++i;
    }
    return false;
}


// OS 10.7 dscl merge command has a bug such that the command:
//     dscl . -merge /Groups/GROUPNAME users USERNAME
// adds the user to the group even if it was already a member, resulting in 
// duplicate (multiple) entries.  Earlier BOINC versions used this command 
// but did not check for this, so we remove duplicate entries if present.
// Note: We now avoid this problem by instead using the command:
//     dscl . -merge /Groups/GROUPNAME GroupMembership USERNAME
// which correctly avoids duplication.

int CountGroupMembershipEntries(const char *userName, const char *groupName) {
    int                 count = 0;
    char                cmd[512], buf[2048];
    FILE                *f;
    char                *p, *q;
    
    // getgrnam(groupName)->gr_mem[] only returns one entry, so we must use dscl
    sprintf(cmd, "dscl . -read /Groups/%s GroupMembership", groupName);
    f = popen(cmd, "r");
    if (f == NULL)
        return 0;
    
    while (PersistentFGets(buf, sizeof(buf), f))
    {
        p = buf;
        while (p) {
            p = strstr(p, userName);
            if (p) {
                q = p-1;
                p += strlen(userName);
                // Count only whole words (preceded and followed by white space) so 
                // that if we have both 'jon' and 'jones' we don't count 'jon' twice
                if (isspace(*q) && isspace(*p)) {
                    ++ count;
               }
            }
        }
    }
    
    return count;
}


// Find all visible users.
// If user is a member of group admin, add user to groups boinc_master and boinc_project.
// Optionally add non-admin users to group boinc_master but not to group boinc_project.
// Set login item for all members of group boinc_master to launch BOINC Manager.
// If our install package included a skin, set those user's preferences to use that skin.
// Optionally set BOINC as screensaver for all users running BOINC.
OSErr UpdateAllVisibleUsers(long brandID)
{
    passwd              *pw;
    vector<string>      human_user_names;
    vector<uid_t>       human_user_IDs;
    uid_t               saved_uid;
    Boolean             deleteLoginItem;
    char                human_user_name[256];
    char                skinName[256];
    char                s[256];
    Boolean             saverAlreadySetForAll = true;
    Boolean             setSaverForAllUsers = false;
    Boolean             allNonAdminUsersAreSet = true;
    Boolean             allowNonAdminUsersToRunBOINC = false;
    Boolean             found = false;
    FILE                *f;
    int                 err;
    Boolean             isAdminGroupMember, isBMGroupMember, isBPGroupMember;
    struct stat         sbuf;
#ifdef SANDBOX
    char                cmd[256];
    int                 BMGroupMembershipCount, BPGroupMembershipCount; 
    int                 i;
#endif
    int                 userIndex;
    char                buf[256];
    char                *p;
    int                 flag;
    
    FindSkinName(skinName, sizeof(skinName));

    // Step through all users
    puts("Beginning first pass through all users\n");
    fflush(stdout);

    f = popen("dscl . list /Users UniqueID", "r");
    if (f) {
        while (PersistentFGets(buf, sizeof(buf), f)) {
            p = strrchr(buf, ' ');
            if (p) {
                int id = atoi(p+1);
                if (id < 501) continue;
                human_user_IDs.push_back((uid_t)id);
            }
            p = strchr(buf, ' ');
            if (p) {
                *p = '\0';
                human_user_names.push_back(string(buf));
                *p = ' ';
            }
        }
        pclose(f);
    }
    
    for (userIndex=human_user_names.size(); userIndex>0; --userIndex) {
        flag = 0;
        strlcpy(human_user_name, human_user_names[userIndex-1].c_str(), sizeof(human_user_name));
        sprintf(cmd, "dscl . -read /Users/%s NFSHomeDirectory", human_user_name);    
        f = popen(cmd, "r");
        if (f) {
            while (PersistentFGets(buf, sizeof(buf), f)) {
                p = strrchr(buf, ' ');
                if (p) {
                    if (strcmp(p, " /var/empty\n") == 0) flag = 1;
                }
            }
            pclose(f);
        }

        sprintf(cmd, "dscl . -read /Users/%s UserShell", human_user_name);    
        f = popen(cmd, "r");
        if (f) {
            while (PersistentFGets(buf, sizeof(buf), f)) {
                p = strrchr(buf, ' ');
                if (p) {
                    if (strcmp(p, " /usr/bin/false\n") == 0) flag |= 2;
                }
            }
            pclose(f);
        }
        
        if (flag == 3) { // if (Home Directory == "/var/empty") && (UserShell == "/usr/bin/false")
            human_user_names.erase(human_user_names.begin()+userIndex-1);
            human_user_IDs.erase(human_user_IDs.begin()+userIndex-1);
        }
    }

    
    for (userIndex=0; userIndex< (int)human_user_names.size(); ++userIndex) {
        strlcpy(human_user_name, human_user_names[userIndex].c_str(), sizeof(human_user_name));
        printf("[1] Checking user %s\n", human_user_name);
        fflush(stdout);
            
        // getpwnam works with either the full / login name (pw->pw_gecos) 
        // or the short / Posix name (pw->pw_name)
        pw = getpwnam(human_user_name);
        if (pw == NULL) {           // "Deleted Users", "Shared", etc.
            printf("[1] %s not in getpwnam data base\n", human_user_name);
            fflush(stdout);
            continue;
        }

        printf("[1] User %s: Posix name=%s, Full name=%s\n", human_user_name, pw->pw_name, pw->pw_gecos);
        fflush(stdout);
        
#ifdef SANDBOX
        isAdminGroupMember = false;
        isBMGroupMember = false;
        
        isAdminGroupMember = IsUserMemberOfGroup(pw->pw_name, admin_group_name);
            if (isAdminGroupMember) {
            // User is a member of group admin, so add user to groups boinc_master and boinc_project
            printf("[1] User %s is a member of group admin\n", pw->pw_name);
            fflush(stdout);
        } else {
            isBMGroupMember = IsUserMemberOfGroup(pw->pw_name, boinc_master_group_name);
            if (isBMGroupMember) {
                // User is a member of group boinc_master
                printf("[1] Non-admin user %s is a member of group boinc_master\n", pw->pw_name);
                fflush(stdout);
            } else {
                allNonAdminUsersAreSet = false;
            }
        }
#else   // SANDBOX
        isGroupMember = true;
#endif  // SANDBOX
        if (isAdminGroupMember || isBMGroupMember) {
            if ((strcmp(loginName, human_user_name) == 0) 
                || (strcmp(loginName, pw->pw_name) == 0) 
                    || (strcmp(loginName, pw->pw_gecos) == 0)) {
                currentUserCanRunBOINC = true;
            }
            
            saved_uid = geteuid();
            seteuid(pw->pw_uid);                        // Temporarily set effective uid to this user
            
            if (OSVersion < 0x1060) {
                f = popen("defaults -currentHost read com.apple.screensaver moduleName", "r");
            } else {
                sprintf(s, "sudo -u %s defaults -currentHost read com.apple.screensaver moduleDict -dict", 
                        pw->pw_name); 
                f = popen(s, "r");
            }
            
            if (f) {
                found = false;
                while (PersistentFGets(s, sizeof(s), f)) {
                    if (strstr(s, saverName[brandID])) {
                        found = true;
                        break;
                    }
                }
                pclose(f);
                if (!found) {
                    saverAlreadySetForAll = false;
                }
            }
            
            seteuid(saved_uid);                         // Set effective uid back to privileged user
        }       // End if (isGroupMember)
    }           // End for (userIndex=0; userIndex< human_user_names.size(); ++userIndex)
    
    ResynchSystem();

    if (allNonAdminUsersAreSet) {
        puts("[2] All non-admin users are already members of group boinc_master\n");
        fflush(stdout);
    } else {
        if (gCommandLineInstall) {
            err = stat("/tmp/nonadminusersok.txt", &sbuf);
            if (err == noErr) {
                puts("nonadminusersok.txt file detected\n");
                fflush(stdout);
                unlink("/tmp/nonadminusersok.txt");
                allowNonAdminUsersToRunBOINC = true;
                currentUserCanRunBOINC = true;
                saverAlreadySetForAll = false;
            }
        } else {
            if (ShowMessage(true, 
                "Users who are permitted to administer this computer will automatically be allowed to "
                "run and control %s.\n\n"
                "Do you also want non-administrative users to be able to run and control %s on this Mac?",
                brandName[brandID], brandName[brandID])
            ) {
                allowNonAdminUsersToRunBOINC = true;
                currentUserCanRunBOINC = true;
                saverAlreadySetForAll = false;
                printf("[2] User answered Yes to allowing non-admin users to run %s\n", brandName[brandID]);
                fflush(stdout);
            } else {
                printf("[2] User answered No to allowing non-admin users to run %s\n", brandName[brandID]);
                fflush(stdout);
            }
        }
    }
    
    if (! saverAlreadySetForAll) {
        if (gCommandLineInstall) {
            err = stat("/tmp/setboincsaver.txt", &sbuf);
            if (err == noErr) {
                puts("setboincsaver.txt file detected\n");
                fflush(stdout);
                unlink("/tmp/setboincsaver.txt");
                setSaverForAllUsers = true;
            }
        } else {
            setSaverForAllUsers = ShowMessage(true, 
                    "Do you want to set %s as the screensaver for all %s users on this Mac?", 
                    brandName[brandID], brandName[brandID]);
        }
    }

    // Step through all users a second time, setting non-admin users and / or our screensaver
    puts("Beginning second pass through all users\n");
    fflush(stdout);

    for (userIndex=0; userIndex<(int)human_user_names.size(); ++userIndex) {
        strlcpy(human_user_name, human_user_names[userIndex].c_str(), sizeof(human_user_name));

        printf("[2] Checking user %s\n", human_user_name);
        fflush(stdout);
            
        pw = getpwnam(human_user_name);
        if (pw == NULL) {           // "Deleted Users", "Shared", etc.
            printf("[2] %s not in getpwnam data base\n", human_user_name);
            fflush(stdout);
            continue;
        }

        printf("[2] User %s: Posix name=%s, Full name=%s\n", human_user_name, pw->pw_name, pw->pw_gecos);
        fflush(stdout);
        
#ifdef SANDBOX
        isAdminGroupMember = false;
        isBMGroupMember = false;
        isBPGroupMember = false;

        isAdminGroupMember = IsUserMemberOfGroup(pw->pw_name, admin_group_name);
        if (isAdminGroupMember) {
            // User is a member of group admin, so add user to groups boinc_master and boinc_project
            printf("[2] User %s is a member of group admin\n", pw->pw_name);
            fflush(stdout);
        }

        // If allNonAdminUsersAreSet, some older BOINC versions added non-admin users only to group 
        // boinc_master; ensure all permitted BOINC users are also members of group boinc_project
        if (isAdminGroupMember || allowNonAdminUsersToRunBOINC || allNonAdminUsersAreSet) {
            // OS 10.7 dscl merge command has a bug that it adds the user to the group even if 
            // it was already a member, resulting in duplicate (multiple) entries.  Earlier BOINC 
            // versions did not check for this, so we remove duplicate entries if present.            
            BMGroupMembershipCount = CountGroupMembershipEntries(pw->pw_name, boinc_master_group_name);
            printf("[2] User %s found in group %s member list %d times\n", 
                        pw->pw_name, boinc_master_group_name, BMGroupMembershipCount);
            fflush(stdout);
            if (BMGroupMembershipCount == 0) {
                sprintf(cmd, "dscl . -merge /groups/%s GroupMembership %s", boinc_master_group_name, pw->pw_name);
                err = system(cmd);
                printf("[2] %s returned %d\n", cmd, err);
                fflush(stdout);
                isBMGroupMember = true;
            } else {
                isBMGroupMember = true;
                for (i=1; i<BMGroupMembershipCount; ++i) {
                    sprintf(cmd, "dscl . -delete /groups/%s GroupMembership %s", boinc_master_group_name, pw->pw_name);
                    err = system(cmd);
                    printf("[2] %s returned %d\n", cmd, err);
                    fflush(stdout);
                }
            }
            
            BPGroupMembershipCount = CountGroupMembershipEntries(pw->pw_name, boinc_project_group_name);
            printf("[2] User %s found in group %s member list %d times\n", 
                   pw->pw_name, boinc_project_group_name, BPGroupMembershipCount);
            fflush(stdout);
            if (BPGroupMembershipCount == 0) {
                sprintf(cmd, "dscl . -merge /groups/%s GroupMembership %s", boinc_project_group_name, pw->pw_name);
                err = system(cmd);
                printf("[2] %s returned %d\n", cmd, err);
                fflush(stdout);
                isBPGroupMember = true;
            } else {
                isBPGroupMember = true;
                for (i=1; i<BPGroupMembershipCount; ++i) {
                    sprintf(cmd, "dscl . -delete /groups/%s GroupMembership %s", boinc_project_group_name, pw->pw_name);
                    err = system(cmd);
                    printf("[2] %s returned %d\n", cmd, err);
                    fflush(stdout);
                }
            }
        }
#else   // SANDBOX
        isBMGroupMember = true;
#endif  // SANDBOX
        saved_uid = geteuid();
        deleteLoginItem = CheckDeleteFile(human_user_name);
        if (CheckDeleteFile(pw->pw_name)) {
            deleteLoginItem = true;
        }
        if (CheckDeleteFile(pw->pw_gecos)) {
            deleteLoginItem = true;
        }
        if (!isBMGroupMember) {
            deleteLoginItem = true;
        }

        // Set login item for this user
        if (OSVersion >= 0x1070) {
            // LoginItemAPI.c does not set hidden property for login items
            // under OS 10.7.0, so use AppleScript instead to prevent Lion 
            // from opening BOINC windows at system startup.  This was 
            // apparently fixed in OS 10.7.1.
            // LoginItemAPI.c does not work at all under OS 10.8 Preview 3 
            // but this version of SetLoginItemOSAScript works for OS 10.7.0 
            // and later, so we use it for OS 10.7.0 and later.
            printf("[2] calling SetLoginItemOSAScript for user %s, euid = %d, deleteLoginItem = %d\n", 
                pw->pw_name, geteuid(), deleteLoginItem);
            fflush(stdout);

            SetLoginItemOSAScript(brandID, deleteLoginItem, pw->pw_name);
        } else {
            seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
            printf("[2] calling SetLoginItemAPI for user %s, euid = %d, deleteLoginItem = %d\n", 
                    pw->pw_name, geteuid(), deleteLoginItem);
            fflush(stdout);
            SetLoginItemAPI(brandID, deleteLoginItem);
            seteuid(saved_uid);     // Set effective uid back to privileged user
        }
        
        if (isBMGroupMember) {
            // For some reason we need to call getpwnam again on OS 10.5
            pw = getpwnam(human_user_name);
            if (pw == NULL) {           // "Deleted Users", "Shared", etc.
                printf("[2] ERROR: %s was in getpwnam data base but now is not!\n", human_user_name);
                fflush(stdout);
                continue;
            }
            SetSkinInUserPrefs(pw->pw_name, skinName);
        
            if (setSaverForAllUsers) {
                if (OSVersion < 0x1060) {
                    seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
                    sprintf(s, "defaults -currentHost write com.apple.screensaver moduleName %s", saverNameEscaped[brandID]);
                    system (s);
                    sprintf(s, "defaults -currentHost write com.apple.screensaver modulePath /Library/Screen\\ Savers/%s.saver", 
                                saverNameEscaped[brandID]);
                    seteuid(saved_uid);     // Set effective uid back to privileged user
                } else {
                    // We must leave effective user ID as privileged user (root) 
                    // because the target user may not be in the sudoers file.
                     sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver moduleDict -dict moduleName %s path /Library/Screen\\ Savers/%s.saver", 
                            pw->pw_name, saverNameEscaped[brandID], saverNameEscaped[brandID]);
                }
                system (s);
            }
        }
    }   // End for (userIndex=0; userIndex< human_user_names.size(); ++userIndex)

    ResynchSystem();

    return noErr;
}


void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
//    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
}


long GetBrandID()
{
    long iBrandId;

    iBrandId = 0;   // Default value
    
    FILE *f = fopen("Contents/Resources/Branding", "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandId);
        fclose(f);
    }
    
    return iBrandId;
}


int TestRPCBind()
{
    sockaddr_in addr;
    int lsock;
    int retval;
    
    lsock = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0)
        return -153;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(31416);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int one = 1;
    retval = setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, (char*)&one, 4);

    if (! retval)
        retval = bind(lsock, (const sockaddr*)(&addr), (socklen_t)sizeof(addr));

    if (! retval)
        retval = listen(lsock, 999);
    
    close(lsock);
    
    return retval;
}


static OSStatus ResynchSystem() {
    OSStatus        err = noErr;
    
    if (OSVersion >= 0x1050) {
        // OS 10.5
        err = system("dscacheutil -flushcache");
        err = system("dsmemberutil flushcache");
        return noErr;
    }
    
    err = system("lookupd -flushcache");
    
    if (OSVersion >= 0x1040)
        err = system("memberd -r");           // Available only in OS 10.4
    
    return noErr;
}


// ---------------------------------------------------------------------------
/* This runs through the process list looking for the indicated application */
/*  Searches for process by file type and signature (creator code)          */
// ---------------------------------------------------------------------------
OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN)
{
    ProcessInfoRec tempInfo;
    FSSpec procSpec;
    Str31 processName;
    OSErr myErr = noErr;
    /* null out the PSN so we're starting at the beginning of the list */
    processSN->lowLongOfPSN = kNoProcess;
    processSN->highLongOfPSN = kNoProcess;
    /* initialize the process information record */
    tempInfo.processInfoLength = sizeof(ProcessInfoRec);
    tempInfo.processName = processName;
    tempInfo.processAppSpec = &procSpec;
    /* loop through all the processes until we */
    /* 1) find the process we want */
    /* 2) error out because of some reason (usually, no more processes) */
    do {
        myErr = GetNextProcess(processSN);
        if (myErr == noErr)
            GetProcessInformation(processSN, &tempInfo);
    }
            while ((tempInfo.processSignature != creatorToFind || tempInfo.processType != typeToFind) &&
                   myErr == noErr);
    return(myErr);
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


int FindSkinName(char *name, size_t len)
{
    FILE *f;
    char buf[MAXPATHLEN];
    char *pattern = "/BOINC Data/skins/";
    char *p, *q;

    name[0] = '\0';
    
    f = popen("lsbom -d -s ./Contents/Archive.bom", "r");
    if (f == NULL)
        return 0;
    
    while (PersistentFGets(buf, sizeof(buf), f)) {
        p = strstr(buf, pattern);
        if (p) {
            p += strlen(pattern);
            q = strchr(p, '/');
            if (q) *q = 0;
            q = strchr(p, '\n');
            if (q) *q = 0;

            if (strlen(p) > (len-1))
                return 0;
            strlcpy(name, p, len);
            pclose(f);
            return 1;
        }
    }
    pclose(f);
    return 0;
}



pid_t FindProcessPID(char* name, pid_t thePID)
{
    FILE *f;
    char buf[1024];
    size_t n = 0;
    pid_t aPID;
    
    if (name != NULL)     // Search ny name
        n = strlen(name);
    
    f = popen("ps -a -x -c -o command,pid", "r");
    if (f == NULL)
        return 0;
    
    while (PersistentFGets(buf, sizeof(buf), f))
    {
        if (name != NULL) {     // Search by name
            if (strncmp(buf, name, n) == 0)
            {
                aPID = atol(buf+16);
                pclose(f);
                return aPID;
            }
        } else {      // Search by PID
            aPID = atol(buf+16);
            if (aPID == thePID) {
                pclose(f);
                return aPID;
            }
        }
    }
    pclose(f);
    return 0;
}


static OSErr QuitOneProcess(OSType signature) {
    bool                done = false;
    ProcessSerialNumber thisPSN;
    ProcessInfoRec		thisPIR;
    OSErr               err = noErr;
    Str63               thisProcessName;
    AEAddressDesc		thisPSNDesc;
    AppleEvent			thisQuitEvent, thisReplyEvent;
    

    thisPIR.processInfoLength = sizeof (ProcessInfoRec);
    thisPIR.processName = thisProcessName;
    thisPIR.processAppSpec = nil;
    
    thisPSN.highLongOfPSN = 0;
    thisPSN.lowLongOfPSN = kNoProcess;
    
    while (done == false) {		
        err = GetNextProcess(&thisPSN);
        if (err == procNotFound) {	
            done = true;		// Finished stepping through all running applications.
            err = noErr;        // Success
        } else {		
            err = GetProcessInformation(&thisPSN,&thisPIR);
            if (err != noErr)
                goto bail;
                    
            if (thisPIR.processSignature == signature) {	// is it our target process?
                err = AECreateDesc(typeProcessSerialNumber, (Ptr)&thisPSN,
                                            sizeof(thisPSN), &thisPSNDesc);
                if (err != noErr)
                    goto bail;

                // Create the 'quit' Apple event for this process.
                err = AECreateAppleEvent(kCoreEventClass, kAEQuitApplication, &thisPSNDesc,
                                                kAutoGenerateReturnID, kAnyTransactionID, &thisQuitEvent);
                if (err != noErr) {
                    AEDisposeDesc (&thisPSNDesc);
                    goto bail;		// don't know how this could happen, but limp gamely onward
                }

                // send the event 
                err = AESend(&thisQuitEvent, &thisReplyEvent, kAEWaitReply,
                                           kAENormalPriority, kAEDefaultTimeout, 0L, 0L);
                AEDisposeDesc (&thisQuitEvent);
                AEDisposeDesc (&thisPSNDesc);

                if (err != noErr)
                    goto bail;
#if 0
                if (err == errAETimeout) {
                    pid_t thisPID;
                        
                    err = GetProcessPID(&thisPSN , &thisPID);
                    if (err == noErr)
                        err = kill(thisPID, SIGKILL);
                }
#endif
                continue;		// There can be multiple instances of the Manager
            }
        }
    }

bail:
    return err;
}


static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;
    
    return noErr;
}

void strip_cr(char *buf)
{
    char *theCR;

    theCR = strrchr(buf, '\n');
    if (theCR)
        *theCR = '\0';
    theCR = strrchr(buf, '\r');
    if (theCR)
        *theCR = '\0';
}

// For debugging
void print_to_log_file(const char *format, ...) {
#if CREATE_LOG
    FILE *f;
    va_list args;
    char path[256], buf[256];
    time_t t;
    strcpy(path, "/Users/Shared/test_log.txt");
//    strcpy(path, "/Users/");
//    strcat(path, getlogin());
//    strcat(path, "/Documents/test_log.txt");
    f = fopen(path, "a");
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
    chmod(path, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

#endif
}

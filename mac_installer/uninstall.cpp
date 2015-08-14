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

/*  uninstall.cpp */

#define TESTING 0    /* for debugging */
    
#include <Carbon/Carbon.h>
#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

#include <grp.h>

#include <unistd.h>	// geteuid, seteuid
#include <pwd.h>	// passwd, getpwnam
#include <dirent.h>
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>   // For stat()
#include <string.h>
#include <vector>
#include <string>

using std::vector;
using std::string;

#define SEARCHFORALLBOINCMANAGERS 0
#define MAX_LANGUAGES_TO_TRY 5

#include "translate.h"

static OSStatus DoUninstall(void);
static OSStatus CleanupAllVisibleUsers(void);
static OSStatus DeleteOurBundlesFromDirectory(CFStringRef bundleID, char *extension, char *dirPath);
static OSStatus GetAuthorization(AuthorizationRef * authRef, const char *pathToTool, char *brandName);
static OSStatus DoPrivilegedExec(char *brandName, const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5);
static void DeleteLoginItemOSAScript(char* user, char* appName);
static void DeleteLoginItemFromPListFile(void);
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
OSErr GetCurrentScreenSaverSelection(char *moduleName, size_t maxLen);
OSErr SetScreenSaverSelection(char *moduleName, char *modulePath, int type);
int GetCountOfLoginItemsFromPlistFile(void);
OSErr GetLoginItemNameAtIndexFromPlistFile(int index, char *name, size_t maxLen);
OSErr DeleteLoginItemNameAtIndexFromPlistFile(int index);
static OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN);
static pid_t FindProcessPID(char* name, pid_t thePID);
static OSStatus QuitOneProcess(OSType signature);
static void KillVBoxProcess(char* name);
static void SleepTicks(UInt32 ticksToSleep);
static void GetPreferredLanguages();
static void LoadPreferredLanguages();
static Boolean ShowMessage(Boolean allowCancel, Boolean continueButton, Boolean yesNoButtons, const char *format, ...);
#if SEARCHFORALLBOINCMANAGERS
static OSStatus GetpathToBOINCManagerApp(char* path, int maxLen, FSRef *theFSRef);
#endif
static int compareOSVersionTo(int toMajor, int toMinor);

static char gAppName[256];
static char gBrandName[256];
static char gCatalogsDir[MAXPATHLEN];
static char * gCatalog_Name = (char *)"BOINC-Setup";
static char * gTempFileName = "/tmp/BOINC_preferred_languages";


/* BEGIN TEMPORARY ITEMS TO ALLOW TRANSLATORS TO START WORK */
void notused() {
    ShowMessage(true, false, false, (char *)_("OK"));
}
/* END TEMPORARY ITEMS TO ALLOW TRANSLATORS TO START WORK */


int main(int argc, char *argv[])
{
    char                        pathToSelf[MAXPATHLEN], pathToVBoxUninstallTool[MAXPATHLEN], *p;
    ProcessSerialNumber         ourPSN;
    FSRef                       ourFSRef;
    Boolean                     cancelled = false;
    struct stat                 sbuf;
    OSStatus                    err = noErr;

    // Get the full path to our executable inside this application's bundle
    err = GetCurrentProcess (&ourPSN);
    if (err == noErr) {
        err = GetProcessBundleLocation(&ourPSN, &ourFSRef);
    }
    if (err == noErr) {
        err = FSRefMakePath (&ourFSRef, (UInt8*)pathToSelf, sizeof(pathToSelf));
    }
    if (err != noErr) {
        ShowMessage(false, false, false, "Couldn't get path to self.  Error %d", err);
        return err;
    }

    strlcpy(pathToVBoxUninstallTool, pathToSelf, sizeof(pathToVBoxUninstallTool));
    strlcat(pathToVBoxUninstallTool, "/Contents/Resources/VirtualBox_Uninstall.tool", sizeof(pathToVBoxUninstallTool));

    // To allow for branding, assume name of executable inside bundle is same as name of bundle
    p = strrchr(pathToSelf, '/');         // Assume name of executable inside bundle is same as name of bundle
    if (p == NULL)
        p = pathToSelf - 1;
    strlcpy(gAppName, p+1, sizeof(gAppName));
    p = strrchr(gAppName, '.');         // Strip off bundle extension (".app")
    if (p)
        *p = '\0'; 

    strlcpy(gCatalogsDir, pathToSelf, sizeof(gCatalogsDir));
    strlcat(gCatalogsDir, "/Contents/Resources/locale/", sizeof(gCatalogsDir));
    
    strlcat(pathToSelf, "/Contents/MacOS/", sizeof(pathToSelf));
    strlcat(pathToSelf, gAppName, sizeof(pathToSelf));

    p = strchr(gAppName, ' ');
    p += 1;         // Point to brand name following "Uninstall "
    strlcpy(gBrandName, p, sizeof(gBrandName));
        
    // Determine whether this is the intial launch or the relaunch with privileges
    if ( (argc == 2) && (strcmp(argv[1], "--privileged") == 0) ) {
        LoadPreferredLanguages();
        
        if (geteuid() != 0) {        // Confirm that we are running as root
            ShowMessage(false, false, false, (char *)_("Permission error after relaunch"));
            BOINCTranslationCleanup();
            return permErr;
        }
        
        ShowMessage(false, true, false, (char *)_("Removal may take several minutes.\nPlease be patient."));
        
        err = DoUninstall();
        
        BOINCTranslationCleanup();

        printf("Done\n");   // Signal DoPrivilegedExec() that we finished
        return err;
    }

    // This is the initial launch.  Authenticate and relaunch ourselves with privileges.

    GetPreferredLanguages();    // We must do this before switching to root user
    LoadPreferredLanguages();
    
    // Grid Republic uses generic dialog with Uninstall application's icon
    cancelled = ! ShowMessage(true, true, false, (char *)_(
            "Are you sure you want to completely remove %s from your computer?\n\n"
            "This will remove the executables but will not touch %s data files."), p, p);
    if (! cancelled) {
        err = DoPrivilegedExec(p, pathToSelf, "--privileged", NULL, NULL, NULL, NULL);
    }
    
    if (cancelled || (err == errAuthorizationCanceled)) {
        ShowMessage(false, false, false, (char *)_("Canceled: %s has not been touched."), p);
        BOINCTranslationCleanup();
        return err;
    }


#if TESTING
    ShowMessage(false, false, false, "DoPrivilegedExec returned %d", err);
#endif

    if (err) {
        ShowMessage(false, false, false, (char *)_("An error occurred: error code %d"), err);
    }else {
        FSRef BOINCDataRef, UserPrefsRef;
        CFStringRef CFBOINCDataPath, CFUserPrefsPath;
        char BOINCDataPath[MAXPATHLEN], temp[MAXPATHLEN], PathToPrefs[MAXPATHLEN];
        Boolean success = false;
        char * loginName = getlogin();
        
        err = FSPathMakeRef((StringPtr)"/Library", &BOINCDataRef, NULL);
        if (err == noErr) {
            err = LSCopyDisplayNameForRef(&BOINCDataRef, &CFBOINCDataPath);
        }
        if (err == noErr) {
            success = CFStringGetCString(CFBOINCDataPath, temp,
                        sizeof(temp), kCFStringEncodingUTF8);
        }
        if (success) {
            success = false;
            strlcpy(BOINCDataPath, "/", sizeof(BOINCDataPath));
            strlcat(BOINCDataPath, temp, sizeof(BOINCDataPath));
            strlcat(BOINCDataPath, "/", sizeof(BOINCDataPath));
            
            err = FSPathMakeRef((StringPtr)"/Library/Application Support", &BOINCDataRef, NULL);
            if (err == noErr) {
                err = LSCopyDisplayNameForRef(&BOINCDataRef, &CFBOINCDataPath);
            }
            if (err == noErr) {
                success = CFStringGetCString(CFBOINCDataPath, temp,
                            sizeof(temp), kCFStringEncodingUTF8);
            }
        }
        if (success) {
            strlcat(BOINCDataPath, temp, sizeof(BOINCDataPath));
            strlcat(BOINCDataPath, "/BOINC Data", sizeof(BOINCDataPath));
        } else {
            strlcpy(BOINCDataPath,
                    "/Library/Application Support/BOINC Data",
                    sizeof(BOINCDataPath));
        }
        
        success = false;
                
//        err = FSPathMakeRef((StringPtr)"/Users", &UserPrefsRef, NULL);
        err = FSPathMakeRef((StringPtr)"/Users", &UserPrefsRef, NULL);
        if (err == noErr) {
            err = LSCopyDisplayNameForRef(&UserPrefsRef, &CFUserPrefsPath);
        }
        if (err == noErr) {
            success = CFStringGetCString(CFUserPrefsPath, temp,
                        sizeof(temp), kCFStringEncodingUTF8);
        }
        if (success) {
            success = false;
            strlcpy(PathToPrefs, "/", sizeof(PathToPrefs));
            strlcat(PathToPrefs, temp, sizeof(PathToPrefs));
            strlcat(PathToPrefs, "/[", sizeof(PathToPrefs));
            strlcat(PathToPrefs, (char *)_("name  of user"), sizeof(PathToPrefs));
            strlcat(PathToPrefs, "]/", sizeof(PathToPrefs));
            
            sprintf(temp, "/Users/%s/Library", loginName);
            err = FSPathMakeRef((StringPtr)temp, &UserPrefsRef, NULL);
            if (err == noErr) {
                err = LSCopyDisplayNameForRef(&UserPrefsRef, &CFUserPrefsPath);
            }
            if (err == noErr) {
                success = CFStringGetCString(CFUserPrefsPath, temp,
                            sizeof(temp), kCFStringEncodingUTF8);
            }
        }
        if (success) {
            success = false;
            strlcat(PathToPrefs, temp, sizeof(PathToPrefs));
            strlcat(PathToPrefs, "/", sizeof(PathToPrefs));
            
            sprintf(temp, "/Users/%s/Library/Preferences", loginName);
            err = FSPathMakeRef((StringPtr)temp, &UserPrefsRef, NULL);
            if (err == noErr) {
                err = LSCopyDisplayNameForRef(&UserPrefsRef, &CFUserPrefsPath);
            }
            if (err == noErr) {
                success = CFStringGetCString(CFUserPrefsPath, temp,
                            sizeof(temp), kCFStringEncodingUTF8);
            }
        }
        if (success) {
            strlcat(PathToPrefs, temp, sizeof(PathToPrefs));
            strlcat(PathToPrefs, "/BOINC Manager Preferences", sizeof(PathToPrefs));
        } else {
            strlcpy(PathToPrefs,
                    "/Users/[username]/Library/Preferences/BOINC Manager Preferences",
                    sizeof(PathToPrefs));
        }

        // stat() returns zero on success
        if (stat(pathToVBoxUninstallTool, &sbuf) == 0) {
            char cmd[MAXPATHLEN+30];

            cancelled = ! ShowMessage(true, false, true, (char *)_(
                "Do you also want to remove VirtualBox from your computer?\n"
                "(VirtualBox was installed along with BOINC.)"));
            if (! cancelled) {
                err = QuitOneProcess('VBOX');
                if (err != procNotFound) {
                    sleep(5);
                }
                // List of processes to kill taken from my_processes
                // array in VirtualBox_Uninstall.tool script:
                KillVBoxProcess("VirtualBox");
                KillVBoxProcess("VirtualBox-amd64");
                KillVBoxProcess("VirtualBox-x86");
                KillVBoxProcess("VirtualBoxVM");
                KillVBoxProcess("VirtualBoxVM-amd64");
                KillVBoxProcess("VirtualBoxVM-x86");
                KillVBoxProcess("VBoxManage");
                KillVBoxProcess("VBoxManage-amd64");
                KillVBoxProcess("VBoxManage-x86");
                KillVBoxProcess("VBoxHeadless");
                KillVBoxProcess("VBoxHeadless-amd64");
                KillVBoxProcess("VBoxHeadless-x86");
                KillVBoxProcess("vboxwebsrv");
                KillVBoxProcess("vboxwebsrv-amd64");
                KillVBoxProcess("vboxwebsrv-x86");
                KillVBoxProcess("VBoxXPCOMIPCD");
                KillVBoxProcess("VBoxXPCOMIPCD-amd64");
                KillVBoxProcess("VBoxXPCOMIPCD-x86");
                KillVBoxProcess("VBoxSVC");
                KillVBoxProcess("VBoxSVC-amd64");
                KillVBoxProcess("VBoxSVC-x86");
                KillVBoxProcess("VBoxNetDHCP");
                KillVBoxProcess("VBoxNetDHCP-amd64");
                KillVBoxProcess("VBoxNetDHCP-x86");
                sleep(2);
                
                snprintf(cmd, sizeof(cmd), "source \"%s\" --unattended", pathToVBoxUninstallTool);
                system(cmd);
            }
        }
        
        ShowMessage(false, false, false, (char *)_("Removal completed.\n\n You may want to remove the following remaining items using the Finder: \n"
         "the directory \"%s\"\n\nfor each user, the file\n"
         "\"%s\"."), BOINCDataPath, PathToPrefs);
    }
    
    BOINCTranslationCleanup();
    return err;
}


static OSStatus DoUninstall(void) {
    pid_t                   coreClientPID = 0;
    pid_t                   BOINCManagerPID = 0;
    char                    cmd[1024];
    char                    *p;
    passwd                  *pw;
    OSStatus                err = noErr;
#if SEARCHFORALLBOINCMANAGERS
    char                    myRmCommand[MAXPATHLEN+10], plistRmCommand[MAXPATHLEN+10];
    char                    notBoot[] = "/Volumes/";
    FSRef                   theFSRef;
    int                     pathOffset, i;
#endif

#if TESTING
    ShowMessage(false, false, false, "Permission OK after relaunch");
#endif

    //TODO: It would be nice to get the app name from the bundle ID or signature
    // so we don't have to try all 4 and to allow for future branded versions

    for (;;) {
        BOINCManagerPID = FindProcessPID("BOINCManager", 0);
        if (BOINCManagerPID == 0) break;
        kill(BOINCManagerPID, SIGTERM);
        sleep(2);
    }
    
    for (;;) {
        BOINCManagerPID = FindProcessPID("GridRepublic Desktop", 0);
        if (BOINCManagerPID == 0) break;
        kill(BOINCManagerPID, SIGTERM);
        sleep(2);
    }
    
    for (;;) {
        BOINCManagerPID = FindProcessPID("Progress Thru Processors Desktop", 0);
        if (BOINCManagerPID == 0) break;
        kill(BOINCManagerPID, SIGTERM);
        sleep(2);
    }
    
    for (;;) {
        BOINCManagerPID = FindProcessPID("Charity Engine Desktop", 0);
        if (BOINCManagerPID == 0) break;
        kill(BOINCManagerPID, SIGTERM);
        sleep(2);
    }

    // Core Client may still be running if it was started without Manager
    coreClientPID = FindProcessPID("boinc", 0);
    if (coreClientPID)
        kill(coreClientPID, SIGTERM);   // boinc catches SIGTERM & exits gracefully

#if SEARCHFORALLBOINCMANAGERS
    // Phase 1: try to find all our applications using LaunchServices
    for (i=0; i<100; i++) {
        strlcpy(myRmCommand, "rm -rf \"", 10);
        pathOffset = strlen(myRmCommand);
    
        err = GetpathToBOINCManagerApp(myRmCommand+pathOffset, MAXPATHLEN, &theFSRef);
        if (err)
            break;
        
        strlcat(myRmCommand, "\"", sizeof(myRmCommand));
    
#if TESTING
        ShowMessage(false, false, false, "manager: %s", myRmCommand);
#endif

        p = strstr(myRmCommand, notBoot);
        
        if (p == myRmCommand+pathOffset) {
#if TESTING
            ShowMessage(false, false, false, "Not on boot volume: %s", myRmCommand);
#endif
            break;
        } else {

            // First delete just the application's info.plist file and update the 
            // LaunchServices Database; otherwise LSFindApplicationForInfo might 
            // return this application again after it's been deleted.
            strlcpy(plistRmCommand, myRmCommand, sizeof(plistRmCommand));
            strlcat(plistRmCommand, "/Contents/info.plist", sizeof(plistRmCommand));
#if TESTING
        ShowMessage(false, false, false, "Deleting info.plist: %s", plistRmCommand);
#endif
            system(plistRmCommand); 
            err = LSRegisterFSRef(&theFSRef, true);
#if TESTING
            if (err)
                ShowMessage(false, false, false, "LSRegisterFSRef returned error %d", err);
#endif
            system(myRmCommand);
        }
    }
#endif

    // Phase 2: step through default Applications directory searching for our applications
    err = DeleteOurBundlesFromDirectory(CFSTR("edu.berkeley.boinc"), "app", "/Applications");

    // Phase 3: step through default Screen Savers directory searching for our screen savers
    err = DeleteOurBundlesFromDirectory(CFSTR("edu.berkeley.boincsaver"), "saver", "/Library/Screen Savers");

    // Phase 4: Delete our files and directories at our installer's default locations
    // Remove everything we've installed, whether BOINC, GridRepublic, Progress Thru Processors or
    // Charity Engine
    
    //TODO: It would be nice to get the app name from the bundle ID or signature
    // so we don't have to try all 4 and to allow for future branded versions
    
    // These first 4 should already have been deleted by the above code, but do them anyway for safety
    system ("rm -rf /Applications/BOINCManager.app");
    system ("rm -rf \"/Library/Screen Savers/BOINCSaver.saver\"");
    
    system ("rm -rf \"/Applications/GridRepublic Desktop.app\"");
    system ("rm -rf \"/Library/Screen Savers/GridRepublic.saver\"");
    
    system ("rm -rf \"/Applications/Progress Thru Processors Desktop.app\"");
    system ("rm -rf \"/Library/Screen Savers/Progress Thru Processors.saver\"");
    
    system ("rm -rf \"/Applications/Charity Engine Desktop.app\"");
    system ("rm -rf \"/Library/Screen Savers/Charity Engine.saver\"");
    
    // Delete any receipt from an older installer (which had 
    // a wrapper application around the installer package.)
    system ("rm -rf /Library/Receipts/GridRepublic.pkg");
    system ("rm -rf /Library/Receipts/Progress\\ Thru\\ Processors.pkg");
    system ("rm -rf /Library/Receipts/Charity\\ Engine.pkg");
    system ("rm -rf /Library/Receipts/BOINC.pkg");

    // Delete any receipt from a newer installer (a bare package.) 
    system ("rm -rf /Library/Receipts/GridRepublic\\ Installer.pkg");
    system ("rm -rf /Library/Receipts/Progress\\ Thru\\ Processors\\ Installer.pkg");
    system ("rm -rf /Library/Receipts/Charity\\ Engine\\ Installer.pkg");
    system ("rm -rf /Library/Receipts/BOINC\\ Installer.pkg");

    // Phase 5: Set BOINC Data owner and group to logged in user
    // We don't customize BOINC Data directory name for branding
//    system ("rm -rf \"/Library/Application Support/BOINC Data\"");
    p = getlogin();
    pw = getpwnam(p);
    sprintf(cmd, "chown -R %d:%d \"/Library/Application Support/BOINC Data\"", pw->pw_uid, pw->pw_gid);
    system (cmd);
    system("chmod -R u+rw-s,g+r-w-s,o+r-w \"/Library/Application Support/BOINC Data\"");
    system("chmod 600 \"/Library/Application Support/BOINC Data/gui_rpc_auth.cfg\"");
    
    // Phase 6: step through all users and do user-specific cleanup
    CleanupAllVisibleUsers();
    
    system ("dscl . -delete /users/boinc_master");
    system ("dscl . -delete /groups/boinc_master");
    system ("dscl . -delete /users/boinc_project");
    system ("dscl . -delete /groups/boinc_project");


    return 0;
}


static OSStatus DeleteOurBundlesFromDirectory(CFStringRef bundleID, char *extension, char *dirPath) {
    DIR                     *dirp;
    dirent                  *dp;
    CFStringRef             urlStringRef = NULL;
    int                     index;
    CFStringRef             thisID = NULL;
    CFBundleRef             thisBundle = NULL;
    CFURLRef                bundleURLRef = NULL;
    char                    myRmCommand[MAXPATHLEN+10], *p;
    int                     pathOffset;

    dirp = opendir(dirPath);
    if (dirp == NULL) {      // Should never happen
        ShowMessage(false, false, false, "Error: opendir(\"%s\") failed", dirPath);
        return -1;
    }
    
    index = -1;
    while (true) {
        index++;
        
        dp = readdir(dirp);
        if (dp == NULL)
            break;                  // End of list

        p = strrchr(dp->d_name, '.');
        if (p == NULL)
            continue;

        if (strcmp(p+1, extension))
            continue;
        
        strlcpy(myRmCommand, "rm -rf \"", 10);
        pathOffset = strlen(myRmCommand);
        strlcat(myRmCommand, dirPath, sizeof(myRmCommand));
        strlcat(myRmCommand, "/", sizeof(myRmCommand));
        strlcat(myRmCommand, dp->d_name, sizeof(myRmCommand));
        urlStringRef = CFStringCreateWithCString(kCFAllocatorDefault, myRmCommand+pathOffset, CFStringGetSystemEncoding());
        if (urlStringRef) {
            bundleURLRef = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, urlStringRef, kCFURLPOSIXPathStyle, false);
            if (bundleURLRef) {
                thisBundle = CFBundleCreate( kCFAllocatorDefault, bundleURLRef );
                if (thisBundle) {
                    thisID = CFBundleGetIdentifier(thisBundle);
                        if (thisID) {
                            strlcat(myRmCommand, "\"", sizeof(myRmCommand));
                            if (CFStringCompare(thisID, bundleID, 0) == kCFCompareEqualTo) {
#if TESTING
                                ShowMessage(false, false, false, "Bundles: %s", myRmCommand);
#endif

                               system(myRmCommand);
                            } else {
#if TESTING
//                                ShowMessage(false, false, false, "Bundles: Not deleting %s", myRmCommand+pathOffset);
#endif
                            }

                
                        } // if (thisID)
#if TESTING
                        else
                            ShowMessage(false, false, false, "CFBundleGetIdentifier failed for index %d", index);                                
#endif
                        CFRelease(thisBundle);                
               } //if (thisBundle)
#if TESTING
                        else
                            ShowMessage(false, false, false, "CFBundleCreate failed for index %d", index);
#endif
                CFRelease(bundleURLRef);                
            } // if (bundleURLRef)
#if TESTING
        else
            ShowMessage(false, false, false, "CFURLCreateWithFileSystemPath failed");
#endif
        
            CFRelease(urlStringRef);
        } // if (urlStringRef)
#if TESTING
        else
            ShowMessage(false, false, false, "CFStringCreateWithCString failed");
#endif
    }   // while true
    
    closedir(dirp);

    return noErr;
}


enum {
	kSystemEventsCreator = 'sevs'
};


// Find all visible users and delete their login item to launch BOINC Manager.
// Remove each user from groups boinc_master and boinc_project.
// For now, don't delete user's BOINC Preferences file.
static OSStatus CleanupAllVisibleUsers(void)
{
    passwd              *pw;
    vector<string>      human_user_names;
    vector<uid_t>       human_user_IDs;
    uid_t               saved_uid, saved_euid;
    char                human_user_name[256];
    int                 i;
    int                 userIndex;
    int                 flag;
    char                buf[256];
    char                s[1024];
    char                cmd[2048];
    char                systemEventsPath[1024];
    ProcessSerialNumber SystemEventsPSN;
	FSRef               appRef;
    FILE                *f;
    char                *p;
    int                 id;
    OSStatus            err;
    Boolean             changeSaver;

    saved_uid = getuid();
    saved_euid = geteuid();

    // First, find all users on system
    f = popen("dscl . list /Users UniqueID", "r");
    if (f) {
        while (PersistentFGets(buf, sizeof(buf), f)) {
            p = strrchr(buf, ' ');
            if (p) {
                id = atoi(p+1);
                if (id < 501) {
#if TESTING
//                    printf("skipping user ID %d\n", id);
//                    fflush(stdout);
#endif
                    continue;
                }
                
                while (p > buf) {
                    if (*p != ' ') break;
                    --p;
                }

                *(p+1) = '\0';
                human_user_names.push_back(string(buf));
                human_user_IDs.push_back((uid_t)id);
#if TESTING
                ShowMessage(false, false, false, "user ID %d: %s\n", id, buf);
#endif
                *(p+1) = ' ';
            }
        }
        pclose(f);
    }
    
    for (userIndex=human_user_names.size(); userIndex>0; --userIndex) {
        flag = 0;
        strlcpy(human_user_name, human_user_names[userIndex-1].c_str(), sizeof(human_user_name));

        // Check whether this user is a login (human) user 
        sprintf(s, "dscl . -read \"/Users/%s\" NFSHomeDirectory", human_user_name);    
        f = popen(s, "r");
        if (f) {
            while (PersistentFGets(buf, sizeof(buf), f)) {
                p = strrchr(buf, ' ');
                if (p) {
                    if (strstr(p, "/var/empty") != NULL) flag = 1;
                }
            }
            pclose(f);
        }

        sprintf(s, "dscl . -read \"/Users/%s\" UserShell", human_user_name);    
        f = popen(s, "r");
        if (f) {
            while (PersistentFGets(buf, sizeof(buf), f)) {
                p = strrchr(buf, ' ');
                if (p) {
                    if (strstr(p, "/usr/bin/false") != NULL) flag |= 2;
                }
            }
            pclose(f);
        }
        
        // Skip all non-human (non-login) users
        if (flag == 3) { // if (Home Directory == "/var/empty") && (UserShell == "/usr/bin/false")
#if TESTING
            ShowMessage(false, false, false, "Flag=3: skipping user ID %d: %s", human_user_IDs[userIndex-1], buf);
#endif
            continue;
        }

        pw = getpwnam(human_user_name);
        if (pw == NULL)
            continue;

#if TESTING
        ShowMessage(false, false, false, "Deleting login item for user %s: Posix name=%s, Full name=%s, UID=%d",
                human_user_name, pw->pw_name, pw->pw_gecos, pw->pw_uid);
#endif

        // Remove user from groups boinc_master and boinc_project
        sprintf(s, "dscl . -delete /groups/boinc_master users \"%s\"", human_user_name);
        system (s);

        sprintf(s, "dscl . -delete /groups/boinc_project users \"%s\"", human_user_name);
        system (s);

#if TESTING
//    ShowMessage(false, false, false, "Before seteuid(%d) for user %s, euid = %d", pw->pw_uid, human_user_name, geteuid());
#endif
        setuid(0);
        // Delete our login item(s) for this user
       if (compareOSVersionTo(10, 8) >= 0) {
            seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
            DeleteLoginItemFromPListFile();
            seteuid(saved_euid);    // Set effective uid back to privileged user
        } else {            // OS 10.7.x
            // We must leave effective user ID as privileged user (root) 
            // because the target user may not be in the sudoers file.

            // We must launch the System Events application for the target user

#if TESTING
            ShowMessage(false, false, false, "Telling System Events to quit (before DeleteLoginItemOSAScript)");
#endif
            // Find SystemEvents process.  If found, quit it in case 
            // it is running under a different user.
            err = QuitOneProcess(kSystemEventsCreator);
#if TESTING
            if (err != noErr) {
                ShowMessage(false, false, false, "QuitOneProcess(kSystemEventsCreator) returned error %d ", (int) err);
            }
#endif
            // Wait for the process to be gone
            for (i=0; i<50; ++i) {      // 5 seconds max delay
                SleepTicks(6);  // 6 Ticks == 1/10 second
                err = FindProcess ('APPL', kSystemEventsCreator, &SystemEventsPSN);
                if (err != noErr) break;
            }
#if TESTING
            if (i >= 50) {
                ShowMessage(false, false, false, "Failed to make System Events quit");
            }
#endif
            sleep(2);
            
            err = LSFindApplicationForInfo(kSystemEventsCreator, NULL, NULL, &appRef, NULL);
            if (err != noErr) {
#if TESTING
                ShowMessage(false, false, false, "LSFindApplicationForInfo(kSystemEventsCreator) returned error %d ", (int) err);
#endif
            } else {
                FSRefMakePath(&appRef, (UInt8*)systemEventsPath, sizeof(systemEventsPath));
#if TESTING
                ShowMessage(false, false, false, "SystemEvents is at %s", systemEventsPath);
                ShowMessage(false, false, false, "Launching SystemEvents for user %s", pw->pw_name);
#endif

                sprintf(cmd, "sudo -u \"%s\" \"%s/Contents/MacOS/System Events\" &", pw->pw_name, systemEventsPath);
                err = system(cmd);
                if (err == noErr) {
                    // Wait for the process to start
                    for (i=0; i<50; ++i) {      // 5 seconds max delay
                        SleepTicks(6);  // 6 Ticks == 1/10 second
                        err = FindProcess ('APPL', kSystemEventsCreator, &SystemEventsPSN);
                        if (err == noErr) break;
                    }
#if TESTING
                    if (i >= 50) {
                        ShowMessage(false, false, false, "Failed to launch System Events for user %s", pw->pw_name);
                    }
#endif
                    sleep(2);

                    DeleteLoginItemOSAScript(pw->pw_name, "BOINCManager");
                    DeleteLoginItemOSAScript(pw->pw_name, "GridRepublic Desktop");
                    DeleteLoginItemOSAScript(pw->pw_name, "Progress Thru Processors Desktop");
                    DeleteLoginItemOSAScript(pw->pw_name, "Charity Engine Desktop");
#if TESTING
                } else {
                    ShowMessage(false, false, false, "[2] Command: %s returned error %d", cmd, (int) err);
#endif
                }
            }
        }
        
        // We don't delete the user's BOINC Manager preferences
//        sprintf(s, "rm -f \"/Users/%s/Library/Preferences/BOINC Manager Preferences\"", human_user_name);
//        system (s);
        
        // Delete per-user BOINC Manager and screensaver files
        sprintf(s, "rm -fR \"/Users/%s/Library/Application Support/BOINC\"", human_user_name);
        system (s);
        
        //  Set screensaver to "Computer Name" default screensaver only 
        //  if it was BOINC, GridRepublic, Progress Thru Processors or Charity Engine.
        changeSaver = false;
        seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
        if (compareOSVersionTo(10, 6) < 0) {
            f = popen("defaults -currentHost read com.apple.screensaver moduleName", "r");
            if (f) {
                while (PersistentFGets(s, sizeof(s), f)) {
                    if (strstr(s, "BOINCSaver")) {
                        changeSaver = true;
                        break;
                    }
                    if (strstr(s, "GridRepublic")) {
                        changeSaver = true;
                        break;
                    }
                    if (strstr(s, "Progress Thru Processors")) {
                        changeSaver = true;
                        break;
                    }
                    if (strstr(s, "Charity Engine")) {
                        changeSaver = true;
                        break;
                    }
                }
                pclose(f);
            }
        } else {
            err = GetCurrentScreenSaverSelection(s, sizeof(s) -1);
            if (err == noErr) {
                if (strstr(s, "BOINCSaver")) {
                    changeSaver = true;
                }
                if (strstr(s, "GridRepublic")) {
                    changeSaver = true;
                }
                if (strstr(s, "Progress Thru Processors")) {
                    changeSaver = true;
                }
                if (strstr(s, "Charity Engine")) {
                    changeSaver = true;
                }
            }
        }
        
        if (changeSaver) {
            if (compareOSVersionTo(10, 6) < 0) {
                system ("defaults -currentHost write com.apple.screensaver moduleName \"Computer Name\"");
                system ("defaults -currentHost write com.apple.screensaver modulePath \"/System/Library/Frameworks/ScreenSaver.framework/Versions/A/Resources/Computer Name.saver\"");
            } else {
                err = SetScreenSaverSelection("Computer Name", 
                    "/System/Library/Frameworks/ScreenSaver.framework/Versions/A/Resources/Computer Name.saver", 0);
            }
        }

        seteuid(saved_euid);    // Set effective uid back to privileged user
        
#if TESTING
//    ShowMessage(false, false, false, "After seteuid(%d) for user %s, euid = %d, saved_uid = %d", pw->pw_uid, human_user_name, geteuid(), saved_uid);
#endif
    }       // End userIndex loop
    
    sleep(1);
    
    if (compareOSVersionTo(10, 7) >= 0) {
#if TESTING
        ShowMessage(false, false, false, "Telling System Events to quit (at end)");
#endif
        err = QuitOneProcess(kSystemEventsCreator);
#if TESTING
        if (err != noErr) {
            ShowMessage(false, false, false, "QuitOneProcess(kSystemEventsCreator) returned error %d ", (int) err);
        }
#endif
    }

    return noErr;
}


static OSStatus GetAuthorization(AuthorizationRef * authRef, const char *pathToTool, char *brandName) {
    static Boolean              sIsAuthorized = false;
    AuthorizationRights         ourAuthRights;
    AuthorizationFlags          ourAuthFlags;
    AuthorizationItem           ourAuthRightsItem[1];
    AuthorizationEnvironment    ourAuthEnvironment;
    AuthorizationItem           ourAuthEnvItem[1];
    char                        prompt[256];
    OSStatus                    err = noErr;

    if (sIsAuthorized)
        return noErr;
    
    sprintf(prompt, (char *)_("Enter your administrator password to completely remove %s from you computer.\n\n"), brandName);

    ourAuthRights.count = 0;
    ourAuthRights.items = NULL;

    err = AuthorizationCreate (&ourAuthRights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, authRef);
    if (err != noErr) {
        ShowMessage(false, false, false, "AuthorizationCreate returned error %d", err);
        return err;
    }
     
    ourAuthRightsItem[0].name = kAuthorizationRightExecute;
    ourAuthRightsItem[0].value = (void *)pathToTool;
    ourAuthRightsItem[0].valueLength = strlen (pathToTool);
    ourAuthRightsItem[0].flags = 0;
    ourAuthRights.count = 1;
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
    err = AuthorizationCopyRights (*authRef, &ourAuthRights, &ourAuthEnvironment, ourAuthFlags, NULL);
    
    if (err == noErr)
        sIsAuthorized = true;
    
    return err;
}



static OSStatus DoPrivilegedExec(char *brandName, const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5) {
    AuthorizationRef    authRef = NULL;
    short               i;
    char                *args[8];
    OSStatus            err;
    FILE                *ioPipe;
    char                *p, junk[256];

    err = GetAuthorization(&authRef, pathToTool, brandName);
    if (err != noErr) {
        if (err != errAuthorizationCanceled)
            ShowMessage(false, false, false, "GetAuthorization returned error %d", err);
        return err;
    }

    for (i=0; i<5; i++) {       // Retry 5 times if error
        args[0] = arg1;
        args[1] = arg2;
        args[2] = arg3;
        args[3] = arg4;
        args[4] = arg5;
        args[5] = NULL;

        err = AuthorizationExecuteWithPrivileges (authRef, pathToTool, 0, args, &ioPipe);
        // We use the pipe to signal us when the command has completed
        if (ioPipe) {
            do {
                p = fgets(junk, sizeof(junk), ioPipe);
                // On some older versions of OS X, the pipe does not send EOF when our tool exits
                if (!strcmp(p, "Done\n")) break;
            } while (p);
            
            fclose (ioPipe);
        }
        
        if (err == noErr)
            break;
    }

if (err != noErr)
    ShowMessage(false, false, false, "\"%s %s %s %s %s %s\" returned error %d", pathToTool,
                        arg1 ? arg1 : "", arg2 ? arg2 : "", arg3 ? arg3 : "", 
                        arg4 ? arg4 : "", arg5 ? arg5 : "", err);

   return err;
}


// Used for OS <= 10.7
static void DeleteLoginItemOSAScript(char* user, char* appName)
{
    char                    cmd[2048];
    OSErr                   err;

    sprintf(cmd, "sudo -u \"%s\" osascript -e 'tell application \"System Events\"' -e 'delete (every login item whose name contains \"%s\")' -e 'end tell'", user, appName);
    err = system(cmd);
#if TESTING
    if (err) {
        ShowMessage(false, false, false, "Command: %s returned error %d", cmd, err);
    }
#endif
}
    

// Used for OS >= 10.8
static void DeleteLoginItemFromPListFile(void)
{
    Boolean                 success;
    int                     numberOfLoginItems, counter;
    char                    theName[256], *q;

    success = false;

    numberOfLoginItems = GetCountOfLoginItemsFromPlistFile();
    
    // Search existing login items in reverse order, deleting ours
    for (counter = numberOfLoginItems ; counter > 0 ; counter--)
    {
        GetLoginItemNameAtIndexFromPlistFile(counter-1, theName, sizeof(theName));
        q = theName;
        while (*q)
        {
            // It is OK to modify the returned string because we "own" it
            *q = toupper(*q);	// Make it case-insensitive
            q++;
        }
            
        if (strstr(theName, "BOINCMANAGER"))
            success = DeleteLoginItemNameAtIndexFromPlistFile(counter-1);
        if (strstr(theName, "GRIDREPUBLIC DESKTOP"))
            success = DeleteLoginItemNameAtIndexFromPlistFile(counter-1);
        if (strstr(theName, "PROGRESS THRU PROCESSORS DESKTOP"))
            success = DeleteLoginItemNameAtIndexFromPlistFile(counter-1);
        if (strstr(theName, "CHARITY ENGINE DESKTOP"))
            success = DeleteLoginItemNameAtIndexFromPlistFile(counter-1);
    }
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


OSErr GetCurrentScreenSaverSelection(char *moduleName, size_t maxLen) {
    OSErr err = noErr;
    CFStringRef nameKey = CFStringCreateWithCString(NULL,"moduleName",kCFStringEncodingASCII);
    CFStringRef moduleNameAsCFString;
    CFDictionaryRef theData;
    
    theData = (CFDictionaryRef)CFPreferencesCopyValue(CFSTR("moduleDict"), 
                CFSTR("com.apple.screensaver"), 
                kCFPreferencesCurrentUser,
                kCFPreferencesCurrentHost
                );
    if (theData == NULL) {
        CFRelease(nameKey);
        return (-1);
    }
    
    if (CFDictionaryContainsKey(theData, nameKey)  == false) 	
	{
        moduleName[0] = 0;
        CFRelease(nameKey);
        CFRelease(theData);
	    return(-1);
	}
    
    moduleNameAsCFString = CFStringCreateCopy(NULL, (CFStringRef)CFDictionaryGetValue(theData, nameKey));
    CFStringGetCString(moduleNameAsCFString, moduleName, maxLen, kCFStringEncodingASCII);		    

    CFRelease(nameKey);
    CFRelease(theData);
    CFRelease(moduleNameAsCFString);
    return err;
}


OSErr SetScreenSaverSelection(char *moduleName, char *modulePath, int type) {
    OSErr err = noErr;
    CFStringRef preferenceName = CFSTR("com.apple.screensaver");
    CFStringRef mainKeyName = CFSTR("moduleDict");
    CFDictionaryRef emptyData;
    CFMutableDictionaryRef newData;
    Boolean success;

    CFStringRef nameKey = CFStringCreateWithCString(NULL, "moduleName", kCFStringEncodingASCII);
    CFStringRef nameValue = CFStringCreateWithCString(NULL, moduleName, kCFStringEncodingASCII);
		
    CFStringRef pathKey = CFStringCreateWithCString(NULL, "path", kCFStringEncodingASCII);
    CFStringRef pathValue = CFStringCreateWithCString(NULL, modulePath, kCFStringEncodingASCII);
    
    CFStringRef typeKey = CFStringCreateWithCString(NULL, "type", kCFStringEncodingASCII);
    CFNumberRef typeValue = CFNumberCreate(NULL, kCFNumberIntType, &type);
    
    emptyData = CFDictionaryCreate(NULL, NULL, NULL, 0, NULL, NULL);
    if (emptyData == NULL) {
        CFRelease(nameKey);
        CFRelease(nameValue);
        CFRelease(pathKey);
        CFRelease(pathValue);
        CFRelease(typeKey);
        CFRelease(typeValue);
        return(-1);
    }

    newData = CFDictionaryCreateMutableCopy(NULL,0, emptyData);

    if (newData == NULL)
	{
        CFRelease(nameKey);
        CFRelease(nameValue);
        CFRelease(pathKey);
        CFRelease(pathValue);
        CFRelease(typeKey);
        CFRelease(typeValue);
        CFRelease(emptyData);
        return(-1);
    }

    CFDictionaryAddValue(newData, nameKey, nameValue); 	
    CFDictionaryAddValue(newData, pathKey, pathValue); 	
    CFDictionaryAddValue(newData, typeKey, typeValue); 	

    CFPreferencesSetValue(mainKeyName, newData, preferenceName, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);
    success = CFPreferencesSynchronize(preferenceName, kCFPreferencesCurrentUser, kCFPreferencesCurrentHost);

    if (!success) {
        err = -1;
    }
    
    CFRelease(nameKey);
    CFRelease(nameValue);
    CFRelease(pathKey);
    CFRelease(pathValue);
    CFRelease(typeKey);
    CFRelease(typeValue);
    CFRelease(emptyData);
    CFRelease(newData);

    return err;
}


int GetCountOfLoginItemsFromPlistFile() {
    CFArrayRef	arrayOfLoginItemsFixed;
    int valueToReturn = -1;
    CFStringRef loginItemArrayKeyName = CFSTR("CustomListItems");
    CFDictionaryRef topLevelDict;

    topLevelDict = (CFDictionaryRef)CFPreferencesCopyValue(CFSTR("SessionItems"), CFSTR("com.apple.loginitems"), kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    if (topLevelDict == NULL) {
        return (0);
    }

    if (CFDictionaryContainsKey(topLevelDict, loginItemArrayKeyName)  == false) 	
	{
        CFRelease(topLevelDict);
	    return(0);
	}
    
    arrayOfLoginItemsFixed = (CFArrayRef)CFDictionaryGetValue(topLevelDict, loginItemArrayKeyName);
    if( arrayOfLoginItemsFixed == NULL)
	{
        CFRelease(topLevelDict);
	    return(0); 
	}
	
    valueToReturn = (int) CFArrayGetCount(arrayOfLoginItemsFixed);

    CFRelease(topLevelDict);
    return(valueToReturn);    
}


// Returns empty string on failure
OSErr GetLoginItemNameAtIndexFromPlistFile(int index, char *name, size_t maxLen) {
    CFArrayRef	arrayOfLoginItemsFixed = NULL;
    CFStringRef loginItemArrayKeyName = CFSTR("CustomListItems");
    CFDictionaryRef topLevelDict = NULL;
    CFDictionaryRef loginItemDict = NULL;
    CFStringRef nameKey = CFSTR("Name");
    CFStringRef nameAsCFString = NULL;
    
    *name = 0;  // Prepare for failure

    topLevelDict = (CFDictionaryRef)CFPreferencesCopyValue(CFSTR("SessionItems"), CFSTR("com.apple.loginitems"), kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    if (topLevelDict == NULL) {
        return (-1);
    }

    if (CFDictionaryContainsKey(topLevelDict, loginItemArrayKeyName)  == false) 	
	{
        CFRelease(topLevelDict);
	    return(-1);
	}
    
    arrayOfLoginItemsFixed = (CFArrayRef) CFDictionaryGetValue(topLevelDict, loginItemArrayKeyName);
    if( arrayOfLoginItemsFixed == NULL)
	{
        CFRelease(topLevelDict);
	    return(-1); 
	}
    
    loginItemDict = (CFDictionaryRef) CFArrayGetValueAtIndex(arrayOfLoginItemsFixed, (CFIndex)index);
    if (loginItemDict == NULL)
    {
        CFRelease(topLevelDict);
        return(-1); 
    }

	if (CFDictionaryContainsKey(loginItemDict, nameKey) == false) 	
	{
        CFRelease(topLevelDict);
        return(-1); 
	}

    nameAsCFString = CFStringCreateCopy(NULL, (CFStringRef)CFDictionaryGetValue(loginItemDict, nameKey));
    CFStringGetCString(nameAsCFString, name, maxLen, kCFStringEncodingASCII);		    

    CFRelease(topLevelDict);
    CFRelease(nameAsCFString);
    return(noErr);
}


OSErr DeleteLoginItemNameAtIndexFromPlistFile(int index){
    CFArrayRef	arrayOfLoginItemsFixed = NULL;
    CFMutableArrayRef arrayOfLoginItemsModifiable;
    CFStringRef topLevelKeyName = CFSTR("SessionItems");
    CFStringRef preferenceName = CFSTR("com.apple.loginitems");
    CFStringRef loginItemArrayKeyName = CFSTR("CustomListItems");
    CFDictionaryRef oldTopLevelDict = NULL;
    CFMutableDictionaryRef newTopLevelDict = NULL;
    Boolean success;
    OSErr err = noErr;

    oldTopLevelDict = (CFDictionaryRef)CFPreferencesCopyValue(topLevelKeyName, preferenceName, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    if (oldTopLevelDict == NULL) {
        return (-1);
    }

    if (CFDictionaryContainsKey(oldTopLevelDict, loginItemArrayKeyName)  == false) 	
	{
        CFRelease(oldTopLevelDict);
	    return(-1);
	}
    
    arrayOfLoginItemsFixed = (CFArrayRef) CFDictionaryGetValue(oldTopLevelDict, loginItemArrayKeyName);
    if( arrayOfLoginItemsFixed == NULL)
	{
        CFRelease(oldTopLevelDict);
	    return(-1); 
	}

    arrayOfLoginItemsModifiable = CFArrayCreateMutableCopy(NULL, 0, arrayOfLoginItemsFixed);
    if( arrayOfLoginItemsModifiable == NULL)
	{
        CFRelease(oldTopLevelDict);
	    return(-1); 
	}

    CFArrayRemoveValueAtIndex(arrayOfLoginItemsModifiable, (CFIndex) index);    

    newTopLevelDict = CFDictionaryCreateMutableCopy(NULL, 0, oldTopLevelDict);
    CFDictionaryReplaceValue(newTopLevelDict, loginItemArrayKeyName, arrayOfLoginItemsModifiable);

    CFPreferencesSetValue(topLevelKeyName, newTopLevelDict, preferenceName, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    success = CFPreferencesSynchronize(preferenceName, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);

    if (!success) {
        err = -1;
    }
 
    CFRelease(oldTopLevelDict);
    CFRelease(newTopLevelDict);
   
    return(err);

}


// ---------------------------------------------------------------------------
/* This runs through the process list looking for the indicated application */
/*  Searches for process by file type and signature (creator code)          */
// ---------------------------------------------------------------------------
static OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN)
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


static pid_t FindProcessPID(char* name, pid_t thePID)
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


static OSStatus QuitOneProcess(OSType signature) {
    bool			done = false;
    ProcessSerialNumber         thisPSN;
    ProcessInfoRec		thisPIR;
    OSStatus                    err = noErr;
    Str63			thisProcessName;
    AEAddressDesc		thisPSNDesc;
    AppleEvent			thisQuitEvent, thisReplyEvent;
    

    thisPIR.processInfoLength = sizeof (ProcessInfoRec);
    thisPIR.processName = thisProcessName;
    thisPIR.processAppSpec = nil;
    
    thisPSN.highLongOfPSN = 0;
    thisPSN.lowLongOfPSN = kNoProcess;
    
    while (done == false) {		
        err = GetNextProcess(&thisPSN);
        if (err == procNotFound)	
            done = true;		// apparently the app isn't running.
        else {		
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
#if 0
                if (err == errAETimeout) {
                    pid_t thisPID;
                        
                    err = GetProcessPID(&thisPSN , &thisPID);
                    if (err == noErr)
                        err = kill(thisPID, SIGKILL);
                }
#endif
                done = true;		// we've killed the process, presumably
            }
        }
    }

bail:
    return err;
}


// Kill a process if it exists
static void KillVBoxProcess(char* name) {
    pid_t   VBoxPid;
    
    VBoxPid = FindProcessPID(name, 0);
    if (!VBoxPid) return;   // No such process
    
    kill(VBoxPid, SIGQUIT);
    sleep(2);
    if (FindProcessPID(NULL, VBoxPid)) {
        kill(VBoxPid, SIGKILL);
    }
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


// Because language preferences are set on a per-user basis, we
// must get the preferred languages while set to the current 
// user, before we switch to root in our second pass.
// So we get the preferred languages here and write them to a
// temporary file to be retrieved by our second pass.
// We must do it this way because, for unknown reasons, the
// CFBundleCopyLocalizationsForPreferences() API does not work
// correctly if we seteuid and setuid to the logged in user 
// after running as root.

static void GetPreferredLanguages() {
    DIR *dirp;
    struct dirent *dp;
    char searchPath[MAXPATHLEN];
    struct stat sbuf;
    CFMutableArrayRef supportedLanguages;
    CFStringRef aLanguage;
    char shortLanguage[32];
    CFArrayRef preferredLanguages;
    int i, j, k;
    char * language;
    char *uscore;
    FILE *f;

    // Create an array of all our supported languages
    supportedLanguages = CFArrayCreateMutable(kCFAllocatorDefault, 100, &kCFTypeArrayCallBacks);
    
    aLanguage = CFStringCreateWithCString(NULL, "en", kCFStringEncodingMacRoman);
    CFArrayAppendValue(supportedLanguages, aLanguage);
    aLanguage = NULL;

    dirp = opendir(gCatalogsDir);
    if (!dirp) goto bail;
    while (true) {
        dp = readdir(dirp);
        if (dp == NULL)
            break;                  // End of list

        if (dp->d_name[0] == '.')
            continue;               // Ignore names beginning with '.'

        strlcpy(searchPath, gCatalogsDir, sizeof(searchPath));
        strlcat(searchPath, dp->d_name, sizeof(searchPath));
        strlcat(searchPath, "/", sizeof(searchPath));
        strlcat(searchPath, gCatalog_Name, sizeof(searchPath));
        strlcat(searchPath, ".mo", sizeof(searchPath));
        // stat() returns zero on success
        if (stat(searchPath, &sbuf) != 0) continue;
//        printf("Adding %s to supportedLanguages array\n", dp->d_name);
        aLanguage = CFStringCreateWithCString(NULL, dp->d_name, kCFStringEncodingMacRoman);
        CFArrayAppendValue(supportedLanguages, aLanguage);
        aLanguage = NULL;
        
        // If it has a region code ("it_IT") also try without region code ("it")
        // TODO: Find a more general solution
        strlcpy(shortLanguage, dp->d_name, sizeof(shortLanguage));
        uscore = strchr(shortLanguage, '_');
        if (uscore) {
            *uscore = '\0';
            aLanguage = CFStringCreateWithCString(NULL, shortLanguage, kCFStringEncodingMacRoman);
            CFArrayAppendValue(supportedLanguages, aLanguage);
            aLanguage = NULL;
        }
    }
    
    closedir(dirp);

    // Write a temp file to tell our PostInstall.app our preferred languages
    f = fopen(gTempFileName, "w");

    for (i=0; i<MAX_LANGUAGES_TO_TRY; ++i) {
    
        preferredLanguages = CFBundleCopyLocalizationsForPreferences(supportedLanguages, NULL );
        
#if 0   // For testing
        int c = CFArrayGetCount(preferredLanguages);
        for (k=0; k<c; ++k) {
        CFStringRef s = (CFStringRef)CFArrayGetValueAtIndex(preferredLanguages, k);
            printf("Preferred language %u is %s\n", k, CFStringGetCStringPtr(s, kCFStringEncodingMacRoman));
        }

#endif

        for (j=0; j<CFArrayGetCount(preferredLanguages); ++j) {
            aLanguage = (CFStringRef)CFArrayGetValueAtIndex(preferredLanguages, j);
            language = (char *)CFStringGetCStringPtr(aLanguage, kCFStringEncodingMacRoman);
            if (f) {
                fprintf(f, "%s\n", language);
            }
            
            // Remove all copies of this language from our list of supported languages 
            // so we can get the next preferred language in order of priority
            for (k=CFArrayGetCount(supportedLanguages)-1; k>=0; --k) {
                if (CFStringCompare(aLanguage, (CFStringRef)CFArrayGetValueAtIndex(supportedLanguages, k), 0) == kCFCompareEqualTo) {
                    CFArrayRemoveValueAtIndex(supportedLanguages, k);
                }
            }

            // Since the original strings are English, no 
            // further translation is needed for language en.
            if (!strcmp(language, "en")) {
                fclose(f);
                CFRelease(preferredLanguages);
                preferredLanguages = NULL;
                CFArrayRemoveAllValues(supportedLanguages);
                CFRelease(supportedLanguages);
                supportedLanguages = NULL;
                return;
            }
        }
        
        CFRelease(preferredLanguages);
        preferredLanguages = NULL;

    }
    fclose(f);

bail:
    CFArrayRemoveAllValues(supportedLanguages);
    CFRelease(supportedLanguages);
    supportedLanguages = NULL;
}


static void LoadPreferredLanguages(){
    FILE *f;
    int i;
    char *p;
    char language[32];

    BOINCTranslationInit();

    // First pass wrote a list of our preferred languages to a temp file
    f = fopen(gTempFileName, "r");
    if (!f) return;
    
    for (i=0; i<MAX_LANGUAGES_TO_TRY; ++i) {
        fgets(language, sizeof(language), f);
        if (feof(f)) break;
        language[sizeof(language)-1] = '\0';    // Guarantee a null terminator
        p = strchr(language, '\n');
        if (p) *p = '\0';           // Replace newline with null terminator 
        if (language[0]) {
            if (!BOINCTranslationAddCatalog(gCatalogsDir, language, gCatalog_Name)) {
                printf("could not load catalog for langage %s\n", language);
            }
        }
    }
    fclose(f);
}


static Boolean ShowMessage(Boolean allowCancel, Boolean continueButton, Boolean yesNoButtons, const char *format, ...) {
    va_list                 args;
    char                    s[1024];
    CFOptionFlags           responseFlags;
    CFURLRef                myIconURLRef = NULL;
    CFBundleRef             myBundleRef;
    CFDictionaryRef         myInfoPlistData;
    CFStringRef             appIconName;
    ProcessSerialNumber     ourProcess;
    Boolean                 result;

    myBundleRef = CFBundleGetMainBundle();
    if (myBundleRef) {
        if (!strcmp(gBrandName, "BOINC")) {
            // For BOINC uninstaller use our special icon
            myIconURLRef = CFBundleCopyResourceURL(myBundleRef, CFSTR("PutInTrash.icns"), NULL, NULL);
        } else {
            // For branded uninstaller (GR, CE, PtP) use uninstaller application icon
            myInfoPlistData = CFBundleGetInfoDictionary(myBundleRef);
            if (myInfoPlistData) {
                appIconName = (CFStringRef)CFDictionaryGetValue(myInfoPlistData, CFSTR("CFBundleIconFile"));
                myIconURLRef = CFBundleCopyResourceURL(myBundleRef, appIconName, NULL, NULL);
            }
        }
    }

    va_start(args, format);
    vsprintf(s, format, args);
    va_end(args);

    CFStringRef myString = CFStringCreateWithCString(NULL, s, kCFStringEncodingUTF8);
    CFStringRef theTitle = CFStringCreateWithCString(NULL, gAppName, kCFStringEncodingUTF8);
    CFStringRef cancelString = CFStringCreateWithCString(NULL, (char*)_("Cancel"), kCFStringEncodingUTF8);
    CFStringRef continueString = CFStringCreateWithCString(NULL, (char*)_("Continue..."), kCFStringEncodingUTF8);
    CFStringRef yesString = CFStringCreateWithCString(NULL, (char*)_("Yes"), kCFStringEncodingUTF8);
    CFStringRef noString = CFStringCreateWithCString(NULL, (char*)_("No"), kCFStringEncodingUTF8);

    ::GetCurrentProcess (&ourProcess);
    ::SetFrontProcess(&ourProcess);

    // Set default button to Continue, OK or No
    // Set alternate button to Cancel, Yes, or hidden
    SInt32 retval = CFUserNotificationDisplayAlert(0.0, kCFUserNotificationPlainAlertLevel,
                myIconURLRef, NULL, NULL, theTitle, myString,
                continueButton ? continueString : (yesNoButtons ? noString : NULL),
                (allowCancel || yesNoButtons) ? (yesNoButtons ? yesString : cancelString) : NULL,
                NULL, &responseFlags);
    
    if (myIconURLRef) CFRelease(myIconURLRef);
    if (myString) CFRelease(myString);
    if (theTitle) CFRelease(theTitle);
    if (cancelString) CFRelease(cancelString);
    if (continueString) CFRelease(continueString);
    if (yesString) CFRelease(yesString);
    if (noString) CFRelease(noString);

    if (retval) return false;
    
    result = (responseFlags == kCFUserNotificationDefaultResponse);
    // Return TRUE if user clicked Continue, Yes or OK, FALSE if user clicked Cancel or No
    // Note: if yesNoButtons is true, we made default button "No" and alternate button "Yes" 
    return (yesNoButtons ? !result : result);
}


#if SEARCHFORALLBOINCMANAGERS
static OSStatus GetpathToBOINCManagerApp(char* path, int maxLen, FSRef *theFSRef)
{
    CFStringRef             bundleID = CFSTR("edu.berkeley.boinc");
    OSType                  creator = 'BNC!';
    OSStatus                status = noErr;

        status = LSFindApplicationForInfo(creator, bundleID, NULL, theFSRef, NULL);
        if (status) {
#if TESTING
            ShowMessage(false, false, false, "LSFindApplicationForInfo returned error %d", status);
#endif 
            return status;
        }
    
        status = FSRefMakePath(theFSRef, (unsigned char *)path, maxLen);
#if TESTING
        if (status)
            ShowMessage(false, false, false, "GetpathToBOINCManagerApp FSRefMakePath returned error %d, %s", status, path);
#endif
    
    return status;
}
#endif  // SEARCHFORALLBOINCMANAGERS


static int compareOSVersionTo(int toMajor, int toMinor) {
    SInt32 major, minor;
    OSStatus err = noErr;
    
    err = Gestalt(gestaltSystemVersionMajor, &major);
    if (err != noErr) {
        fprintf(stderr, "Gestalt(gestaltSystemVersionMajor) returned error %ld\n", err);
        fflush(stderr);
        return -1;  // gestaltSystemVersionMajor selector was not available before OS 10.4
    }
    if (major < toMajor) return -1;
    if (major > toMajor) return 1;
    err = Gestalt(gestaltSystemVersionMinor, &minor);
    if (err != noErr) {
        fprintf(stderr, "Gestalt(gestaltSystemVersionMinor) returned error %ld\n", err);
        fflush(stderr);
        return -1;  // gestaltSystemVersionMajor selector was not available before OS 10.4
    }
    if (minor < toMinor) return -1;
    if (minor > toMinor) return 1;
    return 0;
}

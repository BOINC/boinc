// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

#define TESTING 0       /* for debugging */
#define VERBOSE_TEST 0  /* for debugging callPosixSpawn */

    
#include <Carbon/Carbon.h>

#include <grp.h>

#include <unistd.h>	// geteuid, seteuid
#include <pwd.h>	// passwd, getpwnam
#include <dirent.h>
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>   // For stat()
#include <sys/time.h>
#include <string.h>
#include <vector>
#include <string>

using std::vector;
using std::string;


// WARNING -- SEARCHFORALLBOINCMANAGERS CODE HAS NOT BEEN TESTED
#define SEARCHFORALLBOINCMANAGERS 0

#define MAX_LANGUAGES_TO_TRY 5
#define MANIPULATE_LOGINITEM_PLIST_FILE 0

#include "mac_util.h"
#include "translate.h"


static OSStatus DoUninstall(void);
static OSStatus CleanupAllVisibleUsers(void);
static OSStatus DeleteOurBundlesFromDirectory(CFStringRef bundleID, char *extension, char *dirPath);
static void DeleteLoginItemOSAScript(char* user, char* appName);
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
OSErr GetCurrentScreenSaverSelection(char *moduleName, size_t maxLen);
OSErr SetScreenSaverSelection(char *moduleName, char *modulePath, int type);
static pid_t FindProcessPID(char* name, pid_t thePID);
static int KillOneProcess(char* name);
static double dtime(void);
static void SleepSeconds(double seconds);
static void GetPreferredLanguages();
static void LoadPreferredLanguages();
static Boolean ShowMessage(Boolean allowCancel, Boolean continueButton, Boolean yesNoButtons, const char *format, ...);
int callPosixSpawn(const char *cmd, bool delayForResult=false);
void print_to_log_file(const char *format, ...);

#if MANIPULATE_LOGINITEM_PLIST_FILE
static void DeleteLoginItemFromPListFile(void);
int GetCountOfLoginItemsFromPlistFile(void);
OSErr GetLoginItemNameAtIndexFromPlistFile(int index, char *name, size_t maxLen);
OSErr DeleteLoginItemNameAtIndexFromPlistFile(int index);
#endif

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
    char                        cmd[MAXPATHLEN+64];
    Boolean                     cancelled = false;
    struct stat                 sbuf;
    OSStatus                    err = noErr;

    pathToSelf[0] = '\0';
    // Get the full path to our executable inside this application's bundle
    getPathToThisApp(pathToSelf, sizeof(pathToSelf));
    if (!pathToSelf[0]) {
        ShowMessage(false, false, false, "Couldn't get path to self.");
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
        // The "activate" comand brings the password dialog to the front and makes it the active window.
        // "with administrator privileges" launches the helper application as user root.
        sprintf(cmd, "osascript -e 'activate' -e 'do shell script \"sudo \\\"%s\\\" --privileged\" with administrator privileges'", pathToSelf);
        err = callPosixSpawn(cmd, true);
    }
    
    if (cancelled || (err != noErr)) {
        ShowMessage(false, false, false, (char *)_("Canceled: %s has not been touched."), p);
        BOINCTranslationCleanup();
        return err;
    }

    CFStringRef CFBOINCDataPath, CFUserPrefsPath;
    char BOINCDataPath[MAXPATHLEN], temp[MAXPATHLEN], PathToPrefs[MAXPATHLEN];
    Boolean success = false;
    char * loginName = getlogin();
    
    CFURLRef urlref = CFURLCreateWithFileSystemPath(NULL, CFSTR("/Library"),
                                                    kCFURLPOSIXPathStyle, true);
    success = CFURLCopyResourcePropertyForKey(urlref, kCFURLLocalizedNameKey,
                                                &CFBOINCDataPath, NULL);
    CFRelease(urlref);
    if (success) {
        success = CFStringGetCString(CFBOINCDataPath, temp,
                    sizeof(temp), kCFStringEncodingUTF8);
        CFRelease(CFBOINCDataPath);
    }

    if (success) {
        success = false;
        strlcpy(BOINCDataPath, "/", sizeof(BOINCDataPath));
        strlcat(BOINCDataPath, temp, sizeof(BOINCDataPath));
        strlcat(BOINCDataPath, "/", sizeof(BOINCDataPath));
        
        urlref = CFURLCreateWithFileSystemPath(NULL, CFSTR("/Library/Application Support"),
                                                kCFURLPOSIXPathStyle, true);
        success = CFURLCopyResourcePropertyForKey(urlref, kCFURLLocalizedNameKey,
                                                    &CFBOINCDataPath, NULL);
        CFRelease(urlref);
    }
    if (success) {
        success = CFStringGetCString(CFBOINCDataPath, temp,
                                        sizeof(temp), kCFStringEncodingUTF8);
        CFRelease(CFBOINCDataPath);
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

    urlref = CFURLCreateWithFileSystemPath(NULL, CFSTR("/Users"),
                                                    kCFURLPOSIXPathStyle, true);
    success = CFURLCopyResourcePropertyForKey(urlref, kCFURLLocalizedNameKey,
                                                &CFUserPrefsPath, NULL);
    CFRelease(urlref);
    if (success) {
        success = CFStringGetCString(CFUserPrefsPath, temp, sizeof(temp),
                                        kCFStringEncodingUTF8);
        CFRelease(CFUserPrefsPath);
    }
    if (success) {
        success = false;
        strlcpy(PathToPrefs, "/", sizeof(PathToPrefs));
        strlcat(PathToPrefs, temp, sizeof(PathToPrefs));
        strlcat(PathToPrefs, "/[", sizeof(PathToPrefs));
        strlcat(PathToPrefs, (char *)_("name  of user"), sizeof(PathToPrefs));
        strlcat(PathToPrefs, "]/", sizeof(PathToPrefs));
        
        sprintf(temp, "/Users/%s/Library", loginName);
        CFUserPrefsPath = CFStringCreateWithCString(kCFAllocatorDefault, temp,
                                                    kCFStringEncodingUTF8);

        urlref = CFURLCreateWithFileSystemPath(NULL, CFUserPrefsPath,
                                                    kCFURLPOSIXPathStyle, true);
        CFRelease(CFUserPrefsPath);
        success = CFURLCopyResourcePropertyForKey(urlref, kCFURLLocalizedNameKey,
                                                &CFUserPrefsPath, NULL);
        CFRelease(urlref);
    }
    if (success) {
        success = CFStringGetCString(CFUserPrefsPath, temp, sizeof(temp),
                                        kCFStringEncodingUTF8);
        CFRelease(CFUserPrefsPath);
    }
    if (success) {
        success = false;
        strlcat(PathToPrefs, temp, sizeof(PathToPrefs));
        strlcat(PathToPrefs, "/", sizeof(PathToPrefs));
        
        sprintf(temp, "/Users/%s/Library/Preferences", loginName);
        CFUserPrefsPath = CFStringCreateWithCString(kCFAllocatorDefault, temp,
                                                    kCFStringEncodingUTF8);
        urlref = CFURLCreateWithFileSystemPath(NULL, CFUserPrefsPath,
                                                    kCFURLPOSIXPathStyle, true);
        CFRelease(CFUserPrefsPath);
        success = CFURLCopyResourcePropertyForKey(urlref, kCFURLLocalizedNameKey,
                                                &CFUserPrefsPath, NULL);
        CFRelease(urlref);
    }
    if (success) {
        success = CFStringGetCString(CFUserPrefsPath, temp, sizeof(temp),
                                        kCFStringEncodingUTF8);
        CFRelease(CFUserPrefsPath);
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
            if (KillOneProcess("VirtualBox")) {
                sleep(5);
            }
            // List of processes to kill taken from my_processes
            // array in VirtualBox_Uninstall.tool script:
            KillOneProcess("VirtualBox-amd64");
            KillOneProcess("VirtualBox-x86");
            KillOneProcess("VirtualBoxVM");
            KillOneProcess("VirtualBoxVM-amd64");
            KillOneProcess("VirtualBoxVM-x86");
            KillOneProcess("VBoxManage");
            KillOneProcess("VBoxManage-amd64");
            KillOneProcess("VBoxManage-x86");
            KillOneProcess("VBoxHeadless");
            KillOneProcess("VBoxHeadless-amd64");
            KillOneProcess("VBoxHeadless-x86");
            KillOneProcess("vboxwebsrv");
            KillOneProcess("vboxwebsrv-amd64");
            KillOneProcess("vboxwebsrv-x86");
            KillOneProcess("VBoxXPCOMIPCD");
            KillOneProcess("VBoxXPCOMIPCD-amd64");
            KillOneProcess("VBoxXPCOMIPCD-x86");
            KillOneProcess("VBoxSVC");
            KillOneProcess("VBoxSVC-amd64");
            KillOneProcess("VBoxSVC-x86");
            KillOneProcess("VBoxNetDHCP");
            KillOneProcess("VBoxNetDHCP-amd64");
            KillOneProcess("VBoxNetDHCP-x86");
            sleep(2);
            
            snprintf(cmd, sizeof(cmd), "source \"%s\" --unattended", pathToVBoxUninstallTool);
            callPosixSpawn(cmd);
        }
    }
    
    ShowMessage(false, false, false, (char *)_("Removal completed.\n\n You may want to remove the following remaining items using the Finder: \n"
     "the directory \"%s\"\n\nfor each user, the file\n"
     "\"%s\"."), BOINCDataPath, PathToPrefs);

    
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
    CFStringRef             cfPath;
    CFURLRef                appURL;
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
// WARNING -- SEARCHFORALLBOINCMANAGERS CODE HAS NOT BEEN TESTED

    // Phase 1: try to find all our applications using LaunchServices
    for (i=0; i<100; i++) {
        strlcpy(myRmCommand, "rm -rf \"", 10);
        pathOffset = strlen(myRmCommand);
    
        err = GetPathToAppFromID('BNC!', CFSTR("edu.berkeley.boinc"),  myRmCommand+pathOffset, MAXPATHLEN);
        if (err) {
            break;
        }
        
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
            // LaunchServices Database; otherwise GetPathToAppFromID might return
            // this application again after it's been deleted.
            strlcpy(plistRmCommand, myRmCommand, sizeof(plistRmCommand));
            strlcat(plistRmCommand, "/Contents/info.plist", sizeof(plistRmCommand));
#if TESTING
        ShowMessage(false, false, false, "Deleting info.plist: %s", plistRmCommand);
#endif
            callPosixSpawn(plistRmCommand);
            cfPath = CFStringCreateWithCString(NULL, myRmCommand+pathOffset, kCFStringEncodingUTF8);
            appURL = CFURLCreateWithFileSystemPath(NULL, CFStringRef filePath, kCFURLPOSIXPathStyle, true);
            if (cfPath) {
                CFRelease(cfPath);
            }
            if (appURL) {
                CFRelease(appURL);
            }

            err = LSRegisterURL, true);
#if TESTING
            if (err)
                ShowMessage(false, false, false, "LSRegisterFSRef returned error %d", err);
#endif
            callPosixSpawn(myRmCommand);
        }
    }
#endif  // SEARCHFORALLBOINCMANAGERS

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
    callPosixSpawn ("rm -rf /Applications/BOINCManager.app");
    callPosixSpawn ("rm -rf \"/Library/Screen Savers/BOINCSaver.saver\"");
    
    callPosixSpawn ("rm -rf \"/Applications/GridRepublic Desktop.app\"");
    callPosixSpawn ("rm -rf \"/Library/Screen Savers/GridRepublic.saver\"");
    
    callPosixSpawn ("rm -rf \"/Applications/Progress Thru Processors Desktop.app\"");
    callPosixSpawn ("rm -rf \"/Library/Screen Savers/Progress Thru Processors.saver\"");
    
    callPosixSpawn ("rm -rf \"/Applications/Charity Engine Desktop.app\"");
    callPosixSpawn ("rm -rf \"/Library/Screen Savers/Charity Engine.saver\"");
    
    // Delete any receipt from an older installer (which had 
    // a wrapper application around the installer package.)
    callPosixSpawn ("rm -rf /Library/Receipts/GridRepublic.pkg");
    callPosixSpawn ("rm -rf /Library/Receipts/Progress\\ Thru\\ Processors.pkg");
    callPosixSpawn ("rm -rf /Library/Receipts/Charity\\ Engine.pkg");
    callPosixSpawn ("rm -rf /Library/Receipts/BOINC.pkg");

    // Delete any receipt from a newer installer (a bare package.) 
    callPosixSpawn ("rm -rf /Library/Receipts/GridRepublic\\ Installer.pkg");
    callPosixSpawn ("rm -rf /Library/Receipts/Progress\\ Thru\\ Processors\\ Installer.pkg");
    callPosixSpawn ("rm -rf /Library/Receipts/Charity\\ Engine\\ Installer.pkg");
    callPosixSpawn ("rm -rf /Library/Receipts/BOINC\\ Installer.pkg");

    // Phase 5: Set BOINC Data owner and group to logged in user
    // We don't customize BOINC Data directory name for branding
//    callPosixSpawn ("rm -rf \"/Library/Application Support/BOINC Data\"");
    p = getlogin();
    pw = getpwnam(p);
    sprintf(cmd, "chown -R %d:%d \"/Library/Application Support/BOINC Data\"", pw->pw_uid, pw->pw_gid);
    callPosixSpawn (cmd);
    callPosixSpawn("chmod -R u+rw-s,g+r-w-s,o+r-w \"/Library/Application Support/BOINC Data\"");
    callPosixSpawn("chmod 600 \"/Library/Application Support/BOINC Data/gui_rpc_auth.cfg\"");
    
    // Phase 6: step through all users and do user-specific cleanup
    CleanupAllVisibleUsers();
    
    callPosixSpawn ("dscl . -delete /users/boinc_master");
    callPosixSpawn ("dscl . -delete /groups/boinc_master");
    callPosixSpawn ("dscl . -delete /users/boinc_project");
    callPosixSpawn ("dscl . -delete /groups/boinc_project");


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

                               callPosixSpawn(myRmCommand);
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

CFStringRef kSystemEventsBundleID = CFSTR("com.apple.systemevents");
char *systemEventsAppName = "System Events";

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
    pid_t               systemEventsPID;
    FILE                *f;
    char                *p;
    int                 id;
    OSStatus            err;
    Boolean             changeSaver;

    saved_uid = getuid();
    saved_euid = geteuid();

    err = noErr;
    systemEventsPath[0] = '\0';

    err = GetPathToAppFromID(kSystemEventsCreator, kSystemEventsBundleID,  systemEventsPath, sizeof(systemEventsPath));

#if TESTING
    if (err == noErr) {
        ShowMessage(false, false, false, "SystemEvents is at %s", systemEventsPath);
    } else {
        ShowMessage(false, false, false, "GetPathToAppFromID(kSystemEventsCreator, kSystemEventsBundleID) returned error %d ", (int) err);
    }
#endif

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
        callPosixSpawn (s);

        sprintf(s, "dscl . -delete /groups/boinc_project users \"%s\"", human_user_name);
        callPosixSpawn (s);

#if TESTING
//    ShowMessage(false, false, false, "Before seteuid(%d) for user %s, euid = %d", pw->pw_uid, human_user_name, geteuid());
#endif
        setuid(0);
        // Delete our login item(s) for this user
#if MANIPULATE_LOGINITEM_PLIST_FILE
       if (compareOSVersionTo(10, 8) >= 0) {
            seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
            DeleteLoginItemFromPListFile();
            seteuid(saved_euid);    // Set effective uid back to privileged user
        } else {            // OS 10.7.x
#endif
            // We must leave effective user ID as privileged user (root)
            // because the target user may not be in the sudoers file.

            // We must launch the System Events application for the target user

#if TESTING
            ShowMessage(false, false, false, "Telling System Events to quit (before DeleteLoginItemOSAScript)");
#endif
            // Find SystemEvents process.  If found, quit it in case 
            // it is running under a different user.
            systemEventsPID = FindProcessPID(systemEventsAppName, 0);
            if (systemEventsPID != 0) {
                err = kill(systemEventsPID, SIGKILL);
            }
#if TESTING
            if (err != noErr) {
                ShowMessage(false, false, false, "kill(systemEventsPID, SIGKILL) returned error %d ", (int) err);
            }
#endif
            // Wait for the process to be gone
            for (i=0; i<50; ++i) {      // 5 seconds max delay
                SleepSeconds(0.1);      // 1/10 second
                systemEventsPID = FindProcessPID(systemEventsAppName, 0);
                if (systemEventsPID == 0) break;
            }
#if TESTING
            if (i >= 50) {
                ShowMessage(false, false, false, "Failed to make System Events quit");
            }
#endif
            sleep(2);

            if (systemEventsPath[0] != '\0') {
#if TESTING
                ShowMessage(false, false, false, "Launching SystemEvents for user %s", pw->pw_name);
#endif
                sprintf(cmd, "sudo -u \"%s\" -b \"%s/Contents/MacOS/System Events\"", pw->pw_name, systemEventsPath);
                err = callPosixSpawn(cmd);
                if (err == noErr) {
                    // Wait for the process to start
                    for (i=0; i<50; ++i) {      // 5 seconds max delay
                        SleepSeconds(0.1);      // 1/10 second
                        systemEventsPID = FindProcessPID(systemEventsAppName, 0);
                        if (systemEventsPID != 0) break;
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
#if MANIPULATE_LOGINITEM_PLIST_FILE
        }
#endif

        // We don't delete the user's BOINC Manager preferences
//        sprintf(s, "rm -f \"/Users/%s/Library/Preferences/BOINC Manager Preferences\"", human_user_name);
//        callPosixSpawn (s);
        
        // Delete per-user BOINC Manager and screensaver files
        sprintf(s, "rm -fR \"/Users/%s/Library/Application Support/BOINC\"", human_user_name);
        callPosixSpawn (s);
        
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
                callPosixSpawn ("defaults -currentHost write com.apple.screensaver moduleName \"Computer Name\"");
                callPosixSpawn ("defaults -currentHost write com.apple.screensaver modulePath \"/System/Library/Frameworks/ScreenSaver.framework/Versions/A/Resources/Computer Name.saver\"");
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
    
#if TESTING
    ShowMessage(false, false, false, "Telling System Events to quit (at end)");
#endif
    systemEventsPID = FindProcessPID(systemEventsAppName, 0);
    if (systemEventsPID != 0) {
        err = kill(systemEventsPID, SIGKILL);
    }
#if TESTING
    if (err != noErr) {
        ShowMessage(false, false, false, "kill(systemEventsPID, SIGKILL) returned error %d ", (int) err);
    }
#endif

    return noErr;
}


// Used for OS <= 10.7
static void DeleteLoginItemOSAScript(char* user, char* appName)
{
    char                    cmd[2048];
    OSErr                   err;

    sprintf(cmd, "sudo -u \"%s\" osascript -e 'tell application \"System Events\"' -e 'delete (every login item whose name contains \"%s\")' -e 'end tell'", user, appName);
    err = callPosixSpawn(cmd);
#if TESTING
    if (err) {
        ShowMessage(false, false, false, "Command: %s returned error %d", cmd, err);
    }
#endif
}
    

#if MANIPULATE_LOGINITEM_PLIST_FILE
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
#endif // MANIPULATE_LOGINITEM_PLIST_FILE


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


#if MANIPULATE_LOGINITEM_PLIST_FILE

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
#endif // MANIPULATE_LOGINITEM_PLIST_FILE


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


// Kill a process if it exists
static int KillOneProcess(char* name) {
    pid_t   thePid;
    char cmd[MAXPATHLEN+10];
    
    thePid = FindProcessPID(name, 0);
    if (thePid <= 0) return 0;   // No such process
    
    kill(thePid, SIGQUIT);
    for (int i=0; i<20; ++i) {
        SleepSeconds(0.1);
        if (!FindProcessPID(NULL, thePid)) {
            return 0;
        }
    }
    kill(thePid, SIGKILL);
    for (int i=0; i<20; ++i) {
        SleepSeconds(0.1);
        if (!FindProcessPID(NULL, thePid)) {
            return 0;
        }
    }
    sprintf(cmd, "killall %s", name);
    return callPosixSpawn(cmd);
}


// return time of day (seconds since 1970) as a double
//
static double dtime(void) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + (tv.tv_usec/1.e6);
}

// Uses usleep to sleep for full duration even if a signal is received
static void SleepSeconds(double seconds) {
    double end_time = dtime() + seconds - 0.01;
    // sleep() and usleep() can be interrupted by SIGALRM,
    // so we may need multiple calls
    //
    while (1) {
        if (seconds >= 1) {
            sleep((unsigned int) seconds);
        } else {
            usleep((int)fmod(seconds*1000000, 1000000));
        }
        seconds = end_time - dtime();
        if (seconds <= 0) break;
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
            if (language == NULL) {
                if (CFStringGetCString(aLanguage, shortLanguage, sizeof(shortLanguage), kCFStringEncodingMacRoman)) {
                    language = shortLanguage;
                }
            }
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

    BringAppToFront();

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


#define NOT_IN_TOKEN                0
#define IN_SINGLE_QUOTED_TOKEN      1
#define IN_DOUBLE_QUOTED_TOKEN      2
#define IN_UNQUOTED_TOKEN           3

int parse_command_line(char* p, char** argv) {
    int state = NOT_IN_TOKEN;
    int argc=0;

    while (*p) {
        switch(state) {
        case NOT_IN_TOKEN:
            if (isspace(*p)) {
            } else if (*p == '\'') {
                p++;
                argv[argc++] = p;
                state = IN_SINGLE_QUOTED_TOKEN;
                break;
            } else if (*p == '\"') {
                p++;
                argv[argc++] = p;
                state = IN_DOUBLE_QUOTED_TOKEN;
                break;
            } else {
                argv[argc++] = p;
                state = IN_UNQUOTED_TOKEN;
            }
            break;
        case IN_SINGLE_QUOTED_TOKEN:
            if (*p == '\'') {
                if (*(p-1) == '\\') break;
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_DOUBLE_QUOTED_TOKEN:
            if (*p == '\"') {
                if (*(p-1) == '\\') break;
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_UNQUOTED_TOKEN:
            if (isspace(*p)) {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        }
        p++;
    }
    argv[argc] = 0;
    return argc;
}

#include <spawn.h>

int callPosixSpawn(const char *cmdline, bool delayForResult) {
    char command[1024];
    char progName[1024];
    char progPath[MAXPATHLEN];
    char* argv[100];
    int argc = 0;
    char *p;
    pid_t thePid = 0;
    int result = 0;
    int status = 0;
    extern char **environ;
    
    // Make a copy of cmdline because parse_command_line modifies it
    strlcpy(command, cmdline, sizeof(command));
    argc = parse_command_line(const_cast<char*>(command), argv);
    strlcpy(progPath, argv[0], sizeof(progPath));
    strlcpy(progName, argv[0], sizeof(progName));
    p = strrchr(progName, '/');
    if (p) {
        argv[0] = p+1;
    } else {
        argv[0] = progName;
    }
    
#if VERBOSE_TEST
    print_to_log_file("***********");
    for (int i=0; i<argc; ++i) {
        print_to_log_file("argv[%d]=%s", i, argv[i]);
    }
    print_to_log_file("***********\n");
#endif

    errno = 0;

    result = posix_spawnp(&thePid, progPath, NULL, NULL, argv, environ);
#if VERBOSE_TEST
    print_to_log_file("callPosixSpawn command: %s", cmdline);
    print_to_log_file("callPosixSpawn: posix_spawnp returned %d: %s", result, strerror(result));
#endif
    if (result) {
        return result;
    }
// CAF    int val =
    waitpid(thePid, &status, WUNTRACED);
// CAF        if (val < 0) printf("first waitpid returned %d\n", val);
    if (delayForResult) {
        // For unknow reason, we don't get the error immediately if user cancelled authorization dialog.
        SleepSeconds(0.5);
// CAF    val =
        waitpid(thePid, &status, WUNTRACED);
// CAF        if (val < 0) printf("delayed waitpid returned %d\n", val);
    }
    if (status != 0) {
#if VERBOSE_TEST
        print_to_log_file("waitpid() returned status=%d", status);
#endif
        result = status;
    } else {
        if (WIFEXITED(status)) {
            result = WEXITSTATUS(status);
            if (result == 1) {
#if VERBOSE_TEST
                print_to_log_file("WEXITSTATUS(status) returned 1, errno=%d: %s", errno, strerror(errno));
#endif
                result = errno;
            }
#if VERBOSE_TEST
            else if (result) {
                print_to_log_file("WEXITSTATUS(status) returned %d", result);
            }
#endif
        }   // end if (WIFEXITED(status)) else
    }       // end if waitpid returned 0 sstaus else
    
    return result;
}


#if VERBOSE_TEST
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
#endif	// VERBOSE_TEST


// For debugging
void print_to_log_file(const char *format, ...) {
#if VERBOSE_TEST
    FILE *f;
    va_list args;
    char buf[256];
    time_t t;
    strlcpy(buf, getenv("HOME"), sizeof(buf));
    strlcat(buf, "/Documents/test_log.txt", sizeof(buf));
    f = fopen(buf, "a");
    if (!f) return;

//  freopen(buf, "a", stdout);
//  freopen(buf, "a", stderr);

    time(&t);
    strlcpy(buf, asctime(localtime(&t)), sizeof(buf));
    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    
    fputs("\n", f);
    fflush(f);
    fclose(f);
#endif
}

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
#include <string.h>
#include <vector>
#include <string>

using std::vector;
using std::string;

#define SEARCHFORALLBOINCMANAGERS 0

#include "LoginItemAPI.h"

static OSStatus DoUninstall(void);
static OSStatus CleanupAllVisibleUsers(void);
static OSStatus DeleteOurBundlesFromDirectory(CFStringRef bundleID, char *extension, char *dirPath);
static OSStatus GetAuthorization(AuthorizationRef * authRef, const char *pathToTool, char *brandName);
static OSStatus DoPrivilegedExec(char *brandName, const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5);
static void DeleteLoginItemOSAScript(char* user, char* appName);
static void DeleteLoginItemAPI(void);
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
static OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN);
static pid_t FindProcessPID(char* name, pid_t thePID);
static OSStatus QuitOneProcess(OSType signature);
//static void SleepTicks(UInt32 ticksToSleep);
static Boolean ShowMessage(Boolean allowCancel, const char *format, ...);
#if SEARCHFORALLBOINCMANAGERS
static OSStatus GetpathToBOINCManagerApp(char* path, int maxLen, FSRef *theFSRef);
#endif


int main(int argc, char *argv[])
{
    char                        pathToSelf[MAXPATHLEN], appName[256], *p;
    ProcessSerialNumber         ourPSN;
    FSRef                       ourFSRef;
    Boolean                     cancelled = false;
    OSStatus                    err = noErr;

    // Determine whether this is the intial launch or the relaunch with privileges
    if ( (argc == 2) && (strcmp(argv[1], "--privileged") == 0) ) {
        if (geteuid() != 0) {        // Confirm that we are running as root
            ShowMessage(false, "Permission error after relaunch");
            return permErr;
        }
        
        return DoUninstall();
    }

    // This is the initial launch.  Authenticate and relaunch ourselves with privileges.
    
    // Get the full path to our executable inside this application's bundle
    err = GetCurrentProcess (&ourPSN);
    if (err == noErr) {
        err = GetProcessBundleLocation(&ourPSN, &ourFSRef);
    }
    if (err == noErr) {
        err = FSRefMakePath (&ourFSRef, (UInt8*)pathToSelf, sizeof(pathToSelf));
    }
    if (err != noErr) {
        ShowMessage(false, "Couldn't get path to self.  Error %d", err);
        return err;
    }

    // To allow for branding, assume name of executable inside bundle is same as name of bundle
    p = strrchr(pathToSelf, '/');         // Assume name of executable inside bundle is same as name of bundle
    if (p == NULL)
        p = pathToSelf - 1;
    strlcpy(appName, p+1, sizeof(appName));
    p = strrchr(appName, '.');         // Strip off bundle extension (".app")
    if (p)
        *p = '\0'; 

    strlcat(pathToSelf, "/Contents/MacOS/", sizeof(pathToSelf));
    strlcat(pathToSelf, appName, sizeof(pathToSelf));

    p = strchr(appName, ' ');
    p += 1;         // Point to brand name following "Uninstall "

    if (GetResource('ALRT', 128)) {
        // BOINC uses custom dialog with custom PICT
        cancelled = (Alert(128, NULL)  == cancel);
    } else {
        // Grid Republic uses generic dialog with Uninstall application's icon
        cancelled = ! ShowMessage(true, "Are you sure you want to completely remove %s from your computer?\n\n"
                                        "This will remove the executables but will not touch %s data files.", p, p);
    }

    if (! cancelled)
        err = DoPrivilegedExec(p, pathToSelf, "--privileged", NULL, NULL, NULL, NULL);
    
    if (cancelled || (err == errAuthorizationCanceled)) {
        ShowMessage(false, "Canceled: %s has not been touched.", p);
        return err;
    }

#if TESTING
    ShowMessage(false, "DoPrivilegedExec returned %d", err);
#endif

    if (err)
        ShowMessage(false, "An error occurred: error code %d", err);
    else
        ShowMessage(false, "Removal completed.\n\n You may want to remove the following remaining items using the Finder: \n"
         "\"/Library/Application Support/BOINC Data\" directory\n\nfor each user, the file\n"
         "\"/Users/[username]/Library/Preferences/BOINC Manager Preferences\".");
        
    return err;
}


static OSStatus DoUninstall(void) {
    pid_t                   coreClientPID = 0;
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
    ShowMessage(false, "Permission OK after relaunch");
#endif

    QuitOneProcess('BNC!'); // Quit any old instance of BOINC manager
    sleep(2);

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
        ShowMessage(false, "manager: %s", myRmCommand);
#endif

        p = strstr(myRmCommand, notBoot);
        
        if (p == myRmCommand+pathOffset) {
#if TESTING
            ShowMessage(false, "Not on boot volume: %s", myRmCommand);
#endif
            break;
        } else {

            // First delete just the application's info.plist file and update the 
            // LaunchServices Database; otherwise LSFindApplicationForInfo might 
            // return this application again after it's been deleted.
            strlcpy(plistRmCommand, myRmCommand, sizeof(plistRmCommand));
            strlcat(plistRmCommand, "/Contents/info.plist", sizeof(plistRmCommand));
#if TESTING
        ShowMessage(false, "Deleting info.plist: %s", plistRmCommand);
#endif
            system(plistRmCommand); 
            err = LSRegisterFSRef(&theFSRef, true);
#if TESTING
            if (err)
                ShowMessage(false, "LSRegisterFSRef returned error %d", err);
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
    // These first 4 should already have been deleted by the above code, but do them anyway for safety
    system ("rm -rf /Applications/BOINCManager.app");
    system ("rm -rf \"/Library/Screen Savers/BOINCSaver.saver\"");
    
    system ("rm -rf \"/Applications/GridRepublic Desktop.app\"");
    system ("rm -rf \"/Library/Screen Savers/GridRepublic.saver\"");
    
    system ("rm -rf \"/Applications/Progress\\ Thru\\ Processors\\ Desktop.app\"");
    system ("rm -rf \"/Library/Screen Savers/Progress\\ Thru\\ Processors.saver\"");
    
    system ("rm -rf \"/Applications/Charity\\ Engine\\ Desktop.app\"");
    system ("rm -rf \"/Library/Screen Savers/Charity\\ Engine.saver\"");
    
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
        ShowMessage(false, "opendir(\"%s\") failed", dirPath);
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
                                ShowMessage(false, "Bundles: %s", myRmCommand);                                
#endif

                               system(myRmCommand);
                            } else {
#if TESTING
//                                ShowMessage(false, "Bundles: Not deleting %s", myRmCommand+pathOffset);
#endif
                            }

                
                        } // if (thisID)
#if TESTING
                        else
                            ShowMessage(false, "CFBundleGetIdentifier failed for index %d", index);                                
#endif
                        CFRelease(thisBundle);                
               } //if (thisBundle)
#if TESTING
                        else
                            ShowMessage(false, "CFBundleCreate failed for index %d", index);                                
#endif
                CFRelease(bundleURLRef);                
            } // if (bundleURLRef)
#if TESTING
        else
            ShowMessage(false, "CFURLCreateWithFileSystemPath failed");                                
#endif
        
            CFRelease(urlStringRef);
        } // if (urlStringRef)
#if TESTING
        else
            ShowMessage(false, "CFStringCreateWithCString failed");                                
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
    long                OSVersion = 0;
    passwd              *pw;
    vector<string>      human_user_names;
    vector<uid_t>       human_user_IDs;
    uid_t               saved_uid, saved_euid;
    char                human_user_name[256];
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

    err = Gestalt(gestaltSystemVersion, &OSVersion);
    if (err != noErr) {
        OSVersion = 0;
    }

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
            }
            p = strchr(buf, ' ');
            if (p) {
                *p = '\0';
                human_user_names.push_back(string(buf));
                human_user_IDs.push_back((uid_t)id);
#if TESTING
                ShowMessage(false, "user ID %d: %s\n", id, buf);
#endif
                *p = ' ';
            }
        }
        pclose(f);
    }
    
    for (userIndex=human_user_names.size(); userIndex>0; --userIndex) {
        flag = 0;
        strlcpy(human_user_name, human_user_names[userIndex-1].c_str(), sizeof(human_user_name));

        // Check whether this user is a login (human) user 
        sprintf(s, "dscl . -read /Users/%s NFSHomeDirectory", human_user_name);    
        f = popen(s, "r");
        if (f) {
            while (PersistentFGets(buf, sizeof(buf), f)) {
                p = strrchr(buf, ' ');
                if (p) {
                    if (strcmp(p, " /var/empty\n") == 0) flag = 1;
                }
            }
            pclose(f);
        }

        sprintf(s, "dscl . -read /Users/%s UserShell", human_user_name);    
        f = popen(s, "r");
        if (f) {
            while (PersistentFGets(buf, sizeof(buf), f)) {
                p = strrchr(buf, ' ');
                if (p) {
                    if (strcmp(p, " /usr/bin/false\n") == 0) flag |= 2;
                }
            }
            pclose(f);
        }
        
        // Skip all non-human (non-login) users
        if (flag == 3) { // if (Home Directory == "/var/empty") && (UserShell == "/usr/bin/false")
#if TESTING
            ShowMessage(false, "Flag=3: skipping user ID %d: %s", human_user_IDs[userIndex-1], buf);
#endif
            continue;
        }

        pw = getpwnam(human_user_name);
        if (pw == NULL)             // "Deleted Users", "Shared", etc.
            continue;

#if TESTING
        ShowMessage(false, "Deleting login item for user %s: Posix name=%s, Full name=%s, UID=%d", 
                human_user_name, pw->pw_name, pw->pw_gecos, pw->pw_uid);
#endif

        // Remove user from groups boinc_master and boinc_project
        sprintf(s, "dscl . -delete /groups/boinc_master users %s", human_user_name);
        system (s);

        sprintf(s, "dscl . -delete /groups/boinc_project users %s", human_user_name);
        system (s);

#if TESTING
//    ShowMessage(false, "Before seteuid(%d) for user %s, euid = %d", pw->pw_uid, human_user_name, geteuid());
#endif
        
        if (OSVersion >= 0x1060) {
            setuid(0);
        }
        // Delete our login item(s) for this user
        if (OSVersion < 0x1070) {
            seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
            DeleteLoginItemAPI();
            seteuid(saved_euid);    // Set effective uid back to privileged user
        } else {
            // We must leave effective user ID as privileged user (root) 
            // because the target user may not be in the sudoers file.

            // We must launch the System Events application for the target user

            // Find SystemEvents process.  If found, quit it in case 
            // it is running under a different user.
            err = FindProcess ('APPL', kSystemEventsCreator, &SystemEventsPSN);
            if (err == noErr) {
#if TESTING
                ShowMessage(false, "Telling System Events to quit (before DeleteLoginItemOSAScript)");
#endif
                err = QuitOneProcess(kSystemEventsCreator);
#if TESTING
                if (err != noErr) {
                    ShowMessage(false, "QuitOneProcess(kSystemEventsCreator) returned error %d ", (int) err);
                }
#endif
            }
            
            err = LSFindApplicationForInfo(kSystemEventsCreator, NULL, NULL, &appRef, NULL);
            if (err != noErr) {
#if TESTING
                ShowMessage(false, "LSFindApplicationForInfo(kSystemEventsCreator) returned error %d ", (int) err);
#endif
            } else {
#if TESTING
                FSRefMakePath(&appRef, (UInt8*)systemEventsPath, sizeof(systemEventsPath));
                ShowMessage(false, "SystemEvents is at %s", systemEventsPath);
                ShowMessage(false, "Lunching SystemEvents for user %s", human_user_name);
#endif

                sprintf(cmd, "sudo -u %s \"%s/Contents/MacOS/System Events\" &", human_user_name, systemEventsPath);
                err = system(cmd);
#if TESTING
                if (err) {
                    ShowMessage(false, "[2] Command: %s returned error %d", cmd, (int) err);
                }
#endif
            }

            DeleteLoginItemOSAScript(human_user_name, "BOINCManager");
            DeleteLoginItemOSAScript(human_user_name, "GridRepublic Desktop");
            DeleteLoginItemOSAScript(human_user_name, "Progress Thru Processors Desktop");
            DeleteLoginItemOSAScript(human_user_name, "Charity Engine Desktop");
        }
        
        // We don't delete the user's BOINC Manager preferences
//        sprintf(s, "rm -f \"/Users/%s/Library/Preferences/BOINC Manager Preferences\"", human_user_name);
//        system (s);
        
        //  Set screensaver to "Computer Name" default screensaver only 
        //  if it was BOINC, GridRepublic, Progress Thru Processors or Charity Engine.
        changeSaver = false;
        if (OSVersion < 0x1060) {
            seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
            f = popen("defaults -currentHost read com.apple.screensaver moduleName", "r");
        } else {
            // We must leave effective user ID as privileged user (root) 
            // because the target user may not be in the sudoers file.
            sprintf(s, "sudo -u %s defaults -currentHost read com.apple.screensaver moduleDict -dict", 
                    human_user_name); 
            f = popen(s, "r");
        }
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
        
        if (changeSaver) {
            if (OSVersion < 0x1060) {
                system ("defaults -currentHost write com.apple.screensaver moduleName \"Computer Name\"");
                system ("defaults -currentHost write com.apple.screensaver modulePath \"/System/Library/Frameworks/ScreenSaver.framework/Versions/A/Resources/Computer Name.saver\"");
            } else {
                sprintf(s, 
                    "sudo -u %s defaults -currentHost write com.apple.screensaver moduleDict -dict moduleName \"Computer Name\" path \"/System/Library/Frameworks/ScreenSaver.framework/Versions/A/Resources/Computer Name.saver\"", 
                    human_user_name);
                system (s);
            }
        }

        if (OSVersion < 0x1060) {
            seteuid(saved_euid);    // Set effective uid back to privileged user
        }
        
#if TESTING
//    ShowMessage(false, "After seteuid(%d) for user %s, euid = %d, saved_uid = %d", pw->pw_uid, human_user_name, geteuid(), saved_uid);
#endif
    }
    
    if (OSVersion >= 0x1070) {
#if TESTING
        ShowMessage(false, "Telling System Events to quit (at end)");
#endif
        err = QuitOneProcess(kSystemEventsCreator);
#if TESTING
        if (err != noErr) {
            ShowMessage(false, "QuitOneProcess(kSystemEventsCreator) returned error %d ", (int) err);
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
    
    sprintf(prompt, "Enter administrator password to completely remove %s from you computer.\n\n", brandName);

    ourAuthRights.count = 0;
    ourAuthRights.items = NULL;

    err = AuthorizationCreate (&ourAuthRights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, authRef);
    if (err != noErr) {
        ShowMessage(false, "AuthorizationCreate returned error %d", err);
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
            ShowMessage(false, "GetAuthorization returned error %d", err);
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
            } while (p);
            
            fclose (ioPipe);
        }
        
        if (err == noErr)
            break;
    }

if (err != noErr)
    ShowMessage(false, "\"%s %s %s %s %s %s\" returned error %d", pathToTool, 
                        arg1 ? arg1 : "", arg2 ? arg2 : "", arg3 ? arg3 : "", 
                        arg4 ? arg4 : "", arg5 ? arg5 : "", err);

   return err;
}


static void DeleteLoginItemOSAScript(char* user, char* appName)
{
    char                    cmd[2048];
    OSErr                   err;

    sprintf(cmd, "sudo -u %s osascript -e 'tell application \"System Events\"' -e 'delete (every login item whose name contains \"%s\")' -e 'end tell'", user, appName);
    err = system(cmd);
#if TESTING
    if (err) {
        ShowMessage(false, "Command: %s returned error %d", cmd, err);
    }
#endif
}
    
    
static void DeleteLoginItemAPI(void)
{
    Boolean                 Success;
    int                     NumberOfLoginItems, Counter;
    char                    *p, *q;

    Success = false;

    NumberOfLoginItems = GetCountOfLoginItems(kCurrentUser);
    
    // Search existing login items in reverse order, deleting ours
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
            
        if (strcmp(p, "BOINCMANAGER.APP") == 0)
            Success = RemoveLoginItemAtIndex(kCurrentUser, Counter-1);
        if (strcmp(p, "GRIDREPUBLIC DESKTOP.APP") == 0)
            Success = RemoveLoginItemAtIndex(kCurrentUser, Counter-1);
        if (strcmp(p, "PROGRESS THRU PROCESSORS DESKTOP.APP") == 0)
            Success = RemoveLoginItemAtIndex(kCurrentUser, Counter-1);
        if (strcmp(p, "CHARITY ENGINE DESKTOP.APP") == 0)
            Success = RemoveLoginItemAtIndex(kCurrentUser, Counter-1);
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
            done = true;		// apparently the demo app isn't running.  Odd but not impossible
        else {		
            err = GetProcessInformation(&thisPSN,&thisPIR);
            if (err != noErr)
                goto bail;
                    
            if (thisPIR.processSignature == signature) {	// is it or target process?
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

#if 0
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
#endif


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
    alertParams.defaultText = (StringPtr)-1;
    alertParams.cancelText = allowCancel ? (StringPtr)-1 : NULL;
    alertParams.otherText = NULL;
    alertParams.defaultButton = kAlertStdAlertOKButton;
    alertParams.cancelButton = allowCancel ? kAlertStdAlertCancelButton : 0;
    alertParams.position = kWindowDefaultPosition;

    ::GetCurrentProcess (&ourProcess);
    ::SetFrontProcess(&ourProcess);

    StandardAlert (kAlertNoteAlert, (StringPtr)s, NULL, &alertParams, &itemHit);
    
    return (itemHit == kAlertStdAlertOKButton);
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
            ShowMessage(false, "LSFindApplicationForInfo returned error %d", status);
#endif 
            return status;
        }
    
        status = FSRefMakePath(theFSRef, (unsigned char *)path, maxLen);
#if TESTING
        if (status)
            ShowMessage(false, "GetpathToBOINCManagerApp FSRefMakePath returned error %d, %s", status, path);
#endif
    
    return status;
}
#endif  // SEARCHFORALLBOINCMANAGERS

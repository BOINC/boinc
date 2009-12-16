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

#include "LoginItemAPI.h"  //please take a look at LoginItemAPI.h for an explanation of the routines available to you.

static OSStatus DoUninstall(void);
static OSStatus GetpathToBOINCManagerApp(char* path, int maxLen, FSRef *theFSRef);
static OSStatus CleanupAllVisibleUsers(void);
static OSStatus DeleteOurBundlesFromDirectory(CFStringRef bundleID, char *extension, char *dirPath);
static OSStatus GetAuthorization(AuthorizationRef * authRef, const char *pathToTool, char *brandName);
static OSStatus DoPrivilegedExec(char *brandName, const char *pathToTool, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5);
static void DeleteLoginItem(void);
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
static pid_t FindProcessPID(char* name, pid_t thePID);
static OSStatus QuitBOINCManager(OSType signature);
//static void SleepTicks(UInt32 ticksToSleep);
static Boolean ShowMessage(Boolean allowCancel, const char *format, ...);


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
    char                    myRmCommand[MAXPATHLEN+10], plistRmCommand[MAXPATHLEN+10], *p;
    char                    notBoot[] = "/Volumes/";
    char                    cmd[1024];
    FSRef                   theFSRef;
    int                     pathOffset, i;
    passwd                  *pw;
    OSStatus                err = noErr;

#if TESTING
    ShowMessage(false, "Permission OK after relaunch");
#endif

    QuitBOINCManager('BNC!'); // Quit any old instance of BOINC manager
    sleep(2);

    // Core Client may still be running if it was started without Manager
    coreClientPID = FindProcessPID("boinc", 0);
    if (coreClientPID)
        kill(coreClientPID, SIGTERM);   // boinc catches SIGTERM & exits gracefully

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

    // Phase 2: step through default Applications directory searching for our applications
    err = DeleteOurBundlesFromDirectory(CFSTR("edu.berkeley.boinc"), "app", "/Applications");

    // Phase 3: step through default Screen Savers directory searching for our screen savers
    err = DeleteOurBundlesFromDirectory(CFSTR("edu.berkeley.boincsaver"), "saver", "/Library/Screen Savers");

    // Phase 4: Delete our files and directories at our installer's default locations
    // Remove everything we've installed, whether BOINC, GridRepublic or Progress Thru Processors
    // These first 4 should already have been deleted by the above code, but do them anyway for safety
    system ("rm -rf /Applications/BOINCManager.app");
    system ("rm -rf \"/Library/Screen Savers/BOINCSaver.saver\"");
    
    system ("rm -rf \"/Applications/GridRepublic Desktop.app\"");
    system ("rm -rf \"/Library/Screen Savers/GridRepublic.saver\"");
    
    system ("rm -rf \"/Applications/Progress\\ Thru\\ Processors\\ Desktop.app\"");
    system ("rm -rf \"/Library/Screen Savers/Progress\\ Thru\\ Processors.saver\"");


    system ("rm -rf /Library/Receipts/GridRepublic.pkg");
    system ("rm -rf /Library/Receipts/Progress\\ Thru\\ Processors.pkg");
    system ("rm -rf /Library/Receipts/BOINC.pkg");

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


// Find all visible users and delete their login item to launch BOINC Manager.
// Remove each user from groups boinc_master and boinc_project.
// For now, don't delete user's BOINC Preferences file.
static OSStatus CleanupAllVisibleUsers(void)
{
    DIR                 *dirp;
    dirent              *dp;
    passwd              *pw;
    uid_t               saved_uid, saved_euid;
    char                s[1024];
    FILE                *f;
    long                OSVersion = 0;
    OSStatus            err;
    Boolean             changeSaver;

    err = Gestalt(gestaltSystemVersion, &OSVersion);
    if (err != noErr) {
        OSVersion = 0;
    }

    dirp = opendir("/Users");
    if (dirp == NULL) {      // Should never happen
        ShowMessage(false, "opendir(\"/Users\") failed");
        return -1;
    }
    
    saved_uid = getuid();
    saved_euid = geteuid();

    while (true) {
        dp = readdir(dirp);
        if (dp == NULL)
            break;                  // End of list
            
        if (dp->d_name[0] == '.')
            continue;               // Ignore names beginning with '.'
    
        pw = getpwnam(dp->d_name);
        if (pw == NULL)             // "Deleted Users", "Shared", etc.
            continue;

        // Remove user from groups boinc_master and boinc_project
        sprintf(s, "dscl . -delete /groups/boinc_master users %s", dp->d_name);
        system (s);

        sprintf(s, "dscl . -delete /groups/boinc_project users %s", dp->d_name);
        system (s);

#if TESTING
//    ShowMessage(false, "Before seteuid(%d) for user %s, euid = %d", pw->pw_uid, dp->d_name, geteuid());
#endif
        
        if (OSVersion >= 0x1060) {
            setuid(0);
        }
        seteuid(pw->pw_uid);                        // Temporarily set effective uid to this user

        DeleteLoginItem();                          // Delete our login item(s) for this user

//        sprintf(s, "rm -f \"/Users/%s/Library/Preferences/BOINC Manager Preferences\"", dp->d_name);
//        system (s);
        
        //  Set screensaver to "Computer Name" default screensaver only 
        //  if it was BOINC, GridRepublic or Progress Thru Processors.
        changeSaver = false;
        if (OSVersion < 0x1060) {
            f = popen("defaults -currentHost read com.apple.screensaver moduleName", "r");
        } else {
            sprintf(s, "sudo -u %s defaults -currentHost read com.apple.screensaver moduleDict -dict", 
                    dp->d_name); 
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
                    dp->d_name);
                system (s);
            }
        }
        
        seteuid(saved_euid);                         // Set effective uid back to privileged user
#if TESTING
//    ShowMessage(false, "After seteuid(%d) for user %s, euid = %d, saved_uid = %d", pw->pw_uid, dp->d_name, geteuid(), saved_uid);
#endif
    }
    
    closedir(dirp);
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


static void DeleteLoginItem(void)
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


static OSStatus QuitBOINCManager(OSType signature) {
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

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
/* if (TESTING!=0), set DEBUG_PRINT to 1 to print debug output to log file or 0 to display in dialogs */
#define DEBUG_PRINT 1

#define USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS false

#include <Carbon/Carbon.h>
#import <ServiceManagement/ServiceManagement.h>
#import <Security/Authorization.h>

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

#define USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS false

#include "mac_util.h"
#include "translate.h"
#include "file_names.h"
#include "mac_branding.h"


static OSStatus DoUninstall(void);
static OSStatus CleanupAllVisibleUsers(void);
static OSStatus DeleteOurBundlesFromDirectory(CFStringRef bundleID, char *extension, char *dirPath, Boolean deleteSymLink);
static void DeleteLoginItemOSAScript(char *userName);
static Boolean DeleteLoginItemLaunchAgent(long brandID, passwd *pw);
static void DeleteScreenSaverLaunchAgent(passwd *pw);
static void FixLaunchServicesDataBase(uid_t userID, char *theBundleID);
long GetBrandID(char *path);
static Boolean IsUserLoggedIn(const char *userName);
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
OSErr GetCurrentScreenSaverSelection(passwd *pw, char *moduleName, size_t maxLen);
OSErr SetScreenSaverSelection(char *moduleName, char *modulePath, int type);
static pid_t FindProcessPID(char* name, pid_t thePID);
static int KillOneProcess(char* name);
static double dtime(void);
static void SleepSeconds(double seconds);
static void GetPreferredLanguages();
static void LoadPreferredLanguages();
static void showDebugMsg(const char *format, ...);
static Boolean ShowMessage(Boolean allowCancel, Boolean continueButton, Boolean yesNoButtons, const char *format, ...);
int callPosixSpawn(const char *cmd, bool delayForResult=false);
static void print_to_log_file(const char *format, ...);

static char gAppName[256];
static char gBrandName[256];
static char gCatalogsDir[MAXPATHLEN];
static char * gCatalog_Name = (char *)"BOINC-Setup";
static char loginName[256];


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
    Boolean                     isPrivileged = false;
    char *                      myArgv[2];
    struct stat                 sbuf;
    FILE                        *f = NULL;
    FILE                        *pipe = NULL;
    OSStatus                    err = noErr;

    FILE * stdout_file = freopen("/tmp/BOINC_Uninstall_log.txt", "w", stdout);
    if (stdout_file) {
        setbuf(stdout_file, 0);
    }

    FILE * stderr_file = freopen("/tmp/BOINC_Uninstall_log.txt", "a", stderr);
    if (stderr_file) {
        setbuf(stderr_file, 0);
    }

    pathToSelf[0] = '\0';
    // Get the full path to our executable inside this application's bundle
    getPathToThisApp(pathToSelf, sizeof(pathToSelf));
    if (!pathToSelf[0]) {
        ShowMessage(false, false, false, "Couldn't get path to self.");
        return err;
    }
    strlcpy(pathToVBoxUninstallTool, pathToSelf, sizeof(pathToVBoxUninstallTool));
    strlcat(pathToVBoxUninstallTool, "/Contents/Resources/VirtualBox_Uninstall.tool", sizeof(pathToVBoxUninstallTool));

    if (!check_branding_arrays(cmd, sizeof(cmd))) {
        ShowMessage(false, false, false, (char *)_("Branding array has too few entries: %s"), cmd);
        return -1;
    }

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

    // Determine whether this is the initial launch or the relaunch with privileges
    for (int i=0; i<argc; ++i) {
        if (strcmp(argv[i], "--privileged") == 0) {
            isPrivileged = true;
            break;
        }
    }
    if (isPrivileged) {
        setuid(0);  // This is permitted bcause euid == 0

        LoadPreferredLanguages();

        if (geteuid() != 0) {        // Confirm that we are running as root
            ShowMessage(false, false, false, (char *)_("Permission error after relaunch"));
            err = permErr;
        }

        if (!err) {
            ShowMessage(false, true, false, (char *)_("Removal may take several minutes.\nPlease be patient."));
            err = DoUninstall();
        }

        BOINCTranslationCleanup();
        return err;
    }

    // This is the initial launch.  Authenticate and relaunch ourselves with privileges.

    // getlogin() gives unreliable results under OS 10.6.2, so use environment
    strncpy(loginName, getenv("USER"), sizeof(loginName)-1);
    mkdir("/tmp/UninstallBOINC", 0777);
    chmod("/tmp/UninstallBOINC", 0777);  // Needed because mkdir sets permissions restricted by umask (022)
    f = fopen("/tmp/UninstallBOINC/loginName", "w");
    if (!f) {
        ShowMessage(false, false, false, (char *)_("Error saving user name"), p);
        return fnOpnErr;
    }
    fprintf(f, "%s", loginName);
    fclose(f);

    GetPreferredLanguages();    // We must do this before switching to root user
    LoadPreferredLanguages();

    // Grid Republic uses generic dialog with Uninstall application's icon
    cancelled = ! ShowMessage(true, true, false, (char *)_(
            "Are you sure you want to completely remove %s from your computer?\n\n"
            "This will remove the executables but will not touch %s data files."), p, p);

    if (! cancelled) {
        // TODO: Upgrade this to use SMJobBless
        // I have been trying to get the SMJobBless sample application to work
        // but have not yet succeeded.
        // Although SMJobBless is deprecated, the currently recomended SMAppService
        // API is available only since MacOS 13.0, and we want to suppport older
        // versions of MacOS. Also, it is not clear from the documentation how
        // SMAppService allows you to run a helper tool as root after presenting
        // an authorization dialog to the user.
        myArgv[0] = "--privileged";
        myArgv[1] = NULL;
        AuthorizationItem authItem		= { kAuthorizationRightExecute, 0, NULL, 0 };
        AuthorizationRights authRights	= { 1, &authItem };
        AuthorizationFlags flags		=	kAuthorizationFlagInteractionAllowed	|
                                            kAuthorizationFlagExtendRights;

        AuthorizationRef authRef = NULL;

        /* Obtain the right to install privileged helper tools (kSMRightBlessPrivilegedHelper). */
        err = AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment, flags, &authRef);
        if (err == errAuthorizationSuccess) {
            err = AuthorizationExecuteWithPrivileges(authRef, pathToSelf, kAuthorizationFlagDefaults, myArgv, &pipe);
        }
        if (err == errAuthorizationSuccess) {
            char buf[1024];

            // TODO: Handle errors from privileged uninstall
            while (true) {
                fgets(buf, sizeof(buf), pipe);  // Receive stdout from privileged uninstall
                if (feof(f)) {
                    break;
                }
                puts(buf);  // Write stdout from privileged uninstall to stdout_file;
            }
            pclose(pipe);   // Should this be fclose(pipe)?
        }
    }

    if (cancelled || (err != noErr)) {
        ShowMessage(false, false, false, (char *)_("Canceled: %s has not been touched."), p);
        BOINCTranslationCleanup();
        return err;
    }

    CFStringRef CFBOINCDataPath, CFUserPrefsPath;
    char BOINCDataPath[MAXPATHLEN], PodmanDataPath[MAXPATHLEN], temp[MAXPATHLEN], PathToPrefs[MAXPATHLEN];
    Boolean success = false;

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
        strlcpy(PodmanDataPath, BOINCDataPath, sizeof(PodmanDataPath));
        strlcat(BOINCDataPath, "/BOINC Data", sizeof(BOINCDataPath));
        strlcat(PodmanDataPath, "/BOINC podman", sizeof(PodmanDataPath));
    } else {
        strlcpy(BOINCDataPath,
                "/Library/Application Support/BOINC Data",
                sizeof(BOINCDataPath));
        strlcpy(PodmanDataPath,
                "/Library/Application Support/BOINC podman",
                sizeof(PodmanDataPath));
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

    // TODO: Change this message if there were errors in the privileged app
    ShowMessage(false, false, false, (char *)_("Removal completed.\n\n You may want to remove the following remaining items using the Finder:\n\n "
     "the directory \"%s\"\n\nthe directory \"%s\"\n\nfor each user, the file\n"
     "\"%s\"."), BOINCDataPath, PodmanDataPath, PathToPrefs);

    BOINCTranslationCleanup();
    return err;
}


static OSStatus DoUninstall(void) {
    FILE                        *f;
    pid_t                   coreClientPID = 0;
    int                     i;
    char                    cmd[1024];
    passwd                  *pw;
    OSStatus                err __attribute__((unused)) = noErr;
#if SEARCHFORALLBOINCMANAGERS
    char                    myRmCommand[MAXPATHLEN+10], plistRmCommand[MAXPATHLEN+10];
    char                    notBoot[] = "/Volumes/";
    CFStringRef             cfPath;
    CFURLRef                appURL;
    int                     pathOffset;
#endif

fprintf(stderr, "Starting privileged tool\n");
fprintf(stdout, "Starting privileged tool\n");
#if TESTING
    showDebugMsg("Permission OK after relaunch");
#endif

    f = fopen("/tmp/UninstallBOINC/loginName", "r");
    if (f) {
        fgets(loginName, sizeof(loginName), f);
        fclose(f);
    }

    // With fast user switching, each logged in user can be running
    // a separate copy of the Manager; killall terminates all of them
    for (i=0; i<NUMBRANDS; ++i) {
#if TESTING
        showDebugMsg("killing any running instances of %s", appName[i]);
#endif
        snprintf(cmd, sizeof(cmd), "killall \"%s\"", appName[i]);
        callPosixSpawn(cmd);
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
        showDebugMsg("manager: %s", myRmCommand);
#endif

        p = strstr(myRmCommand, notBoot);

        if (p == myRmCommand+pathOffset) {
#if TESTING
            showDebugMsg("Not on boot volume: %s", myRmCommand);
#endif
            break;
        } else {

            // First delete just the application's info.plist file and update the
            // LaunchServices Database; otherwise GetPathToAppFromID might return
            // this application again after it's been deleted.
            strlcpy(plistRmCommand, myRmCommand, sizeof(plistRmCommand));
            strlcat(plistRmCommand, "/Contents/info.plist", sizeof(plistRmCommand));
#if TESTING
        showDebugMsg("Deleting info.plist: %s", plistRmCommand);
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
                showDebugMsg("LSRegisterFSRef returned error %d", err);
#endif
            callPosixSpawn(myRmCommand);
        }
    }
#endif  // SEARCHFORALLBOINCMANAGERS

    // Phase 2: step through default Applications directory searching for our applications
    // We installed the BOINC Manager in "/Library/Application Support" with a
    // soft link to it from the /Applications directory. For an explanation why
    // we do it this way see the comment in CBOINCGUIApp::OnInit() under
    // "if (DetectDuplicateInstance())"
    err = DeleteOurBundlesFromDirectory(CFSTR("edu.berkeley.boinc"), "app", "/Library/Application Support", true);
    // Older installations put BOINC Manager in /Applications
    err = DeleteOurBundlesFromDirectory(CFSTR("edu.berkeley.boinc"), "app", "/Applications", false);

    // Phase 3: step through default Screen Savers directory searching for our screen savers
    err = DeleteOurBundlesFromDirectory(CFSTR("edu.berkeley.boincsaver"), "saver", "/Library/Screen Savers", false);

    // Phase 4: Delete our files and directories at our installer's default locations
    // Remove everything we may have installed, though the above 2 calls already deleted some

    for (i=0; i<NUMBRANDS; ++i) {
        sprintf(cmd, "rm -rf \"%s\"", appPath[i]);
        callPosixSpawn(cmd);
        sprintf(cmd, "rm -rf \"/Library/Screen Savers/%s.saver\"", saverName[i]);
        callPosixSpawn(cmd);
    }

    for (i=0; i<NUMBRANDS; ++i) {
        // NOTE: the following work for older versions of OS X, but newer versions
        // of OS X store the receipts elsewhere. However, this step is probably
        // not needed to allow installing older versions of BOINC over newer ones
        // with more recent versions of OS X.
        // Delete any receipt from a very old BOINC Installer
        sprintf(cmd, "rm -rf \"/Library/Receipts/%s\".pkg", brandName[i]);
        callPosixSpawn(cmd);

        // Delete any receipt from a newer BOINC installer (which has
        // a wrapper application around the installer package.)
        sprintf(cmd, "rm -rf \"%s\"", receiptName[i]);
        callPosixSpawn(cmd);
    }

    // Phase 5: Set BOINC Data owner and group to logged in user
    // We don't customize BOINC Data directory name for branding
//    callPosixSpawn ("rm -rf \"/Library/Application Support/BOINC Data\"");
    pw = getpwnam(loginName);
    sprintf(cmd, "chown -R %d:%d \"/Library/Application Support/BOINC Data\"", pw->pw_uid, pw->pw_gid);
    callPosixSpawn (cmd);
    callPosixSpawn("chmod -R u+rw-s,g+r-w-s,o+r-w \"/Library/Application Support/BOINC Data\"");
    callPosixSpawn("chmod 600 \"/Library/Application Support/BOINC Data/gui_rpc_auth.cfg\"");

    // Phase 6: step through all users and do user-specific cleanup
    CleanupAllVisibleUsers();

    // Use of sudo here may help avoid a warning alert from MacOS
    callPosixSpawn ("sudo dscl . -delete /users/boinc_master");
    callPosixSpawn ("sudo dscl . -delete /groups/boinc_master");
    callPosixSpawn ("sudo dscl . -delete /users/boinc_project");
    callPosixSpawn ("sudo dscl . -delete /groups/boinc_project");

    return 0;
}


static OSStatus DeleteOurBundlesFromDirectory(CFStringRef bundleID, char *extension, char *dirPath, Boolean deleteSymLink) {
    DIR                     *dirp;
    dirent                  *dp;
    CFStringRef             urlStringRef = NULL;
    CFStringRef             thisID = NULL;
    CFBundleRef             thisBundle = NULL;
    CFURLRef                bundleURLRef = NULL;
    char                    myRmCommand[MAXPATHLEN+10], *p;
    int                     pathOffset;
#if TESTING
    int                     index = -1;
#endif

    dirp = opendir(dirPath);
    if (dirp == NULL) {      // Should never happen
        showDebugMsg("Error: opendir(\"%s\") failed", dirPath);
        return -1;
    }

    while (true) {
#if TESTING
        index++;
#endif
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
                                showDebugMsg("Bundles: %s", myRmCommand);
#endif

                                callPosixSpawn(myRmCommand);
                                if (deleteSymLink) {
                                    strlcpy(myRmCommand, "rm -rf \"/Applications/", sizeof(myRmCommand));
                                    strlcat(myRmCommand, dp->d_name, sizeof(myRmCommand));
                                    strlcat(myRmCommand, "\"", sizeof(myRmCommand));
                                    callPosixSpawn(myRmCommand);
                                }
                            } else {
#if TESTING
//                                showDebugMsg("Bundles: Not deleting %s", myRmCommand+pathOffset);
#endif
                            }


                        } // if (thisID)
#if TESTING
                        else
                            showDebugMsg("CFBundleGetIdentifier failed for index %d", index);
#endif
                        CFRelease(thisBundle);
               } //if (thisBundle)
#if TESTING
                        else
                            showDebugMsg("CFBundleCreate failed for index %d", index);
#endif
                CFRelease(bundleURLRef);
            } // if (bundleURLRef)
#if TESTING
        else
            showDebugMsg("CFURLCreateWithFileSystemPath failed");
#endif

            CFRelease(urlStringRef);
        } // if (urlStringRef)
#if TESTING
        else
            showDebugMsg("CFStringCreateWithCString failed");
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
    long                brandID = 0;
   passwd              *pw;
    vector<string>      human_user_names;
    vector<uid_t>       human_user_IDs;
//    uid_t               saved_uid;
    uid_t               saved_euid;
    char                human_user_name[256];
    int                 i;
    int                 userIndex;
    int                 flag;
    char                buf[256];
    char                s[2*MAXPATHLEN];
    FILE                *f;
    char                *p;
    int                 id;
    OSStatus            err;
    Boolean             changeSaver;
    Boolean             isCatalinaOrLater = (compareOSVersionTo(10, 15) >= 0);
    Boolean             hadLoginItemLaunchAgent = false;

//    saved_uid = getuid();
    saved_euid = geteuid();

    err = noErr;

    // The branding is in the resources of this uninstall app
    getPathToThisApp(s, sizeof(s));
    strncat(s, "/Contents/Resources/Branding", sizeof(s)-1);
    brandID = GetBrandID(s);
    fflush(stdout);

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
                showDebugMsg("user ID %d: %s\n", id, buf);
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
                    if (strstr(p, "/var/empty") != NULL) {
                        flag = 1;
                        break;
                    }
                }
            }
            pclose(f);
        }

        if (flag) {
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
        }
        // Skip all non-human (non-login) users
        if (flag == 3) { // if (Home Directory == "/var/empty") && (UserShell == "/usr/bin/false")
#if TESTING
            showDebugMsg("Flag=3: skipping user ID %d: %s", human_user_IDs[userIndex-1], buf);
#endif
            continue;
        }

        pw = getpwnam(human_user_name);
        if (pw == NULL)
            continue;

#if TESTING
        showDebugMsg("Deleting login item for user %s: Posix name=%s, Full name=%s, UID=%d",
                human_user_name, pw->pw_name, pw->pw_gecos, pw->pw_uid);
#endif

        // Remove user from groups boinc_master and boinc_project
        sprintf(s, "sudo dscl . -delete /groups/boinc_master GroupMembership \"%s\"", pw->pw_name);
        callPosixSpawn (s);

        sprintf(s, "sudo dscl . -delete /groups/boinc_project GroupMembership \"%s\"", pw->pw_name);
        callPosixSpawn (s);

       // Set login item for this user
        bool useOSASript = false;

        if ((compareOSVersionTo(10, 13) < 0)
            || (strcmp(loginName, human_user_name) == 0)
                || (strcmp(loginName, pw->pw_name) == 0)
                    || (strcmp(loginName, pw->pw_gecos) == 0)) {
            useOSASript = true;
        }
#if USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS
        if (! useOSASript) {
            useOSASript = IsUserLoggedIn(pw->pw_name);
        }
#endif
       if (useOSASript) {
            hadLoginItemLaunchAgent = false;
            snprintf(s, sizeof(s), "/Users/%s/Library/LaunchAgents/edu.berkeley.launchboincmanager.plist", pw->pw_name);
            if (boinc_file_exists(s)) hadLoginItemLaunchAgent = true;
            boinc_delete_file(s);
            // DeleteLoginItemOSAScript uses "System Events" which can trigger an aert which
            // the user may find alraming. If we previously set a login item launch agent,
            // we removed the old style login item at that time, so we avoid that alert.
            if (!hadLoginItemLaunchAgent) {
#if TESTING
                showDebugMsg("calling DeleteLoginItemOSAScript for user %s, euid = %d\n",
                    pw->pw_name, geteuid());
#endif
                DeleteLoginItemOSAScript(pw->pw_name);
            }

            // Under OS 10.13 High Sierra or later, this code deletes the per-user BOINC
            // Manager files only for the user running this app. For each user other than
            // one running this app, we put BOINCManager_Finish_Uninstall in its per-user
            // BOINC directory, so we can't delete it now. BOINCManager_Finish_Uninstall
            // will delete that user's per-user BOINC directory as its final task.
            sprintf(s, "rm -fR \"/Users/%s/Library/Application Support/BOINC\"", pw->pw_name);
            callPosixSpawn (s);
        } else {
#if TESTING
            showDebugMsg("calling DeleteLoginItemLaunchAgent for user %s, euid = %d\n",
                pw->pw_name, geteuid());
#endif
            DeleteLoginItemLaunchAgent(brandID, pw);
        }

        if (compareOSVersionTo(10, 13) >= 0) {
            sprintf(s, "rm -f \"/Users/%s/Library/LaunchAgents/edu.berkeley.launchboincmanager.plist\"", pw->pw_name);
            callPosixSpawn (s);
        }

#if TESTING
        showDebugMsg("calling DeleteScreenSaverLaunchAgent for user %s, euid = %d\n",
                pw->pw_name);
#endif
        DeleteScreenSaverLaunchAgent(pw);

        // We don't delete the user's BOINC Manager preferences
//        sprintf(s, "rm -f \"/Users/%s/Library/Preferences/BOINC Manager Preferences\"", human_user_name);
//        callPosixSpawn (s);

        //  Set screensaver to "Flurry" screensaver only
        //  if it was BOINC unbranded or branded screensaver.
        changeSaver = false;

        if (isCatalinaOrLater) {
            // As of Catalina, Screensaver output files are put in the user's Containers
            // directory.
            snprintf(s, sizeof(s), "rm -fR \"/Users/%s/Library/Containers/com.apple.ScreenSaver.Engine.legacyScreenSaver/Data/Library/Application Support/BOINC\"",
                    pw->pw_name);
            callPosixSpawn(s);
        }

        err = GetCurrentScreenSaverSelection(pw, s, sizeof(s) -1);
        if (err == noErr) {
            for (i=0; i<NUMBRANDS; ++i) {
                if (strcmp(s, saverName[i]) == 0) {
                    changeSaver = true;
                }
            }
#if TESTING
            showDebugMsg("Current screensaver selection for user %s is %s\n", pw->pw_name, s);
#endif
        }

        if (changeSaver) {
#if TESTING
            showDebugMsg("Setting screensaver for user %s to Flurry\n", pw->pw_name);
#endif
            seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
            err = SetScreenSaverSelection("Flurry", "/System/Library/Screen Savers/Flurry.saver", 0);
            seteuid(saved_euid);    // Set effective uid back to privileged user
            // This seems to work also:
            // sprintf(s, "su -l \"%s\" -c 'defaults -currentHost write com.apple.screensaver moduleDict -dict moduleName \"Computer Name\" path \"/System/Library/Frameworks/ScreenSaver.framework/Resources/Computer Name.saver"\" type 0'");
            // callPosixSpawn(s);
#if TESTING
//    showDebugMsg("After seteuid(%d) for user %s, euid = %d, saved_uid = %d", pw->pw_uid, human_user_name, geteuid(), saved_uid);
#endif
        }
    }       // End userIndex loop

    sleep(1);

    return noErr;
}


static void DeleteLoginItemOSAScript(char *userName)
{
    int                     i, j;
    char                    cmd[2048];
    char                    systemEventsPath[1024];
    pid_t                   systemEventsPID;
    OSErr                   err, err2;
#if USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS
    Boolean                 isHighSierraOrLater = (compareOSVersionTo(10, 13) >= 0);
    // NOTE: It may not be necessary to kill and relaunch the
    // System Events application for each logged in user under High Sierra
#endif

#if TESTING
    showDebugMsg("Adjusting login items for user %s\n", userName);
#endif

    // We must launch the System Events application for the target user
    err = noErr;
    systemEventsPath[0] = '\0';

    err = GetPathToAppFromID(kSystemEventsCreator, kSystemEventsBundleID, systemEventsPath, sizeof(systemEventsPath));

#if TESTING
    if (err == noErr) {
        showDebugMsg("SystemEvents is at %s\n", systemEventsPath);
    } else {
        showDebugMsg("GetPathToAppFromID(kSystemEventsCreator, kSystemEventsBundleID) returned error %d ", (int) err);
    }
#endif

    if (err == noErr) {
        // Find SystemEvents process.  If found, quit it in case
        // it is running under a different user.
#if TESTING
        showDebugMsg("Telling System Events to quit (at start of DeleteLoginItemOSAScript)\n");
#endif
        systemEventsPID = FindProcessPID(systemEventsAppName, 0);
        if (systemEventsPID != 0) {
            err = kill(systemEventsPID, SIGKILL);
        }
#if TESTING
        if (err != noErr) {
            showDebugMsg("(systemEventsPID, SIGKILL) returned error %d \n", (int) err);
        }
#endif
        // Wait for the process to be gone
        for (i=0; i<50; ++i) {      // 5 seconds max delay
            SleepSeconds(0.1);      // 1/10 second
            systemEventsPID = FindProcessPID(systemEventsAppName, 0);
            if (systemEventsPID == 0) break;
        }
        if (i >= 50) {
#if TESTING
            showDebugMsg("Failed to make System Events quit\n");
#endif
            err = noErr;
            goto cleanupSystemEvents;
        }
        sleep(4);
    }

    if (systemEventsPath[0] != '\0') {
 #if TESTING
        showDebugMsg("Launching SystemEvents for user %s\n", userName);
#endif

        for (j=0; j<5; ++j) {
            sprintf(cmd, "sudo -u \"%s\" open \"%s\"", userName, systemEventsPath);
            err = callPosixSpawn(cmd);
            if (err) {
 #if TESTING
                showDebugMsg("Command: %s returned error %d (try %d of 5)\n", cmd, (int) err, j);
#endif
            }
            // Wait for the process to start
            for (i=0; i<50; ++i) {      // 5 seconds max delay
                SleepSeconds(0.1);      // 1/10 second
                systemEventsPID = FindProcessPID(systemEventsAppName, 0);
                if (systemEventsPID != 0) break;
            }
            if (i < 50) break;  // Exit j loop on success
        }
        if (j >= 5) {
#if TESTING
            showDebugMsg("Failed to launch System Events for user %s\n", userName);
#endif
            err = noErr;
            goto cleanupSystemEvents;
        }
    }
    sleep(2);

    for (i=0; i<NUMBRANDS; i++) {
#if TESTING
        showDebugMsg("Deleting any login items containing %s for user %s\n", appName[i], userName);
#endif
#if USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS
        if (isHighSierraOrLater) {
            sprintf(cmd, "su -l \"%s\" -c 'osascript -e \"tell application \\\"System Events\\\" to delete login item \\\"%s\\\"\"'", userName, appName[i]);
        } else
#endif
        {
            sprintf(cmd, "sudo -u \"%s\" osascript -e 'tell application \"System Events\" to delete login item \"%s\"'", userName, appName[i]);
        }
        err = callPosixSpawn(cmd);
#if TESTING
        if (err) {
            showDebugMsg("Command: %s\n", cmd);
            showDebugMsg("Delete login item containing %s returned error %d\n", appName[i], err);
        }
#endif
    }

cleanupSystemEvents:
    // Clean up in case this was our last user
    fprintf(stdout, "Telling System Events to quit (at end of DeleteLoginItemOSAScript)\n");
    fflush(stdout);
    systemEventsPID = FindProcessPID(systemEventsAppName, 0);
    err2 = noErr;
    if (systemEventsPID != 0) {
        err2 = kill(systemEventsPID, SIGKILL);
    }
    if (err2 != noErr) {
#if TESTING
        showDebugMsg("kill(systemEventsPID, SIGKILL) returned error %d \n", (int) err2);
#endif
    }
    // Wait for the process to be gone
    for (i=0; i<50; ++i) {      // 5 seconds max delay
        SleepSeconds(0.1);      // 1/10 second
        systemEventsPID = FindProcessPID(systemEventsAppName, 0);
        if (systemEventsPID == 0) break;
    }
    if (i >= 50) {
#if TESTING
        showDebugMsg("Failed to make System Events quit\n");
#endif
    }

    sleep(4);
}


// As of OS 10.13 High Sierra, telling System Events to modify Login Items for
// users who are not currently logged in no longer works, even when System Events
// is running as that user.
// So we create a LaunchAgent for that user. The next time that user logs in, the
// LaunchAgent will make the desired changes to that user's Login Items, launch
// BOINC Manager if appropriate, and delete itself.
//
// While we could just use a LaunchAgent to launch BOINC Manager on every login
// instead of using it to create a Login Item, I prefer Login Items because:
//  * they are more readily visible to a less technically aware user through
//    System Preferences, and
//  * they are more easily added or removed through System Preferences, and
//  * continuing to use them is consistent with older versions of BOINC Manager.
//
Boolean DeleteLoginItemLaunchAgent(long brandID, passwd *pw)
{
    struct stat             sbuf;
    char                    path[MAXPATHLEN];
    char                    s[2*MAXPATHLEN];
    int                     i;
    OSErr                   err;

    FixLaunchServicesDataBase(pw->pw_uid, "edu.berkeley.boinc.finish-install");

    snprintf(s, sizeof(s), "mkdir -p \"/Users/%s/Library/Application Support/BOINC/\"", pw->pw_name);
    err = callPosixSpawn(s);
     if (err) {
        printf("Command %s returned error %d\n", s, err);
        fflush(stdout);
    }
    snprintf(s, sizeof(s), "/Users/%s/Library/Application Support/BOINC/", pw->pw_name);
    chmod(s, 0771);
    chown(s, pw->pw_uid, pw->pw_gid);

    for (i=0; i< NUMBRANDS; i++) {
        // If we previously ran the installer for any brand but did not log in to
        // this user, remove the user's unused BOINC_Manager_Finish_Install file.
        snprintf(s, sizeof(s), "rm -fR \"/Users/%s/Library/Application Support/BOINC/%s_Finish_Install.app\"", pw->pw_name, brandName[i]);
        err = callPosixSpawn(s);
        if (err) {
            printf("Command %s returned error %d\n", s, err);
            fflush(stdout);
        }

        // If we previously ran the installer for any brand but did not log in to
        // this user, remove the user's unused BOINC_Manager_Finish_Uninstall file.
        snprintf(s, sizeof(s), "rm -fR \"/Users/%s/Library/Application Support/BOINC/%s_Finish_Uninstall.app\"", pw->pw_name, brandName[i]);
        err = callPosixSpawn(s);
        if (err) {
            printf("Command %s returned error %d\n", s, err);
            fflush(stdout);
        }
    }


    getPathToThisApp(path, sizeof(path));
    snprintf(s, sizeof(s), "cp -fR \"%s/Contents/Resources/%s_Finish_Uninstall.app\" \"/Users/%s/Library/Application Support/BOINC/\"",
            path, brandName[brandID], pw->pw_name);
    err = callPosixSpawn(s);
     if (err) {
        printf("Command %s returned error %d\n", s, err);
        fflush(stdout);
    }

    snprintf(s, sizeof(s), "chown -fR %s \"/Users/%s/Library/Application Support/BOINC/%s_Finish_Uninstall.app\"",
                pw->pw_name, pw->pw_name, brandName[brandID]);
    err = callPosixSpawn(s);
    if (err) {
        printf("Command %s returned error %d\n", s, err);
        fflush(stdout);
    }

    // Register this copy of BOINCFinish_Install.app. See comments on FixLaunchServicesDataBase.
    sprintf(s, "sudo -u \"%s\" /System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Versions/Current/Support/lsregister \"/Users/%s/Library/Application Support/BOINC/%s_Finish_Uninstall.app\"", pw->pw_name, pw->pw_name, brandName[brandID]);
    err = callPosixSpawn(s);
    if (err) {
        printf("*** user %s: lsregister call returned error %d for %s_Finish_Uninstall.app\n", pw->pw_name, err, brandName[brandID]);
        fflush(stdout);
    }

    // Create a LaunchAgent to finish uninstall for the specified user, replacing any LaunchAgent
    // created previously (such as by Uninstaller or by installing a differently branded BOINC.)

    // Create LaunchAgents directory for this user if it does not yet exist
    snprintf(s, sizeof(s), "/Users/%s/Library/LaunchAgents", pw->pw_name);
    if (stat(s, &sbuf) != 0) {
        mkdir(s, 0755);
        chown(s, pw->pw_uid, pw->pw_gid);
    }

    snprintf(s, sizeof(s), "/Users/%s/Library/LaunchAgents/edu.berkeley.boinc.plist", pw->pw_name);
    FILE* f = fopen(s, "w");
    if (!f) return false;
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
    fprintf(f, "<plist version=\"1.0\">\n");
    fprintf(f, "<dict>\n");
    fprintf(f, "\t<key>Label</key>\n");
    fprintf(f, "\t<string>edu.berkeley.fix_login_items</string>\n");
    fprintf(f, "\t<key>ProgramArguments</key>\n");
    fprintf(f, "\t<array>\n");
    fprintf(f, "\t\t<string>/Users/%s/Library/Application Support/BOINC/%s_Finish_Uninstall.app/Contents/MacOS/%s_Finish_Uninstall</string>\n", pw->pw_name, brandName[brandID], brandName[brandID]);
    // If this user was previously authorized to run the Manager, there
    // may still be a Login Item for this user, and the Login Item may
    // launch the Manager before the LaunchAgent deletes the Login Item.
    // To guard against this, we have the LaunchAgent kill the Manager
    // (for this user only) if it is running.
    //
    // Actually, the uninstaller should have deleted the Manager before
    // that could happen, so this step is probably unnecessary.
    //
    fprintf(f, "\t\t<string>-d</string>\n");
    fprintf(f, "\t\t<string>%d</string>\n", (int)brandID);
    fprintf(f, "\t</array>\n");
    if (compareOSVersionTo(13, 0) >= 0) {
        fprintf(f, "\t<key>AssociatedBundleIdentifiers</key>\n");
        fprintf(f, "\t<string>edu.berkeley.boinc.finish-install</string>\n");
    }
    fprintf(f, "\t<key>RunAtLoad</key>\n");
    fprintf(f, "\t<true/>\n");
    fprintf(f, "</dict>\n");
    fprintf(f, "</plist>\n");
    fclose(f);

    chmod(s, 0644);
    chown(s, pw->pw_uid, pw->pw_gid);

    if (IsUserLoggedIn(pw->pw_name)) {
        sprintf(s, "su -l \"%s\" -c 'launchctl unload /Users/%s/Library/LaunchAgents/edu.berkeley.boinc.plist'", pw->pw_name, pw->pw_name);
        callPosixSpawn(s);
        sprintf(s, "su -l \"%s\" -c 'launchctl load /Users/%s/Library/LaunchAgents/edu.berkeley.boinc.plist'", pw->pw_name, pw->pw_name);
        callPosixSpawn(s);
    }

    return true;
}

// Some older versions of BOINC installed a Screensaver LaunchAgent for each user.
// Even though we no longer do this, delete it if it exists, for backward compatibility
void DeleteScreenSaverLaunchAgent(passwd *pw) {
    char                    cmd[MAXPATHLEN];

    sprintf(cmd, "/Users/%s/Library/LaunchAgents/edu.berkeley.boinc-sshelper.plist", pw->pw_name);
    if (boinc_file_exists(cmd)) {
        sprintf(cmd, "su -l \"%s\" -c 'launchctl unload /Users/%s/Library/LaunchAgents/edu.berkeley.boinc-sshelper.plist'", pw->pw_name, pw->pw_name);
        callPosixSpawn(cmd);

        snprintf(cmd, sizeof(cmd),
            "/Users/%s/Library/LaunchAgents/edu.berkeley.boinc-sshelper.plist",
            pw->pw_name);
        boinc_delete_file(cmd);
    }
}


// Delete references to old copies of BOINC_Finish_Install, whose
// presence, for reasons I don't understand, causes its signing entity
// to be shown instead of its application name in the Login Items System
// Settings under MacOS 13 Ventura.
// NOTE: The new copy of BOINC_Finish_Install must then be registered.
//
// Each user has their own copy of the Launch Services database, so this
// must be done for each user.
//
static void FixLaunchServicesDataBase(uid_t userID, char *theBundleID) {
    uid_t saved_uid;
    char foundPath[MAXPATHLEN];
    char cmd[MAXPATHLEN+250];
    long i, n;
    CFArrayRef appRefs = NULL;
    OSStatus err;

    if (compareOSVersionTo(10, 8) < 0) {
        return;  // Notifications before OS 10.8 just bounce our Dock icon
    }

    saved_uid = geteuid();
    CFStringRef bundleID = CFStringCreateWithCString(NULL, theBundleID, kCFStringEncodingUTF8);
    if (LSCopyApplicationURLsForBundleIdentifier) { // Weak linked; not available before OS 10.10
        seteuid(userID);    // Temporarily set effective uid to this user
        appRefs = LSCopyApplicationURLsForBundleIdentifier(bundleID, NULL);
        seteuid(saved_uid);     // Set effective uid back to privileged user
        if (appRefs == NULL) {
            printf("Call to LSCopyApplicationURLsForBundleIdentifier(%d, %s) returned NULL\n",
                userID, theBundleID);
            fflush(stdout);
            return;
        }
        n = CFArrayGetCount(appRefs);   // Returns all results at once, in database order
        printf("LSCopyApplicationURLsForBundleIdentifier(%d, %s) returned %ld results\n",
            userID, theBundleID, n);
        fflush(stdout);
    } else {
        n = 500;    // Prevent infinite loop
    }

    for (i=0; i<n; ++i) {     // Prevent infinite loop
        if (appRefs) {
            CFURLRef appURL = (CFURLRef)CFArrayGetValueAtIndex(appRefs, i);
            foundPath[0] = '\0';
            if (appURL) {
                CFRetain(appURL);
                CFStringRef CFPath = CFURLCopyFileSystemPath(appURL, kCFURLPOSIXPathStyle);
                CFStringGetCString(CFPath, foundPath, sizeof(foundPath), kCFStringEncodingUTF8);
                if (CFPath) CFRelease(CFPath);
                CFRelease(appURL);
                appURL = NULL;
            }
        } else {
            seteuid(userID);    // Temporarily set effective uid to this user
            // GetPathToAppFromID() returns only first result from database
            err = GetPathToAppFromID('BNC!', bundleID,  foundPath, sizeof(foundPath));
            seteuid(saved_uid);     // Set effective uid back to privileged user
            if (err) {
                printf("Call %ld to GetPathToAppFromID returned error %d\n", i, err);
                fflush(stdout);
                break;
            }
        }

        printf("Unregistering %3ld: %s\n", i, foundPath);
        fflush(stdout);
        // Remove this entry from the Launch Services database
        sprintf(cmd, "sudo -u #%d /System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Versions/Current/Support/lsregister -u \"%s\"", userID, foundPath);
        err = callPosixSpawn(cmd);
        if (err) {
            printf("*** lsregister -u call returned error %d for %s\n", err, foundPath);
            fflush(stdout);
        }
    }

    CFRelease(bundleID);
}


long GetBrandID(char *path)
{
    long iBrandId;

    iBrandId = 0;   // Default value

    FILE *f = fopen(path, "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandId);
        fclose(f);
    }
    if ((iBrandId < 0) || (iBrandId > (NUMBRANDS-1))) {
        iBrandId = 0;
    }
    return iBrandId;
}


static Boolean IsUserLoggedIn(const char *userName){
    char s[1024];

    sprintf(s, "w -h \"%s\"", userName);
    FILE *f = popen(s, "r");
    if (f) {
        if (PersistentFGets(s, sizeof(s), f) != NULL) {
            pclose (f);
            printf("User %s is currently logged in\n", userName);
            return true; // this user is logged in (perhaps via fast user switching)
        }
        pclose (f);
    }
    return false;
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


OSErr GetCurrentScreenSaverSelection(passwd *pw, char *moduleName, size_t maxLen) {
    char                buf[1024];
    FILE                *f;
    char                *p, *q;
    int                 i;

    *moduleName = '\0';
    sprintf(buf, "su -l \"%s\" -c 'defaults -currentHost read com.apple.screensaver moduleDict'", pw->pw_name);
    f = popen(buf, "r");
    if (f == NULL) {
#if TESTING
        showDebugMsg("Could not get current screensaver selection for user %s\n", pw->pw_name);
#endif
        return fnfErr;
    }

    while (PersistentFGets(buf, sizeof(buf), f))
    {
        p = strstr(buf, "moduleName = ");
        if (p) {
            p += 13;    // Point past "moduleName = "
            q = moduleName;
            for (i=0; i<maxLen-1; ++i) {
                if (*p == '"') {
                    ++p;
                    continue;
                }
                if (*p == ';') break;
                *q++ = *p++;
            }
            *q = '\0';
            pclose(f);
            return 0;
        }
    }

    pclose(f);
    return fnfErr;
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
    f = fopen("/tmp/UninstallBOINC/BOINC_preferred_languages", "w");

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
    f = fopen("/tmp/UninstallBOINC/BOINC_preferred_languages", "r");
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


static void showDebugMsg(const char *format, ...) {
    va_list                 args;
    char                    s[1024];

    va_start(args, format);
    vsprintf(s, format, args);
    va_end(args);

#if DEBUG_PRINT
    print_to_log_file(s);
#else
    ShowMessage(false, false, false, s);
#endif
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
    int argc __attribute__((unused)) = 0;
    char *p;
    pid_t thePid = 0;
    int result = 0;
    int status = 0;
    extern char **environ;

    // Show  command in stderr_file to asociate error messages with their context
    fprintf(stderr, "\ncmd: %s\n", cmdline);

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


#if VERBOSE_TEST || TESTING
static void strip_cr(char *buf)
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
static void print_to_log_file(const char *format, ...) {
#if VERBOSE_TEST || TESTING
    FILE *f;
    va_list args;
    char buf[256];
    time_t t;
    sprintf(buf, "/Users/Shared/test_log_uninstallBOINC.txt");
    f = fopen(buf, "a");
    if (!f) return;

    // File will be owned by root, so make it world readable & writable
    chmod(buf, 0666);

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

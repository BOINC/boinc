// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

/*  AddRemoveUser.cpp */

#define VERBOSE 0
#define VERBOSE_SPAWN 0  /* for debugging callPosixSpawn */

#define USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS false

#include <Carbon/Carbon.h>

#include <sys/types.h>	// getpwname, getpwuid, getuid
#include <sys/time.h>
#include <pwd.h>	// getpwname, getpwuid, getuid
#include <grp.h>        // getgrnam
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>   // for chmod
#include "mac_util.h"
#include "filesys.h"
#include "util.h"
#include "mac_branding.h"

void printUsage(long brandID);
Boolean SetLoginItemOSAScript(long brandID, Boolean deleteLogInItem, char *userName);
Boolean SetLoginItemLaunchAgent(long brandID, Boolean deleteLogInItem, passwd *pw);
Boolean SetLoginItemLaunchAgentShellScript(long brandID, Boolean deleteLogInItem, passwd *pw);
Boolean SetLoginItemLaunchAgentFinishInstallApp(long brandID, Boolean deleteLogInItem, passwd *pw);
OSErr GetCurrentScreenSaverSelection(passwd *pw, char *moduleName, size_t maxLen);
OSErr SetScreenSaverSelection(passwd *pw, char *moduleName, char *modulePath, int type);
pid_t FindProcessPID(char* name, pid_t thePID);
#if USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS
static Boolean IsUserLoggedIn(const char *userName);
#endif
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
static void SleepSeconds(double seconds);
long GetBrandID(void);
static int parse_posix_spawn_command_line(char* p, char** argv);
int callPosixSpawn(const char *cmd);


int main(int argc, char *argv[])
{
    long                brandID = 0;
    Boolean             AddUsers = false;
    Boolean             SetSavers = false;
    Boolean             isBMGroupMember, isBPGroupMember;
    Boolean             saverIsSet = false;
    passwd              *pw;
    uid_t               saved_uid;
    group               grpBOINC_master, *grpBOINC_masterPtr;
    group               grpBOINC_project, *grpBOINC_projectPtr;
    char                bmBuf[32768];
    char                bpBuf[32768];
    char                loginName[256];
    short               index, i;
    FILE                *f;
    int                 flag;
    char                *p;
    char                s[1024], buf[1024];
    OSStatus            err;

    brandID = GetBrandID();

#ifndef _DEBUG
    if (getuid() != 0) {
        printf("This program must be run as root\n");
        printUsage(brandID);
        return 0;
    }
#endif
    saved_uid = geteuid();

    if (argc < 3) {
        printUsage(brandID);
        return 0;
    }

    if (strcmp(argv[1], "-a") == 0) {
        AddUsers = true;
    } else if (strcmp(argv[1], "-s") == 0) {
        AddUsers = true;
        SetSavers = true;
    } else if (strcmp(argv[1], "-r") != 0) {
        printUsage(brandID);
        return 0;
    }

    printf("\n");

    if (!check_branding_arrays(s, sizeof(s))) {
        printf("Branding array has too few entries: %s\n", s);
        return -1;
    }

    loginName[0] = '\0';
    strncpy(loginName, getenv("USER"), sizeof(loginName)-1);

    err = getgrnam_r("boinc_master", &grpBOINC_master, bmBuf, sizeof(bmBuf), &grpBOINC_masterPtr);
    if (err) {          // Should never happen unless buffer too small
        puts("getgrnam(\"boinc_master\") failed\n");
        return -1;
    }

    err = getgrnam_r("boinc_project", &grpBOINC_project, bpBuf, sizeof(bpBuf), &grpBOINC_projectPtr);
    if (err) {          // Should never happen unless buffer too small
        puts("getgrnam(\"boinc_project\") failed\n");
        return -1;
    }

    for (index=2; index<argc; index++) {
        // getpwnam works with either the full / login name (pw->pw_gecos)
        // or the short / Posix name (pw->pw_name)
        pw = getpwnam(argv[index]);
        if ((pw == NULL) || (pw->pw_uid < 501)) {
            printf("User %s not found.\n\n", argv[index]);
            continue;
        }

        flag = 0;
        sprintf(s, "dscl . -read \"/Users/%s\" NFSHomeDirectory", pw->pw_name);
        f = popen(s, "r");
        if (!f) {
            flag = 1;
            } else {
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
            sprintf(s, "dscl . -read \"/Users/%s\" UserShell", pw->pw_name);
            f = popen(s, "r");
            if (!f) {
                flag |= 2;
            } else {
                while (PersistentFGets(buf, sizeof(buf), f)) {
                    p = strrchr(buf, ' ');
                    if (p) {
                        if (strstr(p, "/usr/bin/false") != NULL) {
                            flag |= 2;
                            break;
                        }
                    }
                }
                pclose(f);
            }
        }

        if (flag == 3) { // if (Home Directory == "/var/empty") && (UserShell == "/usr/bin/false")
            printf("%s is not a valid user name.\n\n", argv[index]);
            continue;
        }

        printf("%s user %s (/Users/%s)\n", AddUsers? "Adding" : "Removing", pw->pw_gecos, pw->pw_name);

        isBMGroupMember = false;
        i = 0;
        while ((p = grpBOINC_master.gr_mem[i]) != NULL) {  // Step through all users in group boinc_master
            if (strcmp(p, pw->pw_name) == 0) {      // Only the short / Posix names are in the list
                // User is a member of group boinc_master
                isBMGroupMember = true;
                break;
            }
            ++i;
        }

        isBPGroupMember = false;
        i = 0;
        while ((p = grpBOINC_project.gr_mem[i]) != NULL) {  // Step through all users in group boinc_project
            if (strcmp(p, pw->pw_name) == 0) {      // Only the short / Posix names are in the list
                // User is a member of group boinc_master
                isBPGroupMember = true;
                break;
            }
            ++i;
        }

        if ((!isBMGroupMember) && AddUsers) {
            sprintf(s, "dscl . -merge /groups/boinc_master GroupMembership %s", pw->pw_name);
            callPosixSpawn(s);
        }

        if ((!isBPGroupMember) && AddUsers) {
            sprintf(s, "dscl . -merge /groups/boinc_project GroupMembership %s", pw->pw_name);
            callPosixSpawn(s);
        }

        if (isBMGroupMember && (!AddUsers)) {
            sprintf(s, "dscl . -delete /Groups/boinc_master GroupMembership %s", pw->pw_name);
            callPosixSpawn(s);
        }

        if (isBPGroupMember && (!AddUsers)) {
            sprintf(s, "dscl . -delete /Groups/boinc_project GroupMembership %s", pw->pw_name);
            callPosixSpawn(s);
        }

        // Set or remove login item for this user
        bool useOSASript = false;

        if ((compareOSVersionTo(10, 13) < 0)
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
            if (!AddUsers) {
                // Delete per-user BOINC Manager and screensaver files
                sprintf(s, "rm -fR \"/Users/%s/Library/Application Support/BOINC\"", pw->pw_name);
                callPosixSpawn (s);
            }

            snprintf(s, sizeof(s), "/Users/%s/Library/LaunchAgents/edu.berkeley.boinc.plist", pw->pw_name);
            boinc_delete_file(s);
            SetLoginItemOSAScript(brandID, !AddUsers, pw->pw_name);
        } else {
            SetLoginItemLaunchAgent(brandID, !AddUsers, pw);
        }

        saverIsSet = false;
        err = GetCurrentScreenSaverSelection(pw, s, sizeof(s) -1);
#if VERBOSE
        fprintf(stderr, "Current Screensaver Selection for user %s is: \"%s\"\n", pw->pw_name, s);
#endif
        if (err == noErr) {
            if (!strcmp(s, saverName[brandID])) {
                saverIsSet = true;
            }
        }

        if (SetSavers) {
            if (saverIsSet) {
                printf("Screensaver already set to %s for user %s (/Users/%s)\n", saverName[brandID], pw->pw_gecos, pw->pw_name);
            } else {
                printf("Setting screensaver to %s for user %s (/Users/%s)\n", saverName[brandID], pw->pw_gecos, pw->pw_name);
            }
        }

        if ((!saverIsSet) && SetSavers) {
            seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
            sprintf(s, "/Library/Screen Savers/%s.saver", saverName[brandID]);
            err = SetScreenSaverSelection(pw, saverName[brandID], s, 0);
#if VERBOSE
            fprintf(stderr, "SetScreenSaverSelection for user %s (uid %d) to \"%s\" returned error %d\n", pw->pw_name, geteuid(), saverName[brandID], err);
#endif
            seteuid(saved_uid);     // Set effective uid back to privileged user
            // This seems to work also:
            // sprintf(buf, "su -l \"%s\" -c 'defaults -currentHost write com.apple.screensaver moduleDict -dict moduleName \"%s\" path \"%s\ type 0'", pw->pw_name, saverName[brandID], s);
            // callPosixSpawn(s);
        }

        if (saverIsSet && (!AddUsers)) {
            printf("Setting screensaver to Flurry for user %s (/Users/%s)\n", pw->pw_gecos, pw->pw_name);
            seteuid(pw->pw_uid);    // Temporarily set effective uid to this user
            err = SetScreenSaverSelection(pw, "Flurry", "/System/Library/Screen Savers/Flurry.saver", 0);
#if VERBOSE
            fprintf(stderr, "SetScreenSaverSelection for user %s (%d) to Flurry returned error %d\n", pw->pw_name, geteuid(), err);
#endif
            seteuid(saved_uid);     // Set effective uid back to privileged user
        }

        seteuid(saved_uid);                         // Set effective uid back to privileged user

        printf("\n");
    }

    printf("WARNING: Changes may require a system restart to take effect.\n");

    return 0;
}

void printUsage(long brandID) {
    printf("Usage: sudo AddRemoveUser [-a | -s | -r] [user1 [user2 [user3...]]]\n");
    printf("   -a: add users to those with permission to run %s.\n", appName[brandID]);
    printf("   -s: same as -a plus set users' screensaver to %s.\n", brandName[brandID]);
    printf("   -r: remove users' permission to run %s, and \n", appName[brandID]);
    printf("      if their screensaver was set to %s change it to Flurry.\n", brandName[brandID]);
    printf("\n");
}

enum {
	kSystemEventsCreator = 'sevs'
};
CFStringRef kSystemEventsBundleID = CFSTR("com.apple.systemevents");
char *systemEventsAppName = "System Events";

Boolean SetLoginItemOSAScript(long brandID, Boolean deleteLogInItem, char *userName)
{
    int                     i, j;
    char                    cmd[2048];
    char                    systemEventsPath[1024];
    pid_t                   systemEventsPID;
    OSErr                   err = 0, err2 __attribute__((unused)) = 0;
#if USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS
    // NOTE: It may not be necessary to kill and relaunch the
    // System Events application for each logged in user under High Sierra
    Boolean                 isHighSierraOrLater = (compareOSVersionTo(10, 13) >= 0);
#endif

#if VERBOSE
    fprintf(stderr, "Adjusting login items for user %s\n", userName);
    fflush(stderr);
#endif

    // We must launch the System Events application for the target user
    err = noErr;
    systemEventsPath[0] = '\0';

    err = GetPathToAppFromID(kSystemEventsCreator, kSystemEventsBundleID, systemEventsPath, sizeof(systemEventsPath));
#if VERBOSE
    if (err == noErr) {
        fprintf(stderr, "SystemEvents is at %s\n", systemEventsPath);
    } else {
        fprintf(stderr, "GetPathToAppFromID(kSystemEventsCreator, kSystemEventsBundleID) returned error %d ", (int) err);
    }
#endif

    if (err == noErr) {
        // Find SystemEvents process.  If found, quit it in case
        // it is running under a different user.
#if VERBOSE
        fprintf(stderr, "Telling System Events to quit (at start of SetLoginItemOSAScript)\n");
        fflush(stderr);
#endif
        systemEventsPID = FindProcessPID(systemEventsAppName, 0);
        if (systemEventsPID != 0) {
            err = kill(systemEventsPID, SIGKILL);
        }
#if VERBOSE
        if (err != noErr) {
            fprintf(stderr, "(systemEventsPID, SIGKILL) returned error %d \n", (int) err);
            fflush(stderr);
        }
#endif
        // Wait for the process to be gone
        for (i=0; i<50; ++i) {      // 5 seconds max delay
            SleepSeconds(0.1);      // 1/10 second
            systemEventsPID = FindProcessPID(systemEventsAppName, 0);
            if (systemEventsPID == 0) break;
        }
        if (i >= 50) {
#if VERBOSE
            fprintf(stderr, "Failed to make System Events quit\n");
            fflush(stderr);
#endif
            err = noErr;
            goto cleanupSystemEvents;
        }
        sleep(4);
    }

    if (systemEventsPath[0] != '\0') {
#if VERBOSE
        fprintf(stderr, "Launching SystemEvents for user %s\n", userName);
        fflush(stderr);
#endif

        for (j=0; j<5; ++j) {
            sprintf(cmd, "sudo -u \"%s\" -b \"%s/Contents/MacOS/System Events\" &", userName, systemEventsPath);
            err = callPosixSpawn(cmd);
#if VERBOSE
            if (err) {
                fprintf(stderr, "Command: %s returned error %d (try %d of 5)\n", cmd, (int) err, j);
                fflush(stderr);
            }
#endif
            // Wait for the process to start
            for (i=0; i<50; ++i) {      // 5 seconds max delay
                SleepSeconds(0.1);      // 1/10 second
                systemEventsPID = FindProcessPID(systemEventsAppName, 0);
                if (systemEventsPID != 0) break;
            }
            if (i < 50) break;  // Exit j loop on success
        }
        if (j >= 5) {
#if VERBOSE
            fprintf(stderr, "Failed to launch System Events for user %s\n", userName);
            fflush(stderr);
#endif
            err = noErr;
            goto cleanupSystemEvents;
        }
    }
    sleep(2);

#if VERBOSE
    fprintf(stderr, "Deleting any login items containing %s for user %s\n", appName[brandID], userName);
    fflush(stderr);
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
#if VERBOSE
    if (err) {
        fprintf(stderr, "Command: %s\n", cmd);
        fprintf(stderr, "Delete login item containing %s returned error %d\n", appName[brandID], err);
        fflush(stderr);
    }
#endif

    if (deleteLogInItem) {
        err = noErr;
        goto cleanupSystemEvents;
    }

#if VERBOSE
    fprintf(stderr, "Making new login item %s for user %s\n", appName[brandID], userName);
    fflush(stderr);
#endif
#if USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS
    if (isHighSierraOrLater) {
        sprintf(cmd, "su -l \"%s\" -c 'osascript -e \"tell application \\\"System Events\\\" to make new login item at end with properties {path:\\\"%s\\\", hidden:true, name:\\\"%s\\\"}\"'", userName, appPath[brandID], appName[brandID]);
    } else
#endif
    {
        sprintf(cmd, "sudo -u \"%s\" osascript -e 'tell application \"System Events\" to make new login item at end with properties {path:\"%s\", hidden:true, name:\"%s\"}'", userName, appPath[brandID], appName[brandID]);
    }
    err = callPosixSpawn(cmd);
#if VERBOSE
    if (err) {
        fprintf(stderr, "Command: %s\n", cmd);
        printf("[Make login item for %s returned error %d\n", appPath[brandID], err);
    }
#endif
    fflush(stderr);

cleanupSystemEvents:
    // Clean up in case this was our last user
#if VERBOSE
    fprintf(stderr, "Telling System Events to quit (at end of SetLoginItemOSAScript)\n");
    fflush(stderr);
#endif
    systemEventsPID = FindProcessPID(systemEventsAppName, 0);
    err2 = noErr;
    if (systemEventsPID != 0) {
        err2 = kill(systemEventsPID, SIGKILL);
    }
#if VERBOSE
    if (err2 != noErr) {
        fprintf(stderr, "kill(systemEventsPID, SIGKILL) returned error %d \n", (int) err2);
        fflush(stderr);
    }
#endif
    // Wait for the process to be gone
    for (i=0; i<50; ++i) {      // 5 seconds max delay
        SleepSeconds(0.1);      // 1/10 second
        systemEventsPID = FindProcessPID(systemEventsAppName, 0);
        if (systemEventsPID == 0) break;
    }
#if VERBOSE
    if (i >= 50) {
        fprintf(stderr, "Failed to make System Events quit\n");
        fflush(stderr);
    }
#endif

    sleep(4);

    return (err == noErr);
}


// Under OS 10.13 High Sierra, telling System Events to modify Login Items for
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
Boolean SetLoginItemLaunchAgent(long brandID, Boolean deleteLogInItem, passwd *pw)
{
    struct stat             sbuf;
    char                    s[2048];
    int                     err;

    // Create a LaunchAgent for the specified user, replacing any LaunchAgent created
    // previously (such as by Ininstaller or by installing a differently branded BOINC.)

    // Register BOINCFinish_Install.app, which may have been unregistered by our installer.
    sprintf(s, "sudo -u #%d /System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Versions/Current/Support/lsregister \"//Library/Application Support/BOINC Data/%s_Finish_Install.app\"", pw->pw_uid, brandName[brandID]);
    err = callPosixSpawn(s);
    if (err) {
        printf("*** user %s: lsregister call returned error %d for %s_Finish_Install.app\n", pw->pw_name, err, brandName[brandID]);
        fflush(stdout);
    }

    // Create LaunchAgents directory for this user if it does not yet exist
    snprintf(s, sizeof(s), "/Users/%s/Library/LaunchAgents", pw->pw_name);
    if (stat(s, &sbuf) != 0) {
        mkdir(s, 0755);
        chown(s, pw->pw_uid, pw->pw_gid);
    }

    snprintf(s, sizeof(s), "/Library/Application Support/BOINC Data/%s_Finish_Install.app", brandName[brandID]);
    if (stat(s, &sbuf) != 0) {
        return SetLoginItemLaunchAgentShellScript(brandID, deleteLogInItem, pw);
    } else {
         snprintf(s, sizeof(s), "/Users/%s/Library/Application Support/BOINC", pw->pw_name);
        if (stat(s, &sbuf) != 0) {
            snprintf(s, sizeof(s), "sudo -u \"%s\" mkdir -p -m 0775 \"/Users/%s/Library/Application Support/BOINC\"",
                    pw->pw_name, pw->pw_name);
            err = callPosixSpawn(s);
            if (err) {
                fprintf(stderr, "Command: %s returned error %d\n", s, (int) err);
                fflush(stderr);
            }
        }

        snprintf(s, sizeof(s), "cp -fR \"/Library/Application Support/BOINC Data/%s_Finish_Install.app\" \"/Users/%s/Library/Application Support/BOINC/%s_Finish_Install.app\"", brandName[brandID], pw->pw_name, brandName[brandID]);
        err = callPosixSpawn(s);
        if (err) {
            fprintf(stderr, "Command: %s returned error %d\n", s, (int) err);
            fflush(stderr);
        }
        snprintf(s, sizeof(s), "chown -R %d:%d \"/Users/%s/Library/Application Support/BOINC/%s_Finish_Install.app\"", pw->pw_uid, pw->pw_gid, pw->pw_name, brandName[brandID]);
        err = callPosixSpawn(s);
        if (err) {
            fprintf(stderr, "Command: %s returned error %d\n", s, (int) err);
            fflush(stderr);
        }
        snprintf(s, sizeof(s), "chmod -R a+rw \"/Users/%s/Library/Application Support/BOINC/%s_Finish_Install.app\"", pw->pw_name, brandName[brandID]);
        if (err) {
            fprintf(stderr, "Command: %s returned error %d\n", s, (int) err);
            fflush(stderr);
        }

        return SetLoginItemLaunchAgentFinishInstallApp(brandID, deleteLogInItem, pw);
    }
}


Boolean SetLoginItemLaunchAgentShellScript(long brandID, Boolean deleteLogInItem, passwd *pw) {
    char                    s[2048];

    snprintf(s, sizeof(s), "/Users/%s/Library/LaunchAgents/edu.berkeley.boinc.plist", pw->pw_name);
    FILE* f = fopen(s, "w");
    if (!f) return false;
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
    fprintf(f, "<plist version=\"1.0\">\n");
    fprintf(f, "<dict>\n");
    fprintf(f, "\t<key>Label</key>\n");
    fprintf(f, "\t<string>edu.berkeley.test</string>\n");
    fprintf(f, "\t<key>ProgramArguments</key>\n");
    fprintf(f, "\t<array>\n");
    fprintf(f, "\t\t<string>sh</string>\n");
    fprintf(f, "\t\t<string>-c</string>\n");
    fprintf(f, "\t\t<string>");
    fprintf(f, "osascript -e 'tell application \"System Events\" to delete login item \"%s\"';", appName[brandID]);
    if (deleteLogInItem) {
        // If this user was previously authorized to run the Manager, there
        // may still be a Login Item for this user, and the Login Item may
        // launch the Manager before the LaunchAgent deletes the Login Item.
        // To guard against this, we have the LaunchAgent kill the Manager
        // (for this user only) if it is running.
        //
        fprintf(f, "killall -u %d -9 \"%s\";", pw->pw_uid, appName[brandID]);
    } else {
        fprintf(f, "osascript -e 'tell application \"System Events\" to make login item at end with properties {path:\"%s\", hidden:true, name:\"%s\"}';", appPath[brandID], appName[brandID]);
        fprintf(f, "open -jg \"%s\";", appPath[brandID]);
    }
    fprintf(f, "rm -f ~/Library/LaunchAgents/edu.berkeley.boinc.plist</string>\n");
    fprintf(f, "\t</array>\n");
    fprintf(f, "\t<key>RunAtLoad</key>\n");
    fprintf(f, "\t<true/>\n");
    fprintf(f, "</dict>\n");
    fprintf(f, "</plist>\n");
    fclose(f);

    chmod(s, 0644);
    chown(s, pw->pw_uid, pw->pw_gid);

    return true;
}


Boolean SetLoginItemLaunchAgentFinishInstallApp(long brandID, Boolean deleteLogInItem, passwd *pw){
    char                    s[2048];

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
    fprintf(f, "\t\t<string>/Users/%s/Library/Application Support/BOINC/%s_Finish_Install.app/Contents/MacOS/%s_Finish_Install</string>\n", pw->pw_name, brandName[brandID], brandName[brandID]);
     if (deleteLogInItem) {
        // If this user was previously authorized to run the Manager, there
        // may still be a Login Item for this user, and the Login Item may
        // launch the Manager before the LaunchAgent deletes the Login Item.
        // To guard against this, we have the LaunchAgent kill the Manager
        // (for this user only) if it is running.
        //
        fprintf(f, "\t\t<string>-d</string>\n");
        fprintf(f, "\t\t<string>%s</string>\n", appName[brandID]);
    } else  {
        fprintf(f, "\t\t<string>-a</string>\n");
        fprintf(f, "\t\t<string>%s</string>\n", appName[brandID]);
    }
    fprintf(f, "</string>\n");
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

    return true;


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
    if (f == NULL) {
        return 0;
    }

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


OSErr GetCurrentScreenSaverSelection(passwd *pw, char *moduleName, size_t maxLen) {
    char                buf[1024];
    FILE                *f;
    char                *p, *q;
    int                 i;

    *moduleName = '\0';
    sprintf(buf, "su -l \"%s\" -c 'defaults -currentHost read com.apple.screensaver moduleDict'", pw->pw_name);
    f = popen(buf, "r");
    if (f == NULL) {
        fprintf(stderr, "Could not get current screensaver selection for user %s\n", pw->pw_name);
        fflush(stderr);
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


OSErr SetScreenSaverSelection(passwd *pw, char *moduleName, char *modulePath, int type) {
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
        fprintf(stderr, "Could not set screensaver for user %s\n", pw->pw_name);
        fflush(stderr);
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
        fprintf(stderr, "Could not set screensaver for user %s\n", pw->pw_name);
        fflush(stderr);
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
        fprintf(stderr, "Could not set screensaver for user %s\n", pw->pw_name);
        fflush(stderr);
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


#if USE_OSASCRIPT_FOR_ALL_LOGGED_IN_USERS
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
#endif


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


long GetBrandID()
{
    long iBrandId;

    iBrandId = 0;   // Default value

    FILE *f = fopen("/Library/Application Support/BOINC Data/Branding", "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandId);
        fclose(f);
    }
    if ((iBrandId < 0) || (iBrandId > (NUMBRANDS-1))) {
        iBrandId = 0;
    }
    return iBrandId;
}


#define NOT_IN_TOKEN                0
#define IN_SINGLE_QUOTED_TOKEN      1
#define IN_DOUBLE_QUOTED_TOKEN      2
#define IN_UNQUOTED_TOKEN           3

static int parse_posix_spawn_command_line(char* p, char** argv) {
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

int callPosixSpawn(const char *cmdline) {
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

    // Make a copy of cmdline because parse_posix_spawn_command_line modifies it
    strlcpy(command, cmdline, sizeof(command));
    argc = parse_posix_spawn_command_line(const_cast<char*>(command), argv);
    strlcpy(progPath, argv[0], sizeof(progPath));
    strlcpy(progName, argv[0], sizeof(progName));
    p = strrchr(progName, '/');
    if (p) {
        argv[0] = p+1;
    } else {
        argv[0] = progName;
    }

#if VERBOSE_SPAWN
    fprintf(stderr, "***********");
    for (int i=0; i<argc; ++i) {
        fprintf(stderr, "argv[%d]=%s", i, argv[i]);
    }
    fprintf(stderr, "***********\n");
#endif

    errno = 0;

    result = posix_spawnp(&thePid, progPath, NULL, NULL, argv, environ);
#if VERBOSE_SPAWN
    fprintf(stderr, "callPosixSpawn command: %s", cmdline);
    fprintf(stderr, "callPosixSpawn: posix_spawnp returned %d: %s", result, strerror(result));
#endif
    if (result) {
        return result;
    }
// CAF    int val =
    waitpid(thePid, &status, WUNTRACED);
// CAF        if (val < 0) printf("first waitpid returned %d\n", val);
    if (status != 0) {
#if VERBOSE_SPAWN
        fprintf(stderr, "waitpid() returned status=%d", status);
#endif
        result = status;
    } else {
        if (WIFEXITED(status)) {
            result = WEXITSTATUS(status);
            if (result == 1) {
#if VERBOSE_SPAWN
                fprintf(stderr, "WEXITSTATUS(status) returned 1, errno=%d: %s", errno, strerror(errno));
#endif
                result = errno;
            }
#if VERBOSE_SPAWN
            else if (result) {
                fprintf(stderr, "WEXITSTATUS(status) returned %d", result);
            }
#endif
        }   // end if (WIFEXITED(status)) else
    }       // end if waitpid returned 0 sstaus else

    return result;
}

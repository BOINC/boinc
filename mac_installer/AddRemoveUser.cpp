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

/*  AddRemoveUser.cpp */

#define VERBOSE 0
#define VERBOSE_SPAWN 0  /* for debugging callPosixSpawn */

#include <Carbon/Carbon.h>

#include <unistd.h>	// getlogin
#include <sys/types.h>	// getpwname, getpwuid, getuid
#include <sys/time.h>
#include <pwd.h>	// getpwname, getpwuid, getuid
#include <grp.h>        // getgrnam
#include <sys/param.h>  // for MAXPATHLEN
#include "mac_util.h"

void printUsage(long brandID);
Boolean SetLoginItemOSAScript(long brandID, Boolean addLogInItem, char *userName);
pid_t FindProcessPID(char* name, pid_t thePID);
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
static double dtime(void);
static void SleepSeconds(double seconds);
long GetBrandID(void);
static int parse_posic_spawn_command_line(char* p, char** argv);
int callPosixSpawn(const char *cmd);

#define NUMBRANDS 4
static char *appName[NUMBRANDS];
static char *appPath[NUMBRANDS];
static char *brandName[NUMBRANDS];


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
    short               index, i;
    char                *p;
    char                s[256];
    FILE                *f;
    OSStatus            err;
    
    appName[0] = "BOINCManager";
    appPath[0] = "/Applications/BOINCManager.app";
    brandName[0] = "BOINC";
    appName[1] = "GridRepublic Desktop";
    appPath[1] = "/Applications/GridRepublic Desktop.app";
    brandName[1] = "GridRepublic";
    appName[2] = "Progress Thru Processors Desktop";
    appPath[2] = "/Applications/Progress Thru Processors Desktop.app";
    brandName[2] = "Progress Thru Processors";
    appName[3] = "Charity Engine Desktop";
    appPath[3] = "/Applications/Charity Engine Desktop.app";
    brandName[3] = "Charity Engine";

    brandID = GetBrandID();

#ifndef _DEBUG
    if (getuid() != 0) {
        printf("This program must be run as root\n");
        printUsage(brandID);
        return 0;
    }
#endif

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
        if (pw == NULL) {
            printf("User %s not found.\n", argv[index]);
            continue;
        }

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
        SetLoginItemOSAScript(brandID, AddUsers, pw->pw_name);

        saved_uid = geteuid();
        seteuid(pw->pw_uid);                        // Temporarily set effective uid to this user

        if (compareOSVersionTo(10, 6) < 0) {
            sprintf(s, "sudo -u %s defaults -currentHost read com.apple.screensaver moduleName", 
                    pw->pw_name); 
        } else {
            sprintf(s, "sudo -u %s defaults -currentHost read com.apple.screensaver moduleDict -dict", 
                    pw->pw_name); 
        }
        f = popen(s, "r");
        
        if (f) {
            saverIsSet = false;
            while (PersistentFGets(s, sizeof(s), f)) {
                if (strstr(s, "BOINCSaver")) {
                    saverIsSet = true;
                    break;
                }
            }
            pclose(f);
        }

        if ((!saverIsSet) && SetSavers) {
            if (compareOSVersionTo(10, 6) < 0) {
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver moduleName BOINCSaver",
                    pw->pw_name); 
                callPosixSpawn(s);
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver modulePath \"/Library/Screen Savers/BOINCSaver.saver\"", 
                    pw->pw_name); 
                callPosixSpawn(s);
            } else {
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver moduleDict -dict moduleName BOINCSaver path \"/Library/Screen Savers/BOINCSaver.saver\"", 
                        pw->pw_name);
                callPosixSpawn(s);
            }
        }
        
        if (saverIsSet && (!AddUsers)) {
            if (compareOSVersionTo(10, 6) < 0) {
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver moduleName Flurry",
                    pw->pw_name); 
                callPosixSpawn(s);
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver modulePath \"/System/Library/Screen Savers/Flurry.saver\"", 
                    pw->pw_name); 
                callPosixSpawn(s);
            } else {
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver moduleDict -dict moduleName Flurry path \"/callPosixSpawn/Library/Screen Savers/Flurry.saver\"",
                        pw->pw_name);
                callPosixSpawn(s);
            }
        }

        seteuid(saved_uid);                         // Set effective uid back to privileged user
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

Boolean SetLoginItemOSAScript(long brandID, Boolean addLogInItem, char *userName)
{
    int                     i, j;
    char                    cmd[2048];
    char                    systemEventsPath[1024];
    pid_t                   systemEventsPID;
    OSErr                   err = 0, err2 = 0;

#if VERBOSE
    fprintf(stderr, "Adjusting login items for user %s\n", userName);
    fflush(stderr);
#endif

    // We must launch the System Events application for the target user

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
    if (err != noErr) {
        fprintf(stderr, "(systemEventsPID, SIGKILL) returned error %d \n", (int) err);
        fflush(stderr);
    }
    // Wait for the process to be gone
    for (i=0; i<50; ++i) {      // 5 seconds max delay
        SleepSeconds(0.1);      // 1/10 second
        systemEventsPID = FindProcessPID(systemEventsAppName, 0);
        if (systemEventsPID == 0) break;
    }
    if (i >= 50) {
        fprintf(stderr, "Failed to make System Events quit\n");
        fflush(stderr);
        err = noErr;
        goto cleanupSystemEvents;
    }
        sleep(4);
    
    err = GetPathToAppFromID(kSystemEventsCreator, kSystemEventsBundleID,  systemEventsPath, sizeof(systemEventsPath));
    if (err != noErr) {
        fprintf(stderr, "GetPathToAppFromID(kSystemEventsCreator) returned error %d \n", (int) err);
        fflush(stderr);
        goto cleanupSystemEvents;
    }
#if VERBOSE
    fprintf(stderr, "SystemEvents is at %s\n", systemEventsPath);
    fprintf(stderr, "Launching SystemEvents for user %s\n", userName);
    fflush(stderr);
#endif

    for (j=0; j<5; ++j) {
        sprintf(cmd, "sudo -u \"%s\" -b \"%s/Contents/MacOS/System Events\" &", userName, systemEventsPath);
        err = callPosixSpawn(cmd);
        if (err) {
            fprintf(stderr, "Command: %s returned error %d (try %d of 5)\n", cmd, (int) err, j);
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
        fprintf(stderr, "Failed to launch System Events for user %s\n", userName);
        fflush(stderr);
        err = noErr;
        goto cleanupSystemEvents;
    }

    sleep(2);
    
#if VERBOSE
    fprintf(stderr, "Deleting any login items containing %s for user %s\n", appName[brandID], userName);
    fflush(stderr);
#endif
    sprintf(cmd, "sudo -u \"%s\" osascript -e 'tell application \"System Events\"' -e 'delete (every login item whose path contains \"%s\")' -e 'end tell'", userName, appName[brandID]);
    err = callPosixSpawn(cmd);
    if (err) {
        fprintf(stderr, "Command: %s\n", cmd);
        fprintf(stderr, "Delete login item containing %s returned error %d\n", appName[brandID], err);
        fflush(stderr);
    }
    
    if (addLogInItem == false) {
        err = noErr;
        goto cleanupSystemEvents;
    }
    
    fprintf(stderr, "Making new login item %s for user %s\n", appName[brandID], userName);
    fflush(stderr);
    sprintf(cmd, "sudo -u \"%s\" osascript -e 'tell application \"System Events\"' -e 'make new login item at end with properties {path:\"%s\", hidden:true, name:\"%s\"}' -e 'end tell'", userName, appPath[brandID], appName[brandID]);
    err = callPosixSpawn(cmd);
    if (err) {
        fprintf(stderr, "Command: %s\n", cmd);
        printf("[Make login item for %s returned error %d\n", appPath[brandID], err);
    }
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
    if (err2 != noErr) {
        fprintf(stderr, "kill(systemEventsPID, SIGKILL) returned error %d \n", (int) err2);
        fflush(stderr);
    }
    // Wait for the process to be gone
    for (i=0; i<50; ++i) {      // 5 seconds max delay
        SleepSeconds(0.1);      // 1/10 second
        systemEventsPID = FindProcessPID(systemEventsAppName, 0);
        if (systemEventsPID == 0) break;
    }
    if (i >= 50) {
        fprintf(stderr, "Failed to make System Events quit\n");
        fflush(stderr);
    }
    
    sleep(4);
        
    return (err == noErr);
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


#define NOT_IN_TOKEN                0
#define IN_SINGLE_QUOTED_TOKEN      1
#define IN_DOUBLE_QUOTED_TOKEN      2
#define IN_UNQUOTED_TOKEN           3

static int parse_posic_spawn_command_line(char* p, char** argv) {
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
    int argc = 0;
    char *p;
    pid_t thePid = 0;
    int result = 0;
    int status = 0;
    extern char **environ;
    
    // Make a copy of cmdline because parse_posic_spawn_command_line modifies it
    strlcpy(command, cmdline, sizeof(command));
    argc = parse_posic_spawn_command_line(const_cast<char*>(command), argv);
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

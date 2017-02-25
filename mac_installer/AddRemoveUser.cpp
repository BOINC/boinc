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

#include <Carbon/Carbon.h>

#include <unistd.h>	// getlogin
#include <sys/types.h>	// getpwname, getpwuid, getuid
#include <sys/time.h>
#include <pwd.h>	// getpwname, getpwuid, getuid
#include <grp.h>        // getgrnam
#include "mac_util.h"

void printUsage(void);
Boolean SetLoginItemOSAScript(Boolean addLogInItem, char *userName);
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
static int compareOSVersionTo(int toMajor, int toMinor);
OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN);
static double dtime(void);
static void SleepSeconds(double seconds);
static OSErr QuitOneProcess(OSType signature);


int main(int argc, char *argv[])
{
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
    
#ifndef _DEBUG
    if (getuid() != 0) {
        printf("This program must be run as root\n");
        printUsage();
        return 0;
    }
#endif

    if (argc < 3) {
        printUsage();
        return 0;
    }
    
    if (strcmp(argv[1], "-a") == 0) {
        AddUsers = true;
    } else if (strcmp(argv[1], "-s") == 0) {
        AddUsers = true;
        SetSavers = true;
    } else if (strcmp(argv[1], "-r") != 0) {
        printUsage();
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
            system(s);
        }
        
        if ((!isBPGroupMember) && AddUsers) {
            sprintf(s, "dscl . -merge /groups/boinc_project GroupMembership %s", pw->pw_name);
            system(s);
        }
        
        if (isBMGroupMember && (!AddUsers)) {
            sprintf(s, "dscl . -delete /Groups/boinc_master GroupMembership %s", pw->pw_name);
            system(s);
        }

        if (isBPGroupMember && (!AddUsers)) {
            sprintf(s, "dscl . -delete /Groups/boinc_project GroupMembership %s", pw->pw_name);
            system(s);
        }

        // Set or remove login item for this user
        SetLoginItemOSAScript(AddUsers, pw->pw_name);

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
                system(s);
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver modulePath \"/Library/Screen Savers/BOINCSaver.saver\"", 
                    pw->pw_name); 
                system(s);
            } else {
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver moduleDict -dict moduleName BOINCSaver path \"/Library/Screen Savers/BOINCSaver.saver\"", 
                        pw->pw_name);
                system(s);
            }
        }
        
        if (saverIsSet && (!AddUsers)) {
            if (compareOSVersionTo(10, 6) < 0) {
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver moduleName Flurry",
                    pw->pw_name); 
                system(s);
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver modulePath \"/System/Library/Screen Savers/Flurry.saver\"", 
                    pw->pw_name); 
                system(s);
            } else {
                sprintf(s, "sudo -u %s defaults -currentHost write com.apple.screensaver moduleDict -dict moduleName Flurry path \"/System/Library/Screen Savers/Flurry.saver\"", 
                        pw->pw_name);
                system(s);
            }
        }

        seteuid(saved_uid);                         // Set effective uid back to privileged user
    }

    printf("WARNING: Changes may require a system restart to take effect.\n");
    
    return 0;
}

void printUsage() {
    printf("Usage: sudo AddRemoveUser [-a | -s | -r] [user1 [user2 [user3...]]]\n");
    printf("   -a: add users to those with permission to run BOINC Manager.\n");
    printf("   -s: same as -a plus set users' screensaver to BOINC.\n");
    printf("   -r: remove users' permission to run BOINC Manager, and \n");
    printf("      if their screensaver was set to BOINC change it to Flurry.\n");
    printf("\n");
}

enum {
	kSystemEventsCreator = 'sevs'
};
CFStringRef kSystemEventsBundleID = CFSTR("com.apple.systemevents");

Boolean SetLoginItemOSAScript(Boolean addLogInItem, char *userName)
{
    int                     i, j;
    char                    cmd[2048];
    char                    systemEventsPath[1024];
    ProcessSerialNumber     SystemEventsPSN;
    CFURLRef                appURL = NULL;
    OSErr                   err, err2;

#if VERBOSE
    fprintf(stderr, "Adjusting login items for user %s\n", userName);
    fflush(stderr);
#endif

    // We must launch the System Events application for the target user

    err = FindProcess ('APPL', kSystemEventsCreator, &SystemEventsPSN);
    if (err == noErr) {
        // Find SystemEvents process.  If found, quit it in case 
        // it is running under a different user.
#if VERBOSE
        fprintf(stderr, "Telling System Events to quit (at start of SetLoginItemOSAScript)\n");
        fflush(stderr);
#endif
        err = QuitOneProcess(kSystemEventsCreator);
        if (err != noErr) {
            fprintf(stderr, "QuitOneProcess(kSystemEventsCreator) returned error %d \n", (int) err);
            fflush(stderr);
        }
        // Wait for the process to be gone
        for (i=0; i<50; ++i) {      // 5 seconds max delay
            SleepSeconds(0.1);      // 1/10 second
            err = FindProcess ('APPL', kSystemEventsCreator, &SystemEventsPSN);
            if (err != noErr) break;
        }
        if (i >= 50) {
            fprintf(stderr, "Failed to make System Events quit\n");
            fflush(stderr);
            err = noErr;
            goto cleanupSystemEvents;
        }
        sleep(4);
    }
    
    err = LSFindApplicationForInfo(kSystemEventsCreator, kSystemEventsBundleID, NULL, NULL, &appURL);
    if (err != noErr) {
        fprintf(stderr, "LSFindApplicationForInfo(kSystemEventsCreator) returned error %d \n", (int) err);
        fflush(stderr);
        goto cleanupSystemEvents;
    } else {
        CFStringRef CFPath = CFURLCopyFileSystemPath(appURL, kCFURLPOSIXPathStyle);
        CFStringGetCString(CFPath, systemEventsPath, sizeof(systemEventsPath), kCFStringEncodingUTF8);
        CFRelease(CFPath);
    if (appURL) {
        CFRelease(appURL);
    }
#if VERBOSE
        fprintf(stderr, "SystemEvents is at %s\n", systemEventsPath);
        fprintf(stderr, "Launching SystemEvents for user %s\n", userName);
        fflush(stderr);
#endif

        for (j=0; j<5; ++j) {
            sprintf(cmd, "sudo -u \"%s\" \"%s/Contents/MacOS/System Events\" &", userName, systemEventsPath);
            err = system(cmd);
            if (err) {
                fprintf(stderr, "Command: %s returned error %d (try %d of 5)\n", cmd, (int) err, j);
            }
            // Wait for the process to start
            for (i=0; i<50; ++i) {      // 5 seconds max delay
                SleepSeconds(0.1);      // 1/10 second
                err = FindProcess ('APPL', kSystemEventsCreator, &SystemEventsPSN);
                if (err == noErr) break;
            }
            if (i < 50) break;  // Exit j loop on success
        }
        if (j >= 5) {
            fprintf(stderr, "Failed to launch System Events for user %s\n", userName);
            fflush(stderr);
            err = noErr;
            goto cleanupSystemEvents;
        }
    }
    sleep(2);
    
#if VERBOSE
    fprintf(stderr, "Deleting any login items containing BOINCManager for user %s\n", userName);
    fflush(stderr);
#endif
    sprintf(cmd, "sudo -u \"%s\" osascript -e 'tell application \"System Events\"' -e 'delete (every login item whose path contains \"BOINCManager\")' -e 'end tell'", userName);
    err = system(cmd);
    if (err) {
        fprintf(stderr, "Command: %s\n", cmd);
        fprintf(stderr, "Delete login item containing \"BOINCManager\" returned error %d\n", err);
        fflush(stderr);
    }

    if (addLogInItem == false) {
        err = noErr;
        goto cleanupSystemEvents;
    }
    
    fprintf(stdout, "Making new login item %s for user %s\n", "BOINCManager", userName);
    fflush(stderr);
    sprintf(cmd, "sudo -u \"%s\" osascript -e 'tell application \"System Events\"' -e 'make new login item at end with properties {path:\"/Applications/BOINCManager.app\", hidden:true, name:\"BOINCManager\"}' -e 'end tell'", userName);
    err = system(cmd);
    if (err) {
        fprintf(stderr, "Command: %s\n", cmd);
        printf("Make login item for %s returned error %d\n", "/Applications/BOINCManager.app", err);
    }
    fflush(stderr);

cleanupSystemEvents:
    // Clean up in case this was our last user
#if VERBOSE
    fprintf(stderr, "Telling System Events to quit (at end of SetLoginItemOSAScript)\n");
    fflush(stderr);
#endif
    err2 = QuitOneProcess(kSystemEventsCreator);
    if (err2 != noErr) {
        fprintf(stderr, "QuitOneProcess(kSystemEventsCreator) returned error %d \n", (int) err2);
        fflush(stderr);
    }
    // Wait for the process to be gone
    for (i=0; i<50; ++i) {      // 5 seconds max delay
        SleepSeconds(0.1);      // 1/10 second
        err2 = FindProcess ('APPL', kSystemEventsCreator, &SystemEventsPSN);
        if (err2 != noErr) break;
    }
    if (i >= 50) {
        fprintf(stderr, "Failed to make System Events quit\n");
        fflush(stderr);
    }
    
    sleep(4);
        
    return (err == noErr);
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

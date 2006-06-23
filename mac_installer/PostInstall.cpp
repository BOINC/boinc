// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/* PostInstall.c */

#define CREATE_LOG 1    /* for debugging */

#include <Carbon/Carbon.h>
#include <grp.h>

#include <unistd.h>	// getlogin
#include <sys/types.h>	// getpwname, getpwuid, getuid
#include <pwd.h>	// getpwname, getpwuid, getuid
#include <sys/wait.h>	// waitpid
#include <dirent.h>

#include "LoginItemAPI.h"  //please take a look at LoginItemAPI.h for an explanation of the routines available to you.

#include "SetupSecurity.h"

void Initialize(void);	/* function prototypes */
int DeleteReceipt(void);
void SetLoginItem(long brandID);
void SetUIDBackToUser (void);
OSErr UpdateAllVisibleUsers(long brandID);
long GetBrandID(void);
OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN);
pid_t FindProcessPID(char* name, pid_t thePID);
static OSErr QuitBOINCManager(OSType signature);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

extern int check_security(char *bundlePath, char *dataPath);


Boolean			gQuitFlag = false;	/* global */

int main(int argc, char *argv[])
{
    Boolean                 Success;
    long                    response;
    ProcessSerialNumber     ourProcess, installerPSN;
    short                   itemHit;
    long                    brandID = 0;
    int                     i;
    pid_t                   installerPID = 0, coreClientPID = 0;
    FSRef                   fileRef;
    OSStatus                err, err_fsref;
    char                    *p;
#ifndef SANDBOX
    char                    *q;
    group                   *grp;
    char                    s[256];
#endif
    
    for (i=0; i<argc; i++) {
        if (strcmp(argv[i], "-part2") == 0) {
            return DeleteReceipt();
        }
    }
    Initialize();

    ::GetCurrentProcess (&ourProcess);

    QuitBOINCManager('BNC!'); // Quit any old instance of BOINC manager
    sleep(2);

    // Core Client may still be running if it was started without Manager
    coreClientPID = FindProcessPID("boinc", 0);
    if (coreClientPID)
        kill(coreClientPID, SIGTERM);   // boinc catches SIGTERM & exits gracefully

    err = FindProcess ('APPL', 'xins', &installerPSN);
    if (err == noErr)
        err = GetProcessPID(&installerPSN , &installerPID);

    brandID = GetBrandID();

    err = Gestalt(gestaltSystemVersion, &response);
    if (err != noErr)
        return err;
    
    if (response < 0x1030) {
        ::SetFrontProcess(&ourProcess);
        // Remove everything we've installed
        if (brandID == 1) {
            StandardAlert (kAlertStopAlert, "\pSorry, GridRepublic requires system 10.3 or higher.",
                                                NULL, NULL, &itemHit);
            system ("rm -rf /Applications/GridRepublic\\ Desktop.app");
            system ("rm -rf /Library/Screen\\ Savers/GridRepublic.saver");
// We don't customize BOINC Data directory name for branding
//            system ("rm -rf /Library/Application\\ Support/GridRepublic\\ Data");
            system ("rm -rf /Library/Receipts/GridRepublic.pkg");
            StandardAlert (kAlertStopAlert, "\pSorry, BOINC requires system 10.3 or higher.",
                                                NULL, NULL, &itemHit);
	} else {
            system ("rm -rf /Applications/BOINCManager.app");
            system ("rm -rf /Library/Screen\\ Savers/BOINCSaver.saver");
// We don't customize BOINC Data directory name for branding
//            system ("rm -rf /Library/Application\\ Support/BOINC\\ Data");
            system ("rm -rf /Library/Receipts/BOINC.pkg");
        }

        system ("rm -rf /Library/Application\\ Support/BOINC\\ Data");

        err = kill(installerPID, SIGKILL);

	ExitToShell();
    }
    
    Success = false;
    
#ifdef SANDBOX

   if (brandID == 1)
        p = "/Applications/GridRepublic Desktop.app";
    else
        p = "/Applications/BOINCManager.app";

    for (i=0; i<5; ++i) {
        err = CreateBOINCUsersAndGroups();
        if (err != noErr) {
//          print_to_log_file("CreateBOINCUsersAndGroups returned %d (repetition=%d)", err, i);
            continue;
        }
        
       if (brandID == 1)
            err = SetBOINCAppOwnersGroupsAndPermissions(p);
        else
            err = SetBOINCAppOwnersGroupsAndPermissions(p);
        
        if (err != noErr) {
//          print_to_log_file("SetBOINCAppOwnersGroupsAndPermissions returned %d (repetition=%d)", err, i);
            continue;
        }

        err = SetBOINCDataOwnersGroupsAndPermissions();
        if (err != noErr) {
//          print_to_log_file("SetBOINCDataOwnersGroupsAndPermissions returned %d (repetition=%d)", err, i);
            continue;
        }
        
        err = check_security(p, "/Library/Application Support/BOINC Data");
        if (err == noErr)
            break;
//          print_to_log_file("check_security returned %d (repetition=%d)", err, i);
    }
    
#else   // ! defined(SANDBOX)

    // The BOINC Manager and Core Client have the set-user-ID-on-execution 
    // flag set, so their ownership is important and must match the 
    // ownership of the BOINC Data directory.
    
    // Find an appropriate admin user to set as owner of installed files
    // First, try the user currently logged in
    q = getlogin();

    grp = getgrnam("admin");
    i = 0;
    while ((p = grp->gr_mem[i]) != NULL) {   // Step through all users in group admin
        if (strcmp(p, q) == 0) {
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

    // Set owner of BOINCManager and contents, including core client
   if (brandID == 1) {
        sprintf(s, "chown -Rf %s /Applications/GridRepublic\\ Desktop.app", p);
        system (s);

        // Set owner of BOINC Screen Saver
        sprintf(s, "chown -Rf %s /Library/Screen\\ Savers/GridRepublic.saver", p);
        system (s);

//      We don't customize BOINC Data directory name for branding
        // Set owner of GridRepublic Data
//        sprintf(s, "chown -Rf %s /Library/Application\\ Support/GridRepublic\\ Data", p);
        sprintf(s, "chown -Rf %s /Library/Application\\ Support/BOINC\\ Data", p);
        system (s);

	system ("chmod -R a+s /Applications/GridRepublic\\ Desktop.app");   // Installing GridRepublic over BOINC
    } else {
        sprintf(s, "chown -Rf %s /Applications/BOINCManager.app", p);
        system (s);

        // Set owner of BOINC Screen Saver
        sprintf(s, "chown -Rf %s /Library/Screen\\ Savers/BOINCSaver.saver", p);
        system (s);

// We don't customize BOINC Data directory name for branding
        // Set owner of BOINC Data
        sprintf(s, "chown -Rf %s /Library/Application\\ Support/BOINC\\ Data", p);
        system (s);
	system ("chmod -R a+s /Applications/BOINCManager.app");
    }

#endif   // ! defined(SANDBOX)

   if (brandID == 1) {
 	system ("rm -rf /Applications/BOINCManager.app");                   // Installing GridRepublic over BOINC
	system ("rm -rf /Library/Screen\\ Savers/BOINCSaver.saver");        // Installing GridRepublic over BOINC

        err_fsref = FSPathMakeRef((StringPtr)"/Applications/GridRepublic Desktop.app", &fileRef, NULL);
    } else {
	system ("rm -rf /Applications/GridRepublic\\ Desktop.app");     // Installing BOINC over GridRepublic
	system ("rm -rf /Library/Screen\\ Savers/GridRepublic.saver");  // Installing BOINC over GridRepublic

        err_fsref = FSPathMakeRef((StringPtr)"/Applications/BOINCManager.app", &fileRef, NULL);
    }
    
    if (err_fsref == noErr)
        err = LSRegisterFSRef(&fileRef, true);
    
#ifdef SANDBOX
    err = UpdateAllVisibleUsers(brandID);
    if (err != noErr)
        return err;
#else
    // Installer is running as root.  We must setuid back to the logged in user 
    //  in order to add a startup item to the user's login preferences
    SetUIDBackToUser ();
    SetLoginItem(brandID);
#endif
 
    return 0;
}


int DeleteReceipt()
{
    ProcessSerialNumber     installerPSN;
    long                    brandID = 0;
    int                     i;
    pid_t                   installerPID = 0;
    OSStatus                err;
//    FSRef                   fileRef;
//    OSStatus                err_fsref;

    Initialize();

    err = FindProcess ('APPL', 'xins', &installerPSN);
    if (err == noErr)
        err = GetProcessPID(&installerPSN , &installerPID);


   // Launch BOINC Manager when user closes installer or after 15 seconds

    for (i=0; i<15; i++) { // Wait 15 seconds max for installer to quit
        sleep (1);
        if (err == noErr)
            if (FindProcessPID(NULL, installerPID) == 0)
                break;
    }

    brandID = GetBrandID();

    // Remove installer package receipt so we can run installer again if needed to fix permissions
   if (brandID == 1) {
        system ("rm -rf /Library/Receipts/GridRepublic.pkg");
//        err_fsref = FSPathMakeRef((StringPtr)"/Applications/GridRepublic Desktop.app", &fileRef, NULL);
    } else {
        system ("rm -rf /Library/Receipts/BOINC.pkg");
//        err_fsref = FSPathMakeRef((StringPtr)"/Applications/BOINCManager.app", &fileRef, NULL);
    }
    
//    if (err_fsref == noErr)
//        err = LSOpenFSRef(&fileRef, NULL);

    return 0;
}


void SetLoginItem(long brandID)
{
    Boolean                 Success;
    int                     NumberOfLoginItems, Counter;
    char                    *p, *q;

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
            
        if (strcmp(p, "BOINCMANAGER.APP") == 0)
            Success = RemoveLoginItemAtIndex(kCurrentUser, Counter-1);
        if (strcmp(p, "GRIDREPUBLIC DESKTOP.APP") == 0)
            Success = RemoveLoginItemAtIndex(kCurrentUser, Counter-1);
    }

    if (brandID == 1)
        Success = AddLoginItemWithPropertiesToUser(kCurrentUser,
                            "/Applications/GridRepublic Desktop.app", kDoNotHideOnLaunch);
    else
        Success = AddLoginItemWithPropertiesToUser(kCurrentUser,
                            "/Applications/BOINCManager.app", kDoNotHideOnLaunch);
}

void SetUIDBackToUser (void)
{
    char *p;
    uid_t login_uid;
    passwd *pw;

    p = getlogin();
    pw = getpwnam(p);
    login_uid = pw->pw_uid;

    setuid(login_uid);
}

// Find all visible users and set their login item to launch BOIN CManager.
// If user is a member of group admin, add user to groups boinc_master and boinc_project.
OSErr UpdateAllVisibleUsers(long brandID)
{
    DIR                 *dirp;
    dirent              *dp;
    char                *p;
    group               *grp;
    passwd              *pw;
    uid_t               saved_uid;
    short               i;
    OSErr 		err = noErr;

    grp = getgrnam("admin");
    if (grp == NULL) {      // Should never happen
        puts("getgrnam(\"admin\") failed\n");
        return -1;
    }
    dirp = opendir("/Users");
    if (dirp == NULL) {      // Should never happen
        puts("opendir(\"/Users\") failed\n");
        return -1;
    }
    
    while (true) {
        dp = readdir(dirp);
        if (dp == NULL)
            break;                  // End of list
            
        if (dp->d_name[0] == '.')
            continue;               // Ignore names beginning with '.'
    
        pw = getpwnam(dp->d_name);
        if (pw == NULL)             // "Deleted Users", "Shared", etc.
            continue;

        i = 0;
        while ((p = grp->gr_mem[i]) != NULL) {  // Step through all users in group admin
            if (strcmp(p, dp->d_name) == 0) {
                // User is a member of group admin, so add user to groups boinc_master and boinc_project
                err = AddAdminUserToGroups(p);
                if (err != noErr)
                    return err;
                break;
            }
            ++i;
        }

        saved_uid = geteuid();
        seteuid(pw->pw_uid);                // Temporarily set effective uid to this user
        SetLoginItem(brandID);                     // Set login item for this user
        seteuid(saved_uid);                 // Set effective uid back to privileged user
    }
    
    closedir(dirp);
    return noErr;
}


void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
    InitCursor();

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
        if (name != NULL) {     // Search ny name
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


static OSErr QuitBOINCManager(OSType signature) {
    bool			done = false;
    ProcessSerialNumber         thisPSN;
    ProcessInfoRec		thisPIR;
    OSErr                       err = noErr;
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


static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;
    
    return noErr;
}


// For debugging
void print_to_log_file(const char *format, ...) {
#if CREATE_LOG
    FILE *f;
    va_list args;
    char buf[256];
    time_t t;
    strcpy(buf, getenv("HOME"));
    strcat(buf, "/Documents/test_log.txt");
    f = fopen(buf, "a");
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
#endif
}

#if CREATE_LOG
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
#endif	// CREATE_LOG
const char *BOINC_RCSID_c7abe0490e="$Id$";

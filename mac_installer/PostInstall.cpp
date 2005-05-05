// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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

#include <unistd.h>	// getlogin
#include <sys/types.h>	// getpwname, getpwuid, getuid
#include <pwd.h>	// getpwname, getpwuid, getuid
#include <sys/wait.h>	// waitpid

#include "LoginItemAPI.h"  //please take a look at LoginItemAPI.h for a explanation of the routines available to you.

void Initialize(void);	/* function prototypes */
void SetUIDBackToUser (void);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
static OSErr DoKillInstaller(void);
void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

Boolean			gQuitFlag = false;	/* global */

int main(int argc, char *argv[])
{
    char *p, *q;
    Boolean Success;
    long response;
    ProcessSerialNumber	ourProcess;
    short itemHit;
    int NumberOfLoginItems, Counter;
    pid_t myPid;
    OSStatus err;

    Initialize();

    Success = false;
    
    err = Gestalt(gestaltSystemVersion, &response);
    if (err != noErr)
        return err;
    
    if (response < 0x1030) {
        ::GetCurrentProcess (&ourProcess);
        ::SetFrontProcess(&ourProcess);
        StandardAlert (kAlertStopAlert, "\pSorry, BOINC requires system 10.3 or higher.",
                                                NULL, NULL, &itemHit);

        // Remove everything we've installed
	system ("rm -rf /Applications/BOINCManager.app");
	system ("rm -rf /Library/Screen\\ Savers/BOINCSaver.saver");
	system ("rm -rf /Library/Application\\ Support/BOINC\\ Data");
	system ("rm -rf /Library/Receipts/BOINC.pkg");
	
	DoKillInstaller();
	ExitToShell();
    }
    
// *****************************************************************************************
//  Everything after this REQUIRES us to be setuid to the login user's user ID
// *****************************************************************************************
    // Installer is running as root.  We must setuid back to the logged in user 
    //  in order to add a startup item to the user's login preferences

    SetUIDBackToUser ();

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
    }

    Success = AddLoginItemWithPropertiesToUser(kCurrentUser,
                            "/Applications/BOINCManager.app", kDoNotHideOnLaunch);
    

    // Fork a process to launch the BOINCManager after the installer quits
    if ( (myPid = fork()) < 0)
        return -1;
    else if (myPid == 0)			// child

    {
        // Give installer time to finish
        sleep (7);
        system("/Applications/BOINCManager.app/Contents/MacOS/BOINCManager");
    }
    
    return 0;
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


void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
}


static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;
    
    return noErr;
}


static OSErr DoKillInstaller(void)
{
	bool				done = false;
	ProcessSerialNumber thisPSN;
	ProcessInfoRec		thisPIR;
	OSErr				err = noErr;
	Str63				thisProcessName;
	

	thisPIR.processInfoLength = sizeof (ProcessInfoRec);
	thisPIR.processName = thisProcessName;
	thisPIR.processAppSpec = nil;
	
	thisPSN.highLongOfPSN = 0;
	thisPSN.lowLongOfPSN = kNoProcess;
	
	while (done == false) {		
		err = GetNextProcess(&thisPSN);
		if (err == procNotFound)	
			done = true;		// apparently the installer isn't running.  This makes my head hurt.
		else {		
			err = GetProcessInformation(&thisPSN,&thisPIR);
			if (err != noErr)
				goto bail;
				
			if (thisPIR.processSignature == 'xins') {	// is it an instance of the package installer?
				pid_t thisPID;				// if so, we need to kill it off with SIGKILL
															// (and we hope there's only one installer running now)					
				err = GetProcessPID(&thisPSN , &thisPID);
				if (err == noErr)
					err = kill(thisPID, SIGKILL);

				done = true;		// we've done it... we're a patricide
			}
		}
	}

bail:

	return err;

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

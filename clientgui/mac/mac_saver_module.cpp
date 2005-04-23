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

//
//  mac_saver_module.cpp
//  BOINC_Saver_Module
//

#include <Carbon/Carbon.h>

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "gui_rpc_client.h"

//#include <drivers/event_status_driver.h>
#ifdef __cplusplus
extern "C" {
#endif

void setFrame(Rect *frame);
int initBOINCSaver(Boolean ispreview);
OSStatus initBOINCApp(void);
int drawGraphics(GrafPtr aPort);
void drawPreview(GrafPtr aPort);
void closeBOINCSaver(void);
void setBannerText(ConstStringPtr msg, GrafPtr aPort);
void drawBanner(GrafPtr aPort);
void FlashIcon(void);
OSErr FindBOINCApplication(FSSpecPtr applicationFSSpecPtr);
pid_t FindProcessPID(char* name, pid_t thePID);
OSErr GetpathToBOINCManagerApp(char* path, int maxLen);
OSStatus RPCThread(void* param);

#ifdef __cplusplus
}	// extern "C"
#endif

#define CREATE_LOG 1

#ifdef __cplusplus
extern "C" {
#endif
void print_to_log_file(const char *format, ...);
#ifdef __cplusplus
}

void strip_cr(char *buf);
#endif

#define BANNER_GAP 30	/* Space between repeats of banner text */

enum SaverState {
    SaverState_Idle,
    SaverState_LaunchingCoreClient,
    SaverState_CoreClientRunning,
    SaverState_CoreClientSetToSaverMode,
    
    SaverState_CantLaunchCoreClient,
    SaverState_UnrecoverableError
};


extern int gGoToBlank;      // True if we are to blank the screen
extern int gBlankingTime;   // Delay in minutes before blanking the screen


static Boolean wasAlreadyRunning;
static pid_t CoreClientPID = nil;
static Str255 bannerText;
static int bannerWidth;
static SaverState saverState = SaverState_Idle;
static StringPtr CurrentBannerMessage = 0;
static DISPLAY_INFO di;     // Contains all NULLs for the Macintosh
static RPC_CLIENT *rpc;
MPQueueID gTerminationQueue;	// This queue will report the completion of our threads
MPTaskID gRPC_thread_id;	// IDs of the thread we create
int gClientSaverStatus = 0;     // status returned by get_screensaver_mode RPC
Boolean gQuitRPCThread = false; // Flag to tell RPC thread to exit gracefully

const ConstStringPtr CantLaunchCCMsg = "\pUnable to launch BOINC application.";
const ConstStringPtr LaunchingCCMsg = "\pLaunching BOINC application.";
const ConstStringPtr BOINCSuspendedMsg = "\pBOINC is currently suspended.";
const ConstStringPtr BOINCNoAppsExecutingMsg = "\pBOINC is currently idle.";
const ConstStringPtr BOINCNoProjectsDetectedMsg = "\pBOINC is not attached to any projects. Please attach to projects using the BOINC Manager.";
const ConstStringPtr BOINCNoGraphicAppsExecutingMsg = "\pBOINC is currently not executing any applications with graphics";
const ConstStringPtr BOINCUnrecoverableErrorMsg = "\pSorry, an unrecoverable error occurred";



// Returns desired Animation Frequency (per second) or 0 for no change
int initBOINCSaver(Boolean ispreview) {
    int newFrequency = 15;
    OSStatus err;
    
    if (ispreview)
        return 8;
        
    setBannerText(0, NULL);
    
    if (ispreview)
        return 0;

    // If there are multiple displays, initBOINCSaver may get called 
    // multiple times (once for each display), so we need to guard 
    // against launching multiple instances of the core client
    if (saverState == SaverState_Idle) {
        err = initBOINCApp();

        if (saverState == SaverState_LaunchingCoreClient)
        {
            gClientSaverStatus = 0;
            gQuitRPCThread = false;
            if (rpc == NULL)
                rpc = new RPC_CLIENT;
            newFrequency = 4;
        }
    }
    return newFrequency;
}


OSStatus initBOINCApp() {
    char boincPath[2048], buf[256];
    pid_t myPid;
    int status;
    OSStatus err;

    saverState = SaverState_CantLaunchCoreClient;
    
    CoreClientPID = FindProcessPID("boinc", 0);
    if (CoreClientPID) {
        wasAlreadyRunning = true;
        saverState = SaverState_LaunchingCoreClient;
        return noErr;
    }
    
    err = GetpathToBOINCManagerApp(boincPath, sizeof(boincPath));
    if (err)    // If we couldn't find BOINCManager.app, try default path
        strcpy(boincPath, "/Applications/BOINCManager.app");    // 
        
    strcat(boincPath, "/Contents/Resources/boinc");

    if ( (myPid = fork()) < 0)
        return -1;
    else if (myPid == 0)			// child
    {
        strcpy(buf, getenv("HOME"));
        strcat(buf, "/Library/Application Support/BOINC Data");
        status = chdir(buf);
        if (status) {
            perror("chdir");
            _exit(status);
        }

        status = execl(boincPath, " -redirectio", (char *) 0);
        _exit(127);         // execl error (execl should never return)
    } else {
        CoreClientPID = myPid;		// make this available globally
        saverState = SaverState_LaunchingCoreClient;
    }

    return noErr;
}


// Returns new desired Animation Frequency (per second) or 0 for no change
int drawGraphics(GrafPtr aPort) {

    CGrafPtr savePort;
    GDHandle saveGDH;
    int newFrequency = 15;
    pid_t myPid;
    OSStatus err;
    
    ObscureCursor();

    switch (saverState) {
    case  SaverState_LaunchingCoreClient:
        setBannerText(LaunchingCCMsg, aPort);
        
        myPid = FindProcessPID(NULL, CoreClientPID);
        if (myPid) {
            saverState = SaverState_CoreClientRunning;
            rpc->init(NULL);   // Initialize communications with Core Client

            // Set up a separate thread for communicating with Core Client
            if (!MPLibraryIsLoaded()) {
                saverState = SaverState_UnrecoverableError;
                break;
            }

            if (gTerminationQueue == NULL) {
                err = MPCreateQueue(&gTerminationQueue);	/* Create the queue which will report the completion of the task. */
                if (err) {
                    saverState = SaverState_UnrecoverableError;
                    break;
                }
            }
            err = MPCreateTask(RPCThread,	/* This is the task function. */
                        0,		/* This is the parameter to the task function. */
                        (50*1024),		/* Stack size for thread. */
                        gTerminationQueue,	/* We'll use this to sense task completion. */
                        0,	/* We won't use the first part of the termination message. */
                        0,	/* We won't use the second part of the termination message. */
                        0,	/* Use the normal task options. (Currently this *must* be zero!) */
                        &gRPC_thread_id);	/* Here's where the ID of the new task will go. */
            
            if (err) {
                MPDeleteQueue(gTerminationQueue);
                saverState = SaverState_UnrecoverableError;
                break;
            }
        }
        // ToDo: Add a timeout after which we display error message
        break;

    case SaverState_CoreClientRunning:
            // set_screensaver_mode RPC called in RPCThread()
        break;
    
    case SaverState_CoreClientSetToSaverMode:
        switch (gClientSaverStatus) {
        case 0:
            break;  // No status response yet from get_screensaver_mode RPC
        case SS_STATUS_ENABLED:
        default:
            setBannerText(0, aPort);   // No text message
            // Let the science app draw over our window
            break;
        case SS_STATUS_BLANKED:
            setBannerText(0, aPort);   // No text message
            break;
        case SS_STATUS_BOINCSUSPENDED:
            setBannerText(BOINCSuspendedMsg, aPort);
            break;
        case SS_STATUS_NOAPPSEXECUTING:
            setBannerText(BOINCNoAppsExecutingMsg, aPort);
            break;
        case SS_STATUS_NOPROJECTSDETECTED:
            setBannerText(BOINCNoProjectsDetectedMsg, aPort);
            break;
        case SS_STATUS_NOGRAPHICSAPPSEXECUTING:
            setBannerText(BOINCNoGraphicAppsExecutingMsg, aPort);
            break;
        case SS_STATUS_QUIT:
            // Handled in RPCThread()
            break;
        }       // end switch (gClientSaverStatus)
        break;

    case SaverState_UnrecoverableError:
        setBannerText(BOINCUnrecoverableErrorMsg, aPort);
        break;

    case SaverState_CantLaunchCoreClient:
        setBannerText(CantLaunchCCMsg, aPort);
        break;

    case SaverState_Idle:
        break;      // Should never get here; fixes compiler warning
    }           // end switch (saverState)

    if (bannerText[0]) {
        GetGWorld(&savePort, &saveGDH);
        SetPort(aPort);
        drawBanner(aPort);
        SetGWorld(savePort, saveGDH);
        newFrequency = 45;
    } else
        newFrequency = 4;
    
    return newFrequency;
}


void drawPreview(GrafPtr aPort) {
    SetPort(aPort);
    setBannerText("\p BOINC", aPort);
    drawBanner(aPort);
}


// If there are multiple displays, closeBOINCSaver may get called 
// multiple times (once for each display), so we need to guard 
// against any problems that may cause.
void closeBOINCSaver() {
    gQuitRPCThread = true;  // Tell RPC Thread to exit
    if (gTerminationQueue) {  // Wait up to 10 seconds for RPC thread to exit
        MPWaitOnQueue(gTerminationQueue, NULL, NULL, NULL, kDurationMillisecond*10000);
        MPDeleteQueue(gTerminationQueue);
        gTerminationQueue = NULL;
    }
    
    if (rpc)
    {
        rpc->set_screensaver_mode(false, 0, di);
        delete rpc;
    }
    rpc = NULL;

    setBannerText(0, NULL);
    
    // Kill core client if we launched it
    if (!wasAlreadyRunning)
        if (CoreClientPID)
        {
            kill(CoreClientPID, SIGTERM);
        }
        CoreClientPID = NULL;
        wasAlreadyRunning = false;
        gQuitRPCThread = false;
        saverState = SaverState_Idle;
}


OSStatus RPCThread(void* param) {
    int val = 0;
//    CC_STATE state;
    AbsoluteTime timeToUnblock;
    long time_to_blank;

    while (true) {
        if (gQuitRPCThread)     // If main thread has requested we exit
            MPExit(noErr);       // Exit the thread

        timeToUnblock = AddDurationToAbsolute(durationSecond/4, UpTime());
        MPDelayUntil(&timeToUnblock);

        // Calculate the estimated blank time by adding the current time
        //   and and the user specified time which is in minutes
        if (gGoToBlank)
            time_to_blank = time(0) + (gBlankingTime * 60);
        else
            time_to_blank = 0;
        val = rpc->set_screensaver_mode(true, time_to_blank, di);
        if (val == noErr)
            break;
        
        // Attempt to reinitialize the RPC client and state
        rpc->close();
        rpc->init(NULL);
        // Error message after timeout?
    }

    saverState = SaverState_CoreClientSetToSaverMode;

    while (true) {
        if (gQuitRPCThread)     // If main thread has requested we exit
{
            MPExit(noErr);       // Exit the thread
}

        timeToUnblock = AddDurationToAbsolute(durationSecond/4, UpTime());
        MPDelayUntil(&timeToUnblock);

#if 0
        val = rpc->get_state(state);
        if (val) {
        }
#endif        
        val = rpc->get_screensaver_mode(gClientSaverStatus);
        if (val) {
            // Attempt to reinitialize the RPC client and state
            rpc->close();
            rpc->init(NULL);
            // Error message after timeout?
        }
        
        if (gClientSaverStatus == SS_STATUS_QUIT) {
            rpc->set_screensaver_mode(false, 0, di);
            MPExit(noErr);       // Exit the thread
        }
    }
}



void setBannerText(ConstStringPtr msg, GrafPtr aPort) {
    CGrafPtr savePort;
    RGBColor saveBackColor;
    Rect wRect;
    
    if ((ConstStringPtr)CurrentBannerMessage == msg)
        return;
    
    CurrentBannerMessage = (StringPtr)msg;
    
    if (msg == 0) {
        bannerText[0] = 0;
        if (aPort != NULL)
            drawBanner(aPort);
        return;
    }
        
    GetPort(&savePort);
    SetPort(aPort);

    GetPortBounds(aPort, &wRect);

    GetBackColor(&saveBackColor);
    BackColor ( blackColor );
    EraseRect(&wRect);
    RGBBackColor(&saveBackColor);
   
    BlockMove(msg, bannerText, msg[0]+1);
    TextSize(24);
    TextFace(bold);
    bannerWidth = StringWidth(bannerText) + BANNER_GAP;
    SetPort(savePort);
}


void drawBanner(GrafPtr aPort) {
    CGrafPtr savePort;
    GDHandle saveGDH;
    RGBColor saveForeColor, saveBackColor;
    short x, y;
    Rect wRect;
    FontInfo fInfo;

    static short bannerPos;
    
    GetGWorld(&savePort, &saveGDH);
    SetPort(aPort);
    
    GetForeColor(&saveForeColor);
    GetBackColor(&saveBackColor);
    ForeColor(whiteColor);
    BackColor(blackColor);
    GetPortBounds(aPort, &wRect);
    if ( (bannerPos + bannerWidth) < wRect.left)
        bannerPos = wRect.left;
    else
        bannerPos -= 1;

   x = bannerPos;
   y = (wRect.bottom - wRect.top) / 3 + wRect.top;
   
    GetFontInfo(&fInfo);
    wRect.top = y - fInfo.ascent;
    wRect.bottom = y + fInfo.descent;
    EraseRect(&wRect);
       
    do { 
        MoveTo(x, y);
         DrawString(bannerText);
        x+= bannerWidth;
    } while (x < wRect.right);
    
    RGBForeColor(&saveForeColor);
    RGBBackColor(&saveBackColor);

    SetGWorld(savePort, saveGDH);
}


pid_t FindProcessPID(char* name, pid_t thePID)
{
    FILE *f;
    char buf[1024];
    size_t n;
    pid_t aPID;
    
    if (name != NULL)     // Search ny name
        n = strlen(name);
    
    f = popen("ps -a -x -c -o command,pid", "r");
    if (f == NULL)
        return 0;
    
    while (fgets(buf, sizeof(buf), f))
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


OSErr GetpathToBOINCManagerApp(char* path, int maxLen)
{
    CFStringRef bundleID = CFSTR("edu.berkeley.boinc");
    OSType creator = 'BNC!';
    FSRef theFSRef;
    OSStatus status = noErr;

    status = LSFindApplicationForInfo(creator, bundleID, NULL, &theFSRef, NULL);
    if (status == noErr)
        status = FSRefMakePath(&theFSRef, (unsigned char *)path, maxLen);
    return status;
}


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

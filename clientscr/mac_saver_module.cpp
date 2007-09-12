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
#include <sys/wait.h>

#include "gui_rpc_client.h"
#include "common_defs.h"
#include "util.h"
#include "screensaver.h"

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
void setBannerText(const char *msg, GrafPtr aPort);
void updateBannerText(char *msg, GrafPtr aPort);
void drawBanner(GrafPtr aPort);
int GetBrandID(void);
void FlashIcon(void);
OSErr FindBOINCApplication(FSSpecPtr applicationFSSpecPtr);
pid_t FindProcessPID(char* name, pid_t thePID, pid_t thePPID);
OSErr GetpathToBOINCManagerApp(char* path, int maxLen);
OSStatus RPCThread(void* param);
void HandleRPCError(void);
OSErr KillScreenSaver(void);
int Mac_launch_screensaver(RESULT* rp, int& graphics_application, pid_t& launcher_shell);

#ifdef __cplusplus
}	// extern "C"
#endif

// The science application can't display its graphics if, due to Fast User Switching, 
// Switching, the active user is not the one who launched BOINC.  If we always display 
// the progress information as scrolled text whenever a science application is running,
// then the progress text would appear in this case.  The text would be covered up by 
// the graphics if they are displayed. 
// But displaying the scrolled progress info takes up too much CPU time.  We need to 
// find a way to determine when Fast User Switching prevents graphics display and 
// display scrolled progress text only in that case.
// Of course, we also display scrolled progress text when the science application does 
// not support graphics).
#define ALWAYS_DISPLAY_PROGRESS_TEXT 0

// Flags for testing & debugging
#define SIMULATE_NO_GRAPHICS 0
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
#define BANNERDELTA 2   /* Number of pixels to move banner each frame */
#define BANNERFREQUENCY 90 /* Number of times per second to scroll banner */
#define NOBANNERFREQUENCY 4 /* Times per second to call drawGraphics if no banner */
#define STATUSUPDATEINTERVAL 5 /* seconds between status display updates */
#define TASK_RUN_CHECK_PERIOD 5  /* Seconds between safety check that task is actually running */
#define GFX_CHANGE_PERIOD 600 /* if > 1 CPUs, change screensaver every 600 secs */

enum SaverState {
    SaverState_Idle,
    SaverState_LaunchingCoreClient,
    SaverState_CoreClientRunning,
    SaverState_RelaunchCoreClient,
    SaverState_CoreClientSetToSaverMode,
    
    SaverState_CantLaunchCoreClient,
    SaverState_ControlPanelTestMode,
    SaverState_UnrecoverableError
};


extern int gGoToBlank;      // True if we are to blank the screen
extern int gBlankingTime;   // Delay in minutes before blanking the screen

static time_t SaverStartTime;
static Boolean wasAlreadyRunning = false;
static pid_t CoreClientPID = nil;
static char msgBuf[2048], bannerText[2048];
static int bannerWidth;
static SaverState saverState = SaverState_Idle;
static StringPtr CurrentBannerMessage = 0;
static DISPLAY_INFO di;     // Contains all NULLs for the Macintosh
static RPC_CLIENT *rpc;
MPQueueID gTerminationQueue;	// This queue will report the completion of our threads
MPTaskID gRPC_thread_id;	// IDs of the thread we create
int gClientSaverStatus = 0;     // status determined by RPCThread
Boolean gQuitRPCThread = false; // Flag to tell RPC thread to exit gracefully
int gQuitCounter = 0;
long gBrandId = 0;
char * gBrandText = "BOINC";
RGBColor gBrandColor = {0xFFFF, 0xFFFF, 0xFFFF};
RGBColor gTextColor = {0xFFFF, 0xFFFF, 0xFFFF};
RGBColor gWhiteTextColor = {0xFFFF, 0xFFFF, 0xFFFF};
RGBColor gOrangeTextColor = {0xFFFF, 0x6262, 0x0000};
RGBColor gGrayTextColor = {0x9999, 0x9999, 0x9999};


// Display first status update after 5 seconds
static int statusUpdateCounter = ((STATUSUPDATEINTERVAL-5) * BANNERFREQUENCY);
Boolean gStatusMessageUpdated = false;

const char * CantLaunchCCMsg = "Unable to launch BOINC application.";
const char *  LaunchingCCMsg = "Launching BOINC application.";
const char *  ConnectingCCMsg = "Connecting to BOINC application.";
const char *  BOINCSuspendedMsg = "BOINC is currently suspended.";
const char *  BOINCNoAppsExecutingMsg = "BOINC is currently idle.";
const char *  BOINCNoProjectsDetectedMsg = "BOINC is not attached to any projects. Please attach to projects using the BOINC Manager.";
const char *  BOINCNoGraphicAppsExecutingMsg = "Project does not support screensaver graphics: ";
const char *  BOINCUnrecoverableErrorMsg = "Sorry, an unrecoverable error occurred";
const char *  BOINCTestmodeMsg = "BOINC screensaver is running, but cannot display graphics in test mode.";
//const char *  BOINCExitedSaverMode = "BOINC is no longer in screensaver mode.";


// Returns desired Animation Frequency (per second) or 0 for no change
int initBOINCSaver(Boolean ispreview) {
    int newFrequency = 15;
    ProcessSerialNumber psn;
    ProcessInfoRec pInfo;
    OSStatus err;
    
    if (ispreview)
        return 8;
        
    setBannerText(0, NULL);

    // Ugly workaround for a problem with the System Preferences app
    // For an unknown reason, when this screensaver is run using the 
    // Test button in the System Prefs Screensaver control panel, the 
    // control panel calls our stopAnimation function as soon as the 
    // science application opens a GLUT window.  This problem does not 
    // occur when the screensaver is run normally (from the screensaver 
    // engine.)  So we just display a message and don't access the core 
    // client.
    GetCurrentProcess(&psn);
    memset(&pInfo, 0, sizeof(pInfo));
    pInfo.processInfoLength = sizeof( ProcessInfoRec );
    pInfo.processName = NULL;
    err = GetProcessInformation(&psn, &pInfo);
    if ( (err == noErr) && (pInfo.processSignature == 'sprf') ) {
        saverState = SaverState_ControlPanelTestMode;
    }

    gBrandId = GetBrandID();
    switch(gBrandId) {
    case 1:
        gBrandText = "GridRepublic";
        gBrandColor = gOrangeTextColor; // Orange
        gTextColor = gGrayTextColor;  // Gray
        break;
    default:
        gBrandText = "BOINC";
        gBrandColor = gWhiteTextColor; // White
        gTextColor = gWhiteTextColor; // White
        break;
    }

    // If there are multiple displays, initBOINCSaver may get called 
    // multiple times (once for each display), so we need to guard 
    // against launching multiple instances of the core client
    if (saverState == SaverState_Idle) {
        SaverStartTime = time(0);
        
        err = initBOINCApp();

        if (saverState == SaverState_LaunchingCoreClient)
        {
            gClientSaverStatus = 0;
            gQuitRPCThread = false;
            if (rpc == NULL)
                rpc = new RPC_CLIENT;
            newFrequency = NOBANNERFREQUENCY;
        }
    }
    return newFrequency;
}


OSStatus initBOINCApp() {
    char boincPath[2048];
    pid_t myPid;
    int status;
    OSStatus err;
    static int retryCount = 0;
    
    saverState = SaverState_CantLaunchCoreClient;
    
    CoreClientPID = FindProcessPID("boinc", 0, 0);
    if (CoreClientPID) {
        wasAlreadyRunning = true;
        saverState = SaverState_LaunchingCoreClient;
        return noErr;
    }
    
    wasAlreadyRunning = false;
    
    if (++retryCount > 3)   // Limit to 3 relaunches to prevent thrashing
        return -1;

    err = GetpathToBOINCManagerApp(boincPath, sizeof(boincPath));
    if (err) {   // If we couldn't find BOINCManager.app, try default path
        strcpy(boincPath, "/Applications/");
        if (gBrandId)
            strcat(boincPath, gBrandText);
        else
            strcat(boincPath, "BOINCManager");
            strcat(boincPath, ".app");
    }
    
    strcat(boincPath, "/Contents/Resources/boinc");

    if ( (myPid = fork()) < 0)
        return -1;
    else if (myPid == 0)			// child
    {
      // We don't customize BOINC Data directory name for branding
#if 0   // Code for separate data in each user's private directory
        char buf[256];
        strcpy(buf, getenv("HOME"));
        strcat(buf, "/Library/Application Support/BOINC Data");
        status = chdir(buf);
#else   // All users share the same data
        status = chdir("/Library/Application Support/BOINC Data");
#endif
        if (status) {
            perror("chdir");
            fflush(NULL);
            _exit(status);
        }

        status = execl(boincPath, boincPath, "-redirectio", "-saver", (char *) 0);
        fflush(NULL);
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
    
    statusUpdateCounter++;

    switch (saverState) {
    case SaverState_RelaunchCoreClient:
        err = initBOINCApp();
        break;
    
    case  SaverState_LaunchingCoreClient:
        if (wasAlreadyRunning)
            setBannerText(ConnectingCCMsg, aPort);
        else
            setBannerText(LaunchingCCMsg, aPort);
       
        myPid = FindProcessPID(NULL, CoreClientPID, 0);
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
            // ToDo: Add a timeout after which we display error message
        } else
            // Take care of the possible race condition where the Core Client was in the  
            // process of shutting down just as ScreenSaver started, so initBOINCApp() 
            // found it already running but now it has shut down.
            if (wasAlreadyRunning)  // If we launched it, then just wait for it to start
                saverState = SaverState_RelaunchCoreClient;
         break;

    case SaverState_CoreClientRunning:
            // RPC called in RPCThread()
            setBannerText(ConnectingCCMsg, aPort);
        break;
    
    case SaverState_CoreClientSetToSaverMode:
        switch (gClientSaverStatus) {
        case 0:
            break;  // No status response yet from RPCThread
        case SS_STATUS_BLANKED:
        default:
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
        case SS_STATUS_ENABLED:
#if ! ALWAYS_DISPLAY_PROGRESS_TEXT
            setBannerText(0, aPort);   // No text message
            // Let the science app draw over our window
            break;
#endif
        case SS_STATUS_NOGRAPHICSAPPSEXECUTING:
            if (msgBuf[0] == 0) {
                strcpy(msgBuf, BOINCNoGraphicAppsExecutingMsg);
                setBannerText(msgBuf, aPort);
            }
            if (gStatusMessageUpdated) {
                updateBannerText(msgBuf, aPort);
                gStatusMessageUpdated = false;
            }
            // Handled in RPCThread()
            break;
#if 0
        case SS_STATUS_QUIT:
//            setBannerText(BOINCExitedSaverMode, aPort);
            // Wait 1 second to allow Give ScreenSaver engine to close us down
            if (++gQuitCounter > (bannerText[0] ? BANNERFREQUENCY : NOBANNERFREQUENCY)) {
                closeBOINCSaver();
                KillScreenSaver(); // Stop the ScreenSaver Engine
            }
            break;
#endif
        }       // end switch (gClientSaverStatus)
        break;

    case SaverState_ControlPanelTestMode:
        setBannerText(BOINCTestmodeMsg, aPort);
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
        newFrequency = BANNERFREQUENCY;
    } else
        newFrequency = NOBANNERFREQUENCY;
    
    return newFrequency;
}


void drawPreview(GrafPtr aPort) {
    SetPort(aPort);
    setBannerText(" BOINC", aPort);
    drawBanner(aPort);
}


// If there are multiple displays, closeBOINCSaver may get called 
// multiple times (once for each display), so we need to guard 
// against any problems that may cause.
void closeBOINCSaver() {
    gQuitRPCThread = true;  // Tell RPC Thread to exit
    if (gTerminationQueue) {  // Wait up to 5 seconds for RPC thread to exit
        MPWaitOnQueue(gTerminationQueue, NULL, NULL, NULL, kDurationMillisecond*5000);
        MPDeleteQueue(gTerminationQueue);
        gTerminationQueue = NULL;
    }
    
    if (rpc) {
#if 0       // OS X calls closeBOINCSaver() when energy saver puts display
            // to sleep, but we want to keep crunching.  So don't kill it.
            // Code in core client now quits on user activity if screen
            // saver launched it (2/28/07).
            // Also, under sandbox security, screensaver doesn't have access 
            // to rpc password in gui_rpc_auth.cfg file, so core client won't 
            // accept rpc->quit from screensaver.
        if (CoreClientPID && (!wasAlreadyRunning)) {
            rpc->quit();    // Kill core client if we launched it
        }
#endif
        delete rpc;
    }

    setBannerText(0, NULL);

    CoreClientPID = 0;
    gQuitCounter = 0;
    wasAlreadyRunning = false;
    gQuitRPCThread = false;
    saverState = SaverState_Idle;
}


OSStatus RPCThread(void* param) {
    int             retval                  = 0;
    CC_STATE        state;
    CC_STATUS       cc_status;
    AbsoluteTime    timeToUnblock;
    time_t          time_to_blank;
    pid_t           graphics_app_pid        = 0;
    pid_t           launcher_shell_pid      = 0;
    char            statusBuf[256];
    unsigned int    len;
    RESULTS         results;
    RESULT*         theResult               = NULL;
    RESULT*         graphics_app_result_ptr = NULL;
    PROJECT*        pProject;
    RESULT          previous_result;
    RESULT*         previous_result_ptr     = NULL;
    int             iResultCount            = 0;
    int             iIndex                  = 0;
    double          percent_done;
    double          launch_time             = 0.0;
    double          last_change_time        = 0.0;
    double          last_run_check_time     = 0.0;

        // Calculate the estimated blank time by adding the starting 
        //  time and and the user-specified time which is in minutes
        if (gGoToBlank)
            time_to_blank = SaverStartTime + (gBlankingTime * 60);
        else
            time_to_blank = 0;
            
    while (true) {
        if ((gGoToBlank) && (time(0) > time_to_blank)) {
            gClientSaverStatus = SS_STATUS_BLANKED;
            gQuitRPCThread = true;
        }

        if (gQuitRPCThread) {     // If main thread has requested we exit
            if (graphics_app_pid) {
                terminate_screensaver(graphics_app_pid);
                graphics_app_pid = 0;
                launcher_shell_pid = 0;
                graphics_app_result_ptr = NULL;
                previous_result_ptr = NULL;
            }
            MPExit(noErr);       // Exit the thread
        }

        timeToUnblock = AddDurationToAbsolute(durationSecond/2, UpTime());
        MPDelayUntil(&timeToUnblock);

        // Try and get the current state of the CC
//Get_CC_State:
        retval = rpc->get_state(state);
        MPYield();
        if (retval == noErr)
            break;

        // CC may not yet be running
        HandleRPCError();
    }

    saverState = SaverState_CoreClientSetToSaverMode;

    while (true) {
        if ((gGoToBlank) && (time(0) > time_to_blank)) {
            gClientSaverStatus = SS_STATUS_BLANKED;
            gQuitRPCThread = true;
        }

        if (gQuitRPCThread) {     // If main thread has requested we exit
            if (graphics_app_pid) {
                terminate_screensaver(graphics_app_pid);
                graphics_app_pid = 0;
                launcher_shell_pid = 0;
                graphics_app_result_ptr = NULL;
                previous_result_ptr = NULL;
            }
            MPExit(noErr);       // Exit the thread
        }
        
        timeToUnblock = AddDurationToAbsolute(durationSecond/2, UpTime());
        MPDelayUntil(&timeToUnblock);

        retval = rpc->get_cc_status(cc_status);
	if (cc_status.task_suspend_reason != 0) {
            gClientSaverStatus = SS_STATUS_BOINCSUSPENDED;
            if (graphics_app_pid) {
                terminate_screensaver(graphics_app_pid);
                // waitpid test will clear graphics_app_pid and graphics_app_result_ptr
                previous_result_ptr = NULL;
            }
            continue;
        }
        
        MPYield();
                
        retval = rpc->get_results(results);
        MPYield();
        if (retval) {
            // rpc call returned error
            HandleRPCError();
            continue;
        }
        
#if SIMULATE_NO_GRAPHICS /* FOR TESTING */

        gClientSaverStatus = SS_STATUS_NOGRAPHICSAPPSEXECUTING;
        
#else                   /* NORMAL OPERATION */

        // Is the current graphics app's associated task still running?
        if ((graphics_app_pid) && (graphics_app_result_ptr)) {

            iResultCount = results.results.size();
            graphics_app_result_ptr = NULL;

            // Find the current task in the new results vector (if it still exists)
            for (iIndex = 0; iIndex < iResultCount; iIndex++) {
                theResult = results.results.at(iIndex);

                if (is_same_task(theResult, previous_result_ptr)) {
                    graphics_app_result_ptr = theResult;
                    previous_result = *theResult;
                    previous_result_ptr = &previous_result;
                    break;
                }
            }

            if ((graphics_app_result_ptr == NULL) || (is_task_active(graphics_app_result_ptr) == false)) {
//                if (previous_result_ptr) print_to_log_file("%s finished", previous_result.name.c_str());
                terminate_screensaver(graphics_app_pid);
                previous_result_ptr = NULL;
                // waitpid test will clear graphics_app_pid and graphics_app_result_ptr
           }
#if 0
            // Safety check that task is actually running
            if (last_run_check_time && ((dtime() - last_run_check_time) > TASK_RUN_CHECK_PERIOD)) {
                if (FindProcessPID(NULL, ???, 0) == 0) {
                    terminate_screensaver(graphics_app_pid);
                    previous_result_ptr = NULL;
                    // waitpid test will clear graphics_app_pid and graphics_app_result_ptr
                }
            }
#endif
            if (last_change_time && ((dtime() - last_change_time) > GFX_CHANGE_PERIOD)) {
                if (count_active_graphic_apps(results, previous_result_ptr) > 0) {
//                    if (previous_result_ptr) print_to_log_file("time to change: %s", previous_result.name.c_str());
                    terminate_screensaver(graphics_app_pid);
                    // Save previous_result and previous_result_ptr for get_random_graphics_app() call
                    // waitpid test will clear graphics_app_pid and graphics_app_result_ptr
                }
                last_change_time = dtime();
            }
        }

        // If no current graphics app, pick an active task at random and launch its graphics app
        if (graphics_app_pid == 0) {
            graphics_app_result_ptr = get_random_graphics_app(results, previous_result_ptr);
            previous_result_ptr = NULL;
            
            if (graphics_app_result_ptr) {
                retval = Mac_launch_screensaver(graphics_app_result_ptr, graphics_app_pid, launcher_shell_pid);
                if (retval) {
                    graphics_app_pid = 0;
                    launcher_shell_pid = 0;
                    previous_result_ptr = NULL;
                    graphics_app_result_ptr = NULL;
                } else {
                    gClientSaverStatus = SS_STATUS_ENABLED;
                    launch_time = dtime();
                    last_change_time = launch_time;
                    last_run_check_time = launch_time;
                    // Make a local copy of current result, since original pointer 
                    // may have been freed by the time we perform later tests
                    previous_result = *graphics_app_result_ptr;
                    previous_result_ptr = &previous_result;
//                    if (previous_result_ptr) print_to_log_file("launching %s", previous_result.name.c_str());                    
                }
            } else {
                if (state.projects.size() == 0) {
                    // We are not attached to any projects
                    gClientSaverStatus = SS_STATUS_NOPROJECTSDETECTED;
                } else if (results.results.size() == 0) {
                    // We currently do not have any applications to run
                    gClientSaverStatus = SS_STATUS_NOAPPSEXECUTING;
                } else {
                    // We currently do not have any graphics capable application
                    gClientSaverStatus = SS_STATUS_NOGRAPHICSAPPSEXECUTING;
                }
            }
        } else {    // End if (graphics_app_pid == 0)
            // Is the graphics app still running?
            if (waitpid(launcher_shell_pid, 0, WNOHANG) == launcher_shell_pid) {
                graphics_app_pid = 0;
                launcher_shell_pid = 0;
                graphics_app_result_ptr = NULL;
                continue;
            }
        }
#endif      // ! SIMULATE_NO_GRAPHICS

        if ((gClientSaverStatus == SS_STATUS_NOGRAPHICSAPPSEXECUTING)
#if ALWAYS_DISPLAY_PROGRESS_TEXT
                || (gClientSaverStatus == SS_STATUS_ENABLED)
#endif
            )
 {
            if (statusUpdateCounter >= (STATUSUPDATEINTERVAL * BANNERFREQUENCY) ) {
                statusUpdateCounter = 0;

                if (! gStatusMessageUpdated) {
                    strcpy(msgBuf, BOINCNoGraphicAppsExecutingMsg);
        
                        iResultCount = results.results.size();

                        for (iIndex = 0; iIndex < iResultCount; iIndex++) {
                            theResult = results.results.at(iIndex);
                            if (! is_task_active(theResult)) continue;

                            // The get_state rpc is time-consuming, so we assume the list of 
                            // attached projects does not change while the screensaver is active.
                            pProject = state.lookup_project(theResult->project_url);
                            if (pProject != NULL) {
                                percent_done = theResult->fraction_done * 100;
                                if (percent_done < 0.01)
                                    len = sprintf(statusBuf, ("    %s"), pProject->project_name.c_str());
                                 else    // Display percent_done only after we have a valid value
                                   len = sprintf(statusBuf, ("    %s: %.2f%%"), 
                                        pProject->project_name.c_str(), percent_done);

                                strlcat(msgBuf, statusBuf, sizeof(msgBuf));
                            }       // end if (pProject != NULL)
//                            else {          // (pProject += NULL): re-synch with client
//                                goto Get_CC_State;  // Should never happen
//                            }
                        }           // end for() loop
                        gStatusMessageUpdated = true;
                    
                }   // end if (! gStatusMessageUpdated)
                
            }                   // end if (statusUpdateCounter > time to update)
        }                       // end if SS_STATUS_NOGRAPHICSAPPSEXECUTING
    }                           // end while(true)
            return noErr;       // should never get here; it fixes compiler warning
}


void HandleRPCError() {
    // Attempt to restart BOINC Client if needed, reinitialize the RPC client and state
    rpc->close();
    
    // There is a possible race condition where the Core Client was in the  
    // process of shutting down just as ScreenSaver started, so initBOINCApp() 
    // found it already running but now it has shut down.  This code takes 
    // care of that and other situations where the Core Client quits unexpectedy.  
    if (FindProcessPID("boinc", 0, 0) == 0) {
        saverState = SaverState_RelaunchCoreClient;
        MPExit(noErr);      // Exit the thread
     }
    
    rpc->init(NULL);    // Otherwise just reinitialize the RPC client and state and keep trying
    // Error message after timeout?
}


void setBannerText(const char * msg, GrafPtr aPort) {
    if (msg == 0)
        bannerText[0] = 0;
    
    if ((char *)CurrentBannerMessage != msg)
        updateBannerText((char *)msg, aPort);
}


void updateBannerText(char *msg, GrafPtr aPort) {
    CGrafPtr savePort;
    RGBColor saveBackColor;
    Rect wRect;
    char *p, *s;
    
    CurrentBannerMessage = (StringPtr)msg;

    if (aPort == NULL)
        return;
        
    GetPort(&savePort);
    SetPort(aPort);

    GetPortBounds(aPort, &wRect);

    GetBackColor(&saveBackColor);
    BackColor ( blackColor );
    EraseRect(&wRect);
    RGBBackColor(&saveBackColor);
   
   if (msg) {
        TextSize(24);
        TextFace(bold);
        s = msg;
        bannerText[0] = '\0';
        do {
            p = strstr(s, "BOINC");
            if (p == NULL) {
                strcat(bannerText, s);
            } else {
                strncat(bannerText, s, p - s);
                strcat(bannerText, gBrandText);
                s = p + 5;  // s = p + strlen("BOINC");
            }
        } while (p);
        
        bannerWidth = TextWidth(bannerText, 0, strlen(bannerText)) + BANNER_GAP;
        // Round up bannerWidth to an integral multiple of BANNERDELTA
        bannerWidth = ((bannerWidth + BANNERDELTA - 1) / BANNERDELTA) * BANNERDELTA;
    }
    
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
    char *p, *s;
    
    if (aPort == NULL)
        return;
        
    GetGWorld(&savePort, &saveGDH);
    SetPort(aPort);
    
    GetForeColor(&saveForeColor);
    GetBackColor(&saveBackColor);
    RGBForeColor(&gTextColor);
    BackColor(blackColor);
    GetPortBounds(aPort, &wRect);
    if ( (bannerPos + bannerWidth) <= (wRect.left + BANNERDELTA) )
        bannerPos = wRect.left;
    else
        bannerPos -= BANNERDELTA;

   x = bannerPos;
   y = (wRect.bottom - wRect.top) / 3 + wRect.top;
   
    GetFontInfo(&fInfo);
    wRect.top = y - fInfo.ascent;
    wRect.bottom = y + fInfo.descent;
    EraseRect(&wRect);
       
    do { 
        MoveTo(x, y);
        s = bannerText;
        do {
            p = strstr(s, gBrandText);
            if (p == NULL) {
                DrawText(s, 0, strlen(s));
            } else {
                DrawText(s, 0, p - s);
                RGBForeColor(&gBrandColor);
                DrawText(gBrandText, 0, strlen(gBrandText));
                s = p + strlen(gBrandText);
                RGBForeColor(&gTextColor);
            }
        } while (p);
        
        x+= bannerWidth;
    } while (x < wRect.right);
    
    RGBForeColor(&saveForeColor);
    RGBBackColor(&saveBackColor);

    SetGWorld(savePort, saveGDH);
}


int GetBrandID()
{
    char buf[1024];
    long iBrandId;
    OSErr err;

    iBrandId = 0;   // Default value
    
    err = GetpathToBOINCManagerApp(buf, sizeof(buf));
    if (err) {     
        // If we couldn't find our application bundle, look in BOINC Data Directory 
        // (the installer put a copy there for us)
        strcpy(buf, "/Library/Application Support/BOINC Data/Branding");
    } else
        strcat(buf, "/Contents/Resources/Branding");

    FILE *f = fopen(buf, "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandId);
        fclose(f);
    }
    
    return iBrandId;
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


pid_t FindProcessPID(char* name, pid_t thePID, pid_t thePPID)
{
    FILE *f;
    char buf[1024];
    size_t n = 0;
    pid_t aPID, aPPID;
    
    if (name != NULL) {     // Search by name
        n = strlen(name);
    }
    
    f = popen("ps -a -x -c -o pid,ppid,command", "r");
    if (f == NULL)
        return 0;
    
    while (PersistentFGets(buf, sizeof(buf), f))
    {
        if (name != NULL) {     // Search by name
            if (strncmp(buf+12, name, n) == 0)
            {
                aPID = atol(buf);
                pclose(f);
                return aPID;
            }
        } else 
        if (thePID != 0) {      // Search by PID
            aPID = atol(buf);
            if (aPID == thePID) {
                pclose(f);
                return aPID;
            }
        } else
            if (thePPID != 0) {      // Search by PPID
            aPPID = atol(buf+6);
            if (aPPID == thePPID) {
                aPID = atol(buf);
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


// Send a Quit AppleEvent to the process which called this module
// (i.e., tell the ScreenSaver engine to quit)
OSErr KillScreenSaver() {
    ProcessSerialNumber         thisPSN;
    pid_t                       thisPID;
    OSErr                       err = noErr;

    GetCurrentProcess(&thisPSN);
    err = GetProcessPID(&thisPSN , &thisPID);
    if (err == noErr)
        err = kill(thisPID, SIGABRT);   // SIGINT
    return err;
}


// Launch the graphics application
//
int Mac_launch_screensaver(RESULT* rp, pid_t& graphics_application, pid_t& launcher_shell)
{
    char cmd[1024];
    int retval;
    int pid = fork();
    if (pid == 0) {
        retval = chdir(rp->slot_path.c_str());
        if (retval) return retval;
        // Launching the graphics application using fork() and execv() 
        // results in it getting "RegisterProcess failed (error = -50)"
        // so we launch it via a shell using the system() api.
        sprintf(cmd, "\"%s\" --fullscreen", rp->graphics_exec_path.c_str());
        system(cmd);
        exit(errno);
    }
    
    // Remember our child's pid: our child is the shell launched by system()
    launcher_shell = pid;
    // Find the pid of the graphics application (our grandchild)
    graphics_application = FindProcessPID(NULL, 0, pid);
    return 0;
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
const char *BOINC_RCSID_7ce0778d35="$Id$";

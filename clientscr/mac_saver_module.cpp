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

//
//  mac_saver_module.cpp
//  BOINC_Saver_Module
//

#include <IOKit/IOKitLib.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#ifdef __cplusplus
}    // extern "C"
#endif

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/param.h>  // for MAXPATHLEN

#include "gui_rpc_client.h"
#include "common_defs.h"
#include "util.h"
#include "Mac_Saver_Module.h"
#include "screensaver.h"
#include "diagnostics.h"
#include "str_replace.h"

//#include <drivers/event_status_driver.h>

// Flags for testing & debugging
#define CREATE_LOG 0
#define USE_SPECIAL_LOG_FILE 1

#define TEXTLOGOFREQUENCY 60 /* Number of times per second to update moving logo with text */
#define NOTEXTLOGOFREQUENCY 4 /* Times per second to call animateOneFrame if no moving logo with text */
#define GFX_STARTING_MSG_DURATION 45 /* seconds to show ScreenSaverAppStartingMsg */
#define RPC_RETRY_INTERVAL 10 /* # of seconds between retries connecting to core client */
#define BATTERY_CHECK_INTERVAL 3 /* # of seconds between checks of whether running on batteries */

enum SaverState {
    SaverState_Idle,
    SaverState_LaunchingCoreClient,
    SaverState_CoreClientRunning,
    SaverState_RelaunchCoreClient,
    SaverState_ConnectedToCoreClient,
    
    SaverState_CantLaunchCoreClient,
    SaverState_ControlPanelTestMode,
    SaverState_UnrecoverableError
};


static CScreensaver* gspScreensaver = NULL;

extern int gGoToBlank;      // True if we are to blank the screen
extern int gBlankingTime;   // Delay in minutes before blanking the screen
extern CFStringRef gPathToBundleResources;

static SaverState saverState = SaverState_Idle;
// int gQuitCounter = 0;

static bool IsDualGPUMacbook = false;
static io_connect_t GPUSelectConnect = IO_OBJECT_NULL;
static bool OKToRunOnBatteries = false;
static bool RunningOnBattery = true;
static time_t ScreenSaverStartTime = 0;
static bool ScreenIsBlanked = false;
static int retryCount = 0;

const char *  CantLaunchCCMsg = "Unable to launch BOINC application.";
const char *  LaunchingCCMsg = "Launching BOINC application.";
const char *  ConnectingCCMsg = "Connecting to BOINC application.";
const char *  ConnectedCCMsg = "Communicating with BOINC application.";
const char *  BOINCUnrecoverableErrorMsg = "Sorry, an unrecoverable error occurred";
const char *  BOINCTestModeMsg = "BOINC screensaver test: success.";
const char *  ScreenSaverAppStartingMsg = "Starting screensaver graphics.\nPlease wait ...";
const char *  CantLaunchDefaultGFXAppMsg = "Can't launch default screensaver module. Please reinstall BOINC";
const char *  DefaultGFXAppCantRPCMsg = "Default screensaver module couldn't connect to BOINC application";
const char *  DefaultGFXAppCrashedMsg = "Default screensaver module had an unrecoverable error";
const char *  RunningOnBatteryMsg = "Computing and screensaver disabled while running on battery power.";

//const char *  BOINCExitedSaverMode = "BOINC is no longer in screensaver mode.";


// If there are multiple displays, this may get called 
// multiple times (once for each display), so we need to guard 
// against any problems that may cause.
void initBOINCSaver() {
    diagnostics_init(
        BOINC_DIAG_PERUSERLOGFILES |
        BOINC_DIAG_REDIRECTSTDOUT |
        BOINC_DIAG_REDIRECTSTDERR |
        BOINC_DIAG_TRACETOSTDOUT,
        "stdoutscr", "stderrscr"
        );

    if (gspScreensaver == NULL) {
        gspScreensaver = new CScreensaver();
    }
}


int startBOINCSaver() {
    if (gspScreensaver) {
        return gspScreensaver->Create();
    }
    return TEXTLOGOFREQUENCY;
}


int getSSMessage(char **theMessage, int* coveredFreq) {
    if (gspScreensaver) {
        return gspScreensaver->getSSMessage(theMessage, coveredFreq);
    } else {
        *theMessage = "";
        *coveredFreq = 0;
        return NOTEXTLOGOFREQUENCY;
    }
};


void windowIsCovered() {
    if (gspScreensaver) {
        gspScreensaver->windowIsCovered();
    }
}


void drawPreview(CGContextRef myContext) {
    if (gspScreensaver) {
        gspScreensaver->drawPreview(myContext);
    }
};


// If there are multiple displays, this may get called 
// multiple times (once for each display), so we need to guard 
// against any problems that may cause.
void closeBOINCSaver() {
    if (gspScreensaver) {
        gspScreensaver->ShutdownSaver();
        delete gspScreensaver;
        gspScreensaver = NULL;
    }
}


double getGFXDefaultPeriod() {
    if (gspScreensaver) {
        return gspScreensaver->m_fGFXDefaultPeriod;
    }
    return 0;
}


double getGFXSciencePeriod() {
    if (gspScreensaver) {
        return gspScreensaver->m_fGFXSciencePeriod;
    }
    return 0;
}


double getGGFXChangePeriod() {
    if (gspScreensaver) {
        return gspScreensaver->m_fGFXChangePeriod;
    }
    return 0;
}


void setGFXDefaultPeriod(double value) {
    if (gspScreensaver) {
        gspScreensaver->m_fGFXDefaultPeriod = value;
    }
}


void setGFXSciencePeriod(double value) {
    if (gspScreensaver) {
        gspScreensaver->m_fGFXSciencePeriod = value;
    }
}


void setGGFXChangePeriod(double value) {
    if (gspScreensaver) {
        gspScreensaver->m_fGFXChangePeriod = value;
    }
}


double getDTime() {
    return dtime();
}


CScreensaver::CScreensaver() {
    struct ss_periods periods;
    
    m_dwBlankScreen = 0;
    m_dwBlankTime = 0;
    m_bErrorMode = false;
    m_hrError = 0;
    // Display first status update after 5 seconds
    m_iGraphicsStartingMsgCounter = 0;
    saverState = SaverState_Idle;
    m_wasAlreadyRunning = false;
    m_CoreClientPID = nil;
    setSSMessageText(0);
    m_CurrentBannerMessage = 0;
    m_bQuitDataManagementProc = false;
    m_bDataManagementProcStopped = false;
    m_BrandText = "BOINC";
    
    m_hDataManagementThread = NULL;
    m_hGraphicsApplication = NULL;
    m_bResetCoreState = true;
    rpc = 0;
    m_bConnected = false;
    
    // Get project-defined default values for GFXDefaultPeriod, GFXSciencePeriod, GFXChangePeriod
    GetDefaultDisplayPeriods(periods);
    m_bShow_default_ss_first = periods.Show_default_ss_first;


    m_fGFXDefaultPeriod = periods.GFXDefaultPeriod;
    m_fGFXSciencePeriod = periods.GFXSciencePeriod;
    m_fGFXChangePeriod = periods.GFXChangePeriod;
}


int CScreensaver::Create() {
    ProcessSerialNumber psn;
    ProcessInfoRec pInfo;
    OSStatus err;
    
    // Ugly workaround for a problem with the System Preferences app
    // For an unknown reason, when this screensaver is run using the 
    // Test button in the System Prefs Screensaver control panel, the 
    // control panel calls our stopAnimation function as soon as the 
    // science application opens a GLUT window.  This problem does not 
    // occur when the screensaver is run normally (from the screensaver 
    // engine.)  So we just display a message and don't access the core 
    // client.
    // With V6 graphics when using gfx_switcher, the graphics application 
    // fails to run and stderr shows the message: 
    // "The process has forked and you cannot use this CoreFoundation 
    // functionality safely. You MUST exec()" 
    GetCurrentProcess(&psn);
    memset(&pInfo, 0, sizeof(pInfo));
    pInfo.processInfoLength = sizeof( ProcessInfoRec );
    pInfo.processName = NULL;
    err = GetProcessInformation(&psn, &pInfo);
    if ( (err == noErr) && (pInfo.processSignature == 'sprf') ) {
        saverState = SaverState_ControlPanelTestMode;
    }

    // Calculate the estimated blank time by adding the starting
    //  time and and the user-specified time which is in minutes
    // On dual-GPU Macbok Pros, the CScreensaver class will be
    // constructed and destructed each time we switch beteen
    // battery and AC power, so we need to get the starting time
    // only once.
    if (!ScreenSaverStartTime) {
        ScreenSaverStartTime = time(0);
    }
    
    m_dwBlankScreen = gGoToBlank;
    if (gGoToBlank && (gBlankingTime > 0))
        m_dwBlankTime = ScreenSaverStartTime + (gBlankingTime * 60);
    else
        m_dwBlankTime = 0;
    
    // If there are multiple displays, initBOINCSaver may get called 
    // multiple times (once for each display), so we need to guard 
    // against launching multiple instances of the core client
    if (saverState == SaverState_Idle) {
        CFStringGetCString(gPathToBundleResources, m_gfx_Switcher_Path, sizeof(m_gfx_Switcher_Path), kCFStringEncodingMacRoman);
        strlcat(m_gfx_Switcher_Path, "/gfx_switcher", sizeof(m_gfx_Switcher_Path));

        err = initBOINCApp();

        CGDisplayHideCursor(kCGNullDirectDisplay);
    
        if (saverState == SaverState_LaunchingCoreClient)
        {
            SetError(FALSE, 0);
            m_bQuitDataManagementProc = false;
            m_bDataManagementProcStopped = false;
            if (rpc == NULL) {
                rpc = new RPC_CLIENT;
            }
        }
    }
    
    if (!IsDualGPUMacbook) {
        SetDiscreteGPU(false);
        if (IsDualGPUMacbook && (GPUSelectConnect != IO_OBJECT_NULL)) {
            IOServiceClose(GPUSelectConnect);
            GPUSelectConnect = IO_OBJECT_NULL;
        }
    }
    
    return TEXTLOGOFREQUENCY;
}


OSStatus CScreensaver::initBOINCApp() {
    char boincPath[2048];
    pid_t myPid;
    int status;
    OSStatus err;
    long brandId = 0;

    saverState = SaverState_CantLaunchCoreClient;
    
    brandId = GetBrandID();
    switch(brandId) {
    case 1:
        m_BrandText = "GridRepublic Desktop";
         break;
    case 2:
        m_BrandText = "Progress Thru Processors Desktop";
         break;
    case 3:
        m_BrandText = "Charity Engine Desktop";
         break;
    default:
        m_BrandText = "BOINC";
        break;
    }

    m_CoreClientPID = FindProcessPID("boinc", 0);
    if (m_CoreClientPID) {
        m_wasAlreadyRunning = true;
        saverState = SaverState_LaunchingCoreClient;
        retryCount = 0;
        return noErr;
    }
    
    m_wasAlreadyRunning = false;
    
    if (++retryCount > 3)   // Limit to 3 relaunches to prevent thrashing
        return -1;

#ifdef _DEBUG
    err = -1;
#else
    err = GetpathToBOINCManagerApp(boincPath, sizeof(boincPath));
#endif
   if (err) 
    {   // If we couldn't find BOINCManager.app, try default path
        strcpy(boincPath, "/Applications/");
        if (brandId)
            strcat(boincPath, m_BrandText);
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
        safe_strcpy(buf, getenv("HOME"));
        safe_strcat(buf, "/Library/Application Support/BOINC Data");
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
        m_CoreClientPID = myPid;		// make this available globally
        saverState = SaverState_LaunchingCoreClient;
    }

    return noErr;
}


// Returns new desired Animation Frequency (per second) or 0 for no change
int CScreensaver::getSSMessage(char **theMessage, int* coveredFreq) {
    int newFrequency = TEXTLOGOFREQUENCY;
    *coveredFreq = 0;
    pid_t myPid;
    CC_STATE ccstate;
    OSStatus err;
    
    if (ScreenIsBlanked) {
        setSSMessageText(0);   // No text message
        *theMessage = m_MessageText;
        return NOTEXTLOGOFREQUENCY;
    }
    
    CheckDualGPUStatus();
    
    switch (saverState) {
    case SaverState_RelaunchCoreClient:
        err = initBOINCApp();
        break;
    
    case  SaverState_LaunchingCoreClient:
        if (m_wasAlreadyRunning) {
            setSSMessageText(ConnectingCCMsg);
        } else {
            setSSMessageText(LaunchingCCMsg);
        }
            
        myPid = FindProcessPID(NULL, m_CoreClientPID);
        if (myPid) {
            saverState = SaverState_CoreClientRunning;
            if (!rpc->init(NULL)) {     // Initialize communications with Core Client
                m_bConnected = true;
                if (IsDualGPUMacbook) {
                    ccstate.clear();
                    ccstate.global_prefs.init_bools();
                    int result = rpc->get_state(ccstate);
                    if (!result) {
                        OKToRunOnBatteries = ccstate.global_prefs.run_on_batteries;
                    } else {
                        OKToRunOnBatteries = false;
                    }
                    
                    if (OKToRunOnBatteries) {
                        SetDiscreteGPU(true);
                    }
                }
            }

            // Set up a separate thread for communicating with Core Client
            // and running screensaver graphics
            CreateDataManagementThread();
            // ToDo: Add a timeout after which we display error message
        } else
            // Take care of the possible race condition where the Core Client was in the  
            // process of shutting down just as ScreenSaver started, so initBOINCApp() 
            // found it already running but now it has shut down.
            if (m_wasAlreadyRunning) { // If we launched it, then just wait for it to start
                saverState = SaverState_RelaunchCoreClient;
            }
         break;

    case SaverState_CoreClientRunning:
        if (IsDualGPUMacbook && RunningOnBattery && !OKToRunOnBatteries) {
            setSSMessageText(RunningOnBatteryMsg);
            break;
        }
            
        // RPC called in DataManagementProc()
        setSSMessageText(ConnectingCCMsg);
        
        if (! m_bResetCoreState) {
            saverState = SaverState_ConnectedToCoreClient;
        }
    break;
    
    case SaverState_ConnectedToCoreClient:
        if (IsDualGPUMacbook && RunningOnBattery && !OKToRunOnBatteries) {
            setSSMessageText(RunningOnBatteryMsg);
            break;
        }

        switch (m_hrError) {
        case 0:
            setSSMessageText(ConnectedCCMsg);
            break;  // No status response yet from DataManagementProc
        case SCRAPPERR_SCREENSAVERBLANKED:
            setSSMessageText(0);   // No text message
            ScreenIsBlanked = true;
            if (IsDualGPUMacbook && (GPUSelectConnect != IO_OBJECT_NULL)) {
                IOServiceClose(GPUSelectConnect);
                GPUSelectConnect = IO_OBJECT_NULL;
            }
            break;
#if 0   // Not currently used
        case SCRAPPERR_QUITSCREENSAVERREQUESTED:
//            setSSMessageText(BOINCExitedSaverMode);
            // Wait 1 second to allow ScreenSaver engine to close us down
            if (++gQuitCounter > (m_MessageText[0] ? TEXTLOGOFREQUENCY : NOTEXTLOGOFREQUENCY)) {
                closeBOINCSaver();
                KillScreenSaver(); // Stop the ScreenSaver Engine
            }
            break;
#endif
        case SCRAPPERR_CANTLAUNCHDEFAULTGFXAPP:
            setSSMessageText(CantLaunchDefaultGFXAppMsg);
            break;
        case SCRAPPERR_DEFAULTGFXAPPCANTCONNECT:
            setSSMessageText(DefaultGFXAppCantRPCMsg);
            break;
         case SCRAPPERR_DEFAULTGFXAPPCRASHED:
            setSSMessageText(DefaultGFXAppCrashedMsg);
            break;
        default:
            // m_bErrorMode is TRUE if we should display moving logo (no graphics app is running)
            // m_bErrorMode is FALSE if a graphics app was launched and has not exit
            if (! m_bErrorMode) { 
                // NOTE: My tests seem to confirm that the top window is always the first 
                // window returned by NSWindowList under OS 10.5 and the second window 
                // returned by NSWindowList under OS 10.3.9 and OS 10.4.  However, Apple's 
                // documentation is unclear whether we can depend on this.  So I have 
                // added some safety by doing two things:
                // [1] Only use the NSWindowList test when we have started project or default 
                //      graphics.
                // [2] Assume that our window is covered 45 seconds after starting project 
                //     graphics even if the NSWindowList test did not indicate that is so.
                //
                // The -animateOneFrame method in Mac_SaverModuleView.m does the NSWindowList test 
                // only if we return a non-zero value for coveredFreq.
                //
                // Tell the calling routine to set the frame rate to NOTEXTLOGOFREQUENCY if 
                // NSWindowList indicates that science app graphics window has covered our window.
                *coveredFreq = NOTEXTLOGOFREQUENCY;
                
                if (m_iGraphicsStartingMsgCounter > 0) {
                    // Show ScreenSaverAppStartingMsg for GFX_STARTING_MSG_DURATION seconds or until 
                    // NSWindowList indicates that science app graphics window has covered our window
                    setSSMessageText(ScreenSaverAppStartingMsg);
                    m_iGraphicsStartingMsgCounter--;
                } else {
                    // Don't waste CPU cycles when the science app is drawing over our window
                    setSSMessageText(0);   // No text message
                }
            }           // End if (! m_bErrorMode)
            break;      // End default case of switch (m_hrError)
            
        }       // end switch (m_hrError)
        break;  // End case SaverState_ConnectedToCoreClient of switch (saverState)

    case SaverState_ControlPanelTestMode:
        setSSMessageText(BOINCTestModeMsg);
        break;

    case SaverState_UnrecoverableError:
        setSSMessageText(BOINCUnrecoverableErrorMsg);
        break;

    case SaverState_CantLaunchCoreClient:
        if (IsDualGPUMacbook && RunningOnBattery && !OKToRunOnBatteries) {
            setSSMessageText(RunningOnBatteryMsg);
            break;
        }
        
        setSSMessageText(CantLaunchCCMsg);
        
        // Set up a separate thread for running screensaver graphics 
        // even if we can't communicate with core client
        CreateDataManagementThread();
        break;

    case SaverState_Idle:
        break;      // Should never get here; fixes compiler warning
    }           // end switch (saverState)

    if (IsDualGPUMacbook && RunningOnBattery && !OKToRunOnBatteries) {
        if ((m_dwBlankScreen) && (time(0) > m_dwBlankTime) && (m_dwBlankTime > 0)) {
            setSSMessageText(0);   // No text message
            ScreenIsBlanked = true;
        }
    }
    
    if (m_MessageText[0]) {
        newFrequency = TEXTLOGOFREQUENCY;
    } else {
        newFrequency = NOTEXTLOGOFREQUENCY;
    }
    
    *theMessage = m_MessageText;
    return newFrequency;
}


void CScreensaver::windowIsCovered() {
    m_iGraphicsStartingMsgCounter = 0;
}


void CScreensaver::drawPreview(CGContextRef myContext) {
    // For possible future use
}


void CScreensaver::ShutdownSaver() {
    DestroyDataManagementThread();
    
    if (rpc) {
#if 0       // OS X calls closeBOINCSaver() when energy saver puts display
            // to sleep, but we want to keep crunching.  So don't kill it.
            // Code in core client now quits on user activity if screen
            // saver launched it (2/28/07).
            // Also, under sandbox security, screensaver doesn't have access 
            // to rpc password in gui_rpc_auth.cfg file, so core client won't 
            // accept rpc->quit from screensaver.
        if (m_CoreClientPID && (!m_wasAlreadyRunning)) {
            rpc->quit();    // Kill core client if we launched it
        }
#endif
        delete rpc;
        rpc = NULL;
    }

    setSSMessageText(0);

    m_CoreClientPID = 0;
//    gQuitCounter = 0;
    m_wasAlreadyRunning = false;
    m_bQuitDataManagementProc = false;
    saverState = SaverState_Idle;
    retryCount = 0;
}


// This function forwards to DataManagementProc, which has access to the
//       "this" pointer.
//
void * CScreensaver::DataManagementProcStub(void* param) {
    return gspScreensaver->DataManagementProc();
}


void CScreensaver::HandleRPCError() {
    static time_t last_RPC_retry = 0;
    time_t now = time(0);
   
    // Attempt to restart BOINC Client if needed, reinitialize the RPC client and state
    rpc->close();
    m_bConnected = false;
    
    if (saverState == SaverState_CantLaunchCoreClient) {
        if ((now - last_RPC_retry) < RPC_RETRY_INTERVAL) {
            return;
        }
        last_RPC_retry = now;
    } else {
        // There is a possible race condition where the Core Client was in the  
        // process of shutting down just as ScreenSaver started, so initBOINCApp() 
        // found it already running but now it has shut down.  This code takes 
        // care of that and other situations where the Core Client quits unexpectedy.  
        // Code in initBOINC_App() limits # launch retries to 3 to prevent thrashing.
        if (FindProcessPID("boinc", 0) == 0) {
            saverState = SaverState_RelaunchCoreClient;
            m_bResetCoreState = true;
         }
    }
    
    // If Core Client is hung, it might cause RPCs to hang, preventing us from 
    // shutting down the Data Management Thread, so don't reinitialize the RPC 
    // client if we have told the Data Management Thread to exit.
    if (m_bQuitDataManagementProc) {
        return;
    }
    
    // Otherwise just reinitialize the RPC client and state and keep trying
    if (!rpc->init(NULL)) {
        m_bConnected = true;
    }
    // Error message after timeout?
}

bool CScreensaver::CreateDataManagementThread() {
    int retval;
    
    // On dual-GPU Macbook Pros, our OpenGL scrensaver
    // applications trigger a switch to the power-hungry 
    // discrete GPU. To extend battery life, don't run
    // them when on battery power.
    if (IsDualGPUMacbook && RunningOnBattery && !OKToRunOnBatteries) return true;
    
    if (m_hDataManagementThread == NULL) {
        retval = pthread_create(&m_hDataManagementThread, NULL, DataManagementProcStub, 0);
        if (retval) {
            saverState = SaverState_UnrecoverableError;
            return false;
        }
        pthread_detach(m_hDataManagementThread);
    }
    return true;
}


bool CScreensaver::DestroyDataManagementThread() {
    m_bQuitDataManagementProc = true;  // Tell DataManagementProc thread to exit
    if (!m_hDataManagementThread) return true;
    
    for (int i=0; i<10; i++) {  // Wait up to 1 second for DataManagementProc thread to exit
        if (m_bDataManagementProcStopped) return true;
        boinc_sleep(0.1);
    }

    if (rpc) {
        rpc->close();    // In case DataManagementProc is hung waiting for RPC
    }
    m_hDataManagementThread = NULL; // Don't delay more if this routine is called again.
    if (m_hGraphicsApplication) {
        terminate_screensaver(m_hGraphicsApplication, NULL);
        m_hGraphicsApplication = 0;
    }

    return true;
}


//
bool CScreensaver::SetError(bool bErrorMode, unsigned int hrError) {
    // bErrorMode is TRUE if we should display moving logo (no graphics app is running)
    // bErrorMode is FALSE if a graphics app was launched and has not exit
    m_bErrorMode = bErrorMode;
    m_hrError = hrError;

    if (bErrorMode) {
        // Reset our timer for showing ScreenSaverAppStartingMsg to 
        // GFX_STARTING_MSG_DURATION seconds
        m_iGraphicsStartingMsgCounter = GFX_STARTING_MSG_DURATION * TEXTLOGOFREQUENCY;
    }
    return true;
}

void CScreensaver::setSSMessageText(const char * msg) {
    if (msg == 0)
        m_MessageText[0] = 0;
    
    if (m_CurrentBannerMessage != msg)
        updateSSMessageText((char *)msg);
}


void CScreensaver::updateSSMessageText(char *msg) {
    char *p, *s;
    
    m_CurrentBannerMessage = msg;

   if (msg) {
        s = msg;
        m_MessageText[0] = '\0';
        do {
            p = strstr(s, "BOINC");
            if (p == NULL) {
                strcat(m_MessageText, s);
            } else {
                strncat(m_MessageText, s, p - s);
                strcat(m_MessageText, m_BrandText);
                s = p + 5;  // s = p + strlen("BOINC");
            }
        } while (p);
        
    }
}


int CScreensaver::GetBrandID()
{
    char buf[1024];
    long iBrandId;
    OSErr err;

    iBrandId = 0;   // Default value
    
    // The installer put a copy of Branding file in the BOINC Data Directory
    FILE *f = fopen("/Library/Application Support/BOINC Data/Branding", "r");
    if (f == NULL) {
       // If we couldn't find our Branding file in the BOINC Data Directory,  
       // look in our application bundle
        err = GetpathToBOINCManagerApp(buf, sizeof(buf));
        if (err == noErr) {
            strcat(buf, "/Contents/Resources/Branding");
            f = fopen(buf, "r");
        }
    }
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandId);
        fclose(f);
    }
        
    return iBrandId;
}


char * CScreensaver::PersistentFGets(char *buf, size_t buflen, FILE *f) {
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


pid_t CScreensaver::FindProcessPID(char* name, pid_t thePID)
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


OSErr CScreensaver::GetpathToBOINCManagerApp(char* path, int maxLen)
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
OSErr CScreensaver::KillScreenSaver() {
    ProcessSerialNumber         thisPSN;
    pid_t                       thisPID;
    OSErr                       err = noErr;

    GetCurrentProcess(&thisPSN);
    err = GetProcessPID(&thisPSN , &thisPID);
    if (err == noErr)
        err = kill(thisPID, SIGABRT);   // SIGINT
    return err;
}


bool CScreensaver::Host_is_running_on_batteries() {
    CFDictionaryRef pSource = NULL;
    CFStringRef psState;
    int i;
    bool retval = false;

    CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    CFArrayRef list = IOPSCopyPowerSourcesList(blob);

    for (i=0; i<CFArrayGetCount(list); i++) {
        pSource = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(list, i));
        if(!pSource) break;
        psState = (CFStringRef)CFDictionaryGetValue(pSource, CFSTR(kIOPSPowerSourceStateKey));
        if(!CFStringCompare(psState,CFSTR(kIOPSBatteryPowerValue),0))
        retval = true;
    }

    CFRelease(blob);
    CFRelease(list);
        
    return retval;
}


// On Dual-GPU Macbook Pros, Apple's Screensaver Engine 
// will detect any GPU change and call stopAnimation,
// then initWithFrame and startAnimation.
//
// When we launch boincscr or a project screensaver
// app which uses OpenGL, that will trigger a switch to
// the discrete GPU, causing the Screensaver Engine to
// call stopAnimation, which will then shut down boincscr
// or the project screensaver.  This will then release
// the discrete GPU, triggering a switch to the intrinsic
// GPU, which will again cause a call to stopAnimation,
// and so forth in an infinite loop.
//
// The solution is to request the discrete GPU ourselves
// before launching boincscr or a project screensaver so
// the OpenGL app does not cause a GPU switch.
//
// We initially call this with setDiscrete = false to
// test whether we are running on a Dual-GPU Macbook Pro.
//
void CScreensaver::SetDiscreteGPU(bool setDiscrete) {
    kern_return_t kernResult = 0;
    io_service_t service = IO_OBJECT_NULL;

    if (GPUSelectConnect == IO_OBJECT_NULL) {
        service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleGraphicsControl"));
        if (service != IO_OBJECT_NULL) {
            kernResult = IOServiceOpen(service, mach_task_self(), setDiscrete ? 1 : 0, &GPUSelectConnect);
            if (kernResult == KERN_SUCCESS) {
                IsDualGPUMacbook = true;
            }
        }
    }
}


// On Dual-GPU Macbook Pros only:
// switch to intrinsic GPU if running on battery
// switch to discrete GPU if running on AC power
//
// Apple's Screensaver Engine will detect the GPU change and
// call stopAnimation, then initWithFrame and startAnimation.
void CScreensaver::CheckDualGPUStatus() {
    static double lastBatteryCheckTime = 0;
    double currentTime;
    bool nowOnBattery;
    
    if (!IsDualGPUMacbook) return;
    if (OKToRunOnBatteries) return;
    
    currentTime = dtime();
    if (currentTime < lastBatteryCheckTime + BATTERY_CHECK_INTERVAL) return;
    lastBatteryCheckTime = currentTime;
    
    nowOnBattery = Host_is_running_on_batteries();
    if (nowOnBattery == RunningOnBattery) return;
    
    RunningOnBattery = nowOnBattery;
    if (nowOnBattery) {
        if (GPUSelectConnect != IO_OBJECT_NULL) {
            IOServiceClose(GPUSelectConnect);
            GPUSelectConnect = IO_OBJECT_NULL;
        }
        // If an OpenGL screensaver app is running, we must shut it down
        // to release its claim on the discrete GPU to save battery power.
        DestroyDataManagementThread();
    } else {
        SetDiscreteGPU(true);
    }
}


void print_to_log_file(const char *format, ...) {
#if CREATE_LOG
    va_list args;
    char buf[256];
    time_t t;
#if USE_SPECIAL_LOG_FILE
    safe_strcpy(buf, getenv("HOME"));
    safe_strcat(buf, "/Documents/test_log.txt");
    FILE *f;
    f = fopen(buf, "a");
    if (!f) return;

//  freopen(buf, "a", stdout);
//  freopen(buf, "a", stderr);
#else
    #define f stderr
#endif
    time(&t);
    safe_strcpy(buf, asctime(localtime(&t)));
    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    
    fputs("\n", f);
#if USE_SPECIAL_LOG_FILE
    fflush(f);
    fclose(f);
#endif
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

void PrintBacktrace(void) {
// Dummy routine to satisfy linker
}


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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
#include <sys/param.h>  // for MAXPATHLEN

#include "gui_rpc_client.h"
#include "common_defs.h"
#include "util.h"
#include "Mac_Saver_Module.h"
#include "screensaver.h"
 
//#include <drivers/event_status_driver.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}	// extern "C"
#endif

// It would be nice to alwats display the scrolled progress info in case the 
// graphics application fails to show its window, but displaying the scrolled 
// progress info takes up too much CPU time for this to be practical.  
#define ALWAYS_DISPLAY_PROGRESS_TEXT 0

// Flags for testing & debugging
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
RGBColor gBrandColor = {0xFFFF, 0xFFFF, 0xFFFF};
RGBColor gTextColor = {0xFFFF, 0xFFFF, 0xFFFF};
RGBColor gWhiteTextColor = {0xFFFF, 0xFFFF, 0xFFFF};
RGBColor gOrangeTextColor = {0xFFFF, 0x6262, 0x0000};
RGBColor gGrayTextColor = {0x9999, 0x9999, 0x9999};

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
    if (ispreview)
        return 8;
        
    gspScreensaver = new CScreensaver();
    
    return gspScreensaver->Create();
}

int drawGraphics(GrafPtr aPort) {
    return gspScreensaver->drawGraphics(aPort);
};


void drawPreview(GrafPtr aPort) {
    gspScreensaver->drawPreview(aPort);
};


void closeBOINCSaver() {
    gspScreensaver->ShutdownSaver();
}

CScreensaver::CScreensaver() {
    
    m_dwBlankScreen = 0;
    m_dwBlankTime = 0;
    m_bErrorMode = false;
    m_hrError = 0;
    m_StatusMessageUpdated = false;
    // Display first status update after 5 seconds
    m_statusUpdateCounter = ((STATUSUPDATEINTERVAL-5) * BANNERFREQUENCY);
    
    saverState = SaverState_Idle;
    m_wasAlreadyRunning = false;
    m_CoreClientPID = nil;
    m_MsgBuf[0] = 0;
    setBannerText(0, NULL);
    m_BannerWidth = 0;
    m_CurrentBannerMessage = 0;
    m_QuitDataManagementProc = false;
    m_BrandText = "BOINC";
    m_updating_results = false;
    
    m_hDataManagementThread = NULL;
    m_hGraphicsApplication = NULL;
    m_bResetCoreState = TRUE;
    rpc = 0;
}


int CScreensaver::Create() {
    int newFrequency = 15;
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
    GetCurrentProcess(&psn);
    memset(&pInfo, 0, sizeof(pInfo));
    pInfo.processInfoLength = sizeof( ProcessInfoRec );
    pInfo.processName = NULL;
    err = GetProcessInformation(&psn, &pInfo);
    if ( (err == noErr) && (pInfo.processSignature == 'sprf') ) {
        saverState = SaverState_ControlPanelTestMode;
    }

    // If there are multiple displays, initBOINCSaver may get called 
    // multiple times (once for each display), so we need to guard 
    // against launching multiple instances of the core client
    if (saverState == SaverState_Idle) {
        // Calculate the estimated blank time by adding the starting 
        //  time and and the user-specified time which is in minutes
        m_dwBlankScreen = gGoToBlank;
        if (gGoToBlank)
            m_dwBlankTime = time(0) + (gBlankingTime * 60);
        else
            m_dwBlankTime = 0;

        CFStringGetCString(gPathToBundleResources, m_gfx_Switcher_Path, sizeof(m_gfx_Switcher_Path), kCFStringEncodingMacRoman);
        strlcat(m_gfx_Switcher_Path, "/gfx_switcher", sizeof(m_gfx_Switcher_Path));

        err = initBOINCApp();

        if (saverState == SaverState_LaunchingCoreClient)
        {
            SetError(FALSE, 0);
            m_QuitDataManagementProc = false;
            if (rpc == NULL)
                rpc = new RPC_CLIENT;
            newFrequency = NOBANNERFREQUENCY;
        }
    }
    return newFrequency;
}


OSStatus CScreensaver::initBOINCApp() {
    char boincPath[2048];
    pid_t myPid;
    int status;
    OSStatus err;
    static int retryCount = 0;
    long brandId = 0;
    
    saverState = SaverState_CantLaunchCoreClient;
    
    m_CoreClientPID = FindProcessPID("boinc", 0);
    if (m_CoreClientPID) {
        m_wasAlreadyRunning = true;
        saverState = SaverState_LaunchingCoreClient;
        return noErr;
    }
    
    m_wasAlreadyRunning = false;
    
    if (++retryCount > 3)   // Limit to 3 relaunches to prevent thrashing
        return -1;

    brandId = GetBrandID();
    switch(brandId) {
    case 1:
        m_BrandText = "GridRepublic";
        gBrandColor = gOrangeTextColor; // Orange
        gTextColor = gGrayTextColor;  // Gray
        break;
    default:
        m_BrandText = "BOINC";
        gBrandColor = gWhiteTextColor; // White
        gTextColor = gWhiteTextColor; // White
        break;
    }

    err = GetpathToBOINCManagerApp(boincPath, sizeof(boincPath));
    if (err) {   // If we couldn't find BOINCManager.app, try default path
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
        m_CoreClientPID = myPid;		// make this available globally
        saverState = SaverState_LaunchingCoreClient;
    }

    return noErr;
}


// Returns new desired Animation Frequency (per second) or 0 for no change
int CScreensaver::drawGraphics(GrafPtr aPort) {
    CGrafPtr savePort;
    GDHandle saveGDH;
    int newFrequency = 15;
    pid_t myPid;
    OSStatus err;
    
    ObscureCursor();
    
    m_statusUpdateCounter++;

    switch (saverState) {
    case SaverState_RelaunchCoreClient:
        err = initBOINCApp();
        break;
    
    case  SaverState_LaunchingCoreClient:
        if (m_wasAlreadyRunning)
            setBannerText(ConnectingCCMsg, aPort);
        else
            setBannerText(LaunchingCCMsg, aPort);
       
        myPid = FindProcessPID(NULL, m_CoreClientPID);
        if (myPid) {
            saverState = SaverState_CoreClientRunning;
            rpc->init(NULL);   // Initialize communications with Core Client

            // Set up a separate thread for communicating with Core Client
            CreateDataManagementThread();
            // ToDo: Add a timeout after which we display error message
        } else
            // Take care of the possible race condition where the Core Client was in the  
            // process of shutting down just as ScreenSaver started, so initBOINCApp() 
            // found it already running but now it has shut down.
            if (m_wasAlreadyRunning)  // If we launched it, then just wait for it to start
                saverState = SaverState_RelaunchCoreClient;
         break;

    case SaverState_CoreClientRunning:
            // RPC called in DataManagementProc()
            setBannerText(ConnectingCCMsg, aPort);
            if (! m_bResetCoreState) {
                saverState = SaverState_ConnectedToCoreClient;
            }
        break;
    
    case SaverState_ConnectedToCoreClient:
        switch (m_hrError) {
        case 0:
            break;  // No status response yet from DataManagementProc
        case SCRAPPERR_SCREENSAVERBLANKED:
        default:
            setBannerText(0, aPort);   // No text message
            break;
        case SCRAPPERR_BOINCSUSPENDED:
            setBannerText(BOINCSuspendedMsg, aPort);
            break;
        case SCRAPPERR_BOINCNOAPPSEXECUTING:
            setBannerText(BOINCNoAppsExecutingMsg, aPort);
            break;
        case SCRAPPERR_BOINCNOPROJECTSDETECTED:
            setBannerText(BOINCNoProjectsDetectedMsg, aPort);
            break;
        case SCRAPPERR_SCREENSAVERRUNNING:
#if ! ALWAYS_DISPLAY_PROGRESS_TEXT
            setBannerText(0, aPort);   // No text message
            // Let the science app draw over our window
            break;
#endif
        case SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING:
            if (m_StatusMessageUpdated) {
                setBannerText(m_MsgBuf, aPort);
                updateBannerText(m_MsgBuf, aPort);
                m_StatusMessageUpdated = false;
            }
            break;
#if 0
        case SCRAPPERR_QUITSCREENSAVERREQUESTED:
//            setBannerText(BOINCExitedSaverMode, aPort);
            // Wait 1 second to allow ScreenSaver engine to close us down
            if (++gQuitCounter > (bannerText[0] ? BANNERFREQUENCY : NOBANNERFREQUENCY)) {
                closeBOINCSaver();
                KillScreenSaver(); // Stop the ScreenSaver Engine
            }
            break;
#endif
        }       // end switch (m_hrError)
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

    if (m_BannerText[0]) {
        GetGWorld(&savePort, &saveGDH);
        SetPort(aPort);
        drawBanner(aPort);
        SetGWorld(savePort, saveGDH);
        newFrequency = BANNERFREQUENCY;
    } else
        newFrequency = NOBANNERFREQUENCY;
    
    return newFrequency;
}


void CScreensaver::drawPreview(GrafPtr aPort) {
    SetPort(aPort);
    setBannerText(" BOINC", aPort);
    drawBanner(aPort);
}


// If there are multiple displays, closeBOINCSaver may get called 
// multiple times (once for each display), so we need to guard 
// against any problems that may cause.
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

    setBannerText(0, NULL);

    m_CoreClientPID = 0;
//    gQuitCounter = 0;
    m_wasAlreadyRunning = false;
    m_QuitDataManagementProc = false;
    saverState = SaverState_Idle;
}


// This function forwards to DataManagementProc, which has access to the
//       "this" pointer.
//
void * CScreensaver::DataManagementProcStub(void* param) {
    return gspScreensaver->DataManagementProc();
}


void CScreensaver::HandleRPCError() {
    // Attempt to restart BOINC Client if needed, reinitialize the RPC client and state
    rpc->close();
    
    // There is a possible race condition where the Core Client was in the  
    // process of shutting down just as ScreenSaver started, so initBOINCApp() 
    // found it already running but now it has shut down.  This code takes 
    // care of that and other situations where the Core Client quits unexpectedy.  
    if (FindProcessPID("boinc", 0) == 0) {
        saverState = SaverState_RelaunchCoreClient;
        m_bResetCoreState = true;
     }
    
    rpc->init(NULL);    // Otherwise just reinitialize the RPC client and state and keep trying
    // Error message after timeout?
}

bool CScreensaver::CreateDataManagementThread() {
    int retval;
    
    if (m_hDataManagementThread == NULL) {
        retval = pthread_create(&m_hDataManagementThread, NULL, DataManagementProcStub, 0);
        if (retval) {
            saverState = SaverState_UnrecoverableError;
            return false;
        }
    }
    return true;
}


bool CScreensaver::DestroyDataManagementThread() {
    m_QuitDataManagementProc = true;  // Tell DataManagementProc thread to exit
    if (m_hDataManagementThread) {  // Wait for DataManagementProc thread to exit
        pthread_join(m_hDataManagementThread, NULL);
        m_hDataManagementThread = NULL;
    }
    return true;
}


bool CScreensaver::SetError(bool bErrorMode, unsigned int hrError) {
    m_bErrorMode = bErrorMode;
    m_hrError = hrError;
    if ((hrError == SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING)
#if ALWAYS_DISPLAY_PROGRESS_TEXT
            || (hrError == SCRAPPERR_SCREENSAVERRUNNING)
#endif
            )
    {
        UpdateProgressText();
    }
    return true;
}

void CScreensaver::UpdateProgressText() {
    int iResultCount;
    int iIndex;
    unsigned int len;
    RESULT* theResult;
    PROJECT* pProject;
    char  statusBuf[256];
    double percent_done;

     if ( (m_statusUpdateCounter >= (STATUSUPDATEINTERVAL * BANNERFREQUENCY) ) && !m_updating_results ) {
        if (! m_StatusMessageUpdated) {
            m_statusUpdateCounter = 0;
            strcpy(m_MsgBuf, BOINCNoGraphicAppsExecutingMsg);

        iResultCount = results.results.size();
        theResult = NULL;
        for (iIndex = 0; iIndex < iResultCount; iIndex++) {
            theResult = results.results.at(iIndex);

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

                strlcat(m_MsgBuf, statusBuf, sizeof(m_MsgBuf));
            } else {          // (pProject == NULL): re-synch with client
                 HandleRPCError();
                 return;
            }
        }           // end for() loop
                m_StatusMessageUpdated = true;
        }   // end if (! m_StatusMessageUpdated)
    }                   // end if (m_statusUpdateCounter > time to update)
}


void CScreensaver::setBannerText(const char * msg, GrafPtr aPort) {
    if (msg == 0)
        m_BannerText[0] = 0;
    
    if ((char *)m_CurrentBannerMessage != msg)
        updateBannerText((char *)msg, aPort);
}


void CScreensaver::updateBannerText(char *msg, GrafPtr aPort) {
    CGrafPtr savePort;
    RGBColor saveBackColor;
    Rect wRect;
    char *p, *s;
    
    m_CurrentBannerMessage = (StringPtr)msg;

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
        m_BannerText[0] = '\0';
        do {
            p = strstr(s, "BOINC");
            if (p == NULL) {
                strcat(m_BannerText, s);
            } else {
                strncat(m_BannerText, s, p - s);
                strcat(m_BannerText, m_BrandText);
                s = p + 5;  // s = p + strlen("BOINC");
            }
        } while (p);
        
        m_BannerWidth = TextWidth(m_BannerText, 0, strlen(m_BannerText)) + BANNER_GAP;
        // Round up m_BannerWidth to an integral multiple of BANNERDELTA
        m_BannerWidth = ((m_BannerWidth + BANNERDELTA - 1) / BANNERDELTA) * BANNERDELTA;
    }
    
    SetPort(savePort);
}


void CScreensaver::drawBanner(GrafPtr aPort) {
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
    if ( (bannerPos + m_BannerWidth) <= (wRect.left + BANNERDELTA) )
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
        s = m_BannerText;
        do {
            p = strstr(s, m_BrandText);
            if (p == NULL) {
                DrawText(s, 0, strlen(s));
            } else {
                DrawText(s, 0, p - s);
                RGBForeColor(&gBrandColor);
                DrawText(m_BrandText, 0, strlen(m_BrandText));
                s = p + strlen(m_BrandText);
                RGBForeColor(&gTextColor);
            }
        } while (p);
        
        x+= m_BannerWidth;
    } while (x < wRect.right);
    
    RGBForeColor(&saveForeColor);
    RGBBackColor(&saveBackColor);

    SetGWorld(savePort, saveGDH);
}


int CScreensaver::GetBrandID()
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


void CScreensaver::print_to_log_file(const char *format, ...) {
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
void CScreensaver::strip_cr(char *buf)
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

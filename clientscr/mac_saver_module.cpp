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

//
//  mac_saver_module.cpp
//  BOINC_Saver_Module
//

#include <IOKit/IOKitLib.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

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
#include <pthread.h>
#include <pwd.h>    // getpwuid

#include "gui_rpc_client.h"
#include "common_defs.h"
#include "util.h"
#include "Mac_Saver_Module.h"
#include "screensaver.h"
#include "diagnostics.h"
#include "str_replace.h"
#include "mac_util.h"
#include "mac_branding.h"

//#include <drivers/event_status_driver.h>

// Flags for testing & debugging
#define CREATE_LOG 0
#define USE_SPECIAL_LOG_FILE 1
#define FOR_TESTING_ONLY 0  // Set to 1 to simulate dualGPU MacBook Pro on other Macs
// Set to 1 to simulate 30 seconds on AC, 30 on battery, 30 AC, 30 battery, etc.
#define SIMULATE_AC_BATTERY_SWITCHING 0

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
extern RESULT* graphics_app_result_ptr;

static SaverState saverState = SaverState_Idle;
// int gQuitCounter = 0;

static long brandId = 0;
bool IsDualGPUMacbook = false;
static io_connect_t GPUSelectConnect = IO_OBJECT_NULL;
static bool OKToRunOnBatteries = false;
static bool RunningOnBattery = true;
static time_t ScreenSaverStartTime = 0;
bool ScreenIsBlanked = false;
static int retryCount = 0;
static pthread_mutexattr_t saver_mutex_attr;
pthread_mutex_t saver_mutex;
static char passwd_buf[256];
char gUserName[64];
bool gIsHighSierra = false; // OS 10.13 or later
bool gIsMojave = false;     // OS 10.14 or later
bool gIsCatalina = false;   // OS 10.15 or later
bool gIsSonoma = false;     // OS 14.0 or later

// As of MacOS 14.0, the legacyScreenSave sandbox
// prevents using bootstrap_look_up.
bool gMach_bootstrap_unavailable_to_screensavers = false;

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
const char *  IncompatibleMsg = "Could not connect to screensaver ";
const char *  CCNotRunningMsg = "BOINC is not running.";

//const char *  BOINCExitedSaverMode = "BOINC is no longer in screensaver mode.";

static bool created = false;    // CAF

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

    // When gfx_cleanup exits, it will send a SIGCHLD to legacysceensaver
    // which is normally set to also exit when it receives that signal
    struct sigaction temp;
    sigaction(SIGCHLD, NULL, &temp);
    temp.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &temp, NULL);

for (int i=1; i<NSIG; ++i)
boinc_set_signal_handler(i, boinc_catch_signal);
    if (gspScreensaver == NULL) {
        gspScreensaver = new CScreensaver();
        created = false;    // CAF
   }
}


int startBOINCSaver() {
#if FOR_TESTING_ONLY
    // Simulate dual GPU MacBook Pro on other Macs
    IsDualGPUMacbook = true;
#endif
    if (gspScreensaver) {
        if (!created) {     // CAF
            created = true; // CAF
            return gspScreensaver->Create();
        }                   // CAF
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


void stopAllGFXApps() {
    if (gspScreensaver) {
        gspScreensaver->Shared_Offscreen_Buffer_Unavailable();
    }
}


// If there are multiple displays, this may get called
// multiple times (once for each display), so we need to guard
// against any problems that may cause.
void closeBOINCSaver() {
    if (gspScreensaver) {
        gspScreensaver->ShutdownSaver();
        delete gspScreensaver;
        gspScreensaver = NULL;
    }
for (int i=1; i<32; ++i)
boinc_set_signal_handler(i, boinc_catch_signal);
}


void incompatibleGfxApp(char * appPath, char * wuName, pid_t pid, int slot){
    char *p;
    static char buf[1024];
    static double msgstartTime = 0.0;
    int retval;
    bool gotAppName = false;
    int exitStatus;

    if (gspScreensaver) {
        if (msgstartTime == 0.0) {
            msgstartTime = getDTime();
            buf[0] = '\0';

            if (gspScreensaver->HasProcessExited(pid, exitStatus)) {
                return;
            }

            retval = gspScreensaver->rpc->get_state(gspScreensaver->state);
            if (!retval) {
                strlcpy(buf, IncompatibleMsg, sizeof(buf));
                for (int i=0; i<gspScreensaver->state.results.size(); i++) {
                    RESULT* r = gspScreensaver->state.results[i];
                    if (r->slot == slot) {
                        if (r->app) {
                            if (r->app->user_friendly_name[0]) {
                                strlcat(buf, "of application ", sizeof(buf));
                                strlcat(buf, r->app->user_friendly_name, sizeof(buf));
                                gotAppName = true;
                            }
                        }
                    }
                }
            } // if (!retval)

            if (!gotAppName) {
                p = strrchr(appPath, '/');
                if (!p) p = appPath;
                strlcat(buf, "\"", sizeof(buf));
                strlcat(buf, p+1, sizeof(buf));
                strlcat(buf, "\"", sizeof(buf));
            }
            gspScreensaver->setSSMessageText(buf);
            gspScreensaver->SetError(0, SCRAPPERR_GFXAPPINCOMPATIBLE);
        }   // End if (msgstartTime == 0.0)

        if (msgstartTime && (getDTime() - msgstartTime > 5.0)) {
            gspScreensaver->markAsIncompatible(wuName);
            launchedGfxApp("", "", 0, -1);
            msgstartTime = 0.0;
            gspScreensaver->terminate_v6_screensaver(pid);
        }
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


void doBoinc_Sleep(double seconds) {
    boinc_sleep(seconds);
}


CScreensaver::CScreensaver() {
    struct ss_periods periods;
    char saved_dir[MAXPATHLEN];
    std::string msg;

    m_dwBlankScreen = 0;
    m_dwBlankTime = 0;
    m_bErrorMode = false;
    m_hrError = 0;
    // Display first status update after 5 seconds
    m_iGraphicsStartingMsgCounter = 0;
    saverState = SaverState_Idle;
    m_wasAlreadyRunning = false;
    m_CoreClientPID = 0;
    setSSMessageText(0);
    m_CurrentBannerMessage = 0;
    m_bQuitDataManagementProc = false;
    m_bDataManagementProcStopped = false;

    m_hDataManagementThread = NULL;
    m_hGraphicsApplication = NULL;
    m_bResetCoreState = true;
    rpc = 0;
    m_bConnected = false;
    m_gfx_Cleanup_IPC = NULL;
    safe_strcpy(passwd_buf, "");

    getcwd(saved_dir, sizeof(saved_dir));
    chdir("/Library/Application Support/BOINC Data");
    read_gui_rpc_password(passwd_buf, msg);
    chdir(saved_dir);

    CFStringRef cf_gUserName = SCDynamicStoreCopyConsoleUser(NULL, NULL, NULL);
    CFStringGetCString(cf_gUserName, gUserName, sizeof(gUserName), kCFStringEncodingUTF8);

    // Get project-defined default values for GFXDefaultPeriod, GFXSciencePeriod, GFXChangePeriod
    GetDefaultDisplayPeriods(periods);
    m_bShow_default_ss_first = periods.Show_default_ss_first;

    m_fGFXDefaultPeriod = periods.GFXDefaultPeriod;
    m_fGFXSciencePeriod = periods.GFXSciencePeriod;
    m_fGFXChangePeriod = periods.GFXChangePeriod;

    // MUTEX may help prevent crashes when terminating an older gfx app which
    // we were displaying using CGWindowListCreateImage under OS X >= 10.13
    pthread_mutexattr_settype(&saver_mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&saver_mutex, &saver_mutex_attr);

    brandId = GetBrandID();
    m_BrandText = brandName[brandId];
}


CScreensaver::~CScreensaver() {
}


int CScreensaver::Create() {
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
    pid_t SystemPrefsPID = getPidIfRunning("com.apple.systempreferences");
    if (SystemPrefsPID == getpid()) {
        saverState = SaverState_ControlPanelTestMode;
    }

    // Calculate the estimated blank time by adding the starting
    //  time and and the user-specified time which is in minutes
    // On dual-GPU Macbook Pros, the CScreensaver class will be
    // constructed and destructed each time we switch between
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
        strlcpy(m_gfx_Cleanup_Path, "\"", sizeof(m_gfx_Cleanup_Path));
        strlcat(m_gfx_Cleanup_Path, m_gfx_Switcher_Path, sizeof(m_gfx_Cleanup_Path));
        strlcat(m_gfx_Switcher_Path, "/gfx_switcher", sizeof(m_gfx_Switcher_Path));
        strlcat(m_gfx_Cleanup_Path, "/gfx_cleanup\"", sizeof(m_gfx_Cleanup_Path));
        // Launch helper app to work around a bug in OS 10.15 Catalina to
        // kill current graphics app if ScreensaverEngine exits without
        // first calling [ScreenSaverView stopAnimation]
        //TODO: Should we use this on OS 10.13+ ?
        if (!m_gfx_Cleanup_IPC) {
            m_gfx_Cleanup_IPC = popen(m_gfx_Cleanup_Path, "w");
        }
        initBOINCApp();

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
    saverState = SaverState_CantLaunchCoreClient;

    m_CoreClientPID = getClientPID();
    if (m_CoreClientPID) {
        m_wasAlreadyRunning = true;
        saverState = SaverState_LaunchingCoreClient;
        retryCount = 0;
        return noErr;
    }

    m_wasAlreadyRunning = false;

    return noErr;   // Screensavers can't launch setuid /setgid processes as of Catalina
}


// Returns new desired Animation Frequency (per second) or 0 for no change
int CScreensaver::getSSMessage(char **theMessage, int* coveredFreq) {
    int retval;
    int newFrequency = TEXTLOGOFREQUENCY;
    *coveredFreq = 0;
    pid_t myPid;
    CC_STATE ccstate;

    if (ScreenIsBlanked) {
        setSSMessageText(0);   // No text message
        *theMessage = m_MessageText;
        return NOTEXTLOGOFREQUENCY;
    }

    CheckDualGPUPowerSource();

    switch (saverState) {
    case SaverState_RelaunchCoreClient:
        initBOINCApp();
        break;

    case  SaverState_LaunchingCoreClient:
        if (m_wasAlreadyRunning) {
            setSSMessageText(ConnectingCCMsg);
        } else {
            setSSMessageText(LaunchingCCMsg);
        }

        myPid = getClientPID();
        if (myPid) {
            saverState = SaverState_CoreClientRunning;
            if (!rpc->init(NULL)) {     // Initialize communications with Core Client
                m_bConnected = true;

                if (strlen(passwd_buf)) {
                    retval = rpc->authorize(passwd_buf);
                    if (retval) {
                        fprintf(stderr, "Screensaver RPC authorization failure: %d\n", retval);
                    }
                }

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
        } else {
            return noErr;   // Screensavers can't launch setuid /setgid processes as of Catalina
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
                IOServiceClose(GPUSelectConnect);   // Remove our hold on discrete GPU
                GPUSelectConnect = IO_OBJECT_NULL;
            }
            break;
#if 0   // Not currently used
        case SCRAPPERR_QUITSCREENSAVERREQUESTED:
//            setSSMessageText(BOINCExitedSaverMode);
            // Wait 1 second to allow ScreenSaver engine to close us down
            if (++gQuitCounter > (m_MessageText[0] ? TEXTLOGOFREQUENCY : NOTEXTLOGOFREQUENCY)) {
                closeBOINCSaver();
                exit(0);    // Stop the ScreenSaver Engine
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
            case SCRAPPERR_GFXAPPINCOMPATIBLE:
                // Message was set in incompatibleGfxApp()
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

        setSSMessageText(CCNotRunningMsg);
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


void CScreensaver::Shared_Offscreen_Buffer_Unavailable() {
    DestroyDataManagementThread();  // Kills current GFX app
}


void CScreensaver::ShutdownSaver() {
    DestroyDataManagementThread();

    if (m_gfx_Cleanup_IPC) {
        fprintf(m_gfx_Cleanup_IPC, "Quit\n");
        fflush(m_gfx_Cleanup_IPC);
        pclose(m_gfx_Cleanup_IPC);
        m_gfx_Cleanup_IPC = NULL;
    }

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
        rpc->close();
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
    ScreenSaverStartTime = 0;
    ScreenIsBlanked = false;
}


// This function forwards to DataManagementProc, which has access to the
//       "this" pointer.
//
void * CScreensaver::DataManagementProcStub(void* param) {
    return gspScreensaver->DataManagementProc();
}


void CScreensaver::HandleRPCError() {
    int retval;

    // Attempt to restart BOINC Client if needed, reinitialize the RPC client and state
    rpc->close();
    m_bConnected = false;

    if (saverState == SaverState_CantLaunchCoreClient) {
        return;   // Screensavers can't launch setuid /setgid processes as of Catalina
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
        if (strlen(passwd_buf)) {
            retval = rpc->authorize(passwd_buf);
            if (retval) {
                fprintf(stderr, "Screensaver RPC authorization failure: %d\n", retval);
            }
        }
    }
    // Error message after timeout?
}

bool CScreensaver::CreateDataManagementThread() {
    int retval;

    // On dual-GPU Macbook Pros, our OpenGL scrensaver
    // applications trigger a switch to the power-hungry
    // discrete GPU. To extend battery life, don't run
    // them when on battery power.
    if (IsDualGPUMacbook) {
        if (RunningOnBattery && !OKToRunOnBatteries) return true;
        if (GPUSelectConnect == IO_OBJECT_NULL) return true;
    }

    m_bQuitDataManagementProc = false;
    m_bDataManagementProcStopped = false;

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
    if (!m_hDataManagementThread) return true;
    m_bQuitDataManagementProc = true;  // Tell DataManagementProc thread to exit

    int maxWait = IsDualGPUMacbook ? 50 : 10;
    for (int i=0; i<maxWait; i++) {  // Wait up to 1 second or 5 seconds for DataManagementProc thread to exit
        if (m_bDataManagementProcStopped) {
            m_hDataManagementThread = NULL;
            return true;
        }
        boinc_sleep(0.1);
    }
    m_hDataManagementThread = NULL; // Don't delay more if this routine is called again.

    if (m_hGraphicsApplication) {
        terminate_v6_screensaver(m_hGraphicsApplication);
        m_hGraphicsApplication = 0;
    }

    if (rpc) {
        rpc->close();    // In case DataManagementProc is hung waiting for RPC
    }
    return true;
}


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
        err = GetPathToAppFromID('BNC!', CFSTR("edu.berkeley.boinc"),  buf, sizeof(buf));
        if (err == noErr) {
            strcat(buf, "/Contents/Resources/Branding");
            f = fopen(buf, "r");
        }
    }
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandId);
        fclose(f);
    }
    if ((iBrandId < 0) || (iBrandId > (NUMBRANDS-1))) {
        iBrandId = 0;
    }
    return iBrandId;
}


pid_t CScreensaver::getClientPID() {
    int fd;
    fd = open("//Library/Application Support/BOINC Data/lockfile", O_RDONLY);
    if (fd<0) {
        return 0;   // lockfile doesn't exist (probably)
    }
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_pid = 0;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    // fcntl F_GETLK sets fl.l_pid to PID of lock's owner if the
    // file is locked, else leaves it unchanged
    if (fcntl(fd, F_GETLK, &fl) != 0) {
        close(fd);
        return 0;   // fcntl failed (should never happen)
    }

    close(fd);
    return fl.l_pid;
}


bool CScreensaver::Host_is_running_on_batteries() {
#if SIMULATE_AC_BATTERY_SWITCHING
    // Simulate 30 seconds on AC, 30 on battery, 30 AC, 30 battery, etc.
    time_t elapsed = ScreenSaverStartTime - time(0);
    bool isOnBattery = (elapsed / 30) & 1;
    return isOnBattery;
#else   // NOT SIMULATE_AC_BATTERY_SWITCHING
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
#endif  // NOT SIMULATE_AC_BATTERY_SWITCHING
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

#if FOR_TESTING_ONLY
    if (GPUSelectConnect == IO_OBJECT_NULL) {
        GPUSelectConnect = setDiscrete;
        CreateDataManagementThread();
    }
#else   // NOT FOR_TESTING_ONLY
    if (GPUSelectConnect == IO_OBJECT_NULL) {
        service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleGraphicsControl"));
        if (service != IO_OBJECT_NULL) {
            kernResult = IOServiceOpen(service, mach_task_self(), setDiscrete ? 1 : 0, &GPUSelectConnect);
            if (kernResult == KERN_SUCCESS) {
                IsDualGPUMacbook = true;
                CreateDataManagementThread();
            }
        }
    }
#endif  // NOT FOR_TESTING_ONLY
}


// On Dual-GPU Macbook Pros only:
// switch to intrinsic GPU if running on battery
// switch to discrete GPU if running on AC power
//
// Apple's Screensaver Engine will detect the GPU change and
// call stopAnimation, then initWithFrame and startAnimation.
void CScreensaver::CheckDualGPUPowerSource() {
    static double lastBatteryCheckTime = 0;
    double currentTime;
    bool nowOnBattery;

    if (!IsDualGPUMacbook) return;
    if (OKToRunOnBatteries && (GPUSelectConnect != IO_OBJECT_NULL)) return;

    currentTime = dtime();
    if (currentTime < lastBatteryCheckTime + BATTERY_CHECK_INTERVAL) return;
    lastBatteryCheckTime = currentTime;

    nowOnBattery = Host_is_running_on_batteries();
    if (nowOnBattery == RunningOnBattery) return;

    RunningOnBattery = nowOnBattery;
    if (nowOnBattery) {
        if (GPUSelectConnect != IO_OBJECT_NULL) {
            // If an OpenGL screensaver app is running, we must shut it down
            // to release its claim on the discrete GPU to save battery power.
            DestroyDataManagementThread();
            IOServiceClose(GPUSelectConnect);
            GPUSelectConnect = IO_OBJECT_NULL;
        }
    } else {
        SetDiscreteGPU(true);
    }
}


void print_to_log_file(const char *format, ...) {
#if CREATE_LOG
    va_list args;
    time_t t;
#if USE_SPECIAL_LOG_FILE
    FILE *f = fopen("/Users/Shared/test_log_BOINCSaver.txt", "a");   // CAF
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

#include "QCrashReport.h"

void PrintBacktrace(void) {
// We need this special version of PrintBacktrace because
// mac_backtrace.cpp doesn't work with screensavr sandbox limitatons
#define CALL_STACK_SIZE 128
    typedef int     (*backtraceProc)(void**,int);
    typedef char ** (*backtrace_symbolsProc)(void* const*,int);
    QCrashReportRef             crRef = NULL;

    int                         frames, i;
    void                        *systemlib = NULL;
    void                        *callstack[CALL_STACK_SIZE];
    char                        **symbols = NULL;
    char                        outBuf[1024];
    backtraceProc               myBacktraceProc = NULL;
    backtrace_symbolsProc       myBacktrace_symbolsProc = NULL;

    QCRCreateFromSelf(&crRef);

    systemlib = dlopen ("/usr/lib/libSystem.dylib", RTLD_NOW );
    if (systemlib) {
        myBacktraceProc = (backtraceProc)dlsym(systemlib, "backtrace");
     }
    if (! myBacktraceProc) {
        fprintf(stderr, "no myBacktraceProc\n");
        return;    // Should never happen
    }
    frames = myBacktraceProc(callstack, CALL_STACK_SIZE);
    myBacktrace_symbolsProc = (backtrace_symbolsProc)dlsym(systemlib, "backtrace_symbols");
    if (myBacktrace_symbolsProc) {
        symbols = myBacktrace_symbolsProc(callstack, frames);
    }

    for (i=0; i<frames; i++) {
        strlcpy(outBuf, symbols[i], sizeof(outBuf));
        fprintf(stderr, "%s\n", outBuf);
    }
    fprintf(stderr, "\n");

    // make sure this much gets written to file in case future
    // versions of OS break our crash dump code beyond this point.
    fflush(stderr);

    QCRPrintThreadState(crRef, stderr);
    QCRPrintImages(crRef, stderr);
}

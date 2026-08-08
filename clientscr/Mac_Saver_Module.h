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

//  Mac_Saver_Module.h
//  BOINC_Saver_Module
//

#ifndef _SCREENSAVER_MAC_H
#define _SCREENSAVER_MAC_H

#include <Carbon/Carbon.h>

// The declarations below must be kept in sync with
// the corresponding ones in Mac_Saver_ModuleView.h
#ifdef _DEBUG
    #undef _T
    #define _T(x) x
#endif

#ifdef __cplusplus
extern "C" {
#endif

void            initBOINCSaver(void);
int             startBOINCSaver(void);
int             getSSMessage(char **theMessage, int* coveredFreq);
void            windowIsCovered();
void            drawPreview(CGContextRef myContext);
void            stopAllGFXApps(void);
void            closeBOINCSaver(void);
void            setDefaultDisplayPeriods(void);
bool            getShow_default_ss_first();
double          getGFXDefaultPeriod();
double          getGFXSciencePeriod();
double          getGGFXChangePeriod();
void            incompatibleGfxApp(char * appPath, char * wuName, pid_t pid, int slot);
void            setShow_default_ss_first(bool value);
void            setGFXDefaultPeriod(double value);
void            setGFXSciencePeriod(double value);
void            setGGFXChangePeriod(double value);
double          getDTime();
void            doBoinc_Sleep(double seconds);
void            launchedGfxApp(char * appPath, char * wuName, pid_t thePID, int slot);
int             compareBOINCLibVersionTo(int toMajor, int toMinor, int toRelease);
void            print_to_log_file(const char *format, ...);
void            strip_cr(char *buf);
void            PrintBacktrace(void);
extern char     gUserName[64];
extern bool     gIsMojave;
extern bool     gIsCatalina;
extern bool     gIsHighSierra;
extern bool     gIsSonoma;
extern bool     gMach_bootstrap_unavailable_to_screensavers;
extern mach_port_name_t commsPort;

#ifdef __cplusplus
}	// extern "C"
#endif

// The declarations above must be kept in sync with
// the corresponding ones in Mac_Saver_ModuleView.h

struct ss_periods
{
    double          GFXDefaultPeriod;
    double          GFXSciencePeriod;
    double          GFXChangePeriod;
    bool            Show_default_ss_first;
};

//-----------------------------------------------------------------------------
// Name: class CScreensaver
// Desc: Screensaver class
//-----------------------------------------------------------------------------
class CScreensaver
{
public:
    CScreensaver();
    ~CScreensaver();

    int             Create();
    int             Run();

    //
    // Infrastructure layer
    //
protected:
    OSStatus        initBOINCApp(void);
    int             GetBrandID(void);
    pid_t           getClientPID(void);
    void            updateSSMessageText(char *msg);
    void            strip_cr(char *buf);
    char            m_gfx_Switcher_Path[PATH_MAX];
    char            m_gfx_Cleanup_Path[PATH_MAX];
    FILE*           m_gfx_Cleanup_IPC;
    void            SetDiscreteGPU(bool setDiscrete);
    void            CheckDualGPUPowerSource();
    bool            Host_is_running_on_batteries();

    bool            m_bErrorMode;        // Whether to draw moving logo and possibly display an error
    unsigned int    m_hrError;           // Error code to display

    bool            m_wasAlreadyRunning;
    pid_t           m_CoreClientPID;
    int             m_dwBlankScreen;
    time_t          m_dwBlankTime;
    int             m_iGraphicsStartingMsgCounter;
    bool            m_bDefault_ss_exists;
    bool            m_bScience_gfx_running;
    bool            m_bDefault_gfx_running;
    bool            m_bConnected;
    std::vector<char*>   m_vIncompatibleGfxApps;
    //
    // Data management layer
    //
protected:
    bool            CreateDataManagementThread();
    bool            DestroyDataManagementThread();

    void*           DataManagementProc();
    static void*    DataManagementProcStub( void* param );
    int             terminate_screensaver(int graphics_application);
    int             terminate_default_screensaver(int graphics_application);
    int             launch_screensaver(RESULT* rp, int& graphics_application);
    int             launch_default_screensaver(char *dir_path, int& graphics_application);
    void            HandleRPCError(void);
    void            GetDefaultDisplayPeriods(struct ss_periods &periods);
    pthread_t       m_hDataManagementThread;

// Determine if two RESULT pointers refer to the same task
    bool            is_same_task(RESULT* taska, RESULT* taskb);

// Count the number of active graphics-capable apps
    int             count_active_graphic_apps(RESULTS& results, RESULT* exclude = NULL);

// Choose a random graphics application from the vector that
//   was passed in.

    RESULT*         get_random_graphics_app(RESULTS& results, RESULT* exclude = NULL);

    bool            m_bResetCoreState;
    bool            m_bQuitDataManagementProc;
    bool            m_bDataManagementProcStopped;


    //
    // Presentation layer
    //
protected:
    char            m_MessageText[2048];
    char*           m_CurrentBannerMessage;
    char*           m_BrandText;
public:
    int             getSSMessage(char **theMessage, int* coveredFreq);
    void            windowIsCovered(void);
    void            drawPreview(CGContextRef myContext);
    void            Shared_Offscreen_Buffer_Unavailable(void);
    void            ShutdownSaver();
    void            markAsIncompatible(char *gfxAppName);
    bool            isIncompatible(char *appName);
    bool            SetError( bool bErrorMode, unsigned int hrError );
    void            setSSMessageText(const char *msg);

    int             terminate_v6_screensaver(int graphics_application);
    bool            HasProcessExited(pid_t pid, int &exitCode);

    CC_STATE        state;
    RESULTS         results;
    RPC_CLIENT      *rpc;

    double          m_fGFXDefaultPeriod;
    double          m_fGFXSciencePeriod;
    double          m_fGFXChangePeriod;
    bool            m_bShow_default_ss_first;

    pid_t           m_hGraphicsApplication;

protected:
};

#endif

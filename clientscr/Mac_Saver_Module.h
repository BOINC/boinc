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

//  Mac_Saver_Module.h
//  BOINC_Saver_Module
//

#ifndef _SCREENSAVER_MAC_H
#define _SCREENSAVER_MAC_H

#include <Carbon/Carbon.h>

#ifdef _DEBUG
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
void            closeBOINCSaver(void);
void            setDefaultDisplayPeriods(void);
bool            getShow_default_ss_first();
double          getGFXDefaultPeriod();
double          getGFXSciencePeriod();
double          getGGFXChangePeriod();
bool            validateNumericString(CFStringRef s);
void            setShow_default_ss_first(bool value);
void            setGFXDefaultPeriod(double value);
void            setGFXSciencePeriod(double value);
void            setGGFXChangePeriod(double value);
void            print_to_log_file(const char *format, ...);
void            strip_cr(char *buf);
void            PrintBacktrace(void);

#ifdef __cplusplus
}	// extern "C"
#endif

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

    int             Create();
    int             Run();


    //
    // Infrastructure layer 
    //
protected:
    OSStatus        initBOINCApp(void);
    int             GetBrandID(void);
    char*           PersistentFGets(char *buf, size_t buflen, FILE *f);
    pid_t           FindProcessPID(char* name, pid_t thePID);
    OSErr           GetpathToBOINCManagerApp(char* path, int maxLen);
    bool            SetError( bool bErrorMode, unsigned int hrError );
    void            setSSMessageText(const char *msg);
    void            updateSSMessageText(char *msg);
    void            strip_cr(char *buf);
    char            m_gfx_Switcher_Path[PATH_MAX];
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

    //
    // Data management layer
    //
protected:
    bool            CreateDataManagementThread();
    bool            DestroyDataManagementThread();

    void*           DataManagementProc();
    static void*    DataManagementProcStub( void* param );
    int             terminate_v6_screensaver(int& graphics_application);
    int             terminate_screensaver(int& graphics_application, RESULT *worker_app);
    int             terminate_default_screensaver(int& graphics_application);
    int             launch_screensaver(RESULT* rp, int& graphics_application);
    int             launch_default_screensaver(char *dir_path, int& graphics_application);
    void            HandleRPCError(void);
    OSErr           KillScreenSaver(void);
    void            GetDefaultDisplayPeriods(struct ss_periods &periods);
    bool            HasProcessExited(pid_t pid, int &exitCode);
    pthread_t       m_hDataManagementThread;
    pid_t           m_hGraphicsApplication;

// Determine if two RESULT pointers refer to the same task
    bool            is_same_task(RESULT* taska, RESULT* taskb);

// Count the number of active graphics-capable apps
    int             count_active_graphic_apps(RESULTS& results, RESULT* exclude = NULL);

// Choose a random graphics application from the vector that
//   was passed in.

    RESULT*         get_random_graphics_app(RESULTS& results, RESULT* exclude = NULL);

    RPC_CLIENT      *rpc;
    CC_STATE        state;
    RESULTS         results;
 
    bool            m_bResetCoreState;
    bool            m_QuitDataManagementProc;
    bool            m_bV5_GFX_app_is_running;


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
    void            ShutdownSaver();

    double          m_fGFXDefaultPeriod;
    double          m_fGFXSciencePeriod;
    double          m_fGFXChangePeriod;
    bool            m_bShow_default_ss_first;

protected:
};

#endif

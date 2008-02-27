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

//  Mac_Saver_Module.h
//  BOINC_Saver_Module
//

#ifndef _SCREENSAVER_MAC_H
#define _SCREENSAVER_MAC_H

#include <Carbon/Carbon.h>



#ifdef __cplusplus
extern "C" {
#endif

int initBOINCSaver(Boolean ispreview);
int getSSMessage(char **theMessage, int* coveredFreq);
void drawPreview(CGContextRef myContext);
void closeBOINCSaver(void);
void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

#ifdef __cplusplus
}	// extern "C"
#endif


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
    void            UpdateProgressText(unsigned int hrError);
    void            setSSMessageText(const char *msg);
    void            updateSSMessageText(char *msg);
    void            strip_cr(char *buf);
    char            m_gfx_Switcher_Path[MAXPATHLEN];
    bool            m_bErrorMode;        // Whether to display an error
    unsigned int    m_hrError;           // Error code to display

    bool            m_wasAlreadyRunning;
    pid_t           m_CoreClientPID;
    int             m_dwBlankScreen;
    time_t          m_dwBlankTime;
    int             m_iStatusUpdateCounter;
    int             m_iGraphicsStartingMsgCounter;
    int             m_iLastResultShown;
    int             m_tLastResultChangeCounter;
    bool            m_StatusMessageUpdated;

    //
    // Data management layer
    //
protected:
    bool            CreateDataManagementThread();
    bool            DestroyDataManagementThread();

    void*           DataManagementProc();
    static void*    DataManagementProcStub( void* param );
    int             terminate_screensaver(int& graphics_application, RESULT *worker_app);
    int             launch_screensaver(RESULT* rp, int& graphics_application);
    void            HandleRPCError(void);
    OSErr           KillScreenSaver(void);
    pthread_t       m_hDataManagementThread;
    pid_t           m_hGraphicsApplication;

// Determine if two RESULT pointers refer to the same task
    bool            is_same_task(RESULT* taska, RESULT* taskb);

// Count the number of active graphics-capable apps
    int             count_active_graphic_apps(RESULTS& results, RESULT* exclude = NULL);

// Choose a ramdom graphics application from the vector that
//   was passed in.

    RESULT*         get_random_graphics_app(RESULTS& results, RESULT* exclude = NULL);

    RPC_CLIENT      *rpc;
    CC_STATE        state;
    RESULTS         results;
    RESULT          m_running_result;
    bool            m_updating_results;

    bool            m_bResetCoreState;
    bool            m_QuitDataManagementProc;
    bool            m_bV5_GFX_app_is_running;


    //
    // Presentation layer
    //
protected:
    char            m_MsgBuf[2048];
    char            m_MessageText[2048];
    char*           m_CurrentBannerMessage;
    char*           m_BrandText;
public:
    int             getSSMessage(char **theMessage, int* coveredFreq);
    void            drawPreview(CGContextRef myContext);
    void            ShutdownSaver();

protected:
};

#endif

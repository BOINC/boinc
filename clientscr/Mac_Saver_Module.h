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
int drawGraphics(GrafPtr aPort);
void drawPreview(GrafPtr aPort);
void closeBOINCSaver(void);

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

#ifdef _WIN32
    virtual HRESULT Create( HINSTANCE hInstance );
    HRESULT         DisplayErrorMsg( HRESULT hr );
#elif defined(__APPLE__)
    int             Create();
#endif
    int             Run();


    //
    // Infrastructure layer 
    //
protected:
#ifdef _WIN32
    SaverMode       ParseCommandLine( TCHAR* pstrCommandLine );
    VOID            EnumMonitors( VOID );

    int             UtilGetRegKey(LPCTSTR name, DWORD &keyval);
    int             UtilSetRegKey(LPCTSTR name, DWORD value);
    int             UtilGetRegStartupStr(LPCTSTR name, LPTSTR str);

    BOOL            IsConfigStartupBOINC();

    BOOL            CreateInfrastructureMutexes();

    BOOL            GetError( BOOL& bErrorMode, HRESULT& hrError, TCHAR* pszError, size_t iErrorSize );
    BOOL            SetError( BOOL bErrorMode, HRESULT hrError );
    VOID            UpdateErrorBoxText();
    virtual BOOL    GetTextForError( HRESULT hr, TCHAR* pszError, DWORD dwNumChars );


    // Variables for non-fatal error management
    HANDLE          m_hErrorManagementMutex;
    BOOL            m_bErrorMode;        // Whether to display an error
    HRESULT         m_hrError;           // Error code to display
    TCHAR           m_szError[400];      // Error message text

    BOOL            m_bBOINCConfigChecked;
    BOOL            m_bBOINCStartupConfigured;
    DWORD           m_dwBlankScreen;
    DWORD           m_dwBlankTime;
#elif defined(__APPLE__)
    OSStatus        initBOINCApp(void);
    int             GetBrandID(void);
    char*           PersistentFGets(char *buf, size_t buflen, FILE *f);
    pid_t           FindProcessPID(char* name, pid_t thePID);
    OSErr           GetpathToBOINCManagerApp(char* path, int maxLen);
    bool            SetError( bool bErrorMode, int hrError );
    void            setBannerText(const char *msg, GrafPtr aPort);
    void            updateBannerText(char *msg, GrafPtr aPort);
    void            drawBanner(GrafPtr aPort);
    void            print_to_log_file(const char *format, ...);
    void            strip_cr(char *buf);
    char            m_gfx_Switcher_Path[MAXPATHLEN];
    bool            m_bErrorMode;        // Whether to display an error
    unsigned int    m_hrError;           // Error code to display

    bool            m_wasAlreadyRunning;
    pid_t           m_CoreClientPID;
    int             m_dwBlankScreen;
    time_t          m_dwBlankTime;
#endif


    //
    // Data management layer
    //
protected:
    bool            CreateDataManagementThread();
    bool            DestoryDataManagementThread();

#ifdef _WIN32
    DWORD WINAPI    DataManagementProc();
    static DWORD WINAPI DataManagementProcStub( LPVOID lpParam );
    int             launch_screensaver(RESULT* rp, HANDLE& graphics_application);
    int             terminate_screensaver(HANDLE& graphics_application, RESULT *worker_app);
    HANDLE          m_hDataManagementThread;
    HANDLE          m_hGraphicsApplication;
#elif defined(__APPLE__)
    void*           DataManagementProc();
    static void*    DataManagementProcStub( void* param );
    int             terminate_screensaver(int& graphics_application, RESULT *worker_app);
    int             launch_screensaver(RESULT* rp, int& graphics_application);
    void            HandleRPCError(void);
    OSErr           KillScreenSaver(void);
    pthread_t       m_hDataManagementThread;
    pid_t           m_hGraphicsApplication;
#endif

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
    int             m_iLastResultShown;
    time_t          m_tLastResultChangeTime;

    bool            m_bResetCoreState;
    bool            m_QuitDataManagementProc;


    //
    // Presentation layer
    //
#ifdef _WIN32
protected:
    HRESULT         CreateSaverWindow();
    VOID            UpdateErrorBox();
    VOID            InterruptSaver();
    VOID            ChangePassword();

    VOID            DoConfig();
    HRESULT         DoSaver();
    VOID            DoPaint( HWND hwnd, HDC hdc, LPPAINTSTRUCT lpps );

    void            DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, LONG xStart, LONG yStart, COLORREF cTransparentColor);

    LRESULT         SaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR         ConfigureDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

    static LRESULT CALLBACK SaverProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK ConfigureDialogProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    void            ShutdownSaver();
#elif defined(__APPLE__)        
    char            m_MsgBuf[2048];
    char            m_BannerText[2048];
    int             m_BannerWidth;
    StringPtr       m_CurrentBannerMessage;
    char*           m_BrandText;
public:
    int             drawGraphics(GrafPtr aPort);
    void            drawPreview(GrafPtr aPort);
    void            ShutdownSaver();
#endif

protected:
#ifdef _WIN32
    SaverMode				m_SaverMode;         // sm_config, sm_full, sm_preview, etc.
    BOOL					m_bAllScreensSame;   // If TRUE, show same image on all screens
    HWND					m_hWnd;              // Focus window and device window on primary
    HWND					m_hWndParent;
    HINSTANCE				m_hInstance;
    BOOL					m_bWaitForInputIdle;  // Used to pause when preview starts
    DWORD					m_dwSaverMouseMoveCount;
    BOOL					m_bIs9x;
    BOOL					m_bCheckingSaverPassword;
    BOOL					m_bWindowed;

    INTERNALMONITORINFO		m_Monitors[MAX_DISPLAYS];
    DWORD					m_dwNumMonitors;
    RECT					m_rcRenderTotal;     // Rect of entire area to be rendered
    RECT					m_rcRenderCurDevice; // Rect of render area of current device
    BOOL					m_bPaintingInitialized;

    TCHAR					m_strWindowTitle[200]; // Title for the app's window

    DWORD                   m_dwLastInputTimeAtStartup;
#endif
};

#endif

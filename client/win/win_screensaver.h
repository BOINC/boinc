// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//     DirectX 8.1 Screen Saver Framework from Microsoft.
//     Microsoft Knowledge Base Article - 79212
//

#ifndef _BOINC_H
#define _BOINC_H

#ifdef _DEBUG

// Used for TRACE Macros and the like.
#include <afxwin.h>
#include <windowsx.h>
#include <string.h>
#include <regstr.h>
#include <mmsystem.h>

#else

// Used during the release code
#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <tchar.h>
#include <regstr.h>
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#include <mmsystem.h>

#endif

#include "boinc_ss.h"
#include "win_util.h"


//-----------------------------------------------------------------------------
// Diagnostics Support
//-----------------------------------------------------------------------------
#ifndef _DEBUG

#define TRACE		__noop		// If we are compiling for a release build noop the
								//   trace macro, otherwise use the native debugging
								//   support in the MFC libraries

#endif


//-----------------------------------------------------------------------------
// Error codes
//-----------------------------------------------------------------------------

#define SCRAPPERR_BOINCNOTDETECTED    0x82000001
#define SCRAPPERR_BOINCNOTFOUND       0x82000002
#define SCRAPPERR_NOPREVIEW           0x8200000f


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define MAX_DISPLAYS 9
#define NO_ADAPTER 0xffffffff
#define NO_MONITOR 0xffffffff


//***************************************************************************************
// Modes of operation for screensaver
enum SaverMode
{
    sm_config,         // Config dialog box
    sm_preview,        // Mini preview window in Display Properties dialog
    sm_full,           // Full-on screensaver mode
    sm_test,           // Test mode
    sm_passwordchange  // Change password
};


// Prototype for VerifyScreenSavePwd() in password.cpl, used on Win9x
typedef BOOL (PASCAL * VERIFYPWDPROC) (HWND);


//-----------------------------------------------------------------------------
// Name: struct INTERNALMONITORINFO
// Desc: Structure for holding information about a monitor
//-----------------------------------------------------------------------------
struct INTERNALMONITORINFO
{
    TCHAR          strDeviceName[128];
    TCHAR          strMonitorName[128];
    HMONITOR       hMonitor;
    RECT           rcScreen;
    HWND           hWnd;

    // Error message state
    FLOAT          xError;
    FLOAT          yError;
    FLOAT          widthError;
    FLOAT          heightError;
    FLOAT          xVelError;
    FLOAT          yVelError;
};


// Use the following structure rather than DISPLAY_DEVICE, since some old 
// versions of DISPLAY_DEVICE are missing the last two fields and this can
// cause problems with EnumDisplayDevices on Windows 2000.
struct DISPLAY_DEVICE_FULL
{
    DWORD  cb;
    TCHAR  DeviceName[32];
    TCHAR  DeviceString[128];
    DWORD  StateFlags;
    TCHAR  DeviceID[128];
    TCHAR  DeviceKey[128];
};


//-----------------------------------------------------------------------------
// Name: class CScreensaver
// Desc: Screensaver class
//-----------------------------------------------------------------------------
class CScreensaver
{
public:
    CScreensaver();

    virtual HRESULT Create( HINSTANCE hInstance );
    virtual INT     Run();
    HRESULT         DisplayErrorMsg( HRESULT hr );


protected:
	VOID			RenderBOINC();
	VOID			ShutdownBOINC();

	SaverMode       ParseCommandLine( TCHAR* pstrCommandLine );
	void			DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, LONG xStart, LONG yStart, COLORREF cTransparentColor);

	VOID            EnumMonitors( VOID );
	VOID            ChangePassword();
    HRESULT         CreateSaverWindow();

    VOID            DoConfig();
	HRESULT         DoSaver();
	VOID            DoPaint( HWND hwnd, HDC hdc );

    VOID            InterruptSaver();
    VOID            ShutdownSaver();

	VOID            UpdateErrorBox();
	virtual BOOL    GetTextForError( HRESULT hr, TCHAR* pszError, DWORD dwNumChars );

	LRESULT PrimarySaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	LRESULT GenericSaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	static LRESULT CALLBACK PrimarySaverProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK GenericSaverProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	INT_PTR ConfigureDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR CALLBACK ConfigureDialogProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
    SaverMode				m_SaverMode;         // sm_config, sm_full, sm_preview, etc.
    BOOL					m_bAllScreensSame;   // If TRUE, show same image on all screens
    HWND					m_hWnd;              // Focus window and device window on primary
    HWND					m_hWndParent;
    HINSTANCE				m_hInstance;
    BOOL					m_bWaitForInputIdle;  // Used to pause when preview starts
    DWORD					m_dwSaverMouseMoveCount;
    BOOL					m_bIs9x;
    HINSTANCE				m_hPasswordDLL;
    VERIFYPWDPROC			m_VerifySaverPassword;
    BOOL					m_bCheckingSaverPassword;
    BOOL					m_bWindowed;

    // Variables for non-fatal error management
    BOOL					m_bErrorMode;        // Whether to display an error
    HRESULT					m_hrError;           // Error code to display
    TCHAR					m_szError[400];      // Error message text

    INTERNALMONITORINFO		m_Monitors[MAX_DISPLAYS];
    DWORD					m_dwNumMonitors;
    RECT					m_rcRenderTotal;     // Rect of entire area to be rendered
    RECT					m_rcRenderCurDevice; // Rect of render area of current device
	BOOL					m_bPaintingInitialized;

    TCHAR					m_strWindowTitle[200]; // Title for the app's window
	BOOL					m_bBOINCCoreNotified;

	BOOL					m_bLogMessagePump;

	// Global Messages
	int						BOINC_SS_START_MSG;
	int						BOINC_SS_STOP_MSG;
};

#endif

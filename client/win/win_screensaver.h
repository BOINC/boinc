// 
// Contributor(s):
//     DirectX 8.1 Screen Saver Framework from Microsoft.
//     Microsoft Knowledge Base Article - 79212
//

#ifndef _BOINC_H
#define _BOINC_H

#include "gui_rpc_client.h"


//-----------------------------------------------------------------------------
// Error codes
//-----------------------------------------------------------------------------

#define SCRAPPERR_BOINCNOTDETECTED                0x82000001
#define SCRAPPERR_BOINCNOTDETECTEDSTARTUP         0x82000002
#define SCRAPPERR_BOINCSUSPENDED                  0x82000003
#define SCRAPPERR_BOINCNOTGRAPHICSCAPABLE         0x82000004
#define SCRAPPERR_BOINCNOAPPSEXECUTING            0x82000005
#define SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING    0x82000006
#define SCRAPPERR_NOPREVIEW                       0x8200000f


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define MAX_DISPLAYS                              9
#define NO_ADAPTER                                0xffffffff
#define NO_MONITOR                                0xffffffff


#define BOINC_SHORTCUT_NAME                       _T("\\BOINC Manager.lnk")
#define BOINC_WINDOW_CLASS_NAME                   _T("BOINC_app")


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
	VOID			StartupBOINC();
	VOID			ShutdownBOINC();

	SaverMode       ParseCommandLine( TCHAR* pstrCommandLine );
	VOID            EnumMonitors( VOID );

	HRESULT         CreateSaverWindow();
	VOID            UpdateErrorBox();
	VOID            UpdateErrorBoxText();
    VOID            InterruptSaver();
    VOID            ShutdownSaver();
	VOID            ChangePassword();

    VOID            DoConfig();
	HRESULT         DoSaver();
	VOID            DoPaint( HWND hwnd, HDC hdc );

	void			DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, LONG xStart, LONG yStart, COLORREF cTransparentColor);

	virtual BOOL    GetTextForError( HRESULT hr, TCHAR* pszError, DWORD dwNumChars );
	BOOL			IsConfigStartupBOINC();

	LRESULT         PrimarySaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	LRESULT         GenericSaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR         ConfigureDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

    static LRESULT CALLBACK PrimarySaverProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK GenericSaverProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR CALLBACK ConfigureDialogProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    int             UtilGetRegKey(LPCTSTR name, DWORD &keyval);
    int             UtilSetRegKey(LPCTSTR name, DWORD value);
    int             UtilGetRegStartupStr(LPCTSTR name, LPTSTR str);

protected:
    RPC_CLIENT              rpc;
    CC_STATE                state;

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
    BOOL                    m_bResetCoreState;
    DWORD                   m_dwBOINCTimerCounter;
    DWORD                   m_dwBlankTime;

    BOOL                    m_bBOINCConfigChecked;
    BOOL                    m_bBOINCStartupConfigured;
};

#endif

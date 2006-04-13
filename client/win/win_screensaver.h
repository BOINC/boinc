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

#define SCRAPPERR_BOINCNOTDETECTED                          0x82000001
#define SCRAPPERR_BOINCNOTDETECTEDSTARTUP                   0x82000002
#define SCRAPPERR_BOINCSUSPENDED                            0x82000003
#define SCRAPPERR_BOINCNOTGRAPHICSCAPABLE                   0x82000004
#define SCRAPPERR_BOINCNOAPPSEXECUTING                      0x82000005
#define SCRAPPERR_BOINCNOPROJECTSDETECTED                   0x82000006
#define SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING              0x82000007  
#define SCRAPPERR_BOINCSCREENSAVERLOADING                   0x82000008
#define SCRAPPERR_BOINCAPPFOUNDGRAPHICSLOADING              0x82000009
#define SCRAPPERR_BOINCSHUTDOWNEVENT                        0x8200000a
#define SCRAPPERR_NOPREVIEW                                 0x8200000f


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define MAX_DISPLAYS                              9
#define NO_ADAPTER                                0xffffffff
#define NO_MONITOR                                0xffffffff

#define BSF_ALLOWSFW                              0x00000080

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


//-----------------------------------------------------------------------------
// Name: struct LASTINPUTINFO
// Desc: Structure for holding input idle detection values on Win2k+
//       systems.
//-----------------------------------------------------------------------------
typedef struct tagLASTINPUTINFO {
    UINT cbSize;
    DWORD dwTime;
} LASTINPUTINFO, *PLASTINPUTINFO;


//-----------------------------------------------------------------------------
// Name: struct INTERNALMONITORINFO
// Desc: Use the following structure rather than DISPLAY_DEVICE, since some
//       old versions of DISPLAY_DEVICE are missing the last two fields and
//       this can cause problems with EnumDisplayDevices on Windows 2000.
//-----------------------------------------------------------------------------
struct DISPLAY_DEVICE_FULL
{
    DWORD  cb;
    TCHAR  DeviceName[32];
    TCHAR  DeviceString[128];
    DWORD  StateFlags;
    TCHAR  DeviceID[128];
    TCHAR  DeviceKey[128];
};


// Prototype for VerifyScreenSavePwd() in password.cpl, used on Win9x
typedef BOOL (WINAPI *VERIFYPWDPROC)(HWND);

// Prototype for GetLastInputInto() in user32.dll, used on Win2k or better.
typedef BOOL (WINAPI *MYGETLASTINPUTINFO)(PLASTINPUTINFO);

// Prototype for GetLastInputInto() in user32.dll, used on Win2k or better.
typedef BOOL (WINAPI *MYISHUNGAPPWINDOW)(HWND hWnd);

// Prototype for BroadcastSystemMessage() in user32.dll.
typedef long (WINAPI *MYBROADCASTSYSTEMMESSAGE)(DWORD dwFlags, LPDWORD lpdwRecipients, UINT uiMessage, WPARAM wParam, LPARAM lParam);

// Prototype for SHGetFolderPath() in shlwapi.dll.
typedef HRESULT (WINAPI *MYSHGETFOLDERPATH)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);


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


    //
    // Infrastructure layer 
    //
protected:
	SaverMode       ParseCommandLine( TCHAR* pstrCommandLine );
	VOID            EnumMonitors( VOID );

    int             UtilGetRegKey(LPCTSTR name, DWORD &keyval);
    int             UtilSetRegKey(LPCTSTR name, DWORD value);
    int             UtilGetRegStartupStr(LPCTSTR name, LPTSTR str);

	BOOL			IsConfigStartupBOINC();

    BOOL            CreateInfrastructureMutexes();

    BOOL            GetError( BOOL& bErrorMode, HRESULT& hrError, TCHAR* pszError, size_t iErrorSize );
    BOOL            SetError( BOOL bErrorMode, HRESULT hrError );
	VOID            UpdateErrorBoxText();
	virtual BOOL    GetTextForError( HRESULT hr, TCHAR* pszError, DWORD dwNumChars );


    // Variables for non-fatal error management
    HANDLE          m_hErrorManagementMutex;
    BOOL			m_bErrorMode;        // Whether to display an error
    HRESULT			m_hrError;           // Error code to display
    TCHAR			m_szError[400];      // Error message text

    BOOL            m_bBOINCConfigChecked;
    BOOL            m_bBOINCStartupConfigured;
    DWORD           m_dwBlankScreen;
    DWORD           m_dwBlankTime;


    //
    // Data management layer
    //
protected:
    BOOL            CreateDataManagementThread();
    BOOL            DestoryDataManagementThread();

    DWORD WINAPI    DataManagementProc();
    static DWORD WINAPI DataManagementProcStub( LPVOID lpParam );

    VOID			StartupBOINC();
	VOID			ShutdownBOINC();

    RPC_CLIENT      rpc;
    CC_STATE        state;

    HANDLE          m_hDataManagementThread;

    BOOL			m_bCoreNotified;
    BOOL            m_bResetCoreState;
    int             m_iStatus;


    //
    // Presentation layer
    //
protected:
	HRESULT         CreateSaverWindow();
	VOID            UpdateErrorBox();
    VOID            InterruptSaver();
    VOID            ShutdownSaver();
	VOID            ChangePassword();

    VOID            DoConfig();
	HRESULT         DoSaver();
	VOID            DoPaint( HWND hwnd, HDC hdc );

	void			DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, LONG xStart, LONG yStart, COLORREF cTransparentColor);

	LRESULT         SaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR         ConfigureDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

    static LRESULT CALLBACK SaverProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
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
    BOOL					m_bCheckingSaverPassword;
    BOOL					m_bWindowed;

    INTERNALMONITORINFO		m_Monitors[MAX_DISPLAYS];
    DWORD					m_dwNumMonitors;
    RECT					m_rcRenderTotal;     // Rect of entire area to be rendered
    RECT					m_rcRenderCurDevice; // Rect of render area of current device
	BOOL					m_bPaintingInitialized;

    TCHAR					m_strWindowTitle[200]; // Title for the app's window

    DWORD                   m_dwLastInputTimeAtStartup;
};

#endif

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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCGUIApp.h"
#endif

#ifdef __WXMAC__
#include <Carbon/Carbon.h>
#include "filesys.h"
#include "util.h"
#if (defined(SANDBOX) && defined(_DEBUG))
#include "SetupSecurity.h"
#endif
#include "sandbox.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "network.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "Events.h"
#include "common/wxFlatNotebook.h"
#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCClientManager.h"
#include "BOINCTaskBar.h"
#include "BOINCBaseFrame.h"
#include "AdvancedFrame.h"
#include "sg_ImageLoader.h"
#include "sg_StatImageLoader.h"
#include "sg_BoincSimpleGUI.h"
#include "DlgGenericMessage.h"

static bool s_bSkipExitConfirmation = false;

#ifdef __WXMSW__
EXTERN_C BOOL  ClientLibraryStartup();
EXTERN_C BOOL  IdleTrackerAttach();
EXTERN_C void  IdleTrackerDetach();
EXTERN_C void  ClientLibraryShutdown();
EXTERN_C DWORD BOINCGetIdleTickCount();
#endif


DEFINE_EVENT_TYPE(wxEVT_RPC_FINISHED)

IMPLEMENT_APP(CBOINCGUIApp)
IMPLEMENT_DYNAMIC_CLASS(CBOINCGUIApp, wxApp)

BEGIN_EVENT_TABLE (CBOINCGUIApp, wxApp)
    EVT_RPC_FINISHED(CBOINCGUIApp::OnRPCFinished)
END_EVENT_TABLE ()


bool CBOINCGUIApp::OnInit() {

    // Setup variables with default values
    m_strBOINCArguments = wxEmptyString;
    m_strBOINCMGRRootDirectory = wxEmptyString;
    m_strBOINCMGRDataDirectory = wxEmptyString;
    m_pLocale = NULL;
    m_pSkinManager = NULL;
    m_pFrame = NULL;
    m_pDocument = NULL;
    m_pTaskBarIcon = NULL;
#ifdef __WXMAC__
    m_pMacSystemMenu = NULL;
    printf("Using %s.\n", wxVERSION_STRING);    // For debugging
#endif
    m_bGUIVisible = true;
    m_strDefaultWindowStation = wxEmptyString;
    m_strDefaultDesktop = wxEmptyString;
    m_strDefaultDisplay = wxEmptyString;
    m_iGUISelected = BOINC_SIMPLEGUI;
    m_bSafeMessageBoxDisplayed = 0;
#ifdef __WXMSW__
    m_hClientLibraryDll = NULL;
#endif

#if (defined(SANDBOX) || defined(__WXMAC__))
    int errCode = 0;
#endif

#ifdef SANDBOX
    g_use_sandbox = true;
#else
    g_use_sandbox = false;
#endif

    // Commandline parsing is done in wxApp::OnInit()
    if (!wxApp::OnInit()) {
        return false;
    }
    
#ifndef _WIN32
    if (g_use_sandbox)
        umask (2);  // Set file creation mask to be writable by both user and group
                    // Our umask will be inherited by all our child processes
#endif

    // Setup application and company information
    SetAppName(wxT("BOINC Manager"));
    SetVendorName(wxT("Space Sciences Laboratory, U.C. Berkeley"));

    // Initialize the configuration storage module
    m_pConfig = new wxConfig(GetAppName());
    wxConfigBase::Set(m_pConfig);
    wxASSERT(m_pConfig);

    m_pConfig->SetPath(wxT("/"));

#ifdef __WXMSW__

    //
    // Determine BOINCMgr Data Directory
    //
	LONG    lReturnValue;
	HKEY    hkSetupHive;
    LPTSTR  lpszRegistryValue = NULL;
	DWORD   dwSize = 0;

    // change the current directory to the boinc data directory if it exists
	lReturnValue = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, 
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup"),
		0, 
        KEY_READ,
        &hkSetupHive
    );
    if (lReturnValue == ERROR_SUCCESS) {
        // How large does our buffer need to be?
        lReturnValue = RegQueryValueEx(
            hkSetupHive,
            _T("DATADIR"),
            NULL,
            NULL,
            NULL,
            &dwSize
        );
        if (lReturnValue != ERROR_FILE_NOT_FOUND) {
            // Allocate the buffer space.
            lpszRegistryValue = (LPTSTR) malloc(dwSize);
            (*lpszRegistryValue) = NULL;

            // Now get the data
            lReturnValue = RegQueryValueEx( 
                hkSetupHive,
                _T("DATADIR"),
                NULL,
                NULL,
                (LPBYTE)lpszRegistryValue,
                &dwSize
            );

            SetCurrentDirectory(lpszRegistryValue);

            // Store the root directory for later use.
            m_strBOINCMGRDataDirectory = lpszRegistryValue;
        }
    }

    // Cleanup
	if (hkSetupHive) RegCloseKey(hkSetupHive);
    if (lpszRegistryValue) free(lpszRegistryValue);


    //
    // Determine BOINCMgr Root Directory
    //
    TCHAR   szPath[MAX_PATH-1];

    // change the current directory to the boinc install directory
    GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR)));
		
    TCHAR *pszProg = strrchr(szPath, '\\');
    if (pszProg) {
        szPath[pszProg - szPath + 1] = 0;
    }

    // Store the root directory for later use.
    m_strBOINCMGRRootDirectory = szPath;

#endif

#ifdef __WXMAC__

#if wxCHECK_VERSION(2,8,0)
// In wxMac-2.8.7, default wxListCtrl::RefreshItem() does not work
// so use traditional generic implementation.
// This has been fixed in wxMac-2.8.8, but the Mac native implementation:
//  - takes 3 times the CPU time as the Mac generic version.
//  - seems to always redraw entire control even if asked to refresh only one row.
//  - causes major flicker of progress bars, (probably due to full redraws.)
    wxSystemOptions::SetOption(wxT("mac.listctrl.always_use_generic"), 1);
#endif
	
    wxString strDirectory = wxEmptyString;
    bool success;
    ProcessSerialNumber psn;

    // Set the current directory ahead of the application launch so the core
    //   client can find its files

    // The mac installer sets the "setuid & setgid" bits for the 
    // BOINC Manager and core client so any user can run them and 
    // they can operate on shared data.
    strDirectory = wxT("/Library/Application Support/");

    success = ::wxSetWorkingDirectory(strDirectory);
    if (success) {
        // If SetWD failed, don't create a directory in wrong place
        strDirectory += wxT("BOINC Data");  // We don't customize BOINC Data directory name for branding
        if (! g_use_sandbox) {
            if (! wxDirExists(strDirectory))
                success = wxMkdir(strDirectory, 0777);    // Does nothing if dir exists
        }
        success = ::wxSetWorkingDirectory(strDirectory);
//    wxChar *wd = wxGetWorkingDirectory(buf, 1000);  // For debugging
    }

    if (!success)  // wxSetWorkingDirectory("/Library/Application Support/BOINC Data") FAILED
        errCode = -1016;
#endif      // __WXMAC__
 
#ifdef SANDBOX
    // Make sure owners, groups and permissions are correct for the current setting of g_use_sandbox
    if (!errCode) {
#if (defined(__WXMAC__) && defined(_DEBUG))     // TODO: implement this for other platforms
        // GDB can't attach to applications which are running as a different user   
        //  or group, so fix up data with current user and group during debugging
        if (check_security(g_use_sandbox, true)) {
            CreateBOINCUsersAndGroups();
            SetBOINCDataOwnersGroupsAndPermissions();
            SetBOINCAppOwnersGroupsAndPermissions(NULL);
        }
#endif  // __WXMAC__ && _DEBUG
        errCode = check_security(g_use_sandbox, true);
    }
       
    if (errCode) {
        wxString strDialogMessage = wxEmptyString;
        strDialogMessage.Printf(
            _("BOINC ownership or permissions are not set properly; please reinstall BOINC.\n(Error code %d)"),
            errCode
        );
        wxMessageDialog* pDlg = new wxMessageDialog(NULL, strDialogMessage, wxT(""), wxOK);
        GetCurrentProcess(&psn);
        SetFrontProcess(&psn);  // Shows process if hidden
        pDlg->ShowModal();
        if (pDlg)
            pDlg->Destroy();
        return false;
    }
#endif      // SANDBOX

    // Initialize the BOINC Diagnostics Framework
    int dwDiagnosticsFlags =
        BOINC_DIAG_DUMPCALLSTACKENABLED | 
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_MEMORYLEAKCHECKENABLED |
#if defined(__WXMSW__) || defined(__WXMAC__)
        BOINC_DIAG_REDIRECTSTDERR |
        BOINC_DIAG_REDIRECTSTDOUT |
#endif
        BOINC_DIAG_TRACETOSTDOUT;

    diagnostics_init(
        dwDiagnosticsFlags,
        "stdoutgui",
        "stderrgui"
    );

    // Enable Logging and Trace Masks
    m_pLog = new wxLogBOINC();
    wxLog::SetActiveTarget(m_pLog);

    m_pLog->AddTraceMask(wxT("Function Start/End"));
    m_pLog->AddTraceMask(wxT("Function Status"));

    // Enable known image types
    wxImage::AddHandler(new wxXPMHandler);
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxGIFHandler);
	wxImage::AddHandler(new wxICOHandler);

    // Initialize the internationalization module
    m_pLocale = new wxLocale();
    wxASSERT(m_pLocale);

    wxInt32 iSelectedLanguage = m_pConfig->Read(wxT("Language"), 0L);

    // Look for the localization files by absolute and relative locations.
    //   preference given to the absolute location.
    m_pLocale->Init(iSelectedLanguage);
    if (!m_strBOINCMGRRootDirectory.IsEmpty()) {
        m_pLocale->AddCatalogLookupPathPrefix(
            wxString(m_strBOINCMGRRootDirectory + wxT("locale"))
        );
    }
    m_pLocale->AddCatalogLookupPathPrefix(wxT("locale"));
    m_pLocale->AddCatalog(wxT("BOINC-Manager"));

    InitSupportedLanguages();

    // Note: JAWS for Windows will only speak the context-sensitive
    // help if you use this help provider:
    wxHelpProvider::Set(new wxHelpControllerHelpProvider());

    // Initialize the skin manager
    m_pSkinManager = new CSkinManager();
    wxASSERT(m_pSkinManager);

    m_pSkinManager->ReloadSkin(
        m_pLocale, 
        m_pConfig->Read(wxT("Skin"), m_pSkinManager->GetDefaultSkinName())
    );

#ifdef __WXMSW__
    // Perform any last minute checks that should keep the manager
    // from starting up.
    wxString strRebootPendingFile = 
        GetRootDirectory() + wxFileName::GetPathSeparator() + wxT("RebootPending.txt");
    
    wxFileInputStream fisRebootPending(strRebootPendingFile);
    if (fisRebootPending.IsOk()) {

        wxMessageDialog dialog(
            NULL,
            _("A reboot is required in order for BOINC to run properly.\nPlease reboot your computer and try again."),
            _("BOINC Manager"),
            wxOK|wxICON_ERROR
        );

        dialog.ShowModal();
        return false;
    }
#endif

    // Initialize the main document
    m_pDocument = new CMainDocument();
    wxASSERT(m_pDocument);

    m_pDocument->OnInit();

    // Which GUI should be displayed?
    m_iGUISelected = m_pConfig->Read(wxT("GUISelection"), BOINC_SIMPLEGUI);

    // Is there a condition in which the Simple GUI should not be used?
    if (BOINC_SIMPLEGUI == m_iGUISelected) {

        // Screen too small?
        if (wxGetDisplaySize().GetHeight() < 600) {
            m_iGUISelected = BOINC_ADVANCEDGUI;
        }

        // Screen reader in use?
#ifdef __WXMSW__
        BOOL bScreenReaderEnabled = false;
        SystemParametersInfo(SPI_GETSCREENREADER, NULL, &bScreenReaderEnabled, NULL);
        if (bScreenReaderEnabled) {
            m_iGUISelected = BOINC_ADVANCEDGUI;
        }
#endif
    }

#ifdef __WXMAC__
#if 0       // We may still need this code; don't remove it yet -- CAF 1/30/08
    // When running BOINC Client as a daemon / service, the menubar icon is sometimes 
    // unresponsive to mouse clicks if we create it before connecting to the Client.
    CBOINCClientManager* pcm = m_pDocument->m_pClientManager;
    if (pcm->IsSystemBooting() && pcm->IsBOINCConfiguredAsDaemon()) {
        pcm->StartupBOINCCore();
    }
#endif
#endif

    // Initialize the task bar icon
#if defined(__WXMSW__) || defined(__WXMAC__)
	m_pTaskBarIcon = new CTaskBarIcon(
        m_pSkinManager->GetAdvanced()->GetApplicationName(), 
        m_pSkinManager->GetAdvanced()->GetApplicationIcon(),
        m_pSkinManager->GetAdvanced()->GetApplicationDisconnectedIcon(),
        m_pSkinManager->GetAdvanced()->GetApplicationSnoozeIcon()
    );
    wxASSERT(m_pTaskBarIcon);
#endif

    // Detect the display info and store for later use.
    DetectDisplayInfo();

    // Startup the System Idle Detection code
    ClientLibraryStartup();
    IdleTrackerAttach();

#ifdef __WXMAC__
    s_bSkipExitConfirmation = false;
    AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );

    m_pMacSystemMenu = new CMacSystemMenu(
        m_pSkinManager->GetAdvanced()->GetApplicationName(), 
        m_pSkinManager->GetAdvanced()->GetApplicationIcon(),
        m_pSkinManager->GetAdvanced()->GetApplicationDisconnectedIcon(),
        m_pSkinManager->GetAdvanced()->GetApplicationSnoozeIcon()
    );
    wxASSERT(m_pMacSystemMenu);

    ProcessInfoRec pInfo;
    OSStatus err;
    
    GetCurrentProcess(&psn);
    memset(&pInfo, 0, sizeof(pInfo));
    pInfo.processInfoLength = sizeof( ProcessInfoRec );
    err = GetProcessInformation(&psn, &pInfo);
    if (!err) {
        psn = pInfo.processLauncher;
        memset(&pInfo, 0, sizeof(pInfo));
        pInfo.processInfoLength = sizeof( ProcessInfoRec );
        err = GetProcessInformation(&psn, &pInfo);
    }
    // Don't open main window if we were started automatically at login
    if (pInfo.processSignature == 'lgnw') {  // Login Window app
        m_bGUIVisible = false;

        // If the system was just started, we usually get a "Connection 
        // failed" error if we try to connect too soon, so delay a bit.
        sleep(10);
    }
#endif

    // Show the UI
    SetActiveGUI(m_iGUISelected, false);
    if (m_bGUIVisible) {
        SetActiveGUI(m_iGUISelected);
    } else {
#ifdef __WXMAC__
        GetCurrentProcess(&psn);
        ShowHideProcess(&psn, false);
#endif
	}

    return true;
}


int CBOINCGUIApp::OnExit() {
    // Shutdown the System Idle Detection code
    IdleTrackerDetach();
    ClientLibraryShutdown();

    if (m_pDocument) {
        m_pDocument->OnExit();
        delete m_pDocument;
    }

    if (m_pSkinManager) {
        m_pConfig->Write(wxT("Skin"), m_pSkinManager->GetSelectedSkin());
        delete m_pSkinManager;
    }

    if (m_pLocale) {
        delete m_pLocale;
    }

    return wxApp::OnExit();
}


void CBOINCGUIApp::OnInitCmdLine(wxCmdLineParser &parser) {
    wxApp::OnInitCmdLine(parser);
    static const wxCmdLineEntryDesc cmdLineDesc[] = {
        { wxCMD_LINE_SWITCH, wxT("s"), wxT("systray"), _("Startup BOINC so only the system tray icon is visible")},
        { wxCMD_LINE_SWITCH, wxT("b"), wxT("boincargs"), _("Startup BOINC with these optional arguments")},
        { wxCMD_LINE_SWITCH, wxT("i"), wxT("insecure"), _("disable BOINC security users and permissions")},
        { wxCMD_LINE_NONE}  //DON'T forget this line!!
    };
    parser.SetDesc(cmdLineDesc);
}


bool CBOINCGUIApp::OnCmdLineParsed(wxCmdLineParser &parser) {
    // Give default processing (-?, --help and --verbose) the chance to do something.
    wxApp::OnCmdLineParsed(parser);
    parser.Found(wxT("boincargs"), &m_strBOINCArguments);
    if (parser.Found(wxT("systray"))) {
        m_bGUIVisible = false;
    }
    if (parser.Found(wxT("insecure"))) {
        g_use_sandbox = false;
    }
    return true;
}


void CBOINCGUIApp::DetectDisplayInfo() {
#ifdef __WXMSW__
    wxChar szWindowStation[256];
    memset(szWindowStation, 0, sizeof(szWindowStation)/sizeof(wxChar));
    wxChar szDesktop[256];
    memset(szDesktop, 0, sizeof(szDesktop)/sizeof(wxChar));

    if (wxWIN95 != wxGetOsVersion(NULL, NULL)) {
        // Retrieve the current window station and desktop names
        GetUserObjectInformation(
            GetProcessWindowStation(), 
            UOI_NAME, 
            szWindowStation,
            (sizeof(szWindowStation) / sizeof(wxChar)),
            NULL
        );
        GetUserObjectInformation(
            GetThreadDesktop(GetCurrentThreadId()), 
            UOI_NAME, 
            szDesktop,
            (sizeof(szDesktop) / sizeof(wxChar)),
            NULL
        );
        m_strDefaultWindowStation = szWindowStation;
        m_strDefaultDesktop = szDesktop;
    }

#else
    wxString p = wxString(getenv("DISPLAY"), wxConvUTF8);
    if (p) m_strDefaultDisplay = p;
#endif

}


void CBOINCGUIApp::InitSupportedLanguages() {
    wxInt32               iIndex = 0;
    const wxLanguageInfo* liLanguage = NULL;

    // Prepare the array
    m_astrLanguages.Insert(wxEmptyString, 0, wxLANGUAGE_USER_DEFINED+1);

    // These are just special tags so deal with them in a special way
    m_astrLanguages[wxLANGUAGE_DEFAULT]                    = _("(Automatic Detection)");
    m_astrLanguages[wxLANGUAGE_UNKNOWN]                    = _("(Unknown)");
    m_astrLanguages[wxLANGUAGE_USER_DEFINED]               = _("(User Defined)");

    for (iIndex = 0; iIndex <= wxLANGUAGE_USER_DEFINED; iIndex++) {
        liLanguage = wxLocale::GetLanguageInfo(iIndex);
        if (liLanguage) {
            m_astrLanguages[iIndex] = liLanguage->Description;
        }
    }
}


#ifdef __WXMAC__

// Set s_bSkipExitConfirmation to true if cancelled because of logging out or shutting down
OSErr CBOINCGUIApp::QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon ) {
        DescType		senderType;
        Size			actualSize;
        ProcessSerialNumber	SenderPSN, ourPSN;
        Boolean			isSame;
        ProcessInfoRec		pInfo;
        FSSpec			fileSpec;
 	OSStatus		anErr;

        // Refuse to quit if a modal dialog is open.  
        // Unfortunately, I know of no way to disable the Quit item in our Dock menu
        if (wxGetApp().IsModalDialogDisplayed()) {
            SysBeep(4);
            return userCanceledErr;
        }
                
       anErr = AEGetAttributePtr(appleEvt, keyAddressAttr, typeProcessSerialNumber,
                                    &senderType, &SenderPSN, sizeof(SenderPSN), &actualSize);

        if (anErr == noErr) {
             
            GetCurrentProcess(&ourPSN);

            anErr = SameProcess(&SenderPSN, &ourPSN, &isSame);

            if (anErr == noErr) {
                if (!isSame) {

                pInfo.processInfoLength = sizeof( ProcessInfoRec );
                pInfo.processName = NULL;
                pInfo.processAppSpec = &fileSpec;

                anErr = GetProcessInformation(&SenderPSN, &pInfo);

                // Consider a Quit command from our Dock menu as coming from this application
                if (pInfo.processSignature != 'dock') {
                    s_bSkipExitConfirmation = true; // Not from our app, our dock icon or our taskbar icon
                    wxGetApp().ExitMainLoop();  // Prevents wxMac from issuing events to closed frames
                }
            }
        }
    }
    
    return wxGetApp().MacHandleAEQuit((AppleEvent*)appleEvt, reply);
}

#endif


int CBOINCGUIApp::ClientLibraryStartup() {
#ifdef __WXMSW__
    ::ClientLibraryStartup();
#endif
    return 0;
}


int CBOINCGUIApp::IdleTrackerAttach() {
#ifdef __WXMSW__
    ::IdleTrackerAttach();
#endif
    return 0;
}


int CBOINCGUIApp::IdleTrackerDetach() {
#ifdef __WXMSW__
    ::IdleTrackerDetach();
#endif
    return 0;
}


int CBOINCGUIApp::ClientLibraryShutdown() {
#ifdef __WXMSW__
    ::ClientLibraryShutdown();
#endif
    return 0;
}


void CBOINCGUIApp::OnRPCFinished( CRPCFinishedEvent& event ) {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
   
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    
    pDoc->OnRPCComplete(event);
}


int CBOINCGUIApp::UpdateSystemIdleDetection() {
#ifdef __WXMSW__
    return BOINCGetIdleTickCount();
#else
    return TRUE;
#endif
}


int CBOINCGUIApp::StartBOINCScreensaverTest() {
#ifdef __WXMSW__
    wxString strExecute = wxEmptyString;
    wxChar   szExecutableDirectory[4096];
    memset(szExecutableDirectory, 0, sizeof(szExecutableDirectory));

    // On Windows the screensaver is located in the Windows directory.
    GetWindowsDirectory(
        szExecutableDirectory,
        (sizeof(szExecutableDirectory) / sizeof(wxChar))
    );

    // Append boinc.scr to the end of the strExecute string and get ready to rock
    strExecute = wxT("\"") + wxString(szExecutableDirectory) + wxT("\\boinc.scr\" /t");
    ::wxExecute(strExecute);
#endif
    return 0;
}


// The skin has changed and all UI elements need to reload their bitmaps.
//
void CBOINCGUIApp::FireReloadSkin() {
    if (m_pFrame) {
	    m_pFrame->FireReloadSkin();
    }
    if (m_pTaskBarIcon) {
	    m_pTaskBarIcon->FireReloadSkin();
    }
}


bool CBOINCGUIApp::SetActiveGUI(int iGUISelection, bool bShowWindow) {
    CBOINCBaseFrame* pNewFrame = NULL;

    // Create the new window
    if ((iGUISelection != m_iGUISelected) || !m_pFrame) {
        switch(iGUISelection) {
            case BOINC_SIMPLEGUI:
            default:
                // Initialize the simple gui window
                pNewFrame = new CSimpleFrame(
                    m_pSkinManager->GetAdvanced()->GetApplicationName(), 
                    m_pSkinManager->GetAdvanced()->GetApplicationIcon(),
                    m_pSkinManager->GetAdvanced()->GetApplicationIcon32()
                );
                break;
            case BOINC_ADVANCEDGUI:
                // Initialize the advanced gui window
                pNewFrame = new CAdvancedFrame(
                    m_pSkinManager->GetAdvanced()->GetApplicationName(), 
                    m_pSkinManager->GetAdvanced()->GetApplicationIcon(),
                    m_pSkinManager->GetAdvanced()->GetApplicationIcon32()
                );
                break;
        }
        wxASSERT(pNewFrame);
        if (pNewFrame) {
            SetTopWindow(pNewFrame);

#ifdef __WXMAC__
            // So closing old view doesn't hide application
            pNewFrame->m_iWindowType = iGUISelection;
            m_iGUISelected = iGUISelection;
#endif
            // Delete the old one if it exists
            if (m_pFrame) m_pFrame->Destroy();

            // Store the new frame for future use
            m_pFrame = pNewFrame;
        }
    }

    // Show the new frame if needed 
    if (m_pFrame && bShowWindow) m_pFrame->Show();

    m_iGUISelected = iGUISelection;
    m_pConfig->Write(wxT("GUISelection"), iGUISelection);

    return true;
}


int CBOINCGUIApp::GetCurrentViewPage() {
    if (!m_pFrame) return 0;

    if (m_iGUISelected == BOINC_SIMPLEGUI) {
        if (((CSimpleFrame*)m_pFrame)->isMessagesDlgOpen()) {
            return VW_SGUI | VW_SMSG;
        } else {
            return VW_SGUI;
        }
    }
    
    switch (((CAdvancedFrame*)m_pFrame)->GetViewTabIndex()) {
    case 0:
        return VW_PROJ;
    case 1:
        return VW_TASK;
    case 2:
        return VW_XFER;
    case 3:
        return VW_MSGS;
    case 4:
        return VW_STAT;
    case 5:
        return VW_DISK;
    }
    
    return 0;       // Should never happen.
}


int CBOINCGUIApp::ConfirmExit() {
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString strTitle;

    wxASSERT(pDoc);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));
    
    if (! (pDoc->m_pClientManager->WasBOINCStartedByManager()))
        return 1;   // Don't run dialog if exiting manager won't shut down Client or its tasks
        
    if (!m_iDisplayExitWarning)
        return 1;

#ifdef __WXMAC__
    // Don't run confirmation dialog if logging out or shutting down
    if (s_bSkipExitConfirmation)
        return 1;

    ProcessSerialNumber psn;

    GetCurrentProcess(&psn);
    bool wasVisible = IsProcessVisible(&psn);
    SetFrontProcess(&psn);  // Shows process if hidden
#endif

    strTitle.Printf(
        _("%s - Exit Confirmation"), 
        pSkinAdvanced->GetApplicationName().c_str()
    );

    CDlgGenericMessage dlg(NULL);

    dlg.SetTitle(strTitle);
    dlg.m_DialogMessage->SetLabel(pSkinAdvanced->GetExitMessage());
    dlg.Fit();
    dlg.Centre();

    if (wxID_OK == dlg.ShowModal()) {
        if (dlg.m_DialogDisableMessage->GetValue()) {
            m_iDisplayExitWarning = 0;
        }
        s_bSkipExitConfirmation = true;     // Don't ask twice (only affects Mac)
        return 1;
    }
#ifdef __WXMAC__
    if (!wasVisible) {
        ShowHideProcess(&psn, false);
    }
#endif
    return 0;       // User cancelled exit
}


// Use this instead of wxMessageBox from all tab Views to suppress 
// Periodic RPCs.  See comment in CMainDocument::RunPeriodicRPCs()
// for a fuller explanation.
int CBOINCGUIApp::SafeMessageBox(const wxString& message, const wxString& caption, long style,
                 wxWindow *parent, int x, int y )
{
    int retval;
    
    m_bSafeMessageBoxDisplayed++;
    
    retval = wxMessageBox(message, caption, style, parent, x, y);

    m_bSafeMessageBoxDisplayed--;

    return retval;
}


bool CBOINCGUIApp::IsModalDialogDisplayed() {
    if (m_bSafeMessageBoxDisplayed) return true;
    
    // Search for the dialog by ID since all of BOINC Manager's 
    // dialog IDs are 10000.
    if (wxDynamicCast(wxWindow::FindWindowById(ID_ANYDIALOG), wxDialog)) {
        return true;
    }
    
    if (m_pDocument) {
        if (m_pDocument->WaitingForRPC()) {
            return true;
        }
    }
    return false;
}


// Prevent recursive entry of CMainDocument::RequestRPC()
int CBOINCGUIApp::FilterEvent(wxEvent &event) {
    if (!m_pDocument) return -1;
    if (!m_pDocument->WaitingForRPC()) return -1;

    // If in RPC Please Wait dialog, reject all events except 
    // RPC Finished or those for that dialog or its children.
    if (event.GetEventType() == wxEVT_RPC_FINISHED) return -1;

    wxDialog* theRPCWaitDialog = m_pDocument->GetRPCWaitDialog();
    wxObject * theObject = event.GetEventObject();
    while (theObject) {
        if (! theObject->IsKindOf(CLASSINFO(wxWindow))) break;
        if (theObject == theRPCWaitDialog) return -1;
        theObject = ((wxWindow*)theObject)->GetParent();
    }
    
    return false;
}

const char *BOINC_RCSID_487cbf3018 = "$Id$";

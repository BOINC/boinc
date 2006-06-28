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
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "network.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "AdvancedFrame.h"

#ifdef SIMPLEGUI
#include "common/wxAnimate.h"
#include "common/wxFlatNotebook.h"
#include "sg_ImageLoader.h"
#include "sg_StatImageLoader.h"
#include "sg_BoincSimpleGUI.h"
#endif


////@begin XPM images
#ifdef __APPLE__
#include "res/boinc_mac.xpm"
#include "res/gridrepublic_mac.xpm"
#else
#include "res/boinc.xpm"
#include "res/gridrepublic16.xpm"
#include "res/gridrepublic32.xpm"
#include "res/cpdnbbc16.xpm"
#include "res/cpdnbbc32.xpm"
#include "res/cpdnbbcapwizard.xpm"
#include "res/seed16.xpm"
#include "res/seed32.xpm"
#include "res/seedamwizard.xpm"
#include "res/seedapwizard.xpm"
#endif
#include "res/boincsm.xpm"
#include "res/gridrepublicamwizard.xpm"
////@end XPM images


#ifdef __WXMSW__
EXTERN_C BOOL  IsBOINCServiceInstalled();
EXTERN_C BOOL  IsBOINCServiceStarting();
EXTERN_C BOOL  IsBOINCServiceRunning();
EXTERN_C BOOL  IsBOINCServiceStopping();
EXTERN_C BOOL  IsBOINCServiceStopped();
EXTERN_C BOOL  StartBOINCService();
EXTERN_C BOOL  StopBOINCService();
EXTERN_C BOOL  ClientLibraryStartup();
EXTERN_C void  ClientLibraryShutdown();
EXTERN_C int   BOINCIsNetworkAlive(long* lpdwFlags);
EXTERN_C int   BOINCIsNetworkAlwaysOnline();
EXTERN_C DWORD BOINCGetIdleTickCount();
#endif

IMPLEMENT_APP(CBOINCGUIApp)
IMPLEMENT_DYNAMIC_CLASS(CBOINCGUIApp, wxApp)


bool CBrandingScheme::OnInit( wxConfigBase *pConfig ) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxInt32     iBrandId = 0;

    wxASSERT(pConfig);

#ifdef __WXMAC__
    wxChar buf[1024];
    ProcessSerialNumber ourPSN;
    FSRef ourFSRef;
    OSErr err;

    iBrandId = 0;   // Default value
    // Get the full path to core client inside this application's bundle
    err = GetCurrentProcess (&ourPSN);
    if (err == noErr) {
        err = GetProcessBundleLocation(&ourPSN, &ourFSRef);
    }
    if (err == noErr) {
        err = FSRefMakePath (&ourFSRef, (UInt8*)buf, sizeof(buf));
    }
    if (err == noErr) {
        strcat(buf, "/Contents/Resources/Branding");
        FILE *f = fopen(buf, "r");
        if (f) {
            fscanf(f, "BrandId=%d\n", &iBrandId);
            fclose(f);
        }
    }
#else
    strBaseConfigLocation = pConfig->GetPath();
    pConfig->SetPath(strBaseConfigLocation + wxT("Branding"));
    pConfig->Read(wxT("BrandId"), &iBrandId, 0);
    pConfig->SetPath(strBaseConfigLocation);
#endif

    // If the BrandId is greater than 0 then we are running in
    //   branded mode
    if (iBrandId) {
        m_bIsBranded = true;
    } else {
        m_bIsBranded = false;  // wxWidgets automatically sets bools to
                               //   true by default.
    }

    // Now determine which resources are needed for each BrandId
    switch (iBrandId) {
        case 1:
            // Running as a GridRepublic client.
            m_strApplicationName = wxT("GridRepublic Desktop");
#ifdef __APPLE__
            m_iconApplicationIcon = wxIcon(gridrepublic_xpm);
            m_bitmapApplicationLogo = wxBitmap(gridrepublic_xpm);
#else
            m_iconApplicationIcon = wxIcon(gridrepublic16_xpm);
            m_bitmapApplicationLogo = wxBitmap(gridrepublic32_xpm);
#endif
            m_strCompanyName = wxT("GridRepublic");
            m_strCompanyWebsite = wxT("http://www.gridrepublic.org/");
            m_strProjectName = wxT("GridRepublic");
            m_strAPWizardTitle = wxEmptyString;
            m_bDefaultTabSpecified = false;
            m_iDefaultTab = 0;
            m_bitmapAPWizardLogo = wxBitmap(gridrepublicamwizard_xpm);
            m_strAPWizardAccountInfoText = wxEmptyString;
            m_strAPWizardCompletionTitle = wxEmptyString;
            m_strAPWizardCompletionBrandedMessage = wxEmptyString;
            m_strAPWizardCompletionMessage = wxEmptyString;
            m_strAMWizardTitle = wxEmptyString;
            m_bitmapAMWizardLogo = wxBitmap(gridrepublicamwizard_xpm);
            m_strAMWizardAccountInfoText = 
                _("Please provide the email address and password you used to register\n"
                  "at GridRepublic. (if you have not yet created an account at\n"
                  "GridRepublic, please do so at http://www.gridrepublic.org)");
            m_strAMWizardAttachMessage = wxEmptyString;
            m_strExitMessage = wxEmptyString;
            break;
#ifndef __APPLE__
        case 2:
            // Running as a CPDNBBC client.
            m_strApplicationName = wxT("The BOINC Manager for the BBC Climate Change Experiment");
            m_iconApplicationIcon = wxIcon(cpdnbbc16_xpm);
            m_bitmapApplicationLogo = wxBitmap(cpdnbbc32_xpm);
            m_strCompanyName = wxT("ClimatePrediction.net");
            m_strCompanyWebsite = wxT("http://bbc.cpdn.org/");
            m_strProjectName = wxT("BBC Climate Change Experiment");
            m_bDefaultTabSpecified = true;
            m_iDefaultTab = ID_LIST_WORKVIEW - ID_LIST_BASE;
            m_strAPWizardTitle = _("Start Project");
            m_bitmapAPWizardLogo = wxBitmap(cpdnbbcapwizard_xpm);
            m_strAPWizardAccountInfoText = 
                _("Please enter an email address and password. You will need your email\n"
                  "address if you want to change your account options or use our message\n"
                  "boards.\n"
                  "\n"
                  "We will send you occasional emails. You can stop these at any time.\n"
                  "We will not pass your email address on to others.");
            m_strAPWizardCompletionTitle = 
                _("Project Started");
            m_strAPWizardCompletionBrandedMessage =
                _("Congratulations, you have now successfully started your Climate\n"
                  "Change Experiment.");
            m_strAPWizardCompletionMessage =
                _("Click finish to exit. You will be taken to a web page which tells\n"
                  "you more about your model.");
            m_strAMWizardTitle = wxEmptyString;
            m_bitmapAMWizardLogo = wxBitmap(cpdnbbcapwizard_xpm);
            m_strAMWizardAccountInfoText = wxEmptyString;
            m_strAMWizardAttachMessage = wxEmptyString;
            m_strExitMessage = 
                _("This will shut down your experiment until it restarts automatically\n"
                  "following your user preferences. Close window to close the manager\n"
                  "without stopping the experiment.");
            break;
        case 3:
            // Running as a Seed client.
            m_strApplicationName = wxT("seed@home Client");
            m_iconApplicationIcon = wxIcon(seed16_xpm);
            m_bitmapApplicationLogo = wxBitmap(seed32_xpm);
            m_strCompanyName = wxT("Seed");
            m_strCompanyWebsite = wxT("http://www.seedmagazine.com/");
            m_strProjectName = wxT("seed@home");
            m_bDefaultTabSpecified = false;
            m_iDefaultTab = 0;
            m_strAPWizardTitle = wxEmptyString;
            m_bitmapAPWizardLogo = wxBitmap(seedapwizard_xpm);
            m_strAPWizardCompletionTitle = wxEmptyString;
            m_strAPWizardCompletionBrandedMessage = wxEmptyString;
            m_strAPWizardCompletionMessage = wxEmptyString;
            m_strAMWizardTitle = wxEmptyString;
            m_strAPWizardAccountInfoText = wxEmptyString;
            m_bitmapAMWizardLogo = wxBitmap(seedamwizard_xpm);
            m_strAMWizardAccountInfoText = wxEmptyString;
            m_strAMWizardAttachMessage = wxEmptyString;
            m_strExitMessage = wxEmptyString;
            break;
#endif  // __APPLE__
        default:
            // Running in native mode without any branding
            m_strApplicationName = wxT("BOINC Manager");
            m_iconApplicationIcon = wxIcon(boinc_xpm);
            m_bitmapApplicationLogo = wxBitmap(boincsm_xpm);
            m_strCompanyName = wxT("Space Sciences Laboratory, U.C. Berkeley");
            m_strCompanyWebsite = wxT("http://boinc.berkeley.edu/");
            m_strProjectName = wxT("BOINC");
            m_bDefaultTabSpecified = false;
            m_iDefaultTab = 0;
            m_strAPWizardTitle = wxEmptyString;
            m_bitmapAPWizardLogo = wxNullBitmap;
            m_strAPWizardCompletionTitle = wxEmptyString;
            m_strAPWizardCompletionBrandedMessage = wxEmptyString;
            m_strAPWizardCompletionMessage = wxEmptyString;
            m_strAMWizardTitle = wxEmptyString;
            m_strAPWizardAccountInfoText = wxEmptyString;
            m_bitmapAMWizardLogo = wxNullBitmap;
            m_strAMWizardAccountInfoText = wxEmptyString;
            m_strAMWizardAttachMessage = wxEmptyString;
            m_strExitMessage = wxEmptyString;
            break;
    }

    return true;
}


bool CBOINCGUIApp::OnInit() {

#if (defined(SANDBOX) && !defined(_WIN32))
    umask (2);  // Set file creation mask to be writable by both user and group
                // Our umask will be inherited by all our child processes
#endif

    // Setup variables with default values
    m_bBOINCStartedByManager = false;
    m_pFrame = NULL;
    m_bGUIVisible = true;
#ifdef SIMPLEGUI
    m_iGUISelected = BOINC_SIMPLEGUI;
#else
    m_iGUISelected = BOINC_ADVANCEDGUI;
#endif
    m_lBOINCCoreProcessId = 0;
#ifdef __WXMSW__
    m_hBOINCCoreProcess = NULL;
    m_hClientLibraryDll = NULL;
#endif
    m_strDefaultWindowStation = wxT("");
    m_strDefaultDesktop = wxT("");
    m_strDefaultDisplay = wxT("");

    // Setup application and company information
    SetAppName(wxT("BOINC Manager"));
    SetVendorName(wxT("Space Sciences Laboratory, U.C. Berkeley"));


    // Initialize the configuration storage module
    m_pConfig = new wxConfig(GetAppName());
    wxConfigBase::Set(m_pConfig);
    wxASSERT(m_pConfig);

    m_pConfig->SetPath(wxT("/"));

#ifdef __WXMSW__

    TCHAR   szPath[MAX_PATH-1];

    // change the current directory to the boinc install directory
    GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR)));
		
    TCHAR *pszProg = strrchr(szPath, '\\');
    if (pszProg) {
        szPath[pszProg - szPath + 1] = 0;
        SetCurrentDirectory(szPath);
    }

#endif

    // Setup the branding scheme
    // NOTE: SetAppName(), SetVendorName(), and the creation of the m_pConfig
    //   variable need to be done before the branding object can be used on
    //   any platform other than the Mac.
    m_pBranding = new CBrandingScheme;
    wxASSERT(m_pBranding);

    m_pBranding->OnInit(m_pConfig);

#ifdef __WXMAC__

    wxString strDirectory = wxEmptyString;
    bool success;

    // Set the current directory ahead of the application launch so the core
    //   client can find its files
#if 0       // Code for separate data in each user's private directory
    wxChar buf[1024];
    wxExpandPath(buf, "~/Library/Application Support");
    strDirectory = wxT(buf);
#else       // All users share the same data
    // The mac installer sets the "setuid & setgid" bits for the 
    // BOINC Manager and core client so any user can run them and 
    // they can operate on shared data.
    strDirectory = wxT("/Library/Application Support/");
#endif

    success = ::wxSetWorkingDirectory(strDirectory);
    if (success) {
        // If SetWD failed, don't create a directory in wrong place
        strDirectory += wxT("BOINC Data");  // We don't customize BOINC Data directory name for branding
        if (! wxDirExists(strDirectory)) {
#ifdef SANDBOX
            // Create BOINC Data directory writable by group boinc_master only
            success = wxMkdir(strDirectory, 0575);
#ifndef _DEBUG
            gid_t gid;
            lookup_group("boinc_master", gid);
            boinc_chown("BOINC Data", gid);
#endif      // ! _DEBUG
#else       // SANDBOX
            success = wxMkdir(strDirectory, 0777);    // Does nothing if dir exists
#endif      // ! SANDBOX
        }
        success = ::wxSetWorkingDirectory(strDirectory);
//    wxChar *wd = wxGetWorkingDirectory(buf, 1000);  // For debugging
    }

#ifdef SANDBOX
#ifdef _DEBUG
            // GDB can't attach to applications which are running as a diferent user   
            //  or group, so fix up data with current user and group during debugging
            if (check_security()) {
                CreateBOINCUsersAndGroups();
                SetBOINCDataOwnersGroupsAndPermissions();
                SetBOINCAppOwnersGroupsAndPermissions(NULL);
            }
#endif  // _DEBUG

    int i = check_security();
    if (i) {
        wxString            strDialogMessage = wxEmptyString;
        strDialogMessage.Printf(
            _("BOINC ownership or permissions are not set properly; please reinstall BOINC.\n(Error code %d)"), i);
        wxMessageDialog* pDlg = 
        new wxMessageDialog(m_pFrame, strDialogMessage, wxT(""), wxOK);
        pDlg->ShowModal();
        if (pDlg)
            pDlg->Destroy();
        return false;
    }
#endif      // SANDBOX
#endif      // __WXMAC__

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

    // Initialize the internationalization module
    m_pLocale = new wxLocale();
    wxASSERT(m_pLocale);

    wxInt32 iSelectedLanguage = m_pConfig->Read(wxT("Language"), 0L);

    // Locale information is stored relative to the executable.
    m_pLocale->Init(iSelectedLanguage);
    m_pLocale->AddCatalogLookupPathPrefix(wxT("locale"));
    m_pLocale->AddCatalog(wxT("BOINC Manager"));

    InitSupportedLanguages();

    // Note: JAWS for Windows will only speak the context-sensitive
    // help if you use this help provider:
    wxHelpProvider::Set(new wxHelpControllerHelpProvider());

    // Commandline parsing is done in wxApp::OnInit()
    if (!wxApp::OnInit()) {
        return false;
    }

    // Initialize the main document
    m_pDocument = new CMainDocument();
    wxASSERT(m_pDocument);

    m_pDocument->OnInit();

#ifdef SIMPLEGUI
    // Which GUI should be displayed?
    m_iGUISelected = m_pConfig->Read(wxT("GUISelection"), BOINC_SIMPLEGUI);
#endif

    // Initialize the task bar icon
    m_pTaskBarIcon = new CTaskBarIcon(
        m_pBranding->GetApplicationName(), 
        m_pBranding->GetApplicationIcon()
    );
    wxASSERT(m_pTaskBarIcon);
#ifdef __WXMAC__
    m_pMacSystemMenu = new CMacSystemMenu(
        m_pBranding->GetApplicationName(), 
        m_pBranding->GetApplicationIcon()
    );
    wxASSERT(m_pMacSystemMenu);
#endif

    // Detect the display info and store for later use.
    DetectDisplayInfo();

    // Startup the System Idle Detection code
    ClientLibraryStartup();

    // Detect if we need to start the BOINC Core Client due to configuration
    StartupBOINCCore();

#ifdef __WXMAC__
    ProcessSerialNumber psn;
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
        m_bFrameVisible = false;

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
    // Detect if we need to stop the BOINC Core Client due to configuration
    ShutdownBOINCCore();

    // Shutdown the System Idle Detection code
    ClientLibraryShutdown();

    if (m_pTaskBarIcon) {
        delete m_pTaskBarIcon;
    }
#ifdef __WXMAC__
    if (m_pMacSystemMenu) {
        delete m_pMacSystemMenu;
    }
#endif

    if (m_pDocument) {
        m_pDocument->OnExit();
        delete m_pDocument;
    }

    if (m_pBranding) {
        delete m_pBranding;
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
        { wxCMD_LINE_NONE}  //DON'T forget this line!!
    };
    parser.SetDesc(cmdLineDesc);
}


bool CBOINCGUIApp::OnCmdLineParsed(wxCmdLineParser &parser) {
    // Give default processing (-?, --help and --verbose) the chance to do something.
    wxApp::OnCmdLineParsed(parser);
    if (parser.Found(wxT("systray"))) {
        m_bGUIVisible = false;
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


bool CBOINCGUIApp::IsBOINCCoreRunning() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCGUIApp::IsBOINCCoreRunning - Function Begin"));

    int retval;
    bool running;
    RPC_CLIENT rpc;

#ifdef __WXMSW__

    if (IsBOINCServiceInstalled()) {
        running = (FALSE != IsBOINCServiceStarting()) || (FALSE != IsBOINCServiceRunning());
    } else {
        retval = rpc.init("localhost");  // synchronous is OK since local
        running = (retval == 0);
        rpc.close();
    }
    
#else
    retval = rpc.init("localhost");  // synchronous is OK since local
    running = (retval == 0);
    rpc.close();
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCGUIApp::IsBOINCCoreRunning - Function End"));
    return running;
}


void CBOINCGUIApp::StartupBOINCCore() {
    if (!IsBOINCCoreRunning()) {
#ifndef __WXMAC__
        wxString strDirectory = wxEmptyString;
#endif  // ! __WXMAC__

        wxString strExecute = wxEmptyString;
        wxChar   szExecutableDirectory[4096];

        memset(szExecutableDirectory, 0, sizeof(szExecutableDirectory));

#ifdef __WXMSW__

        // On the surface it would seem that GetCurrentDirectory would be a better choice
        //   for determing which directory we should prepend to the execution string before
        //   starting BOINC, except that we cannot depend on any shortcuts being configured
        //   to startup in the correct directory, since the user may have created the
        //   shortcut themselves.  So determine where boinc.exe is based off of our
        //   current execution location and then execute it.
        GetModuleFileName(
            NULL, 
            szExecutableDirectory,
            (sizeof(szExecutableDirectory) / sizeof(wxChar))
        );

#endif

#ifdef __WXMAC__

        {
            wxChar buf[1024];
            wxChar *argv[3];
            ProcessSerialNumber ourPSN;
            FSRef ourFSRef;
            OSErr err;

            // Get the full path to core client inside this application's bundle
            err = GetCurrentProcess (&ourPSN);
            if (err == noErr) {
                err = GetProcessBundleLocation(&ourPSN, &ourFSRef);
            }
            if (err == noErr) {
                err = FSRefMakePath (&ourFSRef, (UInt8*)buf, sizeof(buf));
            }
            if (err == noErr) {
#if 0   // The Mac version of wxExecute(wxString& ...) crashes if there is a space in the path
                strExecute = wxT("\"");            
                strExecute += wxT(buf);
                strExecute += wxT("/Contents/Resources/boinc\" -redirectio");
                m_lBOINCCoreProcessId = ::wxExecute(strExecute);
#else   // Use wxExecute(wxChar **argv ...) instead of wxExecute(wxString& ...)
                strcat(buf, "/Contents/Resources/boinc");
                argv[0] = buf;
                argv[1] = "-redirectio";
                argv[2] = NULL;
                m_lBOINCCoreProcessId = ::wxExecute(argv);
#endif
            } else {
                buf[0] = '\0';
            }
        }

#else   // ! __WXMAC__

#ifndef __WXMSW__
        // copy the path to the boinmgr from argv[0]
        strncpy((char*)szExecutableDirectory, (const char*)wxGetApp().argv[0], sizeof(szExecutableDirectory));
#endif 

        // We are only interested in the path component of the fully qualified path.
        wxFileName::SplitPath(szExecutableDirectory, &strDirectory, NULL, NULL);

        // Set the current directory ahead of the application launch so the core
        //   client can find its files
        ::wxSetWorkingDirectory(strDirectory);

#endif  // ! __WXMAC__

#ifdef __WXMSW__

        // Append boinc.exe to the end of the strExecute string and get ready to rock
        strExecute = wxT("\"") + strDirectory + wxT("\\boinc.exe\" -redirectio");

        PROCESS_INFORMATION pi;
        STARTUPINFO         si;
        BOOL                bProcessStarted;

        memset(&pi, 0, sizeof(pi));
        memset(&si, 0, sizeof(si));
 
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        bProcessStarted = CreateProcess(
            NULL,
            (LPTSTR)strExecute.c_str(),
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW,
            NULL,
            (LPTSTR)strDirectory.c_str(),
            &si,
            &pi
        );
        if (bProcessStarted) {
            m_lBOINCCoreProcessId = pi.dwProcessId;
            m_hBOINCCoreProcess = pi.hProcess;
        }

#else

#ifndef __WXMAC__

        // Append boinc.exe to the end of the strExecute string and get ready to rock
        strExecute = wxT("./boinc -redirectio");
        m_lBOINCCoreProcessId = ::wxExecute(strExecute);
        
#endif  // ! __WXMAC__

#endif  // ! __WXMSW__

        if (0 != m_lBOINCCoreProcessId) {
            m_bBOINCStartedByManager = true;
        }
    }
}


#if defined(__WXMSW__)

void CBOINCGUIApp::ShutdownBOINCCore() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCGUIApp::ShutdownBOINCCore - Function Begin"));

    wxInt32    iCount = 0;
    DWORD      dwExitCode = 0;
    bool       bClientQuit = false;
    wxString   strConnectedCompter = wxEmptyString;
    wxString   strPassword = wxEmptyString;

    if (m_bBOINCStartedByManager) {
        m_pDocument->GetConnectedComputerName(strConnectedCompter);
        if (!m_pDocument->IsComputerNameLocal(strConnectedCompter)) {
            RPC_CLIENT rpc;
            if (!rpc.init("localhost")) {
                m_pDocument->m_pNetworkConnection->GetLocalPassword(strPassword);
                rpc.authorize((const char*)strPassword.mb_str());
                if (GetExitCodeProcess(m_hBOINCCoreProcess, &dwExitCode)) {
                    if (STILL_ACTIVE == dwExitCode) {
                        rpc.quit();
                        for (iCount = 0; iCount <= 10; iCount++) {
                            if (!bClientQuit && GetExitCodeProcess(m_hBOINCCoreProcess, &dwExitCode)) {
                                if (STILL_ACTIVE != dwExitCode) {
                                    wxLogTrace(wxT("Function Status"), wxT("CBOINCGUIApp::ShutdownBOINCCore - (localhost) Application Exit Detected"));
                                    bClientQuit = true;
                                    break;
                                }
                            }
                            wxLogTrace(wxT("Function Status"), wxT("CBOINCGUIApp::ShutdownBOINCCore - (localhost) Application Exit NOT Detected, Sleeping..."));
                            ::wxSleep(1);
                        }
                    }
                }
            }
            rpc.close();
        } else {
            if (GetExitCodeProcess(m_hBOINCCoreProcess, &dwExitCode)) {
                if (STILL_ACTIVE == dwExitCode) {
                    m_pDocument->CoreClientQuit();
                    for (iCount = 0; iCount <= 10; iCount++) {
                        if (!bClientQuit && GetExitCodeProcess(m_hBOINCCoreProcess, &dwExitCode)) {
                            if (STILL_ACTIVE != dwExitCode) {
                                wxLogTrace(wxT("Function Status"), wxT("CBOINCGUIApp::ShutdownBOINCCore - Application Exit Detected"));
                                bClientQuit = true;
                                break;
                            }
                        }
                        wxLogTrace(wxT("Function Status"), wxT("CBOINCGUIApp::ShutdownBOINCCore - Application Exit NOT Detected, Sleeping..."));
                        ::wxSleep(1);
                    }
                }
            }
        }

        if (!bClientQuit) {
            ::wxKill(m_lBOINCCoreProcessId);
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCGUIApp::ShutdownBOINCCore - Function End"));
}

#elif defined(__WXMAC__)

static char * PersistentFGets(char *buf, size_t buflen, FILE *f) {
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

bool CBOINCGUIApp::ProcessExists(pid_t thePID)
{
    FILE *f;
    char buf[256];
    pid_t aPID;

    f = popen("ps -a -x -c -o pid,state", "r");
    if (f == NULL)
        return false;
    
    while (PersistentFGets(buf, sizeof(buf), f)) {
        aPID = atol(buf);
        if (aPID == thePID) {
            if (strchr(buf, 'Z'))   // A 'zombie', stopped but waiting
                break;              // for us (its parent) to quit
            pclose(f);
            return true;
        }
    }
    pclose(f);
    return false;
}

// wxProcess::Exists and wxKill are unimplemented in WxMac-2.6.0
void CBOINCGUIApp::ShutdownBOINCCore() {
    wxInt32    iCount = 0;
    wxString   strConnectedCompter = wxEmptyString;
    wxString   strPassword = wxEmptyString;

    if (m_bBOINCStartedByManager) {
        m_pDocument->GetConnectedComputerName(strConnectedCompter);
        if (!m_pDocument->IsComputerNameLocal(strConnectedCompter)) {
            RPC_CLIENT rpc;
            if (!rpc.init("localhost")) {
                m_pDocument->m_pNetworkConnection->GetLocalPassword(strPassword);
                rpc.authorize((const char*)strPassword.mb_str());
                if (ProcessExists(m_lBOINCCoreProcessId)) {
                    rpc.quit();
                    for (iCount = 0; iCount <= 10; iCount++) {
                        if (!ProcessExists(m_lBOINCCoreProcessId))
                            return;
                        ::wxSleep(1);
                    }
                }
            }
            rpc.close();
        } else {
            if (ProcessExists(m_lBOINCCoreProcessId)) {
                m_pDocument->CoreClientQuit();
                for (iCount = 0; iCount <= 10; iCount++) {
                    if (!ProcessExists(m_lBOINCCoreProcessId))
                        return;

                    ::wxSleep(1);
                }
            }
        }
        
        // Client did not quit after 10 seconds so kill it
        kill(m_lBOINCCoreProcessId, SIGKILL);
    }
}

#else

void CBOINCGUIApp::ShutdownBOINCCore() {
    wxInt32    iCount = 0;
    bool       bClientQuit = false;
    wxString   strConnectedCompter = wxEmptyString;
    wxString   strPassword = wxEmptyString;

    if (m_bBOINCStartedByManager) {
        m_pDocument->GetConnectedComputerName(strConnectedCompter);
        if (!m_pDocument->IsComputerNameLocal(strConnectedCompter)) {
            RPC_CLIENT rpc;
            if (!rpc.init("localhost")) {
                m_pDocument->m_pNetworkConnection->GetLocalPassword(strPassword);
                rpc.authorize((const char*)strPassword.mb_str());
                if (wxProcess::Exists(m_lBOINCCoreProcessId)) {
                    rpc.quit();
                    for (iCount = 0; iCount <= 10; iCount++) {
                        if (!bClientQuit && !wxProcess::Exists(m_lBOINCCoreProcessId)) {
                            bClientQuit = true;
                            break;
                        }
                        ::wxSleep(1);
                    }
                }
            }
            rpc.close();
        } else {
            if (wxProcess::Exists(m_lBOINCCoreProcessId)) {
                m_pDocument->CoreClientQuit();
                for (iCount = 0; iCount <= 10; iCount++) {
                    if (!bClientQuit && !wxProcess::Exists(m_lBOINCCoreProcessId)) {
                        bClientQuit = true;
                        break;
                    }
                    ::wxSleep(1);
                }
            }
        }

        if (!bClientQuit) {
            ::wxKill(m_lBOINCCoreProcessId);
        }
    }
}

#endif


int CBOINCGUIApp::ClientLibraryStartup() {
#ifdef __WXMSW__
    ::ClientLibraryStartup();
#endif
    return 0;
}


int CBOINCGUIApp::ClientLibraryShutdown() {
#ifdef __WXMSW__
    ::ClientLibraryShutdown();
#endif
    return 0;
}


int CBOINCGUIApp::IsNetworkAlive(long* lpdwFlags) {
#ifdef __WXMSW__
    return BOINCIsNetworkAlive(lpdwFlags);
#endif
    return TRUE;
}


int CBOINCGUIApp::IsNetworkAlwaysOnline() {
#ifdef __WXMSW__
    return BOINCIsNetworkAlwaysOnline();
#endif
    return TRUE;
}


int CBOINCGUIApp::UpdateSystemIdleDetection() {
#ifdef __WXMSW__
    return BOINCGetIdleTickCount();
#endif
    return 0;
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


bool CBOINCGUIApp::SetActiveGUI(int iGUISelection, bool bShowWindow) {
#ifdef SIMPLEGUI
    CBOINCBaseFrame* pNewFrame = NULL;

    // Create the new window
    if ((iGUISelection != m_iGUISelected) || !pNewFrame) {
        switch(iGUISelection) {
            case BOINC_SIMPLEGUI:
                // Initialize the simple gui window
                pNewFrame = new CSimpleFrame(
                    m_pBranding->GetApplicationName(), 
                    m_pBranding->GetApplicationIcon()
                );
                break;
            case BOINC_ADVANCEDGUI:
            default:
                // Initialize the advanced gui window
                pNewFrame = new CAdvancedFrame(
                    m_pBranding->GetApplicationName(), 
                    m_pBranding->GetApplicationIcon()
                );
                break;
        }
        wxASSERT(pNewFrame);
        if (pNewFrame) {
            SetTopWindow(pNewFrame);

            // Delete the old one if it exists
            if (m_pFrame) m_pFrame->Destroy();

            // Store the new frame for future use
            m_pFrame = pNewFrame;
        }
    }
#else
    if (!m_pFrame) {
        // Initialize the advanced gui window
        iGUISelection = BOINC_ADVANCEDGUI;
        m_pFrame = new CAdvancedFrame(
            m_pBranding->GetApplicationName(), 
            m_pBranding->GetApplicationIcon()
        );
        wxASSERT(m_pFrame);
        SetTopWindow(m_pFrame);
    }
#endif

    // Show the new frame if needed 
    if (m_pFrame && bShowWindow) m_pFrame->Show();

    m_iGUISelected = iGUISelection;
    m_pConfig->Write(wxT("GUISelection"), iGUISelection);

    return true;
}


const char *BOINC_RCSID_487cbf3018 = "$Id$";

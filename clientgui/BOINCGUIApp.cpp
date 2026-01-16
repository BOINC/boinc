// This file is part of BOINC.
// https://boinc.berkeley.edu
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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCGUIApp.h"
#endif

#ifdef __WXMAC__
#include <Carbon/Carbon.h>
#include <wx/uilocale.h>
#include "filesys.h"
#include "util.h"
#include "mac_util.h"
#include "sandbox.h"
#include "mac_branding.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "network.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "idlemon.h"

#include "Events.h"
#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCClientManager.h"
#include "BOINCTaskBar.h"
#include "BOINCBaseFrame.h"
#include "AdvancedFrame.h"
#include "DlgExitMessage.h"
#include "DlgEventLog.h"
#include "procinfo.h"
#include "sg_BoincSimpleFrame.h"
#include "DlgGenericMessage.h"


bool s_bSkipExitConfirmation = false;


DEFINE_EVENT_TYPE(wxEVT_RPC_FINISHED)

IMPLEMENT_APP(CBOINCGUIApp)
IMPLEMENT_DYNAMIC_CLASS(CBOINCGUIApp, wxApp)

BEGIN_EVENT_TABLE (CBOINCGUIApp, wxApp)
    EVT_ACTIVATE_APP(CBOINCGUIApp::OnActivateApp)
    EVT_RPC_FINISHED(CBOINCGUIApp::OnRPCFinished)
#ifndef __WXMAC__
    EVT_END_SESSION(CBOINCGUIApp::OnEndSession)
#endif
END_EVENT_TABLE ()

#if defined(__WXGTK__) && defined(BUILD_WITH_VCPKG)
extern "C" {
    void _gdk_pixbuf__svg_fill_info (void*);
    void _gdk_pixbuf__svg_fill_vtable (void*);
    unsigned int rsvg_error_quark (void);
    void rsvg_handle_get_pixbuf (void*);
}

typedef void (*GdkPixbufFillInfo) (void*);
typedef void (*GdkPixbufFillVtable) (void*);
typedef unsigned int (*RsvgErrorQuark) (void);
typedef void (*RsvgHandleGetPixbuf) (void*);
#endif

bool CBOINCGUIApp::OnInit() {
#if defined(__WXGTK__) && defined(BUILD_WITH_VCPKG)
    try {
        GdkPixbufFillInfo fi = _gdk_pixbuf__svg_fill_info;
        GdkPixbufFillVtable fv = _gdk_pixbuf__svg_fill_vtable;
        RsvgErrorQuark eq = rsvg_error_quark;
        RsvgHandleGetPixbuf hp = rsvg_handle_get_pixbuf;
        fi(NULL);
        fv(NULL);
        eq();
        hp(NULL);
    } catch (...) {}
#endif
    // Initialize globals
#ifdef SANDBOX
    g_use_sandbox = true;
#else
    g_use_sandbox = false;
#endif

    m_isDarkMode = false;
#if SUPPORTDARKMODE
    wxSystemAppearance appearance = wxSystemSettings::GetAppearance();
    m_isDarkMode = appearance.IsDark();
#endif

    s_bSkipExitConfirmation = false;
    m_bFilterEvents = false;
    m_bAboutDialogIsOpen = false;

    // Initialize class variables
    m_pInstanceChecker = NULL;
    m_pLocale = NULL;
    m_pSkinManager = NULL;
    m_pFrame = NULL;
    m_pDocument = NULL;
#ifndef __WXGTK__
    m_pTaskBarIcon = NULL;
#endif
    m_pEventLog = NULL;
    m_bEventLogWasActive = false;
    m_bProcessingActivateAppEvent = false;
#ifdef __WXMAC__
    m_pMacDockIcon = NULL;
#endif
    m_strBOINCMGRExecutableName = wxEmptyString;
    m_strBOINCMGRRootDirectory = wxEmptyString;
    m_strBOINCMGRDataDirectory = wxEmptyString;
    m_strHostNameArg = wxEmptyString;
    m_strPasswordArg = wxEmptyString;
    m_iRPCPortArg = GUI_RPC_PORT;
    m_strBOINCArguments = wxEmptyString;
    m_strISOLanguageCode = wxEmptyString;
    m_bUseDefaultLocale = true;
    m_bGUIVisible = true;
    m_bDebugSkins = false;
    m_bMultipleInstancesOK = false;
    m_bHostnamePasswordSet = false;
    m_bBOINCMGRAutoStarted = false;
    m_iBOINCMGRDisableAutoStart = 0;
    m_iShutdownCoreClient = 0;
    m_iDisplayExitDialog = 1;
    m_iDisplayShutdownConnectedClientDialog = 1;
    m_iDisplayAnotherInstanceRunningDialog = 1;
#ifdef __WXMAC__
    m_iHideMenuBarIcon = 0;
    m_iWasShutDownBySystemWhileHidden = 0;
#endif
    m_iGUISelected = BOINC_SIMPLEGUI;
    m_bSafeMessageBoxDisplayed = 0;
    m_bRunDaemon = true;
    m_bNeedRunDaemon = true;

    // Initialize local variables
    int      iDesiredLanguageCode = wxLANGUAGE_DEFAULT;
    bool     bOpenEventLog = false;
    wxString strDesiredSkinName = wxEmptyString;
#ifdef SANDBOX
    int      iErrorCode = 0;
    wxString strDialogMessage = wxEmptyString;
#endif
    bool     success = false;


    // Configure wxWidgets platform specific code
#ifdef __WXMSW__
    wxSystemOptions::SetOption(wxT("msw.staticbox.optimized-paint"), 0);
#endif
#ifdef __WXMAC__
    // In wxMac-2.8.7, default wxListCtrl::RefreshItem() does not work
    // so use traditional generic implementation.
    // This has been fixed in wxMac-2.8.8, but the Mac native implementation:
    //  - takes 3 times the CPU time as the Mac generic version.
    //  - seems to always redraw entire control even if asked to refresh only one row.
    //  - causes major flicker of progress bars, (probably due to full redraws.)
    wxSystemOptions::SetOption(wxT("mac.listctrl.always_use_generic"), 1);

    AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
#endif

    // Commandline parsing is done in wxApp::OnInit()
    if (!wxApp::OnInit()) {
        return false;
    }

#ifdef __WXMAC__
    // Don't open main window if we were started automatically at login
    if (compareOSVersionTo(13, 0) >= 0) {
        m_bGUIVisible = !m_bBOINCMGRAutoStarted;
    } else {
        m_bGUIVisible = IsApplicationVisible();
    }
    if (getTimeSinceBoot() < 30.) {
        // If the system was just started, we usually get a "Connection
        // failed" error if we try to connect too soon, so delay a bit.
        sleep(10);
    }
#endif

    if (g_use_sandbox) {
        wxCHANGE_UMASK(2);  // Set file creation mask to be writable by both user and group
                            // Our umask will be inherited by all our child processes
    }

    // Setup application and company information
    SetAppName(wxT("BOINC Manager"));
    SetVendorName(wxT("Space Sciences Laboratory, U.C. Berkeley"));

#ifdef __WXMAC__
    char displayName[MAXPATHLEN];
    getDisplayNameForThisApp(displayName, sizeof(displayName));
    SetAppDisplayName(wxString(displayName)); // {ass the display name to wxWidgets
#endif


    // Initialize the configuration storage module
    m_pConfig = new wxConfig(GetAppName());
    wxConfigBase::Set(m_pConfig);
    wxASSERT(m_pConfig);


    // Restore Application State
    m_pConfig->SetPath(wxT("/"));
    m_pConfig->Read(wxT("AutomaticallyShutdownClient"), &m_iShutdownCoreClient, 0L);
    m_pConfig->Read(wxT("DisplayShutdownClientDialog"), &m_iDisplayExitDialog, 1L);
    m_pConfig->Read(wxT("DisplayShutdownConnectedClientDialog"), &m_iDisplayShutdownConnectedClientDialog, 1L);
    m_pConfig->Read(wxT("DisplayAnotherInstanceRunningDialog"), &m_iDisplayAnotherInstanceRunningDialog, 1L);
#ifdef __WXMAC__
    m_pConfig->Read(wxT("HideMenuBarIcon"), &m_iHideMenuBarIcon, 0L);
    m_pConfig->Read(wxT("WasShutDownBySystemWhileHidden"), &m_iWasShutDownBySystemWhileHidden, 0L);
    // If Manager was hidden and was shut down by system when user last logged
    // out, MacOS's "Reopen windows when logging in" functionality may relaunch
    // us visible before our LaunchAgent launches us with the "autostart" arg.
    // QuitAppleEventHandler() set m_iWasShutDownBySystemWhileHidden to 1, causing
    // CBOINCGUIApp::SaveState to set WasShutDownBySystemWhileHidden in our
    // configuration file to tell us to treat this as an autostart and launch hidden.
    if (m_iWasShutDownBySystemWhileHidden) {
        m_iWasShutDownBySystemWhileHidden = 0;
        m_bBOINCMGRAutoStarted = true;
        m_bGUIVisible = false;
    }
#endif
    m_pConfig->Read(wxT("DisableAutoStart"), &m_iBOINCMGRDisableAutoStart, 0L);
    m_pConfig->Read(wxT("LanguageISO"), &m_strISOLanguageCode, wxT(""));
    m_bUseDefaultLocale = false;
    bool bUseDefaultLocaleDefault = false;
#ifdef __WXMAC__
    // Because our translations don't use Apple's standard localization
    // scheme, the Cocoa APIs used by wxLocale for wxLANGUAGE_DEFAULT
    // always return English as the language for the reasons explained
    // in https://stackoverflow.com/questions/48136456. The wxLocale
    // documentation warns us to use wxUILocale::GetSystemLanguage().
    int systemLanguageCode = wxUILocale::GetSystemLanguage();
    wxString defaultLanguageCode = wxLocale::GetLanguageCanonicalName(systemLanguageCode);
    bUseDefaultLocaleDefault = m_strISOLanguageCode == defaultLanguageCode;
#else
    const wxLanguageInfo *defaultLanguageInfo = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);
    if (defaultLanguageInfo != NULL) {
        // Migration: assume a selected language code that matches the system default means "auto select"
        bUseDefaultLocaleDefault = m_strISOLanguageCode == defaultLanguageInfo->CanonicalName;;
    }
#endif
    m_pConfig->Read(wxT("UseDefaultLocale"), &m_bUseDefaultLocale, bUseDefaultLocaleDefault);
    m_pConfig->Read(wxT("GUISelection"), &m_iGUISelected, BOINC_SIMPLEGUI);
    m_pConfig->Read(wxT("EventLogOpen"), &bOpenEventLog);
    m_pConfig->Read(wxT("RunDaemon"), &m_bRunDaemon, 1L);

    // Detect if the daemon should be launched
    m_bNeedRunDaemon = m_bNeedRunDaemon && m_bRunDaemon;

    // Should we abort the BOINC Manager startup process?
    if (m_bBOINCMGRAutoStarted && m_iBOINCMGRDisableAutoStart) {
        return false;
    }

    // Detect where BOINC Manager executable name.
    DetectExecutableName();

    // Detect where BOINC Manager was installed to.
    DetectRootDirectory();

    // Detect where the BOINC Data files are.
    DetectDataDirectory();


    // Switch the current directory to the BOINC Data directory
    if (!GetDataDirectory().IsEmpty()) {
    	success = wxSetWorkingDirectory(GetDataDirectory());
        if (!success) {
            if (!g_use_sandbox) {
                if (!wxDirExists(GetDataDirectory())) {
                    success = wxMkdir(GetDataDirectory(), 0777);    // Does nothing if dir exists
                }
            }
        }
    }

#ifdef SANDBOX
    if (!success) iErrorCode = -1016;
#endif

    // Initialize the BOINC Diagnostics Framework
    int dwDiagnosticsFlags =
#ifdef _DEBUG
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_MEMORYLEAKCHECKENABLED |
#endif
        BOINC_DIAG_DUMPCALLSTACKENABLED |
        BOINC_DIAG_PERUSERLOGFILES |
        BOINC_DIAG_REDIRECTSTDERR |
        BOINC_DIAG_REDIRECTSTDOUT |
        BOINC_DIAG_TRACETOSTDOUT;

    diagnostics_init(dwDiagnosticsFlags, "stdoutgui", "stderrgui");

    // Enable Logging and Trace Masks
    m_pLog = new wxLogBOINC();
    wxLog::SetActiveTarget(m_pLog);

    m_pLog->AddTraceMask(wxT("Function Start/End"));
    m_pLog->AddTraceMask(wxT("Function Status"));


    // Initialize the internationalization module
#ifdef __WXMSW__
    // On Windows, set all locales for this thread on a per-thread basis
    _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
#endif
    m_pLocale = new wxLocale();
    wxASSERT(m_pLocale);

    //
    if (!m_bUseDefaultLocale && !m_strISOLanguageCode.IsEmpty()) {
        const wxLanguageInfo* pLI = wxLocale::FindLanguageInfo(m_strISOLanguageCode);
        if (pLI) {
            iDesiredLanguageCode = pLI->Language;
        }
    }

#ifdef __WXMAC__
    // wxLocale::Init(wxLANGUAGE_DEFAULT) does not work correctly
    // on the Mac so we must use wxUILocale::GetSystemLanguage().
    if (m_bUseDefaultLocale || (iDesiredLanguageCode == wxLANGUAGE_DEFAULT)) {
        iDesiredLanguageCode = wxUILocale::GetSystemLanguage();
        m_strISOLanguageCode = wxLocale::GetLanguageCanonicalName(iDesiredLanguageCode);
    }
#endif

    m_pLocale->Init(iDesiredLanguageCode);
    if (iDesiredLanguageCode == wxLANGUAGE_DEFAULT) {
        m_strISOLanguageCode = m_pLocale->GetCanonicalName();
    }

    // Look for the localization files by absolute and relative locations.
    //   preference given to the absolute location.
    if (!m_strBOINCMGRRootDirectory.IsEmpty()) {
        m_pLocale->AddCatalogLookupPathPrefix(
            wxString(m_strBOINCMGRRootDirectory + wxT("locale"))
        );
    }
    m_pLocale->AddCatalogLookupPathPrefix(wxT("locale"));
    m_pLocale->AddCatalog(wxT("BOINC-Manager"));
    m_pLocale->AddCatalog(wxT("BOINC-Client"));
    m_pLocale->AddCatalog(wxT("BOINC-Web"));

    InitSupportedLanguages();

    // Note: JAWS for Windows will only speak the context-sensitive
    // help if you use this help provider:
    wxHelpProvider::Set(new wxHelpControllerHelpProvider());

    // Enable known image types
    wxInitAllImageHandlers();

    // Initialize the skin manager
    m_pSkinManager = new CSkinManager(m_bDebugSkins);
    wxASSERT(m_pSkinManager);


    // Load desired manager skin
    m_pConfig->Read(wxT("Skin"), &strDesiredSkinName, m_pSkinManager->GetDefaultSkinName());
    m_pSkinManager->ReloadSkin(strDesiredSkinName);

#ifdef SANDBOX
    // Make sure owners, groups and permissions are correct for the current setting of g_use_sandbox
    //
    // NOTE: GDB and LLDB can't attach to applications which are running as
    // a different user or group.
    // Normally, the Mac Development (Debug) builds do not define SANDBOX, so
    // check_security() is never called. However, it is possible to use GDB
    // or LLDB on sandbox-specific code, as long as the code is run as the
    // current user (i.e., not as boinc_master or boinc_project), and the
    // current user is a member of both groups boinc_master and boinc_project.
    // However, this has not been thoroughly tested. Please see the comments
    // in SetupSecurity.cpp and check_security.cpp for more details.
    //
    char path_to_error[MAXPATHLEN];
    path_to_error[0] = '\0';

    if (!iErrorCode) {
        iErrorCode = check_security(
            g_use_sandbox, true, path_to_error, sizeof(path_to_error)
        );
    }
    if (iErrorCode) {

#if (defined(__WXMAC__) && (!defined (_DEBUG)))
        if (!IsApplicationVisible()) {  // If we were (probably) launched from a Login Item
            wxString launchAgentPath = wxFileName::GetHomeDir() + "/Library/LaunchAgents/edu.berkeley.boinc.plist";
            if (wxFileName::FileExists(launchAgentPath)) {  // If PostInstall app set up a LaunchAgent for this user
                boinc_sleep(30.);   // Allow time for LaunchAgent to terminate us before complaining
            }
        }
#endif
        ShowApplication(true);

        if (iErrorCode == -1099) {
#if (defined(__WXMAC__) && defined (_DEBUG))
            strDialogMessage.Printf(
                "To debug with sandbox security enabled, the current user\n"
                "must be a member of both groups boinc_master and boinc_project."
            );
#else   // ! (defined(__WXMAC__) && defined (_DEBUG))
            strDialogMessage.Printf(
                _("You currently are not authorized to manage %s.\n\nTo run %s as this user, please:\n- reinstall %s answering \"Yes\" to the question about non-administrative users\n or\n- contact your administrator to add you to the 'boinc_master' user group."),
                m_pSkinManager->GetAdvanced()->GetApplicationShortName().c_str(),
                m_pSkinManager->GetAdvanced()->GetApplicationShortName().c_str(),
                m_pSkinManager->GetAdvanced()->GetApplicationShortName().c_str()
            );
#endif  // ! (defined(__WXMAC__) && defined (_DEBUG))
        } else {
            strDialogMessage.Printf(
                _("%s ownership or permissions are not set properly; please reinstall %s.\n(Error code %d"),
                m_pSkinManager->GetAdvanced()->GetApplicationShortName().c_str(),
                m_pSkinManager->GetAdvanced()->GetApplicationShortName().c_str(),
                iErrorCode
            );
            if (path_to_error[0]) {
                strDialogMessage += _(" at ");
                strDialogMessage += wxString::FromUTF8(path_to_error);
            }
            strDialogMessage += _(")");

            fprintf(stderr, "%s\n", (const char*)strDialogMessage.utf8_str());
        }

        wxMessageDialog* pDlg = new wxMessageDialog(
                                    NULL,
                                    strDialogMessage,
                                    m_pSkinManager->GetAdvanced()->GetApplicationName(),
                                    wxOK
                                    );

        pDlg->ShowModal();
        if (pDlg)
            pDlg->Destroy();

        return false;
    }
#endif      // SANDBOX


#ifdef __WXMSW__
    // Perform any last minute checks that should keep the manager
    // from starting up.
    wxString strRebootPendingFile =
        GetRootDirectory() + wxFileName::GetPathSeparator() + wxT("RebootPending.txt");

    if (wxFile::Exists(strRebootPendingFile)) {
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

#ifdef __WXMAC__
    // Prevent a situation where wxSingleInstanceChecker lock file
    // from last login auto start (with same pid) was not deleted.
    // This path must match that in DetectDuplicateInstance()
    wxString lockFilePath = wxString(wxFileName::GetHomeDir() +
                                        "/Library/Application Support/BOINC/" +
                                        wxTheApp->GetAppName() +
                                        '-' + wxGetUserId()
                                        );
    if (WasFileModifiedBeforeSystemBoot((char *)(const char*)lockFilePath.utf8_str())) {
        boinc_delete_file(lockFilePath.utf8_str());
    }

    wxString launchAgentPath = wxFileName::GetHomeDir() + "/Library/LaunchAgents/edu.berkeley.launchboincmanager.plist";
    if (!wxFileName::FileExists(launchAgentPath)) { // If we are still using old style login item
        char s[MAXPATHLEN];
        snprintf(s, sizeof(s), "open \"/Library/Application Support/BOINC Data/%s_Finish_Install.app\" --args -m", brandName[GetBrandID()]);
        callPosixSpawn(s);   // run BOINC_Finish_Install utility to update to launchagent
    }
#endif

    // Detect if BOINC Manager is already running, if so, bring it into the
    // foreground and then exit.
    if (DetectDuplicateInstance()) {
#ifdef __WXMAC__
        // Each time a second instance of BOINC Manager is launched, it
        // normally adds an additional BOINC icon to the "recently run apps"
        // section of the Dock, cluttering it up. To avoid this, we call
        // this routine on the second instance to tell the system to hide
        // the icon in the Dock.
        // NOTE: This technique works only if BOINCManager.app is NOT in the
        // /Applications directory or any of its subdirectories. Putting it
        // in "/Library/Application Support" with a soft link to it from the
        // /Applications directory does work.
        // https://stackoverflow.com/questions/620841/how-to-hide-the-dock-icon
        SetActivationPolicyAccessory(true);    // Hide our Dock tile

        // A second instance of BOINC Manager is sometimes launched at login
        // when the Manager is launched by the login startup item and also
        // by the "restore open windows" or "restore open applications"
        // feature of MacOS. In that case, don't show the "Another instance
        // of BOINC Manager is running" dialog.
        if (m_bBOINCMGRAutoStarted) return false;
        // TODO: Should we also ignore duplicate instances that occur within
        // TODO: a short time after login (determined by getlastlogxbyname(),
        // TODO: in case MacOS's "Reopen windows when logging in" functionality
        // TODO: relaunched us AFTER our LaunchAgent autostarted us?
#endif
        if (GetBOINCMGRDisplayAnotherInstanceRunningMessage()) {
            wxString appName = m_pSkinManager->GetAdvanced()->GetApplicationName();
            wxString message;
            message.Printf(_("Another instance of %s is already running."), appName);
            CDlgGenericMessageParameters params;
            params.caption = appName;
            params.message = message;
            params.button2 = CDlgGenericMessageButton(false);
            CDlgGenericMessage dlg(NULL, &params);
            ShowApplication(true);
            dlg.ShowModal();
            SetBOINCMGRDisplayAnotherInstanceRunningMessage(!dlg.GetDisableMessageValue());
        }
        return false;
    }

    // Initialize the main document
    m_pDocument = new CMainDocument();
    wxASSERT(m_pDocument);

    m_pDocument->OnInit();


    // Is there a condition in which the Simple GUI should not be used?
    if (BOINC_SIMPLEGUI == m_iGUISelected) {
        // Screen too small?
        if (wxGetDisplaySize().GetHeight() < 600) {
            m_iGUISelected = BOINC_ADVANCEDGUI;
        }
    }

#ifndef __WXGTK__
    // Initialize the task bar icon
	m_pTaskBarIcon = new CTaskBarIcon(
        m_pSkinManager->GetAdvanced()->GetApplicationIcon(),
        m_pSkinManager->GetAdvanced()->GetApplicationDisconnectedIcon(),
        m_pSkinManager->GetAdvanced()->GetApplicationSnoozeIcon()
#ifdef __WXMAC__
        , wxTBI_CUSTOM_STATUSITEM
#endif
    );
    wxASSERT(m_pTaskBarIcon);
#endif // __WXGTK__
#ifdef __WXMAC__
    m_pMacDockIcon = new CTaskBarIcon(
        m_pSkinManager->GetAdvanced()->GetApplicationIcon(),
        m_pSkinManager->GetAdvanced()->GetApplicationDisconnectedIcon(),
        m_pSkinManager->GetAdvanced()->GetApplicationSnoozeIcon()
        , wxTBI_DOCK
    );
    wxASSERT(m_pMacDockIcon);
#endif

    // Startup the System Idle Detection code
    IdleTrackerAttach();

    // Show the UI
    SetActiveGUI(m_iGUISelected, m_bGUIVisible);

    if (!m_bGUIVisible) {
        ShowApplication(false);
	}

    if (bOpenEventLog) {
        DisplayEventLog(m_bGUIVisible);
        if (m_bGUIVisible && m_pFrame) {
            m_pFrame->Raise();
        }
    }

    return true;
}

#ifdef __WXMAC__
// We can "show" (unhide) the main window when the
// application is hidden and it won't be visible.
// If we don't do this under wxCocoa 3.0, the Dock
// icon will bounce (as in notification) when we
// click on our menu bar icon.
// But wxFrame::Show(true) makes the application
// visible again, so we instead call
// m_pFrame->wxWindow::Show() here.
//
// We need to call HideThisApp() after the event
// loop is running, so this is called from
// CBOINCBaseFrame::OnPeriodicRPC() at the first
// firing of ID_PERIODICRPCTIMER.
//
void CBOINCGUIApp::OnFinishInit() {
    if (!m_bGUIVisible) {
        HideThisApp();

        m_pFrame->wxWindow::Show();

        if (m_pEventLog) {
            m_pEventLog->wxWindow::Show();
        }
    }
}
#endif


int CBOINCGUIApp::OnExit() {
    // Shutdown the System Idle Detection code
    IdleTrackerDetach();

// Under wxWidgets 2.8.0, the task bar icons
// must be deleted for app to exit its main loop
#ifdef __WXMAC__
    if (m_pMacDockIcon) {
        delete m_pMacDockIcon;
    }
    m_pMacDockIcon = NULL;
#endif
#ifndef __WXGTK__
    if (m_pTaskBarIcon) {
        delete m_pTaskBarIcon;
    }
    m_pTaskBarIcon = NULL;
#endif
    if (m_pDocument) {
        m_pDocument->OnExit();
        delete m_pDocument;
        m_pDocument = NULL;
    }

    // Save Application State
    SaveState();

    if (m_pSkinManager) {
        delete m_pSkinManager;
        m_pSkinManager = NULL;
    }

    if (m_pLocale) {
        delete m_pLocale;
        m_pLocale = NULL;
    }

    if (m_pEventLog) {
        m_pEventLog->Destroy();
        m_pEventLog = NULL;
    }

    if (m_pInstanceChecker) {
        delete m_pInstanceChecker;
        m_pInstanceChecker = NULL;
    }

    diagnostics_finish();

    return wxApp::OnExit();
}


#ifndef __WXMAC__
// Ensure we shut down gracefully on Windows logout or shutdown
void CBOINCGUIApp::OnEndSession(wxCloseEvent& ) {
    s_bSkipExitConfirmation = true;

    // On Windows Vista with UAC turned on, we have to spawn a new process to change the
    // state of a service.  When Windows is shutting down it'll prevent new processes from
    // being created.  Sometimes it'll present a crash dialog for the newly spawned application.
    //
    // So, we will just let the OS shutdown the service via the service control manager.
    //
    if (m_iShutdownCoreClient && m_pDocument->m_pClientManager->IsBOINCConfiguredAsDaemon()) {
        m_iShutdownCoreClient = false;
    }

    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_EXIT);
    // The event loop has already been stopped,
    // so we must call OnExit directly
    pFrame->OnExit(evt);
    OnExit();
}
#endif


void CBOINCGUIApp::SaveState() {
    // Save Application State
    m_pConfig->SetPath(wxT("/"));
    if (m_pSkinManager) {
        m_pConfig->Write(wxT("Skin"), m_pSkinManager->GetSelectedSkin());
    }
    m_pConfig->Write(wxT("LanguageISO"), m_strISOLanguageCode);
    m_pConfig->Write(wxT("UseDefaultLocale"), m_bUseDefaultLocale);
    m_pConfig->Write(wxT("AutomaticallyShutdownClient"), m_iShutdownCoreClient);
    m_pConfig->Write(wxT("DisplayShutdownClientDialog"), m_iDisplayExitDialog);
    m_pConfig->Write(wxT("DisplayShutdownConnectedClientDialog"), m_iDisplayShutdownConnectedClientDialog);
    m_pConfig->Write(wxT("DisplayAnotherInstanceRunningDialog"), m_iDisplayAnotherInstanceRunningDialog);
#ifdef __WXMAC__
    m_pConfig->Write(wxT("HideMenuBarIcon"), m_iHideMenuBarIcon);
    m_pConfig->Write(wxT("WasShutDownBySystemWhileHidden"), m_iWasShutDownBySystemWhileHidden);
#endif
    m_pConfig->Write(wxT("DisableAutoStart"), m_iBOINCMGRDisableAutoStart);
    m_pConfig->Write(wxT("RunDaemon"), m_bRunDaemon);
}


///
/// Pass the command line parameters and discriptions to wxWidgets for displaying.
///
void CBOINCGUIApp::OnInitCmdLine(wxCmdLineParser &parser) {
    wxApp::OnInitCmdLine(parser);
    parser.AddSwitch("a", "autostart", _("BOINC Manager was started by the operating system automatically"));
#if defined(__WXMSW__) || defined(__WXMAC__)
    parser.AddSwitch("s", "systray", _("Startup BOINC so only the system tray icon is visible"));
#else
    parser.AddOption("e", "clientdir", _("Directory containing the BOINC Client executable"));
    parser.AddOption("d", "datadir", _("BOINC data directory"));
#endif
    parser.AddOption("n", "namehost", _("Host name or IP address"));
    parser.AddOption("g", "gui_rpc_port", _("GUI RPC port number"));
    parser.AddOption("p", "password", _("Password"));
    parser.AddOption("b", "boincargs", _("Startup BOINC with these optional arguments"));
    parser.AddSwitch("i","insecure", _("disable BOINC security users and permissions"));
    parser.AddSwitch("c", "checkskins", _("set skin debugging mode to enable skin manager error messages"));
    parser.AddSwitch("m", "multiple", _("multiple instances of BOINC Manager allowed"));
#if (defined(__WXMAC__) && defined(_DEBUG))
    parser.AddLongOption("NSDocumentRevisionsDebugMode", _("Not used: workaround for bug in XCode 4.2"));
#endif
    parser.AddSwitch("nd", "no-daemon", _("Don't run the client"));
}


///
/// Parse command line parameters.
///
bool CBOINCGUIApp::OnCmdLineParsed(wxCmdLineParser &parser) {
    // Give default processing (-?, --help and --verbose) the chance to do something.
    wxApp::OnCmdLineParsed(parser);
    wxString portNum = wxEmptyString;
    long longPort;
    bool hostNameSpecified = false;
    bool passwordSpecified = false;

    parser.Found(wxT("boincargs"), &m_strBOINCArguments);
    if (parser.Found(wxT("autostart"))) {
        m_bBOINCMGRAutoStarted = true;
    }
#if defined(__WXMSW__) || defined(__WXMAC__)
    if (parser.Found(wxT("systray"))) {
        m_bGUIVisible = false;
#ifdef __WXMAC__
        m_bBOINCMGRAutoStarted = true;
#endif
    }
#endif
    if (parser.Found(wxT("insecure"))) {
        g_use_sandbox = false;
    }
    if (parser.Found(wxT("checkskins"))) {
        m_bDebugSkins = true;
    }
    if (parser.Found(wxT("multiple"))) {
        m_bMultipleInstancesOK = true;
    }

#if !(defined(__WXMSW__) || defined(__WXMAC__))
    if (!parser.Found(wxT("clientdir"), &m_strBOINCMGRRootDirectory)) {
        m_strBOINCMGRRootDirectory = ::wxGetCwd();
    }
    if (m_strBOINCMGRRootDirectory.Last() != '/') {
        m_strBOINCMGRRootDirectory.Append('/');
    }

    if (!parser.Found(wxT("datadir"), &m_strBOINCMGRDataDirectory)) {
        m_strBOINCMGRDataDirectory = m_strBOINCMGRRootDirectory;
    }
    if (m_strBOINCMGRDataDirectory.Last() != '/') {
        m_strBOINCMGRDataDirectory.Append('/');
    }
#endif

    if (parser.Found(wxT("namehost"), &m_strHostNameArg)) {
        hostNameSpecified = true;
    } else {
        m_strHostNameArg = wxT("localhost");
    }

     if (parser.Found(wxT("gui_rpc_port"), &portNum)) {
        if (portNum.ToLong(&longPort)) {
            m_iRPCPortArg = longPort;
        } else {
            m_iRPCPortArg = GUI_RPC_PORT;  // conversion failed
        }
    } else {
        m_iRPCPortArg = GUI_RPC_PORT;
    }

    if (parser.Found(wxT("password"), &m_strPasswordArg)) {
        passwordSpecified = true;
    } else {
        m_strPasswordArg = wxEmptyString;
    }

    if (hostNameSpecified && passwordSpecified) {
        m_bMultipleInstancesOK = true;
        m_bHostnamePasswordSet = true;
    }

    if (parser.Found(wxT("no-daemon"))) {
        m_bNeedRunDaemon = false;
    }
    return true;
}


///
/// Detect if another instance of this application is running.
//  Returns true if there is and it is forbidden, otherwise false
//
// We must initialize m_pInstanceChecker even if m_bMultipleInstancesOK
// is true so CMainDocument::OnPoll() can call IsMgrMultipleInstance().
///
bool CBOINCGUIApp::DetectDuplicateInstance() {
#ifdef __WXMAC__
    m_pInstanceChecker = new wxSingleInstanceChecker(
            wxTheApp->GetAppName() + '-' + wxGetUserId(),
            wxFileName::GetHomeDir() + "/Library/Application Support/BOINC"
            );
#else
    m_pInstanceChecker = new wxSingleInstanceChecker();
#endif
    if (m_pInstanceChecker->IsAnotherRunning()) {
        if (m_bMultipleInstancesOK) return false;
#ifdef __WXMSW__
        CTaskBarIcon::FireAppRestore();
#endif
        return true;
    }
    return false;
}


///
/// Determines what name BOINC Manager is called.
///
void CBOINCGUIApp::DetectExecutableName() {
#ifdef __WXMSW__
    TCHAR   szPath[MAX_PATH-1];

    // change the current directory to the boinc install directory
    GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR)));

    TCHAR *pszProg = _tcsrchr(szPath, '\\');
    if (pszProg) {
        pszProg++;
    }

    // Store the root directory for later use.
    m_strBOINCMGRExecutableName = pszProg;
#elif defined(__WXGTK__)
    char path[PATH_MAX];
    if (!get_real_executable_path(path, PATH_MAX)) {
        // find filename component
        char* name = strrchr(path, '/');
        if (name) {
            name++;
            m_strBOINCMGRExecutableName = name;
        }
    }
#endif
}


///
/// Determines where the BOINC Manager is executing from.
///
void CBOINCGUIApp::DetectRootDirectory() {
#ifdef __WXMSW__
    TCHAR   szPath[MAX_PATH-1];

    // change the current directory to the boinc install directory
    GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR)));

    TCHAR *pszProg = _tcsrchr(szPath, '\\');
    if (pszProg) {
        szPath[pszProg - szPath + 1] = 0;
    }

    // Store the root directory for later use.
    m_strBOINCMGRRootDirectory = szPath;
#elif defined(__WXGTK__)
    char path[PATH_MAX];
    if (!get_real_executable_path(path, PATH_MAX)) {
        // find path component
        char* name = strrchr(path, '/');
        if (name) {
            name++;
            *name = '\0';
            m_strBOINCMGRRootDirectory = path;
        }
    }
#endif
}


///
/// Determines where the BOINC data directory is.
///
void CBOINCGUIApp::DetectDataDirectory() {
#ifdef __WXMSW__
    //
    // Determine BOINCMgr Data Directory
    //
	LONG    lReturnValue;
	HKEY    hkSetupHive;
    TCHAR   szPath[MAX_PATH];
    LPTSTR  lpszValue = NULL;
    LPTSTR  lpszExpandedValue = NULL;
    DWORD   dwValueType = REG_EXPAND_SZ;
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
            &dwValueType,
            NULL,
            &dwSize
        );
        if (lReturnValue != ERROR_FILE_NOT_FOUND) {
            // Allocate the buffer space.
            lpszValue = (LPTSTR) malloc(dwSize);
            (*lpszValue) = NULL;

            // Now get the data
            lReturnValue = RegQueryValueEx(
                hkSetupHive,
                _T("DATADIR"),
                NULL,
                &dwValueType,
                (LPBYTE)lpszValue,
                &dwSize
            );

            // Expand the Strings
            // We need to get the size of the buffer needed
            dwSize = 0;
            lReturnValue = ExpandEnvironmentStrings(lpszValue, NULL, dwSize);

            if (lReturnValue) {
                // Make the buffer big enough for the expanded string
                lpszExpandedValue = (LPTSTR) malloc(lReturnValue*sizeof(TCHAR));
                (*lpszExpandedValue) = NULL;
                dwSize = lReturnValue;

                ExpandEnvironmentStrings(lpszValue, lpszExpandedValue, dwSize);

                // Store the root directory for later use.
                m_strBOINCMGRDataDirectory = lpszExpandedValue;
            }
        }
    } else {
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath))) {
            _tcsncat(szPath, _T("\\boinc"), ((sizeof(szPath)/sizeof(TCHAR)) - _tcslen(szPath)));
            if (wxDir::Exists(szPath)) {
                // Store the root directory for later use.
                m_strBOINCMGRDataDirectory = szPath;
            }
        }
    }

    // Cleanup
	if (hkSetupHive) RegCloseKey(hkSetupHive);
    if (lpszValue) free(lpszValue);
    if (lpszExpandedValue) free(lpszExpandedValue);
#endif
#ifdef __WXMAC__
    m_strBOINCMGRDataDirectory = wxT("/Library/Application Support/BOINC Data");
#endif
}


void CBOINCGUIApp::InitSupportedLanguages() {
    m_astrLanguages.clear();

    // Find available translations
    std::vector<const wxLanguageInfo*> availableTranslations;
    // English is a special case:
    //  - it's guaranteed to be available because it's compiled in
    //  - it must be added to the list even though we don't expect to find a translation for it
    const wxLanguageInfo* pLIen = wxLocale::GetLanguageInfo(wxLANGUAGE_ENGLISH);
    if (pLIen) {
        availableTranslations.push_back(pLIen);
    }
    // Now fill in the rest from the available message catalogs
    const wxTranslations* pTranslations = wxTranslations::Get();
    if (pTranslations) {
        wxArrayString langCodes = pTranslations->GetAvailableTranslations(wxT("BOINC-Manager"));
        for (const wxString& langCode : langCodes) {
            if (langCode == wxT("en")) continue;
            const wxLanguageInfo* pLI = wxLocale::FindLanguageInfo(langCode);
            if (pLI) {
                availableTranslations.push_back(pLI);
            }
        }
    }

    // Synthesize labels to be used in the options dialog
    //
    // As we are building strings that potentially contain both left-to-right and
    // right-to-left text, we must insert direction markers to ensure the layout is
    // correct. Otherwise strange things happen - particularly when the strings contain
    // parentheses, which can end up in the wrong place and pointing the wrong way.
    // The usage here has been determined largely by trial and error, and may not be
    // strictly correct...
    const wxString LRM = L'\x200E'/*LEFT-TO-RIGHT MARK*/;
    const wxString RLM = L'\x200F'/*RIGHT-TO-LEFT MARK*/;
    const wxLanguageInfo* pLIui = wxLocale::FindLanguageInfo(GetISOLanguageCode());
    wxLayoutDirection uiLayoutDirection = pLIui ? pLIui->LayoutDirection : wxLayout_Default;
    GUI_SUPPORTED_LANG newItem;

    // CDlgOptions depends on "Auto" being the first item in the list
    // if
    newItem.Language = wxLANGUAGE_DEFAULT;
    wxString strAutoEnglish = wxT("(Automatic Detection)");
    wxString strAutoTranslated = wxGetTranslation(strAutoEnglish);
    newItem.Label = strAutoTranslated;
    if (strAutoTranslated != strAutoEnglish) {
        if (uiLayoutDirection == wxLayout_RightToLeft) {
            newItem.Label += RLM;
        } else if (uiLayoutDirection == wxLayout_LeftToRight) {
            newItem.Label += LRM;
        }
        newItem.Label += wxT(" ");
        if (uiLayoutDirection == wxLayout_RightToLeft) {
            newItem.Label += RLM;
        }
        newItem.Label += LRM + strAutoEnglish + LRM;
    }
    m_astrLanguages.push_back(newItem);

    // Add known locales to the list
    for (int langID = wxLANGUAGE_UNKNOWN+1; langID < wxLANGUAGE_USER_DEFINED; ++langID) {
        const wxLanguageInfo* pLI = wxLocale::GetLanguageInfo(langID);
        if (pLI == NULL) continue;
        wxString lang_region = pLI->CanonicalName.BeforeFirst('@');
        wxString lang = lang_region.BeforeFirst('_');
        wxString script = pLI->CanonicalName.AfterFirst('@');
        wxString lang_script = lang;
        if (!script.empty()) {
            lang_script += wxT("@") + script;
        }
        std::vector<const wxLanguageInfo*>::const_iterator foundit = availableTranslations.begin();
        while (foundit != availableTranslations.end()) {
            const wxLanguageInfo* pLIavail = *foundit;
            if (pLIavail->CanonicalName == lang_script ||
                pLIavail->CanonicalName == pLI->CanonicalName) {
                break;
            }
            ++foundit;
        }
        // If we don't have a translation, don't add to the list -
        // unless the locale has been explicitly selected by the user
        // (setting migrated from an earlier version, or manually configured)
        if (foundit == availableTranslations.end() && pLI != pLIui) continue;
        newItem.Language = langID;
#if wxCHECK_VERSION(3,1,6)
        if (pLI->DescriptionNative != pLI->Description &&
            !pLI->DescriptionNative.empty()) {
            // The "NativeName (EnglishName)" format of the label matches that used
            // for Web sites [language_select() in html/inc/language_names.inc]
            newItem.Label = pLI->DescriptionNative;
            if (pLI->LayoutDirection == wxLayout_RightToLeft) {
                newItem.Label += RLM;
            } else if (pLI->LayoutDirection == wxLayout_LeftToRight) {
                newItem.Label += LRM;
            }
            newItem.Label += wxT(" ");
            if (uiLayoutDirection == wxLayout_RightToLeft) {
                newItem.Label += RLM;
            }
            newItem.Label += LRM + wxT("(") + pLI->Description + wxT(")") + LRM;
        } else {
            newItem.Label = pLI->Description + LRM;
        }
#else
        newItem.Label = pLI->Description + LRM;
#endif
        m_astrLanguages.push_back(newItem);
    }
}


int CBOINCGUIApp::IdleTrackerAttach() {
#ifdef __WXMSW__
    ::attach_idle_monitor();
#endif
    return 0;
}


int CBOINCGUIApp::IdleTrackerDetach() {
#ifdef __WXMSW__
    ::detach_idle_monitor();
#endif
    return 0;
}


// TODO: Does the Mac really need the OnActivateApp() routine?
void CBOINCGUIApp::OnActivateApp(wxActivateEvent& event) {
    m_bProcessingActivateAppEvent = true;

#ifndef __WXMSW__  // On Win, the following raises the wrong window
    if (event.GetActive())
#endif
    {
#ifdef __WXMAC__
        // When our LaunchAgent / login item launches us at login, it activates
        // this app, but we want it hidden. So we immediatly hide it upon the
        // first activation when run as a login item.
        static bool first = true;

        CMainDocument*      pDoc = wxGetApp().GetDocument();
        if (m_bBOINCMGRAutoStarted && (!pDoc->IsConnected()) && first) {
            first = false;
            ShowApplication(false);
        } else
#endif
#if (defined (__WXMAC__) || defined(__WXGTK__))
        {
           // Linux allows the Event Log to be brought forward and made active
            // even if we have a modal dialog displayed (associated with our
            // main frame.) This test is needed to allow bringing the modal
            // dialog forward again by clicking on its title bar.
            if (!IsModalDialogDisplayed())
            {
                bool keepEventLogInFront = m_bEventLogWasActive;

                if (m_pEventLog && !m_pEventLog->IsIconized() && !keepEventLogInFront) {
                    m_pEventLog->Raise();
                }
                if (m_pFrame) {
                    m_pFrame->Raise();
                }
                if (m_pEventLog && !m_pEventLog->IsIconized() && keepEventLogInFront) {
                    m_pEventLog->Raise();
                }
            }
        }
#endif
    }

    event.Skip();

    m_bProcessingActivateAppEvent = false;
}


void CBOINCGUIApp::OnRPCFinished( CRPCFinishedEvent& event ) {
    CMainDocument*      pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->OnRPCComplete(event);
}


int CBOINCGUIApp::UpdateSystemIdleDetection() {
#ifdef __WXMSW__
    return get_idle_tick_count();
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


int CBOINCGUIApp::StartBOINCDefaultScreensaverTest() {
#ifdef __WXMSW__
    wxString strExecute = wxEmptyString;
    strExecute = wxT("\"") + m_strBOINCMGRRootDirectory + wxT("\\boincscr.exe\" --test");
    ::wxExecute(strExecute);
#endif
    return 0;
}


// Display the Event Log, it is a modeless dialog not owned by
// any other UI element.
// To work around a Linux bug in wxWidgets 3.0 which prevents
// bringing the main frame forward on top of a modeless dialog,
// the Event Log is now a wxFrame on Linux only.
void CBOINCGUIApp::DisplayEventLog(bool bShowWindow) {
    if (m_pEventLog) {
        if (bShowWindow) {
            if (m_pEventLog->IsIconized()) {
                m_pEventLog->Iconize(false);
            }
            m_pEventLog->Raise();
        }
    } else {
        m_pEventLog = new CDlgEventLog();
        if (m_pEventLog) {
                m_pEventLog->Show(bShowWindow);
            if (bShowWindow) {
                m_pEventLog->Raise();
            }
            if (m_pFrame) {
                m_pFrame->UpdateRefreshTimerInterval();
            }
        }
    }
}


void CBOINCGUIApp::OnEventLogClose() {
    m_pEventLog = NULL;
    if (m_pFrame) {
        m_pFrame->UpdateRefreshTimerInterval();
    }
}


// The skin has changed and all UI elements need to reload their bitmaps.
//
void CBOINCGUIApp::FireReloadSkin() {
    if (m_pFrame) {
	    m_pFrame->FireReloadSkin();
    }
#ifndef __WXGTK__
    if (m_pTaskBarIcon) {
	    m_pTaskBarIcon->FireReloadSkin();
    }
#endif
}


bool CBOINCGUIApp::SetActiveGUI(int iGUISelection, bool bShowWindow) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCGUIApp::SetActiveGUI - Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCGUIApp::SetActiveGUI - GUI Selection: '%d', Show: %d'"), iGUISelection, (int)bShowWindow);

    CBOINCBaseFrame* pNewFrame = NULL;
    CBOINCBaseFrame* pOldFrame = m_pFrame;
    wxInt32          iTop = 0;
    wxInt32          iLeft = 0;
    wxInt32          iHeight = 0;
    wxInt32          iWidth = 0;
    bool             bWindowMaximized = false;


    // Create the new window
    if ((iGUISelection != m_iGUISelected) || !m_pFrame) {

        // Retrieve the desired window state before creating the
        //   desired frames
        if (BOINC_ADVANCEDGUI == iGUISelection) {
            m_pConfig->SetPath(wxT("/"));
            m_pConfig->Read(wxT("YPos"), &iTop, 30);
            m_pConfig->Read(wxT("XPos"), &iLeft, 30);
            m_pConfig->Read(wxT("Width"), &iWidth, 800);
            m_pConfig->Read(wxT("Height"), &iHeight, 600);
            m_pConfig->Read(wxT("WindowMaximized"), &bWindowMaximized, false);
            // Guard against a rare situation where registry values are zero
            if (iWidth < 50) iWidth = 800;
            if (iHeight < 50) iHeight = 600;
        } else {
            m_pConfig->SetPath(wxT("/Simple"));
            m_pConfig->Read(wxT("YPos"), &iTop, 30);
            m_pConfig->Read(wxT("XPos"), &iLeft, 30);

            // We don't save Simple View's width & height since it's
            // window is not resizable, so don't try to read them
#ifdef __WXMAC__
//            m_pConfig->Read(wxT("Width"), &iWidth, 409);
//            m_pConfig->Read(wxT("Height"), &iHeight, 561);
            iWidth = 409;
            iHeight = 561;
#else
//            m_pConfig->Read(wxT("Width"), &iWidth, 416);
//            m_pConfig->Read(wxT("Height"), &iHeight, 570);
            iWidth = 416;
            iHeight = 570;
#endif
        }


        // Make sure that the new window is going to be visible
        //   on a screen
#ifdef __WXMAC__
    if (!IsWindowOnScreen(iLeft, iTop, iWidth, iHeight)) {
        iTop = iLeft = 30;
    }
#else
	    // If either co-ordinate is less then 0 then set it equal to 0 to ensure
	    // it displays on the screen.
	    if ( iLeft < 0 ) iLeft = 30;
	    if ( iTop < 0 ) iTop = 30;

	    // Read the size of the screen
	    wxInt32 iMaxWidth = wxSystemSettings::GetMetric( wxSYS_SCREEN_X );
	    wxInt32 iMaxHeight = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y );

	    // Max sure that it doesn't go off to the right or bottom
	    if ( iLeft + iWidth > iMaxWidth ) iLeft = iMaxWidth - iWidth;
	    if ( iTop + iHeight > iMaxHeight ) iTop = iMaxHeight - iHeight;
#endif

        // Create the main window
        //
        if (BOINC_ADVANCEDGUI == iGUISelection) {
            // Initialize the advanced gui window
            pNewFrame = new CAdvancedFrame(
                m_pSkinManager->GetAdvanced()->GetApplicationName(),
                m_pSkinManager->GetAdvanced()->GetApplicationIcon(),
                wxPoint(iLeft, iTop),
                wxSize(iWidth, iHeight)
            );
        } else {
            // Initialize the simple gui window
            pNewFrame = new CSimpleFrame(
                m_pSkinManager->GetAdvanced()->GetApplicationName(),
                m_pSkinManager->GetAdvanced()->GetApplicationIcon(),
                wxPoint(iLeft, iTop),
                wxSize(iWidth, iHeight)
            );
        }

        wxASSERT(pNewFrame);

        if (pNewFrame) {
            SetTopWindow(pNewFrame);

            // Store the new frame for future use
            m_pFrame = pNewFrame;

            // Hide the old one if it exists.  We must do this
            // after updating m_pFrame to prevent Mac OSX from
            // hiding the application
            if (pOldFrame) pOldFrame->Hide();

            // Delete the old one if it exists
            if (pOldFrame) pOldFrame->Destroy();

            if (iGUISelection != m_iGUISelected) {
                m_iGUISelected = iGUISelection;
                m_pConfig->SetPath(wxT("/"));
                m_pConfig->Write(wxT("GUISelection"), iGUISelection);
                m_pConfig->Flush();
            }
        }
    }

    // Show the new frame if needed
    if (!m_bProcessingActivateAppEvent) {
        if (m_pFrame && bShowWindow) {
            if (m_pEventLog && !m_pEventLog->IsIconized()) {
                m_pEventLog->Show();
                m_pEventLog->Raise();
    #ifdef __WXMSW__
                ::SetForegroundWindow((HWND)m_pEventLog->GetHWND());
    #endif
            }

            if (!m_pFrame->IsShown()) {
                m_pFrame->Show();
            }
            if (m_pFrame->IsIconized()) {
                m_pFrame->Maximize(false);
            }
            else if (BOINC_ADVANCEDGUI == iGUISelection && bWindowMaximized) {
                m_pFrame->Maximize();
            }
            m_pFrame->Raise();

#ifdef __WXMSW__
            ::SetForegroundWindow((HWND)m_pFrame->GetHWND());
#endif
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCGUIApp::SetActiveGUI - Function End"));
    return true;
}


int CBOINCGUIApp::ConfirmExit() {
    CSkinAdvanced*  pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CMainDocument*  pDoc = wxGetApp().GetDocument();
    wxString        strConnectedCompter = wxEmptyString;
    bool            bWasVisible;
    int             retval = 0;

    wxASSERT(pDoc);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    pDoc->GetConnectedComputerName(strConnectedCompter);
    if (!pDoc->IsComputerNameLocal(strConnectedCompter)) {
        // Don't shut down remote clients on Manager exit
        return 1;
    }

    // Don't run confirmation dialog if logging out or shutting down Mac,
    // or if emergency exit from AsyncRPCDlg
    if (s_bSkipExitConfirmation) return 1;

    // Don't run confirmation dialog if second instance of Manager
    if (IsMgrMultipleInstance()) return 1;

    if (!m_iDisplayExitDialog) {
        // Mac: User doesn't want to display the dialog and just wants to use their previous value.
        // Win & Linux: User doesn't want to display the dialog and wants to shutdown the client.
        return 1;
    }

    bWasVisible = IsApplicationVisible();
    ShowApplication(true);

    CDlgExitMessage dlg(NULL);

    if (!pSkinAdvanced->GetExitMessage().IsEmpty()) {
        dlg.m_DialogExitMessage->SetLabel(pSkinAdvanced->GetExitMessage());
    }

#ifdef __WXMSW__
    if (m_iShutdownCoreClient) {
        dlg.m_DialogShutdownCoreClient->SetValue(TRUE);
    }
#endif

    if (m_iDisplayExitDialog) {
        dlg.m_DialogDisplay->SetValue(FALSE);
    }

    dlg.Fit();
    dlg.Centre();

    if (wxID_OK == dlg.ShowModal()) {
#ifdef __WXMAC__
        s_bSkipExitConfirmation = true;     // Don't ask twice (only affects Mac)
#else
        m_iShutdownCoreClient = dlg.m_DialogShutdownCoreClient->GetValue();
#endif
        m_iDisplayExitDialog = !dlg.m_DialogDisplay->GetValue();
        retval = true;

    }

    if (!bWasVisible) {
        ShowApplication(false);
    }

    return retval;       // User cancelled exit
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


#ifdef __WXMAC__
long CBOINCGUIApp::GetBrandID()
{
    long iBrandId;

    iBrandId = 0;   // Default value

    FILE *f = fopen("/Library/Application Support/BOINC Data/Branding", "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandId);
        fclose(f);
    }
    if ((iBrandId < 0) || (iBrandId > (NUMBRANDS-1))) {
        iBrandId = 0;
    }
    return iBrandId;
}


#else
// See clientgui/mac/BOINCGUIApp.mm for the Mac versions.
///
/// Determines if the current process is visible.
///
/// @return
///  true if the current process is visible, otherwise false.
///
bool CBOINCGUIApp::IsApplicationVisible() {
    return false;
}

///
/// Shows or hides the current process.
///
/// @param bShow
///   true will show the process, false will hide the process.
///
void CBOINCGUIApp::ShowApplication(bool) {
}
#endif


bool CBOINCGUIApp::ShowInterface() {
    ShowApplication(true);
    return SetActiveGUI(m_iGUISelected, true);
}


bool CBOINCGUIApp::ShowNotifications() {
    bool retval = false;

    retval = SetActiveGUI(m_iGUISelected, true);
    if (retval) {
        GetFrame()->FireNotification();
        GetDocument()->UpdateUnreadNoticeState();
    }

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
    int theEventType;
    wxDialog* theRPCWaitDialog;
    wxObject* theObject;

    if (!m_pDocument) return -1;

    theEventType = event.GetEventType();

    if (m_pDocument->WaitingForRPC()) {
        // If in RPC Please Wait dialog, reject all command
        // and timer events except:
        //  - RPC Finished
        //  - those for that dialog or its children
        //  - Open Manager menu item from system tray icon

        if ((theEventType == wxEVT_COMMAND_MENU_SELECTED) && (event.GetId() == wxID_OPEN)) {
            return -1;
        }

        theRPCWaitDialog = m_pDocument->GetRPCWaitDialog();
        theObject = event.GetEventObject();
        while (theObject) {
            if (!theObject->IsKindOf(CLASSINFO(wxWindow))) break;
            if (theObject == theRPCWaitDialog) return -1;
            theObject = ((wxWindow*)theObject)->GetParent();
        }
        // Continue with rest of filtering below
    } else {
        // Do limited filtering if shutting down to allow RPC
        // completion events but not events which start new RPCs
        if (!m_bFilterEvents) return -1;
    }

    // Allow all except Command, Timer and Mouse Moved events
    if (event.IsCommandEvent()) {
        return false;
    }

    if (theEventType == wxEVT_TIMER) {
        return false;
    }

#ifdef __WXMSW__
    if (theEventType == wxEVT_TASKBAR_MOVE) {
        return false;
    }
#endif

    return -1;
}

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
#pragma implementation "sg_BoincSimpleFrame.h"
#endif

#include "stdwx.h"
#ifdef __WXMAC__
#include "MacAccessiblity.h"
#endif
#include "diagnostics.h"
#include "str_util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "Events.h"
#include "BOINCBaseFrame.h"
#include "wizardex.h"
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "error_numbers.h"
#include "version.h"

#include "sg_BoincSimpleFrame.h"
#include "sg_TaskPanel.h"
#include "sg_ProjectPanel.h"
#include "sg_DlgMessages.h"
#include "sg_DlgPreferences.h"
#include "DlgEventLog.h"
#include "DlgAbout.h"
#include "DlgOptions.h"

// Workaround for Linux refresh problem
#if (defined(__WXMSW__) || defined(__WXMAC__))
#define REFRESH_WAIT 0
#else
#define REFRESH_WAIT 1
#endif

IMPLEMENT_DYNAMIC_CLASS(CSimpleFrame, CBOINCBaseFrame)

BEGIN_EVENT_TABLE(CSimpleFrame, CBOINCBaseFrame)
    EVT_SIZE(CSimpleFrame::OnSize)
    EVT_MENU(ID_CHANGEGUI, CSimpleFrame::OnChangeGUI)
    EVT_MENU(ID_SGDEFAULTSKINSELECTOR, CSimpleFrame::OnSelectDefaultSkin)
    EVT_MENU_RANGE(ID_SGFIRSTSKINSELECTOR, ID_LASTSGSKINSELECTOR, CSimpleFrame::OnSelectSkin)
    EVT_HELP(wxID_ANY, CSimpleFrame::OnHelp)
    EVT_FRAME_CONNECT(CSimpleFrame::OnConnect)
    EVT_FRAME_RELOADSKIN(CSimpleFrame::OnReloadSkin)
    EVT_FRAME_NOTIFICATION(CSimpleFrame::OnNotification)
    EVT_MENU(ID_PREFERENCES, CSimpleFrame::OnPreferences)
    EVT_MENU(ID_SGOPTIONS, CSimpleFrame::OnOptions)
    EVT_MENU(ID_HELPBOINC, CSimpleFrame::OnHelpBOINC)
    EVT_MENU(ID_HELPBOINCMANAGER, CSimpleFrame::OnHelpBOINC)
    EVT_MENU(ID_HELPBOINCWEBSITE, CSimpleFrame::OnHelpBOINC)
    EVT_MENU(wxID_ABOUT, CSimpleFrame::OnHelpAbout)
	EVT_MENU(ID_EVENTLOG, CSimpleFrame::OnEventLog)
END_EVENT_TABLE()


CSimpleFrame::CSimpleFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function End"));
}


CSimpleFrame::CSimpleFrame(wxString title, wxIcon* icon, wxIcon* icon32, wxPoint position, wxSize size) : 
    CBOINCBaseFrame((wxFrame *)NULL, ID_SIMPLEFRAME, title, position, size,
                    wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame:: - Overloaded Constructor Function Begin"));

    // Initialize Application
    wxIconBundle icons;
    icons.AddIcon(*icon);
    icons.AddIcon(*icon32);
    SetIcons(icons);
    
    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString           strMenuName;
    wxString           strMenuDescription;

    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    // File menu
    wxMenu *menuFile = new wxMenu;

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Close the %s window"), 
        pSkinAdvanced->GetApplicationName().c_str()
    );
    strMenuName = _("&Close Window");
    strMenuName += wxT("\tCtrl+W");
    menuFile->Append(
        ID_CLOSEWINDOW,
        strMenuName,
        strMenuDescription
    );

    // Skins submenu
    m_pSubmenuSkins = new wxMenu;
    
    BuildSkinSubmenu(m_pSubmenuSkins);
    
    // All other skin names will be appended as radio 
    // menu items with ID_SGFIRSTSKINSELECTOR + index
    
    // View menu
    wxMenu *menuView = new wxMenu;

    menuView->Append(
        ID_CHANGEGUI,
        _("Advanced View...\tCtrl+Shift+A"),
        _("Display the advanced graphical interface.")
    );

    menuView->AppendSeparator();

    menuView->Append(
        ID_SGSKINSELECTOR,
        _("Skin"),
        m_pSubmenuSkins,
        _("Select the appearance of the user interface.")
    );

    // Skins sumenu always contains the Default entry
    if (m_pSubmenuSkins->GetMenuItemCount() <= 1) {
        menuView->Enable(ID_SGSKINSELECTOR, false);
    }

    // Tools menu
    wxMenu *menuTools = new wxMenu;

    menuTools->Append(
        ID_PREFERENCES,
        _("Computing &preferences..."),
        _("Configure computing preferences")
    );
    
    menuTools->Append(
        ID_SGOPTIONS, 
        _("&Options..."),
        _("Configure display options and proxy settings")
    );

    // Help menu
    wxMenu *menuHelp = new wxMenu;

    // %s is the project name
    //    i.e. 'BOINC Manager', 'GridRepublic'
    strMenuName.Printf(
        _("%s &help"), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strMenuDescription.Printf(
        _("Show information about %s"), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINC,
        strMenuName, 
        strMenuDescription
    );

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuName.Printf(
        _("&%s"), 
        pSkinAdvanced->GetApplicationName().c_str()
    );
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Show information about the %s"), 
        pSkinAdvanced->GetApplicationName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINCMANAGER,
        strMenuName, 
        strMenuDescription
    );

    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strMenuName.Printf(
        _("%s &website"), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Show information about BOINC and %s"),
        pSkinAdvanced->GetApplicationName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINCWEBSITE,
        strMenuName, 
        strMenuDescription
    );

    // %s is the project name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuName.Printf(
        _("&About %s..."), 
        pSkinAdvanced->GetApplicationName().c_str()
    );
    menuHelp->Append(
        wxID_ABOUT,
        strMenuName, 
        _("Licensing and copyright information.")
    );

    // construct menu
    m_pMenubar = new wxMenuBar;
    m_pMenubar->Append(
        menuFile,
        _("&File")
    );
    m_pMenubar->Append(
        menuView,
        _("&View")
    );
    m_pMenubar->Append(
        menuTools,
        _("&Tools")
    );
    m_pMenubar->Append(
        menuHelp,
        _("&Help")
    );
    
    wxMenuBar* m_pOldMenubar = GetMenuBar();
    SetMenuBar(m_pMenubar);
    if (m_pOldMenubar) {
        delete m_pOldMenubar;
    }

#ifdef __WXGTK__
    // Force a redraw of the menu under Ubuntu's new interface
    SendSizeEvent();
#endif
#ifdef __WXMAC__
    m_pMenubar->MacInstallMenuBar();
    MacLocalizeBOINCMenu();

    // Enable Mac OS X's standard Preferences menu item (handled in MacSysMenu.cpp)
    EnableMenuCommand(NULL, kHICommandPreferences);
#endif

    m_Shortcuts[0].Set(wxACCEL_NORMAL, WXK_HELP, ID_HELPBOINCMANAGER);
#ifdef __WXMAC__
    m_Shortcuts[1].Set(wxACCEL_CMD|wxACCEL_SHIFT, (int)'E', ID_EVENTLOG);
#else
    m_Shortcuts[1].Set(wxACCEL_CTRL|wxACCEL_SHIFT, (int)'E', ID_EVENTLOG);
#endif
    m_pAccelTable = new wxAcceleratorTable(2, m_Shortcuts);

    SetAcceleratorTable(*m_pAccelTable);
    
    dlgMsgsPtr = NULL;
    m_pBackgroundPanel = new CSimpleGUIPanel(this);
    
    RestoreState();
}


CSimpleFrame::~CSimpleFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Destructor Function Begin"));

    SaveState();
    
    if (m_pAccelTable)
        delete m_pAccelTable;

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Destructor Function End"));
}


bool CSimpleFrame::SaveState() {
	CBOINCBaseFrame::SaveState();
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
	wxString        strBaseConfigLocation = wxString(wxT("/Simple"));

    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("XPos"), GetPosition().x);
    pConfig->Write(wxT("YPos"), GetPosition().y);

    return true;
}


bool CSimpleFrame::RestoreState() {
	CBOINCBaseFrame::RestoreState();
    return true;
}


int CSimpleFrame::_GetCurrentViewPage() {
    if (isMessagesDlgOpen()) {
        return VW_SGUI | VW_SMSG;
    } else {
        return VW_SGUI;
    }
    return 0;       // Should never happen.
}


void CSimpleFrame::OnChangeGUI(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnChangeGUI - Function Begin"));

    wxGetApp().SetActiveGUI(BOINC_ADVANCEDGUI, true);

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnChangeGUI - Function End"));
}


void CSimpleFrame::BuildSkinSubmenu( wxMenu *submenu) {
    unsigned int i;
    wxMenuItem *skinItem;
    wxArrayString astrSkins;
    wxString strSelectedSkin;
    CSkinManager* pSkinManager = wxGetApp().GetSkinManager();

    wxASSERT(pSkinManager);
    wxASSERT(wxDynamicCast(pSkinManager, CSkinManager));


    // The "Default" skin menu item is localized, but 
    // the name of the default skin is not localized
    skinItem = submenu->AppendRadioItem(
        ID_SGDEFAULTSKINSELECTOR,
        _("Default")
    );

    astrSkins = pSkinManager->GetCurrentSkins();
    strSelectedSkin = pSkinManager->GetSelectedSkin();
    
    if (strSelectedSkin == pSkinManager->GetDefaultSkinName()) {
        skinItem->Check(true);
    }
          
    // Skins list always contains the Default entry
    if (astrSkins.GetCount() <= 1) {
        skinItem->Check(true);
        skinItem->Enable(false);
        return; // No non-default skins
    }
 
//    I'd like to put a separator here, but we can't separate radio items
    
    for (i = 0; i < astrSkins.GetCount(); i++) {
        if (astrSkins[i] == pSkinManager->GetDefaultSkinName()) {
            continue;
        }

        skinItem = submenu->AppendRadioItem(
            ID_SGFIRSTSKINSELECTOR + i,
            astrSkins[i]
        );
        if (astrSkins[i] == strSelectedSkin) {
            skinItem->Check(true);
        }
    }
}



void CSimpleFrame::OnSelectDefaultSkin( wxCommandEvent& WXUNUSED(event) ) {
    CSkinManager* pSkinManager = wxGetApp().GetSkinManager();

    wxASSERT(pSkinManager);
    wxASSERT(wxDynamicCast(pSkinManager, CSkinManager));

    // The "Default" skin menu item is localized, but 
    // the name of the default skin is not localized
    pSkinManager->ReloadSkin(pSkinManager->GetDefaultSkinName());
}


void CSimpleFrame::OnSelectSkin( wxCommandEvent& event ){
    CSkinManager *pSkinManager = wxGetApp().GetSkinManager();
    wxMenuItem *oldItem, *selectedItem;
    wxMenuBar *pMenuBar = GetMenuBar();
    int newSkinId = event.GetId();
    int oldSkinID;
    
    wxASSERT(pSkinManager);
    wxASSERT(wxDynamicCast(pSkinManager, CSkinManager));
    
    
    selectedItem = pMenuBar->FindItem(newSkinId);
    if (!selectedItem) return;

    wxString oldSkinName = pSkinManager->GetSelectedSkin();
    wxString newSkinName = selectedItem->GetItemLabelText();
    if (newSkinName == oldSkinName) return;

    if (oldSkinName == pSkinManager->GetDefaultSkinName()) {
        // The "Default" skin menu item is localized, but 
        // the name of the default skin is not localized
        oldSkinID = ID_SGDEFAULTSKINSELECTOR;
    } else {
        oldSkinID = m_pSubmenuSkins->FindItem(oldSkinName);
    }
    oldItem = m_pSubmenuSkins->FindItem(oldSkinID);
    if (oldItem) {
        oldItem->Check(false);
    }

    selectedItem->Check(true);
    pSkinManager->ReloadSkin(newSkinName);
}


void CSimpleFrame::OnPreferences(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnPreferences - Function Begin"));

	m_pBackgroundPanel->SetDlgOpen(true);

	CDlgPreferences dlg(GetParent());
    dlg.ShowModal();

    m_pBackgroundPanel->SetDlgOpen(false);

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnPreferences - Function End"));
}


void CSimpleFrame::OnOptions(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnOptions - Function Begin"));

	m_pBackgroundPanel->SetDlgOpen(true);

// TODO:  Create simple language selection dialog
    CDlgOptions dlg(this);
    dlg.ShowModal();

    m_pBackgroundPanel->SetDlgOpen(false);

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnOptions - Function End"));
}


// TODO: Create ID_HELPBOINCMANAGER web page for each organization for new BOINC version
void CSimpleFrame::OnHelpBOINC(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnHelpBOINC - Function Begin"));

    if (IsShown()) {
    	wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

		wxString wxurl;
		wxurl.Printf(
            wxT("%s?target=simple&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            event.GetId()
        );
		wxLaunchDefaultBrowser(wxurl);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnHelpBOINC - Function End"));
}


void CSimpleFrame::OnHelpAbout(wxCommandEvent& /*event*/) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnHelpAbout - Function Begin"));

	m_pBackgroundPanel->SetDlgOpen(true);

    CDlgAbout dlg(this);
    dlg.ShowModal();

    m_pBackgroundPanel->SetDlgOpen(false);

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnHelpAbout - Function End"));
}


void CSimpleFrame::OnHelp(wxHelpEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnHelp - Function Begin"));

    if (IsShown()) {
    	wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

		wxString wxurl;
		wxurl.Printf(
            wxT("%s?target=simple&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            event.GetId()
        );
        wxLaunchDefaultBrowser(wxurl);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnHelp - Function End"));
}


void CSimpleFrame::OnReloadSkin(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnReloadSkin - Function Start"));
    
    m_pBackgroundPanel->ReskinInterface();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnReloadSkin - Function End"));
}


void CSimpleFrame::OnNotification(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnNotification - Function Begin"));

	CDlgMessages dlg(GetParent());

    m_pBackgroundPanel->SetDlgOpen(true);
    SetMsgsDlgOpen(&dlg);

    m_pBackgroundPanel->NoticesViewed();
    dlg.ShowModal();

    m_pBackgroundPanel->SetDlgOpen(false);
    SetMsgsDlgOpen(NULL);

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnNotification - Function End"));
}


void CSimpleFrame::OnRefreshView(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnRefreshView - Function Start"));
    
    m_pBackgroundPanel->OnFrameRender();
    
    if (dlgMsgsPtr) {
        dlgMsgsPtr->OnRefresh();
    }
    
#ifdef __WXMAC__
    // We disabled tooltips on Mac while menus were popped up because they cover menus
    wxToolTip::Enable(true);
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnRefreshView - Function End"));
}


void CSimpleFrame::OnProjectsAttachToProject() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnProjectsAttachToProject - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {

        CWizardAttach* pWizard = new CWizardAttach(this);

        wxString strName = wxEmptyString;
        wxString strURL = wxEmptyString;
        wxString strTeamName = wxEmptyString;
        pWizard->Run( strName, strURL, strTeamName, false );

        if (pWizard)
            pWizard->Destroy();

    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnProjectsAttachToProject - Function End"));
}


void CSimpleFrame::OnConnect(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnConnect - Function Begin"));
    
    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CWizardAttach*     pWizard = NULL;
    wxString strComputer = wxEmptyString;
    wxString strName = wxEmptyString;
    wxString strURL = wxEmptyString;
    wxString strTeamName = wxEmptyString;
    bool bCachedCredentials = false;
    ACCT_MGR_INFO ami;
    PROJECT_INIT_STATUS pis;
	CC_STATUS     status;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->ForceCacheUpdate();
    pDoc->GetCoreClientStatus(status, true);

#ifdef __WXMAC__
    wxGetApp().GetMacSystemMenu()->BuildMenu();
#endif

	// If we are connected to the localhost, run a really quick screensaver
    //   test to trigger a firewall popup.
    pDoc->GetConnectedComputerName(strComputer);
    if (pDoc->IsComputerNameLocal(strComputer)) {
        wxGetApp().StartBOINCScreensaverTest();
        wxGetApp().StartBOINCDefaultScreensaverTest();
    }


    pDoc->rpc.get_project_init_status(pis);
    pDoc->rpc.acct_mgr_info(ami);
    if (ami.acct_mgr_url.size() && !ami.have_credentials) {
        if (!IsShown()) {
            Show();
        }

        pWizard = new CWizardAttach(this);
        if (pWizard->SyncToAccountManager()) {
            // If successful, hide the main window
            Hide();
        }
    } else if ((pis.url.size() || (0 >= pDoc->GetSimpleProjectCount())) && !status.disallow_attach) {
        if (!IsShown()) {
            Show();
        }

        strName = wxString(pis.name.c_str(), wxConvUTF8);
        strURL = wxString(pis.url.c_str(), wxConvUTF8);
        strTeamName = wxString(pis.team_name.c_str(), wxConvUTF8);
        bCachedCredentials = pis.url.length() && pis.has_account_key;

        pWizard = new CWizardAttach(this);
        pWizard->Run(strName, strURL, strTeamName, bCachedCredentials);
    }

 	if (pWizard) {
        pWizard->Destroy();
        m_pBackgroundPanel->UpdateProjectView();
	}

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnConnect - Function End"));
}


void CSimpleFrame::OnEventLog(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnEventLog - Function Begin"));

    wxGetApp().DisplayEventLog();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnEventLog - Function End"));
}


IMPLEMENT_DYNAMIC_CLASS(CSimpleGUIPanel, wxPanel)

BEGIN_EVENT_TABLE(CSimpleGUIPanel, wxPanel)
    EVT_SIZE(CSimpleGUIPanel::OnSize)
    EVT_ERASE_BACKGROUND(CSimpleGUIPanel::OnEraseBackground)    
	EVT_BUTTON(ID_SGNOTICESBUTTON,CSimpleGUIPanel::OnShowNotices)
	EVT_BUTTON(ID_SGSUSPENDRESUMEBUTTON,CSimpleGUIPanel::OnSuspendResume)
	EVT_BUTTON(ID_SIMPLE_HELP,CSimpleGUIPanel::OnHelp)
	EVT_TIMER(ID_SIMPLEMESSAGECHECKTIMER, CSimpleGUIPanel::OnCheckForNewNotices)
    EVT_PAINT(CSimpleGUIPanel::OnPaint)
END_EVENT_TABLE()


CSimpleGUIPanel::CSimpleGUIPanel() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::CSimpleGUIPanel - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::CSimpleGUIPanel - Default Constructor Function End"));
}


CSimpleGUIPanel::CSimpleGUIPanel(wxWindow* parent) : 
    wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN | wxBORDER_NONE)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::CSimpleGUIPanel - Overloaded Constructor Function Begin"));

    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    m_taskPanel = NULL;
    m_projPanel = NULL;
    m_oldWorkCount = 0;
    m_bNewNoticeAlert = false;
    m_bNoticesButtonIsRed = false;
    m_irefreshCount = 0;
    
	checkForNewNoticesTimer = new wxTimer(this, ID_SIMPLEMESSAGECHECKTIMER);
	checkForNewNoticesTimer->Start(5000); 

	dlgOpen = false;
    m_sSuspendString = _("Suspend");
    m_sResumeString = _("Resume");
    m_sSuspendButtonToolTip = _("Suspend Computing");
    m_sResumeButtonToolTip = _("Resume Computing");

	m_taskPanel = new CSimpleTaskPanel(this);
    m_projPanel = new CSimpleProjectPanel(this);

    // Box Sizer
	mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->AddSpacer(68);
    mainSizer->Add(m_taskPanel, 1, wxLEFT | wxRIGHT | wxEXPAND | wxALIGN_CENTER, SIDEMARGINS);
	mainSizer->AddSpacer(8);
    mainSizer->Add(m_projPanel, 0, wxLEFT | wxRIGHT | wxEXPAND | wxALIGN_CENTER, SIDEMARGINS);
	mainSizer->AddSpacer(8);

	wxBoxSizer* buttonsSizer;
	buttonsSizer = new wxBoxSizer( wxHORIZONTAL );

	m_NoticesButton = new wxButton( this, ID_SGNOTICESBUTTON, _("Notices"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoticesButton->SetToolTip( _("Open a window to view notices from projects or BOINC"));
	buttonsSizer->Add( m_NoticesButton, 0, wxEXPAND | wxALIGN_LEFT, 0 );
    buttonsSizer->AddStretchSpacer();

    int suspendWidth, resumeWidth, y;
    GetTextExtent(m_sSuspendString, &suspendWidth, &y);
    GetTextExtent(m_sResumeString, &resumeWidth, &y);
    
    m_bIsSuspended = suspendWidth > resumeWidth;
    m_SuspendResumeButton = new wxButton( this, ID_SGSUSPENDRESUMEBUTTON, 
                            m_bIsSuspended ? m_sSuspendString : m_sResumeString,
                            wxDefaultPosition, wxDefaultSize, 0 );
    m_SuspendResumeButton->SetToolTip(wxEmptyString);
    
	buttonsSizer->Add( m_SuspendResumeButton, 0, wxEXPAND | wxALIGN_RIGHT, 0 );
    buttonsSizer->AddStretchSpacer();

    m_HelpButton = new wxButton( this, ID_SIMPLE_HELP, _("Help"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonsSizer->Add( m_HelpButton, 0, wxEXPAND | wxALIGN_RIGHT, 0 );

    wxString helpTip;
    helpTip.Printf(_("Get help with %s"), pSkinAdvanced->GetApplicationShortName().c_str());
    m_HelpButton->SetToolTip(helpTip);

	mainSizer->Add( buttonsSizer, 0, wxLEFT | wxRIGHT | wxEXPAND, 2 * SIDEMARGINS );
	mainSizer->AddSpacer(10);

	SetSizer(mainSizer);
    Layout();
    
    mainSizer->Fit(GetParent());

    SetBackgroundBitmap();   

#ifdef __WXMAC__
    // Tell accessibility aids to ignore this panel (but not its contents)
    HIObjectSetAccessibilityIgnored((HIObjectRef)GetHandle(), true);
    
    SInt32 response;
    OSStatus err = Gestalt(gestaltSystemVersion, &response);
    if ((err == noErr) && (response >= 0x1070)) {
        m_iRedRingRadius = 4;
    } else {
        m_iRedRingRadius = 12;
    }
#endif    

    m_SuspendResumeButton->Disable();
    OnFrameRender();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::CSimpleGUIPanel - Overloaded Constructor Function End"));
}


CSimpleGUIPanel::~CSimpleGUIPanel()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::CSimpleGUIPanel - Destructor Function Begin"));
    
    checkForNewNoticesTimer->Stop();
	delete checkForNewNoticesTimer;
    m_bmpBg = wxNullBitmap; // Deletes old bitmap via reference counting
    
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::CSimpleGUIPanel - Destructor Function End"));
}


void CSimpleGUIPanel::SetBackgroundBitmap() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::SetBackgroundBitmap - Function Start"));

    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxColour bgColor(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    SetBackgroundColour(bgColor);

    wxRect panelRect = GetRect();
    m_bmpBg = wxBitmap(panelRect.width, panelRect.height);
    wxMemoryDC dc(m_bmpBg);
    wxBrush bgBrush(bgColor);
    dc.SetBackground(bgBrush);
    dc.Clear();
    dc.DrawBitmap(*pSkinSimple->GetBackgroundImage()->GetBitmap(), 0, 0, false);

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::SetBackgroundBitmap - Function End"));
}

void CSimpleGUIPanel::ReskinInterface() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::ReskinInterface - Function Start"));

    Freeze();
    //bg color
    m_bmpBg = wxNullBitmap; // Deletes old bitmap via reference counting
    SetBackgroundBitmap();

    m_taskPanel->ReskinInterface();
	m_projPanel->ReskinInterface();

    Thaw();
    Refresh();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::ReskinInterface - Function End"));
}


void CSimpleGUIPanel::OnProjectsAttachToProject() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::OnProjectsAttachToProject - Function Begin"));
	
    CSimpleFrame* pFrame = wxDynamicCast(GetParent(), CSimpleFrame);
    wxASSERT(pFrame);

    pFrame->OnProjectsAttachToProject();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnProjectsAttachToProject - Function End"));
}


// called from CSimpleFrame::OnRefreshView()
void CSimpleGUIPanel::OnFrameRender() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    int                 workCount = pDoc->GetSimpleGUIWorkCount();
	CC_STATUS           status;
    bool                isSuspended;

    // OnFrameRender() may be called while SimpleGUI initialization is 
    // in progress due to completion of a periodic get_messages RPC, 
    // causing unintended recursion in CMainDocument::RequestRPC().  
    // Check for that situation here.
    if (pDoc->WaitingForRPC()) return;

    // Workaround for Linux refresh problem
    if (m_irefreshCount < REFRESH_WAIT) {
        ++m_irefreshCount;
        m_taskPanel->UpdatePanel(true);
        return;
    }
	
    if (workCount != m_oldWorkCount) {
        if (workCount < 0) {
            m_projPanel->Hide();
        } else if (m_oldWorkCount == 0) {
            m_projPanel->Show();
        }
        this->Layout();
        ReskinInterface();
    }
    
    if (IsShown()) {
	    if ( pDoc->IsConnected() ) {
        
            // Show Resume or Suspend as appropriate
            pDoc->GetCoreClientStatus(status);

            isSuspended = (RUN_MODE_NEVER == status.task_mode);
            if ((isSuspended != m_bIsSuspended) || (!m_SuspendResumeButton->IsEnabled())) {
                m_bIsSuspended = isSuspended;
                m_SuspendResumeButton->SetLabel(m_bIsSuspended ? m_sResumeString : m_sSuspendString);
                m_SuspendResumeButton->SetToolTip(m_bIsSuspended ? m_sResumeButtonToolTip : m_sSuspendButtonToolTip);
            }
            m_SuspendResumeButton->Enable();
	    } else {
            m_SuspendResumeButton->SetToolTip(wxEmptyString);
            m_SuspendResumeButton->Disable();
        }

		UpdateProjectView();

        if (m_bNewNoticeAlert) {
            wxRect r = m_NoticesButton->GetRect();
            r.Inflate(4, 4);
            RefreshRect(r, m_bNoticesButtonIsRed);
            m_bNoticesButtonIsRed = !m_bNoticesButtonIsRed;
        }


        // State changes can cause the BSG to crash if a dialogue is open.
        // Defer state change until after the dialogue is closed
        if ( dlgOpen ) {
            return;
        }
        
        m_oldWorkCount = workCount;
        
        m_taskPanel->UpdatePanel(false);
    }
}


void CSimpleGUIPanel::UpdateProjectView()
{
	//update Project Panel
    m_projPanel->UpdateInterface();
}


void CSimpleGUIPanel::OnCheckForNewNotices(wxTimerEvent& WXUNUSED(event)) {
	CMainDocument* pDoc = wxGetApp().GetDocument();
	if ( pDoc->GetUnreadNoticeCount() ) {
        m_bNewNoticeAlert = true;
        checkForNewNoticesTimer->Stop();
	}
}


void CSimpleGUIPanel::NoticesViewed() {
	CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);

	m_bNewNoticeAlert = false;
    m_bNoticesButtonIsRed = false;
    wxRect r = m_NoticesButton->GetRect();
    r.Inflate(4, 4);
    RefreshRect(r, true);
    m_bNoticesButtonIsRed = !m_bNoticesButtonIsRed;
	pDoc->UpdateUnreadNoticeState();
	checkForNewNoticesTimer->Start();
}


void CSimpleGUIPanel::OnShowNotices(wxCommandEvent& /*event*/) {
    NoticesViewed();

	CDlgMessages dlg(GetParent());
    SetDlgOpen(true);
    
    ((CSimpleFrame*)GetParent())->SetMsgsDlgOpen(&dlg);
    
    dlg.ShowModal();

    SetDlgOpen(false);
    ((CSimpleFrame*)GetParent())->SetMsgsDlgOpen(NULL);
}


void CSimpleGUIPanel::OnSuspendResume(wxCommandEvent& /*event*/) {
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    CC_STATUS ccs;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (m_bIsSuspended) {
        pDoc->GetCoreClientStatus(ccs);
        
        if ((RUN_MODE_NEVER == ccs.task_mode) && (0 >= ccs.task_mode_delay)) {
            pDoc->SetActivityRunMode(RUN_MODE_AUTO, 0);
        } else {
            pDoc->SetActivityRunMode(RUN_MODE_RESTORE, 0);
        }
    } else {
        pDoc->SetActivityRunMode(RUN_MODE_NEVER, 3600);
    }
    
    m_SuspendResumeButton->SetLabel(m_bIsSuspended ? m_sResumeString : m_sSuspendString);
    m_SuspendResumeButton->SetToolTip(m_bIsSuspended ? m_sResumeButtonToolTip : m_sSuspendButtonToolTip);
}


// TODO: Create ID_SIMPLE_HELP web page for each organization for new BOINC version
void CSimpleGUIPanel::OnHelp(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::OnHelp - Function Begin"));

	wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

    wxString wxurl;
    wxurl.Printf(
        wxT("%s?target=simple&version=%s&controlid=%d"),
        strURL.c_str(),
        wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
        ID_SIMPLE_HELP        

    );

    wxLaunchDefaultBrowser(wxurl);
        
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleGUIPanel::OnHelp - Function End"));
}


void CSimpleGUIPanel::OnPaint(wxPaintEvent& WXUNUSED(event)) {
	wxPaintDC myDC(this);

    if (m_bNewNoticeAlert) {
        wxRect r = m_NoticesButton->GetRect();
        if (m_bNoticesButtonIsRed) {
            wxPen oldPen = myDC.GetPen();
            wxBrush oldBrush = myDC.GetBrush();
            int oldMode = myDC.GetBackgroundMode();
            wxPen bgPen(*wxRED, 3);
            myDC.SetBackgroundMode(wxSOLID);
            myDC.SetPen(bgPen);
            myDC.SetBrush(*wxTRANSPARENT_BRUSH);
#ifdef __WXMAC__
            r.Inflate(2, 2);
            myDC.DrawRoundedRectangle(r.x, r.y, r.width, r.height+1, m_iRedRingRadius);
#elif defined(__WXMSW__)
            r.Inflate(3, 3);
            myDC.DrawRectangle(r.x, r.y, r.width, r.height);
#else
            r.Inflate(3, 3);
            myDC.DrawRoundedRectangle(r.x, r.y, r.width, r.height, 6);
#endif
            // Restore Mode, Pen and Brush 
            myDC.SetBackgroundMode(oldMode);
            myDC.SetPen(oldPen);
            myDC.SetBrush(oldBrush);
        }
    }
}


// We don't reliably get EraseBackground events under Linux, 
// so there is a workaround at CSimplePanelBase::MakeBGBitMap()
void CSimpleGUIPanel::OnEraseBackground(wxEraseEvent& event) {
    wxDC *dc = event.GetDC();
    
#ifdef __WXMAC__
    // Avoid unnecessary drawing due to Mac progress indicator's animation
    wxRect clipRect;
    wxRect taskRect = m_taskPanel->GetRect();
    dc->GetClippingBox(&clipRect.x, &clipRect.y, &clipRect.width, &clipRect.height);
    if (clipRect.IsEmpty() || taskRect.Contains(clipRect)) {
        return;
    }
#endif
    dc->DrawBitmap(m_bmpBg, 0, 0);
}

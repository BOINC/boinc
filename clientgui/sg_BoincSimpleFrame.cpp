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


IMPLEMENT_DYNAMIC_CLASS(CSimpleFrame, CBOINCBaseFrame)

BEGIN_EVENT_TABLE(CSimpleFrame, CBOINCBaseFrame)
    EVT_SIZE(CSimpleFrame::OnSize)
    EVT_MENU(ID_CHANGEGUI, CSimpleFrame::OnChangeGUI)
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

    // View menu
    wxMenu *menuTools = new wxMenu;

    menuTools->Append(
        ID_CHANGEGUI,
        _("Advanced View...\tCtrl+Shift+A"),
        _("Display the advanced graphical interface.")
    );

    menuTools->Append(
        ID_PREFERENCES,
        _("Computing &preferences..."),
        _("Configure computing preferences")
    );
    
    menuTools->Append(
        ID_SGOPTIONS, 
        _("Manager Settings..."),
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

#ifdef __WXMAC__
    m_pMenubar->MacInstallMenuBar();
    MacLocalizeBOINCMenu();

    // Enable Mac OS X's standard Preferences menu item (handled in MacSysMenu.cpp)
    EnableMenuCommand(NULL, kHICommandPreferences);
#endif

    m_Shortcuts[0].Set(wxACCEL_NORMAL, WXK_HELP, ID_HELPBOINCMANAGER);
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

// TODO:  Get code from CAdvancedFrame::OnOptions() or create simple language selection dialog

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
    
    static bool bAlreadyRunning = false;
    
    if (bAlreadyRunning) return;
    bAlreadyRunning = true;
    
    m_pBackgroundPanel->OnFrameRender();
    
    if (dlgMsgsPtr) {
        dlgMsgsPtr->OnRefresh();
    }
    
#ifdef __WXMAC__
    // We disabled tooltips on Mac while menus were popped up because they cover menus
    wxToolTip::Enable(true);
#endif

    bAlreadyRunning = false;

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


IMPLEMENT_DYNAMIC_CLASS(CSimpleGUIPanel, wxPanel)

BEGIN_EVENT_TABLE(CSimpleGUIPanel, wxPanel)
    EVT_SIZE(CSimpleGUIPanel::OnSize)
    EVT_ERASE_BACKGROUND(CSimpleGUIPanel::OnEraseBackground)    
	EVT_BUTTON(ID_SGNOTICESBUTTON,CSimpleGUIPanel::OnShowNotices)
	EVT_BUTTON(ID_SGPAUSERESUMEBUTTON,CSimpleGUIPanel::OnPauseResume)
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
    
	checkForNewNoticesTimer = new wxTimer(this, ID_SIMPLEMESSAGECHECKTIMER);
	checkForNewNoticesTimer->Start(5000); 

	dlgOpen = false;
    m_sPauseString = _("Pause");
    m_sResumeString = _("Resume");
    m_sPauseButtonToolTip = _("Stop all activity");
    m_sResumeButtonToolTip = _("Resume activity");


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

    int pauseWidth, resumeWidth, y;
    GetTextExtent(m_sPauseString, &pauseWidth, &y);
    GetTextExtent(m_sResumeString, &resumeWidth, &y);
    m_PauseResumeButton = new wxButton( this, ID_SGPAUSERESUMEBUTTON, 
                            (pauseWidth > resumeWidth) ? m_sPauseString : m_sResumeString,
                            wxDefaultPosition, wxDefaultSize, 0 );
    
	buttonsSizer->Add( m_PauseResumeButton, 0, wxEXPAND | wxALIGN_RIGHT, 0 );
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
    
    m_bisPaused = false;
    m_PauseResumeButton->SetLabel(m_sPauseString);
    m_PauseResumeButton->SetToolTip(m_sPauseButtonToolTip);

    SetBackgroundBitmap();   

#ifdef __WXMAC__
    // Tell accessibility aids to ignore this panel (but not its contents)
    HIObjectSetAccessibilityIgnored((HIObjectRef)GetHandle(), true);
#endif    

    m_projPanel->Hide();
    m_PauseResumeButton->Disable();
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
    bool                isPaused;

    // OnFrameRender() may be called while SimpleGUI initialization is 
    // in progress due to completion of a periodic get_messages RPC, 
    // causing unintended recursion in CMainDocument::RequestRPC().  
    // Check for that situation here.
    if (pDoc->WaitingForRPC()) return;

        if (workCount != m_oldWorkCount) {
            if (workCount <= 0) {
                m_projPanel->Hide();
            } else if (m_oldWorkCount == 0) {
                m_projPanel->Show();
            }
        }
    
    if (IsShown()) {
	    if ( pDoc->IsConnected() ) {
        
            // Show resume or pause as appropriate
            pDoc->GetCoreClientStatus(status);

            isPaused = (RUN_MODE_NEVER == status.task_mode);
            if (isPaused != m_bisPaused) {
                m_bisPaused = isPaused;
                m_PauseResumeButton->SetLabel(m_bisPaused ? m_sResumeString : m_sPauseString);
                m_PauseResumeButton->SetToolTip(m_bisPaused ? m_sResumeButtonToolTip : m_sPauseButtonToolTip);
            }
            m_PauseResumeButton->Enable();
		    UpdateProjectView();
	    } else {
            m_PauseResumeButton->SetToolTip(wxEmptyString);
            m_PauseResumeButton->Disable();
        }

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
        
        m_taskPanel->Update();
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


void CSimpleGUIPanel::OnPauseResume(wxCommandEvent& /*event*/) {
    CMainDocument* pDoc      = wxGetApp().GetDocument();
    CC_STATUS ccs;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (m_bisPaused) {
        pDoc->GetCoreClientStatus(ccs);
        
        if ((RUN_MODE_NEVER == ccs.task_mode) && (0 >= ccs.task_mode_delay)) {
            pDoc->SetActivityRunMode(RUN_MODE_AUTO, 0);
        } else {
            pDoc->SetActivityRunMode(RUN_MODE_RESTORE, 0);
        }
    } else {
        pDoc->SetActivityRunMode(RUN_MODE_NEVER, 3600);
    }
    
    m_PauseResumeButton->SetLabel(m_bisPaused ? m_sResumeString : m_sPauseString);
    m_PauseResumeButton->SetToolTip(m_bisPaused ? m_sResumeButtonToolTip : m_sPauseButtonToolTip);
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


void CSimpleGUIPanel::OnPaint(wxPaintEvent& event) {
	wxPaintDC myDC(this);

    if (m_bNewNoticeAlert) {
        wxRect r = m_NoticesButton->GetRect();
        r.Inflate(3, 3);
        if (m_bNoticesButtonIsRed) {
            wxPen oldPen = myDC.GetPen();
            wxBrush oldBrush = myDC.GetBrush();
            int oldMode = myDC.GetBackgroundMode();
            wxPen bgPen(*wxRED, 3);
            myDC.SetBackgroundMode(wxSOLID);
            myDC.SetPen(bgPen);
            myDC.SetBrush(*wxTRANSPARENT_BRUSH);
#ifdef __WXMAC__
            myDC.DrawRoundedRectangle(r.x, r.y, r.width, r.height, 12);
#else
            myDC.DrawRectangle(r.x, r.y, r.width, r.height);
#endif
            // Restore Mode, Pen and Brush 
            myDC.SetBackgroundMode(oldMode);
            myDC.SetPen(oldPen);
            myDC.SetBrush(oldBrush);
        }
    }
}


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

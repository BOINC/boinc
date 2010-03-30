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
#pragma implementation "sg_BoincSimpleGUI.h"
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
#include "common/wxFlatNotebook.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "Events.h"
#include "BOINCBaseFrame.h"
#include "wizardex.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "error_numbers.h"
#include "version.h"

#include "sg_BoincSimpleGUI.h"
#include "sg_ImageLoader.h"
#include "sg_ProjectsComponent.h"
#include "sg_ClientStateIndicator.h"
#include "sg_StatImageLoader.h"
#include "sg_ViewTabPage.h"
#include "sg_DlgMessages.h"
#include "DlgEventLog.h"


IMPLEMENT_DYNAMIC_CLASS(CSimpleFrame, CBOINCBaseFrame)

BEGIN_EVENT_TABLE(CSimpleFrame, CBOINCBaseFrame)
    EVT_SIZE(CSimpleFrame::OnSize)
    EVT_MENU(ID_CHANGEGUI, CSimpleFrame::OnChangeGUI)
    EVT_HELP(wxID_ANY, CSimpleFrame::OnHelp)
    EVT_FRAME_CONNECT(CSimpleFrame::OnConnect)
    EVT_FRAME_RELOADSKIN(CSimpleFrame::OnReloadSkin)
    // We can't eliminate the Mac Help menu, so we might as well make it useful.
    EVT_MENU(ID_HELPBOINC, CSimpleFrame::OnHelpBOINC)
    EVT_MENU(ID_HELPBOINCMANAGER, CSimpleFrame::OnHelpBOINC)
    EVT_MENU(ID_HELPBOINCWEBSITE, CSimpleFrame::OnHelpBOINC)
END_EVENT_TABLE()

CSimpleFrame::CSimpleFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function End"));
}


CSimpleFrame::CSimpleFrame(wxString title, wxIcon* icon, wxIcon* icon32, wxPoint position, wxSize size) : 
    CBOINCBaseFrame((wxFrame *)NULL, ID_SIMPLEFRAME, title, position, size,
                    wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Overloaded Constructor Function Begin"));

    // Initialize Application
    wxIconBundle icons;
    icons.AddIcon(*icon);
    icons.AddIcon(*icon32);
    SetIcons(icons);
    
#ifdef __WXMAC__
    // We can't eliminate the Mac menu bar or the Help menu, so we 
    // might as well make them useful.
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
    wxMenu *menuView = new wxMenu;

    menuView->Append(
        ID_CHANGEGUI,
        _("Advanced View...\tCtrl+Shift+A"),
        _("Display the advanced (accessible) graphical interface.")
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

    // Clear menubar
    m_pMenubar = new wxMenuBar;
    m_pMenubar->Append(
        menuHelp,
        _("&Help")
    );
    
    SetMenuBar(m_pMenubar);
    m_pMenubar->MacInstallMenuBar();
    ::ClearMenuBar();       // Remove everything we can from the Mac menu bar
    
    m_pMenubar->Insert(     // Must be done after ClearMenuBar()
        0,
        menuFile,
        _("&File")
    );
    
    m_pMenubar->Append(
        menuView,
        _("&View")
    );

    m_Shortcuts[0].Set(wxACCEL_NORMAL, WXK_HELP, ID_HELPBOINCMANAGER);
    m_pAccelTable = new wxAcceleratorTable(2, m_Shortcuts);
#else
    m_Shortcuts[0].Set(wxACCEL_CTRL|wxACCEL_SHIFT, (int)'A', ID_CHANGEGUI);
    m_pAccelTable = new wxAcceleratorTable(1, m_Shortcuts);
#endif

    SetAcceleratorTable(*m_pAccelTable);
    
    dlgMsgsPtr = NULL;
    m_pBackgroundPanel = new CSimplePanel(this);
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


int CSimpleFrame::_GetCurrentViewPage() {
    if (isMessagesDlgOpen()) {
        return VW_SGUI | VW_SMSG;
    } else {
        return VW_SGUI;
    }
    return 0;       // Should never happen.
}


void CSimpleFrame::OnChangeGUI(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnChangeGUI - Function Begin"));

    wxGetApp().SetActiveGUI(BOINC_ADVANCEDGUI, true);

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnChangeGUI - Function End"));
}


void CSimpleFrame::OnHelpBOINC(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINC - Function Begin"));

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

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINC - Function End"));
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


void CSimpleFrame::OnRefreshView(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnRefreshView - Function Start"));
    
    static bool bAlreadyRunning = false;
    wxTimerEvent    timerEvent;
    
    if (bAlreadyRunning) return;
    bAlreadyRunning = true;
    
    m_pBackgroundPanel->OnFrameRender();
    
    if (dlgMsgsPtr) {
        dlgMsgsPtr->OnRefresh();
    }

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

        CWizardAttachProject* pWizard = new CWizardAttachProject(this);

        wxString strName = wxEmptyString;
        wxString strURL = wxEmptyString;
        std::string foo;
        pWizard->Run( strName, strURL, foo, false );

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
    CWizardAttachProject* pAPWizard = NULL;
    wxString strComputer = wxEmptyString;
    wxString strName = wxEmptyString;
    wxString strURL = wxEmptyString;
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

    pAPWizard = new CWizardAttachProject(this);

    pDoc->rpc.get_project_init_status(pis);
    pDoc->rpc.acct_mgr_info(ami);
    if (ami.acct_mgr_url.size() && !ami.have_credentials) {
        if (!IsShown()) {
            Show();
        }

       if (pAPWizard->SyncToAccountManager()) {
            // If successful, hide the main window
            Hide();
        }
    } else if ((pis.url.size() || (0 >= pDoc->GetSimpleProjectCount())) && !status.disallow_attach) {
        if (!IsShown()) {
            Show();
        }

        strName = wxString(pis.name.c_str(), wxConvUTF8);
        strURL = wxString(pis.url.c_str(), wxConvUTF8);
        bCachedCredentials = pis.url.length() && pis.has_account_key;

        pAPWizard->Run(strName, strURL, pis.team_name, bCachedCredentials);
    }

 	if (pAPWizard){
            pAPWizard->Destroy();
            //update Project Component
            m_pBackgroundPanel->UpdateProjectView();
	}

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnConnect - Function End"));
}


IMPLEMENT_DYNAMIC_CLASS(CSimplePanel, wxPanel)

BEGIN_EVENT_TABLE(CSimplePanel, wxPanel)
    EVT_SIZE(CSimplePanel::OnSize)
    EVT_ERASE_BACKGROUND(CSimplePanel::OnEraseBackground)
END_EVENT_TABLE()


CSimplePanel::CSimplePanel() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Default Constructor Function End"));
}


CSimplePanel::CSimplePanel(wxWindow* parent) : 
    wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Overloaded Constructor Function Begin"));

    wrkUnitNB = NULL;
    clientState = NULL;
    projComponent = NULL;

	projectViewInitialized = false;
	emptyViewInitialized = false;
	notebookViewInitialized = false;
	dlgOpen = false;

	InitEmptyView();

#ifdef __WXMAC__
    // Have the screen reader tell user to switch to advanced view.
    oldSimpleGUIWorkCount = -1;
    
    SetupMacAccessibilitySupport();
#endif    

    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Overloaded Constructor Function End"));
}


CSimplePanel::~CSimplePanel()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Destructor Function Begin"));
    
#ifdef __WXMAC__
    RemoveMacAccessibilitySupport();
#endif    

    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Destructor Function End"));
}


void CSimplePanel::ReskinInterface() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::ReskinInterface - Function Start"));

    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    Freeze();
    //bg color
    SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());

    if(notebookViewInitialized){
		if (wrkUnitNB) wrkUnitNB->ReskinAppGUI();
	} else {
		if (clientState) clientState->ReskinInterface();
	}

    //reskin component 
	if (projComponent) projComponent->ReskinInterface();

    Thaw();
    Refresh();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::ReskinInterface - Function End"));
}


void CSimplePanel::OnProjectsAttachToProject() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::OnProjectsAttachToProject - Function Begin"));
	
    CSimpleFrame* pFrame = wxDynamicCast(GetParent(), CSimpleFrame);
    wxASSERT(pFrame);

    pFrame->OnProjectsAttachToProject();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnProjectsAttachToProject - Function End"));
}


// called from CSimpleFrame::OnRefreshView()
void CSimplePanel::OnFrameRender() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    int                 workCount = pDoc->GetSimpleGUIWorkCount();

    // OnFrameRender() may be called while SimpleGUI initialization is 
    // in progress due to completion of a periodic get_messages RPC, 
    // causing unintended recursion in CMainDocument::RequestRPC().  
    // Check for that situation here.
    if (pDoc->WaitingForRPC()) return;

    if (IsShown()) {

	    if (!projectViewInitialized) {
		    InitProjectView();
		    return;
	    } else if ( pDoc->IsConnected() ) {
		    UpdateProjectView();
	    }

	    // Now check to see if we show the empty state or results
	    if ( workCount > 0 ) {
		    // State changes can cause the BSG to crash if a dialogue is open.
		    // Defer state change until after the dialogue is closed
		    if ( (emptyViewInitialized || !notebookViewInitialized) && dlgOpen ) {
			    return;
		    }

		    // If empty was displayed, remove
		    if ( emptyViewInitialized ) {
			    DestroyEmptyView();
		    }
		    // If we hadn't previously shown the notebook, create it.
		    if ( !notebookViewInitialized ) {
			    InitNotebook();
		    }
		    wrkUnitNB->Update();
	    } else {
		    // State changes can cause the BSG to crash if a dialogue is open.
		    // Defer state change until after the dialogue is closed
		    if ( (!emptyViewInitialized || notebookViewInitialized) && dlgOpen ) {
			    return;
		    }

		    if ( notebookViewInitialized ) {
			    DestroyNotebook();
		    }
		    if ( !emptyViewInitialized ) {
			    InitEmptyView();
		    }
		    UpdateEmptyView();
	    }
            
#ifdef __WXMAC__
        //Accessibility
        // Hide all but top level view from accessibility support so that 
        // the screen reader will tell user to switch to advanced view.
        if (oldSimpleGUIWorkCount != workCount) {
            oldSimpleGUIWorkCount = workCount;
            HIViewRef simple = (HIViewRef)GetHandle();
            AccessibilityIgnoreAllChildren(simple, 1);
        }
#endif
    }
}


void CSimplePanel::UpdateEmptyView() {
	clientState->DisplayState();
}


void CSimplePanel::DestroyEmptyView() {
	delete clientState;
	clientState = NULL;
	emptyViewInitialized = false;
}


void CSimplePanel::InitEmptyView()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	//Set Background color
    SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());

	// Flex Grid Sizer
	mainSizer = new wxFlexGridSizer(3,2);
	SetSizer(mainSizer);

	clientState = new ClientStateIndicator(this,wxPoint(31,94));
	clientState->DisplayState();
	
	emptyViewInitialized = true;
}


void CSimplePanel::UpdateProjectView()
{
	//update Project Component
    projComponent->UpdateInterface();
}


void CSimplePanel::DestroyNotebook() {
	mainSizer->Detach(wrkUnitNB);
	delete wrkUnitNB;
	wrkUnitNB = NULL;
	notebookViewInitialized = false;
}


void CSimplePanel::InitProjectView()
{
	// Do not update screen at this point
	/////////////// MY PROJECTS COMPONENT /////////////////////
    projComponent = new CProjectsComponent(this,wxPoint(31,414));
	///////////////////////////////////////////////////////////
	projectViewInitialized = true;
}


void CSimplePanel::InitNotebook()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::InitNotebook - Function Start"));
	// FlatNotebook
	wrkUnitNB = new WorkunitNotebook(this, -1, wxDefaultPosition, wxSize(370,330), wxFNB_TABS_BORDER_SIMPLE | wxFNB_NO_X_BUTTON | wxFNB_NO_NAV_BUTTONS | wxFNB_FANCY_TABS | wxFNB_NODRAG );
    SetSizer(mainSizer);
	mainSizer->Add(31, 68,0);
	mainSizer->Add(343, 68,0);
	mainSizer->Add(31, 68,0);
	mainSizer->Add(0, 0,1);
	mainSizer->Add(wrkUnitNB);
	mainSizer->Layout();
	notebookViewInitialized = true;
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::InitNotebook - Function End"));
}


void CSimplePanel::OnEraseBackground(wxEraseEvent& event){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxDC *dc = event.GetDC();
    dc->DrawBitmap(*pSkinSimple->GetBackgroundImage()->GetBitmap(), 0, 0);
}



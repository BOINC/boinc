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
#pragma implementation "sg_BoincSimpleGUI.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "common/wxFlatNotebook.h"
#include "common/wxAnimate.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "Events.h"
#include "BOINCBaseFrame.h"
#include "wizardex.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "error_numbers.h"

#include "sg_BoincSimpleGUI.h"
#include "sg_ImageLoader.h"
#include "sg_ProjectsComponent.h"
#include "sg_ClientStateIndicator.h"
#include "sg_StatImageLoader.h"
#include "sg_ViewTabPage.h"


IMPLEMENT_DYNAMIC_CLASS(CSimpleFrame, CBOINCBaseFrame)

BEGIN_EVENT_TABLE(CSimpleFrame, CBOINCBaseFrame)
    EVT_SIZE(CSimpleFrame::OnSize)
	EVT_ERASE_BACKGROUND(CSimpleFrame::OnEraseBackground)
    EVT_FRAME_CONNECT(CSimpleFrame::OnConnect)
    EVT_FRAME_RELOADSKIN(CSimpleFrame::OnReloadSkin)
	EVT_TIMER(ID_SIMPLEFRAMERENDERTIMER, CSimpleFrame::OnFrameRender)
END_EVENT_TABLE()


CSimpleFrame::CSimpleFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function End"));
}


CSimpleFrame::CSimpleFrame(wxString title, wxIcon* icon) : 
    CBOINCBaseFrame((wxFrame *)NULL, ID_SIMPLEFRAME, title, wxDefaultPosition, wxSize(416, 570),
                    wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Overloaded Constructor Function Begin"));

    wrkUnitNB = NULL;
    clientState = NULL;
    projComponent = NULL;

	projectViewInitialized = false;
	emptyViewInitialized = false;
	notebookViewInitialized = false;
	dlgOpen = false;

	RestoreState();

    // Initialize Application
    SetIcon(*icon);
	
	//set polling timer for interface
	m_pFrameRenderTimer = new wxTimer(this, ID_SIMPLEFRAMERENDERTIMER);
    wxASSERT(m_pFrameRenderTimer);
    m_pFrameRenderTimer->Start(1000);                // Send event every 1 second

	InitEmptyView();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Overloaded Constructor Function End"));
}


CSimpleFrame::~CSimpleFrame()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Destructor Function Begin"));
    wxASSERT(m_pFrameRenderTimer);

	SaveState();

	if (m_pFrameRenderTimer) {
        m_pFrameRenderTimer->Stop();
        delete m_pFrameRenderTimer;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Destructor Function End"));
}


bool CSimpleFrame::RestoreState() {
	CBOINCBaseFrame::RestoreState();
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

	// Read the last coordinates of the BSG
	int x = pConfig->Read(wxT("X_Position"), ((wxPoint) wxDefaultPosition).x);
	int y = pConfig->Read(wxT("Y_Position"), ((wxPoint) wxDefaultPosition).y);
	
	// If either co-ordinate is less then 0 then set it equal to 0 to ensure
	// it displays on the screen.
	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;

	// Read the size of the screen
	int maxX = wxSystemSettings::GetMetric( wxSYS_SCREEN_X );
	int maxY = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y );

	// Read the size of the BSG
	int width, height;
	GetSize(&width, &height);

	// Max sure that it doesn't go off to the right or bottom
	if ( x + width > maxX ) x=maxX-width;
	if ( y + height > maxY ) y=maxY-height;

	Move(x,y);
	

	return true;
}


bool CSimpleFrame::SaveState() {
	CBOINCBaseFrame::SaveState();
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
	wxString        strBaseConfigLocation = wxString(wxT("/"));

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

	int x,y;
	GetPosition(&x, &y);
	pConfig->Write(wxT("X_Position"), x);
	pConfig->Write(wxT("Y_Position"), y);
	return true;
}


void CSimpleFrame::OnConnect(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnConnect - Function Begin"));
    
    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CWizardAccountManager* pAMWizard = NULL;
    CWizardAttachProject* pAPWizard = NULL;
    wxString strComputer = wxEmptyString;
    wxString strName = wxEmptyString;
    wxString strURL = wxEmptyString;
    bool bCachedCredentials = false;
    ACCT_MGR_INFO ami;
    PROJECT_INIT_STATUS pis;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	// If we are connected to the localhost, run a really quick screensaver
    //   test to trigger a firewall popup.
    pDoc->GetConnectedComputerName(strComputer);
    if (pDoc->IsComputerNameLocal(strComputer)) {
        wxGetApp().StartBOINCScreensaverTest();
    }


    pDoc->rpc.acct_mgr_info(ami);
    if (ami.acct_mgr_url.size() && !ami.have_credentials) {
        pAMWizard = new CWizardAccountManager(this);

        if (!IsShown()) {
            Show();
        }

        if (pAMWizard->Run()) {
            // If successful, hide the main window
            Hide();
        }
    } else if (0 >= pDoc->GetProjectCount()) {
        pAPWizard = new CWizardAttachProject(this);

        if (!IsShown()) {
            Show();
        }

        pDoc->rpc.get_project_init_status(pis);
        strName = wxString(pis.name.c_str(), wxConvUTF8);
        strURL = wxString(pis.url.c_str(), wxConvUTF8);
        bCachedCredentials = pis.url.length() && pis.has_account_key;

        pAPWizard->Run(strName, strURL, bCachedCredentials);
    }

    if (pAMWizard)
        pAMWizard->Destroy();
	if (pAPWizard){
        pAPWizard->Destroy();
		//update Project Component
		projComponent->UpdateInterface();
	}

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnConnect - Function End"));
}


void CSimpleFrame::OnReloadSkin(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnReloadSkin - Function Start"));

    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	//bg color
	SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());

    if(notebookViewInitialized){
		if (wrkUnitNB) wrkUnitNB->ReskinAppGUI();
	} else {
		if (clientState) clientState->ReskinInterface();
	}

    //reskin component 
	if (projComponent) projComponent->ReskinInterface();

    Refresh();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnReloadSkin - Function End"));
}


void CSimpleFrame::OnProjectsAttachToProject() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnProjectsAttachToProject - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {
        
        m_pFrameRenderTimer->Stop();

        CWizardAttachProject* pWizard = new CWizardAttachProject(this);

        wxString strName = wxEmptyString;
        wxString strURL = wxEmptyString;
        pWizard->Run( strName, strURL, false );

        if (pWizard)
            pWizard->Destroy();

        m_pFrameRenderTimer->Start();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToProject - Function End"));
}


void CSimpleFrame::OnFrameRender(wxTimerEvent& WXUNUSED(event)) {
	CMainDocument* pDoc     = wxGetApp().GetDocument();

	if (!projectViewInitialized) {
		InitProjectView();
		return;
	} else if ( pDoc->IsConnected() ) {
		UpdateProjectView();
	}

	// Now check to see if we show the empty state or results
	if ( pDoc->GetSimpleGUIWorkCount() > 0 ) {
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
}


void CSimpleFrame::UpdateEmptyView() {
	clientState->DisplayState();
}


void CSimpleFrame::DestroyEmptyView() {
	delete clientState;
	clientState = NULL;
	emptyViewInitialized = false;
}


void CSimpleFrame::InitEmptyView()
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


void CSimpleFrame::UpdateProjectView()
{
	//update Project Component
    projComponent->UpdateInterface();
}


void CSimpleFrame::DestroyNotebook() {
	mainSizer->Detach(wrkUnitNB);
	delete wrkUnitNB;
	wrkUnitNB = NULL;
	notebookViewInitialized = false;
}


void CSimpleFrame::InitProjectView()
{
	// Do not update screen at this point
	/////////////// MY PROJECTS COMPONENT /////////////////////
    projComponent = new CProjectsComponent(this,wxPoint(31,413));
	///////////////////////////////////////////////////////////
	projectViewInitialized = true;
}


void CSimpleFrame::InitNotebook()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::InitNotebook - Function Start"));
	// FlatNotebook
	wrkUnitNB = new WorkunitNotebook(this, -1, wxDefaultPosition, wxSize(370,330), wxFNB_TABS_BORDER_SIMPLE | wxFNB_NO_X_BUTTON | wxFNB_NO_NAV_BUTTONS | wxFNB_FANCY_TABS);
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


void CSimpleFrame::OnEraseBackground(wxEraseEvent& event){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxDC *dc = event.GetDC();
    dc->DrawBitmap(*pSkinSimple->GetBackgroundImage()->GetBitmap(), 0, 0);
}


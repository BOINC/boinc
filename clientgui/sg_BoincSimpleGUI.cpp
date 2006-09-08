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
#include "BOINCGUIApp.h"
#include "Events.h"
#include "BOINCBaseFrame.h"

#include "sg_BoincSimpleGUI.h"
#include "sg_SkinClass.h"
#include "sg_ImageLoader.h"
#include "sg_ProjectsComponent.h"
#include "sg_ClientStateIndicator.h"
#include "sg_StatImageLoader.h"
#include "sg_ViewTabPage.h"


#include "wizardex.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "error_numbers.h"
#include <string>

#include "res/boinc.xpm"

IMPLEMENT_DYNAMIC_CLASS(CSimpleFrame, CBOINCBaseFrame)


BEGIN_EVENT_TABLE(CSimpleFrame, CBOINCBaseFrame)
    EVT_BUTTON(-1,CSimpleFrame::OnBtnClick)
    EVT_SIZE(CSimpleFrame::OnSize)
	EVT_ERASE_BACKGROUND(CSimpleFrame::OnEraseBackground)
    EVT_FRAME_CONNECT(CSimpleFrame::OnConnect)
	EVT_TIMER(ID_SIMPLEFRAMERENDERTIMER, CSimpleFrame::OnFrameRender)
	EVT_FLATNOTEBOOK_PAGE_CHANGED(-1, CSimpleFrame::OnPageChanged)
END_EVENT_TABLE()


CSimpleFrame::CSimpleFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function End"));
}


CSimpleFrame::CSimpleFrame(wxString title, wxIcon* icon) : 
    CBOINCBaseFrame((wxFrame *)NULL, ID_SIMPLEFRAME, title, wxDefaultPosition, wxSize(416, 600),
                    wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Overloaded Constructor Function Begin"));
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    // Initialize Application
    SetIcon(*icon);
	
    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Read(wxT("Skin"), &skinName, wxT("default"));

    //init app skin class
	appSkin = SkinClass::Instance();
	appSkin->init_skin(skinName);
	LoadSkinImages();

	projectViewInitialized = false;
	resultViewInitialized = false;
	emptyViewInitialized = false;
	notebookViewInitialized = false;

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

	wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(m_pFrameRenderTimer);

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("Skin"), skinName);

	if (m_pFrameRenderTimer) {
        m_pFrameRenderTimer->Stop();
        delete m_pFrameRenderTimer;
    }
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Destructor Function End"));
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

	if (!projectViewInitialized && pDoc->IsConnected()) {
		InitProjectView(); 
	} else if ( pDoc->IsConnected() ) {
		UpdateProjectView();
	}
	// Now check to see if we show the empty state or results
	if ( pDoc->GetSimpleGUIWorkCount() > 0 ) {
		// If empty was displayed, remove
		if ( emptyViewInitialized ) {
			DestroyEmptyView();
		}
		// If we hadn't previously shown the results, create them.
		if ( !resultViewInitialized ) {
			Freeze();
			InitResultView();
			UpdateResultView();
			Thaw();
		} else {
			UpdateResultView();
		}
	} else {
		if ( resultViewInitialized ) {
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
	emptyViewInitialized = false;
}


void CSimpleFrame::InitEmptyView()
{
	//Set Background color
	SetBackgroundColour(appSkin->GetAppBgCol());

	// Flex Grid Sizer
	mainSizer = new wxFlexGridSizer(3,2);
	SetSizer(mainSizer);

	clientState = new ClientStateIndicator(this,wxPoint(31,124));
	clientState->DisplayState();
	
	emptyViewInitialized = true;
}
void CSimpleFrame::UpdateProjectView()
{
	//update Project Component
    projComponent->UpdateInterface();
}
void CSimpleFrame::InitResultView()
{
	SetSizer(mainSizer);
	mainSizer->Add(31, 98,0);
	mainSizer->Add(343, 98,0);
	mainSizer->Add(31, 98,0);
	mainSizer->Add(0, 0,1);
	mainSizer->Layout();
	resultViewInitialized=true;
	//present any results available
	UpdateResultView();
	//make sure the first tab is in front
	if (m_windows.size() > 0 ) {
		wrkUnitNB->SetSelection(0);
	}
}
void CSimpleFrame::DestroyNotebook() {
	mainSizer->Detach(wrkUnitNB);
	delete wrkUnitNB;
	resultViewInitialized = false;
	notebookViewInitialized = false;
}
void CSimpleFrame::InitProjectView()
{
	// Do not update screen at this point
	Freeze();
	/////////////// MY PROJECTS COMPONENT /////////////////////
    projComponent = new CProjectsComponent(this,wxPoint(31,443));
	///////////////////////////////////////////////////////////
	Thaw();
	projectViewInitialized = true;
}
void CSimpleFrame::UpdateResultView(){
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	//update GUI
    wxString strBuffer = wxEmptyString;
	//assume they are all inactive
	for(int x = 0; x < (int)m_windows.size(); x ++)
	{
		CViewTabPage *currTab = m_windows[x];
		currTab->isAlive = false;
	}
    // Update Tabs
	RESULT* result;
	for(int i = 0; i < (int) pDoc->results.results.size(); i++){
		result = pDoc->result(i);
		if ( result == NULL || !result->active_task ) {
			continue;
		}
		// get tab window
		bool found = false;
		for(int j = 0; j < (int)m_windows.size(); j++)
		{
			CViewTabPage *currTab = m_windows[j];
			//std::string curtabname = currTab->GetTabName();
            //std::string resultname = result->name;

			if(result->name == currTab->GetTabName()){
				//currTab FOUND;
				currTab->isAlive = true;
				currTab->resultWU = result;
				found = true;
				//update tab interface
		        currTab->UpdateInterface();
				if(result->active_task_state == 1 && wrkUnitNB->GetPageImageIndex(j) != 0){
					wrkUnitNB->SetPageImageIndex(j, 0); // this is working process
				} else if ( result->active_task_state != 1 && wrkUnitNB->GetPageImageIndex(j) != -1 ) {
					wrkUnitNB->SetPageImageIndex(j, -1); // this is working process
				}
				//break;
		    }

		}
		if(!found){

			// First check to see if the underlying state object contains this data
			// if not, then force an update and skip this result for now.
			RESULT* resState = NULL;
			std::string projUrl = result->project_url;
			std::string nme = result->name;
            resState = pDoc->state.lookup_result(projUrl, nme);
			if(!resState){
                pDoc->ForceCacheUpdate();
 				continue;
			}
 			wxString appShortName = wxString(resState->app->name.c_str(), wxConvUTF8 );

			bool addNotebookToSizer = false;
			//stop timer untill we add the page
			m_pFrameRenderTimer->Stop();
			// Do not update screen at this point
	        Freeze();
			//Check if notebook was initialized
			if(!notebookViewInitialized){
				//init nootebok
				InitNotebook();
                addNotebookToSizer = true; // since this is first page
			}
			// create one and add it to notebook
			std::string index = " ";
			//index += i;
			appShortName += wxString(index.c_str(), wxConvUTF8 );
			CViewTabPage *wTab = new CViewTabPage(wrkUnitNB,result,nme,projUrl);
		
			wrkUnitNB->AddPage(wTab, appShortName, true);	
			if(result->active_task_state == 1){
				int pageIndex = wrkUnitNB->GetPageIndex(wTab);
				wrkUnitNB->SetPageImageIndex(pageIndex, 0); // this is working process
			}
			//add page to vector
			m_windows.push_back(wTab);
			if(addNotebookToSizer){
                wrkUnitNB->SetSelection(0);	
				// Put Grid in the sizer
				mainSizer->Add(wrkUnitNB);
			}
			//Update screen
			Thaw();
			mainSizer->Layout();
			//start the timer
			m_pFrameRenderTimer->Start();
		}		
	}
	//delete the ones that are not alive
	//assume they are all inactive
	int deleteIndex = 0;
	for(int x = 0; x < (int)m_windows.size(); x ++)
	{
		CViewTabPage *currTab = m_windows[x];
		if(!currTab->isAlive){
			//delete the notebook page
			wrkUnitNB->DeletePage(deleteIndex);
			//delete the page in vector
			m_windows.erase(m_windows.begin()+x);
		}else{
			deleteIndex++;
		}
	}
//	Refresh();
}

void CSimpleFrame::InitNotebook()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::InitNotebook - Function Start"));
	// FlatNotebook
	wrkUnitNB = new wxFlatNotebook(this, -1, wxDefaultPosition, wxSize(370,330), wxFNB_TABS_BORDER_SIMPLE | wxFNB_NO_X_BUTTON | wxFNB_NO_NAV_BUTTONS | wxFNB_FANCY_TABS);
	wrkUnitNB->SetUseBackground(true);
	wrkUnitNB->SetBackgroundColour(appSkin->GetAppBgCol());
	wrkUnitNB->SetTabAreaColour(appSkin->GetAppBgCol());
	wrkUnitNB->SetGradientColors(appSkin->GetTabFromColAc(),appSkin->GetTabToColAc(),appSkin->GetTabBrdColAc());
	wrkUnitNB->SetActiveTabTextColour(wxColour(255,255,255));
	wrkUnitNB->SetGradientColorsInactive(appSkin->GetTabFromColIn(),appSkin->GetTabToColIn(),appSkin->GetTabBrdColIn());
	wrkUnitNB->SetNonActiveTabTextColour(wxColour(255,255,255));
	wrkUnitNB->SetImageList(&m_ImageList);
	notebookViewInitialized = true;
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::InitNotebook - Function End"));
}
void CSimpleFrame::LoadSkinImages(){

	wxString dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	
    fileImgBuf[0].LoadFile(dirPref + appSkin->GetAppBg(),wxBITMAP_TYPE_PNG);
	// work unit icons
	g_icoWorkWU = new wxImage(dirPref + appSkin->GetIcnWorkingWkUnit(), wxBITMAP_TYPE_PNG);
	//////////////////////////////
	frameBg=&fileImgBuf[0];
	/// work unit tabs icons
	wxBitmap const workWUico = wxBitmap(g_icoWorkWU); 
	// push them in image list
	m_ImageList.push_back(workWUico);
}
///
///
void CSimpleFrame::ReskinAppGUI(){
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::ReskinAppGUI - Function Start"));
	//bg color
	LoadSkinImages();
	SetBackgroundColour(appSkin->GetAppBgCol());
	if(notebookViewInitialized){
        // notebook tab color
		wrkUnitNB->SetTabAreaColour(appSkin->GetAppBgCol());
		wrkUnitNB->SetUseBackground(true);
		wrkUnitNB->SetGradientColors(appSkin->GetTabFromColAc(),appSkin->GetTabToColAc(),appSkin->GetTabBrdColAc());
		wrkUnitNB->SetGradientColorsInactive(appSkin->GetTabFromColIn(),appSkin->GetTabToColIn(),appSkin->GetTabBrdColIn());
	
		// notebook pages
		for(int i = 0; i < (int)m_windows.size(); i++){
			CViewTabPage *wTab = m_windows.at(i);
			wTab->ReskinInterface();
		}
	} else {
		clientState->ReskinInterface();
	}
	//reskin component 
	projComponent->ReskinInterface();
	Refresh();
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::ReskinAppGUI - Function End"));
}

void CSimpleFrame::OnBtnClick(wxCommandEvent& event){ //init function
	wxObject *m_wxBtnObj = event.GetEventObject();
}
//end function
void CSimpleFrame::OnPageChanged(wxFlatNotebookEvent& WXUNUSED(event))
{
//	btnCollapse->Refresh();
}
void CSimpleFrame::OnEraseBackground(wxEraseEvent& event){
  wxObject *m_wxWin = event.GetEventObject();
  if(m_wxWin==this){event.Skip(true);DrawBackImg(event,this,*frameBg,0);return;}
  event.Skip(true);
}
void CSimpleFrame::DrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz){

	event.Skip(false);
	wxDC *dc;
	dc=event.GetDC();
	dc->SetBackground(wxBrush(win->GetBackgroundColour(),wxSOLID));
	dc->Clear();
	switch (opz) {
	case 0:{
			dc->DrawBitmap(bitMap, 0, 0);
			break;}
	case 1:{
			wxRect rec=win->GetClientRect();
			rec.SetLeft((rec.GetWidth()-bitMap.GetWidth())   / 2);
			rec.SetTop ((rec.GetHeight()-bitMap.GetHeight()) / 2);
			dc->DrawBitmap(bitMap,rec.GetLeft(),rec.GetTop(),0);
			break;}
	case 2:{
			wxRect rec=win->GetClientRect();
			for(int y=0;y < rec.GetHeight();y+=bitMap.GetHeight()){
			for(int x=0;x < rec.GetWidth();x+=bitMap.GetWidth()){
				dc->DrawBitmap(bitMap,x,y,0);
			}
			}
			break;}
	}
}


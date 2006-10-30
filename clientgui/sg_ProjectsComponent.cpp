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
#pragma implementation "sg_ProjectsComponent.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "sg_BoincSimpleGUI.h"
#include "sg_ImageLoader.h"
#include "sg_CustomControls.h" 
#include "sg_StatImageLoader.h" 
#include "sg_DlgMessages.h"
#include "sg_DlgPreferences.h"
#include "sg_ProjectsComponent.h"
#include "app_ipc.h"


IMPLEMENT_DYNAMIC_CLASS(CProjectsComponent, wxPanel)

BEGIN_EVENT_TABLE(CProjectsComponent, wxPanel)
    EVT_BUTTON(ID_SIMPLE_PREFERENCES, CProjectsComponent::OnPreferences)
    EVT_PAINT(CProjectsComponent::OnPaint)
    EVT_BUTTON(-1,CProjectsComponent::OnBtnClick)
	EVT_ERASE_BACKGROUND(CProjectsComponent::OnEraseBackground)
	EVT_TIMER(ID_SIMPLEMESSAGECHECKTIMER, CProjectsComponent::OnMessageCheck)
END_EVENT_TABLE()

size_t CProjectsComponent::lastMessageId = 0;

CProjectsComponent::CProjectsComponent() {
}


CProjectsComponent::CProjectsComponent(CSimpleFrame* parent,wxPoint coord) :
    wxPanel(parent, -1, coord, wxSize(343,113), wxNO_BORDER)
{
    wxASSERT(parent);
	m_maxNumOfIcons = 6; // max number of icons in component
	m_rightIndex = 0;
	m_leftIndex = 0;
	m_projCnt = 0;
	CreateComponent();

	receivedErrorMessage = false;
	alertMessageDisplayed = false;
	checkForMessagesTimer = new wxTimer(this, ID_SIMPLEMESSAGECHECKTIMER);
	checkForMessagesTimer->Start(5000); 

}

CProjectsComponent::~CProjectsComponent() {
	delete checkForMessagesTimer;
}

void CProjectsComponent::CreateComponent()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	//Set Background color
    SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());

	//Static content in my Projects section
	// add project button
	wxToolTip *ttAddProject = new wxToolTip(_("Add Project"));
	btnAddProj=new wxBitmapButton(
        this,
        -1,
        *pSkinSimple->GetAttachProjectButton()->GetBitmap(),
        wxPoint(235,7),
        wxSize(81,18),
        wxBU_NOAUTODRAW
    );
	btnAddProj->SetBitmapSelected(
        *pSkinSimple->GetAttachProjectButton()->GetBitmapClicked()
    );
	btnAddProj->SetToolTip(ttAddProject);
	
    /// Line
	lnMyProjTop = new CTransparentStaticLine(this, wxID_ANY, wxPoint(29,29),wxSize(292,1));
    lnMyProjTop->SetLineColor(pSkinSimple->GetStaticLineColor());

	/////////////// ICONS /////////////////////
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	m_projCnt = 0;

	if ( pDoc->IsConnected() ) {
		m_projCnt = pDoc->GetProjectCount();
		defaultIcnPath[256];
		// url of project directory

		for(int j = 0; j < m_projCnt; j++){
			PROJECT* project = pDoc->state.projects[j];
			//user credit text
			if(j < m_maxNumOfIcons){
				// Project button
				StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
				i_statW->Move(wxPoint(55 + 40*j,37));
				i_statW->LoadImage();
			
				// push icon in the vector
				m_statProjects.push_back(i_statW);
				//increment left index
				m_rightIndex ++;
			}
		}
	} 

	//// Arrow Btns
	btnArwLeft = new wxBitmapButton(
        this,
        -1,
        *(pSkinSimple->GetLeftArrowButton()->GetBitmap()),
        wxPoint(29,47),
        wxSize(20,20),
        wxBU_NOAUTODRAW
    );
    btnArwLeft->SetBitmapSelected(*(pSkinSimple->GetLeftArrowButton()->GetBitmapClicked()));
    btnArwLeft->Show(false);//on creation this one is always false

	btnArwRight = new wxBitmapButton(
        this,
        -1,
        *(pSkinSimple->GetRightArrowButton()->GetBitmap()),
        wxPoint(301,47),
        wxSize(20,20),
        wxBU_NOAUTODRAW
    );
    btnArwRight->SetBitmapSelected(*(pSkinSimple->GetRightArrowButton()->GetBitmapClicked()));

	if(m_projCnt > m_maxNumOfIcons){//right shows up only if there is more than max num of icons
		btnArwRight->Show(true);
	}else{
        btnArwRight->Show(false);
	}

    //
	//// Messages Play Pause Btns
	wxToolTip *ttMessages = new wxToolTip(_("Messages"));
	btnMessages = new wxBitmapButton(
        this,
        -1,
        *(pSkinSimple->GetMessagesLink()->GetBitmap()),
        wxPoint(11,86),
        wxSize(70,20),
        wxBU_NOAUTODRAW
    );
	btnMessages->SetBitmapSelected(*(pSkinSimple->GetMessagesLink()->GetBitmap()));
	btnMessages->SetToolTip(ttMessages);

	wxToolTip *ttAlertMessages = new wxToolTip(_("Messages"));
	btnAlertMessages = new wxBitmapButton(
        this,
        -1,
        *(pSkinSimple->GetMessagesAlertLink()->GetBitmap()),
        wxPoint(11,86),
        wxSize(70,20),
        wxBU_NOAUTODRAW
    );
	btnAlertMessages->SetBitmapSelected(*(pSkinSimple->GetMessagesAlertLink()->GetBitmap()));
	btnAlertMessages->SetToolTip(ttAlertMessages);
	btnAlertMessages->Show(false);

    //spacer
	wxWindow *w_sp1 = new wxWindow(this,-1,wxPoint(83,91),wxSize(2,11));
    i_spacer = new ImageLoader(w_sp1);
    i_spacer->LoadImage(*(pSkinSimple->GetSpacerImage()->GetBitmap()));

    // pause btn
	wxToolTip *ttPause = new wxToolTip(_("Pause all processing"));
	btnPause = new wxBitmapButton(
        this,
        -1,
        *(pSkinSimple->GetSuspendLink()->GetBitmap()),
        wxPoint(85,86),
        wxSize(59,20),
        wxBU_NOAUTODRAW
    );
	btnPause->SetBitmapSelected(*(pSkinSimple->GetSuspendLink()->GetBitmap()));
	btnPause->SetToolTip(ttPause);

    // resume btn   
	wxToolTip *ttResume = new wxToolTip(_("Resume all Processing"));
	btnResume = new wxBitmapButton(
        this,
        -1,
        *(pSkinSimple->GetResumeLink()->GetBitmap()),
        wxPoint(85,86),
        wxSize(59,20),
        wxBU_NOAUTODRAW
    );
	btnResume->SetBitmapSelected(*(pSkinSimple->GetResumeLink()->GetBitmap()));
	btnResume->SetToolTip(ttResume);

    // Show resume or pause as appropriate
	CC_STATUS status;
	pDoc->GetCoreClientStatus(status);
	if ( status.task_mode == RUN_MODE_NEVER ) {
		btnPause->Show(false);
		btnResume->Show(true);
	} else {
		btnPause->Show(true);
		btnResume->Show(false);
	}

	//spacer
	wxWindow *w_sp2 = new wxWindow(this,-1,wxPoint(144,91),wxSize(2,11));
    i_spacer = new ImageLoader(w_sp2);
    i_spacer->LoadImage(*(pSkinSimple->GetSpacerImage()->GetBitmap()));

    // Pref Btn
	wxToolTip *ttPreferences = new wxToolTip(_("Preferences"));
	btnPreferences = new wxBitmapButton(
        this,
        ID_SIMPLE_PREFERENCES,
        *(pSkinSimple->GetPreferencesLink()->GetBitmap()),
        wxPoint(149,86),
        wxSize(81,20),
        wxBU_NOAUTODRAW
    );
	btnPreferences->SetBitmapSelected(*(pSkinSimple->GetPreferencesLink()->GetBitmap()));
	btnPreferences->SetToolTip(ttPreferences);

    //spacer
	wxWindow *w_sp3 = new wxWindow(this,-1,wxPoint(230,91),wxSize(2,11));
    i_spacer = new ImageLoader(w_sp3);
    i_spacer->LoadImage(*(pSkinSimple->GetSpacerImage()->GetBitmap()));

    // Advanced View
	wxToolTip *ttAdvView = new wxToolTip(_("Advanced View"));
	btnAdvancedView = new wxBitmapButton(
        this,
        -1,
        *(pSkinSimple->GetAdvancedLink()->GetBitmap()),
        wxPoint(233,86),
        wxSize(101,20),
        wxBU_NOAUTODRAW
    );
    btnAdvancedView->SetBitmapSelected(*(pSkinSimple->GetAdvancedLink()->GetBitmap()));
	btnAdvancedView->SetToolTip(ttAdvView);

    /// Line
	lnMyProjBtm = new CTransparentStaticLine(this, wxID_ANY, wxPoint(29,83),wxSize(292,1));
    lnMyProjBtm->SetLineColor(pSkinSimple->GetStaticLineColor());
}

void CProjectsComponent::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
    //My Projects
	dc.SetFont(wxFont(10,74,90,92,0,wxT("Arial"))); 
	dc.DrawText(wxT("My Projects:"), wxPoint(32,9)); 
}


void CProjectsComponent::OnPreferences(wxCommandEvent& /*event*/) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnPreferences - Function Begin"));

	CSimpleFrame* pFrame = wxDynamicCast(GetParent(), CSimpleFrame);

    wxASSERT(pFrame);

	pFrame->SetDlgOpen(true);

	CDlgPreferences* pDlg = new CDlgPreferences(GetParent());
    wxASSERT(pDlg);

    pDlg->ShowModal();

    if (pDlg)
        pDlg->Destroy();

    pFrame->SetDlgOpen(false);

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnPreferences - Function End"));
}


void CProjectsComponent::RemoveProject(std::string prjUrl)
{	
	CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	//update project count
	m_projCnt = pDoc->GetProjectCount();
		
	int indexOfIcon = -1;
	
	for(int m = 0; m < (int)m_statProjects.size(); m++){
		StatImageLoader *i_statWShifting = m_statProjects.at(m);
		if(i_statWShifting->m_prjUrl == prjUrl){
			delete m_statProjects.at(m);
			m_statProjects.erase(m_statProjects.begin()+m);
			indexOfIcon = m;
			break;
		}
	}
	// if last icon is removed but there is still hidden icons on left shifting to right
	if((m_leftIndex > 0) && (m_rightIndex-1 == m_projCnt)){
		//shift icons right
		for(int m = 0; m < indexOfIcon; m++){
			StatImageLoader *i_statWShifting = m_statProjects.at(m);
			i_statWShifting->Move(wxPoint(55 + 40*(m+1),37));
		}
		// create the icon on left
		if(m_leftIndex-1 >= 0){
			PROJECT* project = pDoc->state.projects.at(m_leftIndex-1);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
		    i_statW->Move(wxPoint(55,37));
			i_statW->LoadImage();
			
		    // push icon in the vector
		    m_statProjects.insert(m_statProjects.begin(),i_statW);
			//decrement left index
			m_leftIndex --;
			//decrement right index since project was removed at last slot
			m_rightIndex --;
		}

	}else{
		//shift icons to the left. Nothing will be shifted if last icon is removed
		for(int k = indexOfIcon; k < (int)m_statProjects.size(); k++){
			StatImageLoader *i_statWShifting = m_statProjects.at(k);
			i_statWShifting->Move(wxPoint(55 + 40*k,37));
		}
	    // create the icon on right
		if(m_rightIndex <= m_projCnt){
			PROJECT* project = pDoc->state.projects.at(m_rightIndex-1);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
			i_statW->Move(wxPoint(55 + 40*(m_maxNumOfIcons-1),37));
			i_statW->LoadImage();
			
			// push icon in the vector
			m_statProjects.push_back(i_statW);
		}else{//if nothing can be shifted in place of last icon
			//decrement right index
			m_rightIndex --;
		}
	}

	////////////hide or show arrows///////////
	if(m_leftIndex == 0){
		btnArwLeft->Show(false);
	}else{
		btnArwLeft->Show(true);
	}
	//
	if(m_rightIndex < m_projCnt){
		btnArwRight->Show(true);
	}else{
		btnArwRight->Show(false);
	}
	///////////////////////////////////////////
}

void CProjectsComponent::UpdateInterface()
{
	CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	// Check to see if error messages have been received
	if ( receivedErrorMessage ) {
		Freeze();
		if ( alertMessageDisplayed ) {
			btnAlertMessages->Show(false);
			btnMessages->Show(true);
			alertMessageDisplayed = false;
		} else {
			btnAlertMessages->Show(true);
			btnMessages->Show(false);
			alertMessageDisplayed = true;
		}
		Thaw();
	} else {
		if ( alertMessageDisplayed ) {
			Freeze();
			btnAlertMessages->Show(false);
			btnMessages->Show(true);
			alertMessageDisplayed = false;
			Thaw();
		}
	}

	// Show resume or pause as appropriate
	CC_STATUS status;
	pDoc->GetCoreClientStatus(status);
	if ( status.task_mode == RUN_MODE_NEVER ) {
		btnPause->Show(false);
		btnResume->Show(true);
	} else {
		btnPause->Show(true);
		btnResume->Show(false);
	}

	// Update stat icons
	for(int m = 0; m < (int)m_statProjects.size(); m++){
		StatImageLoader *i_statIcon = m_statProjects.at(m);
		i_statIcon->UpdateInterface();
	}
	
	// Check number of projects
	int oldProjCnt = m_projCnt;
	m_projCnt = pDoc->GetProjectCount();
	if(m_projCnt == oldProjCnt){
		return;
	}

	if(m_projCnt <= m_maxNumOfIcons){
		PROJECT* project = pDoc->state.projects.at(m_projCnt-1);
		StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
		i_statW->Move(wxPoint(55 + 40*(m_projCnt-1),37));
		i_statW->LoadImage();
		
		// push icon in the vector
		m_statProjects.push_back(i_statW);
		//increment left index
		m_rightIndex ++;
	}
	//show arrow if we are at the over max number of projects
	if(m_projCnt > m_maxNumOfIcons){
		btnArwRight->Show(true);
	}

}

void CProjectsComponent::ReskinInterface()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    //Set Background color only
	SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());

	//right button
    btnArwRight->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnArwRight->SetBitmapLabel(*(pSkinSimple->GetRightArrowButton()->GetBitmap()));
    btnArwRight->SetBitmapSelected(*(pSkinSimple->GetRightArrowButton()->GetBitmapClicked()));
	
    //left button
	btnArwLeft->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnArwLeft->SetBitmapLabel(*(pSkinSimple->GetLeftArrowButton()->GetBitmap()));
    btnArwLeft->SetBitmapSelected(*(pSkinSimple->GetLeftArrowButton()->GetBitmapClicked()));

    // add project btn
	btnAddProj->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnAddProj->SetBitmapLabel(*(pSkinSimple->GetAttachProjectButton()->GetBitmap()));
    btnAddProj->SetBitmapSelected(*(pSkinSimple->GetAttachProjectButton()->GetBitmapClicked()));

    // messages btn
	btnMessages->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnMessages->SetBitmapLabel(*(pSkinSimple->GetMessagesLink()->GetBitmap()));
	btnMessages->SetBitmapSelected(*(pSkinSimple->GetMessagesLink()->GetBitmap()));

    // alert messages btn
	btnAlertMessages->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
	btnAlertMessages->SetBitmapLabel(*(pSkinSimple->GetMessagesAlertLink()->GetBitmap()));
	btnAlertMessages->SetBitmapSelected(*(pSkinSimple->GetMessagesAlertLink()->GetBitmap()));

    // pause btn
	btnPause->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnPause->SetBitmapLabel(*(pSkinSimple->GetSuspendLink()->GetBitmap()));
	btnPause->SetBitmapSelected(*(pSkinSimple->GetSuspendLink()->GetBitmap()));

    // resume btn
    btnResume->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnResume->SetBitmapLabel(*(pSkinSimple->GetResumeLink()->GetBitmap()));
	btnResume->SetBitmapSelected(*(pSkinSimple->GetResumeLink()->GetBitmap()));

    // preferences btn
    btnPreferences->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnPreferences->SetBitmapLabel(*(pSkinSimple->GetPreferencesLink()->GetBitmap()));
	btnPreferences->SetBitmapSelected(*(pSkinSimple->GetPreferencesLink()->GetBitmap()));

    // advance view btn
    btnAdvancedView->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnAdvancedView->SetBitmapLabel(*(pSkinSimple->GetAdvancedLink()->GetBitmap()));
	btnAdvancedView->SetBitmapSelected(*(pSkinSimple->GetAdvancedLink()->GetBitmap()));

    //set line colors
    lnMyProjTop->SetLineColor(pSkinSimple->GetStaticLineColor());
	lnMyProjBtm->SetLineColor(pSkinSimple->GetStaticLineColor());

	// Rebuild stat menus and reload icons
	for(int m = 0; m < (int)m_statProjects.size(); m++){
		StatImageLoader *i_statImage = m_statProjects.at(m);
		i_statImage->LoadImage();
		i_statImage->RebuildMenu();
	}
}

void CProjectsComponent::OnBtnClick(wxCommandEvent& event){ //init function
	wxObject *m_wxBtnObj = event.GetEventObject();

	CSimpleFrame* pFrame      = wxDynamicCast(GetParent(), CSimpleFrame);
    wxASSERT(pFrame);


	if (m_wxBtnObj==btnArwLeft){
		//delete proj icon at position max number  - 1(5)
		delete m_statProjects.at(m_maxNumOfIcons-1);
        //remove last element from vector
		m_statProjects.pop_back();
		//shift icons right
		for(int m = 0; m < (int)m_statProjects.size(); m++){
			StatImageLoader *i_statWShifting = m_statProjects.at(m);
			i_statWShifting->Move(wxPoint(55 + 40*(m+1),37));
		}

		CMainDocument* pDoc     = wxGetApp().GetDocument();
		
		if(m_leftIndex-1 >= 0){
			PROJECT* project = pDoc->state.projects.at(m_leftIndex-1);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
		    i_statW->Move(wxPoint(55,37));
			i_statW->LoadImage();
			
		    // push icon in the vector
		    m_statProjects.insert(m_statProjects.begin(),i_statW);
			//decrement left index
			m_leftIndex --;
			//decrement right index
			m_rightIndex --;
			//now show left button
			btnArwRight->Show(true);

		}
		//hide right arrow if we got to the end of the list
		if(m_leftIndex <= 0){
			btnArwLeft->Show(false);
		}
		Refresh();

	} else if(m_wxBtnObj==btnArwRight){
		//delete proj icon at position 1(0)
		delete m_statProjects.at(0);
		//shift the vector
		m_statProjects.assign(m_statProjects.begin()+1,m_statProjects.end());
		//shift icons left
		for(int m = 0; m < (int)m_statProjects.size(); m++){
			StatImageLoader *i_statWShifting = m_statProjects.at(m);
			wxPoint currPoint = i_statWShifting->GetPosition();
			i_statWShifting->Move(wxPoint(55 + 40*m,37));
		}
       
        CMainDocument* pDoc     = wxGetApp().GetDocument();
		//update project count
		m_projCnt = (int)pDoc->state.projects.size();
		if(m_rightIndex+1 <= m_projCnt){
			PROJECT* project = pDoc->state.projects.at(m_rightIndex);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
		    i_statW->Move(wxPoint(55 + 40*(m_maxNumOfIcons-1),37));
			i_statW->LoadImage();
			
		    // push icon in the vector
		    m_statProjects.push_back(i_statW);
			//increment left index
			m_leftIndex ++;
			//increment right index
			m_rightIndex ++;
			//now show left button
			btnArwLeft->Show(true);

		}
		//hide right arrow if we got to the end of the list
		if(m_rightIndex >= m_projCnt){
			btnArwRight->Show(false);
		}
		Refresh();
	} else if(m_wxBtnObj==btnAddProj){
		pFrame->OnProjectsAttachToProject();
		btnAddProj->Refresh();
	} else if(m_wxBtnObj==btnMessages || m_wxBtnObj==btnAlertMessages){
		MessagesViewed();
		pFrame->SetDlgOpen(true);
		CDlgMessages* pDlg = new CDlgMessages(NULL);
		wxASSERT(pDlg);
		pDlg->ShowModal();
		pDlg->Destroy();
		pFrame->SetDlgOpen(false);
    } else if(m_wxBtnObj==btnPause) {
		CMainDocument* pDoc     = wxGetApp().GetDocument();
        CC_STATUS      status;

        pDoc->GetCoreClientStatus(status);

        clientRunMode = status.task_mode;
		pDoc->SetActivityRunMode(RUN_MODE_NEVER);
		pDoc->SetNetworkRunMode(RUN_MODE_NEVER);
		btnPause->Show(false);
		btnResume->Show(true);
    } else if(m_wxBtnObj==btnResume) {
		CMainDocument* pDoc     = wxGetApp().GetDocument();
		pDoc->SetActivityRunMode(RUN_MODE_AUTO);
		pDoc->SetNetworkRunMode(RUN_MODE_AUTO);
		btnResume->Show(false);
		btnPause->Show(true);
    } else if(m_wxBtnObj==btnAdvancedView) {
        wxGetApp().SetActiveGUI(BOINC_ADVANCEDGUI, true);
    }
}


void CProjectsComponent::OnEraseBackground(wxEraseEvent& event){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	wxDC* dc = event.GetDC();
    dc->DrawBitmap(*pSkinSimple->GetProjectAreaBackgroundImage()->GetBitmap(), 0, 0);
}


void CProjectsComponent::OnMessageCheck(wxTimerEvent& WXUNUSED(event)) {
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	MESSAGE* message;
	// Only look at the messages recieved since the last time we looked
	if ( pDoc->GetMessageCount() > (int) lastMessageId ) {
		// Loop through and check for any messages recieved that are error messages
		for(size_t i=lastMessageId; i < pDoc->messages.messages.size(); i++) {
			lastMessageId = i+1;
			message = pDoc->message((unsigned int) i);
			if ( message != NULL && message->priority == MSG_ERROR ) {
				receivedErrorMessage = true;
				checkForMessagesTimer->Stop();
				break;
			}
		}
	}
}


void CProjectsComponent::MessagesViewed() {
	receivedErrorMessage = false;
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	lastMessageId = pDoc->GetMessageCount();
	checkForMessagesTimer->Start();
}
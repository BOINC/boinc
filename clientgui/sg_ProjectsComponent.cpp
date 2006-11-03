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
	m_leftIndex = 0;
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
	Freeze();
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
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetAttachProjectButton()->GetBitmapClicked() != NULL ) {
		btnAddProj->SetBitmapSelected(
			*pSkinSimple->GetAttachProjectButton()->GetBitmapClicked()
		);
	}
	btnAddProj->SetToolTip(ttAddProject);
	
    /// Line
	lnMyProjTop = new CTransparentStaticLine(this, wxID_ANY, wxPoint(29,29),wxSize(292,1));
    lnMyProjTop->SetLineColor(pSkinSimple->GetStaticLineColor());

	//// Arrow Btns
	btnArwLeft = new wxBitmapButton(
        this,
        -1,
        *pSkinSimple->GetLeftArrowButton()->GetBitmap(),
        wxPoint(29,47),
        wxSize(20,20),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetLeftArrowButton()->GetBitmapClicked() != NULL ) {
		btnArwLeft->SetBitmapSelected(*pSkinSimple->GetLeftArrowButton()->GetBitmapClicked());
	}
    btnArwLeft->Show(false);//on creation this one is always false

	btnArwRight = new wxBitmapButton(
        this,
        -1,
        *pSkinSimple->GetRightArrowButton()->GetBitmap(),
        wxPoint(301,47),
        wxSize(20,20),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetRightArrowButton()->GetBitmapClicked() != NULL ) {
		btnArwRight->SetBitmapSelected(*pSkinSimple->GetRightArrowButton()->GetBitmapClicked());
	}
    btnArwRight->Show(false);

    //
	//// Messages Play Pause Btns
	wxToolTip *ttMessages = new wxToolTip(_("Messages"));
	btnMessages = new wxBitmapButton(
        this,
        -1,
        *pSkinSimple->GetMessagesLink()->GetBitmap(),
        wxPoint(11,86),
        wxSize(70,20),
        wxBU_AUTODRAW
    );
	btnMessages->SetToolTip(ttMessages);

	wxToolTip *ttAlertMessages = new wxToolTip(_("Messages"));
	btnAlertMessages = new wxBitmapButton(
        this,
        -1,
        *(pSkinSimple->GetMessagesAlertLink()->GetBitmap()),
        wxPoint(11,86),
        wxSize(70,20),
        wxBU_AUTODRAW
    );
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
        *pSkinSimple->GetSuspendLink()->GetBitmap(),
        wxPoint(85,86),
        wxSize(59,20),
        wxBU_AUTODRAW
    );
	btnPause->SetToolTip(ttPause);

    // resume btn   
	wxToolTip *ttResume = new wxToolTip(_("Resume all Processing"));
	btnResume = new wxBitmapButton(
        this,
        -1,
        *(pSkinSimple->GetResumeLink()->GetBitmap()),
        wxPoint(85,86),
        wxSize(59,20),
        wxBU_AUTODRAW
    );
	btnResume->SetToolTip(ttResume);

    // Show resume or pause as appropriate
	btnPause->Show(true);
	btnResume->Show(false);

	CC_STATUS status;
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	if ( pDoc->IsConnected() ) {
		pDoc->GetCoreClientStatus(status);
		if ( status.task_mode == RUN_MODE_NEVER ) {
			btnPause->Show(false);
			btnResume->Show(true);
		}
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
        wxBU_AUTODRAW
    );
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
        wxBU_AUTODRAW
    );
	btnAdvancedView->SetToolTip(ttAdvView);

    /// Line
	lnMyProjBtm = new CTransparentStaticLine(this, wxID_ANY, wxPoint(29,83),wxSize(292,1));
    lnMyProjBtm->SetLineColor(pSkinSimple->GetStaticLineColor());

	Thaw();
}

void CProjectsComponent::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
    //My Projects
#ifdef __WXMAC__
	dc.SetFont(wxFont(12,74,90,92,0,wxT("Arial"))); 
#else
	dc.SetFont(wxFont(10,74,90,92,0,wxT("Arial"))); 
#endif
	dc.DrawText(wxT("My Projects:"), wxPoint(32,9)); 
}

// Check to see if the # of projects 
void CProjectsComponent::UpdateProjectArray() {
	CMainDocument* pDoc = wxGetApp().GetDocument();

	if ( pDoc->IsConnected() ) {
		int m_projCnt = pDoc->GetProjectCount();

		// If a new project has been added, figure out which one and then add it;
		if ( m_projCnt > (int) m_statProjects.size() ) {
			PROJECT* project;
			for(int i=0; i < m_projCnt; i++) {
				project = pDoc->state.projects[i];
				bool found = false;
				std::vector<StatImageLoader*>::iterator j;
				for(j=m_statProjects.begin(); j < m_statProjects.end(); j++) {
					if ( project->master_url == (*j)->m_prjUrl ) {
						found = true;
						break;
					}
				}
				if ( !found ) {
					StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
					i_statW->LoadImage();
					m_statProjects.push_back(i_statW);
					// recurse in case there is more then one change
					UpdateProjectArray();
				}
			}
		} else if ( m_projCnt < (int) m_statProjects.size() ) {
			PROJECT* project = NULL;
			std::vector<StatImageLoader*>::iterator i;
			for(i=m_statProjects.begin(); i < m_statProjects.end(); i++) {
				project = pDoc->state.lookup_project((*i)->m_prjUrl);
				if ( project == NULL ) {
					(*i)->Show(false);
					delete (*i);
					m_statProjects.erase(i);
					break;
				}
			}
		} else {
			return;
		}
		UpdateDisplayedProjects();
	} else {
		std::vector<StatImageLoader*>::iterator i;
		for(i=m_statProjects.end(); i >= m_statProjects.begin(); i--) {
			(*i)->Show(false);
			delete (*i);
		}
		m_statProjects.clear();
	}
}

void CProjectsComponent::UpdateDisplayedProjects() {
	int size = 7; 
	if ( (int) m_statProjects.size() > size ) {
		size = 6;
		if ( m_leftIndex + size >= (int) m_statProjects.size() ) {
			m_leftIndex = (int) m_statProjects.size() - size;
		}
		if ( m_leftIndex == 0 ) {
			btnArwLeft->Show(false);
			btnArwRight->Show(true);
		} else {
			btnArwLeft->Show(true);
			if ( m_leftIndex + size < (int) m_statProjects.size() ) {
				btnArwRight->Show(true);
			} else {
				btnArwRight->Show(false);
			}
		}
	} else {
		m_leftIndex = 0;
		btnArwLeft->Show(false);
		btnArwRight->Show(false);
	}

	int numProjects = (int) m_statProjects.size();
	for(int i=0; i < numProjects; i++) {
		if ( i < m_leftIndex || i >= m_leftIndex + size) {
			m_statProjects.at(i)->Show(false);
		} else {
			StatImageLoader* projIcon = m_statProjects.at(i);
			projIcon->Show(true);
			int base = -5;
			if ( size == 6 ) {
				base = 15;
			}
			projIcon->Move(wxPoint(base + 40*(i+1-m_leftIndex),37));
		}
	}
	Refresh(true);
	Update();
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

	// Check number of projects
	UpdateProjectArray();

	// Update stat icons
	for(int m = 0; m < (int)m_statProjects.size(); m++){
		StatImageLoader *i_statIcon = m_statProjects.at(m);
		i_statIcon->UpdateInterface();
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
		m_leftIndex--;
		UpdateDisplayedProjects();
		Refresh();
	} else if(m_wxBtnObj==btnArwRight){
		m_leftIndex++;
		UpdateDisplayedProjects();
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
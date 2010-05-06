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
#pragma implementation "sg_ProjectsComponent.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "str_util.h"
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
#include "wizardex.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
//#include "WizardAccountManager.h"
#include "app_ipc.h"
#include "version.h"



IMPLEMENT_DYNAMIC_CLASS(CProjectsComponent, wxPanel)

BEGIN_EVENT_TABLE(CProjectsComponent, wxPanel)
    EVT_BUTTON(ID_SIMPLE_HELP, CProjectsComponent::OnHelp)
    EVT_BUTTON(ID_SIMPLE_MESSAGES, CProjectsComponent::OnMessages)
    EVT_BUTTON(ID_SIMPLE_MESSAGES_ALERT, CProjectsComponent::OnMessages)
    EVT_BUTTON(ID_SIMPLE_SUSPEND, CProjectsComponent::OnSuspend)
    EVT_BUTTON(ID_SIMPLE_RESUME, CProjectsComponent::OnResume)
    EVT_BUTTON(ID_SIMPLE_PREFERENCES, CProjectsComponent::OnPreferences)
    EVT_BUTTON(ID_WIZARDATTACH, CProjectsComponent::OnWizardAttach)
    EVT_BUTTON(ID_WIZARDUPDATE, CProjectsComponent::OnWizardUpdate)
    EVT_PAINT(CProjectsComponent::OnPaint)
    EVT_BUTTON(-1,CProjectsComponent::OnBtnClick)
	EVT_ERASE_BACKGROUND(CProjectsComponent::OnEraseBackground)
	EVT_TIMER(ID_SIMPLEMESSAGECHECKTIMER, CProjectsComponent::OnMessageCheck)
END_EVENT_TABLE()

CProjectsComponent::CProjectsComponent() {
}


CProjectsComponent::CProjectsComponent(CSimplePanel* parent,wxPoint coord) :
    wxPanel(parent, -1, coord, wxSize(343,113), wxNO_BORDER)
{
    wxASSERT(parent);
	m_maxNumOfIcons = 6; // max number of icons in component
	m_leftIndex = 0;
    lastMessageId = 0;
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
	wxToolTip *ttAddProject = new wxToolTip(_("Volunteer for any or all of 30+ projects in many areas of science"));
	btnAddProj=new wxBitmapButton(
        this,
        ID_WIZARDATTACH,
        *pSkinSimple->GetAttachProjectButton()->GetBitmap(),
        wxPoint(214,7),
        wxSize(81,18),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetAttachProjectButton()->GetBitmapClicked() != NULL ) {
		btnAddProj->SetBitmapSelected(
			*pSkinSimple->GetAttachProjectButton()->GetBitmapClicked()
		);
	}
	btnAddProj->SetToolTip(ttAddProject);

	// syncronize button, hidden by default.
    wxToolTip *ttSynchronize = new wxToolTip(_("Synchronize projects with account manager system"));
	btnSynchronize=new wxBitmapButton(
        this,
        ID_WIZARDUPDATE,
        *pSkinSimple->GetSynchronizeButton()->GetBitmap(),
        wxPoint(214,7),
        wxSize(81,18),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetSynchronizeButton()->GetBitmapClicked() != NULL ) {
		btnSynchronize->SetBitmapSelected(
			*pSkinSimple->GetSynchronizeButton()->GetBitmapClicked()
		);
	}
	btnSynchronize->SetToolTip(ttSynchronize);
    btnSynchronize->Show(false);

    /// Help
	wxToolTip *ttHelp = new wxToolTip(_("Get help with BOINC"));
	btnHelp=new wxBitmapButton(
        this,
        ID_SIMPLE_HELP,
        *pSkinSimple->GetHelpButton()->GetBitmap(),
        wxPoint(300,7),
        wxSize(
            (*pSkinSimple->GetHelpButton()->GetBitmap()).GetWidth(),
            (*pSkinSimple->GetHelpButton()->GetBitmap()).GetHeight()
        ),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetHelpButton()->GetBitmapClicked() != NULL ) {
		btnHelp->SetBitmapSelected(
			*pSkinSimple->GetHelpButton()->GetBitmapClicked()
		);
	}
	btnHelp->SetToolTip(ttHelp);

	
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
	wxToolTip *ttMessages = new wxToolTip(_("Open a window to view messages"));
	btnMessages = new CLinkButton(
        this,
        ID_SIMPLE_MESSAGES,
        *pSkinSimple->GetMessagesLink()->GetBitmap(),
        wxPoint(11,86),
        wxSize(70,20),
        wxBU_AUTODRAW
    );
	btnMessages->SetToolTip(ttMessages);

	wxToolTip *ttAlertMessages = new wxToolTip(_("Open a window to view messages"));
	btnAlertMessages = new CLinkButton(
        this,
        ID_SIMPLE_MESSAGES_ALERT,
        *(pSkinSimple->GetMessagesAlertLink()->GetBitmap()),
        wxPoint(11,86),
        wxSize(70,20),
        wxBU_AUTODRAW
    );
	btnAlertMessages->SetToolTip(ttAlertMessages);
	btnAlertMessages->Show(false);

    //spacer
	w_sp1 = new wxWindow(this,-1,wxPoint(83,91),wxSize(2,11));
    i_spacer1 = new ImageLoader(w_sp1);
    i_spacer1->LoadImage(*(pSkinSimple->GetSpacerImage()->GetBitmap()));

    // pause btn
	wxToolTip *ttPause = new wxToolTip(_("Stop all activity"));
	btnPause = new CLinkButton(
        this,
        ID_SIMPLE_SUSPEND,
        *pSkinSimple->GetSuspendLink()->GetBitmap(),
        wxPoint(85,86),
        wxSize(59,20),
        wxBU_AUTODRAW
    );
	btnPause->SetToolTip(ttPause);

    // resume btn   
	wxToolTip *ttResume = new wxToolTip(_("Resume activity"));
	btnResume = new CLinkButton(
        this,
        ID_SIMPLE_RESUME,
        *(pSkinSimple->GetResumeLink()->GetBitmap()),
        wxPoint(85,86),
        wxSize(59,20),
        wxBU_AUTODRAW
    );
	btnResume->SetToolTip(ttResume);

	//spacer
	w_sp2 = new wxWindow(this,-1,wxPoint(144,91),wxSize(2,11));
    i_spacer2 = new ImageLoader(w_sp2);
    i_spacer2->LoadImage(*(pSkinSimple->GetSpacerImage()->GetBitmap()));

    // Pref Btn
	wxToolTip *ttPreferences = new wxToolTip(_("Open a window to set your preferences"));
	btnPreferences = new CLinkButton(
        this,
        ID_SIMPLE_PREFERENCES,
        *(pSkinSimple->GetPreferencesLink()->GetBitmap()),
        wxPoint(149,86),
        wxSize(81,20),
        wxBU_AUTODRAW
    );
	btnPreferences->SetToolTip(ttPreferences);

    //spacer
	w_sp3 = new wxWindow(this,-1,wxPoint(230,91),wxSize(2,11));
    i_spacer3 = new ImageLoader(w_sp3);
    i_spacer3->LoadImage(*(pSkinSimple->GetSpacerImage()->GetBitmap()));

    // Advanced View
	wxToolTip *ttAdvView = new wxToolTip(_("Switch to the BOINC advanced view"));
	btnAdvancedView = new CLinkButton(
        this,
        ID_CHANGEGUI,
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
	dc.DrawText(_("My Projects:"), wxPoint(32,9)); 
}

// Check to see if the # of projects 
void CProjectsComponent::UpdateProjectArray() {
	CMainDocument* pDoc = wxGetApp().GetDocument();

	if ( pDoc->IsConnected() ) {
		int m_projCnt = pDoc->GetSimpleProjectCount();

		// If a new project has been added, figure out which one and then add it;
		if ( m_projCnt > (int) m_statProjects.size() ) {
			PROJECT* project;
			for(int i=0; i < m_projCnt; i++) {
				project = pDoc->state.projects[i];
				bool found = false;
				std::vector<StatImageLoader*>::iterator j;
				for(j=m_statProjects.begin(); j < m_statProjects.end(); j++) {
					if (!strcmp(project->master_url, (*j)->project_url)) {
						found = true;
						break;
					}
				}
				if ( !found ) {
					StatImageLoader *i_statW = new StatImageLoader(this, project->master_url);
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
				project = pDoc->state.lookup_project((*i)->project_url);
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


void CProjectsComponent::OnHelp(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnHelp - Function Begin"));

	wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

    wxString wxurl;
    wxurl.Printf(
        wxT("%s?target=simple&version=%s&controlid=%d"),
        strURL.c_str(),
        wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
        event.GetId()
    );

    wxLaunchDefaultBrowser(wxurl);
        
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnHelp - Function End"));
}


void CProjectsComponent::OnMessages(wxCommandEvent& /*event*/) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnMessages - Function Begin"));

	CSimplePanel* pPanel = wxDynamicCast(GetParent(), CSimplePanel);
    wxASSERT(pPanel);

    MessagesViewed();


	CDlgMessages dlg(GetParent());
    pPanel->SetDlgOpen(true);
    ((CSimpleFrame*)pPanel->GetParent())->SetMsgsDlgOpen(&dlg);
    
    dlg.ShowModal();

    pPanel->SetDlgOpen(false);
    ((CSimpleFrame*)pPanel->GetParent())->SetMsgsDlgOpen(NULL);

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnMessages - Function End"));
}


void CProjectsComponent::OnSuspend(wxCommandEvent& /*event*/) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnSuspend - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->SetActivityRunMode(RUN_MODE_NEVER, 3600);

    btnPause->Show(false);
    btnResume->Show(true);

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnSuspend - Function End"));
}


void CProjectsComponent::OnResume(wxCommandEvent& /*event*/) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnResume - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();
    CC_STATUS ccs;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    pDoc->GetCoreClientStatus(ccs);
    if ((RUN_MODE_NEVER == ccs.task_mode) && (0 >= ccs.task_mode_delay)) {
        pDoc->SetActivityRunMode(RUN_MODE_AUTO, 0);
    } else {
        pDoc->SetActivityRunMode(RUN_MODE_RESTORE, 0);
    }

    btnResume->Show(false);
    btnPause->Show(true);

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnResume - Function End"));
}


void CProjectsComponent::OnPreferences(wxCommandEvent& /*event*/) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnPreferences - Function Begin"));

	CSimplePanel* pPanel = wxDynamicCast(GetParent(), CSimplePanel);

    wxASSERT(pPanel);

	pPanel->SetDlgOpen(true);

	CDlgPreferences dlg(GetParent());
    dlg.ShowModal();

    pPanel->SetDlgOpen(false);

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnPreferences - Function End"));
}


void CProjectsComponent::OnWizardAttach(wxCommandEvent& /*event*/) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnWizardAttach - Function Begin"));

	CSimplePanel* pPanel = wxDynamicCast(GetParent(), CSimplePanel);

    wxASSERT(pPanel);

	pPanel->SetDlgOpen(true);

	pPanel->OnProjectsAttachToProject();
	btnAddProj->Refresh();

    pPanel->SetDlgOpen(false);

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnWizardAttach - Function End"));
}


void CProjectsComponent::OnWizardUpdate(wxCommandEvent& /*event*/) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnWizardUpdate - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
	CSimplePanel*  pPanel = wxDynamicCast(GetParent(), CSimplePanel);

    wxASSERT(pDoc);
    wxASSERT(pPanel);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {

	    pPanel->SetDlgOpen(true);

        CWizardAttachProject* pWizard = new CWizardAttachProject(this);

        pWizard->SyncToAccountManager();

        if (pWizard)
            pWizard->Destroy();

    	btnSynchronize->Refresh();

        pPanel->SetDlgOpen(false);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectsComponent::OnWizardUpdate - Function End"));
}


void CProjectsComponent::UpdateInterface()
{
	CMainDocument* pDoc = wxGetApp().GetDocument();

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

    // Should we display the synchronize button instead of the
    //   attach to project button?
	CC_STATUS       status;
    bool            is_acct_mgr_detected = false;

	pDoc->GetCoreClientStatus(status);

    is_acct_mgr_detected = pDoc->ami.acct_mgr_url.size() ? true : false;

    if (is_acct_mgr_detected) {
		btnAddProj->Show(false);
		btnSynchronize->Show(true);
	} else {
        if (!status.disallow_attach) {
		    btnAddProj->Show(true);
        }
		btnSynchronize->Show(false);
    }

    // Show resume or pause as appropriate
    if (RUN_MODE_NEVER == status.task_mode) {
		btnPause->Show(false);
		btnResume->Show(true);
	} else {
		btnPause->Show(true);
		btnResume->Show(false);
	}

    // Should we disable the attach to project button?
    if (status.disallow_attach || is_acct_mgr_detected) {
        btnAddProj->Show(false);
    } else {
        btnAddProj->Show(true);
    }

    // Should we only be able to see the simple gui?
    if (status.simple_gui_only) {
        btnAdvancedView->Show(false);
    } else {
        btnAdvancedView->Show(true);
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

    // synchronize btn
	btnSynchronize->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnSynchronize->SetBitmapLabel(*(pSkinSimple->GetSynchronizeButton()->GetBitmap()));
    btnSynchronize->SetBitmapSelected(*(pSkinSimple->GetSynchronizeButton()->GetBitmapClicked()));

    // help btn
	btnHelp->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
    btnHelp->SetBitmapLabel(*(pSkinSimple->GetHelpButton()->GetBitmap()));
    btnHelp->SetBitmapSelected(*(pSkinSimple->GetHelpButton()->GetBitmapClicked()));

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

    // spacers
    i_spacer1->LoadImage(*(pSkinSimple->GetSpacerImage()->GetBitmap()));
    i_spacer2->LoadImage(*(pSkinSimple->GetSpacerImage()->GetBitmap()));
    i_spacer2->LoadImage(*(pSkinSimple->GetSpacerImage()->GetBitmap()));

	// Rebuild stat menus and reload icons
	for(int m = 0; m < (int)m_statProjects.size(); m++){
		StatImageLoader *i_statImage = m_statProjects.at(m);
		i_statImage->LoadImage();
		i_statImage->RebuildMenu();
	}
}

void CProjectsComponent::OnBtnClick(wxCommandEvent& event){ //init function
	wxObject *m_wxBtnObj = event.GetEventObject();

	if (m_wxBtnObj==btnArwLeft){
		m_leftIndex--;
		UpdateDisplayedProjects();
		Refresh();
	} else if(m_wxBtnObj==btnArwRight){
		m_leftIndex++;
		UpdateDisplayedProjects();
		Refresh();
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
			if ( message != NULL && message->priority == MSG_USER_ALERT ) {
				receivedErrorMessage = true;
				checkForMessagesTimer->Stop();
				break;
			}
		}
	}
}


void CProjectsComponent::MessagesViewed() {
	receivedErrorMessage = false;
	CMainDocument* pDoc = wxGetApp().GetDocument();
	lastMessageId = pDoc->GetMessageCount();
	checkForMessagesTimer->Start();
}

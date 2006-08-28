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
#include "BOINCGUIApp.h"
#include "sg_BoincSimpleGUI.h"
#include "sg_SkinClass.h"
#include "sg_ImageLoader.h"
#include "sg_StaticLine.h"
#include "sg_StatImageLoader.h" 
#include "sg_DlgMessages.h"
#include "sg_DlgPreferences.h"
#include "sg_ProjectsComponent.h"

#include "app_ipc.h"

#define ID_CHECKFORERRORMESSAGETIMER  13000

IMPLEMENT_DYNAMIC_CLASS(CProjectsComponent, wxPanel)


BEGIN_EVENT_TABLE(CProjectsComponent, wxPanel)
    EVT_PAINT(CProjectsComponent::OnPaint)
    EVT_BUTTON(-1,CProjectsComponent::OnBtnClick)
	EVT_ERASE_BACKGROUND(CProjectsComponent::OnEraseBackground)
	EVT_TIMER(ID_CHECKFORERRORMESSAGETIMER, CProjectsComponent::CheckForErrorMessages)
END_EVENT_TABLE()

int CProjectsComponent::lastMessageId = 0;

CProjectsComponent::CProjectsComponent() {}


CProjectsComponent::CProjectsComponent(CSimpleFrame* parent,wxPoint coord) :
    wxPanel(parent, -1, coord, wxSize(343,113), wxNO_BORDER)
{
    wxASSERT(parent);
	m_maxNumOfIcons = 6; // max number of icons in component
	m_rightIndex = 0;
	m_leftIndex = 0;
	m_projCnt = 0;
    LoadSkinImages();
	CreateComponent();

	receivedErrorMessage = false;
	alertMessageDisplayed = false;
	checkForMessagesTimer = new wxTimer(this, ID_CHECKFORERRORMESSAGETIMER);
	checkForMessagesTimer->Start(5000); 

}

CProjectsComponent::~CProjectsComponent() {
	if ( checkForMessagesTimer->IsRunning() ) {
		checkForMessagesTimer->Stop();
	}
	delete checkForMessagesTimer;
}
void CProjectsComponent::LoadSkinImages(){

	//app skin class
	appSkin = SkinClass::Instance();
	wxString dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	//
	//component bg
	fileImgBuf[0].LoadFile(dirPref + appSkin->GetProjCompBg(),wxBITMAP_TYPE_PNG);
	// default stat icon
	g_statIcnDefault = new wxImage(dirPref + appSkin->GetDefaultStatIcn(), wxBITMAP_TYPE_PNG);
	// arrows
	g_arwLeft = new wxImage(dirPref + appSkin->GetBtnLeftArr(), wxBITMAP_TYPE_PNG);
	g_arwRight = new wxImage(dirPref + appSkin->GetBtnRightArr(), wxBITMAP_TYPE_PNG); 
	g_arwLeftClick = new wxImage(dirPref + appSkin->GetBtnLeftArrClick(), wxBITMAP_TYPE_PNG);
	g_arwRightClick = new wxImage(dirPref + appSkin->GetBtnRightArrClick(), wxBITMAP_TYPE_PNG);
	// add proj
	g_addProj = new wxImage(dirPref + appSkin->GetBtnAddProj(), wxBITMAP_TYPE_PNG);
	g_addProjClick = new wxImage(dirPref + appSkin->GetBtnAddProjClick(), wxBITMAP_TYPE_PNG);
    // messages
	g_messages = new wxImage(dirPref + appSkin->GetBtnMessages(), wxBITMAP_TYPE_PNG);
	g_messagesClick = new wxImage(dirPref + appSkin->GetBtnMessages(), wxBITMAP_TYPE_PNG);
    // error messages
	g_alertMessages = new wxImage(dirPref + appSkin->GetBtnAlertMessages(), wxBITMAP_TYPE_PNG);
	g_alertMessagesClick = new wxImage(dirPref + appSkin->GetBtnAlertMessages(), wxBITMAP_TYPE_PNG);
	// pause
	g_pause = new wxImage(dirPref + appSkin->GetBtnPause(), wxBITMAP_TYPE_PNG);
	g_pauseClick = new wxImage(dirPref + appSkin->GetBtnPause(), wxBITMAP_TYPE_PNG);
	// resume
	g_resume = new wxImage(dirPref + appSkin->GetBtnResume(), wxBITMAP_TYPE_PNG);
	g_resumeClick = new wxImage(dirPref + appSkin->GetBtnResume(), wxBITMAP_TYPE_PNG);
    // resume
	g_pref = new wxImage(dirPref + appSkin->GetBtnPrefer(), wxBITMAP_TYPE_PNG);
	g_prefClick = new wxImage(dirPref + appSkin->GetBtnPrefer(), wxBITMAP_TYPE_PNG);
    // resume
	g_advView = new wxImage(dirPref + appSkin->GetBtnAdvView(), wxBITMAP_TYPE_PNG);
	g_advViewClick = new wxImage(dirPref + appSkin->GetBtnAdvView(), wxBITMAP_TYPE_PNG);
	//spacer
	g_spacer = new wxImage(dirPref + appSkin->GetSpacerImage(), wxBITMAP_TYPE_PNG);
	
   
	btmpComponentBg=&fileImgBuf[0];
	btmpArwL= wxBitmap(g_arwLeft); 
    btmpArwR= wxBitmap(g_arwRight); 
    btmpArwLC= wxBitmap(g_arwLeftClick); 
    btmpArwRC= wxBitmap(g_arwRightClick); 
	btmpAddProj= wxBitmap(g_addProj);
	btmpAddProjC= wxBitmap(g_addProjClick);
	btmpMessages= wxBitmap(g_messages);
	btmpMessagesC= wxBitmap(g_messagesClick);
	btmpAlertMessages= wxBitmap(g_alertMessages);
	btmpAlertMessagesC= wxBitmap(g_alertMessagesClick);
	btmpPause= wxBitmap(g_pause);
	btmpPauseC= wxBitmap(g_pauseClick);
	btmpResume= wxBitmap(g_resume);
	btmpResumeC= wxBitmap(g_resumeClick);
	btmpPref= wxBitmap(g_pref);
	btmpPrefC= wxBitmap(g_prefClick);
	btmpAdvView= wxBitmap(g_advView);
	btmpAdvViewC= wxBitmap(g_advViewClick);

}

void CProjectsComponent::CreateComponent()
{
	//Set Background color
	SetBackgroundColour(appSkin->GetAppBgCol());
	//Static content in my Projects section
	// add project button
	wxToolTip *ttAddProject = new wxToolTip(wxT("Add Project"));
	btnAddProj=new wxBitmapButton(this,-1,btmpAddProj,wxPoint(235,7),wxSize(81,18),wxNO_BORDER);
	btnAddProj->SetBitmapSelected(btmpAddProjC);
	btnAddProj->SetToolTip(ttAddProject);
	/// Line
	lnMyProjTop = new CStaticLine(this,wxPoint(29,29),wxSize(292,1));
	lnMyProjTop->SetLineColor(appSkin->GetStaticLineCol());
	/////////////// ICONS /////////////////////
	CMainDocument* pDoc     = wxGetApp().GetDocument();
    m_projCnt = pDoc->GetProjectCount();
	projectIconName = "stat_icon";
	defaultIcnPath[256];
	// url of project directory
	urlDirectory[256];

	for(int j = 0; j < m_projCnt; j++){
		PROJECT* project = pDoc->state.projects[j];
		//user credit text
		userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
		toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		if(j < m_maxNumOfIcons){
			// Project button
			wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
			i_statW->Move(wxPoint(55 + 40*j,37));
			
			// resolve the proj image 
			url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
			dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
			i_statW->LoadImage(dirProjectGraphic, g_statIcnDefault);
			i_statW->SetToolTip(statToolTip);
			
			// push icon in the vector
			m_statProjects.push_back(i_statW);
			//increment left index
			m_rightIndex ++;
		}
		
	}
	//// Arrow Btns
	btnArwLeft=new wxBitmapButton(this,-1,btmpArwL,wxPoint(29,47),wxSize(20,20),wxNO_BORDER);
	btnArwLeft->SetBitmapSelected(btmpArwLC);
    btnArwLeft->Show(false);//on creation this one is always false
	btnArwRight=new wxBitmapButton(this,-1,btmpArwR,wxPoint(301,47),wxSize(20,20),wxNO_BORDER);
	btnArwRight->SetBitmapSelected(btmpArwRC);
	if(m_projCnt > m_maxNumOfIcons){//right shows up only if there is more than max num of icons
		btnArwRight->Show(true);
	}else{
        btnArwRight->Show(false);
	}
	//
	//// Messages Play Pause Btns
	wxToolTip *ttMessages = new wxToolTip(wxT("Messages"));
	btnMessages=new wxBitmapButton(this,-1,btmpMessages,wxPoint(11,86),wxSize(70,20),wxNO_BORDER);
	btnMessages->SetBitmapSelected(btmpMessagesC);
	btnMessages->SetToolTip(ttMessages);
	wxToolTip *ttAlertMessages = new wxToolTip(wxT("Messages"));
	btnAlertMessages=new wxBitmapButton(this,-1,btmpAlertMessages,wxPoint(11,86),wxSize(70,20),wxNO_BORDER);
	btnAlertMessages->SetBitmapSelected(btmpAlertMessagesC);
	btnAlertMessages->SetToolTip(ttAlertMessages);
	btnAlertMessages->Show(false);
	//spacer
	wxWindow *w_sp1 = new wxWindow(this,-1,wxPoint(83,91),wxSize(2,11));
    i_spacer = new ImageLoader(w_sp1);
    i_spacer->LoadImage(g_spacer);
	// pause btn
	wxToolTip *ttPause = new wxToolTip(wxT("Pause all processing"));
	btnPause=new wxBitmapButton(this,-1,btmpPause,wxPoint(85,86),wxSize(59,20),wxNO_BORDER);
	btnPause->SetBitmapSelected(btmpPauseC);
	btnPause->SetToolTip(ttPause);
    // resume btn   
	wxToolTip *ttResume = new wxToolTip(wxT("Resume all Processing"));
	btnResume=new wxBitmapButton(this,-1,btmpResume,wxPoint(85,86),wxSize(59,20),wxNO_BORDER);
	btnResume->SetBitmapSelected(btmpResumeC);
	btnResume->SetToolTip(ttResume);
	btnResume->Show(false);
	//spacer
	wxWindow *w_sp2 = new wxWindow(this,-1,wxPoint(144,91),wxSize(2,11));
    i_spacer = new ImageLoader(w_sp2);
    i_spacer->LoadImage(g_spacer);
	// Pref Btn
	wxToolTip *ttPreferences = new wxToolTip(wxT("Preferences"));
	btnPreferences=new wxBitmapButton(this,-1,btmpPref,wxPoint(149,86),wxSize(81,20),wxNO_BORDER);
	btnPreferences->SetBitmapSelected(btmpPrefC);
	btnPreferences->SetToolTip(ttPreferences);
	//spacer
	wxWindow *w_sp3 = new wxWindow(this,-1,wxPoint(230,91),wxSize(2,11));
    i_spacer = new ImageLoader(w_sp3);
    i_spacer->LoadImage(g_spacer);
	// Advanced View
	wxToolTip *ttAdvView = new wxToolTip(wxT("Advanced View"));
	btnAdvancedView=new wxBitmapButton(this,-1,btmpAdvView,wxPoint(233,86),wxSize(101,20),wxNO_BORDER);
    btnAdvancedView->SetBitmapSelected(btmpAdvViewC);
	btnAdvancedView->SetToolTip(ttAdvView);
	/// Line
	lnMyProjBtm = new CStaticLine(this,wxPoint(29,83),wxSize(292,1));
	lnMyProjBtm->SetLineColor(appSkin->GetStaticLineCol());
}
void CProjectsComponent::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
    //My Projects
	dc.SetFont(wxFont(10,74,90,92,0,wxT("Arial"))); 
	dc.DrawText(wxT("My Projects:"), wxPoint(32,9)); 
}
void CProjectsComponent::RemoveProject(std::string prjUrl)
{	
	CMainDocument* pDoc     = wxGetApp().GetDocument();
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
			userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
			toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		    wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
		    i_statW->Move(wxPoint(55,37));
			// resolve the proj image 
			url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
			dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
			i_statW->LoadImage(dirProjectGraphic, g_statIcnDefault);
			i_statW->SetToolTip(statToolTip);
			
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
			userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
			toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
			wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
			i_statW->Move(wxPoint(55 + 40*(m_maxNumOfIcons-1),37));
			// resolve the proj image 
			url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
			dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
			i_statW->LoadImage(dirProjectGraphic, g_statIcnDefault);
			i_statW->SetToolTip(statToolTip);
			
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
	CMainDocument* pDoc     = wxGetApp().GetDocument();

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

	// Check number of projects
	int oldProjCnt = m_projCnt;
	m_projCnt = pDoc->GetProjectCount();
	if(m_projCnt == oldProjCnt){
		return;
	}

	if(m_projCnt <= m_maxNumOfIcons){
		PROJECT* project = pDoc->state.projects.at(m_projCnt-1);
		userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
		toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
		StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
		i_statW->Move(wxPoint(55 + 40*(m_projCnt-1),37));
		// resolve the proj image 
		url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
		dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
		i_statW->LoadImage(dirProjectGraphic, g_statIcnDefault);
		i_statW->SetToolTip(statToolTip);
		
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
	//Load new skin images
	LoadSkinImages();
	//Set Background color only
	SetBackgroundColour(appSkin->GetAppBgCol());
	//right button
	btnArwRight->SetBackgroundColour(appSkin->GetAppBgCol());
	btnArwRight->SetBitmapLabel(btmpArwR);
	btnArwRight->SetBitmapSelected(btmpArwRC);
	//left button
	btnArwLeft->SetBackgroundColour(appSkin->GetAppBgCol());
	btnArwLeft->SetBitmapLabel(btmpArwL);
	btnArwLeft->SetBitmapSelected(btmpArwLC);
	// add project btn
	btnAddProj->SetBackgroundColour(appSkin->GetAppBgCol());
	btnAddProj->SetBitmapLabel(btmpAddProj);
	btnAddProj->SetBitmapSelected(btmpAddProjC);
	// messages btn
	btnMessages->SetBackgroundColour(appSkin->GetAppBgCol());
	btnMessages->SetBitmapLabel(btmpMessages);
	btnMessages->SetBitmapSelected(btmpMessagesC);
	// alert messages btn
	btnAlertMessages->SetBackgroundColour(appSkin->GetAppBgCol());
	btnAlertMessages->SetBitmapLabel(btmpAlertMessages);
	btnAlertMessages->SetBitmapSelected(btmpAlertMessagesC);
	// pause btn
	btnPause->SetBackgroundColour(appSkin->GetAppBgCol());
	btnPause->SetBitmapLabel(btmpPause);
	btnPause->SetBitmapSelected(btmpPauseC);
	// resume btn
    btnResume->SetBackgroundColour(appSkin->GetAppBgCol());
	btnResume->SetBitmapLabel(btmpResume);
	btnResume->SetBitmapSelected(btmpResumeC);
	// preferences btn
    btnPreferences->SetBackgroundColour(appSkin->GetAppBgCol());
	btnPreferences->SetBitmapLabel(btmpPref);
	btnPreferences->SetBitmapSelected(btmpPrefC);
	// advance view btn
    btnAdvancedView->SetBackgroundColour(appSkin->GetAppBgCol());
	btnAdvancedView->SetBitmapLabel(btmpAdvView);
	btnAdvancedView->SetBitmapSelected(btmpAdvViewC);
	//set line colors
	lnMyProjTop->SetLineColor(appSkin->GetStaticLineCol());
	lnMyProjBtm->SetLineColor(appSkin->GetStaticLineCol());

}

void CProjectsComponent::OnBtnClick(wxCommandEvent& event){ //init function
	wxObject *m_wxBtnObj = event.GetEventObject();

	CSimpleFrame* pFrame      = wxDynamicCast(GetParent(), CSimpleFrame);
    wxASSERT(pFrame);


	if(m_wxBtnObj==btnArwLeft){
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
			userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
			toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		    wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
		    i_statW->Move(wxPoint(55,37));
			// resolve the proj image 
			url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
			dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
			i_statW->LoadImage(dirProjectGraphic, g_statIcnDefault);

			i_statW->SetToolTip(statToolTip);
			
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

	}else if(m_wxBtnObj==btnArwRight){
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
			userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
			toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		    wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url);
		    i_statW->Move(wxPoint(55 + 40*(m_maxNumOfIcons-1),37));
			// resolve the proj image 
			url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
			dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
			i_statW->LoadImage(dirProjectGraphic, g_statIcnDefault);
			i_statW->SetToolTip(statToolTip);
			
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
	}else if(m_wxBtnObj==btnAddProj){
		pFrame->OnProjectsAttachToProject();
		btnAddProj->Refresh();
	}else if(m_wxBtnObj==btnMessages || m_wxBtnObj==btnAlertMessages){
		MessagesViewed();
		CDlgMessages* pDlg = new CDlgMessages(NULL,appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/"));
		wxASSERT(pDlg);
		pDlg->ShowModal();
		pDlg->Destroy();
    }else if(m_wxBtnObj==btnPause) {
		CMainDocument* pDoc     = wxGetApp().GetDocument();
		pDoc->GetActivityRunMode(clientRunMode);
		pDoc->SetActivityRunMode(RUN_MODE_NEVER);
		btnPause->Show(false);
		btnResume->Show(true);
    }else if(m_wxBtnObj==btnResume) {
		CMainDocument* pDoc     = wxGetApp().GetDocument();
		pDoc->SetActivityRunMode(clientRunMode);
		btnResume->Show(false);
		btnPause->Show(true);
    }else if(m_wxBtnObj==btnPreferences){
		CDlgPreferences* pDlg = new CDlgPreferences(NULL,appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/"));
		wxASSERT(pDlg);
		if ( pDlg->ShowModal() == wxID_OK ){
			if(pDlg->GetSkinName() != pFrame->skinName){
				if ( appSkin->change_skin(pDlg->GetSkinName()) ) {
					pFrame->skinName = pDlg->GetSkinName();
					pFrame->ReskinAppGUI();
				} else {
					wxMessageBox("Incompatible skin. Skin will not be changed.");
					pDlg->SetSkinName(pFrame->skinName);
				}
		   }
		}
		pDlg->Destroy();
    }else if(m_wxBtnObj==btnAdvancedView) {
        wxGetApp().SetActiveGUI(BOINC_ADVANCEDGUI, true);
    }
}
void CProjectsComponent::OnEraseBackground(wxEraseEvent& event){
  wxObject *m_wxWin = event.GetEventObject();
  if(m_wxWin==this){event.Skip(true);DrawBackImg(event,this,*btmpComponentBg,0);return;}
  event.Skip(true);
}
void CProjectsComponent::DrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz){

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

void CProjectsComponent::CheckForErrorMessages(wxTimerEvent& WXUNUSED(event)) {
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	MESSAGE* message;
	// Only look at the messages recieved since the last time we looked
	if ( pDoc->GetMessageCount() > lastMessageId ) {
		// Loop through and check for any messages recieved that are error messages
		for(int i=lastMessageId; i < pDoc->messages.messages.size(); i++) {
			lastMessageId = i+1;
			message = pDoc->message(i);
			if ( message != NULL && message->priority == MSG_PRIORITY_ERROR ) {
				receivedErrorMessage = true;
				checkForMessagesTimer->Stop();
				lastMessageId = pDoc->messages.messages.size();
				break;
			}
		}
	}
}

void CProjectsComponent::MessagesViewed() {
	receivedErrorMessage = false;
	checkForMessagesTimer->Start();
}
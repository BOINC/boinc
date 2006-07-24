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
#include "sg_ProjectsComponent.h"
#include "sg_SkinClass.h"
#include "sg_StatImageLoader.h" 
#include "sg_BoincSimpleGUI.h"

#include "app_ipc.h"


IMPLEMENT_DYNAMIC_CLASS(CProjectsComponent, wxPanel)

enum{
	BTN_SHOW_GRAPHICS,
	BTN_COLLAPSE,
};


BEGIN_EVENT_TABLE(CProjectsComponent, wxPanel)
    EVT_BUTTON(-1,CProjectsComponent::OnBtnClick)
END_EVENT_TABLE()

CProjectsComponent::CProjectsComponent() {}

CProjectsComponent::CProjectsComponent(CSimpleFrame* parent,wxPoint coord) :
    wxPanel(parent, -1, coord, wxSize(370,60), wxNO_BORDER)
{
    wxASSERT(parent);
	m_maxNumOfIcons = 6; // max number of icons in component
	m_rightIndex = 0;
	m_leftIndex = 0;
    LoadSkinImages();
	CreateComponent();

}

CProjectsComponent::~CProjectsComponent() {}
void CProjectsComponent::LoadSkinImages(){

	//app skin class
	appSkin = SkinClass::Instance();
	wxString dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	// default stat icon
	g_statIcnDefault = new wxImage(dirPref + appSkin->GetDefaultStatIcn(), wxBITMAP_TYPE_PNG);
	// arrows
	g_arwLeft = new wxImage(dirPref + appSkin->GetBtnLeftArr(), wxBITMAP_TYPE_PNG);
	g_arwRight = new wxImage(dirPref + appSkin->GetBtnRightArr(), wxBITMAP_TYPE_PNG);
	g_arwLeftClick = new wxImage(dirPref + appSkin->GetBtnLeftArrClick(), wxBITMAP_TYPE_PNG);
	g_arwRightClick = new wxImage(dirPref + appSkin->GetBtnRightArrClick(), wxBITMAP_TYPE_PNG);
	btmpArwL= wxBitmap(g_arwLeft); 
    btmpArwR= wxBitmap(g_arwRight); 
    btmpArwLC= wxBitmap(g_arwLeftClick); 
    btmpArwRC= wxBitmap(g_arwRightClick); 
	
}

void CProjectsComponent::CreateComponent()
{
	//Set Background color
	SetBackgroundColour(appSkin->GetAppBgCol());
	/////////////// ICONS /////////////////////
	CMainDocument* pDoc     = wxGetApp().GetDocument();
    m_projCnt = (int)pDoc->state.projects.size();
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
			//wxWindow *w_statW = new wxWindow(this,-1,wxPoint(29 + 52*j,3),wxSize(52,52));
			wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url,j);
			i_statW->Move(wxPoint(29 + 52*j,3));
			
			// resolve the proj image 
			url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
			dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
			//load stat icon
			if(boinc_resolve_filename(dirProjectGraphic.c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
				g_statIcn = new wxImage(defaultIcnPath, wxBITMAP_TYPE_PNG);
				i_statW->LoadImage(g_statIcn);
			}else{
				i_statW->LoadImage(g_statIcnDefault);
			}

			i_statW->SetToolTip(statToolTip);
			
			// push icon in the vector
			m_statProjects.push_back(i_statW);
			//increment left index
			m_leftIndex ++;
		}
		
	}
	//// Arrow Btns
	btnArwLeft=new wxBitmapButton(this,-1,btmpArwL,wxPoint(2,18),wxSize(24,24),wxSIMPLE_BORDER);
	btnArwLeft->SetBitmapSelected(btmpArwLC);
    btnArwLeft->Show(false);//on creation this one is always false
	btnArwRight=new wxBitmapButton(this,-1,btmpArwR,wxPoint(344,18),wxSize(24,24),wxNO_BORDER);
	btnArwRight->SetBitmapSelected(btmpArwRC);
	if(m_projCnt > m_maxNumOfIcons){//right shows up only if there is more than max num of icons
		btnArwRight->Show(true);
	}else{
        btnArwRight->Show(false);
	}
	///////////
	
}
void CProjectsComponent::RemoveProject(std::string prjUrl)
{	
	CMainDocument* pDoc     = wxGetApp().GetDocument();
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
	//shift icons to the left. Nothing will be shifted if last icon is removed
	for(int k = indexOfIcon; k < (int)m_statProjects.size(); k++){
		StatImageLoader *i_statWShifting = m_statProjects.at(k);
		i_statWShifting->Move(wxPoint(29 + 52*k,3));
		int hj = 9;
	}
	//update project count
	m_projCnt = (int)pDoc->state.projects.size();
	if(m_leftIndex+1 <= m_projCnt){
		PROJECT* project = pDoc->state.projects.at(m_leftIndex);
		userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
		toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
		StatImageLoader *i_statW = new StatImageLoader(this,project->master_url,m_leftIndex+1);
		i_statW->Move(wxPoint(29 + 52*(m_maxNumOfIcons-1),3));
		// resolve the proj image 
		url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
		dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
		if(boinc_resolve_filename(dirProjectGraphic.c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
			g_statIcn = new wxImage(defaultIcnPath, wxBITMAP_TYPE_PNG);
			i_statW->LoadImage(g_statIcn);
		}else{
			i_statW->LoadImage(g_statIcnDefault);
		}

		i_statW->SetToolTip(statToolTip);
		
		// push icon in the vector
		m_statProjects.push_back(i_statW);
		//increment left index
		m_leftIndex ++;
		//increment right index
		m_rightIndex ++;
	}
	//hide right arrow if we are at the end of list now
	if(m_leftIndex >= m_projCnt){
		btnArwRight->Show(false);
	}


}
void CProjectsComponent::UpdateInterface()
{
	/*CMainDocument* pDoc     = wxGetApp().GetDocument();
	
	RESULT* result = pDoc->results.results[m_tabIndex];
	wxString strBuffer = wxEmptyString;
	//Gauge
	gaugeWUMain->SetValue(floor(result->fraction_done * 100000)/1000);
	// Elapsed Time
	FormatCPUTime(result, strBuffer);
	lblElapsedTimeValue->SetLabel(strBuffer);
	lblElapsedTimeValue->Refresh();
    // Remaining time
	FormatTimeToCompletion(result, strBuffer);
	lblTimeRemainingValue->SetLabel(strBuffer);
	lblTimeRemainingValue->Refresh();
*/
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
}

void CProjectsComponent::OnBtnClick(wxCommandEvent& event){ //init function
	wxObject *m_wxBtnObj = event.GetEventObject();

	if(m_wxBtnObj==btnArwLeft){
		//delete proj icon at position max number  - 1(5)
		delete m_statProjects.at(m_maxNumOfIcons-1);
        //remove last element from vector
		m_statProjects.pop_back();
		//shift icons right
		for(int m = 0; m < (int)m_statProjects.size(); m++){
			StatImageLoader *i_statWShifting = m_statProjects.at(m);
			i_statWShifting->Move(wxPoint(29 + 52*(m+1),3));
		}

		CMainDocument* pDoc     = wxGetApp().GetDocument();
		
		if(m_rightIndex-1 >= 0){
			PROJECT* project = pDoc->state.projects.at(m_rightIndex-1);
			userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
			toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		    wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url,m_leftIndex+1);
		    i_statW->Move(wxPoint(29,3));
			// resolve the proj image 
			url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
			dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
			if(boinc_resolve_filename(dirProjectGraphic.c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
				g_statIcn = new wxImage(defaultIcnPath, wxBITMAP_TYPE_PNG);
				i_statW->LoadImage(g_statIcn);
			}else{
				i_statW->LoadImage(g_statIcnDefault);
			}

			i_statW->SetToolTip(statToolTip);
			
		    // push icon in the vector
		    m_statProjects.insert(m_statProjects.begin(),i_statW);
			//increment left index
			m_leftIndex --;
			//increment right index
			m_rightIndex --;
			//now show left button
			btnArwRight->Show(true);

		}
		//hide right arrow if we got to the end of the list
		if(m_rightIndex <= 0){
			btnArwLeft->Show(false);
		}
		btnArwLeft->Refresh();

	}else if(m_wxBtnObj==btnArwRight){
		//delete proj icon at position 1(0)
		delete m_statProjects.at(0);
		//shift the vector
		m_statProjects.assign(m_statProjects.begin()+1,m_statProjects.end());
		//shift icons left
		for(int m = 0; m < (int)m_statProjects.size(); m++){
			StatImageLoader *i_statWShifting = m_statProjects.at(m);
			wxPoint currPoint = i_statWShifting->GetPosition();
			i_statWShifting->Move(wxPoint(29 + 52*m,3));
		}
       
        CMainDocument* pDoc     = wxGetApp().GetDocument();
		//update project count
		m_projCnt = (int)pDoc->state.projects.size();
		if(m_leftIndex+1 <= m_projCnt){
			PROJECT* project = pDoc->state.projects.at(m_leftIndex);
			userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
			toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		    wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
			StatImageLoader *i_statW = new StatImageLoader(this,project->master_url,m_leftIndex+1);
		    i_statW->Move(wxPoint(29 + 52*(m_maxNumOfIcons-1),3));
			// resolve the proj image 
			url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
			dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
			if(boinc_resolve_filename(dirProjectGraphic.c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
				g_statIcn = new wxImage(defaultIcnPath, wxBITMAP_TYPE_PNG);
				i_statW->LoadImage(g_statIcn);
			}else{
				i_statW->LoadImage(g_statIcnDefault);
			}

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
		if(m_leftIndex >= m_projCnt){
			btnArwRight->Show(false);
		}
		btnArwRight->Refresh();
	}
}
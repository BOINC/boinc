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
#include "sg_SkinClass.h"
#include "sg_StatImageLoader.h" 
#include "sg_BoincSimpleGUI.h"
#include "sg_ProjectsComponent.h"
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
    m_projCnt = pDoc->state.projects.size();
	std::string projectIconName = "stat_icon";
	char filePath[256];
	// url of project directory
	char urlDirectory[256];
	std::string dirProjectGraphic;

	for(int j = 0; j < m_projCnt; j++){
		PROJECT* project = pDoc->state.projects[j];
		//user credit text
		userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
		toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		if(j < m_maxNumOfIcons){
			// Project button
			wxWindow *w_statW = new wxWindow(this,-1,wxPoint(29 + 52*j,3),wxSize(52,52));
			wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
			StatImageLoader *i_statW = new StatImageLoader(w_statW,project->master_url,j);
			// resolve the proj image 
			url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
			dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
			//load stat icon
			if(boinc_resolve_filename(dirProjectGraphic.c_str(), filePath, sizeof(filePath)) == 0){
				g_statIcn = new wxImage(filePath, wxBITMAP_TYPE_PNG);
				i_statW->LoadImage(g_statIcn);
			}else{
				i_statW->LoadImage(g_statIcnDefault);
			}

			i_statW->SetToolTip(statToolTip);
			
			// push icon in the vector
			m_statProjects.push_back(i_statW);
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

void CProjectsComponent::OnBtnClick(wxCommandEvent& event){ //init function
	wxObject *m_wxBtnObj = event.GetEventObject();

	if(m_wxBtnObj==btnArwLeft){
        btnArwLeft->Refresh();
	}else if(m_wxBtnObj==btnArwRight){
        delete m_statProjects.at(0);//delete proj icon at position 1(0)
        CMainDocument* pDoc     = wxGetApp().GetDocument();
        //PROJECT* project = pDoc->state.projects[m];
        // shift icons to the left
		for(int m = 0; m < m_statProjects.size(); m++){
			StatImageLoader *i_statWShifting = m_statProjects.at(m);
			i_statWShifting->Move(wxPoint(29 + 52*m,3));
		}
		/*
		//user credit text
		userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
		toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
		// Project button
		wxWindow *w_statW = new wxWindow(this,-1,wxPoint(29 + 52*j,3),wxSize(52,52));
		wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
		StatImageLoader *i_statW = new StatImageLoader(w_statW,project->master_url,j);
		// resolve the proj image 
		url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
		dirProjectGraphic = (std::string)urlDirectory + "/" + projectIconName;
		//load stat icon
		if(boinc_resolve_filename(dirProjectGraphic.c_str(), filePath, sizeof(filePath)) == 0){
			g_statIcn = new wxImage(filePath, wxBITMAP_TYPE_PNG);
			i_statW->LoadImage(g_statIcn);
		}else{
			i_statW->LoadImage(g_statIcnDefault);
		}

		i_statW->SetToolTip(statToolTip);
		
		// push icon in the vector
		m_statProjects.push_back(i_statW);
	*/
		btnArwRight->Refresh();
	}
}
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
#include "sg_StatImageLoader.h"
#include "sg_ProjectsComponent.h"
#include "sg_ViewTabPage.h"
#include "sg_DlgPreferences.h"
#include "sg_ProjectsComponent.h"

#include "wizardex.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "error_numbers.h"
#include "parse.h"
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
    CBOINCBaseFrame((wxFrame *)NULL, ID_SIMPLEFRAME, title, wxDefaultPosition, wxSize(416, 581),
                    wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    // Initialize Application
    SetIcon(*icon);
	
    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Read(wxT("Skin"), &skinName, wxT("default"));
    pConfig->Read(wxT("SkinFolderPath"), &skinFoldPath, wxT("skins"));

    //init app skin class
	appSkin = SkinClass::Instance();
    
    appSkin->SetSkinName(skinName);
	appSkin->SetSkinsFolder(skinFoldPath);
	skinPath = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/")+_T("skin.xml");
	midAppCollapsed = false;
	btmAppCollapsed = false;
	clientGUIInitialized = false;
	// load skin xml and parse it
	LoadSkinXML();
	// load images from skin file
	LoadSkinImages();
	//set polling timer for interface
	m_pFrameRenderTimer = new wxTimer(this, ID_SIMPLEFRAMERENDERTIMER);
    wxASSERT(m_pFrameRenderTimer);
    m_pFrameRenderTimer->Start(1000);                // Send event every 1 second
    //Create client
	InitEmptyState();
	//InitSimpleClient(); moved to timer function
	// center application
	//initAfter();
}

CSimpleFrame::~CSimpleFrame()
{
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(m_pFrameRenderTimer);

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("Skin"), skinName);
    pConfig->Write(wxT("SkinFolderPath"), skinFoldPath);

	if (m_pFrameRenderTimer) {
        m_pFrameRenderTimer->Stop();
        delete m_pFrameRenderTimer;
    }
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
    if (pAPWizard)
        pAPWizard->Destroy();

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
        //FireRefreshView();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToProject - Function End"));
}

void CSimpleFrame::OnFrameRender(wxTimerEvent &event) {

	CMainDocument* pDoc     = wxGetApp().GetDocument();
	int retValue;
	//Update data
	if(pDoc->IsConnected()){
       retValue = pDoc->CachedSimpleGUIUpdate();
	   if(retValue==0){
		   if(!clientGUIInitialized){
			   //Freeze();
               InitSimpleClient();
               initAfter();
			   // Thaw();
			   //Update();
			   clientGUIInitialized = true;
			   Show(true);
		   }else{ //check for changes in the interface
			   UpdateClientGUI();
		   }           
	   }
	}
}


void CSimpleFrame::InitEmptyState()
{
	Show(false);
	Centre();
}
void CSimpleFrame::InitSimpleClient()
{
	//Set Background color
	SetBackgroundColour(appSkin->GetAppBgCol());
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	// Flex Grid Sizer
	mainSizer = new wxFlexGridSizer(3,2);
	//mainSizer->SetFlexibleDirection(wxHORIZONTAL);
	SetSizer(mainSizer);
	// FlatNotebook
	wrkUnitNB = new wxFlatNotebook(this, -1, wxDefaultPosition, wxSize(370,330), wxFNB_TABS_BORDER_SIMPLE | wxFNB_NO_X_BUTTON | wxFNB_NO_NAV_BUTTONS | wxFNB_FANCY_TABS);
	wrkUnitNB->SetBackgroundColour(wxColour(255,255,255));
	wrkUnitNB->SetTabAreaColour(appSkin->GetAppBgCol());
	wrkUnitNB->SetGradientColors(appSkin->GetTabFromColAc(),appSkin->GetTabToColAc(),appSkin->GetTabBrdColAc());
	wrkUnitNB->SetActiveTabTextColour(wxColour(157,165,171));
	wrkUnitNB->SetGradientColorsInactive(appSkin->GetTabFromColIn(),appSkin->GetTabToColIn(),appSkin->GetTabBrdColIn());
	wrkUnitNB->SetNonActiveTabTextColour(wxColour(186,184,200));
	wrkUnitNB->SetImageList(&m_ImageList);
	//create work unit tabs
	int resultCnt = pDoc->results.results.size();
	
	for(int i = 0; i < resultCnt; i++){
		RESULT* result = pDoc->results.results[i];
		RESULT* resState = pDoc->state.lookup_result(result->project_url, result->name);
		wxString friendlyName;

		if(resState!=0){
			friendlyName = wxString(resState->app->name.c_str(), wxConvUTF8 );
		}else{
			friendlyName = wxString(resState->app->name.c_str(), wxConvUTF8 );
		}
		std::string index = " ";
		//index += i;
		friendlyName += wxString(index.c_str(), wxConvUTF8 );
		CViewTabPage *wTab = new CViewTabPage(wrkUnitNB,i,resState->name);
		wrkUnitNB->AddPage(wTab, friendlyName, true);	
		if(result->active_task_state == 1){
			 wrkUnitNB->SetPageImageIndex(i, 0); // this is working process
		}else{
			 wrkUnitNB->SetPageImageIndex(i, 1); // this is sleeping process
		}
	
		m_windows.push_back(wTab);
	}

	wrkUnitNB->SetSelection(0);	
	// Put Grid in the sizer
	mainSizer->Add(20, 70,0);
	mainSizer->Add(370, 70,0);
	mainSizer->Add(20, 70,0);
	mainSizer->Add(0, 0,1);
	mainSizer->Add(wrkUnitNB);
	mainSizer->Add(0, 0,1);
	 
	//Static content in my Projects section
	// My Projects
	stMyProj=new wxStaticText(this,-1,wxT(""),wxPoint(20,434),wxSize(84,18),wxST_NO_AUTORESIZE);
	stMyProj->SetLabel(wxT("My Projects:"));
	stMyProj->SetFont(wxFont(10,74,90,92,0,wxT("Tahoma")));
	// Add Project <><><>
	wxToolTip *ttAddProject = new wxToolTip(wxT("Add Project"));
	btnAddProj=new wxBitmapButton(this,-1,*btmpBtnAttProjL,wxPoint(293,431),wxSize(96,20));
	btnAddProj->SetToolTip(ttAddProject);
	/// Line
	lnMyProjTop=new wxStaticLine(this,-1,wxPoint(20,454),wxSize(370,2));
	/////////////// MY PROJECTS COMPONENT /////////////////////
    projComponent = new CProjectsComponent(this,wxPoint(20,455));
	/// Line
	lnMyProjBtm=new wxStaticLine(this,-1,wxPoint(20,516),wxSize(370,2));
	//// Messages Play Pause Btns
	wxToolTip *ttMessages = new wxToolTip(wxT("Messages"));
	btnMessages=new wxBitmapButton(this,-1,*btmpMessagesBtnL,wxPoint(20,522),wxSize(76,20));
	btnMessages->SetToolTip(ttMessages);
	// pause btn
	wxToolTip *ttPause = new wxToolTip(wxT("Pause all processing"));
	btnPause=new wxBitmapButton(this,-1,*btmpBtnPauseL,wxPoint(97,522),wxSize(59,20));
	btnPause->SetToolTip(ttPause);
    // play btn   
	wxToolTip *ttResume = new wxToolTip(wxT("Resume all Processing"));
	btnResume=new wxBitmapButton(this,-1,*btmpBtnResumeL,wxPoint(97,522),wxSize(62,20));
	btnResume->SetToolTip(ttResume);
	btnResume->Show(false);
	// Pref Btn
	wxToolTip *ttPreferences = new wxToolTip(wxT("Preferences"));
	btnPreferences=new wxBitmapButton(this,-1,*btmpBtnPrefL,wxPoint(183,522),wxSize(86,20));
	btnPreferences->SetToolTip(ttPreferences);
	// Advanced View
	wxToolTip *ttAdvView = new wxToolTip(wxT("Advanced View"));
	btnAdvancedView=new wxBitmapButton(this,-1,*btmpBtnAdvViewL,wxPoint(273,522),wxSize(116,20));
    btnAdvancedView->SetToolTip(ttAdvView);

	Refresh();
}
void CSimpleFrame::UpdateClientGUI(){
	
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	//update GUI
	int resultCnt = pDoc->results.results.size();
    wxString strBuffer = wxEmptyString;
    // Update Tabs
	for(int i = 0; i < resultCnt; i++){
		RESULT* result = pDoc->results.results[i];
		RESULT* resState = pDoc->state.lookup_result(result->project_url, result->name);
		
		// get tab window
		CViewTabPage *currTab = m_windows[i];
		if(result->name == currTab->GetTabName()){
			currTab->UpdateInterface();
		}else{
			//delete tab page
			//wrkUnitNB->RemovePage(i);
			//add replacement page
			//wxString friendlyName;
			//CViewTabPage *wTab = new CViewTabPage(wrkUnitNB,i,resState->name);
			//wrkUnitNB->AddPage(wTab,  wxT(friendlyName, true));	
			//(result->active_task_state == 1){
			//	wrkUnitNB->SetPageImageIndex(i, 0); // this is working process
			//}else{
			//	 wrkUnitNB->SetPageImageIndex(i, 1); // this is sleeping process
			//}
			//push new page
			//m_windows.push_back(wTab);
		}		
		
	}
	//Update Projects
	int projCnt = pDoc->state.projects.size();
	//std::vector<StatImageLoader*> tempProjects;
    unsigned int j;
	/*for(j = 0; j < pDoc->state.projects.size(); j++){
		PROJECT* project = pDoc->state.projects[j];
		
		//only go into if we have enough project icons
		if(j<m_statProjects.size()){
			// get tab window
			StatImageLoader *currProjIcon = m_statProjects[j];
			
			if(project->master_url == currProjIcon->m_prjUrl){ // update credit tooltip
				wxString toolTipTxt;
				wxString userCredit;
				userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
				toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
				wxToolTip *statToolTip = new wxToolTip(toolTipTxt);
				currProjIcon->SetToolTip(statToolTip);
			}else{
				//delete icon and make a new one
				// push icon in the vector
			    //tempProjects.push_back(i_statW);
			}
		}
	}*/
	//Refresh();
}

void CSimpleFrame::initAfter(){
    //add your code here
    //Centre();
    Show(true);
}
//
void CSimpleFrame::LoadSkinImages(){

	wxString dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	
    fileImgBuf[0].LoadFile(dirPref + appSkin->GetAppBg(),wxBITMAP_TYPE_BMP);
	// work unit icons
	g_icoSleepWU = new wxImage(dirPref + appSkin->GetIcnSleepingWkUnit(), wxBITMAP_TYPE_PNG);
	g_icoWorkWU = new wxImage(dirPref + appSkin->GetIcnWorkingWkUnit(), wxBITMAP_TYPE_PNG);
	//////////////////////////////
	fileImgBuf[2].LoadFile(dirPref + appSkin->GetBtnPrefer(),wxBITMAP_TYPE_BMP);
	fileImgBuf[3].LoadFile(dirPref + appSkin->GetBtnAddProj(),wxBITMAP_TYPE_BMP);
	fileImgBuf[4].LoadFile(dirPref + appSkin->GetIcnWorking(),wxBITMAP_TYPE_BMP);
	fileImgBuf[5].LoadFile(dirPref + appSkin->GetBtnMessages(),wxBITMAP_TYPE_BMP);
	fileImgBuf[6].LoadFile(dirPref + appSkin->GetBtnPause(),wxBITMAP_TYPE_BMP);
	fileImgBuf[7].LoadFile(dirPref + appSkin->GetBtnPlay(),wxBITMAP_TYPE_BMP);
	fileImgBuf[8].LoadFile(dirPref + appSkin->GetBtnAdvView(),wxBITMAP_TYPE_BMP);
	fileImgBuf[9].LoadFile(dirPref + appSkin->GetAnimationBG(),wxBITMAP_TYPE_BMP);
	fileImgBuf[10].LoadFile(dirPref + appSkin->GetIcnSleeping(),wxBITMAP_TYPE_BMP);
	CSimpleFrameImg0=&fileImgBuf[0];
	btmpBtnPrefL=&fileImgBuf[2];
	btmpBtnAttProjL=&fileImgBuf[3];
	btmpIcnWorking=&fileImgBuf[4];
	btmpBtnPauseL=&fileImgBuf[6];
	btmpBtnResumeL=&fileImgBuf[7];
	btmpMessagesBtnL=&fileImgBuf[5];
	btmpBtnAdvViewL=&fileImgBuf[8];
	btmpIcnSleeping=&fileImgBuf[10];
	/// work unit tabs icons
	wxBitmap const workWUico = wxBitmap(g_icoWorkWU); 
	wxBitmap const sleepWUico = wxBitmap(g_icoSleepWU); 
	// push them in image list
	m_ImageList.push_back(workWUico);
	m_ImageList.push_back(sleepWUico);
}
///
int CSimpleFrame::LoadSkinXML(){
   
	// parse xml file
	FILE* f;
    f = fopen(skinPath, "r");
	if (!f) return ERR_FOPEN;
    MIOFILE mf;
    mf.init_file(f);
    // parse
	char buf[256];
    std::string val;

    while (mf.fgets(buf, 256)) {
        if (match_tag(buf, "<clientskin")) {
            continue;
		}else if (match_tag(buf, "<simple")) {
            continue;
		}else if (match_tag(buf, "<background")) {
			mf.fgets(buf, 256);
			
			if (parse_str(buf, "<imgsrc>", val)) {
				appSkin->SetAppBg(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<bgcol>", val)) {
				appSkin->SetAppBgCol(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<dlgpreferences")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				appSkin->SetDlgPrefBg(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<gauge")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<fgcol>", val)) {
				appSkin->SetGaugeFgCol(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<bgcol>", val)) {
				appSkin->SetGaugeBgCol(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<buttons")) {
			while (mf.fgets(buf, 256)) {
				std::string val;
				if(match_tag(buf, "</buttons>")){
					//end of the buttons elements break out of while loop
					break;
				}
				if(match_tag(buf, "<preferences>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnPrefer(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<addproj>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnAddProj(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<advancedview>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnAdvView(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<play>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnPlay(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<pause>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnPause(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<messages>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnMessages(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<open>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnOpen(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<save>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnSave(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<cancel>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnCancel(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<leftArr>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnLeftArr(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnLeftArrClick(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<rightArr>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnRightArr(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnRightArrClick(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<expand>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnExpand(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnExpandClick(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<collapse>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnCollapse(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnCollapseClick(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<showgraphics>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnShowGraphic(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnShowGraphicClick(wxString( val.c_str(), wxConvUTF8 ));
					}
				}
			}//end of while
		}else if (match_tag(buf, "<icons")) {
			while (mf.fgets(buf, 256)) {
				std::string val;
				if(match_tag(buf, "</icons>")){
					//end of the buttons elements break out of while loop
					break;
				}
				if(match_tag(buf, "<working>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetIcnWorking(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<sleeping>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetIcnSleeping(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<workingWkUnit>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetIcnWorkingWkUnit(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<frcol>", val)) {
						appSkin->SetTabFromColAc(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<tocol>", val)) {
						appSkin->SetTabToColAc(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<brdcol>", val)) {
						appSkin->SetTabBrdColAc(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<sleepingWkUnit>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetIcnSleepingWkUnit(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<frcol>", val)) {
						appSkin->SetTabFromColIn(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<tocol>", val)) {
						appSkin->SetTabToColIn(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<brdcol>", val)) {
						appSkin->SetTabBrdColIn(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<defaultProjIcon>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetDefaultPrjIcn(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<defaultStatIcon>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetDefaultStatIcn(wxString( val.c_str(), wxConvUTF8 ));
					}
				}
			}// end of while loop
		}else if (match_tag(buf, "<animation")) {
			mf.fgets(buf, 256);
			std::string val;
			if (parse_str(buf, "<background>", val)) {
				appSkin->SetAnimationBg(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<animation>", val)) {
				appSkin->SetAnimationFile(wxString( val.c_str(), wxConvUTF8 ));
			}
        }
	}
	//
    fclose(f);
	return 0;
}
///
void CSimpleFrame::ReskinAppGUI(){
	LoadSkinXML();
	LoadSkinImages();
	// reskin GUI
	//bg color
	SetBackgroundColour(appSkin->GetAppBgCol());
    // notebook tab color
	wrkUnitNB->SetTabAreaColour(appSkin->GetAppBgCol());
    wrkUnitNB->SetGradientColors(appSkin->GetTabFromColAc(),appSkin->GetTabToColAc(),appSkin->GetTabBrdColAc());
    wrkUnitNB->SetGradientColorsInactive(appSkin->GetTabFromColIn(),appSkin->GetTabToColIn(),appSkin->GetTabBrdColIn());
	// btns
	btnMessages->SetBitmapLabel(*btmpMessagesBtnL);
    btnResume->SetBitmapLabel(*btmpBtnResumeL);
	btnPause->SetBitmapLabel(*btmpBtnPauseL);
    btnAddProj->SetBitmapLabel(*btmpBtnAttProjL);
	btnPreferences->SetBitmapLabel(*btmpBtnPrefL);
	btnAdvancedView->SetBitmapLabel(*btmpBtnAdvViewL);

	//gauges
//	gaugeWUMain->SetForegroundColour(appSkin->GetGaugeFgCol());
// gaugeWUMain->SetBackgroundColour(appSkin->GetGaugeBgCol());
//    btnCollapse->SetBackgroundColour(appSkin->GetAppBgCol());
//	btnArwLeft->SetBackgroundColour(appSkin->GetAppBgCol());
  //  btnArwRight->SetBackgroundColour(appSkin->GetAppBgCol());

	Refresh();
}

void CSimpleFrame::OnBtnClick(wxCommandEvent& event){ //init function
	wxObject *m_wxBtnObj = event.GetEventObject();
	if(m_wxBtnObj==btnPreferences){
		CDlgPreferences* pDlg = new CDlgPreferences(NULL,appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/"));
		wxASSERT(pDlg);
        pDlg->ShowModal();
		if (pDlg) {
		   appSkin->SetSkinName(pDlg->GetSkinName());
		   skinPath = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/")+_T("skin.xml");
		   if(skinPath.Length() > 0){
			   ReskinAppGUI();
		   }
           pDlg->Destroy();
		}
    }else if(m_wxBtnObj==btnAdvancedView) {
        wxGetApp().SetActiveGUI(BOINC_ADVANCEDGUI, true);
    }else if(m_wxBtnObj==btnMessages) {
		btnMessages->Refresh();
	}else if(m_wxBtnObj==btnAddProj) {
		OnProjectsAttachToProject();
		btnAddProj->Refresh();
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
    }else{
		//wxMessageBox("OnBtnClick - else");
	}
}
//end function
void CSimpleFrame::OnPageChanged(wxFlatNotebookEvent& event)
{
//	btnCollapse->Refresh();
}
void CSimpleFrame::OnEraseBackground(wxEraseEvent& event){
  wxObject *m_wxWin = event.GetEventObject();
  if(m_wxWin==this){event.Skip(true);DrawBackImg(event,this,*CSimpleFrameImg0,0);return;}
  event.Skip(true);
}
void CSimpleFrame::DrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz){
	if(midAppCollapsed){
        wrkUnitNB->SetSize(-1, -1, wxNotebookSize.x, wxNotebookSize.y); // fix
	}

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


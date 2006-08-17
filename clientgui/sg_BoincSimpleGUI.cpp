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
#include "sg_StatImageLoader.h"
#include "sg_ViewTabPage.h"


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
    CBOINCBaseFrame((wxFrame *)NULL, ID_SIMPLEFRAME, title, wxDefaultPosition, wxSize(416, 600),
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
	clientGUIInitialized = false;
	wrkUnitNotebookInit = false;
	//Check if skin can be loaded
	if(!CheckSkin()){
		//if current skin is not loaded then switch to default skin
		//that is in memory
		appSkin->SetSkinName( wxT("default"));
		skinPath = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/")+_T("skin.xml");
	    wxMessageBox("Incompatible skin. Switching to default");
		LoadSkinXML();
	}
	// load images from skin file
	LoadSkinImages();
	//set polling timer for interface
	m_pFrameRenderTimer = new wxTimer(this, ID_SIMPLEFRAMERENDERTIMER);
    wxASSERT(m_pFrameRenderTimer);
    m_pFrameRenderTimer->Start(1000);                // Send event every 1 second
    //Create client
	InitEmptyState();
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
		//update Project Component
        projComponent->UpdateInterface();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToProject - Function End"));
}

void CSimpleFrame::OnFrameRender(wxTimerEvent& WXUNUSED(event)) {
	int retValue;
	CMainDocument* pDoc     = wxGetApp().GetDocument();

    //Update data
    if(pDoc->IsConnected()){
        // Update the document state, any subsequent calls will just used the
        //   cached data.
        pDoc->GetSimpleGUIWorkCount();
        if(!clientGUIInitialized){
            InitSimpleClient();
            initAfter();
            clientGUIInitialized = true;
        }
        UpdateClientGUI();
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
	mainSizer->Add(31, 98,0);
	mainSizer->Add(343, 98,0);
	mainSizer->Add(31, 98,0);
	mainSizer->Add(0, 0,1);
	// Do not update screen at this point
	Freeze();
	//create work unit tabs
    int resultCnt = (int)pDoc->GetSimpleGUIWorkCount();
	if(resultCnt > 0){
		//init nootebok
		InitNotebook();
	
		for(int i = 0; i < resultCnt; i++){
			RESULT* result = pDoc->results.results[i];
			RESULT* resState = pDoc->state.lookup_result(result->project_url, result->name);
			wxString friendlyName;

			if(resState!=0){
				friendlyName = wxString(resState->app->name.c_str(), wxConvUTF8 );
			}else{
				friendlyName = wxString(resState->app->name.c_str(), wxConvUTF8 );
			}
			CViewTabPage *wTab = new CViewTabPage(wrkUnitNB,i,resState->name,resState->project_url);
			wrkUnitNB->AddPage(wTab, friendlyName, true);	
			if(result->active_task_state == 1){
				wrkUnitNB->SetPageImageIndex(i, 0); // this is working process
			}
			//add page to vector
			m_windows.push_back(wTab);
		}
		wrkUnitNB->SetSelection(0);	
		// Put Grid in the sizer
		mainSizer->Add(wrkUnitNB);
	}
	//mainSizer->Add(0, 0,1);
	 
	/////////////// MY PROJECTS COMPONENT /////////////////////
    projComponent = new CProjectsComponent(this,wxPoint(31,443));
	///////////////////////////////////////////////////////////
	//Update screen
	Thaw();
	mainSizer->Layout();
	//Update();
	//Refresh();
}
void CSimpleFrame::UpdateClientGUI(){
	
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	//update GUI
    int resultCnt = (int)pDoc->GetSimpleGUIWorkCount();
    wxString strBuffer = wxEmptyString;
	//assume they are all inactive
	for(int x = 0; x < (int)m_windows.size(); x ++)
	{
		CViewTabPage *currTab = m_windows[x];
		currTab->isAlive = false;
	}
    // Update Tabs
	RESULT* result;
	for(int i = 0; i < resultCnt; i++){
		result = pDoc->results.results[i];
		// get tab window
		bool found = false;
		for(int j = 0; j < (int)m_windows.size(); j ++)
		{
			CViewTabPage *currTab = m_windows[j];
			std::string curtabname = currTab->GetTabName();
            std::string resultname = result->name;

			if(result->name == currTab->GetTabName()){
				//currTab FOUND;
				currTab->isAlive = true;
				found = true;
				//update tab interface
		        currTab->UpdateInterface();
				break;
		    }

		}
		if(!found){
			bool addNotebookToSizer = false;
			//stop timer untill we add the page
			m_pFrameRenderTimer->Stop();
			// Do not update screen at this point
	        Freeze();
			//Check if notebook was initialized
			if(!wrkUnitNotebookInit){
				//init nootebok
				InitNotebook();
                addNotebookToSizer = true; // since this is first page
			}
			// create one and add it to notebook
			std::string projUrl = result->project_url;
			std::string nme = result->name;
			RESULT* resState = pDoc->state.lookup_result(projUrl, nme);
			wxString friendlyName;

			if(resState!=0){
				friendlyName = wxString(resState->app->name.c_str(), wxConvUTF8 );
			}else{
				friendlyName = wxString(resState->app->name.c_str(), wxConvUTF8 );
			}
			std::string index = " ";
			//index += i;
			friendlyName += wxString(index.c_str(), wxConvUTF8 );
			CViewTabPage *wTab = new CViewTabPage(wrkUnitNB,i,resState->name,resState->project_url);
			wrkUnitNB->AddPage(wTab, friendlyName, true);	
			if(result->active_task_state == 1){
				wrkUnitNB->SetPageImageIndex(i, 0); // this is working process
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
	//Refresh();
}

void CSimpleFrame::initAfter(){
    //add your code here
    Show(true);
}
//
void CSimpleFrame::InitNotebook()
{
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
	wrkUnitNotebookInit = true;
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
	// init skin image array
	skinImageArray = new wxArrayString();

    while (mf.fgets(buf, 256)) {
        if (match_tag(buf, "<clientskin")) {
            continue;
		}else if (match_tag(buf, "<simple")) {
            continue;
		}else if (match_tag(buf, "<background")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				appSkin->SetAppBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<bgcol>", val)) {
				appSkin->SetAppBgCol(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<prjcomponentbg")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				appSkin->SetProjCompBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<tabareabg")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				appSkin->SetTabAreaBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<spacerimage")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				appSkin->SetSpacerImage(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<workunitbg")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				appSkin->SetWorkunitBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<dlgpreferences")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				appSkin->SetDlgPrefBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<dlgmessages")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				appSkin->SetDlgMessBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<staticline")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<col>", val)) {
				appSkin->SetStaticLineCol(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<gauge")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<gaugebg>", val)) {
				appSkin->SetGaugeBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<gaugeprogress>", val)) {
				appSkin->SetGaugeProgressInd(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
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
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<addproj>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnAddProj(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnAddProjClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<advancedview>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnAdvView(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<resume>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnResume(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<pause>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnPause(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<messages>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnMessages(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<save>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnSave(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnSaveClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<cancel>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnCancel(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnCancelClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<close>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnClose(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnCloseClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<clear>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnClear(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnClearClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<leftArr>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnLeftArr(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnLeftArrClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<rightArr>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnRightArr(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						appSkin->SetBtnRightArrClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}
			}//end of while
		}else if (match_tag(buf, "<icons")) {
			while (mf.fgets(buf, 256)) {
				std::string val;
				if(match_tag(buf, "</icons>")){
					//end of the buttons elements break out of while loop
					break;
				}else if(match_tag(buf, "<workingWkUnit>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetIcnWorkingWkUnit(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
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
				}else if(match_tag(buf, "<defaultStatIcon>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetDefaultStatIcn(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}
			}// end of while loop
		}else if (match_tag(buf, "<animation")) {
			mf.fgets(buf, 256);
			std::string val;
			if (parse_str(buf, "<background>", val)) {
				appSkin->SetAnimationBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
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
bool CSimpleFrame::CheckSkin()
{
	//load skin xml file first
	if(!LoadSkinXML()==0){
		return false;//skin xml file is not available
	}

	wxString dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	
	for(int x = 0; x < skinImageArray->Count();x++){
		wxString imgLoc = skinImageArray->Item(x);
		wxBitmap skinImage = wxBitmap(dirPref + skinImageArray->Item(x),wxBITMAP_TYPE_PNG);
		if(!skinImage.Ok()){
			return false;
		}
	}
     
	return true;
}
void CSimpleFrame::ReskinAppGUI(){
	//LoadSkinXML();
	LoadSkinImages();
	// reskin GUI
	//bg color
	SetBackgroundColour(appSkin->GetAppBgCol());
	if(wrkUnitNotebookInit){
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
	}
	//reskin component 
	projComponent->ReskinInterface();
	Refresh();
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


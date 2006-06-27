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
#include "BOINCBaseFrame.h"
#include "Events.h"
#include "common/wxAnimate.h"
#include "common/wxFlatNotebook.h"
#include "sg_ImageLoader.h"
#include "sg_StatImageLoader.h"
#include "sg_DlgPreferences.h"
#include "sg_SkinClass.h"
#include "sg_BoincSimpleGUI.h"
#include "error_numbers.h"
#include "parse.h"
#include <string>
#include "wizardex.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"

#include "res/boinc.xpm"

IMPLEMENT_DYNAMIC_CLASS(CSimpleFrame, CBOINCBaseFrame)

BEGIN_EVENT_TABLE(CSimpleFrame, CBOINCBaseFrame)
    EVT_BUTTON(-1,CSimpleFrame::OnBtnClick)
    EVT_SIZE(CSimpleFrame::OnSize)
	EVT_ERASE_BACKGROUND(CSimpleFrame::OnEraseBackground)
    EVT_FRAME_CONNECT(CSimpleFrame::OnConnect)
END_EVENT_TABLE()


CSimpleFrame::CSimpleFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function End"));
}


CSimpleFrame::CSimpleFrame(wxString title, wxIcon* icon) : 
    CBOINCBaseFrame((wxFrame *)NULL, ID_SIMPLEFRAME, title, wxDefaultPosition, wxSize(416, 581),
                    wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    // Initialize Application
    SetIcon(*icon);

    skinName = _T("default");
	skinsFolder = _T("skins");
    skinPath = skinsFolder+_T("/")+skinName+_T("/")+_T("skin.xml");
	midAppCollapsed = false;
	btmAppCollapsed = false;
	// load skin xml and parse it
	LoadSkinXML();
	// load images from skin file
	LoadSkinImages();
    //Create client
	InitSimpleClient();
	// center application
	initAfter();
}

CSimpleFrame::~CSimpleFrame()
{
//    m_player.Stop();
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


wxPoint& CSimpleFrame::SetwxPoint(long x,long y){
  m_tmppoint.x=x;
  m_tmppoint.y=y;
  return m_tmppoint;
}

wxSize& CSimpleFrame::SetwxSize(long w,long h){
  m_tmpsize.SetWidth(w);
  m_tmpsize.SetHeight(h);
  return m_tmpsize;
}
wxWindow* CSimpleFrame::CreateNotebookPage()
{
	static int newPageCounter = 0;
	wxString caption;
	caption.Printf(_("Work Unit"));
	return new wxWindow(this,-1,wxDefaultPosition,SetwxSize(370,330),wxNO_BORDER);
}
void CSimpleFrame::InitSimpleClient()
{
 Show(false);
 SetBackgroundColour(appSkin->GetAppBgCol());
 // Flex Grid Sizer
 mainSizer = new wxFlexGridSizer(3,2);
 //mainSizer->SetFlexibleDirection(wxHORIZONTAL);
 SetSizer(mainSizer);
 // FlatNotebook
 wrkUnitNB = new wxFlatNotebook(this, -1, wxDefaultPosition, SetwxSize(370,330), wxFNB_TABS_BORDER_SIMPLE | wxFNB_NO_X_BUTTON | wxFNB_NO_NAV_BUTTONS | wxFNB_FANCY_TABS);
 wrkUnitNB->SetBackgroundColour(wxColour(255,255,255));
 wrkUnitNB->SetTabAreaColour(appSkin->GetAppBgCol());
 wrkUnitNB->SetGradientColors(appSkin->GetTabFromColAc(),appSkin->GetTabToColAc(),appSkin->GetTabBrdColAc());
 wrkUnitNB->SetActiveTabTextColour(wxColour(157,165,171));
 wrkUnitNB->SetGradientColorsInactive(appSkin->GetTabFromColIn(),appSkin->GetTabToColIn(),appSkin->GetTabBrdColIn());
 wrkUnitNB->SetNonActiveTabTextColour(wxColour(186,184,200));
 wrkUnitNB->SetImageList(&m_ImageList);
 // Create Page
 wTab1 = this->CreateNotebookPage();
 wTab2 = this->CreateNotebookPage();
 wrkUnitNB->AddPage(wTab1,  wxT("WCG 1"), true);	
 wrkUnitNB->AddPage(wTab2,  wxT("Prd@H 1"), true);
 wrkUnitNB->SetSelection(0);	
 wrkUnitNB->SetPageImageIndex(0, 0); // this is working process
 wrkUnitNB->SetPageImageIndex(1, 1); // this is sleeping process
 // Put Grid in the sizer
 mainSizer->Add(20, 70,0);
 mainSizer->Add(362, 70,0);
 mainSizer->Add(20, 70,0);
 mainSizer->Add(0, 0,1);
 mainSizer->Add(wrkUnitNB);
 // Build Page /////////////////////////////
 w_iconPT1=new wxWindow(wTab1,-1,SetwxPoint(2,2),SetwxSize(22,22));
 i_prjIcnPT1 = new ImageLoader(w_iconPT1);
 i_prjIcnPT1->LoadImage(g_prjIcnWCG);
 ////
 st12c=new wxStaticText(wTab1,-1,wxT(""),SetwxPoint(25,2),SetwxSize(189,18),wxST_NO_AUTORESIZE);
   st12c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
   st12c->SetLabel(wxT("World Community Grid"));
   st12c->SetFont(wxFont(12,74,90,90,0,wxT("Tahoma")));
 lno14c=new wxStaticLine(wTab1,-1,SetwxPoint(9,25),SetwxSize(353,2));
 st15c=new wxStaticText(wTab1,-1,wxT(""),SetwxPoint(15,32),SetwxSize(89,18),wxST_NO_AUTORESIZE);
   st15c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
   st15c->SetLabel(wxT("My Progress:"));
   st15c->SetFont(wxFont(10,74,90,92,0,wxT("Tahoma")));
 st16c=new wxStaticText(wTab1,-1,wxT(""),SetwxPoint(110,34),SetwxSize(79,13),wxST_NO_AUTORESIZE);
   st16c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
   st16c->SetLabel(wxT("Work Unit Name"));
 /////// Main Gauge
 gaugeWuP1=new wxGauge(wTab1,-1,100,SetwxPoint(15,60),SetwxSize(340,30),wxGA_SMOOTH);
 gaugeWuP1->SetForegroundColour(appSkin->GetGaugeFgCol());
 gaugeWuP1->SetBackgroundColour(appSkin->GetGaugeBgCol());
 gaugeWuP1->SetValue(48);
 //
 st18c=new wxStaticText(wTab1,-1,wxT(""),SetwxPoint(15,97),SetwxSize(84,18),wxST_NO_AUTORESIZE);
   st18c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
   st18c->SetLabel(wxT("Elapsed Time:"));
   st18c->SetFont(wxFont(10,74,90,90,0,wxT("Tahoma")));
 st19c=new wxStaticText(wTab1,-1,wxT(""),SetwxPoint(102,97),SetwxSize(364,18),wxST_NO_AUTORESIZE);
   st19c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
   st19c->SetLabel(wxT("16 hours 12 minutes 43 seconds"));
   st19c->SetFont(wxFont(10,74,90,90,0,wxT("Tahoma")));
 st20c=new wxStaticText(wTab1,-1,wxT(""),SetwxPoint(15,119),SetwxSize(154,18),wxST_NO_AUTORESIZE);
   st20c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
   st20c->SetLabel(wxT("Time remaining:"));
   st20c->SetFont(wxFont(10,74,90,90,0,wxT("Tahoma")));
 st21c=new wxStaticText(wTab1,-1,wxT(""),SetwxPoint(115,119),SetwxSize(294,18),wxST_NO_AUTORESIZE);
   st21c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
   st21c->SetLabel(wxT("12 days 13 hours 23 minutes 11 seconds"));
   st21c->SetFont(wxFont(10,74,90,90,0,wxT("Tahoma")));

 // project image behind graphic <><><>
 imgBgAnim=new wxStaticBitmap(wTab1,-1,*btmpBgAnim,SetwxPoint(0,146),SetwxSize(370,182));
 //// Animation Window
 wAnimWk1=new wxWindow(wTab1,-1,SetwxPoint(85,146),SetwxSize(184,182),wxNO_BORDER);
 // media control
 /////////////
 m_canvas = new MyCanvas(wAnimWk1, SetwxPoint(0,0), SetwxSize(184,182));
 #if 0
    m_player.SetDestroyAnimation(false);
    m_player.SetWindow(m_canvas);
    m_player.SetPosition(SetwxPoint(0, 0));
#endif
    m_animationCtrl = new wxGIFAnimationCtrl(m_canvas, -1, wxEmptyString,
        SetwxPoint(0, 0), wxSize(184, 184));

	m_animationCtrl->Stop();
    if (m_animationCtrl->LoadFile(wxT("skins/default/graphic/molecule.gif")))
    {
       m_animationCtrl->Play();
    }
    else
    {
	   wxMessageBox(_T("Sorry, this animation was not a valid animated GIF."));
    }
 /////////////
 //////////////////////////////////////

 // My Projects
 stMyProj=new wxStaticText(this,-1,wxT(""),SetwxPoint(20,434),SetwxSize(84,18),wxST_NO_AUTORESIZE);
   stMyProj->SetLabel(wxT("My Projects:"));
   stMyProj->SetFont(wxFont(10,74,90,92,0,wxT("Tahoma")));
 // Attach Project <><><>
 btnAttProj=new wxBitmapButton(this,-1,*btmpBtnAttProjL,SetwxPoint(250,431),SetwxSize(109,20));
 // Collapse button mid
 btnCollapseMid=new wxBitmapButton(this,-1,btmpCol,SetwxPoint(366,429),SetwxSize(24,24),wxSIMPLE_BORDER);
 btnCollapseMid->SetBitmapSelected(btmpColClick);
 //expand buttons
 btnExpandMid=new wxBitmapButton(this,-1,btmpExp,SetwxPoint(336,429),SetwxSize(24,24),wxSIMPLE_BORDER);
 btnExpandMid->SetBitmapSelected(btmpExpClick);
 btnExpandMid->Show(false); // at initial build there is no need to show
  /// Line
 lnMyProjTop=new wxStaticLine(this,-1,SetwxPoint(20,454),SetwxSize(370,2));
 ///////
 w_statWCG = new wxWindow(this,-1,SetwxPoint(60,460),SetwxSize(52,52));
 wxToolTip *statWCGtip = new wxToolTip(_T("World Community Grid. UserIBM has 123432 points."));
 i_statWCG = new StatImageLoader(w_statWCG);
 i_statWCG->LoadImage(g_statWCG);
 i_statWCG->SetToolTip(statWCGtip);
 //
 w_statSeti = new wxWindow(this,-1,SetwxPoint(112,460),SetwxSize(52,52));
 wxToolTip *statSetitip = new wxToolTip(_T("Seti@Home. UserSETI has 123432 points."));
 i_statSeti = new StatImageLoader(w_statSeti);
 i_statSeti->LoadImage(g_statSeti);
 i_statSeti->SetToolTip(statSetitip);
 //
 w_statPred = new wxWindow(this,-1,SetwxPoint(164,460),SetwxSize(52,52));
 wxToolTip *statPredtip = new wxToolTip(_T("Predictor@Home. UserPRED1 has 123432 points."));
 i_statPred = new StatImageLoader(w_statPred);
 i_statPred->LoadImage(g_statPred);
 i_statPred->SetToolTip(statPredtip);
 //
 //// Arrow Btns
 btnArwLeft=new wxBitmapButton(this,-1,btmpArwL,SetwxPoint(25,473),SetwxSize(24,24),wxSIMPLE_BORDER);
 btnArwLeft->SetBitmapSelected(btmpArwLC);
 btnArwRight=new wxBitmapButton(this,-1,btmpArwR,SetwxPoint(360,473),SetwxSize(24,24),wxNO_BORDER);
 btnArwRight->SetBitmapSelected(btmpArwRC);
 ///////////
 lnMyProjBtm=new wxStaticLine(this,-1,SetwxPoint(20,516),SetwxSize(370,2));
 //// Messages Play Pause Btns
 btnMessages=new wxBitmapButton(this,-1,*btmpMessagesBtnL,SetwxPoint(28,522),SetwxSize(20,20));
 // play pause btn
 btnPause=new wxBitmapButton(this,-1,*btmpBtnPauseL,SetwxPoint(55,522),SetwxSize(20,20));
 btnPlay=new wxBitmapButton(this,-1,*btmpBtnPlayL,SetwxPoint(55,522),SetwxSize(20,20));
 btnPlay->Show(false);
 // Class View ,Pref Btns
 btnPreferences=new wxBitmapButton(this,-1,*btmpBtnPrefL,SetwxPoint(183,522),SetwxSize(86,20));
 btnAdvancedView=new wxBitmapButton(this,-1,*btmpBtnAdvViewL,SetwxPoint(273,522),SetwxSize(116,20));

 Refresh();
}
void CSimpleFrame::initAfter(){
 //add your code here
    Centre();
    Show(true);
}
//
void CSimpleFrame::LoadSkinImages(){

	wxString dirPref = skinsFolder+_T("/")+skinName+_T("/");
	
    fileImgBuf[0].LoadFile(dirPref + appSkin->GetAppBg(),wxBITMAP_TYPE_BMP);
	// prj icons will be removed
	g_prjIcnWCG = new wxImage(dirPref + appSkin->GetIcnPrjWCG(), wxBITMAP_TYPE_PNG);
	g_prjIcnPDRC = new wxImage(dirPref + appSkin->GetIcnPrjPRED(), wxBITMAP_TYPE_PNG);
	// work unit icons
	g_icoSleepWU = new wxImage(dirPref + appSkin->GetIcnSleepingWkUnit(), wxBITMAP_TYPE_PNG);
	g_icoWorkWU = new wxImage(dirPref + appSkin->GetIcnWorkingWkUnit(), wxBITMAP_TYPE_PNG);
	// stat icons
	g_statWCG = new wxImage(_T("skins/default/graphic/statWCG.png"), wxBITMAP_TYPE_PNG);
	g_statSeti = new wxImage(_T("skins/default/graphic/statSeti.png"), wxBITMAP_TYPE_PNG);
	g_statPred = new wxImage(_T("skins/default/graphic/statPred.png"), wxBITMAP_TYPE_PNG);
	// arrows
	g_arwLeft = new wxImage(dirPref + appSkin->GetBtnLeftArr(), wxBITMAP_TYPE_PNG);
	g_arwRight = new wxImage(dirPref + appSkin->GetBtnRightArr(), wxBITMAP_TYPE_PNG);
	g_arwLeftClick = new wxImage(dirPref + appSkin->GetBtnLeftArrClick(), wxBITMAP_TYPE_PNG);
	g_arwRightClick = new wxImage(dirPref + appSkin->GetBtnRightArrClick(), wxBITMAP_TYPE_PNG);
	btmpArwL= wxBitmap(g_arwLeft); 
    btmpArwR= wxBitmap(g_arwRight); 
    btmpArwLC= wxBitmap(g_arwLeftClick); 
    btmpArwRC= wxBitmap(g_arwRightClick); 
	// collapse
    g_collapse = new wxImage(dirPref + appSkin->GetBtnCollapse(), wxBITMAP_TYPE_PNG);
	g_collapseClick = new wxImage(dirPref + appSkin->GetBtnCollapseClick(), wxBITMAP_TYPE_PNG);
	btmpCol= wxBitmap(g_collapse); 
    btmpColClick= wxBitmap(g_collapseClick); 
	// expand
    g_expand = new wxImage(dirPref + appSkin->GetBtnExpand(), wxBITMAP_TYPE_PNG);
	g_expandClick = new wxImage(dirPref + appSkin->GetBtnExpandClick(), wxBITMAP_TYPE_PNG);
	btmpExp= wxBitmap(g_expand); 
    btmpExpClick= wxBitmap(g_expandClick); 
	//////////////////////////////
	fileImgBuf[2].LoadFile(dirPref + appSkin->GetBtnPrefer(),wxBITMAP_TYPE_BMP);
	fileImgBuf[3].LoadFile(dirPref + appSkin->GetBtnAttProj(),wxBITMAP_TYPE_BMP);
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
	btmpBtnPlayL=&fileImgBuf[7];
	btmpMessagesBtnL=&fileImgBuf[5];
	btmpBtnAdvViewL=&fileImgBuf[8];
	btmpBgAnim=&fileImgBuf[9];
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
    //app skin class
	appSkin = SkinClass::Instance();
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
				}else if(match_tag(buf, "<attchproj>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetBtnAttProj(wxString( val.c_str(), wxConvUTF8 ));
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
				}else if(match_tag(buf, "<prjWCGicon>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetIcnPrjWCG(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<prjPREDicon>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						appSkin->SetIcnPrjPRED(wxString( val.c_str(), wxConvUTF8 ));
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
    btnPlay->SetBitmapLabel(*btmpBtnPlayL);
	btnPause->SetBitmapLabel(*btmpBtnPauseL);
    btnAttProj->SetBitmapLabel(*btmpBtnAttProjL);
	btnPreferences->SetBitmapLabel(*btmpBtnPrefL);
	btnAdvancedView->SetBitmapLabel(*btmpBtnAdvViewL);
	//arrows
	btnArwLeft->SetBitmapLabel(btmpArwL);
    btnArwLeft->SetBitmapSelected(btmpArwLC);
    btnArwRight->SetBitmapLabel(btmpArwR);
    btnArwRight->SetBitmapSelected(btmpArwRC);
	//collapse
	btnCollapseMid->SetBitmapLabel(btmpCol);
    btnCollapseMid->SetBitmapSelected(btmpColClick);
    //expand buttons
    btnExpandMid->SetBitmapLabel(btmpExp);
    btnExpandMid->SetBitmapSelected(btmpExpClick);
	//gauges
	gaugeWuP1->SetForegroundColour(appSkin->GetGaugeFgCol());
    gaugeWuP1->SetBackgroundColour(appSkin->GetGaugeBgCol());
	btnExpandMid->SetBackgroundColour(appSkin->GetAppBgCol());
    btnCollapseMid->SetBackgroundColour(appSkin->GetAppBgCol());
	btnArwLeft->SetBackgroundColour(appSkin->GetAppBgCol());
    btnArwRight->SetBackgroundColour(appSkin->GetAppBgCol());

	Refresh();
}

void CSimpleFrame::OnBtnClick(wxCommandEvent& event){ //init function
	wxObject *m_wxBtnObj = event.GetEventObject();
	if(m_wxBtnObj==btnPreferences){
		CDlgPreferences* pDlg = new CDlgPreferences(NULL,skinsFolder+_T("/")+skinName+_T("/"));
		wxASSERT(pDlg);
        pDlg->ShowModal();
		if (pDlg) {
		   skinName = pDlg->GetSkinName();
		   skinPath = skinsFolder+_T("/")+skinName+_T("/")+_T("skin.xml");
		   if(skinPath.Length() > 0){
			   ReskinAppGUI();
		   }
           pDlg->Destroy();
		}
	}else if(m_wxBtnObj==btnArwLeft){
		//refresh btn
		btnArwLeft->Refresh();
	}else if(m_wxBtnObj==btnArwRight){
		//refresh btn
		btnArwRight->Refresh();
	}else if(m_wxBtnObj==btnCollapseMid){
		//refresh btn
		wxNotebookSize = SetwxSize(370, 170); //fix
		if((!midAppCollapsed) && (!btmAppCollapsed)){
            m_canvas->Show(false);
			imgBgAnim->Show(false);
			btnCollapseMid->Show(false);
			btnExpandMid->Show(true);
			this->SetSize(-1, -1, 416, 398);
			wTab1->SetSize(-1, -1, 370, 170);
			wTab2->SetSize(-1, -1, 370, 170);
			wrkUnitNB->SetSize(-1, -1, 370, 170);
			//move controls up
            MoveControlsUp();
			midAppCollapsed = true;
		}else{
			this->SetSize(-1, -1, 416, 305);
			wTab1->SetSize(-1, -1, 370, 170);
			wTab2->SetSize(-1, -1, 370, 170);
			wrkUnitNB->SetSize(-1, -1, 370, 170);
			btnExpandMid->Move(366,247);
			midAppCollapsed = true;
		}
		Refresh();
	}else if(m_wxBtnObj==btnExpandMid){
		if((btmAppCollapsed) && (midAppCollapsed)){ // in this case open up bottom first
			this->SetSize(-1, -1, 416, 398);
			wrkUnitNB->SetSize(-1, -1, 370, 170);
			stMyProj->Show(true);
			btnAttProj->Show(true);
			stMyProj->Move(20,252);//(20,434)
			//move controls up
            MoveControlsUp();
			btmAppCollapsed = false;
		}else if(midAppCollapsed){ //open up mid section
			wxNotebookSize = SetwxSize(370, 330); //fix
            this->SetSize(-1, -1, 416, 581);
			MoveControlsDown();
			m_canvas->Show(true);
			imgBgAnim->Show(true);
			btnExpandMid->Show(false);
			btnCollapseMid->Show(true);
			btnAttProj->Show(true);
			midAppCollapsed = false;
			wrkUnitNB->SetSize(-1, -1, 370, 353); // fix
		}else if((!midAppCollapsed) && (btmAppCollapsed)){
            this->SetSize(-1, -1, 416, 581);
			stMyProj->Show(true);
			btnExpandMid->Show(false);
			btnCollapseMid->Move(366,429);
			btnAttProj->Move(250,431);
			//midAppCollapsed = false;
			btmAppCollapsed = false;
		}
		Refresh();
    }else if(m_wxBtnObj==btnAdvancedView) {
        wxGetApp().SetActiveGUI(BOINC_ADVANCEDGUI, true);
    }else{
		//wxMessageBox("OnBtnClick - else");
	}
}
//end function
void CSimpleFrame::MoveControlsUp(){
	stMyProj->Move(20,252);//(20,434)
	btnAttProj->Move(250,249);//(250,431)
	btnExpandMid->Move(366,247);//(366,429)
	lnMyProjTop->Move(20,272);//(20,454)
	w_statWCG->Move(60,278);//(60,460)
	w_statSeti->Move(112,278);//(112,460)
	w_statPred->Move(164,278);//(164,460)
	btnArwLeft->Move(25,291);//(25,483)
	btnArwRight->Move(360,291);//(360,483)
	lnMyProjBtm->Move(20,334);//(20,516)
	btnMessages->Move(28,340);//(28,522)
	btnPause->Move(55,340);//(55,522)
}

void CSimpleFrame::MoveControlsDown(){
	stMyProj->Move(20,434);
	btnAttProj->Move(250,431);
	btnExpandMid->Move(366,429);
	lnMyProjTop->Move(20,454);
	w_statWCG->Move(60,460);
	w_statSeti->Move(112,460);
	w_statPred->Move(164,460);
	btnArwLeft->Move(25,473);
	btnArwRight->Move(360,473);
	lnMyProjBtm->Move(20,516);
	btnMessages->Move(28,522);
	btnPause->Move(55,522);
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

 event.Skip(false);wxDC *dc;
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
// ---------------------------------------------------------------------------
// MyCanvas
// ---------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
    EVT_PAINT(MyCanvas::OnPaint)
END_EVENT_TABLE()

// Define a constructor for my canvas
MyCanvas::MyCanvas(wxWindow *parent, const wxPoint& pos, const wxSize& size)
        : wxScrolledWindow(parent, -1, pos, size,
                           wxNO_BORDER |
                           wxNO_FULL_REPAINT_ON_RESIZE)
{
    SetBackgroundColour(wxColour(_T("BLACK")));
}

void MyCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
#if 0
    CSimpleFrame* frame = (CSimpleFrame*) GetParent();
    if (frame->GetPlayer().IsPlaying())
    {
        frame->GetPlayer().Draw(dc);
    }
#endif
}

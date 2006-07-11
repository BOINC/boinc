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
#pragma implementation "sg_ViewTabPage.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "common/wxAnimate.h"
#include "common/wxFlatNotebook.h"
#include "sg_SkinClass.h"
#include "sg_ImageLoader.h"
#include "sg_ViewTabPage.h"


IMPLEMENT_DYNAMIC_CLASS(CViewTabPage, wxPanel)

enum{
	BTN_SHOW_GRAPHICS,
	BTN_COLLAPSE,
};

BEGIN_EVENT_TABLE(CViewTabPage, wxPanel)
    EVT_BUTTON(-1,CViewTabPage::OnBtnClick)
END_EVENT_TABLE()

CViewTabPage::CViewTabPage() {}

CViewTabPage::CViewTabPage(wxFlatNotebook* parent,int index,std::string name) :
    wxPanel(parent, -1, wxDefaultPosition, wxSize(370,330), wxNO_BORDER)
{
    wxASSERT(parent);
	m_tabIndex = index;
	m_name = name;
    LoadSkinImages();

	CreatePage();

}

CViewTabPage::~CViewTabPage() {}
void CViewTabPage::LoadSkinImages(){

	//app skin class
	appSkin = SkinClass::Instance();
	wxString dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	
    // prj icons will be removed
	g_prjIcn = new wxImage(dirPref + appSkin->GetIcnPrjWCG(), wxBITMAP_TYPE_PNG);
	// collapse
    g_collapse = new wxImage(dirPref + appSkin->GetBtnCollapse(), wxBITMAP_TYPE_PNG);
	g_collapseClick = new wxImage(dirPref + appSkin->GetBtnCollapseClick(), wxBITMAP_TYPE_PNG);
	btmpCol= wxBitmap(g_collapse); 
    btmpColClick= wxBitmap(g_collapseClick); 
	// expand
   // g_expand = new wxImage(dirPref + appSkin->GetBtnExpand(), wxBITMAP_TYPE_PNG);
	//g_expandClick = new wxImage(dirPref + appSkin->GetBtnExpandClick(), wxBITMAP_TYPE_PNG);
	//btmpExp= wxBitmap(g_expand); 
   // btmpExpClick= wxBitmap(g_expandClick); 
    //show graphic
    g_showGraphic = new wxImage(dirPref + appSkin->GetBtnShowGraphic(), wxBITMAP_TYPE_PNG);
	g_showGraphicClick = new wxImage(dirPref + appSkin->GetBtnShowGraphicClick(), wxBITMAP_TYPE_PNG);
	btmpShwGrph= wxBitmap(g_showGraphic); 
    btmpShwGrphClick= wxBitmap(g_showGraphicClick); 
	//////////////////////////////
	fileImgBuf[0].LoadFile(dirPref + appSkin->GetAnimationBG(),wxBITMAP_TYPE_BMP);
	btmpBgAnim=&fileImgBuf[0];
}







void CViewTabPage::CreatePage()
{
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	
	RESULT* result = pDoc->results.results[m_tabIndex];
	RESULT* resState = pDoc->state.lookup_result(result->project_url, result->name);
		
	//////////////////////Build Tab Page///////////////////////////////
	//Prj Icon
	w_iconPI=new wxWindow(this,-1,wxPoint(2,2),wxSize(22,22));
    i_prjIcnPI = new ImageLoader(w_iconPI);
    i_prjIcnPI->LoadImage(g_prjIcn);
	//Project Name
	lblProjectName=new wxStaticText(this,-1,wxT(""),wxPoint(25,2),wxSize(289,18),wxST_NO_AUTORESIZE);
	lblProjectName->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	wxString projName;
	projName = wxString(resState->project->project_name.c_str(), wxConvUTF8 ) + wxT(" - ") + wxString(resState->app->user_friendly_name.c_str(), wxConvUTF8);
	lblProjectName->SetLabel(projName);
	lblProjectName->SetFont(wxFont(11,74,90,90,0,wxT("Tahoma")));
	//Line Proj Name
	lnProjName=new wxStaticLine(this,-1,wxPoint(9,25),wxSize(353,2));
	//My Progress
	lblMyProgress=new wxStaticText(this,-1,wxT(""),wxPoint(15,32),wxSize(89,18),wxST_NO_AUTORESIZE);
	lblMyProgress->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	lblMyProgress->SetLabel(wxT("My Progress:"));
	lblMyProgress->SetFont(wxFont(10,74,90,92,0,wxT("Tahoma")));
	//Main Gauge
	gaugeWUMain=new wxGauge(this,-1,100,wxPoint(15,60),wxSize(340,30),wxGA_SMOOTH);
	gaugeWUMain->SetForegroundColour(appSkin->GetGaugeFgCol());
	gaugeWUMain->SetBackgroundColour(appSkin->GetGaugeBgCol());
	gaugeWUMain->SetValue(floor(result->fraction_done * 100000)/1000);
    //Work Unit Name
	lblWrkUnitName=new wxStaticText(this,-1,wxT(""),wxPoint(110,34),wxSize(250,13),wxST_NO_AUTORESIZE);
	lblWrkUnitName->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	lblWrkUnitName->SetLabel(wxString(result->name.c_str(),wxConvUTF8));
	//Elapsed Time
	lblElapsedTime=new wxStaticText(this,-1,wxT(""),wxPoint(15,97),wxSize(84,18),wxST_NO_AUTORESIZE);
	lblElapsedTime->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	lblElapsedTime->SetLabel(wxT("Elapsed Time:"));
	lblElapsedTime->SetFont(wxFont(10,74,90,90,0,wxT("Tahoma")));
	//Elapsed time Value
	wxString strBuffer = wxEmptyString;
	lblElapsedTimeValue=new wxStaticText(this,-1,wxT(""),wxPoint(102,97),wxSize(250,18),wxST_NO_AUTORESIZE);
	lblElapsedTimeValue->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	FormatCPUTime(result, strBuffer);
	lblElapsedTimeValue->SetLabel(strBuffer);
	lblElapsedTimeValue->SetFont(wxFont(10,74,90,90,0,wxT("Tahoma")));
	//Time Remaining
	lblTimeRemaining=new wxStaticText(this,-1,wxT(""),wxPoint(15,119),wxSize(154,18),wxST_NO_AUTORESIZE);
	lblTimeRemaining->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	lblTimeRemaining->SetLabel(wxT("Time remaining:"));
	lblTimeRemaining->SetFont(wxFont(10,74,90,90,0,wxT("Tahoma")));
	//Time Remaining Value
	lblTimeRemainingValue=new wxStaticText(this,-1,wxT(""),wxPoint(115,119),wxSize(200,18),wxST_NO_AUTORESIZE);
	lblTimeRemainingValue->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	FormatTimeToCompletion(result, strBuffer);
	lblTimeRemainingValue->SetLabel(strBuffer);
	lblTimeRemainingValue->SetFont(wxFont(10,74,90,90,0,wxT("Tahoma")));
	// show graphic button 
	wxToolTip *ttShowGraphic = new wxToolTip(wxT("Launch Real-Time Graphics"));
	btnShowGraphic=new wxBitmapButton(this,BTN_SHOW_GRAPHICS,btmpShwGrph,wxPoint(315,117),wxSize(24,24),wxSIMPLE_BORDER);
	btnShowGraphic->SetBitmapSelected(btmpShwGrphClick);
	btnShowGraphic->SetBackgroundColour(wxColour(255,255,255));
	btnShowGraphic->SetToolTip(ttShowGraphic);
	// Collapse button
	wxToolTip *ttCollapse = new wxToolTip(wxT("Hide Graphic"));
	btnCollapse=new wxBitmapButton(this,BTN_COLLAPSE,btmpCol,wxPoint(338,117),wxSize(24,24),wxSIMPLE_BORDER);
	btnCollapse->SetBitmapSelected(btmpColClick);
	btnCollapse->SetBackgroundColour(wxColour(255,255,255));
	btnCollapse->SetToolTip(ttCollapse);
	// project image behind graphic <><><>
	imgBgAnim=new wxStaticBitmap(this,-1,*btmpBgAnim,wxPoint(5,146),wxSize(358,176));
	//// Animation Window
	wAnimWk1=new wxWindow(this,-1,wxPoint(85,146),wxSize(184,176),wxNO_BORDER);
	// media control
	/////////////
	m_canvas = new MyCanvas(wAnimWk1, wxPoint(0,0), wxSize(184,176));
	#if 0
		m_player.SetDestroyAnimation(false);
		m_player.SetWindow(m_canvas);
		m_player.SetPosition(wxPoint(0, 0));
	#endif
	m_animationCtrl = new wxGIFAnimationCtrl(m_canvas, -1, wxEmptyString,
		wxPoint(0, 0), wxSize(184, 184));
	m_animationCtrl->Stop();
	if (m_animationCtrl->LoadFile(wxT("skins/default/graphic/molecule.gif")))
	{
		m_animationCtrl->Play();
	}
	else
	{
		wxMessageBox(_T("Sorry, this animation was not a valid animated GIF."));
	}
/**/
	//SetSize(0,0,360,320,wxSIZE_FORCE);
}
void CViewTabPage::UpdateInterface()
{
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	
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

}

void CViewTabPage::OnBtnClick(wxCommandEvent& event){ //init function
	wxObject *m_wxBtnObj = event.GetEventObject();

	if(event.GetId()==BTN_SHOW_GRAPHICS){
		//refresh btn
        btnShowGraphic->Refresh();
	}else if(event.GetId()==BTN_COLLAPSE){
	}

}
wxInt32 CViewTabPage::FormatCPUTime(RESULT* rslt, wxString& strBuffer) const {
    float          fBuffer = 0;
    RESULT*        result = rslt;

    if (result) {
        if (result->active_task) {
            fBuffer = result->current_cpu_time;
        } else {
            if(result->state < RESULT_COMPUTE_ERROR) {
                fBuffer = 0;
            } else {
                fBuffer = result->final_cpu_time;
            }
        }
    }

    if (0 == fBuffer) {
        strBuffer = wxT("---");
    } else {
		SGUITimeFormat(fBuffer,strBuffer);
    }

    return 0;
}

wxInt32 CViewTabPage::FormatTimeToCompletion(RESULT* rslt, wxString& strBuffer) const {
    float          fBuffer = 0;
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    RESULT*        result = rslt;

    if (result) {
        fBuffer = result->estimated_cpu_time_remaining;
    }

    if (0 >= fBuffer) {
        strBuffer = wxT("---");
    } else {
        SGUITimeFormat(fBuffer,strBuffer);
    }

    return 0;
}
void CViewTabPage::SGUITimeFormat(float fBuff, wxString& strBuffer) const{
	float          fBuffer = fBuff;
	wxInt32        iDay = 0;
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
	std::string    timeFormat;
    std::string formatedUnit; // string to recive the number when doing conversion
    char b[50]; // buffer of chars
	int radix=10; // 2:bin, 8:octal, 10:dec, 16:hex
	
	iDay = (wxInt32)(fBuffer / (60 * 60 * 60));
	iHour = (wxInt32)(fBuffer / (60 * 60));
	iMin  = (wxInt32)(fBuffer / 60) % 60;
	iSec  = (wxInt32)(fBuffer) % 60;
	
	if(iDay !=0){
		formatedUnit = itoa(iDay,b,radix);
		timeFormat = formatedUnit + " days ";
	}//else if(iHour !=0){
		formatedUnit = itoa(iHour,b,radix);
		timeFormat += formatedUnit + " hours ";
	//}else if(iMin !=0){
		formatedUnit = itoa(iMin,b,radix);
		timeFormat += formatedUnit + " minutes ";
	//}
	formatedUnit = itoa(iSec,b,radix);
	timeFormat += formatedUnit + " seconds";
	strBuffer = wxString(timeFormat.c_str(), wxConvUTF8);
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
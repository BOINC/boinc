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
#include "app_ipc.h"
#include "sg_ViewTabPage.h"
#include "sg_SkinClass.h"
#include "sg_StaticLine.h"
#include "sg_ProgressBar.h"
#include "sg_ImageButton.h"


IMPLEMENT_DYNAMIC_CLASS(CViewTabPage, wxPanel)

enum{
	BTN_SHOW_GRAPHICS,
	BTN_COLLAPSE,
};


BEGIN_EVENT_TABLE(CViewTabPage, wxPanel)
    EVT_PAINT(CViewTabPage::OnPaint)
	EVT_ERASE_BACKGROUND(CViewTabPage::OnEraseBackground)
END_EVENT_TABLE()

CViewTabPage::CViewTabPage() {}

CViewTabPage::CViewTabPage(wxFlatNotebook* parent,int index,std::string name,std::string url) :
    wxPanel(parent, -1, wxDefaultPosition, wxSize(370,330), wxNO_BORDER)
{
    wxASSERT(parent);
	m_tabIndex = index;
	m_name = name;
	isAlive = true;
	m_prjUrl = url;
    m_hasGraphic = false;
	// init doc and results data
	pDoc  = wxGetApp().GetDocument();
	resultWU = pDoc->results.results[m_tabIndex];
	//load skin images
    LoadSkinImages();
    //create page
	CreatePage();

}

CViewTabPage::~CViewTabPage() {}
void CViewTabPage::LoadSkinImages(){

	//app skin class
	appSkin = SkinClass::Instance();
	dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	//anim bg
	g_projBg = new wxImage(dirPref + appSkin->GetAnimationBg(), wxBITMAP_TYPE_PNG);
	//////////////////////////////
	//component bg
	fileImgBuf[0].LoadFile(dirPref + appSkin->GetWorkunitBg(),wxBITMAP_TYPE_PNG);
	btmpComponentBg=&fileImgBuf[0];
}

void CViewTabPage::CreatePage()
{
	RESULT* resState = NULL;
	resState = pDoc->state.lookup_result(resultWU->project_url, resultWU->name);
	if(resState){
		//////////////////////Build Tab Page///////////////////////////////
		//Project Name
		projName = wxString(resState->project->project_name.c_str(), wxConvUTF8 );
		projectFrName = wxString(resState->app->user_friendly_name.c_str(), wxConvUTF8);
		//
		//Project
	} else {
		projName = wxString("Not Available - Need State Update from Rom", wxConvUTF8 );
		projectFrName = wxString("Not Available - Need State Update from Rom", wxConvUTF8);
	}
	//Line Proj Name
	lnProjName = new CStaticLine(this,wxPoint(20,36),wxSize(316,1));
	lnProjName->SetLineColor(appSkin->GetStaticLineCol());
	//
	wxStaticLine spacerLine = new wxStaticLine(this,-1,wxPoint(20,36),wxSize(305,1));

	//My Progress
	wrkUnitName = wxString(resultWU->name.c_str(),wxConvUTF8);
	//Main Gauge
    gaugeWUMain=new CProgressBar(this,wxPoint(20,89));
	gaugeWUMain->SetValue(floor(resultWU->fraction_done * 100000)/1000);
	//percent
	percNum = (wxInt32)(floor(resultWU->fraction_done * 100000)/1000);
    percStr.Printf(_("%d"), percNum);
	gaugePercent = percStr + _T(" %");
    //Elapsed Time
	FormatCPUTime(resultWU, elapsedTimeValue);
	FormatTimeToCompletion(resultWU, timeRemainingValue);
	// show graphic button 
	if (resultWU->supports_graphics) {
		m_hasGraphic = true;
	}
	// project image behind graphic <><><>
	wxString dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	wxImage g_projBg = new wxImage(dirPref + appSkin->GetAnimationBg(), wxBITMAP_TYPE_PNG);
	btnAminBg = new CImageButton(this,wxBitmap(g_projBg),wxPoint(28,154),wxSize(294,146),m_hasGraphic);

	//// Animation Window
	wAnimWk1=new wxWindow(this,-1,wxPoint(98,156),wxSize(148,142),wxNO_BORDER);
	// media control
	/////////////
	m_canvas = new MyCanvas(wAnimWk1, wxPoint(0,0), wxSize(148,142));
	#if 0
		m_player.SetDestroyAnimation(false);
		m_player.SetWindow(m_canvas);
		m_player.SetPosition(wxPoint(0, 0));
	#endif
	m_animationCtrl = new wxGIFAnimationCtrl(m_canvas, -1, wxEmptyString,
		wxPoint(0, 0), wxSize(148, 142));
	m_animationCtrl->GetPlayer().UseBackgroundColour(true);
	m_animationCtrl->Stop();
	if (m_animationCtrl->LoadFile(dirPref + appSkin->GetAnimationFile()))
	{
		m_animationCtrl->Play();
	}
	else
	{
		wxMessageBox(_T("Sorry, this animation was not a valid animated GIF."));
	}
}
void CViewTabPage::UpdateInterface()
{
	resultWU = pDoc->results.results[m_tabIndex];
	wxString strBuffer = wxEmptyString;
	//Gauge
	gaugeWUMain->UpdateValue(floor(resultWU->fraction_done * 100000)/1000);
	//percent
	percNum = (wxInt32)(floor(resultWU->fraction_done * 100000)/1000);
    percStr.Printf(_("%d"), percNum);
	gaugePercent = percStr + _T(" %");
	// Elapsed Time
	FormatCPUTime(resultWU, elapsedTimeValue);
	//lblElapsedTimeValue->SetLabel(strBuffer);
	//lblElapsedTimeValue->Refresh();
    // Remaining time
	FormatTimeToCompletion(resultWU, timeRemainingValue);
	//lblTimeRemainingValue->SetLabel(strBuffer);
	//lblTimeRemainingValue->Refresh();
	DrawText();

}
void CViewTabPage::ReskinInterface()
{	
	//Load new skin images
	LoadSkinImages();
    //animation bg
	btnAminBg->SetImage(wxBitmap(g_projBg));
    //line
	lnProjName->SetLineColor(appSkin->GetStaticLineCol());
	// gauge
	gaugeWUMain->ReskinInterface();
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

    if (rslt) {
        fBuffer = rslt->estimated_cpu_time_remaining;
    }

    if (0 >= fBuffer) {
        strBuffer = wxT("---");
    } else {
        SGUITimeFormat(fBuffer,strBuffer);
    }

    return 0;
}

void CViewTabPage::SGUITimeFormat(float fBuffer, wxString& strBuffer) const {
	wxInt32        iDay = 0;
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxString       strTemp;

    strBuffer.clear();
    strTemp.clear();

	iHour = (wxInt32)(fBuffer / (60 * 60));
	iMin  = (wxInt32)(fBuffer / 60) % 60;
	iSec  = (wxInt32)(fBuffer) % 60;
	
	strBuffer.Printf(_("%d hours %d minutes %d seconds"), iHour, iMin, iSec);

}

void CViewTabPage::OnWorkShowGraphics() {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::OnWorkShowGraphics - Function Begin"));

    wxInt32  iAnswer        = 0; 
    wxString strMachineName = wxEmptyString;
    
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // TODO: implement hide as well as show
#if (defined(_WIN32) || defined(__WXMAC__))
    pDoc->GetConnectedComputerName(strMachineName);
    if (!pDoc->IsComputerNameLocal(strMachineName)) {
        iAnswer = ::wxMessageBox(
            _("Are you sure you want to display graphics on a remote machine?"),
            _("Show graphics"),
            wxYES_NO | wxICON_QUESTION,
            this
        );
    } else {
        iAnswer = wxYES;
    }
#else
    iAnswer = wxYES;
#endif

	if (wxYES == iAnswer) {
		std::string strDefaultWindowStation = std::string((const char*)wxGetApp().m_strDefaultWindowStation.mb_str());
		std::string strDefaultDesktop = std::string((const char*)wxGetApp().m_strDefaultDesktop.mb_str());
		std::string strDefaultDisplay = std::string((const char*)wxGetApp().m_strDefaultDisplay.mb_str());
        pDoc->WorkShowGraphics(
            m_prjUrl,
            m_name,
            MODE_WINDOW,
            strDefaultWindowStation,
            strDefaultDesktop,
            strDefaultDisplay
        );
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::OnWorkShowGraphics - Function End"));
}
void CViewTabPage::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
    //Project Name
	dc.SetFont(wxFont(16,74,90,90,0,wxT("Arial"))); 
	dc.DrawText(projName, wxPoint(20,8)); 
	//static: APPLICATION,MY PROGRESS,ELAPSED TIME,TIME REMAINING
	dc.SetFont(wxFont(9,74,90,90,0,wxT("Arial")));
	dc.DrawText(wxT("APPLICATION >"), wxPoint(20,49)); 
    dc.DrawText(wxT("MY PROGRESS >"), wxPoint(20,71)); 
	dc.DrawText(wxT("ELAPSED TIME >"), wxPoint(20,115)); 
    dc.DrawText(wxT("TIME REMAINING >"), wxPoint(20,134)); 
    //static: projectFrName,wrkUnitName,gaugePercent,elapsedTimeValue,timeRemainingValue
    dc.SetFont(wxFont(9,74,90,92,0,wxT("Arial")));
	dc.DrawText(projectFrName, wxPoint(110,49)); 
    dc.DrawText(wrkUnitName, wxPoint(120,71)); 
    dc.DrawText(gaugePercent, wxPoint(290,90)); 
    dc.DrawText(elapsedTimeValue, wxPoint(118,115)); 
	dc.DrawText(timeRemainingValue, wxPoint(130,134)); 	
}
void CViewTabPage::OnImageButton() 
{ 
	if(m_hasGraphic){
		OnWorkShowGraphics();
	}

}
void CViewTabPage::DrawText() 
{ 
    wxClientDC dcc(this);
	wxBufferedDC dc(&dcc, wxSize(GetSize().GetWidth(), GetSize().GetHeight())); 

    //Project Name
	dc.DrawBitmap(*btmpComponentBg, 0, 0);
	dc.SetFont(wxFont(16,74,90,90,0,wxT("Arial"))); 
	dc.DrawText(projName, wxPoint(20,8)); 
	//static: APPLICATION,MY PROGRESS,ELAPSED TIME,TIME REMAINING
	dc.SetFont(wxFont(9,74,90,90,0,wxT("Arial")));
	dc.DrawText(wxT("APPLICATION >"), wxPoint(20,49)); 
    dc.DrawText(wxT("MY PROGRESS >"), wxPoint(20,71)); 
	dc.DrawText(wxT("ELAPSED TIME >"), wxPoint(20,115)); 
    dc.DrawText(wxT("TIME REMAINING >"), wxPoint(20,134)); 
    //static: projectFrName,wrkUnitName,gaugePercent,elapsedTimeValue,timeRemainingValue
    dc.SetFont(wxFont(9,74,90,92,0,wxT("Arial")));
	dc.DrawText(projectFrName, wxPoint(110,49)); 
    dc.DrawText(wrkUnitName, wxPoint(120,71)); 
    dc.DrawText(gaugePercent, wxPoint(290,90)); 
    dc.DrawText(elapsedTimeValue, wxPoint(118,115)); 
	dc.DrawText(timeRemainingValue, wxPoint(130,134)); 
}
void CViewTabPage::OnEraseBackground(wxEraseEvent& event){
  wxObject *m_wxWin = event.GetEventObject();
  if(m_wxWin==this){event.Skip(true);DrawBackImg(event,this,*btmpComponentBg,0);return;}
  event.Skip(true);
}
void CViewTabPage::DrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz){

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
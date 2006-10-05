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

CViewTabPage::CViewTabPage(wxFlatNotebook* parent,RESULT* result,std::string name,std::string url) :
    wxPanel(parent, -1, wxDefaultPosition, wxSize(370,330), wxNO_BORDER)
{
    wxASSERT(parent);
	m_name = name;
	isAlive = true;
	m_prjUrl = url;
    m_hasGraphic = false;
	resultWU = result;
	//load skin images
	appSkin = SkinClass::Instance();
    //create page
	CreatePage();

}

CViewTabPage::~CViewTabPage() {
	if ( wSlideShow <= NULL ) {
		delete wSlideShow;
	}
}

void CViewTabPage::CreatePage()
{
	// Show or don't show the icon if the WU is running
	RESULT* resState = NULL;
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	resState = pDoc->state.lookup_result(resultWU->project_url, resultWU->name);
	if(resState){
		projName = wxString(resState->project->project_name.c_str(), wxConvUTF8 );
		projectFrName = wxString(resState->app->user_friendly_name.c_str(), wxConvUTF8);
	} else {
		projName = wxString("Not Available", wxConvUTF8 );
		projectFrName = wxString("Not Available", wxConvUTF8);
	}
	//Line Proj Name
	lnProjName = new CStaticLine(this,wxPoint(20,36),wxSize(316,1));
	lnProjName->SetLineColor(appSkin->GetStaticLineCol());
	//TODO - is this line below needed?
	wxStaticLine* spacerLine = new wxStaticLine(this,-1,wxPoint(20,36),wxSize(305,1));

	//My Progress
	wrkUnitName = wxString(resultWU->name.c_str(),wxConvUTF8);
	//Main Gauge
    gaugeWUMain=new CProgressBar(this,wxPoint(20,89));
	gaugeWUMain->SetValue(floor(resultWU->fraction_done * 100000)/1000);
	//percent
	percNum = (wxFloat64)(floor(resultWU->fraction_done * 100000)/1000);
    percStr.Printf(_("%.1lf"), percNum);
	gaugePercent = percStr + _T(" %");
    //Elapsed Time
	FormatCPUTime(resultWU, elapsedTimeValue);
	FormatTimeToCompletion(resultWU, timeRemainingValue);
	// show graphic button 
	if (resultWU->supports_graphics) {
		m_hasGraphic = true;
	}
	// project image behind graphic <><><>
	btnAminBg = new CImageButton(this,*(appSkin->GetAnimationBg()),wxPoint(28,154),wxSize(294,146),m_hasGraphic);

	CreateSlideShowWindow();
}

void CViewTabPage::LoadSlideShow(std::vector<wxBitmap> *vSlideShow) {
	char urlDirectory[256];
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	RESULT* result = pDoc->state.lookup_result(resultWU->project_url, resultWU->name);
	// If result not found then return
	if ( result <= 0 ) return;
	url_to_project_dir((char *) result->project->master_url.c_str() ,urlDirectory);
	char file[512];
	char resolvedFile[512];
	wxBitmap* btmpSlideShow;
	for(int i=0; i<99; i++) {
		sprintf(file, "%s/slideshow_%s_%02d", urlDirectory, result->app->name.c_str(), i);
		if(boinc_resolve_filename(file, resolvedFile, sizeof(resolvedFile)) == 0){
			btmpSlideShow = new wxBitmap();
			if ( btmpSlideShow->LoadFile(resolvedFile, wxBITMAP_TYPE_ANY) ) {
				if (btmpSlideShow->Ok() ) {
					vSlideShow->push_back(*btmpSlideShow);
				}
			}
			delete btmpSlideShow;
		} else {
			break;
		}
	}
	if ( vSlideShow->size() == 0 ) {
		for(int i=0; i<99; i++) {
			sprintf(file, "%s/slideshow_%02d", urlDirectory, i);
			if(boinc_resolve_filename(file, resolvedFile, sizeof(resolvedFile)) == 0){
				btmpSlideShow = new wxBitmap();
				if ( btmpSlideShow->LoadFile(resolvedFile, wxBITMAP_TYPE_ANY) ) {
					if (btmpSlideShow->Ok() ) {
						vSlideShow->push_back(*btmpSlideShow);
					}
				}
				delete btmpSlideShow;
			} else {
				break;
			}
		}
	}
}

// This function will check to see if application specific images are available from the project.  If not, then
// it will return the default image from the skin
std::vector<wxBitmap> CViewTabPage::GetSlideShow() {
	std::vector<wxBitmap> vSlideShow;
	LoadSlideShow(&vSlideShow);
	if ( vSlideShow.size() == 0 ) {
		vSlideShow.push_back(*(appSkin->GetDefaultWorkunitImage()));
	}
	return vSlideShow;
}
void CViewTabPage::UpdateInterface()
{
	wxString strBuffer = wxEmptyString;
	//Gauge
	gaugeWUMain->UpdateValue(floor(resultWU->fraction_done * 100000)/1000);
	//percent
	percNum = (wxFloat64)(floor(resultWU->fraction_done * 100000)/1000);
    percStr.Printf(_("%.1lf"), percNum);
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

	// check to see if we can display graphics
	bool changed = false;
	if (resultWU->supports_graphics && resultWU->active_task_state == 1) {
		if ( !m_hasGraphic ) {
			changed = true;
		}
		m_hasGraphic = true;
	} else {
		if ( m_hasGraphic ) {
			changed = true;
		}
		m_hasGraphic = false;
	}
	btnAminBg->SetShowText(m_hasGraphic);
	if ( changed ) {
		btnAminBg->Update();
		m_canvas->Refresh();
		m_canvas->Update();
	}
}
void CViewTabPage::CreateSlideShowWindow() {
	if ( wSlideShow <= NULL ) {
		delete wSlideShow;
	}
	wSlideShow=new wxWindow(this,-1,wxPoint(32,168),wxSize(286,116),wxNO_BORDER);
	m_canvas = new MyCanvas(wSlideShow, wxPoint(0,0), wxSize(286,116), GetSlideShow());
}
void CViewTabPage::ReskinInterface()
{	
    //animation bg
	btnAminBg->SetImage(*(appSkin->GetAnimationBg()));
    //line
	lnProjName->SetLineColor(appSkin->GetStaticLineCol());
	// gauge
	gaugeWUMain->ReskinInterface();
	CreateSlideShowWindow();
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
	CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxInt32  iAnswer        = 0; 
    wxString strMachineName = wxEmptyString;
    
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

}
void CViewTabPage::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::OnPaint - Begin"));
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
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::OnPaint - End"));
}
void CViewTabPage::OnImageButton() 
{ 
	if(m_hasGraphic){
		OnWorkShowGraphics();
	}

}
void CViewTabPage::DrawText() 
{ 
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::DrawText - Begin"));
    wxClientDC dcc(this);
	wxBufferedDC dc(&dcc, wxSize(GetSize().GetWidth(), GetSize().GetHeight())); 

    //Project Name
	dc.DrawBitmap(*(appSkin->GetWorkunitBg()), 0, 0);
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
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::DrawText - End"));
}
void CViewTabPage::OnEraseBackground(wxEraseEvent& event){
  wxObject *m_wxWin = event.GetEventObject();
  if(m_wxWin==this){event.Skip(true);DrawBackImg(event,this,*(appSkin->GetWorkunitBg()),0);return;}
  event.Skip(true);
}
void CViewTabPage::DrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap bitMap,int opz){
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::DrawBackImg - Begin"));
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
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::DrawBackImg - End"));
}
// ---------------------------------------------------------------------------
// MyCanvas
// ---------------------------------------------------------------------------

#define ID_CHANGE_SLIDE_TIMER  14000

BEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
    EVT_PAINT(MyCanvas::OnPaint)
	EVT_TIMER(ID_CHANGE_SLIDE_TIMER, MyCanvas::OnChangeSlide)
END_EVENT_TABLE()

// Define a constructor for my canvas
MyCanvas::MyCanvas(wxWindow *parent, const wxPoint& pos, const wxSize& size, std::vector<wxBitmap> images)
        : wxScrolledWindow(parent, -1, pos, size,
                           wxNO_BORDER |
                           wxNO_FULL_REPAINT_ON_RESIZE)
{
    SetBackgroundColour(wxColour(_T("BLACK")));
	wxBitmap* image;
	ImageLoader* il;
	double xRatio, yRatio, ratio;
	for(unsigned int i=0; i < images.size(); i++) {
		image = &(images.at(i));

		// Check to see if they need to be rescaled to fit in the window
		ratio = 1.0;
		xRatio = ((double) size.GetWidth())/((double) image->GetWidth());
		yRatio = ((double) size.GetHeight())/((double) image->GetHeight());
		if ( xRatio < ratio ) {
			ratio = xRatio;
		}
		if ( yRatio < ratio ) {
			ratio = yRatio;
		}
		if ( ratio < 1.0 ) {
			wxImage img = image->ConvertToImage();
			img.Rescale((int) image->GetWidth()*ratio, (int) image->GetHeight()*ratio);
			image = new wxBitmap(img);
		}
		il = new ImageLoader(this, true);
		il->LoadImage(*image);
		if ( ratio < 1.0 ) {
			delete image;
		}
		il->Show(false);
		vSlideShow.push_back(il);
	} 
	currentImageIndex=0;
	vSlideShow.at(currentImageIndex)->Show(true);
	changeSlideTimer = new wxTimer(this, ID_CHANGE_SLIDE_TIMER);
	if (vSlideShow.size() > 1 ) {
		changeSlideTimer->Start(5000); 
	}
}

MyCanvas::~MyCanvas() {
	if ( changeSlideTimer->IsRunning() ) {
		changeSlideTimer->Stop();
	}
	delete changeSlideTimer;
}

void MyCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxLogTrace(wxT("Function Start/End"), wxT("MyCanvas::OnPaint - Begin"));
    wxPaintDC dc(this);
//	if ( vSlideShow.size() > 0 && vSlideShow.at(0).Ok() ) {
//		dc.DrawBitmap(vSlideShow.at(0),0,0,false);
//	}
    wxLogTrace(wxT("Function Start/End"), wxT("MyCanvas::OnPaint - End"));
}

void MyCanvas::OnChangeSlide(wxTimerEvent& WXUNUSED(event)) {
	if ( currentImageIndex+1 == (int) vSlideShow.size() ) {
		vSlideShow.at(0)->Show(true);
		vSlideShow.at(currentImageIndex)->Show(false);
		currentImageIndex=0;
	} else {
		vSlideShow.at(currentImageIndex+1)->Show(true);
		vSlideShow.at(currentImageIndex)->Show(false);
		currentImageIndex++;
	}
	GetParent()->Update();
}

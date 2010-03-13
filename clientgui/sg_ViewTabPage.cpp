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
#pragma implementation "sg_ViewTabPage.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "common/wxFlatNotebook.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "sg_CustomControls.h" 
#include "sg_ProgressBar.h"
#include "sg_ImageButton.h"
#include "sg_ImageLoader.h"
#include "sg_ViewTabPage.h"
#include "app_ipc.h"


IMPLEMENT_DYNAMIC_CLASS(CViewTabPage, wxPanel)

enum{
	BTN_SHOW_GRAPHICS,
	BTN_COLLAPSE,
};


BEGIN_EVENT_TABLE(CViewTabPage, wxPanel)
    EVT_PAINT(CViewTabPage::OnPaint)
	EVT_ERASE_BACKGROUND(CViewTabPage::OnEraseBackground)
END_EVENT_TABLE()

bool isRunning(RESULT* result) {
	bool outcome = false;

	// It must be scheduled to be running
	if ( result->scheduler_state == CPU_SCHED_SCHEDULED ) {
		// If either the project or task have been suspended, then it cannot be running
		if ( !result->suspended_via_gui && !result->project_suspended_via_gui ) {
			CMainDocument* pDoc     = wxGetApp().GetDocument();
			CC_STATUS status;
			pDoc->GetCoreClientStatus(status);
			// Make sure that the core client isn't global suspended for some reason
			if ( status.task_suspend_reason == 0 || status.task_suspend_reason == SUSPEND_REASON_CPU_THROTTLE ) {
				outcome = true;
			}
		}
	}

	return outcome;
}


CViewTabPage::CViewTabPage() {}

CViewTabPage::CViewTabPage(
    WorkunitNotebook* parent,RESULT* result, std::string name, char* url
) :
    wxPanel(parent, -1, wxDefaultPosition, wxSize(370,330), wxNO_BORDER)
{
    wxASSERT(parent);
	m_name = name;
	isAlive = true;
    strcpy(project_url, url);
    m_hasGraphic = false;
	resultWU = result;
    //create page
	CreatePage();
	project_files_downloaded_time = 0;
	
#if wxCHECK_VERSION(2,8,0)
    // Workaround for unknown problem with newer versions of wxWidgets
    // Without this, the size of the panel would be reduced to 336,300
    SetMinSize(wxSize(343, 314));
#endif

}

CViewTabPage::~CViewTabPage() {
}

void CViewTabPage::CreatePage()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));


	// Show or don't show the icon if the WU is running
	RESULT* resState = NULL;
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	resState = pDoc->state.lookup_result(resultWU->project_url, resultWU->name);
	if(resState){
		projName = wxString(resState->project->project_name.c_str(), wxConvUTF8 );
        if (strlen(resState->app->user_friendly_name)) {
            projectFrName = wxString(resState->app->user_friendly_name, wxConvUTF8);
        } else {
            projectFrName = wxString(resState->avp->app_name, wxConvUTF8);
        }
	} else {
		projName = wxString("Not Available", wxConvUTF8 );
		projectFrName = wxString("Not Available", wxConvUTF8);
	}
	//Line Proj Name
	lnProjName = new CTransparentStaticLine(this, wxID_ANY, wxPoint(20,36), wxSize(316,1));
    lnProjName->SetLineColor(pSkinSimple->GetStaticLineColor());
	//Create with a two step process to eliminate compiler warning
	wxStaticLine* spacerLine = new wxStaticLine();
	spacerLine->Create(this,-1,wxPoint(20,36),wxSize(305,1));

	//My Progress
	wrkUnitName = wxString(resultWU->name, wxConvUTF8);
	//Main Gauge
    gaugeWUMain=new CProgressBar(this,wxPoint(20,282));
	gaugeWUMain->SetValue(floor(resultWU->fraction_done * 100000)/1000);
	//percent
	percNum = (wxFloat64)(floor(resultWU->fraction_done * 100000)/1000);
    percStr.Printf(_("%.1lf"), percNum);
	gaugePercent = percStr + _T(" %");
    //Elapsed Time
	FormatCPUTime(resultWU, elapsedTimeValue);
	FormatTimeToCompletion(resultWU, timeRemainingValue);
	// show graphic button 
	if (resultWU->supports_graphics || strlen(resultWU->graphics_exec_path)) {
		m_hasGraphic = true;
	}
	int status = ComputeState();

	// project image behind graphic <><><>
    btnAminBg = new CImageButton(
        this,
        *(pSkinSimple->GetWorkunitAnimationBackgroundImage()->GetBitmap()),
        wxPoint(24,72),
        wxSize(294,146),
        m_hasGraphic,
        status
    );

	CreateSlideShowWindow();
	Update();
}

int CViewTabPage::ComputeState() {
	int status = TAB_STATUS_PREEMPTED;
	if ( isRunning(resultWU) ) {
		status = TAB_STATUS_RUNNING;
	} else {
		CMainDocument* pDoc = wxGetApp().GetDocument();
		CC_STATUS ccStatus;
		pDoc->GetCoreClientStatus(ccStatus);
		if ( ccStatus.task_suspend_reason & SUSPEND_REASON_BATTERIES ) {
			status = TAB_STATUS_PAUSED_POWER;
		} else if ( ccStatus.task_suspend_reason & SUSPEND_REASON_USER_ACTIVE ) {
			status = TAB_STATUS_PAUSED_USER_ACTIVE;
		} else if ( ccStatus.task_suspend_reason & SUSPEND_REASON_USER_REQ ) {
			status = TAB_STATUS_PAUSED_USER_REQ;
		} else if ( ccStatus.task_suspend_reason & SUSPEND_REASON_TIME_OF_DAY ) {
			status = TAB_STATUS_PAUSED_TIME_OF_DAY;
		} else if ( ccStatus.task_suspend_reason & SUSPEND_REASON_BENCHMARKS ) {
			status = TAB_STATUS_PAUSED_BENCHMARKS;
		} else if ( ccStatus.task_suspend_reason & SUSPEND_REASON_INITIAL_DELAY ) {
			status = TAB_STATUS_INITIAL_DELAY;
		}
	}
	return status;
}

void CViewTabPage::LoadSlideShow(std::vector<wxBitmap> *vSlideShow) {
	char urlDirectory[256];
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	RESULT* result = pDoc->state.lookup_result(resultWU->project_url, resultWU->name);
	// If result not found then return
	if ( result <= 0 ) return;
	url_to_project_dir(result->project->master_url, urlDirectory);
	char file[512];
	char resolvedFile[512];
	wxBitmap* btmpSlideShow;
	for(int i=0; i<99; i++) {
		sprintf(file, "%s/slideshow_%s_%02d", urlDirectory, result->app->name, i);
		if(boinc_resolve_filename(file, resolvedFile, sizeof(resolvedFile)) == 0){
			btmpSlideShow = new wxBitmap();
			if ( btmpSlideShow->LoadFile(wxString(resolvedFile,wxConvUTF8), wxBITMAP_TYPE_ANY) ) {
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
				if ( btmpSlideShow->LoadFile(wxString(resolvedFile,wxConvUTF8), wxBITMAP_TYPE_ANY) ) {
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
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    std::vector<wxBitmap> vSlideShow;
	LoadSlideShow(&vSlideShow);
	if ( vSlideShow.size() == 0 ) {
        vSlideShow.push_back(*(pSkinSimple->GetWorkunitAnimationImage()->GetBitmap()));
	}

	return vSlideShow;
}

bool CViewTabPage::Downloading() {
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	if ( pDoc->results.results.size() > 0 ) {
		RESULT* result;
		for(unsigned int i=0; i < pDoc->results.results.size(); i++ ) {
			result = pDoc->result(i);
			if ( result != NULL && result->state == RESULT_FILES_DOWNLOADING ) {
				return true;
			}
		}
	}
	return false;
}

void CViewTabPage::UpdateInterface()
{
	// Check to see if new files have been downloaded for the project
	// If so, reload the slide show
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	PROJECT* project = pDoc->state.lookup_project(resultWU->project_url);
	if ( project > NULL && project->project_files_downloaded_time > project_files_downloaded_time) {
		project_files_downloaded_time = project->project_files_downloaded_time;
		GetCanvas()->ReloadSlideShow(GetSlideShow());
	}


	wxString strBuffer = wxEmptyString;
	//Gauge
	gaugeWUMain->SetValue(floor(resultWU->fraction_done * 100000)/1000);
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
	if ((resultWU->supports_graphics || strlen(resultWU->graphics_exec_path)) && isRunning(resultWU) ) {
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
	int newStatus = ComputeState();
	if ( btnAminBg->GetStatus() != newStatus ) {
		changed = true;
		btnAminBg->SetStatus(newStatus);
	}

	btnAminBg->SetEnableShowGraphics(m_hasGraphic);
	if ( changed ) {
		btnAminBg->Refresh();
		btnAminBg->Update();
		m_canvas->Refresh();
		m_canvas->Update();
	}
#ifdef __WXMAC__
        else {
		btnAminBg->Refresh();
		m_canvas->Refresh();
                lnProjName->Refresh();
        }
        gaugeWUMain->Refresh();
#endif

}


void CViewTabPage::CreateSlideShowWindow() {
	wSlideShow=new wxWindow(this,-1,wxPoint(26,74),wxSize(290,126),wxNO_BORDER);
	m_canvas = new MyCanvas(wSlideShow, wxPoint(0,0), wxSize(290,126), GetSlideShow());
}


void CViewTabPage::ReskinInterface()
{	
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    //animation bg
    btnAminBg->SetImage(*(pSkinSimple->GetWorkunitAnimationBackgroundImage()->GetBitmap()));
    //line
    lnProjName->SetLineColor(pSkinSimple->GetStaticLineColor());
	// gauge
	gaugeWUMain->ReskinInterface();
	wSlideShow->Destroy();
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
	
	strBuffer.Printf(_("%d hr %d min %d sec"), iHour, iMin, iSec);

}

void CViewTabPage::OnWorkShowGraphics() {
    CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxInt32  iAnswer        = 0; 
    wxString strMachineName = wxEmptyString;
    
    // TODO: implement hide as well as show
#if (defined(_WIN32) || defined(__WXMAC__))
    pDoc->GetConnectedComputerName(strMachineName);
    if (!pDoc->IsComputerNameLocal(strMachineName)) {
        iAnswer = wxGetApp().SafeMessageBox(
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
        pDoc->WorkShowGraphics(resultWU);
    }
}

void CViewTabPage::WriteText(wxDC* dc) {
    //Project Name
	dc->SetFont(wxFont(LARGE_FONT,74,90,90,0,wxT("Arial"))); 
	dc->DrawText(projName, wxPoint(20,8)); 

    dc->SetFont(wxFont(SMALL_FONT,74,90,90,0,wxT("Arial")));
	FormatText(_("Application: "), projectFrName, dc, wxPoint(20,47));

	int height, width;
	dc->GetTextExtent(_("Time Remaining: "), &width, &height);
	width = width + 20; // add the starting x position

	FormatText(_("Elapsed Time: "), elapsedTimeValue, dc, wxPoint(20,230), width);
	FormatText(_("Time Remaining: "), timeRemainingValue, dc, wxPoint(20,255), width);

    dc->DrawText(gaugePercent, wxPoint(290,283)); 

}

void CViewTabPage::FormatText(const wxString& title, const wxString& text, wxDC* dc, wxPoint pos, int col_width, wxFont font) {
    wxCoord width, height;
    wxCoord col = pos.x;
    wxCoord max_col = 330;
    wxString translated_text;

    // Title
    dc->SetFont(font);
	dc->GetTextExtent(title, &width, &height);
	if ( col_width == 0 ) {
		pos.x = 20;
		col_width = width + pos.x;
	} else {
		pos.x = col_width - width;
	}
	dc->DrawText(title, pos);

    // How wide was the title? Add 5 pixel buffer before drawing the next piece of text.
    col = col_width + 3;

    // Text
 	dc->GetTextExtent(text, &width, &height);
    if ( width > (max_col - col) ) {
		int i = (int) text.length();
		while ( width > (max_col - col) ) {
			i--;
			dc->GetTextExtent(text.substr(0,i) + _T("..."), &width, &height);
		}
		translated_text = text.substr(0,i) + _T("...");
    } else {
        translated_text = text;
    }
	dc->DrawText(translated_text, wxPoint(col, pos.y));
}


void CViewTabPage::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::OnPaint - Begin"));
	wxPaintDC dc(this);
	WriteText(&dc);
    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::OnPaint - End"));
}


void CViewTabPage::OnImageButton() { 
	if (m_hasGraphic) {
		OnWorkShowGraphics();
	}
}

void CViewTabPage::DrawText() { 
CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::DrawText - Begin"));
    
    wxClientDC dcc(this);
	wxBufferedDC dc(&dcc); 

    //Project Name
    dc.DrawBitmap(*(pSkinSimple->GetWorkunitAreaBackgroundImage()->GetBitmap()), 0, 0);
	WriteText(&dc);

    wxLogTrace(wxT("Function Start/End"), wxT("CViewTabPage::DrawText - End"));
}

void CViewTabPage::OnEraseBackground(wxEraseEvent& event){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	wxDC* dc=event.GetDC();
	dc->DrawBitmap(*pSkinSimple->GetWorkunitAreaBackgroundImage()->GetBitmap(), 0, 0);
}

// ---------------------------------------------------------------------------
// MyCanvas
// ---------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MyCanvas, wxWindow)
    EVT_PAINT(MyCanvas::OnPaint)
END_EVENT_TABLE()

MyCanvas::MyCanvas(wxWindow *parent, const wxPoint& pos, const wxSize& size, std::vector<wxBitmap> images)
        : wxWindow(parent, -1, pos, size, wxNO_BORDER | wxNO_FULL_REPAINT_ON_RESIZE)
{
    SetBackgroundColour(wxColour(_T("BLACK")));
	ssImages = images;
	reloadSlideShow = false;
	LoadSlideShow();
}

void MyCanvas::LoadSlideShow() {
	vSlideShow.clear();

	// Now load the new slide show
	wxBitmap* image;
	ImageLoader* il;
	double xRatio, yRatio, ratio;
	for(unsigned int i=0; i < ssImages.size(); i++) {
		image = &(ssImages.at(i));

		// Check to see if they need to be rescaled to fit in the window
		ratio = 1.0;
		xRatio = ((double) GetSize().GetWidth())/((double) image->GetWidth());
		yRatio = ((double) GetSize().GetHeight())/((double) image->GetHeight());
		if ( xRatio < ratio ) {
			ratio = xRatio;
		}
		if ( yRatio < ratio ) {
			ratio = yRatio;
		}
		if ( ratio < 1.0 ) {
			wxImage img = image->ConvertToImage();
			img.Rescale((int) (image->GetWidth()*ratio), (int) (image->GetHeight()*ratio));
			image = new wxBitmap(img);
		}
		if ( xRatio == yRatio ) {
			// If it perfectly fills the window, don't center
			// Centering causes a bug (at least on windows) where if the taskbar
			// is at the top of the page, the images are offset
			il = new ImageLoader(this, false);
		} else {
			il = new ImageLoader(this, true);
		}
		il->LoadImage(*image);
		if ( ratio < 1.0 ) {
			delete image;
		}
		il->Show(false);
		vSlideShow.push_back(il);
	} 
	currentImageIndex=0;
	vSlideShow.at(currentImageIndex)->Show(true);
}

void MyCanvas::ReloadSlideShow(std::vector<wxBitmap> images) {
	ssImages = images;
	reloadSlideShow = true;
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

void MyCanvas::AdvanceSlide() {
	if ( currentImageIndex+1 == (int) vSlideShow.size() ) {
		if ( reloadSlideShow ) {
			vSlideShow.at(currentImageIndex)->Show(false);
			LoadSlideShow();
			vSlideShow.at(0)->Show(true);
			reloadSlideShow=false;
		} else {
			if ( vSlideShow.size() > 1 ) {
				vSlideShow.at(0)->Show(true);
				vSlideShow.at(currentImageIndex)->Show(false);
			}
		}
		currentImageIndex=0;
	} else {
		vSlideShow.at(currentImageIndex+1)->Show(true);
		vSlideShow.at(currentImageIndex)->Show(false);
		currentImageIndex++;
	}
	GetParent()->Update();
}

#define ID_CHANGE_SLIDE_TIMER  14000

BEGIN_EVENT_TABLE(WorkunitNotebook, wxFlatNotebook)
	EVT_TIMER(ID_CHANGE_SLIDE_TIMER, WorkunitNotebook::OnChangeSlide)
        EVT_FLATNOTEBOOK_PAGE_CHANGED(wxID_ANY, WorkunitNotebook::OnTabChanged)
END_EVENT_TABLE()

WorkunitNotebook::WorkunitNotebook(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
	wxFlatNotebook(parent, id, pos, size, style, name)
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	SetUseBackground(true);
    SetTabAreaBackgroundImage(
        pSkinSimple->GetWorkunitTabAreaBackgroundImage()->GetBitmap()
    );
    SetBackgroundColour(
        *pSkinSimple->GetBackgroundImage()->GetBackgroundColor()
    );
	SetTabAreaColour(
        *pSkinSimple->GetBackgroundImage()->GetBackgroundColor()
    );
	SetGradientColors(
        *pSkinSimple->GetWorkunitActiveTab()->GetGradientFromColor(),
        *pSkinSimple->GetWorkunitActiveTab()->GetGradientToColor(),
        *pSkinSimple->GetWorkunitActiveTab()->GetBorderColor()
    );
	SetGradientColorsInactive(
        *pSkinSimple->GetWorkunitSuspendedTab()->GetGradientFromColor(),
        *pSkinSimple->GetWorkunitSuspendedTab()->GetGradientToColor(),
        *pSkinSimple->GetWorkunitSuspendedTab()->GetBorderColor()
    );

	char red = (char) 
        (((int) pSkinSimple->GetWorkunitSuspendedTab()->GetGradientFromColor()->Red() + 
          (int) pSkinSimple->GetWorkunitSuspendedTab()->GetGradientToColor()->Red() + 255*3)/5);
	char green = (char) 
        (((int) pSkinSimple->GetWorkunitSuspendedTab()->GetGradientFromColor()->Green() +
          (int) pSkinSimple->GetWorkunitSuspendedTab()->GetGradientToColor()->Green() + 255*3)/5);
	char blue = (char)
        (((int) pSkinSimple->GetWorkunitSuspendedTab()->GetGradientFromColor()->Blue() +
          (int) pSkinSimple->GetWorkunitSuspendedTab()->GetGradientToColor()->Blue() + 255*3)/5);
    SetActiveTabTextColour(wxColour(255,255,255));
	SetNonActiveTabTextColour(wxColour(red, green, blue));

    m_ImageList.push_back(*(pSkinSimple->GetWorkunitActiveTab()->GetBitmap()));
	SetImageList(&m_ImageList);

    changeSlideTimer = new wxTimer(this, ID_CHANGE_SLIDE_TIMER);
	changeSlideTimer->Start(10000); 

    Update();
#ifdef __WXMAC__
        // If no applications are running, the page corresponding to the last tab 
        // would be displayed intially, but that tab itself is not visible if there 
        // are more tabs than fit on page, causing problems.  So we select the 
        // first tab as a default.
    if (m_windows.size())       // If tab 0 exists
        SetSelection(0);
#endif

    for (int i=0; i< (int) m_windows.size(); i++) {
		if ( isRunning(m_windows.at(i)->resultWU) ) {
			SetSelection(i);
			break;
		}
	}
}

WorkunitNotebook::~WorkunitNotebook() {
	if ( changeSlideTimer->IsRunning() ) {
		changeSlideTimer->Stop();
	}
	delete changeSlideTimer;

}

void WorkunitNotebook::AddTab(RESULT* result) {
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	RESULT* resState = NULL;
	std::string projUrl = result->project_url;
	std::string nme = result->name;
    resState = pDoc->state.lookup_result(result->project_url, result->name);
	if(!resState){
		pDoc->ForceCacheUpdate();
 		return;
	}
 	wxString appShortName = wxString(resState->app->name, wxConvUTF8 );
	// Do not update screen at this point
    Freeze();
	std::string index = " ";
	appShortName += wxString(index.c_str(), wxConvUTF8 );
	CViewTabPage *wTab = new CViewTabPage(this, result, nme, result->project_url);

	AddPage(wTab, appShortName, true);	
	if(isRunning(resState) ){
		int pageIndex = GetPageIndex(wTab);
		SetPageImageIndex(pageIndex, 0); // this is a running process
	}
	m_windows.push_back(wTab);
	GetParent()->GetSizer()->Layout();
	Thaw();
}

void WorkunitNotebook::ReskinAppGUI() {
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	SetUseBackground(true);
    SetTabAreaBackgroundImage(
        pSkinSimple->GetWorkunitTabAreaBackgroundImage()->GetBitmap()
    );
	SetTabAreaColour(
        *pSkinSimple->GetBackgroundImage()->GetBackgroundColor()
    );

	SetGradientColors(
        *pSkinSimple->GetWorkunitActiveTab()->GetGradientFromColor(),
        *pSkinSimple->GetWorkunitActiveTab()->GetGradientToColor(),
        *pSkinSimple->GetWorkunitActiveTab()->GetBorderColor()
    );
	SetGradientColorsInactive(
        *pSkinSimple->GetWorkunitSuspendedTab()->GetGradientFromColor(),
        *pSkinSimple->GetWorkunitSuspendedTab()->GetGradientToColor(),
        *pSkinSimple->GetWorkunitSuspendedTab()->GetBorderColor()
    );

    m_ImageList.clear();
	m_ImageList.push_back(*(pSkinSimple->GetWorkunitActiveTab()->GetBitmap()));
	SetImageList(&m_ImageList);

 	for(int i = 0; i < (int)m_windows.size(); i++){
		CViewTabPage *wTab = m_windows.at(i);
		wTab->ReskinInterface();
	}
}

void WorkunitNotebook::Update() {
	CMainDocument* pDoc     = wxGetApp().GetDocument();

	// Mark all inactive (this lets us loop only once)
	for(int x = 0; x < (int)m_windows.size(); x ++)
	{
		CViewTabPage *currTab = m_windows[x];
		currTab->isAlive = false;
	}
	// First update existing pages and add new ones
	RESULT* result;
	for(int i = 0; i < (int) pDoc->results.results.size(); i++){
		bool found = false;
		result = pDoc->result(i);
		// only check tasks that are active
		if ( result == NULL || !result->active_task ) {
			continue;
		}
		// loop through the open tabs to find 
		for(int j = 0; j < (int)m_windows.size(); j++) {
			CViewTabPage *currTab = m_windows[j];
			if(result->name == currTab->GetTabName()){
				currTab->resultWU = result;
                                if (GetSelection() == j)
                                    currTab->UpdateInterface();
				if(isRunning(result) && this->GetPageImageIndex(j) != 0){
					SetPageImageIndex(j, 0); // this result is current running
				} else if ( !isRunning(result) && this->GetPageImageIndex(j) != -1 ) {
					SetPageImageIndex(j, -1); // this result is not running
				}
				found = true;
				currTab->isAlive = true;
				break; // skip out of this loop
			}
		}

		// if it isn't currently one of the tabs then we have a new one!  lets add it
		if ( !found ) {
			AddTab(result);
		}

	}

        // Check pages in descending order so deletion won't change indexes of pages yet to be checked
	for(int x = (int)m_windows.size()-1; x >=0; x--) {
		CViewTabPage *currTab = m_windows[x];
		if(!currTab->isAlive){
			DeletePage(x);                          // delete the notebook page
			m_windows.erase(m_windows.begin()+x);   // delete the page in vector
		}
	}
}

void WorkunitNotebook::OnChangeSlide(wxTimerEvent& WXUNUSED(event)) {
	for(int x = 0; x < (int)m_windows.size(); x ++) {
		if ( GetPageImageIndex(x) == 0 ) {
			CViewTabPage *currTab = m_windows[x];
			currTab->GetCanvas()->AdvanceSlide();
		}
	}
}

void WorkunitNotebook::OnTabChanged(wxFlatNotebookEvent& WXUNUSED(event)) {
        int index = GetSelection();
        if (index < 0) return;      // No tabs
        if (index < (int)m_windows.size()) {
            CViewTabPage *currTab = m_windows[index];
            currTab->UpdateInterface();
        }
}

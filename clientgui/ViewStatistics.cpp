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
#pragma implementation "ViewStatistics.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewStatistics.h"
#include "Events.h"
#include "util.h"

#include "res/task.xpm"
#include "res/tips.xpm"


BEGIN_EVENT_TABLE (CPaintStatistics, wxPanel)
    EVT_PAINT(CPaintStatistics::OnPaint)
END_EVENT_TABLE ()

CPaintStatistics::CPaintStatistics() {
	m_SelectedStatistic=0;
	heading="User Total";
}

CPaintStatistics::CPaintStatistics(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
				wxPanel(parent, id, pos, size, style, name) {
	m_SelectedStatistic=0;
	heading="User Total";
}

void CPaintStatistics::OnPaint(wxPaintEvent& WXUNUSED(event)) {

	//Init global
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	PROJECTS *proj=&(pDoc->statistics_status);
	wxASSERT(NULL != proj);

	//Init drawing
    wxPaintDC dc (this);

    wxCoord width = 0, height = 0, heading_height=0, rectangle_x_start=0, rectangle_y_start=0,
		rectangle_x_end=0, rectangle_y_end=0;

    GetClientSize( &width, &height );

	dc.SetBackground(*wxWHITE_BRUSH);

	dc.SetTextForeground (GetForegroundColour ());
    dc.SetTextBackground (GetBackgroundColour ());

	wxFont heading_font(*wxSWISS_FONT);
	heading_font.SetWeight(wxBOLD);

	dc.SetFont(*wxSWISS_FONT);
	

	//Start drawing
	dc.BeginDrawing();

	dc.Clear();

	//Draw heading
	{
		dc.SetFont(heading_font);
		wxCoord w_temp, h_temp, des_temp, lead_temp;
        dc.GetTextExtent(heading, &w_temp, &h_temp, &des_temp, &lead_temp);
		heading_height=h_temp+lead_temp+5;
		dc.DrawText (heading, ((width/2)-(w_temp/2)), lead_temp+5);
		dc.SetFont(*wxSWISS_FONT);
	}

	//Number of Projects with statistics
	wxInt32 nb_proj=0;
	for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();
		i!=proj->projects.end(); ++i) {
			if ((*i)->statistics.size()>1) ++nb_proj;
	}
	if (nb_proj==0) return;

	//How many rows/colums?
	wxInt32 nb_proj_row=0, nb_proj_col=0;
	if (nb_proj<4) {
		nb_proj_col=1;
		nb_proj_row=nb_proj;
	} else {
		nb_proj_col=2;
		nb_proj_row=ceil((nb_proj/static_cast<double>(nb_proj_col)));
	}

	wxInt32 col=1, row=1; //Used to identify the actual row/col

	const double x_fac=width/nb_proj_col;
	const double y_fac=(height-heading_height)/nb_proj_row;

	wxInt32 count=-1;
	
	for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();
		i!=proj->projects.end(); ++i
    ) {
		++count;

		//No statistics
		if ((*i)->statistics.size()<2) continue;

		//Where do we draw in?
		wxCoord x_start=0, y_start=0, x_end=0, y_end=0;
		x_start=x_fac*(col-1);
		x_end=x_fac*(col);
		y_start=y_fac*(row-1)+heading_height;
		y_end=y_fac*row+heading_height;

		//Draw Project name
		{
			wxCoord w_temp, h_temp, des_temp, lead_temp, x, y;
			wxString name;
			pDoc->GetStatisticsProjectName(count,name);

			dc.GetTextExtent(name, &w_temp, &h_temp, &des_temp, &lead_temp);

			x=x_start+((x_end-x_start)/2)-(w_temp/2);
			y=y_start+lead_temp+5;
			dc.DrawText (name, x, y);
		}

		//Find minimum/maximum value
		double min_val=10e32, max_val=0;
		for (std::vector<STATISTIC>::const_iterator j=(*i)->statistics.begin();
			j!=(*i)->statistics.end();++j)
		{
            if (m_SelectedStatistic==0) {
				if (j->user_total_credit>max_val) max_val=j->user_total_credit;
				if (j->user_total_credit<min_val) min_val=j->user_total_credit;
			}
            if (m_SelectedStatistic==1) {
				if (j->user_expavg_credit>max_val) max_val=j->user_expavg_credit;
				if (j->user_expavg_credit<min_val) min_val=j->user_expavg_credit;
			}
            if (m_SelectedStatistic==2) {
				if (j->host_total_credit>max_val) max_val=j->host_total_credit;
				if (j->host_total_credit<min_val) min_val=j->host_total_credit;
			}
            if (m_SelectedStatistic==3) {
				if (j->host_expavg_credit>max_val) max_val=j->host_expavg_credit;
				if (j->host_expavg_credit<min_val) min_val=j->host_expavg_credit;
			}
		}
		min_val=min_val*0.999999-1;
		max_val=max_val*1.000001+1;
		if (min_val<0) min_val=0;

		//Draw scale
		{
			dc.SetBrush(*wxLIGHT_GREY_BRUSH);
			dc.SetPen(wxPen(wxColour ( 0 , 0 , 0 ) , 1 , wxSOLID) );


			wxCoord w_temp, h_temp, des_temp, lead_temp;
			dc.GetTextExtent(wxString::Format("%.0f", max_val), &w_temp, &h_temp, &des_temp, &lead_temp);

			rectangle_x_start=x_start+w_temp+2;
			rectangle_y_start=y_start+heading_height+2;
			rectangle_x_end=x_end-2;
			rectangle_y_end=y_end-2-h_temp;

			dc.GetTextExtent("days", &w_temp, &h_temp, &des_temp, &lead_temp);
			rectangle_x_end-=w_temp;

			dc.GetTextExtent(wxString::Format("%.0f", max_val), &w_temp, &h_temp, &des_temp, &lead_temp);
			dc.DrawRectangle(rectangle_x_start,rectangle_y_start,rectangle_x_end-rectangle_x_start,rectangle_y_end-rectangle_y_start);	
			dc.DrawText(wxString::Format("%.0f", max_val),x_start,rectangle_y_start-h_temp);

			dc.GetTextExtent(wxString::Format("%.0f", min_val), &w_temp, &h_temp, &des_temp, &lead_temp);
			dc.DrawText(wxString::Format("%.0f", min_val),rectangle_x_start-w_temp-2,rectangle_y_end-h_temp);
			dc.DrawText("days", rectangle_x_end, rectangle_y_end);

			//Draw day numbers and lines marking the days
			wxCoord xpos=rectangle_x_start;
			for (std::vector<STATISTIC>::const_iterator j=(*i)->statistics.begin(); j!=(*i)->statistics.end(); ++j) {
				double day=dday()-j->day;
				day=day/(60*60*24);
				dc.SetPen(wxPen(wxColour ( 0 , 0 , 0 ) , 1 , wxSOLID) );
				if (j!=(--(*i)->statistics.end())) dc.DrawText(wxString::Format("-%.0f", day),xpos,rectangle_y_end);
				if (j!=(--(*i)->statistics.end()) && j!=(*i)->statistics.begin()) {
					dc.SetPen(wxPen(wxColour ( 200 , 200 , 200 ) , 1 , wxSOLID) );
					dc.DrawLine(xpos,rectangle_y_start+1,xpos,rectangle_y_end-1);
				}
				xpos+=(rectangle_x_end-rectangle_x_start)/((*i)->statistics.size()-1);
			}
		}

		//Draw graph
		{
			const double yscale=(rectangle_y_end-rectangle_y_start-1)/(max_val-min_val);
			const double xscale=(rectangle_x_end-rectangle_x_start-1)/((*i)->statistics.size()-1);


			dc.SetPen(wxPen(wxColour ( 255, 255, 0 ) , 1 , wxSOLID) );

			wxCoord last_x=rectangle_x_start, last_y=0, xpos=rectangle_x_start, ypos=0;
			std::vector<STATISTIC>::const_iterator j=(*i)->statistics.begin();

			for (std::vector<STATISTIC>::const_iterator j=(*i)->statistics.begin(); j!=(*i)->statistics.end(); ++j) {

				ypos=rectangle_y_end - 1 - (yscale * (j->user_total_credit-min_val));
				if (m_SelectedStatistic==0) {
					ypos=rectangle_y_end - 1 - (yscale * (j->user_total_credit-min_val));
				}
				if (m_SelectedStatistic==1) {
					ypos=rectangle_y_end - 1 - (yscale * (j->user_expavg_credit-min_val));
				}
				if (m_SelectedStatistic==2) {
					ypos=rectangle_y_end - 1 - (yscale * (j->host_total_credit-min_val));
				}
				if (m_SelectedStatistic==3) {
					ypos=rectangle_y_end - 1 - (yscale * (j->host_expavg_credit-min_val));
				}
				
				if (last_y!=0) {
					dc.DrawLine(xpos,ypos,last_x,last_y);
				}

				last_x=xpos;
				last_y=ypos;
				xpos+=xscale;
			}
		}

		//Change row/col
		if (col==nb_proj_col) {
			col=1;
			++row;
		} else {
			++col;
		}
	}
	
	dc.EndDrawing();
}

IMPLEMENT_DYNAMIC_CLASS(CViewStatistics, CBOINCBaseView)

CViewStatistics::CViewStatistics()
{
}


CViewStatistics::CViewStatistics(wxNotebook* pNotebook) :
    CBOINCBaseView( pNotebook, ID_HTML_RESOURCEUTILIZATIONVIEW, DEFAULT_HTML_FLAGS, ID_LIST_RESOURCEUTILIZATIONVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS, true ) {
	wxASSERT(NULL != pNotebook);

    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    m_bItemSelected = false;

    m_strQuickTip = wxEmptyString;
    m_strQuickTipText = wxEmptyString;
	
    //
    // Globalization/Localization
    //
    LINK_DEFAULT             = wxT("default");

    //
    // Setup View
    //
    m_pTaskPane = NULL;
    m_pListPane = NULL;

    SetAutoLayout(TRUE);

    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(NULL != itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);
    
    m_pTaskPane = new CBOINCTaskCtrl( this, ID_HTML_RESOURCEUTILIZATIONVIEW, DEFAULT_HTML_FLAGS );
    wxASSERT(NULL != m_pTaskPane);

	//Needed for compatibility
	m_pListPane = new CBOINCListCtrl( this, ID_LIST_RESOURCEUTILIZATIONVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS );
	wxASSERT(NULL != m_pListPane);
	m_pListPane->Show(false);


	m_PaintStatistics = new CPaintStatistics( this, ID_LIST_RESOURCEUTILIZATIONVIEW, wxDefaultPosition, wxSize(-1, -1), DEFAULT_LIST_SINGLE_SEL_FLAGS );
	wxASSERT(NULL != m_PaintStatistics);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_PaintStatistics, 1, wxGROW|wxALL, 1);


    SetSizerAndFit(itemFlexGridSizer);

    //
    // Globalization/Localization
    //
    VIEW_HEADER              = wxT("statistics");

    SECTION_TASK             = VIEW_HEADER + wxT("task");
    SECTION_TIPS             = VIEW_HEADER + wxT("tips");

    BITMAP_RESOURCES         = VIEW_HEADER + wxT(".xpm");
    BITMAP_TASKHEADER        = SECTION_TASK + wxT(".xpm");
    BITMAP_TIPSHEADER        = SECTION_TIPS + wxT(".xpm");

    LINKDESC_DEFAULT         = 
        _("No available options currently defined.");


    LINK_TASKUSERTOTAL       = SECTION_TASK + wxT("user_total");
    LINKDESC_TASKUSERTOTAL   = _("");

    LINK_TASKUSERAVG         = SECTION_TASK + wxT("user_avg");
    LINKDESC_TASKUSERAVG     = _("");

    LINK_TASKHOSTTOTAL       = SECTION_TASK + wxT("host_total");
    LINKDESC_TASKHOSTTOTAL   = _("");

    LINK_TASKHOSTAVG         = SECTION_TASK + wxT("host_avg");
    LINKDESC_TASKHOSTAVG     = _("");

    //
    // Setup View
    //
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));


    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Tips"));

    m_bTipsHeaderHidden = false;
    m_bItemSelected = false;
	m_bTaskHeaderHidden = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
        LINKDESC_DEFAULT
    );

    UpdateSelection();

}

CViewStatistics::~CViewStatistics() {}


wxString CViewStatistics::GetViewName() {
    return wxString(_("Statistics"));
}


#if 0
const char** CViewStatistics::GetViewIcon() {
    return usage_xpm;
}
#endif


wxInt32 CViewStatistics::GetDocCount() {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

	static long old_val=0;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	long new_val=pDoc->GetStatisticsCount();
	if (new_val!=old_val) {
		m_PaintStatistics->Refresh();
		old_val=new_val;
	}
    
	return 0;
}


void CViewStatistics::OnTaskLinkClicked( const wxHtmlLinkInfo& link ) {
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);
	wxASSERT(NULL != m_PaintStatistics);

    m_bTaskHeaderHidden = false;
    m_bTipsHeaderHidden = false;

	if ( link.GetHref() == LINK_TASKUSERTOTAL ) {
		m_PaintStatistics->heading="User Total";
		m_PaintStatistics->m_SelectedStatistic=0;
	}
	if ( link.GetHref() == LINK_TASKUSERAVG ) {
		m_PaintStatistics->heading="User Average";
		m_PaintStatistics->m_SelectedStatistic=1;
	}
	if ( link.GetHref() == LINK_TASKHOSTTOTAL ) {
		m_PaintStatistics->heading="Host Total";
		m_PaintStatistics->m_SelectedStatistic=2;
	}
	if ( link.GetHref() == LINK_TASKHOSTAVG ) {
		m_PaintStatistics->heading="Host Average";
		m_PaintStatistics->m_SelectedStatistic=3;
	}

    UpdateSelection();
    m_PaintStatistics->Refresh();
}


void CViewStatistics::UpdateSelection() {
    UpdateTaskPane();
}


void CViewStatistics::UpdateTaskPane() {
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden) {
        m_pTaskPane->CreateTask( LINK_TASKUSERTOTAL, _("Show user total"), false );
        m_pTaskPane->CreateTask( LINK_TASKUSERAVG, _("Show user average"), false );
        m_pTaskPane->CreateTask( LINK_TASKHOSTTOTAL, _("Show host total"), false );
        m_pTaskPane->CreateTask( LINK_TASKHOSTAVG, _("Show host average"), false );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip( BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden );

    m_pTaskPane->EndTaskPage();
}

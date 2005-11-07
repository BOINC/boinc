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


BEGIN_EVENT_TABLE (CPaintStatistics, wxPanel)
    EVT_PAINT(CPaintStatistics::OnPaint)
END_EVENT_TABLE ()

CPaintStatistics::CPaintStatistics() {
	m_SelectedStatistic=0;
	heading=_("User Total");
}

CPaintStatistics::CPaintStatistics(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
				wxPanel(parent, id, pos, size, style, name) {
	m_SelectedStatistic=0;
	heading=_("User Total");
}

void CPaintStatistics::OnPaint(wxPaintEvent& WXUNUSED(event)) {

	//Init global
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	PROJECTS *proj=&(pDoc->statistics_status);
	wxASSERT(proj);

	//Init drawing
    wxPaintDC dc (this);

    wxCoord width = 0, height = 0, heading_height=0, rectangle_x_start=0, rectangle_y_start=0,
		rectangle_x_end=0, rectangle_y_end=0;

    GetClientSize(&width, &height);

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
		nb_proj_row=(wxInt32)ceil(static_cast<double>(nb_proj/static_cast<double>(nb_proj_col)));
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
		x_start=(wxCoord)(x_fac*(double)(col-1));
		x_end=(wxCoord)(x_fac*((double)col));
		y_start=(wxCoord)(y_fac*(double)(row-1)+heading_height);
		y_end=(wxCoord)(y_fac*(double)row+heading_height);

		//Draw Project name
		{
			wxCoord w_temp, h_temp, des_temp, lead_temp, x, y;
            PROJECT* statistic = wxGetApp().GetDocument()->statistic(count);
            PROJECT* state_project = NULL;
            wxString name;
            std::string project_name;

            if (statistic) {
                state_project = pDoc->state.lookup_project(statistic->master_url);
                if (state_project) {
                    state_project->get_name(project_name);
                    name = wxString(project_name.c_str());
                }
            }

            dc.GetTextExtent(name, &w_temp, &h_temp, &des_temp, &lead_temp);

			x=x_start+((x_end-x_start)/2)-(w_temp/2);
			y=y_start+lead_temp+5;
			dc.DrawText (name, x, y);
		}

		//Find minimum/maximum value
		double min_val=10e32, max_val=0;
		for (std::vector<DAILY_STATS>::const_iterator j=(*i)->statistics.begin();
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
			dc.SetPen(wxPen(wxColour (0 , 0 , 0) , 1 , wxSOLID));


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
			for (std::vector<DAILY_STATS>::const_iterator j=(*i)->statistics.begin(); j!=(*i)->statistics.end(); ++j) {
				double day=dday()-j->day;
				day=day/(60*60*24);
				dc.SetPen(wxPen(wxColour (0 , 0 , 0) , 1 , wxSOLID));
				if (j!=(--(*i)->statistics.end())) dc.DrawText(wxString::Format("-%.0f", day),xpos,rectangle_y_end);
				if (j!=(--(*i)->statistics.end()) && j!=(*i)->statistics.begin()) {
					dc.SetPen(wxPen(wxColour (200 , 200 , 200) , 1 , wxSOLID));
					dc.DrawLine(xpos,rectangle_y_start+1,xpos,rectangle_y_end-1);
				}
				xpos+=(rectangle_x_end-rectangle_x_start)/((*i)->statistics.size()-1);
			}
		}

		//Draw graph
		{
			const double yscale=(rectangle_y_end-rectangle_y_start-1)/(max_val-min_val);
			const double xscale=(rectangle_x_end-rectangle_x_start-1)/((*i)->statistics.size()-1);


			dc.SetPen(wxPen(wxColour (255, 255, 0) , 1 , wxSOLID));

			wxCoord last_x=rectangle_x_start, last_y=0, xpos=rectangle_x_start, ypos=0;

			for (std::vector<DAILY_STATS>::const_iterator j=(*i)->statistics.begin(); j!=(*i)->statistics.end(); ++j) {

				ypos=(wxCoord)(rectangle_y_end - 1 - (yscale * (double)(j->user_total_credit-min_val)));
				if (m_SelectedStatistic==0) {
					ypos=(wxCoord)(rectangle_y_end - 1 - (yscale * (double)(j->user_total_credit-min_val)));
				}
				if (m_SelectedStatistic==1) {
					ypos=(wxCoord)(rectangle_y_end - 1 - (yscale * (double)(j->user_expavg_credit-min_val)));
				}
				if (m_SelectedStatistic==2) {
					ypos=(wxCoord)(rectangle_y_end - 1 - (yscale * (double)(j->host_total_credit-min_val)));
				}
				if (m_SelectedStatistic==3) {
					ypos=(wxCoord)(rectangle_y_end - 1 - (yscale * (double)(j->host_expavg_credit-min_val)));
				}
				
				if (last_y!=0) {
					dc.DrawLine(xpos,ypos,last_x,last_y);
				}

				last_x=xpos;
				last_y=ypos;
				xpos+=(wxCoord)xscale;
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

BEGIN_EVENT_TABLE (CViewStatistics, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_STATISTICS_USERTOTAL, CViewStatistics::OnStatisticsUserTotal)
    EVT_BUTTON(ID_TASK_STATISTICS_USERAVERAGE, CViewStatistics::OnStatisticsUserAverage)
    EVT_BUTTON(ID_TASK_STATISTICS_HOSTTOTAL, CViewStatistics::OnStatisticsHostTotal)
    EVT_BUTTON(ID_TASK_STATISTICS_HOSTAVERAGE, CViewStatistics::OnStatisticsHostAverage)
    EVT_LIST_ITEM_SELECTED(ID_LIST_STATISTICSVIEW, CViewStatistics::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_STATISTICSVIEW, CViewStatistics::OnListDeselected)
END_EVENT_TABLE ()


CViewStatistics::CViewStatistics()
{}


CViewStatistics::CViewStatistics(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook) 
{
	CTaskItemGroup* pGroup = NULL;
	CTaskItem*      pItem = NULL;

    //
    // Setup View
    //
    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);
    
    m_pTaskPane = new CBOINCTaskCtrl(this, ID_TASK_STATISTICSVIEW, DEFAULT_TASK_FLAGS);
    wxASSERT(m_pTaskPane);

	m_PaintStatistics = new CPaintStatistics(this, ID_LIST_STATISTICSVIEW, wxDefaultPosition, wxSize(-1, -1), 0);
	wxASSERT(m_PaintStatistics);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_PaintStatistics, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();


	pGroup = new CTaskItemGroup( _("Tasks") );
	m_TaskGroups.push_back( pGroup );

	pItem = new CTaskItem(
        _("Show user total"),
        wxT(""),
        ID_TASK_STATISTICS_USERTOTAL 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Show user average"),
        wxT(""),
        ID_TASK_STATISTICS_USERAVERAGE 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Show host total"),
        wxT(""),
        ID_TASK_STATISTICS_HOSTTOTAL 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Show host average"),
        wxT(""),
        ID_TASK_STATISTICS_HOSTAVERAGE 
    );
    pGroup->m_Tasks.push_back( pItem );


    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    UpdateSelection();
}

CViewStatistics::~CViewStatistics() {
    EmptyTasks();
}


wxString& CViewStatistics::GetViewName() {
    static wxString strViewName(_("Statistics"));
    return strViewName;
}


void CViewStatistics::OnStatisticsUserTotal( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsUserTotal - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->heading=_("User Total");
	m_PaintStatistics->m_SelectedStatistic=0;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsUserTotal - Function End"));
}


void CViewStatistics::OnStatisticsUserAverage( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsUserAverage - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->heading=_("User Average");
	m_PaintStatistics->m_SelectedStatistic=1;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsUserAverage - Function End"));
}


void CViewStatistics::OnStatisticsHostTotal( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsHostTotal - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->heading=_("Host Total");
	m_PaintStatistics->m_SelectedStatistic=2;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsHostTotal - Function End"));
}


void CViewStatistics::OnStatisticsHostAverage( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsHostAverage - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->heading=_("Host Average");
	m_PaintStatistics->m_SelectedStatistic=3;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsHostAverage - Function End"));
}


bool CViewStatistics::OnSaveState(wxConfigBase* pConfig) {
    bool bReturnValue = true;

    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    return bReturnValue;
}


bool CViewStatistics::OnRestoreState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }

    return true;
}


void CViewStatistics::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	if (pDoc->GetStatisticsCount()) {
		m_PaintStatistics->Refresh();
	}
}


void CViewStatistics::UpdateSelection() {
    CBOINCBaseView::UpdateSelection();
}


const char *BOINC_RCSID_7aadb93333 = "$Id$";

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
    EVT_SIZE(CPaintStatistics::OnSize)
END_EVENT_TABLE ()

CPaintStatistics::CPaintStatistics() {
	m_SelectedStatistic=0;
	heading=_("User Total");
	m_ModeViewStatistic=0;
	m_NextProjectStatistic=0;
}

CPaintStatistics::CPaintStatistics(
	wxWindow* parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString& name
): wxPanel(parent, id, pos, size, style, name)
{
	m_SelectedStatistic=0;
	heading=_("User Total");
	m_ModeViewStatistic=0;
	m_NextProjectStatistic=0;
}

static void DrawColour(wxColour &grafColour, wxInt32 numberColour) {
	switch (numberColour %10){
	case 0:	grafColour=wxColour(255,0,0);       //Red
		break;                              
	case 1:	grafColour=wxColour(0,160,0);       //Green
		break;
	case 2:	grafColour=wxColour(0,0,255);       //Blue
		break;
	case 3:	grafColour=wxColour(0,0,0);         //Black
		break;
	case 4:	grafColour=wxColour(255,0,255);     //Fluchsia
		break;
	case 5: grafColour=wxColour(255,128,0);     //
		break;
	case 6:	grafColour=wxColour(192,192,0);     //Olive+
		break;
	case 7:	grafColour=wxColour(0,192,192);     //Teal+
		break;
	case 8:	grafColour=wxColour(160,160,160);   //Gray
		break;
	case 9: grafColour=wxColour(160,0,0);
		break;
	default:grafColour=wxColour(255,255,255);   //White
		break;
	}
}

//----Find minimum/maximum value----
static void MinMaxDayCredit(std::vector<PROJECT*>::const_iterator &i, double &min_credit, double &max_credit, double &min_day, double &max_day, const wxInt32 m_SelectedStatistic) {
	
	for (std::vector<DAILY_STATS>::const_iterator j=(*i)->statistics.begin(); j!=(*i)->statistics.end();++j) {
		if (j->day<min_day) min_day=j->day;
		if (j->day>max_day) max_day=j->day;

		switch (m_SelectedStatistic){ 
		case 0:	if (j->user_total_credit>max_credit) max_credit=j->user_total_credit;
			if (j->user_total_credit<min_credit) min_credit=j->user_total_credit;
			break;
		case 1:	if (j->user_expavg_credit>max_credit) max_credit=j->user_expavg_credit;
			if (j->user_expavg_credit<min_credit) min_credit=j->user_expavg_credit;
			break;
		case 2:	if (j->host_total_credit>max_credit) max_credit=j->host_total_credit;
			if (j->host_total_credit<min_credit) min_credit=j->host_total_credit;
			break;
		case 3:	if (j->host_expavg_credit>max_credit) max_credit=j->host_expavg_credit;
			if (j->host_expavg_credit<min_credit) min_credit=j->host_expavg_credit;
			break;
		}
	}

}
//----Draw graph----
static void DrawGraph(wxPaintDC &dc, std::vector<PROJECT*>::const_iterator &i, const wxCoord rectangle_x_start, const wxCoord rectangle_x_end, const wxCoord rectangle_y_start, const wxCoord rectangle_y_end, const wxColour grafColour, const wxInt32 m_SelectedStatistic, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x) {

	const double yscale=(rectangle_y_end-rectangle_y_start)/(max_val_y-min_val_y);
	const double xscale=(rectangle_x_end-rectangle_x_start)/(max_val_x-min_val_x);

	dc.SetPen(wxPen(grafColour , 2 , wxSOLID));

	wxCoord last_x=rectangle_x_start, last_y=0, xpos=rectangle_x_start, ypos=0;

	for (std::vector<DAILY_STATS>::const_iterator j=(*i)->statistics.begin(); j!=(*i)->statistics.end(); ++j) {

		xpos=(wxCoord)(rectangle_x_start  + (xscale * (j->day-min_val_x)));

		switch (m_SelectedStatistic){ 
		case 0:	ypos=(wxCoord)(rectangle_y_end  - (yscale * (double)(j->user_total_credit-min_val_y)));
			break;
		case 1:	ypos=(wxCoord)(rectangle_y_end  - (yscale * (double)(j->user_expavg_credit-min_val_y)));
			break;
		case 2:	ypos=(wxCoord)(rectangle_y_end  - (yscale * (double)(j->host_total_credit-min_val_y)));
			break;
		case 3:	ypos=(wxCoord)(rectangle_y_end  - (yscale * (double)(j->host_expavg_credit-min_val_y)));
			break;
		default:ypos=(wxCoord)(rectangle_y_end  - (yscale * (double)(j->user_total_credit-min_val_y)));
			break;
		}
		if (last_y!=0) {
			dc.DrawLine(xpos,ypos,last_x,last_y);
		}
		last_x=xpos;
		last_y=ypos;
	}
}

//----Draw background, axis(lines), text(01-Jan-1980)----
static void DrawAxis(wxPaintDC &dc, const wxCoord x_start, const wxCoord x_end, const wxCoord y_start, const wxCoord y_end, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x, const wxString head_name, wxCoord &rectangle_x_start, wxCoord &rectangle_x_end, wxCoord &rectangle_y_start, wxCoord &rectangle_y_end) {

	dc.SetBrush(wxBrush(wxColour (192 , 224 , 255) , wxSOLID));
	dc.SetPen(wxPen(wxColour (64 , 128 , 192) , 1 , wxSOLID));

	wxCoord w_temp, h_temp, des_temp, lead_temp;

	dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
	
	rectangle_y_start=y_start+h_temp+2;

	dc.DrawText (head_name, wxCoord(x_start+((x_end-x_start)/2.0)-(w_temp/2.0)), y_start+1);
	
	dc.GetTextExtent(wxString::Format(" %.1f", max_val_y), &w_temp, &h_temp, &des_temp, &lead_temp);

	rectangle_x_start=x_start+w_temp+2;
	rectangle_y_end=y_end-h_temp-2;
	
	dc.GetTextExtent(" ", &w_temp, &h_temp, &des_temp, &lead_temp);

	rectangle_x_end=x_end-w_temp;
	double radius1=(double)(h_temp/2.0);
	double d_y_start1=(double)(h_temp/2.0);
	
	wxDateTime dtTemp1;
	wxString strBuffer1;
	dtTemp1.Set((time_t)max_val_x);
	strBuffer1=dtTemp1.Format("%d.%b.%y");
	dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);
	
	double d_x_start1=(double)(w_temp/2.0);

// Draw background graph
	dc.DrawRoundedRectangle(rectangle_x_start,rectangle_y_start,rectangle_x_end-rectangle_x_start,rectangle_y_end-rectangle_y_start,radius1);

	rectangle_x_start+=(wxCoord)(d_x_start1);
	rectangle_x_end-=(wxCoord)(d_x_start1);
	rectangle_y_start+=(wxCoord)(d_y_start1);
	rectangle_y_end-=(wxCoord)(d_y_start1);
	
	if (rectangle_x_end<rectangle_x_start){
		rectangle_x_end=(rectangle_x_end+rectangle_x_start)/2;
		rectangle_x_start=rectangle_x_end;
	}
	if (rectangle_y_end<rectangle_y_start){
		rectangle_y_end=(rectangle_y_end+rectangle_y_start)/2;
		rectangle_y_start=rectangle_y_end;
	}

//Draw val and lines
	dc.SetPen(wxPen(wxColour (64 , 128 , 192) , 1 , wxDOT));
	wxInt32 d_oy_count=1;
	d_oy_count=(wxInt32)((rectangle_y_end-rectangle_y_start)/(1.2*h_temp));
	if (d_oy_count>5) d_oy_count=5;
	if (d_oy_count<1) d_oy_count=1;
	
	double d_oy=(double)(rectangle_y_end-rectangle_y_start)/d_oy_count;
	double d_oy_val=(double)(max_val_y-min_val_y)/d_oy_count;

	for (double ny=0; ny<=d_oy_count;++ny){
		dc.GetTextExtent(wxString::Format("%.1f", min_val_y+ny*d_oy_val), &w_temp, &h_temp, &des_temp, &lead_temp);
		dc.DrawText(wxString::Format("%.1f", min_val_y+ny*d_oy_val),(wxCoord)(rectangle_x_start-w_temp-2-d_x_start1),(wxCoord)(rectangle_y_end-ny*d_oy-h_temp/2.0));
		dc.DrawLine((wxCoord)(rectangle_x_start-d_x_start1),(wxCoord)(rectangle_y_end-ny*d_oy),(wxCoord)(rectangle_x_end+d_x_start1),(wxCoord)(rectangle_y_end-ny*d_oy));
	}

//Draw day numbers and lines marking the days
	dtTemp1.Set((time_t)max_val_x);
	strBuffer1=dtTemp1.Format("%d.%b.%y");
	dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);

	wxInt32 d_ox_count=1;
	d_ox_count=(wxInt32)((rectangle_x_end-rectangle_x_start)/(1.2*w_temp));
	if (d_ox_count>5) d_ox_count=5;
	if (d_ox_count<1) d_ox_count=1;
	
	double d_ox=(double)(rectangle_x_end-rectangle_x_start)/d_ox_count;
	double d_ox_val=(double)(max_val_x-min_val_x)/d_ox_count;

	for (double nx=0; nx<=d_ox_count;++nx){
		dtTemp1.Set((time_t)(min_val_x+nx*d_ox_val));
		strBuffer1=dtTemp1.Format("%d.%b.%y");
		dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);
		dc.DrawText(strBuffer1, (wxCoord)(rectangle_x_start-w_temp/2.0+nx*d_ox), (wxCoord)(rectangle_y_end+d_y_start1));
		dc.DrawLine((wxCoord)(rectangle_x_start+nx*d_ox),(wxCoord)(rectangle_y_start-d_y_start1), (wxCoord)(rectangle_x_start+nx*d_ox),(wxCoord)(rectangle_y_end+d_y_start1));
	}
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

	wxCoord width = 0, height = 0, heading_height=0;
	wxCoord rectangle_x_start=0, rectangle_y_start=0;
	wxCoord rectangle_x_end=0, rectangle_y_end=0;

	GetClientSize(&width, &height);

	dc.SetBackground(*wxWHITE_BRUSH);

	dc.SetTextForeground (GetForegroundColour ());
	dc.SetTextBackground (GetBackgroundColour ());

	wxFont heading_font(*wxSWISS_FONT);
	heading_font.SetWeight(wxBOLD);

	dc.SetFont(*wxSWISS_FONT);
	

//Start drawing
	dc.BeginDrawing();

//	dc.Clear();
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(wxPen(wxColour (0 , 0 , 0) , 1 , wxSOLID));
	dc.DrawRectangle(0,0,width, height);
	
//Number of Projects
	wxInt32 nb_proj=0;
	for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin(); i!=proj->projects.end(); ++i) {
	++nb_proj;
	}
	if (nb_proj==0) {
		dc.EndDrawing();
		return;
	}

	switch (m_ModeViewStatistic){
	case 0:{
	//Draw heading
		{
		dc.SetFont(heading_font);
		wxCoord w_temp, h_temp, des_temp, lead_temp;
		dc.GetTextExtent(heading, &w_temp, &h_temp, &des_temp, &lead_temp);
		heading_height=h_temp+2;
		dc.DrawText (heading, ((width/2)-(w_temp/2)), 1);
		dc.SetFont(*wxSWISS_FONT);
		}

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
	
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;

		//Find minimum/maximum value
			double min_val_y=10e32, max_val_y=0;
			double min_val_x=10e32, max_val_x=0;

			MinMaxDayCredit(i, min_val_y,max_val_y,min_val_x, max_val_x, m_SelectedStatistic);

			if (min_val_y<0) min_val_y=0;
			if (max_val_y<0) max_val_y=0;
			if (min_val_y>max_val_y) min_val_y=max_val_y;
			if (max_val_y==min_val_y) max_val_y+=1;

			if (min_val_x<0) min_val_x=0;
			if (max_val_x<0) max_val_x=0;
			if (min_val_x>max_val_x) min_val_x=max_val_x;
			if (max_val_x==min_val_x) max_val_x+=1;

		//Where do we draw in?
			wxCoord x_start=0, y_start=0, x_end=0, y_end=0;
			x_start=(wxCoord)(x_fac*(double)(col-1));
			x_end=(wxCoord)(x_fac*((double)col));
			y_start=(wxCoord)(y_fac*(double)(row-1)+heading_height);
			y_end=(wxCoord)(y_fac*(double)row+heading_height);
		
		///Draw scale Draw Project name
			PROJECT* statistic = wxGetApp().GetDocument()->statistic(count);
			PROJECT* state_project = NULL;
			wxString head_name;
			std::string project_name;
			if (statistic) {
				state_project = pDoc->state.lookup_project(statistic->master_url);
				if (state_project) {
				state_project->get_name(project_name);
				head_name = wxString(project_name.c_str());
				}
			}
			DrawAxis(dc, x_start, x_end, y_start, y_end, max_val_y, min_val_y,max_val_x, min_val_x, head_name, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);

		///Draw graph
			wxColour grafColour=wxColour(0,0,0);

			DrawColour(grafColour,m_SelectedStatistic);
			
			DrawGraph(dc, i, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end, grafColour, m_SelectedStatistic,  max_val_y, min_val_y, max_val_x, min_val_x);
			//Change row/col
			if (col==nb_proj_col) {
				col=1;
				++row;
			} else {
				++col;
			}
		}
		break;
		}
	case 1:{
	//Draw heading
		{
		dc.SetFont(heading_font);
		wxCoord w_temp, h_temp, des_temp, lead_temp;
		dc.GetTextExtent(heading, &w_temp, &h_temp, &des_temp, &lead_temp);
		heading_height=h_temp+2;
		dc.DrawText (heading, ((width/2)-(w_temp/2)), 1);
		dc.SetFont(*wxSWISS_FONT);
		}
	
		if ((m_NextProjectStatistic<0)||(m_NextProjectStatistic>=nb_proj)) m_NextProjectStatistic=0;

		wxInt32 count=-1;
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;
			if (count!=m_NextProjectStatistic) continue;

		//Find minimum/maximum value
			double min_val_y=10e32, max_val_y=0;
			double min_val_x=10e32, max_val_x=0;

			MinMaxDayCredit(i, min_val_y,max_val_y,min_val_x, max_val_x, m_SelectedStatistic);

			if (min_val_y<0) min_val_y=0;
			if (max_val_y<0) max_val_y=0;
			if (min_val_y>max_val_y) min_val_y=max_val_y;
			if (max_val_y==min_val_y) max_val_y+=1;

			if (min_val_x<0) min_val_x=0;
			if (max_val_x<0) max_val_x=0;
			if (min_val_x>max_val_x) min_val_x=max_val_x;
			if (max_val_x==min_val_x) max_val_x+=1;

		//Where do we draw in?
			wxCoord x_start=0, y_start=0, x_end=0, y_end=0;
			x_start=(wxCoord)(0);
			x_end=(wxCoord)(width);
			y_start=(wxCoord)(heading_height);
			y_end=(wxCoord)(height);
		
		///Draw scale Draw Project name
			PROJECT* statistic = wxGetApp().GetDocument()->statistic(count);
			PROJECT* state_project = NULL;
			wxString head_name;
			std::string project_name;
			if (statistic) {
				state_project = pDoc->state.lookup_project(statistic->master_url);
				if (state_project) {
				state_project->get_name(project_name);
				head_name = wxString(project_name.c_str());
				}
			}
			DrawAxis(dc, x_start, x_end, y_start, y_end, max_val_y, min_val_y,max_val_x, min_val_x, head_name, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);

		///Draw graph
			wxColour grafColour=wxColour(0,0,0);
			
			DrawColour(grafColour,m_SelectedStatistic);

			DrawGraph(dc, i, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end, grafColour, m_SelectedStatistic,  max_val_y, min_val_y, max_val_x, min_val_x);
			break;
		}
		break;
		}
	case 2:{
		if ((m_NextProjectStatistic<0)||(m_NextProjectStatistic>=nb_proj)) m_NextProjectStatistic=0;
		
		wxInt32 count=-1;
	
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;
			if (count!=m_NextProjectStatistic) continue;

			for (int m_SelectedStatistic_1=0; m_SelectedStatistic_1<=3;++m_SelectedStatistic_1) {
		//Find minimum/maximum value
			double min_val_y=10e32, max_val_y=0;
			double min_val_x=10e32, max_val_x=0;

			MinMaxDayCredit(i, min_val_y,max_val_y,min_val_x, max_val_x, m_SelectedStatistic_1);

			if (min_val_y<0) min_val_y=0;
			if (max_val_y<0) max_val_y=0;
			if (min_val_y>max_val_y) min_val_y=max_val_y;
			if (max_val_y==min_val_y) max_val_y+=1;

			if (min_val_x<0) min_val_x=0;
			if (max_val_x<0) max_val_x=0;
			if (min_val_x>max_val_x) min_val_x=max_val_x;
			if (max_val_x==min_val_x) max_val_x+=1;

		//Draw heading
			PROJECT* statistic = wxGetApp().GetDocument()->statistic(count);
			PROJECT* state_project = NULL;
			wxString head_name;
			std::string project_name;
			if (statistic) {
				state_project = pDoc->state.lookup_project(statistic->master_url);
				if (state_project) {
				state_project->get_name(project_name);
				head_name = wxString(project_name.c_str());
				}
			}
		//Draw heading
			{
			dc.SetFont(heading_font);
			wxCoord w_temp, h_temp, des_temp, lead_temp;
			dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
			heading_height=h_temp+2;
			dc.DrawText (head_name, ((width/2)-(w_temp/2)), 1);
			dc.SetFont(*wxSWISS_FONT);
			}
			
		//Where do we draw in?
			wxCoord x_start=0, y_start=0, x_end=0, y_end=0;
			switch (m_SelectedStatistic_1){
			case 0:	x_start=(wxCoord)(0);
				x_end=(wxCoord)(width/2.0);
				y_start=(wxCoord)(heading_height);
				y_end=(wxCoord)(heading_height+(height-heading_height)/2.0);
				head_name=_("User Total");
				break;
			case 1:	x_start=(wxCoord)(width/2.0);
				x_end=(wxCoord)(width);
				y_start=(wxCoord)(heading_height);
				y_end=(wxCoord)(heading_height+(height-heading_height)/2.0);
				head_name=_("User Average");
				break;
			case 2:	x_start=(wxCoord)(0);
				x_end=(wxCoord)(width/2.0);
				y_start=(wxCoord)(heading_height+(height-heading_height)/2.0);
				y_end=(wxCoord)(height);
				head_name=_("Host Total");
				break;
			case 3:	x_start=(wxCoord)(width/2.0);
				x_end=(wxCoord)(width);
				y_start=(wxCoord)(heading_height+(height-heading_height)/2.0);
				y_end=(wxCoord)(height);
				head_name=_("Host Average");
				break;
			}
		
		///Draw scale Draw Project name
			DrawAxis(dc, x_start, x_end, y_start, y_end, max_val_y, min_val_y,max_val_x, min_val_x, head_name, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);

		///Draw graph
			wxColour grafColour=wxColour(0,0,0);

			DrawColour(grafColour,m_SelectedStatistic_1);

			DrawGraph(dc, i, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end, grafColour, m_SelectedStatistic_1,  max_val_y, min_val_y, max_val_x, min_val_x);
			}
			break;
		}
		break;
		}
	case 3:{
	//Draw heading
		{
		dc.SetFont(heading_font);
		wxCoord w_temp, h_temp, des_temp, lead_temp;
		dc.GetTextExtent(heading, &w_temp, &h_temp, &des_temp, &lead_temp);
		heading_height=h_temp+2;
		dc.DrawText (heading, ((width/2)-(w_temp/2)), 1);
		dc.SetFont(*wxSWISS_FONT);
		}
		
		wxCoord x_start=0, y_start=0, x_end=0, y_end=0;
		x_start=(wxCoord)(0);
		x_end=(wxCoord)(width);
		y_start=(wxCoord)(heading_height);
		y_end=(wxCoord)(height);
			
	//Find minimum/maximum value
		double min_val_y=10e32, max_val_y=0;
		double min_val_x=10e32, max_val_x=0;
		wxCoord project_name_max_width=0;
		
		wxInt32 count=-1;
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;

			MinMaxDayCredit(i, min_val_y,max_val_y,min_val_x, max_val_x, m_SelectedStatistic);
			
			PROJECT* statistic = wxGetApp().GetDocument()->statistic(count);
			PROJECT* state_project = NULL;
			wxString head_name;
			std::string project_name;
			if (statistic) {
				state_project = pDoc->state.lookup_project(statistic->master_url);
				if (state_project) {
				state_project->get_name(project_name);
				head_name = wxString(project_name.c_str())+" ";
				}
			}
			wxCoord w_temp, h_temp, des_temp, lead_temp;
			dc.SetFont(heading_font);
			dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
			dc.SetFont(*wxSWISS_FONT);
			if (project_name_max_width<w_temp) project_name_max_width=w_temp;
		}
		project_name_max_width+=2;

		if (min_val_y<0) min_val_y=0;
		if (max_val_y<0) max_val_y=0;
		if (min_val_y>max_val_y) min_val_y=max_val_y;
		if (max_val_y==min_val_y) max_val_y+=1;

		if (min_val_x<0) min_val_x=0;
		if (max_val_x<0) max_val_x=0;
		if (min_val_x>max_val_x) min_val_x=max_val_x;
		if (max_val_x==min_val_x) max_val_x+=1;

	///Draw axis
		x_end-=project_name_max_width;

		DrawAxis(dc, x_start, x_end, y_start, y_end, max_val_y, min_val_y,max_val_x, min_val_x, "", rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);

		count=-1;
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;

	///Draw graph
			wxColour grafColour=wxColour(0,0,0);

			DrawColour(grafColour,count);

			DrawGraph(dc, i, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end, grafColour, m_SelectedStatistic,  max_val_y, min_val_y, max_val_x, min_val_x);
			
	///Draw project name
			PROJECT* statistic = wxGetApp().GetDocument()->statistic(count);
			PROJECT* state_project = NULL;
			wxString head_name;
			std::string project_name;
			if (statistic) {
				state_project = pDoc->state.lookup_project(statistic->master_url);
				if (state_project) {
				state_project->get_name(project_name);
				head_name = wxString(project_name.c_str());
				}
			}
			wxCoord w_temp, h_temp, des_temp, lead_temp;
			dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
			wxColour tempColour1;
			tempColour1=GetForegroundColour ();
			dc.SetFont(heading_font);
			dc.SetTextForeground (grafColour);
			dc.DrawText (head_name, x_end, rectangle_y_start+2+wxCoord((double)(count)*1.1*(double)(h_temp)));
			dc.SetTextForeground (tempColour1);
			dc.SetFont(*wxSWISS_FONT);
		}
		break;
		}
	default:{
		m_ModeViewStatistic=0;
		break;
		}
	}
	dc.EndDrawing();
}

void CPaintStatistics::OnSize(wxSizeEvent& event) {
    Refresh(TRUE, NULL);
    event.Skip();
}

IMPLEMENT_DYNAMIC_CLASS(CViewStatistics, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewStatistics, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_STATISTICS_USERTOTAL, CViewStatistics::OnStatisticsUserTotal)
    EVT_BUTTON(ID_TASK_STATISTICS_USERAVERAGE, CViewStatistics::OnStatisticsUserAverage)
    EVT_BUTTON(ID_TASK_STATISTICS_HOSTTOTAL, CViewStatistics::OnStatisticsHostTotal)
    EVT_BUTTON(ID_TASK_STATISTICS_HOSTAVERAGE, CViewStatistics::OnStatisticsHostAverage)
    EVT_BUTTON(ID_TASK_STATISTICS_MODEVIEW, CViewStatistics::OnStatisticsModeView)
    EVT_BUTTON(ID_TASK_STATISTICS_NEXTPROJECT, CViewStatistics::OnStatisticsNextProject)
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

	pGroup = new CTaskItemGroup( _("Mode view") );
	m_TaskGroups.push_back( pGroup );
	pItem = new CTaskItem(
        _("All projects"),
        wxT(""),
        ID_TASK_STATISTICS_MODEVIEW 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Next project"),
        wxT(""),
        ID_TASK_STATISTICS_NEXTPROJECT 
    );
    pGroup->m_Tasks.push_back( pItem );

    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

	m_pTaskPane->DisableTask(pGroup->m_Tasks[1]); /// "Next project" button
   
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

void CViewStatistics::OnStatisticsModeView( wxCommandEvent& WXUNUSED(event) ) {
    CTaskItemGroup*     pGroup0 = NULL;
    CTaskItemGroup*     pGroup1 = NULL;
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsModeView - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pGroup0 = m_TaskGroups[0];
    pGroup1 = m_TaskGroups[1];
    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_ModeViewStatistic++;
	switch(m_PaintStatistics->m_ModeViewStatistic){
	case 1:
		m_pTaskPane->EnableTaskGroupTasks(pGroup0);
		m_pTaskPane->UpdateTask(pGroup1->m_Tasks[0], _("One project"), _(""));
        	m_pTaskPane->EnableTask(pGroup1->m_Tasks[1]);
		break;
	case 2:
	        m_pTaskPane->DisableTaskGroupTasks(pGroup0);
		m_pTaskPane->UpdateTask(pGroup1->m_Tasks[0], _("One project(full)"), _(""));
		m_pTaskPane->EnableTask(pGroup1->m_Tasks[1]);
		break;
	case 3:
	        m_pTaskPane->EnableTaskGroupTasks(pGroup0);
		m_pTaskPane->UpdateTask(pGroup1->m_Tasks[0], _("All projects(sum)"), _(""));
		m_pTaskPane->DisableTask(pGroup1->m_Tasks[1]);
		break;                                  
	default:     
		m_pTaskPane->EnableTaskGroupTasks(pGroup0);
		m_pTaskPane->UpdateTask(pGroup1->m_Tasks[0], _("All projects"), _(""));
        	m_pTaskPane->DisableTask(pGroup1->m_Tasks[1]);
		m_PaintStatistics->m_ModeViewStatistic=0;
		break;
	}
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsModeView - Function End"));
}

void CViewStatistics::OnStatisticsNextProject( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsNextProject - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_NextProjectStatistic++;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsNextProject - Function End"));
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
    CBOINCBaseView::PreUpdateSelection();
    CBOINCBaseView::PostUpdateSelection();
}


const char *BOINC_RCSID_7aadb93333 = "$Id$";

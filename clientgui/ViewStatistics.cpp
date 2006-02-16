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


#include "res/stats.xpm"


BEGIN_EVENT_TABLE (CPaintStatistics, wxPanel)
    EVT_PAINT(CPaintStatistics::OnPaint)
    EVT_SIZE(CPaintStatistics::OnSize)
END_EVENT_TABLE ()

CPaintStatistics::CPaintStatistics() {
	m_SelectedStatistic=0;
	heading=_("User Total");
	m_ModeViewStatistic=0;
	m_NextProjectStatistic=0;
	m_GraphLineWidth=2;
	m_GraphPointWidth=4;
	m_brushAxisColour=wxColour (192 , 224 , 255);
	m_ligthbrushAxisColour=wxColour (220 , 240 , 255);
	m_penAxisColour=wxColour (64 , 128 , 192);
	m_font_stdandart=*wxSWISS_FONT;
	m_font_bold=*wxSWISS_FONT;
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
	m_GraphLineWidth=2;
	m_GraphPointWidth=4;
	m_brushAxisColour=wxColour (192 , 224 , 255);
	m_ligthbrushAxisColour=wxColour (220 , 240 , 255);
	m_penAxisColour=wxColour (64 , 128 , 192);
	m_font_stdandart=*wxSWISS_FONT;
	m_font_bold=*wxSWISS_FONT;
}
static void getTypePoint(wxInt32 &typePoint, wxInt32 number) {
        typePoint=number/10;
}
static void getDrawColour(wxColour &grafColour, wxInt32 number) {
	switch (number %10){
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
//----Draw "Point"
static void myDrawPoint(wxPaintDC &dc,wxCoord X, wxCoord Y, wxColour grafColour,wxInt32 numberTypePoint, wxInt32 PointWidth) {
	dc.SetPen(wxPen(grafColour , 1 , wxSOLID));
	switch (numberTypePoint %4){
	case 1: {wxPoint* points = new wxPoint[3];
		points[0] = wxPoint(X, Y-1-(PointWidth/2));
		points[1] = wxPoint(X+(PointWidth/2), Y+(PointWidth/2));
		points[2] = wxPoint(X-(PointWidth/2), Y+(PointWidth/2));
		dc.DrawPolygon(3, points);
		delete[] points;
		break;}
	case 2: {wxPoint* points = new wxPoint[3];
		points[0] = wxPoint(X, Y+1+(PointWidth/2));
		points[1] = wxPoint(X+(PointWidth/2), Y-(PointWidth/2));
		points[2] = wxPoint(X-(PointWidth/2), Y-(PointWidth/2));
		dc.DrawPolygon(3, points);
		delete[] points;
		break;}
	case 3:	dc.DrawRectangle(X-(PointWidth/2),Y-(PointWidth/2),PointWidth+1,PointWidth+1);
		break;                              
	default:dc.DrawCircle(X,Y,PointWidth/2);
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

static void CheckMinMaxD(double &min_val, double &max_val) {
	if (min_val<0) min_val=0;
	if (max_val<0) max_val=0;
	if (min_val>max_val) min_val=max_val;
	if (max_val==min_val) max_val+=1;
}
static void CheckMinMaxC(wxCoord &min_val, wxCoord &max_val) {
	if (min_val<0) min_val=0;
	if (max_val<0) max_val=0;
	if (min_val>max_val) min_val=max_val;
	if (max_val==min_val) max_val+=1;
}

//----Draw Main Head----
void CPaintStatistics::DrawMainHead(wxPaintDC &dc, const wxString head_name, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord &y_end){
	wxCoord w_temp, h_temp, des_temp, lead_temp;
	dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
	dc.DrawText (head_name, x_start+((x_end-x_start-w_temp)/2), y_start+1);
	y_start+=(h_temp+2);
};

//----Draw Legend----
void CPaintStatistics::DrawLegend(wxPaintDC &dc, PROJECTS * &proj, CMainDocument* &pDoc, wxInt32 SelProj, bool bColour, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord &y_end){
	dc.SetFont(m_font_bold);
	wxString head_name="";
	wxCoord project_name_max_width=0;
	double radius1=1;
	wxInt32 count=-1;
	for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
		++count;
		
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
		if (project_name_max_width<w_temp) project_name_max_width=w_temp;
		radius1=(double)(h_temp/2.0);
	}
	project_name_max_width+=8+4+m_GraphPointWidth; ///+5 - Point
	
	dc.SetBrush(wxBrush(m_ligthbrushAxisColour , wxSOLID));
	dc.SetPen(wxPen(m_penAxisColour , 1 , wxSOLID));
	dc.DrawRoundedRectangle(x_end-project_name_max_width+2,y_start+2,project_name_max_width-4,y_end-y_start-4,radius1);

	x_end-=project_name_max_width;
///---
	count=-1;
	for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
		++count;
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
		if (SelProj==count){
			dc.SetBrush(wxBrush(m_brushAxisColour , wxSOLID));
			dc.SetPen(wxPen(m_penAxisColour , 1 , wxSOLID));
			dc.DrawRectangle(x_end+1,y_start+1+wxCoord(((double)(count)+0.5)*1.1*(double)(h_temp)),project_name_max_width-2,h_temp+2);
		}

		wxColour tempColour1=wxColour(0,0,0);
		wxColour grafColour=wxColour(0,0,0);
		wxInt32  typePoint=0;
		tempColour1=GetForegroundColour ();
		if (bColour){
			getTypePoint(typePoint,count);
			getDrawColour(grafColour,count);
		}
		dc.SetTextForeground (grafColour);
		dc.SetBrush(wxBrush(m_ligthbrushAxisColour , wxSOLID));
		myDrawPoint(dc, x_end+4+1+m_GraphPointWidth/2, y_start+2+wxCoord(((double)(count)+1)*1.1*(double)(h_temp)), grafColour, typePoint ,m_GraphPointWidth);
		dc.DrawText (head_name, x_end+4+4+m_GraphPointWidth, y_start+2+wxCoord(((double)(count)+0.5)*1.1*(double)(h_temp)));
		dc.SetTextForeground (tempColour1);
	}
	dc.SetFont(m_font_stdandart);
};

//----Draw graph----
void CPaintStatistics::DrawGraph(wxPaintDC &dc, std::vector<PROJECT*>::const_iterator &i, const wxCoord x_start, const wxCoord x_end, const wxCoord y_start, const wxCoord y_end, const wxColour grafColour, const wxInt32 typePoint, const wxInt32 m_SelectedStatistic, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x) {

	const double yscale=(y_end-y_start)/(max_val_y-min_val_y);
	const double xscale=(x_end-x_start)/(max_val_x-min_val_x);

	dc.SetPen(wxPen(grafColour , m_GraphLineWidth , wxSOLID));

	wxCoord last_x=x_start, last_y=0, xpos=x_start, ypos=0;

	for (std::vector<DAILY_STATS>::const_iterator j=(*i)->statistics.begin(); j!=(*i)->statistics.end(); ++j) {

		xpos=(wxCoord)(x_start  + (xscale * (j->day-min_val_x)));

		switch (m_SelectedStatistic){ 
		case 0:	ypos=(wxCoord)(y_end  - (yscale * (double)(j->user_total_credit-min_val_y)));
			break;
		case 1:	ypos=(wxCoord)(y_end  - (yscale * (double)(j->user_expavg_credit-min_val_y)));
			break;
		case 2:	ypos=(wxCoord)(y_end  - (yscale * (double)(j->host_total_credit-min_val_y)));
			break;
		case 3:	ypos=(wxCoord)(y_end  - (yscale * (double)(j->host_expavg_credit-min_val_y)));
			break;
		default:ypos=(wxCoord)(y_end  - (yscale * (double)(j->user_total_credit-min_val_y)));
			break;
		}
		if (last_y!=0) {
			dc.SetPen(wxPen(grafColour , m_GraphLineWidth , wxSOLID));
			dc.DrawLine(last_x,last_y,xpos,ypos);
			myDrawPoint(dc, last_x, last_y, grafColour, typePoint ,m_GraphPointWidth);
		}
		last_x=xpos;
		last_y=ypos;
	}
	if (last_y!=0) myDrawPoint(dc, last_x, last_y, grafColour, typePoint ,m_GraphPointWidth);
}

//----Draw background, axis(lines), text(01-Jan-1980)----
void CPaintStatistics::DrawAxis(wxPaintDC &dc, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord &y_end) {

	dc.SetBrush(wxBrush(m_brushAxisColour , wxSOLID));
	dc.SetPen(wxPen(m_penAxisColour , 1 , wxSOLID));

	wxCoord w_temp, h_temp, des_temp, lead_temp;
	
	dc.GetTextExtent(wxString::Format(" %.1f", max_val_y), &w_temp, &h_temp, &des_temp, &lead_temp);

	x_start+=(w_temp+2);
	y_end-=(h_temp+2);
	
	dc.GetTextExtent(" ", &w_temp, &h_temp, &des_temp, &lead_temp);

	x_end-=w_temp;
	double radius1=(double)(h_temp/2.0);
	double d_y=(double)(h_temp/2.0);
	
	wxDateTime dtTemp1;
	wxString strBuffer1;
	dtTemp1.Set((time_t)max_val_x);
	strBuffer1=dtTemp1.Format("%d.%b.%y");
	dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);
	
	double d_x=(double)(w_temp/2.0);

// Draw background graph
	dc.DrawRoundedRectangle(x_start,y_start,x_end-x_start,y_end-y_start,radius1);

	x_start+=(wxCoord)(d_x);
	x_end-=(wxCoord)(d_x);
	y_start+=(wxCoord)(d_y);
	y_end-=(wxCoord)(d_y);
	
	if (x_end<x_start) x_start=x_end=(x_end+x_start)/2;
	if (y_end<y_start) y_start=y_end=(y_end+y_start)/2;

//Draw val and lines
	dc.SetPen(wxPen(wxColour (64 , 128 , 192) , 1 , wxDOT));
	wxInt32 d_oy_count=1;
	d_oy_count=(wxInt32)((y_end-y_start)/(1.2*h_temp));
	if (d_oy_count>9) d_oy_count=9;
	if (d_oy_count<1) d_oy_count=1;
	
	double d_oy=(double)(y_end-y_start)/d_oy_count;
	double d_oy_val=(double)(max_val_y-min_val_y)/d_oy_count;

	for (double ny=0; ny<=d_oy_count;++ny){
		dc.GetTextExtent(wxString::Format("%.1f", min_val_y+ny*d_oy_val), &w_temp, &h_temp, &des_temp, &lead_temp);
		dc.DrawText(wxString::Format("%.1f", min_val_y+ny*d_oy_val),(wxCoord)(x_start-w_temp-2-d_x),(wxCoord)(y_end-ny*d_oy-h_temp/2.0));
		dc.DrawLine((wxCoord)(x_start-d_x+1),(wxCoord)(y_end-ny*d_oy),(wxCoord)(x_end+d_x),(wxCoord)(y_end-ny*d_oy));
	}

//Draw day numbers and lines marking the days
	dtTemp1.Set((time_t)max_val_x);
	strBuffer1=dtTemp1.Format("%d.%b.%y");
	dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);

	wxInt32 d_ox_count=1;
	d_ox_count=(wxInt32)((x_end-x_start)/(1.2*w_temp));
	if (d_ox_count>9) d_ox_count=9;
	if (d_ox_count<1) d_ox_count=1;
	
	double d_ox=(double)(x_end-x_start)/d_ox_count;
	double d_ox_val=(double)(max_val_x-min_val_x)/d_ox_count;

	for (double nx=0; nx<=d_ox_count;++nx){
		dtTemp1.Set((time_t)(min_val_x+nx*d_ox_val));
		strBuffer1=dtTemp1.Format("%d.%b.%y");
		dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);
		dc.DrawText(strBuffer1, (wxCoord)(x_start-w_temp/2.0+nx*d_ox), (wxCoord)(y_end+d_y));
		dc.DrawLine((wxCoord)(x_start+nx*d_ox),(wxCoord)(y_start-d_y+1), (wxCoord)(x_start+nx*d_ox),(wxCoord)(y_end+d_y));
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

	wxCoord width = 0, height = 0;
	wxCoord rectangle_x_start=0, rectangle_y_start=0;
	wxCoord rectangle_x_end=0, rectangle_y_end=0;

	GetClientSize(&width, &height);

	dc.SetBackground(*wxWHITE_BRUSH);
	
	dc.SetTextForeground (GetForegroundColour ());
	dc.SetTextBackground (GetBackgroundColour ());

	m_font_stdandart=dc.GetFont();
	m_font_bold=dc.GetFont();
	
	m_font_stdandart.SetWeight(wxNORMAL);
	m_font_bold.SetWeight(wxBOLD);

	dc.SetFont(m_font_stdandart);

//	dc.SetFont(*wxSWISS_FONT);

//Start drawing
	dc.BeginDrawing();
	dc.Clear();
//Number of Projects
	wxInt32 nb_proj=0;
	for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin(); i!=proj->projects.end(); ++i) {
	++nb_proj;
	}
	if (nb_proj==0) {
		dc.EndDrawing();
		return;
	}
// Check m_NextProjectStatistic
	if (m_NextProjectStatistic<0) m_NextProjectStatistic=nb_proj-1;
	if ((m_NextProjectStatistic<0)||(m_NextProjectStatistic>=nb_proj)) m_NextProjectStatistic=0;
// Initial coord
	rectangle_x_start=0;
	rectangle_x_end=width;
	rectangle_y_start=0;
	rectangle_y_end=height;

	switch (m_ModeViewStatistic){
	case 0:{
	///Draw heading
		CheckMinMaxC(rectangle_x_start, rectangle_x_end);
		CheckMinMaxC(rectangle_y_start, rectangle_y_end);
		DrawMainHead(dc, heading, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
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
	
		const double x_fac=(rectangle_x_end-rectangle_x_start)/nb_proj_col;
		const double y_fac=(rectangle_y_end-rectangle_y_start)/nb_proj_row;
	
		wxInt32 count=-1;
	
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;
		//Find minimum/maximum value
			double min_val_y=10e32, max_val_y=0;
			double min_val_x=10e32, max_val_x=0;
			MinMaxDayCredit(i, min_val_y,max_val_y,min_val_x, max_val_x, m_SelectedStatistic);
			CheckMinMaxD(min_val_x, max_val_x);
			CheckMinMaxD(min_val_y, max_val_y);
		//Where do we draw in?
			wxCoord x_start=0, y_start=0, x_end=0, y_end=0;
			x_start=(wxCoord)(rectangle_x_start+x_fac*(double)(col-1));
			x_end=(wxCoord)(rectangle_x_start+x_fac*((double)col));
			y_start=(wxCoord)(rectangle_y_start+y_fac*(double)(row-1));
			y_end=(wxCoord)(rectangle_y_start+y_fac*(double)row);
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
		///Draw heading
			CheckMinMaxC(x_start, x_end);
			CheckMinMaxC(y_start, y_end);
			DrawMainHead(dc, head_name, x_start, x_end, y_start, y_end);
		///Draw axis
			CheckMinMaxC(x_start, x_end);
			CheckMinMaxC(y_start, y_end);
			DrawAxis(dc, max_val_y, min_val_y,max_val_x, min_val_x, x_start, x_end, y_start, y_end);
		///Draw graph
			wxColour grafColour=wxColour(0,0,0);
			getDrawColour(grafColour,m_SelectedStatistic);
			CheckMinMaxC(x_start, x_end);
			CheckMinMaxC(y_start, y_end);
			DrawGraph(dc, i, x_start, x_end, y_start, y_end, grafColour, 0, m_SelectedStatistic,  max_val_y, min_val_y, max_val_x, min_val_x);
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
	///Draw Legend
		CheckMinMaxC(rectangle_x_start, rectangle_x_end);
		CheckMinMaxC(rectangle_y_start, rectangle_y_end);
		DrawLegend(dc, proj, pDoc, m_NextProjectStatistic, false, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
	///Draw heading
		CheckMinMaxC(rectangle_x_start, rectangle_x_end);
		CheckMinMaxC(rectangle_y_start, rectangle_y_end);
		DrawMainHead(dc, heading, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
	//Draw project
		wxInt32 count=-1;
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;
			if (count!=m_NextProjectStatistic) continue;

		///Find minimum/maximum value
			double min_val_y=10e32, max_val_y=0;
			double min_val_x=10e32, max_val_x=0;

			MinMaxDayCredit(i, min_val_y,max_val_y,min_val_x, max_val_x, m_SelectedStatistic);

			CheckMinMaxD(min_val_x, max_val_x);
			CheckMinMaxD(min_val_y, max_val_y);
		///Draw axis + Draw Project name
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
		///Draw heading
			CheckMinMaxC(rectangle_x_start, rectangle_x_end);
			CheckMinMaxC(rectangle_y_start, rectangle_y_end);
			DrawMainHead(dc, head_name, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
		///Draw axis
			CheckMinMaxC(rectangle_x_start, rectangle_x_end);
			CheckMinMaxC(rectangle_y_start, rectangle_y_end);
			DrawAxis(dc, max_val_y, min_val_y,max_val_x, min_val_x, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
		///Draw graph
			wxColour grafColour=wxColour(0,0,0);
			getDrawColour(grafColour,m_SelectedStatistic);
			CheckMinMaxC(rectangle_x_start, rectangle_x_end);
			CheckMinMaxC(rectangle_y_start, rectangle_y_end);
			DrawGraph(dc, i, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end, grafColour, 0, m_SelectedStatistic,  max_val_y, min_val_y, max_val_x, min_val_x);
			break;
		}
		break;
		}
	case 2:{
	///Draw Legend
		CheckMinMaxC(rectangle_x_start, rectangle_x_end);
		CheckMinMaxC(rectangle_y_start, rectangle_y_end);
		DrawLegend(dc, proj, pDoc, -1, true, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
	///Draw heading
		CheckMinMaxC(rectangle_x_start, rectangle_x_end);
		CheckMinMaxC(rectangle_y_start, rectangle_y_end);
		DrawMainHead(dc, heading, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
	//Find minimum/maximum value
		double min_val_y=10e32, max_val_y=0;
		double min_val_x=10e32, max_val_x=0;
		
		wxInt32 count=-1;
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;
			MinMaxDayCredit(i, min_val_y,max_val_y,min_val_x, max_val_x, m_SelectedStatistic);
		}
		CheckMinMaxD(min_val_x, max_val_x);
		CheckMinMaxD(min_val_y, max_val_y);
	///Draw axis
		CheckMinMaxC(rectangle_x_start, rectangle_x_end);
		CheckMinMaxC(rectangle_y_start, rectangle_y_end);
		DrawAxis(dc, max_val_y, min_val_y,max_val_x, min_val_x, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);

		count=-1;
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;
	///Draw graph
			wxColour grafColour=wxColour(0,0,0);
			wxInt32  typePoint=0;
			getTypePoint(typePoint,count);
			getDrawColour(grafColour,count);
			CheckMinMaxC(rectangle_x_start, rectangle_x_end);
			CheckMinMaxC(rectangle_y_start, rectangle_y_end);
			DrawGraph(dc, i, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end, grafColour, typePoint, m_SelectedStatistic,  max_val_y, min_val_y, max_val_x, min_val_x);
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
    EVT_BUTTON(ID_TASK_STATISTICS_MODEVIEW0, CViewStatistics::OnStatisticsModeView0)
    EVT_BUTTON(ID_TASK_STATISTICS_MODEVIEW1, CViewStatistics::OnStatisticsModeView1)
    EVT_BUTTON(ID_TASK_STATISTICS_MODEVIEW2, CViewStatistics::OnStatisticsModeView2)
    EVT_BUTTON(ID_TASK_STATISTICS_NEXTPROJECT, CViewStatistics::OnStatisticsNextProject)
    EVT_BUTTON(ID_TASK_STATISTICS_PREVPROJECT, CViewStatistics::OnStatisticsPrevProject)
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

	pGroup = new CTaskItemGroup( _("Commands") );
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

	pGroup = new CTaskItemGroup( _("Project") );
	m_TaskGroups.push_back( pGroup );

	pItem = new CTaskItem(
        _("< &Back"),
        wxT(""),
        ID_TASK_STATISTICS_PREVPROJECT 
    );
    pGroup->m_Tasks.push_back( pItem );
	pItem = new CTaskItem(
        _("&Next >"),
        wxT(""),
        ID_TASK_STATISTICS_NEXTPROJECT 
    );
    pGroup->m_Tasks.push_back( pItem );

	pGroup = new CTaskItemGroup( _("Mode view") );
	m_TaskGroups.push_back( pGroup );
	pItem = new CTaskItem(
        _("All projects"),
        wxT(""),
        ID_TASK_STATISTICS_MODEVIEW0 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("One project"),
        wxT(""),
        ID_TASK_STATISTICS_MODEVIEW1 
    );
    pGroup->m_Tasks.push_back( pItem );
        
	pItem = new CTaskItem(
        _("All projects(sum)"),
        wxT(""),
        ID_TASK_STATISTICS_MODEVIEW2 
    );
    pGroup->m_Tasks.push_back( pItem );
    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

	m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]); /// "project" button
     
    UpdateSelection();
}

CViewStatistics::~CViewStatistics() {
    EmptyTasks();
}

wxString& CViewStatistics::GetViewName() {
    static wxString strViewName(_("Statistics"));
    return strViewName;
}

const char** CViewStatistics::GetViewIcon() {
    return stats_xpm;
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

void CViewStatistics::OnStatisticsModeView0( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsModeView - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_ModeViewStatistic=0;
	m_pTaskPane->EnableTaskGroupTasks(m_TaskGroups[0]);
        m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]);
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsModeView - Function End"));
}

void CViewStatistics::OnStatisticsModeView1( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsModeView - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_ModeViewStatistic=1;
	m_pTaskPane->EnableTaskGroupTasks(m_TaskGroups[0]);
	m_pTaskPane->EnableTaskGroupTasks(m_TaskGroups[1]);
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsModeView - Function End"));
}

void CViewStatistics::OnStatisticsModeView2( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsModeView - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_ModeViewStatistic=2;
	m_pTaskPane->EnableTaskGroupTasks(m_TaskGroups[0]);
	m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]);
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

void CViewStatistics::OnStatisticsPrevProject( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsPrevProject - Function Begin"));

    CMainFrame* pFrame      = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CMainFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_NextProjectStatistic--;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsPrevProject - Function End"));
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

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
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewStatistics.h"
#include "Events.h"
#include "util.h"


#include "res/stats.xpm"


BEGIN_EVENT_TABLE (CPaintStatistics, wxPanel)
    EVT_PAINT(CPaintStatistics::OnPaint)
    EVT_SIZE(CPaintStatistics::OnSize)
    EVT_LEFT_DOWN(CPaintStatistics::OnLeftMouseDown)
END_EVENT_TABLE ()

CPaintStatistics::CPaintStatistics() {
	m_SelectedStatistic=0;
	heading=wxT("");
	m_ModeViewStatistic=0;
	m_NextProjectStatistic=0;
	m_GraphLineWidth=2;
	m_GraphPointWidth=4;

	m_Legend_dY=0;
	m_Legend_Y1=0;
	m_Legend_Y2=0;
	m_Legend_X1=0;
	m_Legend_X2=0;

	m_brush_AxisColour=wxColour (192 , 224 , 255);
	m_pen_AxisColour=wxColour (64 , 128 , 192);
	m_pen_AxisXColour=wxColour (64 , 128 , 192);
	m_pen_AxisYColour=wxColour (64 , 128 , 192);
	m_pen_AxisXTextColour=wxColour (0 , 0 , 0);
	m_pen_AxisYTextColour=wxColour (0 , 0 , 0);      
	                            
	m_brush_LegendColour=wxColour (220 , 240 , 255);
	m_brush_LegendSelectColour=wxColour (192 , 224 , 255);
	m_pen_LegendSelectColour=wxColour (64 , 128 , 192);
	m_pen_LegendSelectTextColour=wxColour (0 , 0 , 0);
	m_pen_LegendColour=wxColour (64 , 128 , 192);
	m_pen_LegendTextColour=wxColour (0 , 0 , 0);
	                            
	m_brush_MainColour=wxColour (255 , 255 , 255);
	m_pen_MainColour=wxColour (64 , 128 , 192);
	                            
	m_pen_HeadTextColour=wxColour (0 , 0 , 0);
	m_pen_ProjectHeadTextColour=wxColour (0 , 0 , 0);
	                            
	m_pen_GraphTotalColour=wxColour(255,0,0);
	m_pen_GraphRACColour=wxColour(0,160,0);
	m_pen_GraphTotalHostColour=wxColour(0,0,255);
	m_pen_GraphRACHostColour=wxColour(0,0,0);
	                            
	m_pen_GraphColour00=wxColour(255,0,0);
	m_pen_GraphColour01=wxColour(0,160,0);
	m_pen_GraphColour02=wxColour(0,0,255);
	m_pen_GraphColour03=wxColour(0,0,0);
	m_pen_GraphColour04=wxColour(255,0,255);
	m_pen_GraphColour05=wxColour(255,128,0);
	m_pen_GraphColour06=wxColour(192,192,0);
	m_pen_GraphColour07=wxColour(0,192,192);
	m_pen_GraphColour08=wxColour(160,160,160);
	m_pen_GraphColour09=wxColour(160,0,0);
	m_pen_GraphColourDef=wxColour(255,255,255);

	m_font_standart=*wxSWISS_FONT;
	m_font_bold=*wxSWISS_FONT;
	m_font_standart_italic=*wxSWISS_FONT;
}

CPaintStatistics::CPaintStatistics(
	wxWindow* parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString& name
): wxPanel(parent, id, pos, size, style, name)
{
	m_SelectedStatistic=0;
	heading=wxT("");
	m_ModeViewStatistic=0;
	m_NextProjectStatistic=0;
	m_GraphLineWidth=2;
	m_GraphPointWidth=4;

	m_Legend_dY=0;
	m_Legend_Y1=0;
	m_Legend_Y2=0;
	m_Legend_X1=0;
	m_Legend_X2=0;

	m_brush_AxisColour=wxColour (192 , 224 , 255);
	m_pen_AxisColour=wxColour (64 , 128 , 192);
	m_pen_AxisXColour=wxColour (64 , 128 , 192);
	m_pen_AxisYColour=wxColour (64 , 128 , 192);
	m_pen_AxisXTextColour=wxColour (0 , 0 , 0);
	m_pen_AxisYTextColour=wxColour (0 , 0 , 0);      
	                            
	m_brush_LegendColour=wxColour (220 , 240 , 255);
	m_brush_LegendSelectColour=wxColour (192 , 224 , 255);
	m_pen_LegendSelectColour=wxColour (64 , 128 , 192);
	m_pen_LegendSelectTextColour=wxColour (0 , 0 , 0);
	m_pen_LegendColour=wxColour (64 , 128 , 192);
	m_pen_LegendTextColour=wxColour (0 , 0 , 0);
	                            
	m_brush_MainColour=wxColour (255 , 255 , 255);
	m_pen_MainColour=wxColour (64 , 128 , 192);
	                            
	m_pen_HeadTextColour=wxColour (0 , 0 , 0);
	m_pen_ProjectHeadTextColour=wxColour (0 , 0 , 0);
	                            
	m_pen_GraphTotalColour=wxColour(255,0,0);
	m_pen_GraphRACColour=wxColour(0,160,0);
	m_pen_GraphTotalHostColour=wxColour(0,0,255);
	m_pen_GraphRACHostColour=wxColour(0,0,0);
	                            
	m_pen_GraphColour00=wxColour(255,0,0);
	m_pen_GraphColour01=wxColour(0,160,0);
	m_pen_GraphColour02=wxColour(0,0,255);
	m_pen_GraphColour03=wxColour(0,0,0);
	m_pen_GraphColour04=wxColour(255,0,255);
	m_pen_GraphColour05=wxColour(255,128,0);
	m_pen_GraphColour06=wxColour(192,192,0);
	m_pen_GraphColour07=wxColour(0,192,192);
	m_pen_GraphColour08=wxColour(160,160,160);
	m_pen_GraphColour09=wxColour(160,0,0);
	m_pen_GraphColourDef=wxColour(255,255,255);

	m_font_standart=*wxSWISS_FONT;
	m_font_bold=*wxSWISS_FONT;
	m_font_standart_italic=*wxSWISS_FONT;
}
static void getTypePoint(wxInt32 &typePoint, wxInt32 number) {
        typePoint=number/10;
}
void CPaintStatistics::getDrawColour(wxColour &graphColour, wxInt32 number) {
	switch (number %10){
	case 0:	graphColour=m_pen_GraphColour00;
		break;                              
	case 1:	graphColour=m_pen_GraphColour01;
		break;
	case 2:	graphColour=m_pen_GraphColour02;
		break;
	case 3:	graphColour=m_pen_GraphColour03;
		break;
	case 4:	graphColour=m_pen_GraphColour04;
		break;
	case 5: graphColour=m_pen_GraphColour05;
		break;
	case 6:	graphColour=m_pen_GraphColour06;
		break;
	case 7:	graphColour=m_pen_GraphColour07;
		break;
	case 8:	graphColour=m_pen_GraphColour08;
		break;
	case 9: graphColour=m_pen_GraphColour09;
		break;
	default:graphColour=m_pen_GraphColourDef;
		break;
	}
}
//----Draw "Point"
static void myDrawPoint(wxBufferedPaintDC &dc,wxCoord X, wxCoord Y, wxColour graphColour,wxInt32 numberTypePoint, wxInt32 PointWidth) {
	dc.SetPen(wxPen(graphColour , 1 , wxSOLID));
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
void CPaintStatistics::DrawMainHead(wxBufferedPaintDC &dc, const wxString head_name, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord & /*y_end*/){
	wxCoord w_temp, h_temp, des_temp, lead_temp;
	dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
	dc.SetTextForeground (m_pen_HeadTextColour);
	dc.DrawText (head_name, x_start+((x_end-x_start-w_temp)/2), y_start+1);
	y_start+=(h_temp+2);
};
//----Draw Project Head----
void CPaintStatistics::DrawProjectHead(wxBufferedPaintDC &dc, PROJECT * &project1, const wxString head_name_last, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord &y_end){
	wxCoord w_temp, h_temp, des_temp, lead_temp;
	wxString head_name=wxT("");

	if (project1) {
	    head_name = wxString(_("Project"))+wxT(": ")+wxString(project1->project_name.c_str(), wxConvUTF8);
	    dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
	    dc.DrawText (head_name, x_start+((x_end-x_start-w_temp)/2), y_start+1);
	    y_start+=(h_temp+2);

	    head_name = wxString(_("Account"))+wxT(": ")+wxString(project1->user_name.c_str(), wxConvUTF8);
	    dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
	    dc.DrawText (head_name, x_start+((x_end-x_start-w_temp)/2), y_start+1);
	    y_start+=(h_temp+2);

	    head_name = wxString(_("Team"))+wxT(": ")+wxString(project1->team_name.c_str(), wxConvUTF8);
	    dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
	    dc.DrawText (head_name, x_start+((x_end-x_start-w_temp)/2), y_start+1);
	    y_start+=(h_temp+2);

	    dc.GetTextExtent(head_name_last, &w_temp, &h_temp, &des_temp, &lead_temp);
	    dc.DrawText (head_name_last, x_start+((x_end-x_start-w_temp)/2), y_start+1);
	    y_start+=(h_temp+2);
	}
};

//----Draw Legend----
void CPaintStatistics::DrawLegend(wxBufferedPaintDC &dc, PROJECTS * &proj, CMainDocument* &pDoc, wxInt32 SelProj, bool bColour, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord &y_end){
	dc.SetFont(m_font_bold);
	wxString head_name=wxT("0");
	wxCoord project_name_max_width=0;
	const double radius1=5;
	wxCoord buffer_y1=3;
	wxCoord buffer_x1=3;
	wxInt32 count=-1;
	wxCoord w_temp=0, h_temp=0, des_temp=0, lead_temp=0;

	dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
	m_Legend_dY=(double)(h_temp+4);

	for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
		++count;
		
		PROJECT* state_project = pDoc->state.lookup_project((*i)->master_url);
		if (state_project) {
		    head_name = wxString(state_project->project_name.c_str(), wxConvUTF8);
		}

		dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
		if (project_name_max_width<w_temp) project_name_max_width=w_temp;
	}
	project_name_max_width+=8+buffer_x1+buffer_x1+wxCoord(m_GraphPointWidth);
	
	dc.SetBrush(wxBrush(m_brush_LegendColour , wxSOLID));
	dc.SetPen(wxPen(m_pen_LegendColour , 1 , wxSOLID));
	dc.DrawRoundedRectangle(x_end-project_name_max_width+buffer_x1,y_start+buffer_y1,project_name_max_width-buffer_x1-buffer_x1,y_end-y_start-buffer_y1-buffer_y1,radius1);

	x_end-=project_name_max_width;
///---
	m_Legend_Y1=double(y_start+buffer_y1)+radius1;
	m_Legend_X1=double(x_end+buffer_x1);
	m_Legend_Y2=double(y_end-buffer_y1)-radius1;
	m_Legend_X2=double(x_end+project_name_max_width-buffer_x1);
	count=-1;

	for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
		++count;
	///Draw project name
		head_name = wxT("?");
		PROJECT* state_project = pDoc->state.lookup_project((*i)->master_url);
		if (state_project) {
			head_name = wxString(state_project->project_name.c_str(), wxConvUTF8);
		}

		if (SelProj==count){
			dc.SetBrush(wxBrush(m_brush_LegendSelectColour , wxSOLID));
			dc.SetPen(wxPen(m_pen_LegendSelectColour , 1 , wxSOLID));
			dc.DrawRoundedRectangle(x_end+buffer_x1-1,y_start+wxCoord((double)(count)*m_Legend_dY+double(buffer_y1)+radius1),project_name_max_width-buffer_x1-buffer_x1+2,wxCoord(m_Legend_dY),1);
		}

		wxColour graphColour=wxColour(0,0,0);
		wxInt32  typePoint=0;
		if (bColour){
			getTypePoint(typePoint,count);
			getDrawColour(graphColour,count);
		} else if (SelProj==count) {
				graphColour=m_pen_LegendSelectTextColour;
			} else {
				graphColour=m_pen_LegendTextColour;
			}

		dc.SetTextForeground(graphColour);
		dc.SetBrush(wxBrush(m_brush_LegendColour , wxSOLID));
		myDrawPoint(dc, x_end+buffer_x1+4+wxCoord(m_GraphPointWidth)/2, y_start+wxCoord(((double)(count)+0.5)*m_Legend_dY+double(buffer_y1)+radius1), graphColour, typePoint ,m_GraphPointWidth);
		dc.DrawText (head_name, x_end+buffer_x1+7+wxCoord(m_GraphPointWidth), y_start+1+wxCoord((double)(count)*m_Legend_dY+double(buffer_y1)+radius1));
		m_Legend_Y2=(double)y_start+(double)(count+1)*m_Legend_dY+double(buffer_y1)+radius1;
		if ((m_Legend_Y2+m_Legend_dY)>((double)y_end-double(buffer_y1)-radius1))break;
	}
	dc.SetFont(m_font_standart);
};

//----Draw graph----
void CPaintStatistics::DrawGraph(wxBufferedPaintDC &dc, std::vector<PROJECT*>::const_iterator &i, const wxCoord x_start, const wxCoord x_end, const wxCoord y_start, const wxCoord y_end, const wxColour graphColour, const wxInt32 typePoint, const wxInt32 m_SelectedStatistic, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x) {

	dc.SetClippingRegion(x_start-4, y_start-4, x_end-x_start+8, y_end-y_start+8);

	const double yscale=(y_end-y_start)/(max_val_y-min_val_y);
	const double xscale=(x_end-x_start)/(max_val_x-min_val_x);

	dc.SetPen(wxPen(graphColour , m_GraphLineWidth , wxSOLID));

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
			dc.SetPen(wxPen(graphColour , m_GraphLineWidth , wxSOLID));
			dc.DrawLine(last_x,last_y,xpos,ypos);
			myDrawPoint(dc, last_x, last_y, graphColour, typePoint ,m_GraphPointWidth);
		}
		last_x=xpos;
		last_y=ypos;
	}
	if (last_y!=0) myDrawPoint(dc, last_x, last_y, graphColour, typePoint ,m_GraphPointWidth);

	dc.DestroyClippingRegion();
}

//----Draw background, axis(lines), text(01-Jan-1980)----
void CPaintStatistics::DrawAxis(wxBufferedPaintDC &dc, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord &y_end) {

        dc.SetClippingRegion(x_start, y_start, x_end-x_start, y_end-y_start);

	dc.SetBrush(wxBrush(m_brush_AxisColour , wxSOLID));
	dc.SetPen(wxPen(m_pen_AxisColour , 1 , wxSOLID));

	wxCoord w_temp, h_temp, des_temp, lead_temp;
	
	dc.GetTextExtent(wxString::Format(wxT(" %.1f"), max_val_y), &w_temp, &h_temp, &des_temp, &lead_temp);

	x_start+=(w_temp+3);
	y_end-=(h_temp+3);
	
	dc.GetTextExtent(wxT("0"), &w_temp, &h_temp, &des_temp, &lead_temp);

	x_end-=3;//w_temp;
	const double radius1=5;//(double)(h_temp/2.0);
	double d_y=(double)(h_temp/2.0);
	if (d_y<5)d_y=5;
	
	wxDateTime dtTemp1;
	wxString strBuffer1;
	dtTemp1.Set((time_t)max_val_x);
	strBuffer1=dtTemp1.Format(wxT("%d.%b.%y"));
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

	const double yscale=(y_end-y_start)/(max_val_y-min_val_y);
	const double xscale=(x_end-x_start)/(max_val_x-min_val_x);

//Draw val and lines
	dc.SetPen(wxPen(m_pen_AxisYColour , 1 , wxDOT));
	dc.SetTextForeground (m_pen_AxisYTextColour);

	wxInt32 d_oy_count=1;
	d_oy_count=(wxInt32)ceil((y_end-y_start)/(2.0*h_temp));

//	if (d_oy_count>9) d_oy_count=9;
//	if (d_oy_count<1) d_oy_count=1;

	double d_oy_val=fabs((max_val_y-min_val_y)/d_oy_count);
	double d2=pow((double)10.0 , floor(log10(d_oy_val)));

	if (d2>=d_oy_val){
		d_oy_val=1*d2;
	} else	if (2*d2>=d_oy_val){ 
			d_oy_val=2*d2;
		} else	if (5*d2>=d_oy_val){
				d_oy_val=5*d2;
			} else {
				d_oy_val=10*d2;
			}
	
	double y_start_val=ceil(min_val_y/d_oy_val)*d_oy_val;

	for (double ny=0; ny<=d_oy_count;++ny){
		dc.GetTextExtent(wxString::Format(wxT("%.1f"), y_start_val+ny*d_oy_val), &w_temp, &h_temp, &des_temp, &lead_temp);
		if ((y_end - yscale * (y_start_val + ny * d_oy_val - min_val_y))>=(y_start-1)){
		    dc.DrawText(wxString::Format(wxT("%.1f"), y_start_val+ny*d_oy_val),(wxCoord)(x_start-w_temp-2-d_x),(wxCoord)(y_end - yscale * (y_start_val + ny * d_oy_val - min_val_y) - h_temp/2.0));
		    dc.DrawLine((wxCoord)(x_start-d_x+1),(wxCoord)(y_end - yscale * (y_start_val + ny * d_oy_val - min_val_y)),(wxCoord)(x_end+d_x),(wxCoord)(y_end - yscale * (y_start_val + ny * d_oy_val - min_val_y)));
		}
	}

//Draw day numbers and lines marking the days
	dc.SetPen(wxPen(m_pen_AxisXColour , 1 , wxDOT));
	dc.SetTextForeground (m_pen_AxisXTextColour);

	dtTemp1.Set((time_t)max_val_x);
	strBuffer1=dtTemp1.Format(wxT("%d.%b.%y"));
	dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);

	wxInt32 d_ox_count=1;
	d_ox_count=(wxInt32)((x_end-x_start)/(1.2*w_temp));
	
	double d_ox_val=ceil(((double)(max_val_x-min_val_x)/d_ox_count)/86400.0)*86400.0;
	d_ox_count=(wxInt32)ceil((max_val_x-min_val_x)/d_ox_val);

	double x_start_val=ceil(min_val_x/86400.0)*86400.0;
	
	for (double nx=0; nx<=d_ox_count;++nx){
		dtTemp1.Set((time_t)(x_start_val+nx*d_ox_val));
		strBuffer1=dtTemp1.Format(wxT("%d.%b.%y"));
		dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);
		if ((x_start + xscale * (x_start_val + nx * d_ox_val - min_val_x))<=(x_end+1)){
		    dc.DrawText(strBuffer1, (wxCoord)(x_start-w_temp/2.0 + (xscale * (x_start_val + nx * d_ox_val - min_val_x))), (wxCoord)(y_end+d_y));
		    dc.DrawLine((wxCoord)(x_start + xscale * (x_start_val + nx * d_ox_val - min_val_x)),(wxCoord)(y_start-d_y+1), (wxCoord)(x_start + xscale * (x_start_val + nx * d_ox_val - min_val_x)),(wxCoord)(y_end+d_y));
		}
	}
	dc.DestroyClippingRegion();
}

void CPaintStatistics::OnPaint(wxPaintEvent& WXUNUSED(event)) {
//Init global
	CMainDocument* pDoc      = wxGetApp().GetDocument();

	wxASSERT(pDoc);
	wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	PROJECTS *proj=&(pDoc->statistics_status);
	wxASSERT(proj);
//Init drawing
	
	wxCoord width = 0, height = 0;
	wxCoord rectangle_x_start=0, rectangle_y_start=0;
	wxCoord rectangle_x_end=0, rectangle_y_end=0;

	GetClientSize(&width, &height);

	wxBufferedPaintDC dc(this);

	dc.SetBackground(m_brush_MainColour);

//	dc.SetTextForeground (GetForegroundColour ());
	dc.SetTextForeground (m_pen_HeadTextColour);
	dc.SetTextBackground (GetBackgroundColour ());

	m_font_standart=dc.GetFont();
	m_font_bold=dc.GetFont();
	m_font_standart_italic=dc.GetFont();
	
	m_font_standart.SetWeight(wxNORMAL);
	m_font_bold.SetWeight(wxBOLD);
	m_font_standart_italic.SetFaceName(_T("Verdana"));
	m_font_standart_italic.SetStyle(wxFONTSTYLE_ITALIC);

	dc.SetFont(m_font_standart);

//	dc.SetFont(*wxSWISS_FONT);

//Start drawing
	dc.BeginDrawing();
	dc.Clear();
	dc.SetBrush(wxBrush(m_brush_MainColour , wxSOLID));
	dc.SetPen(wxPen(m_pen_MainColour , 1 , wxSOLID));
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
// Check m_NextProjectStatistic
	if (m_NextProjectStatistic<0) m_NextProjectStatistic=nb_proj-1;
	if ((m_NextProjectStatistic<0)||(m_NextProjectStatistic>=nb_proj)) m_NextProjectStatistic=0;
// Initial coord
	rectangle_x_start=0;
	rectangle_x_end=width;
	rectangle_y_start=0;
	rectangle_y_end=height;

	switch (m_SelectedStatistic){
	case 0: heading=_("User Total");
		break;
	case 1: heading=_("User Average");
		break;
	case 2: heading=_("Host Total");
		break;
	case 3: heading=_("Host Average");
		break;
	}

	switch (m_ModeViewStatistic){
	case 0:{
	///Draw heading
		CheckMinMaxC(rectangle_x_start, rectangle_x_end);
		CheckMinMaxC(rectangle_y_start, rectangle_y_end);
		dc.SetFont(m_font_bold);
		DrawMainHead(dc, heading, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
		dc.SetFont(m_font_standart);
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
			min_val_x=floor(min_val_x/86400.0)*86400.0;
			max_val_x=ceil(max_val_x/86400.0)*86400.0;
		//Where do we draw in?
			wxCoord x_start=0, y_start=0, x_end=0, y_end=0;
			x_start=(wxCoord)(rectangle_x_start+x_fac*(double)(col-1));
			x_end=(wxCoord)(rectangle_x_start+x_fac*((double)col));
			y_start=(wxCoord)(rectangle_y_start+y_fac*(double)(row-1));
			y_end=(wxCoord)(rectangle_y_start+y_fac*(double)row);
		///Draw scale Draw Project name
			wxString head_name=wxT("?");
			PROJECT* state_project = pDoc->state.lookup_project((*i)->master_url);
			if (state_project) {
			    head_name = wxString(state_project->project_name.c_str(), wxConvUTF8);
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
			wxColour graphColour=wxColour(0,0,0);
			getDrawColour(graphColour,m_SelectedStatistic);
			CheckMinMaxC(x_start, x_end);
			CheckMinMaxC(y_start, y_end);
			DrawGraph(dc, i, x_start, x_end, y_start, y_end, graphColour, 0, m_SelectedStatistic,  max_val_y, min_val_y, max_val_x, min_val_x);
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
		dc.SetFont(m_font_bold);
		DrawMainHead(dc, heading, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
		dc.SetFont(m_font_standart);
	//Draw project
		wxInt32 count=-1;
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;
			if (count!=m_NextProjectStatistic) continue;

		///Find minimum/maximum value
			double min_val_y=10e32, max_val_y=0;
			double min_val_x=10e32, max_val_x=0;

			MinMaxDayCredit(i, min_val_y,max_val_y,min_val_x, max_val_x, m_SelectedStatistic);

			double t_n1 = dtime();
			double t_d1 = floor((t_n1-max_val_x)/86400.0);

			wxString head_name=wxString::Format(_("Last update: %.0f days ago"),t_d1);

			CheckMinMaxD(min_val_x, max_val_x);
			CheckMinMaxD(min_val_y, max_val_y);
			min_val_x=floor(min_val_x/86400.0)*86400.0;
			max_val_x=ceil(max_val_x/86400.0)*86400.0;
		///Draw axis + Draw Project name
		///Draw heading
			CheckMinMaxC(rectangle_x_start, rectangle_x_end);
			CheckMinMaxC(rectangle_y_start, rectangle_y_end);
			PROJECT* state_project = pDoc->state.lookup_project((*i)->master_url);
			if (state_project) {
				dc.SetFont(m_font_standart_italic);
			    DrawProjectHead(dc, state_project, head_name , rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
				dc.SetFont(m_font_standart);
			}
		///Draw axis
			CheckMinMaxC(rectangle_x_start, rectangle_x_end);
			CheckMinMaxC(rectangle_y_start, rectangle_y_end);
			DrawAxis(dc, max_val_y, min_val_y,max_val_x, min_val_x, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
		///Draw graph
			wxColour graphColour=wxColour(0,0,0);
			getDrawColour(graphColour,m_SelectedStatistic);
			CheckMinMaxC(rectangle_x_start, rectangle_x_end);
			CheckMinMaxC(rectangle_y_start, rectangle_y_end);
			DrawGraph(dc, i, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end, graphColour, 0, m_SelectedStatistic,  max_val_y, min_val_y, max_val_x, min_val_x);
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
		dc.SetFont(m_font_bold);
		DrawMainHead(dc, heading, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);
		dc.SetFont(m_font_standart);
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
		min_val_x=floor(min_val_x/86400.0)*86400.0;
		max_val_x=ceil(max_val_x/86400.0)*86400.0;
	///Draw axis
		CheckMinMaxC(rectangle_x_start, rectangle_x_end);
		CheckMinMaxC(rectangle_y_start, rectangle_y_end);
		DrawAxis(dc, max_val_y, min_val_y,max_val_x, min_val_x, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end);

		count=-1;
		for (std::vector<PROJECT*>::const_iterator i=proj->projects.begin();i!=proj->projects.end(); ++i) {
			++count;
	///Draw graph
			wxColour graphColour=wxColour(0,0,0);
			wxInt32  typePoint=0;
			getTypePoint(typePoint,count);
			getDrawColour(graphColour,count);
			CheckMinMaxC(rectangle_x_start, rectangle_x_end);
			CheckMinMaxC(rectangle_y_start, rectangle_y_end);
			DrawGraph(dc, i, rectangle_x_start, rectangle_x_end, rectangle_y_start, rectangle_y_end, graphColour, typePoint, m_SelectedStatistic,  max_val_y, min_val_y, max_val_x, min_val_x);
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

void CPaintStatistics::OnLeftMouseDown(wxMouseEvent& event) {
	switch (m_ModeViewStatistic){
	case 1:{
		if (m_Legend_dY>0){
			wxPaintDC dc (this);
			wxPoint pt(event.GetLogicalPosition(dc));
			if((pt.y>m_Legend_Y1)&&(pt.y<m_Legend_Y2)&&(pt.x>m_Legend_X1)&&(pt.x<m_Legend_X2)){
				m_NextProjectStatistic=(wxInt32)floor((pt.y-m_Legend_Y1)/m_Legend_dY);
				Refresh(false);
			}
		}
		break;
		}
	}
}

void CPaintStatistics::OnSize(wxSizeEvent& event) {
    Refresh(false);
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
        _("Show total credit for user"),
        ID_TASK_STATISTICS_USERTOTAL 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Show user average"),
        _("Show average credit for user"),
        ID_TASK_STATISTICS_USERAVERAGE 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Show host total"),
        _("Show total credit for host"),
        ID_TASK_STATISTICS_HOSTTOTAL 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("Show host average"),
        _("Show average credit for host"),
        ID_TASK_STATISTICS_HOSTAVERAGE 
    );
    pGroup->m_Tasks.push_back( pItem );

	pGroup = new CTaskItemGroup( _("Project") );
	m_TaskGroups.push_back( pGroup );

	pItem = new CTaskItem(
        _("< &Previous project"),
        _("Show chart for previous project"),
        ID_TASK_STATISTICS_PREVPROJECT 
    );
    pGroup->m_Tasks.push_back( pItem );
	pItem = new CTaskItem(
        _("&Next project >"),
        _("Show chart for next project"),
        ID_TASK_STATISTICS_NEXTPROJECT 
    );
    pGroup->m_Tasks.push_back( pItem );

	pGroup = new CTaskItemGroup( _("Mode view") );
	m_TaskGroups.push_back( pGroup );
	pItem = new CTaskItem(
        _("All projects"),
        _("Show all projects, one chart per project"),
        ID_TASK_STATISTICS_MODEVIEW0 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("One project"),
        _("Show one chart with selected project"),
        ID_TASK_STATISTICS_MODEVIEW1 
    );
    pGroup->m_Tasks.push_back( pItem );
        
	pItem = new CTaskItem(
        _("All projects(sum)"),
        _("Show one chart with all projects"),
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

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_SelectedStatistic=0;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsUserTotal - Function End"));
}

void CViewStatistics::OnStatisticsUserAverage( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsUserAverage - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_SelectedStatistic=1;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsUserAverage - Function End"));
}

void CViewStatistics::OnStatisticsHostTotal( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsHostTotal - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_SelectedStatistic=2;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsHostTotal - Function End"));
}


void CViewStatistics::OnStatisticsHostAverage( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsHostAverage - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_SelectedStatistic=3;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsHostAverage - Function End"));
}

void CViewStatistics::OnStatisticsModeView0( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsModeView - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

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

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

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

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

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

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_NextProjectStatistic++;
    pFrame->UpdateStatusText(wxT(""));

    UpdateSelection();
    pFrame->FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsNextProject - Function End"));
}

void CViewStatistics::OnStatisticsPrevProject( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsPrevProject - Function Begin"));

    CAdvancedFrame* pFrame      = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

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
//--
    wxString    strBaseConfigLocation = wxEmptyString;
    strBaseConfigLocation = wxT("/StatisticPage");
	pConfig->SetPath(strBaseConfigLocation);
	pConfig->Write(wxT("ModeViewStatistic"),m_PaintStatistics->m_ModeViewStatistic);
	pConfig->Write(wxT("SelectedStatistic"),m_PaintStatistics->m_SelectedStatistic);
	pConfig->Write(wxT("NextProjectStatistic"),m_PaintStatistics->m_NextProjectStatistic);
//--
    return bReturnValue;
}

bool CViewStatistics::OnRestoreState(wxConfigBase* pConfig) {
	wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }
//--
    wxInt32     iTempValue = 0;
    wxString    strBaseConfigLocation = wxEmptyString;
    strBaseConfigLocation =wxT("/StatisticPage");
	pConfig->SetPath(strBaseConfigLocation);

	m_PaintStatistics->m_ModeViewStatistic=0;
	pConfig->Read(wxT("ModeViewStatistic"), &iTempValue, -1);
	if ((iTempValue>=0)&&(iTempValue<=2))m_PaintStatistics->m_ModeViewStatistic=iTempValue;

	m_PaintStatistics->m_SelectedStatistic=0;
	pConfig->Read(wxT("SelectedStatistic"), &iTempValue, -1);
	if ((iTempValue>=0)&&(iTempValue<=3))m_PaintStatistics->m_SelectedStatistic=iTempValue;

	m_PaintStatistics->m_NextProjectStatistic=0;
	pConfig->Read(wxT("NextProjectStatistic"), &iTempValue, -1);
	if (iTempValue>=0)m_PaintStatistics->m_NextProjectStatistic=iTempValue;

// Disable/Enable TaskGroups
	if(m_PaintStatistics->m_ModeViewStatistic==1){
		m_pTaskPane->EnableTaskGroupTasks(m_TaskGroups[1]); /// "project" button 
	}else m_pTaskPane->DisableTaskGroupTasks(m_TaskGroups[1]); /// "project" button
//--
    return true;
}

void CViewStatistics::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
	if (wxGetApp().GetDocument()->GetStatisticsCount()) {
		m_PaintStatistics->Refresh(false);
	}
}

void CViewStatistics::UpdateSelection() {
    CBOINCBaseView::PreUpdateSelection();
    CBOINCBaseView::PostUpdateSelection();
}


const char *BOINC_RCSID_7aadb93333 = "$Id$";

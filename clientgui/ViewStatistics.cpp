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

BEGIN_EVENT_TABLE (CPaintStatistics, wxWindow)
	EVT_PAINT(CPaintStatistics::OnPaint)
	EVT_SIZE(CPaintStatistics::OnSize)
	EVT_LEFT_DOWN(CPaintStatistics::OnLeftMouseDown)
	EVT_LEFT_UP(CPaintStatistics::OnLeftMouseUp)
	EVT_LEFT_DCLICK(CPaintStatistics::OnLeftMouseDoubleClick)
	EVT_RIGHT_DOWN(CPaintStatistics::OnRightMouseDown)
	EVT_RIGHT_UP(CPaintStatistics::OnRightMouseUp)
	EVT_MOTION(CPaintStatistics::OnMouseMotion)
	EVT_LEAVE_WINDOW(CPaintStatistics::OnMouseLeaveWindows)
	EVT_ERASE_BACKGROUND(CPaintStatistics::OnEraseBackground)
END_EVENT_TABLE ()


CPaintStatistics::CPaintStatistics(wxWindow* parent, wxWindowID id, const wxPoint& pos,	const wxSize& size, long style, const wxString& name
): wxWindow(parent, id, pos, size, style, name)
{	m_font_standart = *wxSWISS_FONT;
	m_font_bold = *wxSWISS_FONT;
	m_font_standart_italic = *wxSWISS_FONT;

	m_SelectedStatistic = 0;
	heading = wxT("");
	m_ModeViewStatistic = 0;
	m_NextProjectStatistic = 0;
	m_ViewHideProjectStatistic = -1;

	m_GraphLineWidth = 2;
	m_GraphPointWidth = 4;

	m_Legend_Shift_Mode1 = 0;
	m_Legend_Shift_Mode2 = 0;

	m_GraphMarker_X1 = 0;
	m_GraphMarker_Y1 = 0;
	m_GraphMarker1 = false;

	m_GraphZoom_X1 = 0;
	m_GraphZoom_Y1 = 0;
	m_GraphZoom_X2 = 0;
	m_GraphZoom_Y2 = 0;
	m_GraphZoom_X2_old = 0;
	m_GraphZoom_Y2_old = 0;

	m_GraphZoomStart = false;

	m_GraphMove_X1 = 0;
	m_GraphMove_Y1 = 0;
	m_GraphMove_X2 = 0;
	m_GraphMove_Y2 = 0;
	m_GraphMoveStart = false;
	m_GraphMoveGo = false;

	m_Zoom_max_val_X = 0;
	m_Zoom_min_val_X = 0;
	m_Zoom_max_val_Y = 0;
	m_Zoom_min_val_Y = 0;
	m_Zoom_Auto = true;

// XY
	m_main_X_start = 0;
	m_main_X_end = 0;
	m_main_Y_start = 0;
	m_main_Y_end = 0;

	m_WorkSpace_X_start = 0;
	m_WorkSpace_X_end = 0;
	m_WorkSpace_Y_start = 0;
	m_WorkSpace_Y_end = 0;

	m_Legend_X_start = 0;
	m_Legend_X_end = 0;
	m_Legend_Y_start = 0;
	m_Legend_Y_end = 0;

	m_Legend_select_X_start = 0;
	m_Legend_select_X_end = 0;
	m_Legend_select_Y_start = 0;
	m_Legend_select_Y_end = 0;

	m_Graph_X_start = 0;
	m_Graph_X_end = 0;
	m_Graph_Y_start = 0;
	m_Graph_Y_end = 0;

	m_Graph_draw_X_start = 0;
	m_Graph_draw_X_end = 0;
	m_Graph_draw_Y_start = 0;
	m_Graph_draw_Y_end = 0;

	m_Legend_dY = 0;
	m_LegendDraw = true;

// Default colours
	m_pen_MarkerLineColour = wxColour(0, 0, 0);
	m_pen_ZoomRectColour = wxColour (128, 64, 95);
	m_brush_ZoomRectColour = wxColour(24, 31, 0);
	m_brush_AxisColour = wxColour(192, 224, 255);
	m_pen_AxisColour = wxColour(64, 128, 192);
	m_pen_AxisColourZoom = wxColour(255, 64, 0);
	m_pen_AxisColourAutoZoom = wxColour(64, 128, 192);
	m_pen_AxisXColour = wxColour(64, 128, 192);
	m_pen_AxisYColour = wxColour(64, 128, 192);
	m_pen_AxisXTextColour = wxColour(0, 0, 0);
	m_pen_AxisYTextColour = wxColour(0, 0, 0);      
	                            
	m_brush_LegendColour = wxColour(235, 255, 255);//wxColour(220, 240, 255);
	m_brush_LegendSelectColour = wxColour(192, 224, 255);
	m_pen_LegendSelectColour = wxColour(64, 128, 192);
	m_pen_LegendSelectTextColour = wxColour(0, 0, 0);
	m_pen_LegendColour = wxColour(64, 128, 192);
	m_pen_LegendTextColour = wxColour(0, 0, 0);
	                            
	m_brush_MainColour = wxColour(255, 255, 255);
	m_pen_MainColour = wxColour(64, 128, 192);
	                            
	m_pen_HeadTextColour = wxColour(0, 0, 0);
	m_pen_ProjectHeadTextColour = wxColour(0, 0, 0);
	                            
	m_pen_GraphTotalColour = wxColour(255, 0, 0);
	m_pen_GraphRACColour = wxColour(0, 160, 0);
	m_pen_GraphTotalHostColour = wxColour(0, 0, 255);
	m_pen_GraphRACHostColour = wxColour(0, 0, 0);
	                            
	m_pen_GraphColour00 = wxColour(255, 0, 0);
	m_pen_GraphColour01 = wxColour(0, 160, 0);
	m_pen_GraphColour02 = wxColour(0, 0, 255);
	m_pen_GraphColour03 = wxColour(0, 0, 0);
	m_pen_GraphColour04 = wxColour(255, 0, 255);
	m_pen_GraphColour05 = wxColour(255, 128, 0);
	m_pen_GraphColour06 = wxColour(192, 192, 0);
	m_pen_GraphColour07 = wxColour(0, 192, 192);
	m_pen_GraphColour08 = wxColour(160, 160, 160);
	m_pen_GraphColour09 = wxColour(160, 0, 0);

	m_dc_bmp.Create(1, 1);
	m_full_repaint = true;
	m_bmp_OK = false;
}
static void getTypePoint(int &typePoint, int number) {typePoint = number / 10;}

static bool CrossTwoLine(const double X1_1, const double Y1_1, const double X1_2, const double Y1_2, 
						 const double X2_1, const double Y2_1, const double X2_2, const double Y2_2, 
						 double &Xcross, double &Ycross) {
	double A1 = Y1_1 - Y1_2;
	double B1 = X1_2 - X1_1;
	double C1 = - X1_1 * A1 - Y1_1 * B1;
	double A2 = Y2_1 - Y2_2;
	double B2 = X2_2 - X2_1;
	double C2 = - X2_1 * A2 - Y2_1 * B2;
	double tmp1 = (A1 * B2 - A2 * B1);
	if (0 == tmp1){
		Xcross = 0;
		Ycross = 0;
		return false;
	}else{
		Xcross = (B1 * C2 - B2 * C1) / tmp1;
		Ycross = (C1 * A2 - C2 * A1) / tmp1;
		return true;
	}
}

void CPaintStatistics::getDrawColour(wxColour &graphColour, int number) {
	switch (number % 10){
	case 1:	graphColour = m_pen_GraphColour01;	break;
	case 2:	graphColour = m_pen_GraphColour02;	break;
	case 3:	graphColour = m_pen_GraphColour03;	break;
	case 4:	graphColour = m_pen_GraphColour04;	break;
	case 5: graphColour = m_pen_GraphColour05;	break;
	case 6:	graphColour = m_pen_GraphColour06;	break;
	case 7:	graphColour = m_pen_GraphColour07;	break;
	case 8:	graphColour = m_pen_GraphColour08;	break;
	case 9: graphColour = m_pen_GraphColour09;	break;
	default:graphColour = m_pen_GraphColour00;
	}
}
//----Draw "Point"
static void myDrawPoint(wxDC &dc,int X, int Y, wxColour graphColour,int numberTypePoint, int PointWidth) {
	dc.SetPen(wxPen(graphColour , 1 , wxSOLID));
	switch (numberTypePoint % 5){
	case 1: {wxPoint* points = new wxPoint[3];
		points[0] = wxPoint(X, Y - 1 - (PointWidth / 2));
		points[1] = wxPoint(X + (PointWidth / 2), Y + (PointWidth / 2));
		points[2] = wxPoint(X - (PointWidth / 2), Y + (PointWidth / 2));
		dc.DrawPolygon(3, points);
		delete[] points;
		break;}
	case 2: {wxPoint* points = new wxPoint[3];
		points[0] = wxPoint(X, Y + 1 + (PointWidth / 2));
		points[1] = wxPoint(X + (PointWidth / 2), Y - (PointWidth / 2));
		points[2] = wxPoint(X - (PointWidth / 2), Y - (PointWidth / 2));
		dc.DrawPolygon(3, points);
		delete[] points;
		break;}
	case 3:	dc.DrawRectangle(wxCoord(X - (PointWidth / 2)),wxCoord(Y - (PointWidth / 2)),wxCoord(PointWidth + 1),wxCoord(PointWidth + 1));
		break;                              
	case 4: {wxPoint* points = new wxPoint[4];
		points[0] = wxPoint(X, Y - 1 - (PointWidth / 2));
		points[1] = wxPoint(X + 1 + (PointWidth / 2), Y);
		points[2] = wxPoint(X, Y + 1 + (PointWidth / 2));
		points[3] = wxPoint(X - 1 - (PointWidth / 2), Y);
		dc.DrawPolygon(4, points);
		delete[] points;
		break;}
	default:dc.DrawCircle(wxCoord(X), wxCoord(Y), wxCoord(PointWidth / 2));
	}
}
//----Find minimum/maximum value----
static void MinMaxDayCredit(std::vector<PROJECT*>::const_iterator &i, double &min_credit, double &max_credit, double &min_day, double &max_day, const int m_SelectedStatistic, bool first = true) {
	for (std::vector<DAILY_STATS>::const_iterator j = (*i)->statistics.begin(); j != (*i)->statistics.end(); ++j) {
		if (first){
			max_day = j->day;
			switch (m_SelectedStatistic){ 
			case 0:	max_credit = j->user_total_credit;	break;
			case 1:	max_credit = j->user_expavg_credit;	break;
			case 2:	max_credit = j->host_total_credit;	break;
			case 3:	max_credit = j->host_expavg_credit;	break;
			default: max_credit = 0.0;
			}
			min_day = max_day;
			min_credit = max_credit;
			first = false;
		} else {
			if (j->day < min_day) min_day = j->day;
			if (j->day > max_day) max_day = j->day;

			switch (m_SelectedStatistic){ 
			case 0:
				if (j->user_total_credit > max_credit) max_credit = j->user_total_credit;
				if (j->user_total_credit < min_credit) min_credit = j->user_total_credit;
				break;
			case 1:
				if (j->user_expavg_credit > max_credit) max_credit = j->user_expavg_credit;
				if (j->user_expavg_credit < min_credit) min_credit = j->user_expavg_credit;
				break;
			case 2:
				if (j->host_total_credit > max_credit) max_credit = j->host_total_credit;
				if (j->host_total_credit < min_credit) min_credit = j->host_total_credit;
				break;
			case 3:
				if (j->host_expavg_credit > max_credit) max_credit = j->host_expavg_credit;
				if (j->host_expavg_credit < min_credit) min_credit = j->host_expavg_credit;
				break;
			}
		}
	}
}
static void CheckMinMaxD(double &min_val, double &max_val) {
	if (min_val > max_val) min_val = max_val;
	if (max_val == min_val){
		max_val += 0.5;
		min_val -= 0.5;
	}
}
void CPaintStatistics::ClearXY(){
	m_main_X_start = 0;
	m_main_X_end = 0;
	m_main_Y_start = 0;
	m_main_Y_end = 0;

	m_WorkSpace_X_start = 0;
	m_WorkSpace_X_end = 0;
	m_WorkSpace_Y_start = 0;
	m_WorkSpace_Y_end = 0;

	m_Graph_X_start = 0;
	m_Graph_X_end = 0;
	m_Graph_Y_start = 0;
	m_Graph_Y_end = 0;

	m_Graph_draw_X_start = 0;
	m_Graph_draw_X_end = 0;
	m_Graph_draw_Y_start = 0;
	m_Graph_draw_Y_end = 0;
}
void CPaintStatistics::ClearLegendXY(){

	m_Legend_X_start = 0;
	m_Legend_X_end = 0;
	m_Legend_Y_start = 0;
	m_Legend_Y_end = 0;

	m_Legend_select_X_start = 0;
	m_Legend_select_X_end = 0;
	m_Legend_select_Y_start = 0;
	m_Legend_select_Y_end = 0;

	m_Legend_dY = 0;
}
void CPaintStatistics::AB(const double x_coord1, const double y_coord1, const double x_coord2, const double y_coord2, const double x_val1, const double y_val1, const double x_val2, const double y_val2){
// Val -> Coord
	if (0.0 == (x_val2 - x_val1)){
		m_Ax_ValToCoord = 0.0;
		m_Bx_ValToCoord = 0.0;
	}else{
		m_Ax_ValToCoord = (x_coord2 - x_coord1) / (x_val2 - x_val1);
		m_Bx_ValToCoord = x_coord1 - (m_Ax_ValToCoord * x_val1);
	}
	if (0.0 == (y_val2 - y_val1)){
		m_Ay_ValToCoord = 0.0;
		m_By_ValToCoord = 0.0;
	}else{
		m_Ay_ValToCoord = (y_coord2 - y_coord1) / (y_val2 - y_val1);
		m_By_ValToCoord = y_coord1 - (m_Ay_ValToCoord * y_val1);
	}
// Coord -> Val
	if (0.0 == (x_coord2 - x_coord1)){
		m_Ax_CoordToVal = 0.0;
		m_Bx_CoordToVal = 0.0;
	}else{
		m_Ax_CoordToVal = (x_val2 - x_val1) / (x_coord2 - x_coord1);
		m_Bx_CoordToVal = x_val1 - (m_Ax_CoordToVal * x_coord1);
	}
	if (0.0 == (y_coord2 - y_coord1)){
		m_Ay_CoordToVal = 0.0;
		m_By_CoordToVal = 0.0;
	}else{
		m_Ay_CoordToVal = (y_val2 - y_val1) / (y_coord2 - y_coord1);
		m_By_CoordToVal = y_val1 - (m_Ay_CoordToVal * y_coord1);
	}
}
//----Draw Main Head----
void CPaintStatistics::DrawMainHead(wxDC &dc, const wxString head_name){
	wxCoord w_temp = 0, h_temp = 0, des_temp = 0, lead_temp = 0;
	dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
	dc.SetTextForeground (m_pen_HeadTextColour);
	wxCoord x0 = wxCoord(m_WorkSpace_X_start + ((m_WorkSpace_X_end - m_WorkSpace_X_start - double(w_temp)) / 2.0));
	wxCoord y0 = wxCoord(m_WorkSpace_Y_start + 1.0);
	if (x0 > wxCoord(m_WorkSpace_X_end)) x0 = wxCoord(m_WorkSpace_X_end);
	if (x0 < wxCoord(m_WorkSpace_X_start)) x0 = wxCoord(m_WorkSpace_X_start);
	if (x0 < 0) x0 = 0;
	if (y0 > wxCoord(m_WorkSpace_Y_end)) y0 = wxCoord(m_WorkSpace_Y_end);
	if (y0 < wxCoord(m_WorkSpace_Y_start)) y0 = wxCoord(m_WorkSpace_Y_start);
	if (y0 < 0) y0 = 0;
	dc.DrawText (head_name, x0, y0);
	m_WorkSpace_Y_start += double(h_temp) + 2.0;
	if (m_WorkSpace_Y_start > m_WorkSpace_Y_end) m_WorkSpace_Y_start = m_WorkSpace_Y_end;
	if (m_WorkSpace_Y_start < 0.0) m_WorkSpace_Y_start = 0.0;
}
//----Draw Project Head----
void CPaintStatistics::DrawProjectHead(wxDC &dc, PROJECT* project1, const wxString head_name_last){
	wxCoord w_temp = 0, h_temp = 0, des_temp = 0, lead_temp = 0;
	wxString head_name = wxT("");
	wxCoord x0 = 0;
	wxCoord y0 = 0;

	if (project1) {
	    head_name = wxString(_("Project")) + wxT(": ") + wxString(project1->project_name.c_str(), wxConvUTF8);
	    dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
		x0 = wxCoord(m_WorkSpace_X_start + ((m_WorkSpace_X_end - m_WorkSpace_X_start - double(w_temp)) / 2.0));
		y0 = wxCoord(m_WorkSpace_Y_start + 1.0);
		if (x0 > wxCoord(m_WorkSpace_X_end)) x0 = wxCoord(m_WorkSpace_X_end);
		if (x0 < wxCoord(m_WorkSpace_X_start)) x0 = wxCoord(m_WorkSpace_X_start);
		if (x0 < 0) x0 = 0;
		if (y0 > wxCoord(m_WorkSpace_Y_end)) y0 = wxCoord(m_WorkSpace_Y_end);
		if (y0 < wxCoord(m_WorkSpace_Y_start)) y0 = wxCoord(m_WorkSpace_Y_start);
		if (y0 < 0) y0 = 0;
	    dc.DrawText (head_name, x0, y0);
	    m_WorkSpace_Y_start += double(h_temp) + 2.0;
		if (m_WorkSpace_Y_start > m_WorkSpace_Y_end) m_WorkSpace_Y_start = m_WorkSpace_Y_end;
		if (m_WorkSpace_Y_start < 0.0) m_WorkSpace_Y_start = 0.0;

		head_name = wxString(_("Account")) + wxT(": ") + wxString(project1->user_name.c_str(), wxConvUTF8);
	    dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
		x0 = wxCoord(m_WorkSpace_X_start + ((m_WorkSpace_X_end - m_WorkSpace_X_start - double(w_temp)) / 2.0));
		y0 = wxCoord(m_WorkSpace_Y_start + 1.0);
		if (x0 > wxCoord(m_WorkSpace_X_end)) x0 = wxCoord(m_WorkSpace_X_end);
		if (x0 < wxCoord(m_WorkSpace_X_start)) x0 = wxCoord(m_WorkSpace_X_start);
		if (x0 < 0) x0 = 0;
		if (y0 > wxCoord(m_WorkSpace_Y_end)) y0 = wxCoord(m_WorkSpace_Y_end);
		if (y0 < wxCoord(m_WorkSpace_Y_start)) y0 = wxCoord(m_WorkSpace_Y_start);
		if (y0 < 0) y0 = 0;
	    dc.DrawText (head_name, x0, y0);
	    m_WorkSpace_Y_start += double(h_temp) + 2.0;
		if (m_WorkSpace_Y_start > m_WorkSpace_Y_end) m_WorkSpace_Y_start = m_WorkSpace_Y_end;
		if (m_WorkSpace_Y_start < 0.0) m_WorkSpace_Y_start = 0.0;

	    head_name = wxString(_("Team")) + wxT(": ") + wxString(project1->team_name.c_str(), wxConvUTF8);
	    dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
		x0 = wxCoord(m_WorkSpace_X_start + ((m_WorkSpace_X_end - m_WorkSpace_X_start - double(w_temp)) / 2.0));
		y0 = wxCoord(m_WorkSpace_Y_start + 1.0);
		if (x0 > wxCoord(m_WorkSpace_X_end)) x0 = wxCoord(m_WorkSpace_X_end);
		if (x0 < wxCoord(m_WorkSpace_X_start)) x0 = wxCoord(m_WorkSpace_X_start);
		if (x0 < 0) x0 = 0;
		if (y0 > wxCoord(m_WorkSpace_Y_end)) y0 = wxCoord(m_WorkSpace_Y_end);
		if (y0 < wxCoord(m_WorkSpace_Y_start)) y0 = wxCoord(m_WorkSpace_Y_start);
		if (y0 < 0) y0 = 0;
	    dc.DrawText (head_name, x0, y0);
	    m_WorkSpace_Y_start += double(h_temp) + 2.0;
		if (m_WorkSpace_Y_start > m_WorkSpace_Y_end) m_WorkSpace_Y_start = m_WorkSpace_Y_end;
		if (m_WorkSpace_Y_start < 0.0) m_WorkSpace_Y_start = 0.0;

	    dc.GetTextExtent(head_name_last, &w_temp, &h_temp, &des_temp, &lead_temp);
		x0 = wxCoord(m_WorkSpace_X_start + ((m_WorkSpace_X_end - m_WorkSpace_X_start - double(w_temp)) / 2.0));
		y0 = wxCoord(m_WorkSpace_Y_start + 1.0);
		if (x0 > wxCoord(m_WorkSpace_X_end)) x0 = wxCoord(m_WorkSpace_X_end);
		if (x0 < wxCoord(m_WorkSpace_X_start)) x0 = wxCoord(m_WorkSpace_X_start);
		if (x0 < 0) x0 = 0;
		if (y0 > wxCoord(m_WorkSpace_Y_end)) y0 = wxCoord(m_WorkSpace_Y_end);
		if (y0 < wxCoord(m_WorkSpace_Y_start)) y0 = wxCoord(m_WorkSpace_Y_start);
		if (y0 < 0) y0 = 0;
	    dc.DrawText (head_name_last, x0, y0);
	    m_WorkSpace_Y_start += double(h_temp) + 2.0;
		if (m_WorkSpace_Y_start > m_WorkSpace_Y_end) m_WorkSpace_Y_start = m_WorkSpace_Y_end;
		if (m_WorkSpace_Y_start < 0.0) m_WorkSpace_Y_start = 0.0;
	}
}
//----Draw Legend----
void CPaintStatistics::DrawLegend(wxDC &dc, PROJECTS* proj, CMainDocument* pDoc, int SelProj, bool bColour, int &m_Legend_Shift){
	wxString head_name = wxT("0");
	wxCoord project_name_max_width = 0;
	const double radius1 = 5;
	const wxCoord buffer_y1 = 3;
	const wxCoord buffer_x1 = 3;
	int count = -1;
	int project_count = -1;
	wxCoord w_temp = 0, h_temp = 0, des_temp = 0, lead_temp = 0;
	wxCoord x0 = 0;
	wxCoord y0 = 0;
	wxCoord h0 = 0;
	wxCoord w0 = 0;

	dc.SetFont(m_font_bold);
	dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
	m_Legend_dY = (double)(h_temp) + 4.0;
	if (m_Legend_dY < 0) m_Legend_dY = 0;

	for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
		++count;
		PROJECT* state_project = pDoc->state.lookup_project((*i)->master_url);
		if (state_project) head_name = wxString(state_project->project_name.c_str(), wxConvUTF8);
		dc.GetTextExtent(head_name, &w_temp, &h_temp, &des_temp, &lead_temp);
		if (project_name_max_width < w_temp) project_name_max_width = w_temp;
	}
	project_name_max_width += wxCoord(8) + buffer_x1 + buffer_x1 + wxCoord(m_GraphPointWidth) + wxCoord(2);
	if (project_name_max_width < 0) project_name_max_width = 0;

	dc.SetBrush(wxBrush(m_brush_LegendColour , wxSOLID));
	dc.SetPen(wxPen(m_pen_LegendColour , 1 , wxSOLID));
	x0 = wxCoord(m_WorkSpace_X_end) - project_name_max_width + buffer_x1;
	y0 = wxCoord(m_WorkSpace_Y_start) + buffer_y1;
	w0 = project_name_max_width - buffer_x1 - buffer_x1;
	h0 = wxCoord(m_WorkSpace_Y_end - m_WorkSpace_Y_start) - buffer_y1 - buffer_y1;
	if (x0 > wxCoord(m_WorkSpace_X_end)) x0 = wxCoord(m_WorkSpace_X_end);
	if (x0 < wxCoord(m_WorkSpace_X_start)) x0 = wxCoord(m_WorkSpace_X_start);
	if (x0 < 0) x0 = 0;
	if (y0 > wxCoord(m_WorkSpace_Y_end)) y0 = wxCoord(m_WorkSpace_Y_end);
	if (y0 < wxCoord(m_WorkSpace_Y_start)) y0 = wxCoord(m_WorkSpace_Y_start);
	if (y0 < 0) y0 = 0;
	if (w0 < 0) w0 = 0;
	if (h0 < 0) h0 = 0;
	dc.DrawRoundedRectangle(x0, y0, w0, h0, radius1);

	m_Legend_X_start = double(x0);
	m_Legend_X_end = double(x0 + w0);
	m_Legend_Y_start = double(y0);
	m_Legend_Y_end = double(y0 + h0);
	if (m_Legend_X_end > m_WorkSpace_X_end) m_Legend_X_end = m_WorkSpace_X_end;
	if (m_Legend_Y_end > m_WorkSpace_Y_end) m_Legend_Y_end = m_WorkSpace_Y_end;
	if (m_Legend_X_start > m_Legend_X_end) m_Legend_X_start = m_Legend_X_end;
	if (m_Legend_Y_start > m_Legend_Y_end) m_Legend_Y_start = m_Legend_Y_end;

	m_Legend_select_X_start = m_Legend_X_start;
	m_Legend_select_X_end = m_Legend_X_end;
	m_Legend_select_Y_start = m_Legend_Y_start + radius1;
	m_Legend_select_Y_end = m_Legend_Y_end - radius1;
	if (m_Legend_select_Y_start < 0.0) m_Legend_select_Y_start = 0.0;
	if (m_Legend_select_Y_end < 0.0) m_Legend_select_Y_end = 0.0;
	if (m_Legend_select_Y_start > m_Legend_select_Y_end) m_Legend_select_Y_start = m_Legend_select_Y_end;

// Legend Shift (start)
	int Legend_count_temp = 0;
	if (m_Legend_dY > 0) Legend_count_temp = int(floor((m_Legend_select_Y_end - m_Legend_select_Y_start) / m_Legend_dY));
	
	if (SelProj >= 0){
		if (Legend_count_temp <= 0){
			m_Legend_Shift = SelProj;
		}
		else {
			if (SelProj < m_Legend_Shift) m_Legend_Shift = SelProj;
			if (SelProj >= (m_Legend_Shift + Legend_count_temp)) m_Legend_Shift = SelProj - Legend_count_temp + 1;
		}
	}
	if ((m_Legend_Shift + Legend_count_temp) > count) m_Legend_Shift = count - Legend_count_temp + 1;

	if (m_Legend_Shift > count) m_Legend_Shift = count; //???
	if (m_Legend_Shift < 0) m_Legend_Shift = 0;
//	Legend Shift (end)
	if (m_Legend_Shift > 0){
		dc.SetBrush(wxBrush(m_brush_LegendColour , wxSOLID));
		dc.SetPen(wxPen(m_pen_LegendColour , 1 , wxSOLID));
		x0 = wxCoord(m_Legend_select_X_start + ((m_Legend_select_X_end - m_Legend_select_X_start) / 4.0));
		y0 = wxCoord(m_Legend_Y_start - 1.0);
		w0 = wxCoord((m_Legend_select_X_end - m_Legend_select_X_start) / 2.0);
		h0 = wxCoord(3);
		if (x0 < 0) x0 = 0;
		if (y0 < 0) y0 = 0;
		if (w0 < 0) w0 = 0;
		if (h0 < 0) h0 = 0;
		dc.DrawRectangle(x0, y0 ,w0 , h0);
	}
//---------------
	project_count = count;
	count = -1;

	m_WorkSpace_X_end -= double(project_name_max_width);
	if (m_WorkSpace_X_end < m_WorkSpace_X_start) m_WorkSpace_X_end = m_WorkSpace_X_start;
	if (m_WorkSpace_X_end < 0.0) m_WorkSpace_X_end = 0.0;

	for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
		++count;
		if (count < m_Legend_Shift) continue;
	///Draw project name
		head_name = wxT("?");
		PROJECT* state_project = pDoc->state.lookup_project((*i)->master_url);
		if (state_project) head_name = wxString(state_project->project_name.c_str(), wxConvUTF8);

		if (SelProj == count){
			dc.SetBrush(wxBrush(m_brush_LegendSelectColour , wxSOLID));
			dc.SetPen(wxPen(m_pen_LegendSelectColour , 1 , wxSOLID));
			x0 = wxCoord(m_WorkSpace_X_end) + buffer_x1 - wxCoord(1);
			y0 = wxCoord(m_WorkSpace_Y_start + (double)(count - m_Legend_Shift) * m_Legend_dY + double(buffer_y1) + radius1);
			w0 = project_name_max_width - buffer_x1 - buffer_x1 + 2;
			h0 = wxCoord(m_Legend_dY);
			if (x0 < 0) x0 = 0;
			if (y0 < 0) y0 = 0;
			if (w0 < 0) w0 = 0;
			if (h0 < 0) h0 = 0;
			dc.DrawRoundedRectangle(x0 ,y0 , w0, h0, 1);
		}

		wxColour graphColour = wxColour(0, 0, 0);
		int  typePoint = 0;
		if (bColour){
			getTypePoint(typePoint, count);
			getDrawColour(graphColour, count);
		} else if (SelProj == count) {
				graphColour = m_pen_LegendSelectTextColour;
			} else {
				graphColour = m_pen_LegendTextColour;
			}

		dc.SetBrush(wxBrush(m_brush_LegendColour , wxSOLID));
		x0 = wxCoord(m_WorkSpace_X_end) + buffer_x1 + wxCoord(4) + wxCoord(m_GraphPointWidth / 2);
		y0 = wxCoord(m_WorkSpace_Y_start + ((double)(count - m_Legend_Shift) + 0.5) * m_Legend_dY + double(buffer_y1) + radius1);
		if (x0 < 0) x0 = 0;
		if (y0 < 0) y0 = 0;
		if ((SelProj >= 0) || (!(m_HideProjectStatistic.count( wxString( (*i)->master_url, wxConvUTF8 ) )))){
			myDrawPoint(dc, int(x0), int(y0), graphColour, typePoint ,m_GraphPointWidth);
			dc.SetFont(m_font_bold);			
		}else {
			dc.SetFont(m_font_standart_italic);
			graphColour = wxColour(0, 0, 0);
		}

		x0 = wxCoord(m_WorkSpace_X_end) + buffer_x1 + wxCoord(7) + wxCoord(m_GraphPointWidth);
		y0 = wxCoord(m_WorkSpace_Y_start + 1.0 + (double)(count - m_Legend_Shift) * m_Legend_dY + double(buffer_y1) + radius1);
		if (x0 < 0) x0 = 0;
		if (y0 < 0) y0 = 0;
		dc.SetTextForeground(graphColour);
		dc.DrawText(head_name, x0, y0);
		m_Legend_select_Y_end = m_WorkSpace_Y_start + (double)(count - m_Legend_Shift + 1) * m_Legend_dY + double(buffer_y1) + radius1;
		if ((m_Legend_select_Y_end + m_Legend_dY) > (m_WorkSpace_Y_end - double(buffer_y1) - radius1)){
			if (project_count > count){
				dc.SetBrush(wxBrush(m_brush_LegendColour, wxSOLID));
				dc.SetPen(wxPen(m_pen_LegendColour, 1, wxSOLID));
				x0 = wxCoord(m_Legend_select_X_start + ((m_Legend_select_X_end - m_Legend_select_X_start) / 4.0));
				y0 = wxCoord(m_WorkSpace_Y_end) - buffer_y1 - wxCoord(2);
				w0 = wxCoord((m_Legend_select_X_end - m_Legend_select_X_start) / 2.0);
				if (x0 < 0) x0 = 0;
				if (y0 < 0) y0 = 0;
				if (w0 < 0) w0 = 0;
				dc.DrawRectangle(x0, y0, w0, wxCoord(3));
			}
			break;
		}
	}
	dc.SetFont(m_font_standart);
}
//----Draw background, axis(lines), text(01-Jan-1980)----
void CPaintStatistics::DrawAxis(wxDC &dc, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x, 
								wxColour pen_AxisColour, const double max_val_y_all, const double min_val_y_all) {
	wxCoord x0 = wxCoord(m_WorkSpace_X_start);
	wxCoord y0 = wxCoord(m_WorkSpace_Y_start);
	wxCoord w0 = wxCoord(m_WorkSpace_X_end - m_WorkSpace_X_start);
	wxCoord h0 = wxCoord(m_WorkSpace_Y_end - m_WorkSpace_Y_start);
	wxCoord x1 = 0;
	wxCoord y1 = 0;
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (w0 < 0) w0 = 0;
	if (h0 < 0) h0 = 0;
	dc.SetClippingRegion(x0, y0, w0, h0);

	dc.SetBrush(wxBrush(m_brush_AxisColour , wxSOLID));
	dc.SetPen(wxPen(pen_AxisColour , 1 , wxSOLID));

	wxCoord w_temp, h_temp, des_temp, lead_temp;
	wxCoord w_temp2;
	
	dc.GetTextExtent(wxString::Format(wxT(" %.2f"), max_val_y_all), &w_temp, &h_temp, &des_temp, &lead_temp);
	dc.GetTextExtent(wxString::Format(wxT(" %.2f"), min_val_y_all), &w_temp2, &h_temp, &des_temp, &lead_temp);

	if (w_temp < w_temp2) w_temp = w_temp2;

	m_WorkSpace_X_start += double(w_temp) + 3.0;
	m_WorkSpace_Y_end -= double(h_temp) + 3.0;
	
	dc.GetTextExtent(wxT("0"), &w_temp, &h_temp, &des_temp, &lead_temp);

	m_WorkSpace_X_end -= 3.0;//w_temp;
	const double radius1 = 5.0;//(double)(h_temp/2.0);
	double d_y = (double)(h_temp) / 2.0;
	if (d_y < 5.0) d_y = 5.0;
	
	wxDateTime dtTemp1;
	wxString strBuffer1;
	dtTemp1.Set((time_t)max_val_x);
	strBuffer1 = dtTemp1.Format(wxT("%d.%b.%y"), wxDateTime::GMT0);
	dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);
	
	double d_x = (double)(w_temp) / 2.0;

// Draw background graph
	x0 = wxCoord(m_WorkSpace_X_start);
	y0 = wxCoord(m_WorkSpace_Y_start);
	w0 = wxCoord(m_WorkSpace_X_end - m_WorkSpace_X_start);
	h0 = wxCoord(m_WorkSpace_Y_end - m_WorkSpace_Y_start);
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (w0 < 0) w0 = 0;
	if (h0 < 0) h0 = 0;
	dc.DrawRoundedRectangle(x0, y0, w0, h0, radius1);

	m_Graph_X_start = m_WorkSpace_X_start;	//x0;
	m_Graph_X_end = m_WorkSpace_X_end;		//x0 + w0;
	m_Graph_Y_start = m_WorkSpace_Y_start;	//y0;
	m_Graph_Y_end = m_WorkSpace_Y_end;		//y0 + h0;

	m_WorkSpace_X_start += d_x;
	m_WorkSpace_X_end -= d_x;
	m_WorkSpace_Y_start += d_y;
	m_WorkSpace_Y_end -= d_y;

	if (m_WorkSpace_X_end < m_WorkSpace_X_start) m_WorkSpace_X_start = m_WorkSpace_X_end = (m_WorkSpace_X_end + m_WorkSpace_X_start) / 2.0;
	if (m_WorkSpace_Y_end < m_WorkSpace_Y_start) m_WorkSpace_Y_start = m_WorkSpace_Y_end = (m_WorkSpace_Y_end + m_WorkSpace_Y_start) / 2.0;

	m_Graph_draw_X_start = m_WorkSpace_X_start;
	m_Graph_draw_X_end = m_WorkSpace_X_end;
	m_Graph_draw_Y_start = m_WorkSpace_Y_start;
	m_Graph_draw_Y_end = m_WorkSpace_Y_end;
// A B
	AB(m_WorkSpace_X_start, m_WorkSpace_Y_end, m_WorkSpace_X_end, m_WorkSpace_Y_start, 
		min_val_x, min_val_y, max_val_x, max_val_y);
//Draw val and lines
	dc.SetPen(wxPen(m_pen_AxisYColour , 1 , wxDOT));
	dc.SetTextForeground (m_pen_AxisYTextColour);

	int d_oy_count = 1;
	if (h_temp > 0)	d_oy_count = (int)ceil((m_WorkSpace_Y_end - m_WorkSpace_Y_start) / ( 2.0 * double(h_temp)));
	if (d_oy_count <= 0) d_oy_count = 1;
	double d_oy_val = fabs((max_val_y - min_val_y) / double(d_oy_count));
	double d2 = pow(double(10.0) , floor(log10(d_oy_val)));

	if (d2 >= d_oy_val){
		d_oy_val = 1.0 * d2;
	} else	if (2.0 * d2 >= d_oy_val){ 
			d_oy_val = 2.0 * d2;
		} else	if (5.0 * d2 >= d_oy_val){
				d_oy_val = 5.0 * d2;
			} else {
				d_oy_val = 10.0 * d2;
			}
	if (0 == d_oy_val) d_oy_val = 0.01;
	double y_start_val = ceil(min_val_y / d_oy_val) * d_oy_val;
	d_oy_count = (int)floor((max_val_y - y_start_val) / d_oy_val);

	for (double ny = 0; ny <= double(d_oy_count); ++ny){
		dc.GetTextExtent(wxString::Format(wxT("%.2f"), y_start_val + ny * d_oy_val), &w_temp, &h_temp, &des_temp, &lead_temp);
		x0 = wxCoord(m_Graph_X_start + 1.0);
		y0 = wxCoord(m_Ay_ValToCoord * (y_start_val + ny * d_oy_val) + m_By_ValToCoord);
		x1 = wxCoord(m_Graph_X_end - 1.0);
		if ((y0 >= wxCoord(m_WorkSpace_Y_start)) && (y0 <= wxCoord(m_WorkSpace_Y_end))){
			if (x0 < 0) x0 = 0;
			if (y0 < 0) y0 = 0;
			if (x1 < 0) x1 = 0;
			dc.DrawLine(x0, y0, x1, y0);
			x0 = wxCoord(m_Graph_X_start - 2.0) - w_temp;
			y0 = wxCoord(m_Ay_ValToCoord * (y_start_val + ny * d_oy_val) + m_By_ValToCoord - double(h_temp) / 2.0);
			if (x0 < 0) x0 = 0;
			if (y0 < 0) y0 = 0;
			dc.DrawText(wxString::Format(wxT("%.2f"), y_start_val + ny * d_oy_val), x0, y0);
		}
	}

//Draw day numbers and lines marking the days
	dc.SetPen(wxPen(m_pen_AxisXColour , 1 , wxDOT));
	dc.SetTextForeground (m_pen_AxisXTextColour);

	dtTemp1.Set((time_t)max_val_x);
	strBuffer1 = dtTemp1.Format(wxT("%d.%b.%y"), wxDateTime::GMT0);
	dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);

	int d_ox_count = 1;
	if (w_temp > 0)	d_ox_count = (int)((m_WorkSpace_X_end - m_WorkSpace_X_start) / (1.2 * double(w_temp)));
	if (d_ox_count <= 0) d_ox_count = 1;
	
	double d_ox_val = ceil(((double)(max_val_x - min_val_x) / double(d_ox_count)) / 86400.0) * 86400.0;
	if (0 == d_ox_val) d_ox_val = 1;

	double x_start_val = ceil(min_val_x / 86400.0) * 86400.0;
	d_ox_count = (int)floor((max_val_x - x_start_val) / d_ox_val);

	for (double nx = 0; nx <= double(d_ox_count); ++nx){
		dtTemp1.Set((time_t)(x_start_val + nx * d_ox_val));
		strBuffer1 = dtTemp1.Format(wxT("%d.%b.%y"), wxDateTime::GMT0);
		dc.GetTextExtent(strBuffer1, &w_temp, &h_temp, &des_temp, &lead_temp);
		x0 = wxCoord(m_Ax_ValToCoord * (x_start_val + nx * d_ox_val) + m_Bx_ValToCoord);
		y0 = wxCoord(m_Graph_Y_start + 1.0);
		y1 = wxCoord(m_Graph_Y_end - 1.0);
		if ((x0 <= wxCoord(m_WorkSpace_X_end)) && (x0 >= wxCoord(m_WorkSpace_X_start))){
			if (x0 < 0) x0 = 0;
			if (y0 < 0) y0 = 0;
			if (y1 < 0) y1 = 0;
		    dc.DrawLine(x0, y0, x0, y1);
			x0 = wxCoord(m_Ax_ValToCoord * (x_start_val + nx * d_ox_val) + m_Bx_ValToCoord - (double(w_temp) / 2.0));
			y0 = (wxCoord)m_Graph_Y_end;
			if (x0 < 0) x0 = 0;
			if (y0 < 0) y0 = 0;
			dc.DrawText(strBuffer1, x0, y0);
		}
	}
	dc.DestroyClippingRegion();
}
//----Draw graph----
void CPaintStatistics::DrawGraph(wxDC &dc, std::vector<PROJECT*>::const_iterator &i, const wxColour graphColour, const int typePoint, const int m_SelectedStatistic) {
	wxCoord x0 = wxCoord(m_Graph_X_start);
	wxCoord y0 = wxCoord(m_Graph_Y_start);
	wxCoord w0 = wxCoord(m_Graph_X_end - m_Graph_X_start);
	wxCoord h0 = wxCoord(m_Graph_Y_end - m_Graph_Y_start);
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (w0 < 0) w0 = 0;
	if (h0 < 0) h0 = 0;
	dc.SetClippingRegion(x0, y0, w0, h0);

	dc.SetPen(wxPen(graphColour , m_GraphLineWidth , wxSOLID));

	wxCoord last_x = 0;
	wxCoord last_y = 0;
	wxCoord xpos = 0;
	wxCoord ypos = 0;

	double d_last_x = 0;
	double d_last_y = 0;
	bool last_point_in = false;

	double d_xpos = 0;
	double d_ypos = 0;
	bool point_in = false;

	bool b_point1 = false;
	bool b_point2 = false;
// cross
	double d_cross_x1 = 0;
	double d_cross_y1 = 0;
// first point (no line)
	bool first_point = true;
// end point
	double d_end_point_x = 0;
	double d_end_point_y = 0;
	bool end_point = false;
//
	for (std::vector<DAILY_STATS>::const_iterator j = (*i)->statistics.begin(); j != (*i)->statistics.end(); ++j) {
		double d_x1 = 0;
		double d_y1 = 0;
		double d_x2 = 0;
		double d_y2 = 0;
		double d_min1 = 0;
		double d_max1 = 0;
		double d_min2 = 0;
		double d_max2 = 0;

		b_point1 = false;
		b_point2 = false;

		d_xpos = (m_Ax_ValToCoord * j->day + m_Bx_ValToCoord);// добавить округление
		switch (m_SelectedStatistic){  // добавить округление
		case 0:	d_ypos = (m_Ay_ValToCoord * j->user_total_credit + m_By_ValToCoord);	break;
		case 1:	d_ypos = (m_Ay_ValToCoord * j->user_expavg_credit + m_By_ValToCoord);	break;
		case 2:	d_ypos = (m_Ay_ValToCoord * j->host_total_credit + m_By_ValToCoord);	break;
		case 3:	d_ypos = (m_Ay_ValToCoord * j->host_expavg_credit + m_By_ValToCoord);	break;
		default:d_ypos = (m_Ay_ValToCoord * j->user_total_credit + m_By_ValToCoord);	break;
		}

		if (first_point) {
			if ((d_xpos < m_Graph_X_start) || (d_xpos > m_Graph_X_end) || 
				(d_ypos < m_Graph_Y_start) || (d_ypos > m_Graph_Y_end)){
				point_in = false;
				b_point2 = false;
				end_point = false;
			}else {
				point_in = true;
				d_x2 = d_xpos;
				d_y2 = d_ypos;
				b_point2 = true;
				d_end_point_x = d_xpos;
				d_end_point_y = d_ypos;
				end_point = true;
			}
			first_point = false;
		}else {
			dc.SetPen(wxPen(graphColour , m_GraphLineWidth , wxSOLID));
			// проверка попадани€ первой точки линии в область рисовани€
			if (last_point_in){ 
				d_x1 = d_last_x;
				d_y1 = d_last_y;
				b_point1 = true;
			}else b_point1 = false;
			// проверка попадани€ второй точки линии в область рисовани€
			if ((d_xpos < m_Graph_X_start) || (d_xpos > m_Graph_X_end) || 
				(d_ypos < m_Graph_Y_start) || (d_ypos > m_Graph_Y_end)){
				point_in = false;
				b_point2 = false;
			}else {
				point_in = true;
				d_x2 = d_xpos;
				d_y2 = d_ypos;
				b_point2 = true;
			}
			// »щем точку входа линии в область рисовани€ (1) x=const
			if (!b_point1 || !b_point2){
				if (CrossTwoLine(d_last_x, d_last_y, d_xpos, d_ypos, 
								m_Graph_X_start, m_Graph_Y_end, m_Graph_X_start, m_Graph_Y_start,
								d_cross_x1, d_cross_y1)){
					if (d_last_x > d_xpos){
						d_min1 = d_xpos;
						d_max1 = d_last_x;
					}else{
						d_max1 = d_xpos;
						d_min1 = d_last_x;
					}
					if (m_Graph_Y_end > m_Graph_Y_start){
						d_min2 = m_Graph_Y_start;
						d_max2 = m_Graph_Y_end;
					}else{
						d_max2 = m_Graph_Y_end;
						d_min2 = m_Graph_Y_start;
					}
					if ((d_cross_x1 <= d_max1) && (d_cross_x1 >= d_min1) && 
						(d_cross_y1 <= d_max2) && (d_cross_y1 >= d_min2)){
						if (!b_point1){
							d_x1 = d_cross_x1;
							d_y1 = d_cross_y1;
							b_point1 = true;
						} else if (!b_point2){
							d_x2 = d_cross_x1;
							d_y2 = d_cross_y1;
							b_point2 = true;
						}
					}
				}
			}
			// »щем точку входа линии в область рисовани€ (2) x=const
			if (!b_point1 || !b_point2){
				if (CrossTwoLine(d_last_x, d_last_y, d_xpos, d_ypos, 
								m_Graph_X_end, m_Graph_Y_end, m_Graph_X_end, m_Graph_Y_start,
								d_cross_x1, d_cross_y1)){
					if (d_last_x > d_xpos){
						d_min1 = d_xpos;
						d_max1 = d_last_x;
					}else{
						d_max1 = d_xpos;
						d_min1 = d_last_x;
					}
					if (m_Graph_Y_end > m_Graph_Y_start){
						d_min2 = m_Graph_Y_start;
						d_max2 = m_Graph_Y_end;
					}else{
						d_max2 = m_Graph_Y_end;
						d_min2 = m_Graph_Y_start;
					}
					if ((d_cross_x1 <= d_max1) && (d_cross_x1 >= d_min1) && 
						(d_cross_y1 <= d_max2) && (d_cross_y1 >= d_min2)){
						if (!b_point1){
							d_x1 = d_cross_x1;
							d_y1 = d_cross_y1;
							b_point1 = true;
						} else if (!b_point2){
							d_x2 = d_cross_x1;
							d_y2 = d_cross_y1;
							b_point2 = true;
						}
					}
				}
			}
			// »щем точку входа линии в область рисовани€ (3) y=const
			if (!b_point1 || !b_point2){
				if (CrossTwoLine(d_last_x, d_last_y, d_xpos, d_ypos, 
								m_Graph_X_start, m_Graph_Y_start, m_Graph_X_end, m_Graph_Y_start,
								d_cross_x1, d_cross_y1)){
					if (d_last_y > d_ypos){
						d_min1 = d_ypos;
						d_max1 = d_last_y;
					}else{
						d_max1 = d_ypos;
						d_min1 = d_last_y;
					}
					if (m_Graph_X_end > m_Graph_X_start){
						d_min2 = m_Graph_X_start;
						d_max2 = m_Graph_X_end;
					}else{
						d_max2 = m_Graph_X_end;
						d_min2 = m_Graph_X_start;
					}
					if ((d_cross_y1 <= d_max1) && (d_cross_y1 >= d_min1) && 
						(d_cross_x1 <= d_max2) && (d_cross_x1 >= d_min2)){
						if (!b_point1){
							d_x1 = d_cross_x1;
							d_y1 = d_cross_y1;
							b_point1 = true;
						} else if (!b_point2){
							d_x2 = d_cross_x1;
							d_y2 = d_cross_y1;
							b_point2 = true;
						}
					}
				}
			}
			// »щем точку входа линии в область рисовани€ (4) y=const
			if (!b_point1 || !b_point2){
				if (CrossTwoLine(d_last_x, d_last_y, d_xpos, d_ypos, 
								m_Graph_X_start, m_Graph_Y_end, m_Graph_X_end, m_Graph_Y_end,
								d_cross_x1, d_cross_y1)){
					if (d_last_y > d_ypos){
						d_min1 = d_ypos;
						d_max1 = d_last_y;
					}else{
						d_max1 = d_ypos;
						d_min1 = d_last_y;
					}
					if (m_Graph_X_end > m_Graph_X_start){
						d_min2 = m_Graph_X_start;
						d_max2 = m_Graph_X_end;
					}else{
						d_max2 = m_Graph_X_end;
						d_min2 = m_Graph_X_start;
					}
					if ((d_cross_y1 <= d_max1) && (d_cross_y1 >= d_min1) && 
						(d_cross_x1 <= d_max2) && (d_cross_x1 >= d_min2)){
						if (!b_point1){
							d_x1 = d_cross_x1;
							d_y1 = d_cross_y1;
							b_point1 = true;
						} else if (!b_point2){
							d_x2 = d_cross_x1;
							d_y2 = d_cross_y1;
							b_point2 = true;
						}
					}
				}
			}
			if (b_point1 && b_point2){
				last_x = wxCoord(d_x1);
				last_y = wxCoord(d_y1);
				xpos = wxCoord(d_x2);
				ypos = wxCoord(d_y2);
				if (last_x > (wxCoord)m_Graph_X_end) last_x = (wxCoord)m_Graph_X_end;
				if (last_x < 0) last_x = 0;
				if (last_y > (wxCoord)m_Graph_Y_end) last_y = (wxCoord)m_Graph_Y_end;
				if (last_y < 0) last_y = 0;
				if (xpos > (wxCoord)m_Graph_X_end) xpos = (wxCoord)m_Graph_X_end;
				if (xpos < 0) xpos = 0;
				if (ypos > (wxCoord)m_Graph_Y_end) ypos = (wxCoord)m_Graph_Y_end;
				if (ypos < 0) ypos = 0;

				dc.DrawLine(last_x, last_y, xpos, ypos);
				if (last_point_in) myDrawPoint(dc, last_x, last_y, graphColour, typePoint ,m_GraphPointWidth);
				if (point_in){
					d_end_point_x = d_xpos;
					d_end_point_y = d_ypos;
					end_point = true;
				}else end_point = false;
			}else end_point = false;
		}
		d_last_x = d_xpos;
		d_last_y = d_ypos;
		last_point_in = point_in;
	}
	// draw last point
	if (end_point){
		xpos = wxCoord(d_end_point_x);
		ypos = wxCoord(d_end_point_y);
		if (xpos > (wxCoord)m_Graph_X_end) xpos = (wxCoord)m_Graph_X_end;
		if (xpos < 0) xpos = 0;
		if (ypos > (wxCoord)m_Graph_Y_end) ypos = (wxCoord)m_Graph_Y_end;
		if (ypos < 0) ypos = 0;
		myDrawPoint(dc, xpos, ypos, graphColour, typePoint ,m_GraphPointWidth);
	}
	dc.DestroyClippingRegion();
}
//----Draw marker----
void CPaintStatistics::DrawMarker(wxDC &dc) {
	if (m_GraphMarker1){
		wxCoord x0 = wxCoord(m_Graph_X_start);
		wxCoord y0 = wxCoord(m_Graph_Y_start);
		wxCoord w0 = wxCoord(m_Graph_X_end - m_Graph_X_start);
		wxCoord h0 = wxCoord(m_Graph_Y_end - m_Graph_Y_start);
		if (x0 < 0) x0 = 0;
		if (y0 < 0) y0 = 0;
		if (w0 < 0) w0 = 0;
		if (h0 < 0) h0 = 0;
		dc.SetClippingRegion(x0, y0, w0, h0);

		dc.SetPen(wxPen(m_pen_MarkerLineColour , 1 , wxSOLID));
		wxCoord x00 = wxCoord(m_Ax_ValToCoord * m_GraphMarker_X1 + m_Bx_ValToCoord);
		wxCoord y00 = wxCoord(m_Ay_ValToCoord * m_GraphMarker_Y1 + m_By_ValToCoord);
		if (x00 < 0) x00 = 0;
		if (y00 < 0) y00 = 0;
		if ((x00 < wxCoord(m_Graph_X_start)) || (x00 > wxCoord(m_Graph_X_end)) ||
			(y00 < wxCoord(m_Graph_Y_start)) || (y00 > wxCoord(m_Graph_Y_end))){
		}else{
			dc.CrossHair(x00, y00);
			wxDateTime dtTemp1;
			wxString strBuffer1;
			dtTemp1.Set((time_t)m_GraphMarker_X1);
			strBuffer1=dtTemp1.Format(wxT("%d.%b.%y"), wxDateTime::GMT0);

			dc.SetFont(m_font_bold);
			dc.SetTextBackground (m_brush_AxisColour);
			dc.SetBackgroundMode(wxSOLID);
			x0 += 2;
			y0 += 2;
			x00 += 2;
			y00 += 2;
			if (x00 < 0) x00 = 0;
			if (y00 < 0) y00 = 0;
			if (x0 < 0) x0 = 0;
			if (y0 < 0) y0 = 0;

			dc.SetTextForeground (m_pen_AxisYTextColour);
			dc.DrawText(wxString::Format(wxT("%.2f"), m_GraphMarker_Y1) , x0, y00);
			dc.SetTextForeground (m_pen_AxisXTextColour);
			dc.DrawText(strBuffer1 ,x00, y0);
			dc.SetBackgroundMode(wxTRANSPARENT);
		}
		dc.DestroyClippingRegion();
	}
}
//-------- Draw All ---------
void CPaintStatistics::DrawAll(wxDC &dc) {
//Init global
	CMainDocument* pDoc = wxGetApp().GetDocument();

	wxASSERT(pDoc);
	wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	PROJECTS *proj = &(pDoc->statistics_status);
	wxASSERT(proj);

	m_WorkSpace_X_start = m_main_X_start;
	m_WorkSpace_X_end = m_main_X_end;
	m_WorkSpace_Y_start = m_main_Y_start;
	m_WorkSpace_Y_end = m_main_Y_end;

	dc.SetBackground(m_brush_MainColour);

//	dc.SetTextForeground (GetForegroundColour ());
	dc.SetTextForeground (m_pen_HeadTextColour);
	dc.SetTextBackground (GetBackgroundColour ());

	m_font_standart = dc.GetFont();
	m_font_bold = dc.GetFont();
	m_font_standart_italic = dc.GetFont();
	
	m_font_standart.SetWeight(wxNORMAL);
	m_font_bold.SetWeight(wxBOLD);
//	m_font_standart_italic.SetFaceName(_T("Verdana"));
	m_font_standart_italic.SetStyle(wxFONTSTYLE_ITALIC);

	dc.SetFont(m_font_standart);
//Start drawing
	dc.Clear();
	dc.SetBrush(wxBrush(m_brush_MainColour , wxSOLID));
	dc.SetPen(wxPen(m_pen_MainColour , 1 , wxSOLID));

	wxCoord x0 = wxCoord(m_main_X_start);
	wxCoord y0 = wxCoord(m_main_Y_start);
	wxCoord w0 = wxCoord(m_main_X_end - m_main_X_start);
	wxCoord h0 = wxCoord(m_main_Y_end - m_main_Y_start);
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (w0 < 0) w0 = 0;
	if (h0 < 0) h0 = 0;
	dc.DrawRectangle(x0, y0, w0, h0);
//Number of Projects
	int nb_proj = 0;
	for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) { ++nb_proj; }
	if (0 == nb_proj) {
		return;
	}
// Check m_NextProjectStatistic
	if (m_NextProjectStatistic < 0) m_NextProjectStatistic = nb_proj - 1;
	if ((m_NextProjectStatistic < 0) || (m_NextProjectStatistic >= nb_proj)) m_NextProjectStatistic = 0;
// Initial coord
	switch (m_SelectedStatistic){
	case 0: heading = _("User Total");		break;
	case 1: heading = _("User Average");	break;
	case 2: heading = _("Host Total");		break;
	case 3: heading = _("Host Average");	break;
	default:heading = wxT("");
	}

	switch (m_ModeViewStatistic){
	case 0:{
	//Draw Legend
		if (m_ViewHideProjectStatistic >= 0){
			int count = -1;
			std::set<wxString>::iterator s;
			for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
				++count;
				if (m_ViewHideProjectStatistic == count){
					s = m_HideProjectStatistic.find( wxString((*i)->master_url, wxConvUTF8) );
					if (s != m_HideProjectStatistic.end()){
						m_HideProjectStatistic.erase(s);
                    } else {
                        m_HideProjectStatistic.insert(wxString((*i)->master_url, wxConvUTF8));
                    }
					break;
				}
			}
		}
		m_ViewHideProjectStatistic = -1;

		if (m_LegendDraw) DrawLegend(dc, proj, pDoc, -1, false, m_Legend_Shift_Mode2);
	///Draw heading
		dc.SetFont(m_font_bold);
		DrawMainHead(dc, heading);
		dc.SetFont(m_font_standart);
	//How many rows/colums?
		int nb_proj_show = 0;
		for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
			if (!(m_HideProjectStatistic.count( wxString((*i)->master_url, wxConvUTF8) ))){
				++nb_proj_show;
			}
		}
//
		int nb_proj_row = 0, nb_proj_col = 0;
		if (nb_proj_show < 4) {
			nb_proj_col = 1;
			nb_proj_row = nb_proj_show;
		} else {
			nb_proj_col = 2;
			nb_proj_row = (int)ceil(double(nb_proj_show) / double(nb_proj_col));
		}

		int col = 1, row = 1; //Used to identify the actual row/col
	
		double rectangle_x_start = m_WorkSpace_X_start;
		double rectangle_x_end = m_WorkSpace_X_end;
		double rectangle_y_start = m_WorkSpace_Y_start;
		double rectangle_y_end = m_WorkSpace_Y_end;

		if (0 == nb_proj_col) nb_proj_col = 1;
		if (0 == nb_proj_row) nb_proj_row = 1;
		const double x_fac = (rectangle_x_end - rectangle_x_start) / double(nb_proj_col);
		const double y_fac = (rectangle_y_end - rectangle_y_start) / double(nb_proj_row);
	
		double min_val_y_all = 10e32, max_val_y_all = 0;
		double min_val_x_all = 10e32, max_val_x_all = 0;
		
		for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
			if (!(m_HideProjectStatistic.count( wxString((*i)->master_url, wxConvUTF8) ))){
				MinMaxDayCredit(i, min_val_y_all, max_val_y_all, min_val_x_all, max_val_x_all, m_SelectedStatistic, false);
			}
		}

		for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
			if (!(m_HideProjectStatistic.count( wxString((*i)->master_url, wxConvUTF8) ))){
			//Find minimum/maximum value
				double min_val_y = 10e32, max_val_y = 0;
				double min_val_x = 10e32, max_val_x = 0;
				MinMaxDayCredit(i, min_val_y, max_val_y, min_val_x, max_val_x, m_SelectedStatistic);
				CheckMinMaxD(min_val_x, max_val_x);
				CheckMinMaxD(min_val_y, max_val_y);
				min_val_x = floor(min_val_x / 86400.0) * 86400.0;
				max_val_x = ceil(max_val_x / 86400.0) * 86400.0;
			//Where do we draw in?
				ClearXY();
				m_main_X_start = (wxCoord)(rectangle_x_start + x_fac * (double)(col - 1));
				m_main_X_end = (wxCoord)(rectangle_x_start + x_fac * ((double)col));
				m_main_Y_start = (wxCoord)(rectangle_y_start + y_fac * (double)(row - 1));
				m_main_Y_end = (wxCoord)(rectangle_y_start + y_fac * (double)row);
				if (m_main_X_start < 0) m_main_X_start = 0;
				if (m_main_X_start > m_main_X_end) m_main_X_end = m_main_X_start;
				if (m_main_Y_start < 0) m_main_Y_start = 0;
				if (m_main_Y_start > m_main_Y_end) m_main_Y_end = m_main_Y_start;
				
				m_WorkSpace_X_start = m_main_X_start;
				m_WorkSpace_X_end = m_main_X_end;
				m_WorkSpace_Y_start = m_main_Y_start;
				m_WorkSpace_Y_end = m_main_Y_end;

			//Draw scale Draw Project name
				wxString head_name = wxT("?");
				PROJECT* state_project = pDoc->state.lookup_project((*i)->master_url);
				if (state_project) {
					head_name = wxString(state_project->project_name.c_str(), wxConvUTF8);
				}
			//Draw heading
				DrawMainHead(dc, head_name);
			//Draw axis
				DrawAxis(dc, max_val_y, min_val_y,max_val_x, min_val_x, m_pen_AxisColour, max_val_y_all, min_val_y_all);
			//Draw graph
				wxColour graphColour=wxColour(0,0,0);
				getDrawColour(graphColour,m_SelectedStatistic);
				DrawGraph(dc, i, graphColour, 0, m_SelectedStatistic);
			//Change row/col
				if (col == nb_proj_col) {
					col = 1;
					++row;
				} else {
					++col;
				}
			}
		}
		break;
		}
	case 1:{
	//Draw Legend
		if (m_LegendDraw) DrawLegend(dc, proj, pDoc, m_NextProjectStatistic, false, m_Legend_Shift_Mode1);
	//Draw heading
		dc.SetFont(m_font_bold);
		DrawMainHead(dc, heading);
		dc.SetFont(m_font_standart);
	//Draw project
		int count = -1;
		for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
			++count;
			if (count != m_NextProjectStatistic) continue;
		//Find minimum/maximum value
			double min_val_y = 10e32, max_val_y = 0;
			double min_val_x = 10e32, max_val_x = 0;

			MinMaxDayCredit(i, min_val_y, max_val_y, min_val_x, max_val_x, m_SelectedStatistic);

			double t_n1 = dtime();
			double t_d1 = floor((t_n1 - max_val_x) / 86400.0);

			wxString head_name=wxString::Format(_("Last update: %.0f days ago"), t_d1);
			
			wxColour pen_AxisColour1 = m_pen_AxisColourAutoZoom;

			if (m_Zoom_Auto){
				min_val_x = floor(min_val_x / 86400.0) * 86400.0;
				max_val_x = ceil(max_val_x / 86400.0) * 86400.0;
			}else{
				pen_AxisColour1 = m_pen_AxisColourZoom;
				min_val_x = m_Zoom_min_val_X;
				max_val_x = m_Zoom_max_val_X;
				min_val_y = m_Zoom_min_val_Y;
				max_val_y = m_Zoom_max_val_Y;
			}
			CheckMinMaxD(min_val_x, max_val_x);
			CheckMinMaxD(min_val_y, max_val_y);

		    // Draw heading
			PROJECT* state_project = pDoc->state.lookup_project((*i)->master_url);
			if (state_project) {
				dc.SetFont(m_font_standart_italic);
				DrawProjectHead(dc, state_project, head_name);
				dc.SetFont(m_font_standart);
			}
			m_Zoom_min_val_X = min_val_x;
			m_Zoom_max_val_X = max_val_x;
			m_Zoom_min_val_Y = min_val_y;
			m_Zoom_max_val_Y = max_val_y;
		    // Draw axis
			DrawAxis(dc, max_val_y, min_val_y, max_val_x, min_val_x, pen_AxisColour1, max_val_y, min_val_y);
		    // Draw graph
			wxColour graphColour=wxColour(0,0,0);
			getDrawColour(graphColour,m_SelectedStatistic);
			DrawGraph(dc, i, graphColour, 0, m_SelectedStatistic);
		    // Draw marker
			DrawMarker(dc);
			break;
		}
		break;
		}
	case 2:{
	//Draw Legend
		if (m_ViewHideProjectStatistic >= 0){
			int count = -1;
			std::set<wxString>::iterator s;
			for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
				++count;
				if (m_ViewHideProjectStatistic == count){
					s = m_HideProjectStatistic.find( wxString((*i)->master_url, wxConvUTF8) );
					if (s != m_HideProjectStatistic.end()){
						m_HideProjectStatistic.erase(s);
					}else m_HideProjectStatistic.insert( wxString((*i)->master_url, wxConvUTF8) );
					break;
				}
			}
		}
		m_ViewHideProjectStatistic = -1;

		if (m_LegendDraw) DrawLegend(dc, proj, pDoc, -1, true, m_Legend_Shift_Mode2);
	//Draw heading
		dc.SetFont(m_font_bold);
		DrawMainHead(dc, heading);
		dc.SetFont(m_font_standart);
	//Find minimum/maximum value
		double min_val_y = 10e32, max_val_y = 0;
		double min_val_x = 10e32, max_val_x = 0;
		
		wxColour pen_AxisColour1 = m_pen_AxisColourAutoZoom;

		if (m_Zoom_Auto){
			for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
				if (!(m_HideProjectStatistic.count( wxString((*i)->master_url, wxConvUTF8) ))){
					MinMaxDayCredit(i, min_val_y, max_val_y, min_val_x, max_val_x, m_SelectedStatistic, false);
				}
			}
			min_val_x = floor(min_val_x / 86400.0) * 86400.0;
			max_val_x = ceil(max_val_x / 86400.0) * 86400.0;
		}else{
			pen_AxisColour1 = m_pen_AxisColourZoom;
			min_val_x = m_Zoom_min_val_X;
			max_val_x = m_Zoom_max_val_X;
			min_val_y = m_Zoom_min_val_Y;
			max_val_y = m_Zoom_max_val_Y;
		}
		CheckMinMaxD(min_val_x, max_val_x);
		CheckMinMaxD(min_val_y, max_val_y);

		m_Zoom_min_val_X = min_val_x;
        m_Zoom_max_val_X = max_val_x;
		m_Zoom_min_val_Y = min_val_y;
		m_Zoom_max_val_Y = max_val_y;
	//Draw axis
		DrawAxis(dc, max_val_y, min_val_y, max_val_x, min_val_x, pen_AxisColour1, max_val_y, min_val_y);
	//Draw graph
		int count = -1;
		for (std::vector<PROJECT*>::const_iterator i = proj->projects.begin(); i != proj->projects.end(); ++i) {
			++count;
			if (!(m_HideProjectStatistic.count( wxString((*i)->master_url, wxConvUTF8) ))){
				wxColour graphColour = wxColour(0,0,0);
				int  typePoint = 0;
				getTypePoint(typePoint,count);
				getDrawColour(graphColour,count);
				DrawGraph(dc, i, graphColour, typePoint, m_SelectedStatistic);
			}
		}
	//Draw marker
		DrawMarker(dc);
		break;
		}
	default:{
		m_ModeViewStatistic = 0;
		break;
		}
	}
}
//=================================================================
void CPaintStatistics::OnPaint(wxPaintEvent& WXUNUSED(event)) {
	wxPaintDC pdc(this);
	wxMemoryDC mdc;
	wxCoord width = 0, height = 0;
	GetClientSize(&width, &height);
	if (m_full_repaint){
		if (!m_GraphZoomStart){
			ClearXY();
			ClearLegendXY();

			m_main_X_start = 0.0;
			if (width > 0) m_main_X_end = double(width); else m_main_X_end = 0.0;
			m_main_Y_start = 0.0;
			if (height > 0) m_main_Y_end = double(height); else m_main_Y_end = 0.0;

			if (width < 1) width = 1;
			if (height < 1) height = 1;
			m_dc_bmp.Create(width, height);
			mdc.SelectObject(m_dc_bmp);
			DrawAll(mdc);
			m_bmp_OK = true;
			m_full_repaint = false;
		}else if(m_bmp_OK){
			mdc.SelectObject(m_dc_bmp);
		}
	}else{
		if (m_bmp_OK){
			mdc.SelectObject(m_dc_bmp);
			if (m_GraphZoomStart && (width == m_dc_bmp.GetWidth()) &&(height == m_dc_bmp.GetHeight())){

				mdc.SetPen(wxPen(m_pen_ZoomRectColour , 1 , wxSOLID));
				mdc.SetBrush(wxBrush(m_brush_ZoomRectColour , wxSOLID));
				mdc.SetLogicalFunction(wxXOR);

				wxCoord x0 = 0;
				wxCoord y0 = 0;
				wxCoord w0 = 0;
				wxCoord h0 = 0;

				if (m_GraphZoom_X1 < m_GraphZoom_X2_old) x0 = m_GraphZoom_X1;
				else x0 = m_GraphZoom_X2_old;
				if (m_GraphZoom_Y1 < m_GraphZoom_Y2_old) y0 = m_GraphZoom_Y1;
				else y0 = m_GraphZoom_Y2_old;
				w0 = m_GraphZoom_X2_old - m_GraphZoom_X1;
				h0 = m_GraphZoom_Y2_old - m_GraphZoom_Y1;
				if (x0 < 0) x0 = 0;
				if (y0 < 0) y0 = 0;
				if (w0 < 0) w0 = -w0;
				if (h0 < 0) h0 = -h0;
				mdc.DrawRectangle(x0, y0, w0, h0);

				if (m_GraphZoom_X1 < m_GraphZoom_X2) x0 = m_GraphZoom_X1;
				else x0 = m_GraphZoom_X2;
				if (m_GraphZoom_Y1 < m_GraphZoom_Y2) y0 = m_GraphZoom_Y1;
				else y0 = m_GraphZoom_Y2;
				w0 = m_GraphZoom_X2 - m_GraphZoom_X1;
				h0 = m_GraphZoom_Y2 - m_GraphZoom_Y1;
				if (x0 < 0) x0 = 0;
				if (y0 < 0) y0 = 0;
				if (w0 < 0) w0 = -w0;
				if (h0 < 0) h0 = -h0;
				mdc.DrawRectangle(x0, y0, w0, h0);

				m_GraphZoom_X2_old = m_GraphZoom_X2;
				m_GraphZoom_Y2_old = m_GraphZoom_Y2;

				mdc.SetLogicalFunction(wxCOPY);
			}
		}
	}
	if (m_bmp_OK && (width == m_dc_bmp.GetWidth()) &&(height == m_dc_bmp.GetHeight())){
		pdc.Blit(0, 0, width, height,& mdc, 0, 0);         
	}
	mdc.SelectObject(wxNullBitmap);
}

void CPaintStatistics::OnLeftMouseDown(wxMouseEvent& event) {
// Legend
	if (m_Legend_dY > 0){
		wxClientDC dc (this);
		wxPoint pt(event.GetLogicalPosition(dc));
		if((double(pt.y) > m_Legend_select_Y_start) && (double(pt.y) < m_Legend_select_Y_end) && (double(pt.x) > m_Legend_select_X_start) && (double(pt.x) < m_Legend_select_X_end)){
			int i1 = (int)floor((double(pt.y) - m_Legend_select_Y_start) / m_Legend_dY);
			switch (m_ModeViewStatistic){
			case 1: 
				m_NextProjectStatistic = i1 + m_Legend_Shift_Mode1; 
				m_Zoom_Auto = true;
				m_GraphMarker1 = false;
				break;
			case 0:
			case 2:
				m_ViewHideProjectStatistic = i1 + m_Legend_Shift_Mode2; 
				break;
			}
			m_full_repaint = true;
			Refresh(false);
		    event.Skip();
			return;
		}
	}
// Graph
	switch (m_ModeViewStatistic){
	case 1:
	case 2:{
		wxClientDC dc (this);
		wxPoint pt(event.GetLogicalPosition(dc));
		if((double(pt.y) > m_Graph_Y_start) && (double(pt.y) < m_Graph_Y_end) && (double(pt.x) > m_Graph_X_start) && (double(pt.x) < m_Graph_X_end)){
			m_GraphMarker_X1 = m_Ax_CoordToVal * double(pt.x) + m_Bx_CoordToVal;
			m_GraphMarker_Y1 = m_Ay_CoordToVal * double(pt.y) + m_By_CoordToVal;
			m_GraphMarker1 = true;

			m_GraphZoom_X1 = wxCoord(pt.x);
			m_GraphZoom_Y1 = wxCoord(pt.y);
			m_GraphZoom_X2 = wxCoord(pt.x);
			m_GraphZoom_Y2 = wxCoord(pt.y);
			m_GraphZoom_X2_old = wxCoord(pt.x);
			m_GraphZoom_Y2_old = wxCoord(pt.y);

			m_GraphZoomStart = true;
		}
		break;
		}
	}
    event.Skip();
}

void CPaintStatistics::OnLeftMouseDoubleClick(wxMouseEvent& event) {
	m_LegendDraw = !m_LegendDraw;
	m_full_repaint = true;
	Refresh(false);
	event.Skip(); 
}

void CPaintStatistics::OnMouseMotion(wxMouseEvent& event) {
	switch (m_ModeViewStatistic){
	case 1:
	case 2:{
		if (m_GraphZoomStart){
			if (event.LeftIsDown()){
				wxClientDC cdc (this);
				wxPoint pt(event.GetLogicalPosition(cdc));
				if((double(pt.y) > m_Graph_Y_start) && (double(pt.y) < m_Graph_Y_end) && (double(pt.x) > m_Graph_X_start) && (double(pt.x) < m_Graph_X_end)){

				m_GraphZoom_X2 = wxCoord(pt.x);
				m_GraphZoom_Y2 = wxCoord(pt.y);

				m_full_repaint = false;
				Refresh(false);
				}
			}else{
				m_GraphZoomStart = false;

				m_full_repaint = true;
				Refresh(false);
			}
		}else if (m_GraphMoveStart){
			if (event.RightIsDown()){
				wxClientDC cdc (this);
				wxPoint pt(event.GetLogicalPosition(cdc));
				if((double(pt.y) > m_Graph_Y_start) && (double(pt.y) < m_Graph_Y_end) && (double(pt.x) > m_Graph_X_start) && (double(pt.x) < m_Graph_X_end)){

					m_GraphMove_X2 = wxCoord(pt.x);
					m_GraphMove_Y2 = wxCoord(pt.y);

					double X1 = m_Ax_CoordToVal * double(m_GraphMove_X1 - m_GraphMove_X2);
					double Y1 = m_Ay_CoordToVal * double(m_GraphMove_Y1 - m_GraphMove_Y2);

					if ( (X1 != 0) || (Y1 != 0)){
						m_GraphMove_X1 = m_GraphMove_X2;
						m_GraphMove_Y1 = m_GraphMove_Y2;
		
						m_Zoom_min_val_X = m_Zoom_min_val_X + X1;
						m_Zoom_max_val_X = m_Zoom_max_val_X + X1;
						m_Zoom_min_val_Y = m_Zoom_min_val_Y + Y1;
						m_Zoom_max_val_Y = m_Zoom_max_val_Y + Y1;

						m_GraphMoveGo = true;
						m_Zoom_Auto = false;
						m_full_repaint = true;
						Refresh(false);
					}

				}
			}else{
				m_GraphMoveStart = false;
				m_GraphMoveGo = false;

				m_full_repaint = true;
				Refresh(false);
			}
		}
		break;
		}
	}
    event.Skip();
}
void CPaintStatistics::OnLeftMouseUp(wxMouseEvent& event) {
	switch (m_ModeViewStatistic){
	case 1:
	case 2:{
		if (m_GraphZoomStart){
			if ((abs(int(m_GraphZoom_X1 - m_GraphZoom_X2)) > 2) && (abs(int(m_GraphZoom_Y1 - m_GraphZoom_Y2)) > 2)){
				double X1 = m_Ax_CoordToVal * double(m_GraphZoom_X1) + m_Bx_CoordToVal;
				double Y1 = m_Ay_CoordToVal * double(m_GraphZoom_Y1) + m_By_CoordToVal;
				double X2 = m_Ax_CoordToVal * double(m_GraphZoom_X2) + m_Bx_CoordToVal;
				double Y2 = m_Ay_CoordToVal * double(m_GraphZoom_Y2) + m_By_CoordToVal;

				if (X1 > X2){
					m_Zoom_max_val_X = X1;
					m_Zoom_min_val_X = X2;
				}else{
					m_Zoom_min_val_X = X1;
					m_Zoom_max_val_X = X2;
				}
				if (Y1 > Y2){
					m_Zoom_max_val_Y = Y1;
					m_Zoom_min_val_Y = Y2;
				}else{
					m_Zoom_min_val_Y = Y1;
					m_Zoom_max_val_Y = Y2;
				}
				m_GraphMarker1 = false;
				m_Zoom_Auto = false;
			}
			m_GraphZoomStart = false;
			m_full_repaint = true;
			Refresh(false);
		}
		break;
		}
	}
	event.Skip();
}
void CPaintStatistics::OnRightMouseDown(wxMouseEvent& event) {
	switch (m_ModeViewStatistic){
	case 1:
	case 2:{
		if (m_GraphZoomStart){       //???
			m_GraphZoomStart = false;
			m_GraphMarker1 = false;
			m_full_repaint = true;
			Refresh(false);
		}else{
			wxClientDC dc (this);
			wxPoint pt(event.GetLogicalPosition(dc));
			if((double(pt.y) > m_Graph_Y_start) && (double(pt.y) < m_Graph_Y_end) && (double(pt.x) > m_Graph_X_start) && (double(pt.x) < m_Graph_X_end)){
				m_GraphMove_X1 = wxCoord(pt.x);
				m_GraphMove_Y1 = wxCoord(pt.y);

				m_GraphMoveStart = true;
				m_GraphMoveGo = false;
			}
		}
		break;
		}
	}
    event.Skip();
}
void CPaintStatistics::OnRightMouseUp(wxMouseEvent& event) {
	if (m_GraphMoveGo){
		m_GraphMoveStart = false;
		m_GraphMoveGo = false;
	}else if (m_GraphMarker1){
		m_GraphMarker1 = false;
		m_full_repaint = true;
		Refresh(false);
	}else if (!m_Zoom_Auto){
		m_Zoom_Auto = true;
		m_full_repaint = true;
		Refresh(false);
	}
	event.Skip();
}
void CPaintStatistics::OnMouseLeaveWindows(wxMouseEvent& event) {
	if (m_GraphZoomStart){
		m_GraphMarker1 = false;
		m_GraphZoomStart = false;
		m_full_repaint = true;
		Refresh(false);
	}
	if (m_GraphMoveStart || m_GraphMoveGo){
		m_GraphMoveStart = false;
		m_GraphMoveGo = false;
	}
	event.Skip();
}
void CPaintStatistics::OnSize(wxSizeEvent& event) {
	m_full_repaint = true;
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

    UpdateSelection();

#ifdef __WXMAC__
    SetupMacAccessibilitySupport();
#endif
}

CViewStatistics::~CViewStatistics() {
    EmptyTasks();
#ifdef __WXMAC__
    RemoveMacAccessibilitySupport();
#endif
}

wxString& CViewStatistics::GetViewName() {
    static wxString strViewName(wxT("Statistics"));
    return strViewName;
}

wxString& CViewStatistics::GetViewDisplayName() {
    static wxString strViewName(_("Statistics"));
    return strViewName;
}

const char** CViewStatistics::GetViewIcon() {
    return stats_xpm;
}

const int CViewStatistics::GetViewRefreshRate() {
    return 60;
}

const int CViewStatistics::GetViewCurrentViewPage() {
    return VW_STAT;
}

void CViewStatistics::OnStatisticsUserTotal( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewStatistics::OnStatisticsUserTotal - Function Begin"));

    CAdvancedFrame* pFrame = wxDynamicCast(GetParent()->GetParent()->GetParent(), CAdvancedFrame);

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    pFrame->UpdateStatusText(_("Updating charts..."));
	m_PaintStatistics->m_SelectedStatistic = 0;
	m_PaintStatistics->m_Zoom_Auto = true;
	m_PaintStatistics->m_GraphMarker1 = false;
	m_PaintStatistics->m_full_repaint = true;
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
	m_PaintStatistics->m_SelectedStatistic = 1;
	m_PaintStatistics->m_Zoom_Auto = true;
	m_PaintStatistics->m_GraphMarker1 = false;
	m_PaintStatistics->m_full_repaint = true;
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
	m_PaintStatistics->m_SelectedStatistic = 2;
	m_PaintStatistics->m_Zoom_Auto = true;
	m_PaintStatistics->m_GraphMarker1 = false;
	m_PaintStatistics->m_full_repaint = true;
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
	m_PaintStatistics->m_SelectedStatistic = 3;
	m_PaintStatistics->m_Zoom_Auto = true;
	m_PaintStatistics->m_GraphMarker1 = false;
	m_PaintStatistics->m_full_repaint = true;
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
	m_PaintStatistics->m_ModeViewStatistic = 0;
	m_PaintStatistics->m_Zoom_Auto = true;
	m_PaintStatistics->m_GraphMarker1 = false;
	m_PaintStatistics->m_full_repaint = true;
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
	m_PaintStatistics->m_ModeViewStatistic = 1;
	m_PaintStatistics->m_Zoom_Auto = true;
	m_PaintStatistics->m_GraphMarker1 = false;
	m_PaintStatistics->m_full_repaint = true;
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
	m_PaintStatistics->m_ModeViewStatistic = 2;
	m_PaintStatistics->m_Zoom_Auto = true;
	m_PaintStatistics->m_GraphMarker1 = false;
	m_PaintStatistics->m_full_repaint = true;
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
	if (m_PaintStatistics->m_ModeViewStatistic == 1) m_PaintStatistics->m_NextProjectStatistic++;
	m_PaintStatistics->m_Zoom_Auto = true;
	m_PaintStatistics->m_GraphMarker1 = false;
	m_PaintStatistics->m_full_repaint = true;
	if (m_PaintStatistics->m_ModeViewStatistic == 0) m_PaintStatistics->m_Legend_Shift_Mode2++;
	if (m_PaintStatistics->m_ModeViewStatistic == 2) m_PaintStatistics->m_Legend_Shift_Mode2++;
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
	if (m_PaintStatistics->m_ModeViewStatistic == 1) m_PaintStatistics->m_NextProjectStatistic--;
	m_PaintStatistics->m_Zoom_Auto = true;
	m_PaintStatistics->m_GraphMarker1 = false;
	m_PaintStatistics->m_full_repaint = true;
	if (m_PaintStatistics->m_ModeViewStatistic == 0) m_PaintStatistics->m_Legend_Shift_Mode2--;
	if (m_PaintStatistics->m_ModeViewStatistic == 2) m_PaintStatistics->m_Legend_Shift_Mode2--;
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
    strBaseConfigLocation = wxT("/Statistics");
	pConfig->SetPath(strBaseConfigLocation);
	pConfig->Write(wxT("ModeViewStatistic"), m_PaintStatistics->m_ModeViewStatistic);
	pConfig->Write(wxT("SelectedStatistic"), m_PaintStatistics->m_SelectedStatistic);
	pConfig->Write(wxT("NextProjectStatistic"), m_PaintStatistics->m_NextProjectStatistic);
	strBaseConfigLocation = wxT("/Statistics/ViewAll");
	pConfig->DeleteGroup(strBaseConfigLocation);
	pConfig->SetPath(strBaseConfigLocation);
	int count = -1;
	for (std::set<wxString>::const_iterator i_s = m_PaintStatistics->m_HideProjectStatistic.begin(); i_s != m_PaintStatistics->m_HideProjectStatistic.end(); ++i_s) {
		++count;
		pConfig->Write(wxString::Format(wxT("%d"), count), (*i_s));
	}
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
    int     iTempValue = 0;
    wxString    strBaseConfigLocation = wxEmptyString;
    strBaseConfigLocation = wxT("/Statistics");
	pConfig->SetPath(strBaseConfigLocation);

	m_PaintStatistics->m_ModeViewStatistic = 0;
	pConfig->Read(wxT("ModeViewStatistic"), &iTempValue, -1);
	if ((iTempValue >= 0) && (iTempValue <= 2))m_PaintStatistics->m_ModeViewStatistic = iTempValue;

	m_PaintStatistics->m_SelectedStatistic = 0;
	pConfig->Read(wxT("SelectedStatistic"), &iTempValue, -1);
	if ((iTempValue >= 0) && (iTempValue <= 3))m_PaintStatistics->m_SelectedStatistic = iTempValue;

	m_PaintStatistics->m_NextProjectStatistic = 0;
	pConfig->Read(wxT("NextProjectStatistic"), &iTempValue, -1);
	if (iTempValue >= 0)m_PaintStatistics->m_NextProjectStatistic = iTempValue;
// -- Hide View All projects
	strBaseConfigLocation = wxT("/Statistics/ViewAll");
	pConfig->SetPath(strBaseConfigLocation);
	wxString tmpstr1;
	if (!(m_PaintStatistics->m_HideProjectStatistic.empty())) m_PaintStatistics->m_HideProjectStatistic.clear();
	for (int count = 0; count < 1000; ++count) {
		pConfig->Read(wxString::Format(wxT("%d"), count), &tmpstr1, wxT(""));
		if (tmpstr1 == wxEmptyString){ 
			break;
		}else{
			m_PaintStatistics->m_HideProjectStatistic.insert(tmpstr1);
		}
	}
    return true;
}

void CViewStatistics::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
	if (wxGetApp().GetDocument()->GetStatisticsCount()) {
		m_PaintStatistics->m_full_repaint = true;
		m_PaintStatistics->Refresh(false);
	}
}

void CViewStatistics::UpdateSelection() {
    CBOINCBaseView::PreUpdateSelection();
    CBOINCBaseView::PostUpdateSelection();
}


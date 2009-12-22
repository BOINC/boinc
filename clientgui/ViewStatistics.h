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

#ifndef _VIEWSTATISTICS_H_
#define _VIEWSTATISTICS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewStatistics.cpp"
#endif


#include "BOINCBaseView.h"

class CPaintStatistics : public wxWindow
{
public:
	CPaintStatistics();
	CPaintStatistics(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
	
	void DrawMainHead(wxDC &dc, const wxString head_name);
	
	void DrawProjectHead(wxDC &dc, PROJECT* project1, const wxString head_name_last);

	void DrawLegend(wxDC &dc, PROJECTS* proj, CMainDocument* pDoc, int SelProj, bool bColour, int &m_Legend_Shift);
	
	void DrawAxis(wxDC &dc, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x, wxColour pen_AxisColour, const double max_val_y_all, const double min_val_y_all);
	
	void DrawGraph(wxDC &dc, std::vector<PROJECT*>::const_iterator &i, const wxColour graphColour, const int typePoint, const int m_SelectedStatistic);
	
	void DrawMarker(wxDC &dc);

	void getDrawColour(wxColour &graphColour, int number);

	void ClearXY();

	void ClearLegendXY();
	
	void AB(const double x_coord1, const double y_coord1, const double x_coord2, const double y_coord2, const double x_val1, const double y_val1, const double x_val2, const double y_val2);
//--------------------------
	void DrawAll(wxDC &dc);
//--------------------------
    wxBitmap				m_dc_bmp;
	bool					m_full_repaint;
	bool                    m_bmp_OK;
//
    int                     m_SelectedStatistic;
    int                     m_ModeViewStatistic;
    int                     m_NextProjectStatistic;
    int                     m_ViewHideProjectStatistic;
	std::set<wxString>   m_HideProjectStatistic;
	double                  m_Legend_dY;
// Marker
	double                  m_GraphMarker_X1;
    double                  m_GraphMarker_Y1;
    bool                    m_GraphMarker1;
// Zoom
	wxCoord                  m_GraphZoom_X1;
	wxCoord                  m_GraphZoom_Y1;
	wxCoord                  m_GraphZoom_X2;
	wxCoord                  m_GraphZoom_Y2;
	wxCoord                  m_GraphZoom_X2_old;
	wxCoord                  m_GraphZoom_Y2_old;
	bool                     m_GraphZoomStart;

	wxCoord                  m_GraphMove_X1;
	wxCoord                  m_GraphMove_Y1;
	wxCoord                  m_GraphMove_X2;
	wxCoord                  m_GraphMove_Y2;
	bool                     m_GraphMoveStart;
	bool                     m_GraphMoveGo;

	double                  m_Zoom_max_val_X;
	double                  m_Zoom_min_val_X;
	double                  m_Zoom_max_val_Y;
	double                  m_Zoom_min_val_Y;
	bool                    m_Zoom_Auto;
// Shift Legend
	int                     m_Legend_Shift_Mode1;
	int                     m_Legend_Shift_Mode2;
	bool                    m_LegendDraw;
// old
    wxString                heading;
// X'=AX+B; Y'=AY+B;
    double                  m_Ax_ValToCoord;
    double                  m_Bx_ValToCoord;
    double                  m_Ay_ValToCoord;
    double                  m_By_ValToCoord;

	double                  m_Ax_CoordToVal;
    double                  m_Bx_CoordToVal;
    double                  m_Ay_CoordToVal;
    double                  m_By_CoordToVal;
// XY
    double                  m_WorkSpace_X_start;
    double                  m_WorkSpace_X_end;
	double                  m_WorkSpace_Y_start;
    double                  m_WorkSpace_Y_end;
//
	double                  m_main_X_start;
    double                  m_main_X_end;
	double                  m_main_Y_start;
    double                  m_main_Y_end;

    double                  m_Legend_X_start;
	double                  m_Legend_X_end;
    double                  m_Legend_Y_start;
    double                  m_Legend_Y_end;

    double                  m_Legend_select_X_start;
	double                  m_Legend_select_X_end;
    double                  m_Legend_select_Y_start;
    double                  m_Legend_select_Y_end;

    double                  m_Graph_X_start;
	double                  m_Graph_X_end;
    double                  m_Graph_Y_start;
    double                  m_Graph_Y_end;

    double                  m_Graph_draw_X_start;
	double                  m_Graph_draw_X_end;
    double                  m_Graph_draw_Y_start;
    double                  m_Graph_draw_Y_end;
// View
	int					    m_GraphLineWidth;
    int                     m_GraphPointWidth;

    wxFont                  m_font_standart;
    wxFont                  m_font_bold;
    wxFont                  m_font_standart_italic;
// colour
    wxColour                m_pen_MarkerLineColour;
    wxColour                m_pen_ZoomRectColour;
    wxColour                m_brush_ZoomRectColour;

	wxColour                m_brush_AxisColour;
    wxColour                m_pen_AxisColour;
    wxColour                m_pen_AxisColourAutoZoom;
    wxColour                m_pen_AxisColourZoom;
    wxColour                m_pen_AxisXColour;
    wxColour                m_pen_AxisYColour;
    wxColour                m_pen_AxisXTextColour;
    wxColour                m_pen_AxisYTextColour;

    wxColour                m_brush_LegendColour;
    wxColour                m_brush_LegendSelectColour;
    wxColour                m_pen_LegendSelectColour;
    wxColour                m_pen_LegendSelectTextColour;
    wxColour                m_pen_LegendColour;
    wxColour                m_pen_LegendTextColour;

    wxColour                m_brush_MainColour;
    wxColour                m_pen_MainColour;

    wxColour                m_pen_HeadTextColour;
    wxColour                m_pen_ProjectHeadTextColour;

    wxColour                m_pen_GraphTotalColour;
    wxColour                m_pen_GraphRACColour;
    wxColour                m_pen_GraphTotalHostColour;
    wxColour                m_pen_GraphRACHostColour;

    wxColour                m_pen_GraphColour00;
    wxColour                m_pen_GraphColour01;
    wxColour                m_pen_GraphColour02;
    wxColour                m_pen_GraphColour03;
    wxColour                m_pen_GraphColour04;
    wxColour                m_pen_GraphColour05;
    wxColour                m_pen_GraphColour06;
    wxColour                m_pen_GraphColour07;
    wxColour                m_pen_GraphColour08;
    wxColour                m_pen_GraphColour09;
protected:
    void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent & /*event*/){};
    void OnSize(wxSizeEvent& event);
    void OnLeftMouseDown(wxMouseEvent& event);
    void OnLeftMouseUp(wxMouseEvent& event);
	void OnLeftMouseDoubleClick(wxMouseEvent& event);
	void OnMouseMotion(wxMouseEvent& event);
    void OnRightMouseDown(wxMouseEvent& event);
    void OnRightMouseUp(wxMouseEvent& event);
	void OnMouseLeaveWindows(wxMouseEvent& event);

	DECLARE_EVENT_TABLE()
};

class CViewStatistics : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewStatistics )
	DECLARE_EVENT_TABLE()

public:
    CViewStatistics();
    CViewStatistics(wxNotebook* pNotebook);

    ~CViewStatistics();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();
    virtual const int       GetViewRefreshRate();
    virtual const int       GetViewCurrentViewPage();

    void                    OnStatisticsUserTotal( wxCommandEvent& event );
    void                    OnStatisticsUserAverage( wxCommandEvent& event );
    void                    OnStatisticsHostTotal( wxCommandEvent& event );
    void                    OnStatisticsHostAverage( wxCommandEvent& event );
    void                    OnStatisticsNextProject( wxCommandEvent& event );
    void                    OnStatisticsPrevProject( wxCommandEvent& event );
    void                    OnStatisticsModeView0( wxCommandEvent& event );
    void                    OnStatisticsModeView1( wxCommandEvent& event );
    void                    OnStatisticsModeView2( wxCommandEvent& event );

protected:

    CPaintStatistics*       m_PaintStatistics;

#ifdef __WXMAC__
    void                    SetupMacAccessibilitySupport();
    void                    RemoveMacAccessibilitySupport();
    
    EventHandlerRef         m_pStatisticsAccessibilityEventHandlerRef;
#endif

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            OnListRender( wxTimerEvent& event );

    virtual void            UpdateSelection();
};


#endif


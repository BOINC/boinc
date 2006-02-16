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

#ifndef _VIEWSTATISTICS_H_
#define _VIEWSTATISTICS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewStatistics.cpp"
#endif


#include "BOINCBaseView.h"


class CPaintStatistics : public wxPanel
{
public:
	CPaintStatistics();
	CPaintStatistics(wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "panel");
	
	void DrawMainHead(wxPaintDC &dc, const wxString head_name, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord &y_end);
	
	void DrawLegend(wxPaintDC &dc, PROJECTS * &proj, CMainDocument* &pDoc, wxInt32 SelProj, bool bColour, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord &y_end);
	
	void DrawAxis(wxPaintDC &dc, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x, wxCoord &x_start, wxCoord &x_end, wxCoord &y_start, wxCoord &y_end);
	
	void DrawGraph(wxPaintDC &dc, std::vector<PROJECT*>::const_iterator &i, const wxCoord x_start, const wxCoord x_end, const wxCoord y_start, const wxCoord y_end, const wxColour grafColour, const wxInt32 typePoint, const wxInt32 m_SelectedStatistic, const double max_val_y, const double min_val_y, const double max_val_x, const double min_val_x);
    
    wxString                heading;
    
    wxInt32                 m_SelectedStatistic;
    wxInt32                 m_ModeViewStatistic;
    wxInt32                 m_NextProjectStatistic;
    
    wxInt32                 m_GraphLineWidth;
    wxInt32                 m_GraphPointWidth;
    wxColour                m_brushAxisColour;
    wxColour                m_ligthbrushAxisColour;
    wxColour                m_penAxisColour;
    wxFont                  m_font_stdandart;
    wxFont                  m_font_bold;

protected:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);

	DECLARE_EVENT_TABLE()
};

class CViewStatistics : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewStatistics )

public:
    CViewStatistics();
    CViewStatistics(wxNotebook* pNotebook);

    ~CViewStatistics();

    virtual wxString&       GetViewName();
    virtual const char**    GetViewIcon();

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

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            OnListRender( wxTimerEvent& event );

    virtual void            UpdateSelection();

	DECLARE_EVENT_TABLE()
};


#endif


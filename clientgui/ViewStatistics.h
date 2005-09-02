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
	
	wxInt32                 m_SelectedStatistic;
	wxString                heading;

protected:
	void OnPaint(wxPaintEvent& event);

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

    void                    OnStatisticsUserTotal( wxCommandEvent& event );
    void                    OnStatisticsUserAverage( wxCommandEvent& event );
    void                    OnStatisticsHostTotal( wxCommandEvent& event );
    void                    OnStatisticsHostAverage( wxCommandEvent& event );

protected:

	CPaintStatistics*       m_PaintStatistics;

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            OnListRender( wxTimerEvent& event );

    virtual void            UpdateSelection();

	DECLARE_EVENT_TABLE()
};


#endif


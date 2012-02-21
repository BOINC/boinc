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

#ifndef _VIEWRESOURCES_H_
#define _VIEWRESOURCES_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewResources.cpp"
#endif


#include "BOINCBaseView.h"
#include "common/wxPieCtrl.h"
#include <wx/dynarray.h>

WX_DECLARE_OBJARRAY(wxColour, wxArrayColour);

class CViewResources : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewResources )
	DECLARE_EVENT_TABLE()

public:
    CViewResources();
    CViewResources(wxNotebook* pNotebook);

    ~CViewResources();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();
    virtual const int       GetViewRefreshRate();
    virtual const int       GetViewCurrentViewPage();

protected:

	wxPieCtrl*				m_pieCtrlBOINC;
	wxPieCtrl*				m_pieCtrlTotal;

	bool					m_BOINCwasEmpty;

    virtual void            UpdateSelection();

    wxInt32                 FormatProjectName(PROJECT*, wxString& strBuffer ) const;
	wxInt32					FormatDiskSpace(double bytes, wxString& strBuffer) const;

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );
	virtual void            OnListRender( wxTimerEvent& event );
};


#endif


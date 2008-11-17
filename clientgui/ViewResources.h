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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
#ifdef __WXMAC__
    virtual const int       GetViewRefreshRate();
#endif

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


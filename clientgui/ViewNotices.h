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

#ifndef _VIEWNOTIFICATIONS_H_
#define _VIEWNOTIFICATIONS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewNotices.cpp"
#endif


#include "BOINCBaseView.h"


class CNoticeListCtrl;
class NoticeListCtrlEvent;


class CViewNotices : public CBOINCBaseView {
    DECLARE_DYNAMIC_CLASS( CViewNotices )

public:
    CViewNotices();
    CViewNotices(wxNotebook* pNotebook);

    ~CViewNotices();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();
    virtual const int       GetViewRefreshRate();
    virtual const int       GetViewCurrentViewPage();

protected:
	CNoticeListCtrl*        m_pHtmlListPane;

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            OnListRender( wxTimerEvent& event );
    void                    OnLinkClicked( NoticeListCtrlEvent& event );

    DECLARE_EVENT_TABLE()
};


#endif


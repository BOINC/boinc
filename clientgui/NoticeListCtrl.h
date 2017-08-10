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
//
#ifndef BOINC_NOTICELISTCTRL_H
#define BOINC_NOTICELISTCTRL_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "NoticeListCtrl.cpp"
#endif

/*!
 * CNoticeListCtrl class declaration
 */

class CNoticeListCtrl: public wxWindow
{
    DECLARE_DYNAMIC_CLASS( CNoticeListCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CNoticeListCtrl( );
    CNoticeListCtrl( wxWindow* parent );
    ~CNoticeListCtrl();

    /// Creation
    bool    Create( wxWindow* parent );

    int     GetItemCount();
    void    SetItemCount(int newCount);
    
////@begin CNoticeListCtrl event handler declarations

    void OnLinkClicked( wxWebViewEvent& event );
    void OnWebViewError( wxWebViewEvent& event );

////@end CNoticeListCtrl event handler declarations

    void    Clear();
    bool    UpdateUI();

    bool        m_bDisplayFetchingNotices;
    bool        m_bDisplayEmptyNotice;
private:
    wxWebView*  m_browser;
    bool        m_bNeedsReloading;
    int         m_itemCount;
    wxString    m_noticesBody;
};

#endif

// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

/*!
 * CNoticeListCtrl class declaration
 */

class CNoticeListCtrl: public wxWindow
{
    DECLARE_DYNAMIC_CLASS( CNoticeListCtrl )

public:
    /// Constructors
    CNoticeListCtrl() = default;
    CNoticeListCtrl( wxWindow* parent );
    ~CNoticeListCtrl();

    /// Creation
    bool    Create( wxWindow* parent );

    int     GetItemCount();
    void    SetItemCount(int newCount);

////@begin CNoticeListCtrl event handler declarations
#if wxUSE_WEBVIEW
    void OnLinkClicked( wxWebViewEvent& event );
    void OnWebViewError( wxWebViewEvent& event );
#else
    void OnLinkClicked( wxHtmlLinkEvent & event );
#endif

////@end CNoticeListCtrl event handler declarations

    void    Clear();
    bool    UpdateUI();

    bool        m_bDisplayFetchingNotices = false;
    bool        m_bDisplayEmptyNotice = false;
private:
#if wxUSE_WEBVIEW
    wxWebView*  m_browser = nullptr;
#else
    wxHtmlWindow* m_browser = nullptr;
#endif
    bool        m_bNeedsReloading = false;
    int         m_itemCount = 0;
    wxString    m_noticesBody;
};

#endif

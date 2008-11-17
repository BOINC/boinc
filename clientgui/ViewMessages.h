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

#ifndef _VIEWMESSAGES_H_
#define _VIEWMESSAGES_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewMessages.cpp"
#endif


#include "BOINCBaseView.h"

class CBOINCBaseView;

class CViewMessages : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewMessages )
    DECLARE_EVENT_TABLE()

public:
    CViewMessages();
    CViewMessages(wxNotebook* pNotebook);

    ~CViewMessages();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();

    void                    OnMessagesCopyAll( wxCommandEvent& event );
    void                    OnMessagesCopySelected( wxCommandEvent& event );

protected:

    wxInt32                 m_iPreviousDocCount;

    wxListItemAttr*         m_pMessageInfoAttr;
    wxListItemAttr*         m_pMessageErrorAttr;

    virtual void            OnListRender( wxTimerEvent& event );

    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    virtual bool            EnsureLastItemVisible();

    virtual void            UpdateSelection();

    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatMessage( wxInt32 item, wxString& strBuffer ) const;

#ifdef wxUSE_CLIPBOARD
    bool                    m_bClipboardOpen;
    wxString                m_strClipboardData;
    bool                    OpenClipboard();
    wxInt32                 CopyToClipboard( wxInt32 item );
    bool                    CloseClipboard();
#endif
};

#endif


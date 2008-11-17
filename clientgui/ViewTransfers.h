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

#ifndef _VIEWTRANSFERS_H_
#define _VIEWTRANSFERS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewTransfers.cpp"
#endif


#include "BOINCBaseView.h"


class CTransfer : public wxObject {
public:
	CTransfer();
	~CTransfer();

	wxString m_strProjectName;
    wxString m_strFileName;
    wxString m_strProgress;
    wxString m_strSize;
    wxString m_strTime;
    wxString m_strSpeed;
    wxString m_strStatus;
};


class CViewTransfers : public CBOINCBaseView {
    DECLARE_DYNAMIC_CLASS( CViewTransfers )

public:
    CViewTransfers();
    CViewTransfers(wxNotebook* pNotebook);

    ~CViewTransfers();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();

    void                    OnTransfersRetryNow( wxCommandEvent& event );
    void                    OnTransfersAbort( wxCommandEvent& event );

protected:
    std::vector<CTransfer*> m_TransferCache;

    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;

    virtual wxString        OnDocGetItemText( long item, long column ) const;

    virtual wxInt32         AddCacheElement();
    virtual wxInt32         EmptyCache();
    virtual wxInt32         GetCacheCount();
    virtual wxInt32         RemoveCacheElement();
    virtual wxInt32         UpdateCache( long item, long column, wxString& strNewData );

    virtual void            UpdateSelection();

    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatFileName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatProgress( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatSize( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatSpeed( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatStatus( wxInt32 item, wxString& strBuffer ) const;

    DECLARE_EVENT_TABLE()
};


#endif


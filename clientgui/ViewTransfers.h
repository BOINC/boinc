// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

#ifndef BOINC_VIEWTRANSFERS_H
#define BOINC_VIEWTRANSFERS_H

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
    double m_fProgress;
    double m_fBytesXferred;
    double m_fTotalBytes;
    double m_dTime;
    double m_fTimeToCompletion;
    double m_dSpeed;
    wxString m_strStatus;
    wxString m_strProjectURL;   // Used internally, not displayed
    wxString m_strProgress;
    wxString m_strSize;
    wxString m_strTime;
    wxString m_strTimeToCompletion;
    wxString m_strSpeed;
};


class CViewTransfers : public CBOINCBaseView {
    DECLARE_DYNAMIC_CLASS( CViewTransfers )

public:
    CViewTransfers();
    CViewTransfers(wxNotebook* pNotebook);

    ~CViewTransfers();

    void                    AppendColumn(int columnID);
    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();
    virtual int             GetViewCurrentViewPage();

    virtual wxString        GetKeyValue1(int iRowIndex);
    virtual wxString        GetKeyValue2(int iRowIndex);
    virtual int             FindRowIndexByKeyValues(wxString& key1, wxString& key2);

    void                    OnTransfersRetryNow( wxCommandEvent& event );
    void                    OnTransfersAbort( wxCommandEvent& event );
    void                    OnColResize( wxListEvent& event);

    std::vector<CTransfer*> m_TransferCache;

protected:
    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;

    virtual wxInt32         AddCacheElement();
    virtual wxInt32         EmptyCache();
    virtual wxInt32         GetCacheCount();
    virtual wxInt32         RemoveCacheElement();
    virtual bool            SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex);

    virtual bool            IsSelectionManagementNeeded();

    virtual void            UpdateSelection();

    void                    GetDocProjectName(wxInt32 item, wxString& strBuffer) const;
    void                    GetDocFileName(wxInt32 item, wxString& strBuffer) const;
    void                    GetDocProgress(wxInt32 item, double& fBuffer) const;
    wxInt32                 FormatProgress( double fBuffer, wxString& strBuffer ) const;
    void                    GetDocBytesXferred(wxInt32 item, double& fBuffer) const;
    void                    GetDocTotalBytes(wxInt32 item, double& fBuffer) const;
    wxInt32                 FormatSize( double fBytesSent, double fFileSize, wxString& strBuffer ) const;
    void                    GetDocTime(wxInt32 item, double& fBuffer) const;
    void                    GetDocTimeToCompletion(wxInt32 item, double& fBuffer) const;
    void                    GetDocSpeed(wxInt32 item, double& fBuffer) const;
    wxInt32                 FormatSpeed( double fBuffer, wxString& strBuffer ) const;
    void                    GetDocStatus(wxInt32 item, wxString& strBuffer) const;
    void                    GetDocProjectURL(wxInt32 item, wxString& strBuffer) const;

    virtual double          GetProgressValue(long item);
    virtual wxString        GetProgressText( long item);

    int                     GetTransferCacheAtIndex(CTransfer*& transferPtr, int index);

    DECLARE_EVENT_TABLE()
};

#endif

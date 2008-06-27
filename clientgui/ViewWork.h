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

#ifndef _VIEWWORK_H_
#define _VIEWWORK_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewWork.cpp"
#endif


#include "BOINCBaseView.h"


class CWork : public wxObject
{
public:
    CWork();
    ~CWork();

    wxString m_strProjectName;
    wxString m_strApplicationName;
    wxString m_strName;
    float m_fCPUTime;
    float m_fProgress;
    float m_fTimeToCompletion;
    time_t m_tReportDeadline;
    wxString m_strStatus;
};


class CViewWork : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewWork )

public:
    CViewWork();
    CViewWork(wxNotebook* pNotebook);

    ~CViewWork();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();

    void                    OnWorkSuspend( wxCommandEvent& event );
    void                    OnWorkShowGraphics( wxCommandEvent& event );
    void                    OnWorkAbort( wxCommandEvent& event );

    void                    OnProjectWebsiteClicked( wxEvent& event );
    
    std::vector<CWork*>     m_WorkCache;

protected:

    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;

    virtual wxInt32         AddCacheElement();
    virtual wxInt32         EmptyCache();
    virtual wxInt32         GetCacheCount();
    virtual wxInt32         RemoveCacheElement();
    virtual bool            SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex);
    virtual void            UpdateSelection();

    void                    GetDocProjectName(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocApplicationName(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatApplicationName( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocName(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatName( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocCPUTime(wxInt32 item, float& fBuffer) const;
    wxInt32                 FormatCPUTime( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocProgress(wxInt32 item, float& fBuffer) const;
    wxInt32                 FormatProgress( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocTimeToCompletion(wxInt32 item, float& fBuffer) const;
    wxInt32                 FormatTimeToCompletion( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocReportDeadline(wxInt32 item, time_t& time) const;
    wxInt32                 FormatReportDeadline( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocStatus(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatStatus( wxInt32 item, wxString& strBuffer ) const;

    virtual double          GetProgressValue(long item);

    DECLARE_EVENT_TABLE()
};


#endif


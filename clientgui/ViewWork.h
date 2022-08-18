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

#ifndef BOINC_VIEWWORK_H
#define BOINC_VIEWWORK_H

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
    double m_fCPUTime;
    double m_fProgress;
    double m_fTimeToCompletion;
    double m_fDeadlineDiff;
    time_t m_tReportDeadline;
    time_t m_tEstimatedCompletion;
    wxString m_strStatus;
    wxString m_strProjectURL;   // Used internally, not displayed
    wxString m_strCPUTime;
    wxString m_strProgress;
    wxString m_strTimeToCompletion;
    wxString m_strReportDeadline;
    wxString m_strEstimatedCompletion;
    wxString m_strDeadlineDiff;
};


class CViewWork : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewWork )

public:
    CViewWork();
    CViewWork(wxNotebook* pNotebook);

    ~CViewWork();

    void                    AppendColumn(int columnID);
    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();
    virtual int             GetViewCurrentViewPage();

    virtual wxString        GetKeyValue1(int iRowIndex);
    virtual wxString        GetKeyValue2(int iRowIndex);
    virtual int             FindRowIndexByKeyValues(wxString& key1, wxString& key2);

    void                    OnWorkSuspend( wxCommandEvent& event );
    void                    OnWorkShowGraphics( wxCommandEvent& event );
    void                    OnWorkShowVMConsole( wxCommandEvent& event );
    void                    OnWorkAbort( wxCommandEvent& event );
    void                    OnShowItemProperties( wxCommandEvent& event );
    void                    OnActiveTasksOnly( wxCommandEvent& event );

    void                    OnProjectWebsiteClicked( wxEvent& event );
    void                    OnColResize( wxListEvent& event);

    std::vector<CWork*>     m_WorkCache;

protected:

    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual wxInt32         AddCacheElement();
    virtual wxInt32         EmptyCache();
    virtual wxInt32         GetCacheCount();
    virtual wxInt32         RemoveCacheElement();
    virtual bool            SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex);

    virtual bool            IsSelectionManagementNeeded();

    virtual void            UpdateSelection();

    void                    GetDocProjectName(wxInt32 item, wxString& strBuffer) const;
    void                    GetDocApplicationName(wxInt32 item, wxString& strBuffer) const;
    void                    GetDocName(wxInt32 item, wxString& strBuffer) const;
    void                    GetDocCPUTime(wxInt32 item, double& fBuffer) const;
    void                    GetDocProgress(wxInt32 item, double& fBuffer) const;
    wxInt32                 FormatProgress( double fBuffer, wxString& strBuffer ) const;
    void                    GetDocTimeToCompletion(wxInt32 item, double& fBuffer) const;
    void                    GetDocReportDeadline(wxInt32 item, time_t& tBuffer) const;
    void                    GetDocEstCompletionDate(wxInt32 item, time_t& tBuffer) const;
    void                    GetDocEstDeadlineDiff(wxInt32 item, double& fBuffer) const;
    wxInt32                 FormatDateTime( time_t datetime, wxString& strBuffer ) const;
    wxInt32                 FormatStatus( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocProjectURL(wxInt32 item, wxString& strBuffer) const;
    virtual double          GetProgressValue(long item);
    virtual wxString        GetProgressText( long item);

    int                     GetWorkCacheAtIndex(CWork*& workPtr, int index);

    DECLARE_EVENT_TABLE()
};

#endif

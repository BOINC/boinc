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

#ifndef _VIEWPROJECTS_H_
#define _VIEWPROJECTS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewProjects.cpp"
#endif


#include "BOINCBaseView.h"


class CProject : public wxObject
{
public:
	CProject();
	~CProject();

    wxString m_strProjectName;
    wxString m_strAccountName;
    wxString m_strTeamName;
    float m_fTotalCredit;
    float m_fAVGCredit;
    float m_fResourceShare;
    float m_fResourcePercent;
    wxString m_strStatus;
    wxString m_strProjectURL;   // Used internally, not displayed
    wxString m_strTotalCredit;
    wxString m_strAVGCredit;
    wxString m_strResourceShare;
};


class CViewProjects : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewProjects )

public:
    CViewProjects();
    CViewProjects(wxNotebook* pNotebook);

    ~CViewProjects();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();
    virtual const int       GetViewCurrentViewPage();

    virtual wxString        GetKeyValue1(int iRowIndex);
    virtual int             FindRowIndexByKeyValues(wxString& key1, wxString& key2);

    void                    OnProjectUpdate( wxCommandEvent& event );
    void                    OnProjectSuspend( wxCommandEvent& event );
    void                    OnProjectNoNewWork( wxCommandEvent& event );
    void                    OnProjectReset( wxCommandEvent& event );
    void                    OnProjectDetach( wxCommandEvent& event );
    void                    OnShowItemProperties( wxCommandEvent& event );

    void                    OnProjectWebsiteClicked( wxEvent& event );

    std::vector<CProject*>  m_ProjectCache;

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
    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocAccountName(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatAccountName( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocTeamName(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatTeamName( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocTotalCredit(wxInt32 item, float& fBuffer) const;
    wxInt32                 FormatTotalCredit( float fBuffer, wxString& strBuffer ) const;
    void                    GetDocAVGCredit(wxInt32 item, float& fBuffer) const;
    wxInt32                 FormatAVGCredit( float fBuffer, wxString& strBuffer ) const;
    void                    GetDocResourceShare(wxInt32 item, float& fBuffer) const;
    void                    GetDocResourcePercent(wxInt32 item, float& fBuffer) const;
    wxInt32                 FormatResourceShare( float fBuffer, float fBufferPercent, wxString& strBuffer ) const;
    void                    GetDocStatus(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatStatus( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocProjectURL(wxInt32 item, wxString& strBuffer) const;

    virtual double          GetProgressValue(long item);
    virtual wxString        GetProgressText( long item);

    bool                    IsWebsiteLink( const wxString& strLink );
    wxInt32                 ConvertWebsiteIndexToLink( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strLink );
    wxInt32                 ConvertLinkToWebsiteIndex( const wxString& strLink, wxInt32& iProjectIndex, wxInt32& iWebsiteIndex );

    int                     GetProjectCacheAtIndex(CProject*& projectPtr, int index);

    DECLARE_EVENT_TABLE()
};


#endif


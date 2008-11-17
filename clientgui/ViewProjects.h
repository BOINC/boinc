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
    wxString m_strTotalCredit;
    wxString m_strAVGCredit;
    wxString m_strResourceShare;
    wxString m_strStatus;
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

    void                    OnProjectUpdate( wxCommandEvent& event );
    void                    OnProjectSuspend( wxCommandEvent& event );
    void                    OnProjectNoNewWork( wxCommandEvent& event );
    void                    OnProjectReset( wxCommandEvent& event );
    void                    OnProjectDetach( wxCommandEvent& event );

    void                    OnProjectWebsiteClicked( wxEvent& event );

protected:
    std::vector<CProject*>  m_ProjectCache;

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
    wxInt32                 FormatAccountName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTeamName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTotalCredit( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatAVGCredit( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatResourceShare( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatStatus( wxInt32 item, wxString& strBuffer ) const;

    bool                    IsWebsiteLink( const wxString& strLink );
    wxInt32                 ConvertWebsiteIndexToLink( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strLink );
    wxInt32                 ConvertLinkToWebsiteIndex( const wxString& strLink, wxInt32& iProjectIndex, wxInt32& iWebsiteIndex );

    DECLARE_EVENT_TABLE()
};


#endif


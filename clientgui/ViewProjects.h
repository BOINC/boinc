// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//

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

	wxInt32  GetProjectName( wxString& strProjectName );
	wxInt32  GetAccountName( wxString& strAccountName );
	wxInt32  GetTeamName( wxString& strTeamName );
	wxInt32  GetTotalCredit( wxString& strTotalCredit );
	wxInt32  GetAVGCredit( wxString& strAVGCredit );
	wxInt32  GetResourceShare( wxString& strResourceShare );
	wxInt32  GetStatus( wxString& strStatus );

	wxInt32  SetProjectName( wxString& strProjectName );
	wxInt32  SetAccountName( wxString& strAccountName );
	wxInt32  SetTeamName( wxString& strTeamName );
	wxInt32  SetTotalCredit( wxString& strTotalCredit );
	wxInt32  SetAVGCredit( wxString& strAVGCredit );
	wxInt32  SetResourceShare( wxString& strResourceShare );
	wxInt32  SetStatus( wxString& strStatus );

protected:
    wxString m_strProjectName;
    wxString m_strAccountName;
    wxString m_strTeamName;
    wxString m_strTotalCredit;
    wxString m_strAVGCredit;
    wxString m_strResourceShare;
    wxString m_strStatus;
};

WX_DECLARE_OBJARRAY( CProject, CProjectCache );


class CViewProjects : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewProjects )

public:
    CViewProjects();
    CViewProjects(wxNotebook* pNotebook);

    ~CViewProjects();

    virtual wxString        GetViewName();
    virtual char**          GetViewIcon();

protected:

    bool                    m_bTaskHeaderHidden;
    bool                    m_bTaskAttachHidden;
    bool                    m_bTaskDetachHidden;
    bool                    m_bTaskResetHidden;
    bool                    m_bTaskSuspendHidden;
    bool                    m_bTaskResumeHidden;
    bool                    m_bTaskUpdateHidden;

    bool                    m_bWebsiteHeaderHidden;
    bool                    m_bWebsiteBOINCHidden;
    bool                    m_bWebsiteProjectHidden;

    bool                    m_bTipsHeaderHidden;

    bool                    m_bItemSelected;

	CProjectCache           m_ProjectCache;

    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;

    virtual wxString        OnDocGetItemText( long item, long column ) const;

    virtual void            OnTaskLinkClicked( const wxHtmlLinkInfo& link );
    virtual void            OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y );

    virtual wxInt32         AddCacheElement();
    virtual wxInt32         EmptyCache();
    virtual wxInt32         GetCacheCount();
    virtual wxInt32         RemoveCacheElement();
    virtual wxInt32         UpdateCache( long item, long column, wxString& strNewData );

    virtual void            UpdateSelection();
    virtual void            UpdateTaskPane();

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
    void                    ExecuteLink( const wxString& strLink );


    //
    // Globalization/Localization
    //
    wxString                VIEW_HEADER;

    wxString                SECTION_TASK;
    wxString                SECTION_WEB;
    wxString                SECTION_TIPS;

    wxString                BITMAP_PROJECTS;
    wxString                BITMAP_TASKHEADER;
    wxString                BITMAP_WEBHEADER;
    wxString                BITMAP_TIPSHEADER;
    wxString                BITMAP_BOINC;

    wxString                LINKDESC_DEFAULT;

    wxString                LINK_TASKATTACH;
    wxString                LINKDESC_TASKATTACH;

    wxString                LINK_TASKDETACH;
    wxString                LINKDESC_TASKDETACH;

    wxString                LINK_TASKRESET;
    wxString                LINKDESC_TASKRESET;

    wxString                LINK_TASKSUSPEND;
    wxString                LINKDESC_TASKSUSPEND;

    wxString                LINK_TASKRESUME;
    wxString                LINKDESC_TASKRESUME;

    wxString                LINK_TASKUPDATE;
    wxString                LINKDESC_TASKUPDATE;

    wxString                LINK_WEBBOINC;
    wxString                LINKDESC_WEBBOINC;

    wxString                LINK_WEBPROJECT;
    wxString                LINKDESC_WEBPROJECT;

    wxString                LINK_WEB;

};


#endif


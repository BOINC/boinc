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

	wxInt32  GetProjectName( wxString& strProjectName );
	wxInt32  GetApplicationName( wxString& strApplicationName );
	wxInt32  GetName( wxString& strName );
	wxInt32  GetCPUTime( wxString& strCPUTime );
	wxInt32  GetProgress( wxString& strProgress );
	wxInt32  GetTimeToCompletion( wxString& strTimeToCompletion );
	wxInt32  GetReportDeadline( wxString& strReportDeadline );
	wxInt32  GetStatus( wxString& strStatus );

	wxInt32  SetProjectName( wxString& strProjectName );
	wxInt32  SetApplicationName( wxString& strApplicationName );
	wxInt32  SetName( wxString& strName );
	wxInt32  SetCPUTime( wxString& strCPUTime );
	wxInt32  SetProgress( wxString& strProgress );
	wxInt32  SetTimeToCompletion( wxString& strTimeToCompletion );
	wxInt32  SetReportDeadline( wxString& strReportDeadline );
	wxInt32  SetStatus( wxString& strStatus );

protected:
	wxString m_strProjectName;
    wxString m_strApplicationName;
    wxString m_strName;
    wxString m_strCPUTime;
    wxString m_strProgress;
    wxString m_strTimeToCompletion;
    wxString m_strReportDeadline;
    wxString m_strStatus;
};

WX_DECLARE_OBJARRAY( CWork, CWorkCache );


class CViewWork : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewWork )

public:
    CViewWork();
    CViewWork(wxNotebook* pNotebook);

    ~CViewWork();

    virtual wxString        GetViewName();
    virtual char**          GetViewIcon();

protected:

    bool                    m_bTaskHeaderHidden;
    bool                    m_bTaskSuspendHidden;
    bool                    m_bTaskResumeHidden;
    bool                    m_bTaskShowGraphicsHidden;
    bool                    m_bTaskAbortHidden;

    bool                    m_bTipsHeaderHidden;

    bool                    m_bItemSelected;

	CWorkCache              m_WorkCache;

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
    wxInt32                 FormatApplicationName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatCPUTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatProgress( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTimeToCompletion( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatReportDeadline( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatStatus( wxInt32 item, wxString& strBuffer ) const;

};


#endif


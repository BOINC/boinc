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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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


    //
    // Globalization/Localization
    //
    wxString                VIEW_HEADER;

    wxString                SECTION_TASK;
    wxString                SECTION_TIPS;

    wxString                BITMAP_RESULTS;
    wxString                BITMAP_TASKHEADER;
    wxString                BITMAP_TIPSHEADER;

    wxString                LINKDESC_DEFAULT;

    wxString                LINK_TASKSUSPEND;
    wxString                LINKDESC_TASKSUSPEND;

    wxString                LINK_TASKRESUME;
    wxString                LINKDESC_TASKRESUME;

    wxString                LINK_TASKSHOWGRAPHICS;
    wxString                LINKDESC_TASKSHOWGRAPHICS;

    wxString                LINK_TASKABORT;
    wxString                LINKDESC_TASKABORT;

};


#endif


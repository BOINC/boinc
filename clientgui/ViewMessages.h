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

#ifndef _VIEWMESSAGES_H_
#define _VIEWMESSAGES_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewMessages.cpp"
#endif


#include "BOINCBaseView.h"


class CMessage : public wxObject
{
public:
	CMessage();
	~CMessage();

	wxInt32  GetProjectName( wxString& strProjectName );
	wxInt32  GetPriority( wxString& strPriority  );
	wxInt32  GetTime( wxString& strTime );
	wxInt32  GetMessage( wxString& strMessage );

	wxInt32  SetProjectName( wxString& strProjectName );
	wxInt32  SetPriority( wxString& strPriority );
	wxInt32  SetTime( wxString& strTime );
	wxInt32  SetMessage( wxString& strMessage );

protected:
	wxString m_strProjectName;
	wxString m_strPriority;
	wxString m_strTime;
    wxString m_strMessage;
};


class CViewMessages : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewMessages )

public:
    CViewMessages();
    CViewMessages(wxNotebook* pNotebook);

    ~CViewMessages();

    virtual wxString        GetViewName();
    virtual char**          GetViewIcon();

protected:

    bool                    m_bTaskHeaderHidden;
    bool                    m_bTaskCopyAllHidden;
    bool                    m_bTaskCopyMessageHidden;

    bool                    m_bTipsHeaderHidden;

    wxListItemAttr*         m_pMessageInfoAttr;
    wxListItemAttr*         m_pMessageErrorAttr;

    std::vector<CMessage*>  m_MessageCache;

    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    virtual wxString        OnDocGetItemText( long item, long column ) const;

    virtual void            OnTaskLinkClicked( const wxHtmlLinkInfo& link );
    virtual void            OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y );

    virtual wxInt32         AddCacheElement();
    virtual wxInt32         EmptyCache();
    virtual wxInt32         GetCacheCount();
    virtual wxInt32         RemoveCacheElement();
    virtual wxInt32         UpdateCache( long item, long column, wxString& strNewData );

    virtual bool            EnsureLastItemVisible();

    virtual void            UpdateSelection();
    virtual void            UpdateTaskPane();

    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatPriority( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatMessage( wxInt32 item, wxString& strBuffer ) const;

#ifndef NOCLIPBOARD
    bool                    m_bClipboardOpen;
    wxString                m_strClipboardData;
    bool                    OpenClipboard();
    wxInt32                 CopyToClipboard( wxInt32 item );
    bool                    CloseClipboard();
#endif


    //
    // Globalization/Localization
    //
    wxString                VIEW_HEADER;

    wxString                SECTION_TASK;
    wxString                SECTION_TIPS;

    wxString                BITMAP_MESSAGE;
    wxString                BITMAP_TASKHEADER;
    wxString                BITMAP_TIPSHEADER;

    wxString                LINKDESC_DEFAULT;

    wxString                LINK_TASKCOPYALL;
    wxString                LINKDESC_TASKCOPYALL;

    wxString                LINK_TASKCOPYMESSAGE;
    wxString                LINKDESC_TASKCOPYMESSAGE;

};


#endif


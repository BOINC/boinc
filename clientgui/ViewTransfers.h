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

#ifndef _VIEWTRANSFERS_H_
#define _VIEWTRANSFERS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewTransfers.cpp"
#endif


#include "BOINCBaseView.h"


class CTransfer : public wxObject
{
public:
	CTransfer();
	~CTransfer();

	wxInt32  GetProjectName( wxString& strProjectName );
	wxInt32  GetFileName( wxString& strFileName );
	wxInt32  GetProgress( wxString& strProgress );
	wxInt32  GetSize( wxString& strSize );
	wxInt32  GetTime( wxString& strTime );
	wxInt32  GetSpeed( wxString& strSpeed );
	wxInt32  GetStatus( wxString& strStatus );

	wxInt32  SetProjectName( wxString& strProjectName );
	wxInt32  SetFileName( wxString& strFileName );
	wxInt32  SetProgress( wxString& strProgress );
	wxInt32  SetSize( wxString& strSize );
	wxInt32  SetTime( wxString& strTime );
	wxInt32  SetSpeed( wxString& strSpeed );
	wxInt32  SetStatus( wxString& strStatus );

protected:
	wxString m_strProjectName;
    wxString m_strFileName;
    wxString m_strProgress;
    wxString m_strSize;
    wxString m_strTime;
    wxString m_strSpeed;
    wxString m_strStatus;
};

WX_DECLARE_OBJARRAY( CTransfer, CTransferCache );


class CViewTransfers : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewTransfers )

public:
    CViewTransfers();
    CViewTransfers(wxNotebook* pNotebook);

    ~CViewTransfers();

    virtual wxString        GetViewName();
    virtual char**          GetViewIcon();

protected:

    bool                    m_bTaskHeaderHidden;
    bool                    m_bTaskRetryHidden;
    bool                    m_bTaskAbortHidden;

    bool                    m_bTipsHeaderHidden;

    bool                    m_bItemSelected;

	CTransferCache          m_TransferCache;

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
    wxInt32                 FormatFileName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatProgress( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatSize( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatSpeed( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatStatus( wxInt32 item, wxString& strBuffer ) const;

};


#endif


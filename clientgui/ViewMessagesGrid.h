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

#ifndef _VIEWMESSAGESGRID_H_
#define _VIEWMESSAGESGRID_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ViewMessagesGrid.cpp"
#endif


#include "BOINCBaseView.h"
#include "BOINCGridCtrl.h"

class CViewMessagesGrid : public CBOINCBaseView
{
    DECLARE_DYNAMIC_CLASS( CViewMessagesGrid )

	CBOINCGridCtrl*			m_pGridPane;
public:
    CViewMessagesGrid();
    CViewMessagesGrid(wxNotebook* pNotebook);

    ~CViewMessagesGrid();

    virtual wxString&       GetViewName();
    virtual const char**    GetViewIcon();

    void                    OnMessagesCopyAll( wxCommandEvent& event );
    void                    OnMessagesCopySelected( wxCommandEvent& event );

protected:
    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );
    virtual void            OnListRender( wxTimerEvent& event );

    virtual wxInt32         GetDocCount();

    virtual void            UpdateSelection();

    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatMessage( wxInt32 item, wxString& strBuffer ) const;
	wxInt32                 FormatSeqNo( wxInt32 item, wxString& strBuffer ) const;
	wxInt32                 FormatPriority( wxInt32 item, wxString& strBuffer ) const;

#ifdef wxUSE_CLIPBOARD
    bool                    m_bClipboardOpen;
    wxString                m_strClipboardData;
    bool                    OpenClipboard();
    wxInt32                 CopyToClipboard( wxInt32 item );
    bool                    CloseClipboard();
#endif
	
	void					OnSelectCell( wxGridEvent& );

    DECLARE_EVENT_TABLE()
private: 
	bool					m_bIgnoreSelectionEvents;
};

#endif


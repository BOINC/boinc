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


#ifndef _DLG_MESSAGES_H_ 
#define _DLG_MESSAGES_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_DlgMessages.cpp"
#endif


/*!
 * Includes
 */

////@begin includes
#include "ViewNotices.h"    // For NoticeListCtrlEvent
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CNoticeListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DLGMESSAGES 10000
#define SYMBOL_CDLGMESSAGES_STYLE wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER
#define SYMBOL_CDLGMESSAGES_TITLE wxT("")
#define SYMBOL_CDLGMESSAGES_IDNAME ID_DLGMESSAGES
#define SYMBOL_CDLGMESSAGES_SIZE wxDefaultSize
#define SYMBOL_CDLGMESSAGES_POSITION wxDefaultPosition
#define ID_COPYSELECTED 10001
#define ID_COPYAll 10002
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif


/*!
 * CPanelMessages class declaration
 */

class CPanelMessages : public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CPanelMessages )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPanelMessages( );
    CPanelMessages( wxWindow* parent );

    /// Destructors
    ~CPanelMessages( );

    /// Creation
    bool Create();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CPanelMessages event handler declarations
    /// wxEVT_ERASE_BACKGROUND event handler for ID_DLGMESSAGES
    void OnEraseBackground( wxEraseEvent& event );

    void OnRefresh();

    void ReloadNotices();

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOK( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SIMPLE_HELP
    void OnButtonHelp( wxCommandEvent& event );

    /// wxEVT_NOTICELIST_ITEM_DISPLAY event handler for ID_LIST_NOTIFICATIONSVIEW
    void OnLinkClicked( NoticeListCtrlEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LIST_RELOADNOTICES
    void OnRetryButton( wxCommandEvent& event );

////@end CPanelMessages event handler declarations

////@begin CPanelMessages member function declarations
////@end CPanelMessages member function declarations

    bool                    OnSaveState(wxConfigBase* pConfig);
    bool                    OnRestoreState(wxConfigBase* pConfig);

private:
    bool                    m_bProcessingRefreshEvent;
	CNoticeListCtrl*        m_pHtmlListPane;

protected:
    wxStaticText*           m_ReloadNoticesText;
    wxButton*               m_ReloadNoticesButton;
    wxStaticText*           m_FetchingNoticesText;
    wxStaticText*           m_NoNoticesText;
    bool                    m_bMissingItems;
};


class CDlgMessages : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CDlgMessages )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgMessages( );
    CDlgMessages( wxWindow* parent, wxWindowID id = SYMBOL_CDLGMESSAGES_IDNAME, const wxString& caption = SYMBOL_CDLGMESSAGES_TITLE, const wxPoint& pos = SYMBOL_CDLGMESSAGES_POSITION, const wxSize& size = SYMBOL_CDLGMESSAGES_SIZE, long style = SYMBOL_CDLGMESSAGES_STYLE );

    ~CDlgMessages();
    
    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGMESSAGES_IDNAME, const wxString& caption = SYMBOL_CDLGMESSAGES_TITLE, const wxPoint& pos = SYMBOL_CDLGMESSAGES_POSITION, const wxSize& size = SYMBOL_CDLGMESSAGES_SIZE, long style = SYMBOL_CDLGMESSAGES_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_HELP event handler for ID_DLGMESSAGES
    void OnHelp( wxHelpEvent& event );

    /// wxEVT_SHOW event handler for ID_DLGMESSAGES
    void OnShow( wxShowEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOK( wxCommandEvent& event );
    
    void OnRefresh() { m_pBackgroundPanel->OnRefresh(); }
    
private:

    bool SaveState();
    void SaveWindowDimensions();
    bool RestoreState();
    void RestoreWindowDimensions();
    void OnSize(wxSizeEvent& event);
    void OnMove(wxMoveEvent& event);

////@begin CDlgMessages member variables
    CPanelMessages* m_pBackgroundPanel;
////@end CDlgMessages member variables
};


#endif  // end CDlgMessages

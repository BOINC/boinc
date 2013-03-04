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


#ifndef _DLGEVENTLOG_H_ 
#define _DLGEVENTLOG_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgEventLog.cpp"
#endif

#define EVENT_LOG_STRIPES 1
#define EVENT_LOG_RULES 1

#if EVENT_LOG_RULES
#define EVENT_LOG_DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES
#else
#define EVENT_LOG_DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL
#endif


/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CDlgEventLogListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DLGEVENTLOG 30000
#define SYMBOL_CDLGEVENTLOG_STYLE wxDEFAULT_DIALOG_STYLE|wxDIALOG_NO_PARENT|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxRESIZE_BORDER
#define SYMBOL_CDLGEVENTLOG_TITLE wxT("")
#define SYMBOL_CDLGEVENTLOG_IDNAME ID_DLGEVENTLOG
#define SYMBOL_CDLGEVENTLOG_SIZE wxDefaultSize
#define SYMBOL_CDLGEVENTLOG_POSITION wxDefaultPosition
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

class CDlgEventLog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CDlgEventLog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgEventLog( wxWindow* parent = NULL, wxWindowID id = SYMBOL_CDLGEVENTLOG_IDNAME, const wxString& caption = SYMBOL_CDLGEVENTLOG_TITLE, const wxPoint& pos = SYMBOL_CDLGEVENTLOG_POSITION, const wxSize& size = SYMBOL_CDLGEVENTLOG_SIZE, long style = SYMBOL_CDLGEVENTLOG_STYLE );
    ~CDlgEventLog();
    
    /// Creation
    bool Create( wxWindow* parent = NULL, wxWindowID id = SYMBOL_CDLGEVENTLOG_IDNAME, const wxString& caption = SYMBOL_CDLGEVENTLOG_TITLE, const wxPoint& pos = SYMBOL_CDLGEVENTLOG_POSITION, const wxSize& size = SYMBOL_CDLGEVENTLOG_SIZE, long style = SYMBOL_CDLGEVENTLOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Sets text for m_pFilterButton
    void SetFilterButtonText();
    
    /// Text color selection
    void SetTextColor();
    
////@begin CDlgEventLog event handler declarations
    /// wxEVT_HELP event handler for ID_DLGEVENTLOG
    void OnHelp( wxHelpEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOK( wxCommandEvent& event );
    
    /// wxEVT_CLOSE event handler for CDlgEventLog (window close control clicked)
    void OnClose(wxCloseEvent& event);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYAll
    void OnMessagesCopyAll( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYSELECTED
    void OnMessagesCopySelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_TASK_MESSAGES_FILTERBYPROJECT
    void OnMessagesFilter( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SIMPLE_HELP
    void OnButtonHelp( wxCommandEvent& event );

    /// called from CMainDocument::HandleCompletedRPC() after wxEVT_RPC_FINISHED event
    void OnRefresh();
////@end CDlgEventLog event handler declarations

////@begin CDlgEventLog member function declarations
////@end CDlgEventLog member function declarations

    virtual wxInt32         GetFilteredMessageIndex( wxInt32 iRow) const;
    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    bool                    OnSaveState(wxConfigBase* pConfig);
    bool                    OnRestoreState(wxConfigBase* pConfig);

    void                    UpdateButtons();

private:
////@begin CDlgEventLog member variables
////@end CDlgEventLog member variables
    wxTimer*                m_pRefreshTimer;

    wxInt32                 m_iPreviousDocCount;

    CDlgEventLogListCtrl*   m_pList;
    wxArrayInt              m_iFilteredIndexes;
    wxInt32                 m_iTotalDocCount;
    wxInt32                 m_iFilteredDocCount;
    wxInt32                 m_iPreviousTotalDocCount;
    wxInt32                 m_iPreviousRowCount;
    wxButton*               m_pFilterButton;
    wxButton*               m_pCopySelectedButton;

    wxListItemAttr*         m_pMessageInfoAttr;
    wxListItemAttr*         m_pMessageErrorAttr;
    wxListItemAttr*         m_pMessageInfoGrayAttr;
    wxListItemAttr*         m_pMessageErrorGrayAttr;

    bool                    m_bProcessingRefreshEvent;
    
    bool                    m_bEventLogIsOpen;

    bool                    SaveState();
    bool                    RestoreState();

    void                    GetWindowDimensions( wxPoint& position, wxSize& size );
    void                    SetWindowDimensions();
    void                    OnSize(wxSizeEvent& event);
    void                    OnMove(wxMoveEvent& event);

    void                    OnMouseUp(wxMouseEvent& event);

    void                    ResetMessageFiltering();    
    
    bool                    EnsureLastItemVisible();
    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatMessage( wxInt32 item, wxString& strBuffer ) const;

#ifdef wxUSE_CLIPBOARD
    bool                    m_bClipboardOpen;
    wxString                m_strClipboardData;
    bool                    OpenClipboard( wxInt32 size );
    wxInt32                 CopyToClipboard( wxInt32 item );
    bool                    CloseClipboard();
#endif
};


#endif  // end CDlgMessages

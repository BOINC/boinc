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
//
#ifndef _WIZ_PROJECTLISTCTRL_H_
#define _WIZ_PROJECTLISTCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ProjectListCtrl.cpp"
#endif


class CProjectListItemCtrl;
class CProjectListItemStaticCtrl;
class CProjectListItemBitmapCtrl;
class ProjectListCtrlEvent;
class ProjectListItemCtrlEvent;


////@begin control identifiers
#define ID_WEBSITEBUTTON 10001
////@end control identifiers


/*!
 * CProjectListCtrl class declaration
 */

class CProjectListCtrl: public wxScrolledWindow
{    
    DECLARE_DYNAMIC_CLASS( CProjectListCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectListCtrl( );

    CProjectListCtrl( wxWindow* parent );

    /// Creation
    bool Create( wxWindow* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectListCtrl event handler declarations

    /// event handler for window
    void OnItemChange( wxFocusEvent& event );

    /// event handler for window
    void OnItemDisplay( wxCommandEvent& event );

    /// event handler for window
    void OnItemFocusChange( wxFocusEvent& event );

    /// wxEVT_SET_FOCUS, wxEVT_KILL_FOCUS event handler for window
    void OnFocusChanged( wxFocusEvent& event );

    /// wxEVT_KEY_DOWN, wxEVT_KEY_UP event handler for window
    void OnKeyPressed( wxKeyEvent& event );

////@end CProjectListCtrl event handler declarations

    /// Methods
    bool Append(
        wxString strTitle,
        wxString strURL,
        bool bSupported
    );

private:
    wxWindow*   m_pCurrentSelection;
    wxBoxSizer* m_pMainSizer;
};


/*!
 * ProjectListCtrlEvent class declaration
 */

class ProjectListCtrlEvent : public wxNotifyEvent
{
public:
    ProjectListCtrlEvent( wxEventType evtType = wxEVT_NULL, wxString strName = wxEmptyString, wxString strURL = wxEmptyString, bool bSupported = false ) :
      wxNotifyEvent( evtType, wxID_ANY )
    {
        m_strName = strName;
        m_strURL = strURL;
        m_bSupported = bSupported;
    } 

    wxString GetName() { return m_strName; };
    wxString GetURL() { return m_strURL; };
    bool IsSupported() { return m_bSupported; };

    virtual wxNotifyEvent* Clone() const { return new ProjectListCtrlEvent(*this); }

private:
    wxString m_strName;
    wxString m_strURL;
    bool m_bSupported;

    DECLARE_DYNAMIC_CLASS(ProjectListCtrlEvent)
};

// ----------------------------------------------------------------------------
// macros for handling ProjectListCtrlEvent
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_CHANGE, 100000 )
    DECLARE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_DISPLAY, 100001 )
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*ProjectListCtrlEventFunction)(ProjectListCtrlEvent&);

#define ProjectListCtrlEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(ProjectListCtrlEventFunction, &func)

#define wx__DECLARE_PROJECTLISTEVT(evt, fn) \
    wx__DECLARE_EVT0(wxEVT_PROJECTLIST_ ## evt, ProjectListCtrlEventHandler(fn))

#define EVT_PROJECTLIST_ITEM_CHANGE(fn) wx__DECLARE_PROJECTLISTEVT(ITEM_CHANGE, fn)
#define EVT_PROJECTLIST_ITEM_DISPLAY(fn) wx__DECLARE_PROJECTLISTEVT(ITEM_DISPLAY, fn)


/*!
 * CProjectListItemCtrl class declaration
 */

class CProjectListItemCtrl: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CProjectListItemCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectListItemCtrl( );

    CProjectListItemCtrl( wxWindow* parent );

    /// Creation
    bool Create( wxWindow* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectListItemCtrl event handler declarations

    /// wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
    void OnMouseEnterLeave( wxMouseEvent& event );

    /// wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
    void OnMouseClick( wxMouseEvent& event );

    /// wxEVT_SET_FOCUS, wxEVT_KILL_FOCUS event handler for window
    void OnFocusChange( wxFocusEvent& event );

    /// wxEVT_KEY_DOWN, wxEVT_KEY_UP event handler for window
    void OnKeyPressed( wxKeyEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for window
    void OnWebsiteButtonClick( wxCommandEvent& event );

////@end CProjectListItemCtrl event handler declarations

    /// Methods
    wxString GetTitle() { return m_strTitle; };
    wxString GetURL() { return m_strURL; };
    bool     IsSupported() { return m_bSupported; };

    bool SetTitle( wxString strTitle );
    bool SetURL( wxString strURL );
    bool SetSupportedStatus( bool bSupported );

private:
    CProjectListItemStaticCtrl* m_pTitleStaticCtrl;
    CProjectListItemBitmapCtrl* m_pWebsiteButtonCtrl;
    wxString                    m_strTitle;
    wxString                    m_strURL;
    bool                        m_bSupported;

    bool                        m_bLeftButtonDownDetected;
};


/*!
 * CProjectListItemStaticCtrl class declaration
 */

class CProjectListItemStaticCtrl: public wxStaticText
{    
    DECLARE_DYNAMIC_CLASS( CProjectListItemStaticCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectListItemStaticCtrl();

    CProjectListItemStaticCtrl(
        wxWindow *parent,
        wxWindowID id,
        const wxString &label = wxEmptyString,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        long style = 0,
        const wxString &name = _T("ProjectListItemStaticCtrl")
    );

    /// Creation
    bool Create (
        wxWindow *parent,
        wxWindowID id,
        const wxString &label = wxEmptyString,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        long style = 0,
        const wxString &name = _T("ProjectListItemStaticCtrl")
    );

////@begin CProjectListItemStaticCtrl event handler declarations

    /// wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
    void OnMouseEnterLeave( wxMouseEvent& event );

    /// wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
    void OnMouseClick( wxMouseEvent& event );

////@end CProjectListItemStaticCtrl event handler declarations
};


/*!
 * CProjectListItemBitmapCtrl class declaration
 */

class CProjectListItemBitmapCtrl: public wxStaticBitmap
{    
    DECLARE_DYNAMIC_CLASS( CProjectListItemBitmapCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectListItemBitmapCtrl();

    CProjectListItemBitmapCtrl(
        wxWindow *parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        long style = 0,
        const wxString &name = _T("ProjectListItemBitmapCtrl")
    );

    /// Creation
    bool Create (
        wxWindow *parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        long style = 0,
        const wxString &name = _T("ProjectListItemBitmapCtrl")
    );

////@begin CProjectListItemBitmapCtrl event handler declarations

    /// wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
    void OnMouseEnterLeave( wxMouseEvent& event );

    /// wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
    void OnMouseClick( wxMouseEvent& event );

////@end CProjectListItemBitmapCtrl event handler declarations

    bool m_bLeftButtonDownDetected;
};


#endif // _WIZ_PROJECTLISTCTRL_H_

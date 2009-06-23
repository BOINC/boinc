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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ProjectListCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "ProjectListCtrl.h"

////@begin XPM images
#include "res/externalweblink.xpm"
////@end XPM images


/*!
 * CProjectListCtrl event definitions
 */
DEFINE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_CHANGE )
DEFINE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_DISPLAY )

/*!
 * CProjectListCtrl type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectListCtrl, wxScrolledWindow )
IMPLEMENT_DYNAMIC_CLASS( ProjectListCtrlEvent, wxNotifyEvent )

/*!
 * CProjectListCtrl event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectListCtrl, wxScrolledWindow )

////@begin CProjectListCtrl event table entries
    EVT_SET_FOCUS( CProjectListCtrl::OnFocusChanged )
    EVT_KILL_FOCUS( CProjectListCtrl::OnFocusChanged )
    EVT_KEY_DOWN( CProjectListCtrl::OnKeyPressed )
    EVT_KEY_UP( CProjectListCtrl::OnKeyPressed )
////@end CProjectListCtrl event table entries
 
END_EVENT_TABLE()
 
/*!
 * CProjectListCtrl constructors
 */
 
CProjectListCtrl::CProjectListCtrl( )
{
}
 
CProjectListCtrl::CProjectListCtrl( wxWindow* parent )
{
    Create( parent );
}
 
/*!
 * CProjectList creator
 */
 
bool CProjectListCtrl::Create( wxWindow* parent )
{
////@begin CProjectListCtrl member initialisation
    m_pMainSizer = NULL;
////@end CProjectListCtrl member initialisation
    m_pCurrentSelection = NULL;
 
////@begin CProjectListCtrl creation
    wxScrolledWindow::Create( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
        wxSUNKEN_BORDER | wxVSCROLL | wxTAB_TRAVERSAL );

    SetMinSize(GetSize());

    CreateControls();

    SetBackgroundColour( wxT("WHITE") );
    SetScrollRate( 0, 25 );

    GetSizer()->Fit(this);
////@end CProjectListCtrl creation
    return TRUE;
}
 
/*!
 * Control creation for ProjectListCtrl
 */
 
void CProjectListCtrl::CreateControls()
{    
////@begin CProjectListCtrl content construction

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    itemFlexGridSizer5->AddGrowableCol(0);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(1, 1, 0, 0);
    itemFlexGridSizer6->AddGrowableRow(0);
    itemFlexGridSizer6->AddGrowableCol(0);
    itemFlexGridSizer5->Add(itemFlexGridSizer6, 0, wxGROW|wxGROW|wxALL, 0);

    m_pMainSizer = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer6->Add(m_pMainSizer, 0, wxGROW|wxGROW|wxALL, 5);

    SetSizer(itemFlexGridSizer5);

////@end CProjectListCtrl content construction
}


/*!
 *  event handler for window
 */

void CProjectListCtrl::OnItemChange( wxMouseEvent& event ) {

    // Fire an event for the parent window notifing it of the new selection.
    CProjectListItemCtrl* pSelectedItem = wxDynamicCast(event.GetEventObject(), CProjectListItemCtrl);
    if (pSelectedItem) {

        // Store so we know where we are
        m_pCurrentSelection = pSelectedItem;

        // Fire Event
        ProjectListCtrlEvent evt(
            wxEVT_PROJECTLIST_ITEM_CHANGE,
            pSelectedItem->GetTitle(), 
            pSelectedItem->GetURL(),
            pSelectedItem->IsSupported()
        );
        evt.SetEventObject(this);

        GetParent()->AddPendingEvent( evt );
    }

}


/*!
 *  event handler for window
 */

void CProjectListCtrl::OnItemDisplay( wxCommandEvent& event ) {

    // Fire an event for the parent window notifing it to display more information.
    CProjectListItemCtrl* pItem = wxDynamicCast(event.GetEventObject(), CProjectListItemCtrl);
    if (pItem) {

        // Fire Event
        ProjectListCtrlEvent evt(
            wxEVT_PROJECTLIST_ITEM_DISPLAY,
            pItem->GetTitle(), 
            pItem->GetURL(),
            pItem->IsSupported()
        );
        evt.SetEventObject(this);

        GetParent()->AddPendingEvent( evt );
    }

}


/*!
 *  event handler for window
 */

void CProjectListCtrl::OnItemFocusChange( wxMouseEvent& event ) {

    // Reset the background color back to the default
    wxWindowList::compatibility_iterator current = GetChildren().GetFirst();
    while (current) {
        wxWindow* childWin = current->GetData();
        childWin->SetBackgroundColour( wxNullColour );
        childWin->Refresh();
        current = current->GetNext();
    }

    // Set the background color of the window that threw the event to the
    //   default background color of a selected control. 
    CProjectListItemCtrl* pSelectedItem = wxDynamicCast(event.GetEventObject(), CProjectListItemCtrl);
    if (pSelectedItem) {
        pSelectedItem->SetBackgroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) );
        pSelectedItem->Refresh();

        OnItemChange( event );
    } else {
        event.Skip();
    }

}


/*!
 * wxEVT_SET_FOCUS, wxEVT_KILL_FOCUS event handler for window
 */

void CProjectListCtrl::OnFocusChanged( wxFocusEvent& event ) {

    if ( wxEVT_SET_FOCUS == event.GetEventType() ) {
        // Control has focus
        if (!m_pCurrentSelection) {
            // Set the focus to the first child
            GetChildren().GetFirst()->GetData()->SetFocus();
        }
    }
    event.Skip();

}


/*!
 * wxEVT_KEY_DOWN, wxEVT_KEY_UP event handler for window
 */

void CProjectListCtrl::OnKeyPressed( wxKeyEvent& event ) {
    wxWindowList list;
    list = GetChildren();

    if        ( (wxEVT_KEY_DOWN == event.GetEventType()) && (WXK_DOWN == event.GetKeyCode()) ) {

        wxWindowList::compatibility_iterator iter = GetChildren().GetLast();
        while (iter) {
            wxWindow* pCurentWindow = iter->GetData();
            if (pCurentWindow == m_pCurrentSelection) {
                wxWindowListNode* pNextNode = iter->GetNext();
                if (pNextNode) {
                    wxWindow* pNextWindow = pNextNode->GetData();
                    if (pNextWindow) {
                        pNextWindow->SetFocus();
                    }
                }
            }
            iter = iter->GetPrevious();
        }

    } else if ( (wxEVT_KEY_DOWN == event.GetEventType()) && (WXK_UP == event.GetKeyCode()) ) {

        wxWindowList::compatibility_iterator iter = GetChildren().GetFirst();
        while (iter) {
            wxWindow* pCurentWindow = iter->GetData();
            if (pCurentWindow == m_pCurrentSelection) {
                wxWindowListNode* pPreviousNode = iter->GetPrevious();
                if (pPreviousNode) {
                    wxWindow* pPreviousWindow = pPreviousNode->GetData();
                    if (pPreviousWindow) {
                        pPreviousWindow->SetFocus();
                    }
                }
            }
            iter = iter->GetNext();
        }

    } else if ( (wxEVT_KEY_DOWN == event.GetEventType()) && (WXK_TAB == event.GetKeyCode()) ) {

        if (wxMOD_SHIFT == event.GetModifiers()) {
            Navigate( wxNavigationKeyEvent::IsBackward & wxNavigationKeyEvent::WinChange );
        } else {
            Navigate( wxNavigationKeyEvent::IsForward & wxNavigationKeyEvent::WinChange );
        }

    } else {
        event.Skip();
    }
}


/*!
 * Append a new entry to the project list.
 */
 
bool CProjectListCtrl::Append(
    wxString strTitle,
    wxString strURL,
    bool bSupported
)
{
    CProjectListItemCtrl* pItem = new CProjectListItemCtrl();
    pItem->Create( this );
    pItem->SetTitle( strTitle );
    pItem->SetURL( strURL );
    pItem->SetSupportedStatus( bSupported );

    m_pMainSizer->Add( pItem, 0, wxEXPAND );

    FitInside();

    return true;
}


/*!
 * CProjectListItemCtrl type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectListItemCtrl, wxPanel )

 
/*!
 * CProjectListItemCtrl event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectListItemCtrl, wxPanel )
 
////@begin CProjectListItemCtrl event table entries
    EVT_ENTER_WINDOW( CProjectListItemCtrl::OnMouseEnterLeave )
    EVT_LEAVE_WINDOW( CProjectListItemCtrl::OnMouseEnterLeave )
    EVT_LEFT_DOWN( CProjectListItemCtrl::OnMouseClick )
    EVT_LEFT_UP( CProjectListItemCtrl::OnMouseClick )
    EVT_KEY_DOWN( CProjectListItemCtrl::OnKeyPressed )
    EVT_KEY_UP( CProjectListItemCtrl::OnKeyPressed )
    EVT_BUTTON( ID_WEBSITEBUTTON, CProjectListItemCtrl::OnWebsiteButtonClick )
////@end CProjectListItemCtrl event table entries
 
END_EVENT_TABLE()
 
/*!
 * CProjectListItemCtrl constructors
 */
 
CProjectListItemCtrl::CProjectListItemCtrl( )
{
}
 
CProjectListItemCtrl::CProjectListItemCtrl( wxWindow* parent )
{
    Create( parent );
}
 
/*!
 * CProjectListItemCtrl creator
 */
 
bool CProjectListItemCtrl::Create( wxWindow* parent )
{
////@begin CProjectListItemCtrl member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pWebsiteButtonCtrl = NULL;
    m_strTitle = wxEmptyString;
    m_strURL = wxEmptyString;
    m_bSupported = false;
////@end CProjectListItemCtrl member initialisation
 
////@begin CProjectListItemCtrl creation
    wxPanel::Create( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
        wxNO_BORDER | wxWANTS_CHARS | wxTAB_TRAVERSAL );

    CreateControls();
    GetSizer()->Fit(this);
////@end CProjectListItemCtrl creation

    return TRUE;
}
 
/*!
 * Control creation for CProjectListItemCtrl
 */
 
void CProjectListItemCtrl::CreateControls()
{    
////@begin CProjectListItemCtrl content construction

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer8->AddGrowableRow(0);
    itemFlexGridSizer8->AddGrowableCol(0);
    itemBoxSizer7->Add(itemFlexGridSizer8, 0, wxGROW|wxALL, 0);

    m_pTitleStaticCtrl = new CProjectListItemStaticCtrl;
    m_pTitleStaticCtrl->Create( this, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pWebsiteButtonCtrl = new CProjectListItemBitmapCtrl;
    m_pWebsiteButtonCtrl->Create( this, ID_WEBSITEBUTTON, wxBitmap(externalweblink_xpm), wxDefaultPosition, wxSize(12,12), wxNO_BORDER );
    itemFlexGridSizer8->Add(m_pWebsiteButtonCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    SetSizer(itemBoxSizer7);

////@end CProjectListItemCtrl content construction
}

/*!
 * wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
 */

void CProjectListItemCtrl::OnMouseEnterLeave( wxMouseEvent& event ) {
    m_bLeftButtonDownDetected = false;
    event.Skip();
}


/*!
 * wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
 */

void CProjectListItemCtrl::OnMouseClick( wxMouseEvent& event ) {

    if ( event.LeftDown() ) {
        m_bLeftButtonDownDetected = true;
    } else {
        if ( m_bLeftButtonDownDetected ) {
            CProjectListCtrl* pParent = wxDynamicCast(GetParent(), CProjectListCtrl);
            if (pParent) {
                pParent->OnItemFocusChange( event );
            }
        }
    }
    event.Skip();

}


/*!
 * wxEVT_KEY_DOWN, wxEVT_KEY_UP event handler for window
 */

void CProjectListItemCtrl::OnKeyPressed( wxKeyEvent& event ) {
    CProjectListCtrl* pParent = wxDynamicCast(GetParent(), CProjectListCtrl);
    if (pParent) {
        pParent->OnKeyPressed( event );
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for window
 */

void CProjectListItemCtrl::OnWebsiteButtonClick( wxCommandEvent& event ) {

    if (!m_strURL.IsEmpty()) {
        // We now have focus, report that to the parent
        CProjectListCtrl* pParent = wxDynamicCast(GetParent(), CProjectListCtrl);
        if (pParent) {
            pParent->OnItemDisplay( event );
        }
    }

}


bool CProjectListItemCtrl::SetTitle( wxString strTitle ) {
    if (m_pTitleStaticCtrl) m_pTitleStaticCtrl->SetLabel( strTitle );
    m_strTitle = strTitle;
    return true;
}


bool CProjectListItemCtrl::SetURL( wxString strURL ) {
    if (m_pWebsiteButtonCtrl) {
        wxString strBuffer = wxEmptyString;

        strBuffer.Printf(
            _("Click here to go to %s's website."),
            m_strTitle.c_str()
        );

        m_pWebsiteButtonCtrl->SetToolTip(strBuffer);
    }
    m_strURL = strURL;
    return true;
}


bool CProjectListItemCtrl::SetSupportedStatus( bool bSupported ) {
    if (m_pTitleStaticCtrl) {
        m_pTitleStaticCtrl->Enable( bSupported );
    }
    m_bSupported = bSupported;
    return true;
}


/*!
 * CProjectListItemStaticCtrl type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectListItemStaticCtrl, wxStaticText )

/*!
 * CProjectListItemStaticCtrl event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectListItemStaticCtrl, wxStaticText )

////@begin CProjectListItemStaticCtrl event table entries
    EVT_ENTER_WINDOW( CProjectListItemStaticCtrl::OnMouseEnterLeave )
    EVT_LEAVE_WINDOW( CProjectListItemStaticCtrl::OnMouseEnterLeave )
    EVT_LEFT_DOWN( CProjectListItemStaticCtrl::OnMouseClick )
    EVT_LEFT_UP( CProjectListItemStaticCtrl::OnMouseClick )
////@end CProjectListItemStaticCtrl event table entries
 
END_EVENT_TABLE()
 
/*!
 * CProjectListItemStaticCtrl constructors
 */
 
CProjectListItemStaticCtrl::CProjectListItemStaticCtrl( )
{
}
 
CProjectListItemStaticCtrl::CProjectListItemStaticCtrl( 
    wxWindow *parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name
)
{
    Create (parent, id, label, pos, size, style, name);
}
 
/*!
 * CProjectListItemStaticCtrl creator
 */
 
bool CProjectListItemStaticCtrl::Create( 
    wxWindow *parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name
)
{
    bool retval = false;

    retval = wxStaticText::Create (parent, id, label, pos, size, style, name);

    return retval;
}


/*!
 * wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
 */

void CProjectListItemStaticCtrl::OnMouseEnterLeave( wxMouseEvent& event ) {
    CProjectListItemCtrl* pParent = wxDynamicCast(GetParent(), CProjectListItemCtrl);
    if (pParent) {
        pParent->OnMouseEnterLeave( event );
    }
}


/*!
 * wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
 */

void CProjectListItemStaticCtrl::OnMouseClick( wxMouseEvent& event ) {
    CProjectListItemCtrl* pParent = wxDynamicCast(GetParent(), CProjectListItemCtrl);
    if (pParent) {
        pParent->OnMouseClick( event );
    }
}


/*!
 * CProjectListItemBitmapCtrl type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectListItemBitmapCtrl, wxStaticBitmap )

/*!
 * CProjectListItemBitmapCtrl event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectListItemBitmapCtrl, wxStaticBitmap )

////@begin CProjectListItemBitmapCtrl event table entries
    EVT_ENTER_WINDOW( CProjectListItemBitmapCtrl::OnMouseEnterLeave )
    EVT_LEAVE_WINDOW( CProjectListItemBitmapCtrl::OnMouseEnterLeave )
    EVT_LEFT_DOWN( CProjectListItemBitmapCtrl::OnMouseClick )
    EVT_LEFT_UP( CProjectListItemBitmapCtrl::OnMouseClick )
////@end CProjectListItemBitmapCtrl event table entries
 
END_EVENT_TABLE()
 
/*!
 * CProjectListItemBitmapCtrl constructors
 */
 
CProjectListItemBitmapCtrl::CProjectListItemBitmapCtrl( )
{
}
 
CProjectListItemBitmapCtrl::CProjectListItemBitmapCtrl( 
    wxWindow *parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxString& name
)
{
    Create (parent, id, bitmap, pos, size, style, name);
}
 
/*!
 * CProjectListItemBitmapCtrl creator
 */
 
bool CProjectListItemBitmapCtrl::Create( 
    wxWindow *parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxString& name
)
{
    bool retval = false;

    retval = wxStaticBitmap::Create (parent, id, bitmap, pos, size, style, name);

    return retval;
}


/*!
 * wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
 */

void CProjectListItemBitmapCtrl::OnMouseEnterLeave( wxMouseEvent& event ) {
    m_bLeftButtonDownDetected = false;
    event.Skip();
}


/*!
 * wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
 */

void CProjectListItemBitmapCtrl::OnMouseClick( wxMouseEvent& event ) {

    if ( event.LeftDown() ) {
        m_bLeftButtonDownDetected = true;
    } else {
        if ( m_bLeftButtonDownDetected ) {
            // The control that reported the down event is also
            //   the one reporting the up event, so it is a valid
            //   click event.
            wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED);
            evt.SetId( ID_WEBSITEBUTTON );
            evt.SetEventObject(this);

            GetParent()->AddPendingEvent( evt );
        }
    }
    event.Skip();

}


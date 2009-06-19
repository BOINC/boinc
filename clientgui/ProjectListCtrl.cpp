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
#include "hyperlink.h"
#include "ProjectListCtrl.h"

////@begin XPM images
#include "res/externalweblink.xpm"
////@end XPM images


/*!
 * CProjectListCtrl event definitions
 */
DEFINE_EVENT_TYPE( wxEVT_PROJECTLISTCTRL_SELECTION_CHANGED )

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
    EVT_PROJECTLISTITEMCTRL_CLICKED( CProjectListCtrl::OnItemClicked )
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
 
////@begin CProjectListCtrl creation
    wxScrolledWindow::Create( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER );
    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
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
    itemFlexGridSizer6->Add(m_pMainSizer, 0, wxGROW|wxGROW|wxALL, 0);

    SetSizer(itemFlexGridSizer5);

////@end CProjectListCtrl content construction
}


/*!
 * wxEVT_PROJECTLISTITEMCTRL_CLICKED event handler for window
 */

void CProjectListCtrl::OnItemClicked( ProjectListItemCtrlEvent& event ) {
    // Reset the background color back to the default
    wxWindowList::compatibility_iterator current = GetChildren().GetFirst();
    while (current) {
        wxWindow* childWin = current->GetData();
        childWin->SetBackgroundColour( wxNullColour );
        childWin->Refresh();
        current = current->GetNext();
    }

    // Set the background color of the window that threw the event to the
    //   default background color of a selected control. Then fire an event
    //   for the parent window notifing it of the new selection.
    CProjectListItemCtrl* pSelectedItem = wxDynamicCast(event.GetEventObject(), CProjectListItemCtrl);
    if (pSelectedItem) {
        pSelectedItem->SetBackgroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT) );
        pSelectedItem->Refresh();

        // Fire Event
        ProjectListCtrlEvent evt(
            wxEVT_PROJECTLISTCTRL_SELECTION_CHANGED,
            pSelectedItem->GetTitle(), 
            pSelectedItem->GetURL(),
            pSelectedItem->IsSupported()
        );
        evt.SetEventObject(this);

        GetParent()->AddPendingEvent( evt );
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
 * CProjectListItemCtrl event definitions
 */
DEFINE_EVENT_TYPE( wxEVT_PROJECTLISTITEMCTRL_CLICKED )

/*!
 * CProjectListItemCtrl type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectListItemCtrl, wxPanel )
IMPLEMENT_DYNAMIC_CLASS( ProjectListItemCtrlEvent, wxNotifyEvent )

 
/*!
 * CProjectListItemCtrl event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectListItemCtrl, wxPanel )
 
////@begin CProjectListItemCtrl event table entries
    EVT_ENTER_WINDOW( CProjectListItemCtrl::OnMouseEnterLeave )
    EVT_LEAVE_WINDOW( CProjectListItemCtrl::OnMouseEnterLeave )
    EVT_LEFT_DOWN( CProjectListItemCtrl::OnMouseClick )
    EVT_LEFT_UP( CProjectListItemCtrl::OnMouseClick )
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
    m_bLeftButtonDownDetected = false;
////@end CProjectListItemCtrl member initialisation
 
////@begin CProjectListItemCtrl creation
    wxPanel::Create( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );

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
    itemBoxSizer7->Add(itemFlexGridSizer8, 0, wxGROW|wxALL, 1);

    m_pTitleStaticCtrl = new CProjectListItemStaticCtrl;
    m_pTitleStaticCtrl->Create( this, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pWebsiteButtonCtrl = new wxBitmapButton;
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
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectListItemCtrl::OnMouseClick - Function Begin"));

    if ( event.LeftDown() ) {
        m_bLeftButtonDownDetected = true;
    } else {
        if ( m_bLeftButtonDownDetected ) {
            // The control that reported the down event is also
            //   the one reporting the up event, so it is a valid
            //   click event.
            wxLogTrace(wxT("Function Status"), wxT("CProjectListItemCtrl::OnMouseClick - Click Detected!"));

            ProjectListItemCtrlEvent evt(wxEVT_PROJECTLISTITEMCTRL_CLICKED, GetId());
            evt.SetEventObject(this);

            GetParent()->AddPendingEvent( evt );
        }
    }
    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectListItemCtrl::OnMouseClick - Function End"));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for window
 */

void CProjectListItemCtrl::OnWebsiteButtonClick( wxCommandEvent& /*event*/ ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectListItemCtrl::OnWebsiteButtonClick - Function Begin"));

    if (!m_strURL.IsEmpty()) {
        wxHyperLink::ExecuteLink(m_strURL);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectListItemCtrl::OnWebsiteButtonClick - Function End"));
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
    bool okay = FALSE;

    // create static text
    okay = wxStaticText::Create (parent, id, label, pos, size, style, name);
    wxASSERT_MSG (okay, wxT("Failed to create wxStaticText, needed by wxHyperLink!"));

    return okay;
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


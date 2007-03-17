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
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ProjectListCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "ProjectListCtrl.h"

////@begin XPM images
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
    m_pMainSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_pMainSizer);
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
            pSelectedItem->GetURL()
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
    wxString strHome,
    wxString strURL,
    wxBitmap bmpImage,
    wxString strGeneralArea,
    wxString strSpecificArea,
    wxString strDescription
)
{
    CProjectListItemCtrl* pItem = new CProjectListItemCtrl();
    pItem->Create( this );
    pItem->SetTitle( strTitle );
    pItem->SetHome( strHome );
    pItem->SetURL( strURL );
    pItem->SetImage( bmpImage );
    pItem->SetGeneralArea( strGeneralArea );
    pItem->SetSpecificArea( strSpecificArea );
    pItem->SetDescription( strDescription );
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
    m_pHomeStaticCtrl = NULL;
    m_pImageCtrl = NULL;
    m_pGeneralAreaDescriptionStaticCtrl = NULL;
    m_pGeneralAreaStaticCtrl = NULL;
    m_pSpecificAreaDescriptionStaticCtrl = NULL;
    m_pSpecificAreaStaticCtrl = NULL;
    m_pDescriptionStaticCtrl = NULL;
    m_strURL = wxEmptyString;
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

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer3);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( this, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, _T("Tahoma")));
    itemBoxSizer3->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 1);

    m_pHomeStaticCtrl = new wxStaticText;
    m_pHomeStaticCtrl->Create( this, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_pHomeStaticCtrl, 0, wxALIGN_LEFT|wxALL, 1);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer6->AddGrowableCol(0);
    itemFlexGridSizer6->AddGrowableCol(1);
    itemBoxSizer3->Add(itemFlexGridSizer6, 0, wxALIGN_LEFT|wxALL, 2);

    wxBitmap m_pImageCtrlBitmap(wxNullBitmap);
    m_pImageCtrl = new wxStaticBitmap;
    m_pImageCtrl->Create( this, wxID_STATIC, m_pImageCtrlBitmap, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(m_pImageCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer6->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer9 = new wxFlexGridSizer(2, 2, 0, 0);
    itemFlexGridSizer9->AddGrowableCol(1);
    itemBoxSizer8->Add(itemFlexGridSizer9, 0, wxALIGN_LEFT|wxALL, 0);

    m_pGeneralAreaDescriptionStaticCtrl = new wxStaticText;
    m_pGeneralAreaDescriptionStaticCtrl->Create( this, wxID_STATIC, _("General Area:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer9->Add(m_pGeneralAreaDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

    m_pGeneralAreaStaticCtrl = new wxStaticText;
    m_pGeneralAreaStaticCtrl->Create( this, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer9->Add(m_pGeneralAreaStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_pSpecificAreaDescriptionStaticCtrl = new wxStaticText;
    m_pSpecificAreaDescriptionStaticCtrl->Create( this, wxID_STATIC, _("Specific Area:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer9->Add(m_pSpecificAreaDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 1);

    m_pSpecificAreaStaticCtrl = new wxStaticText;
    m_pSpecificAreaStaticCtrl->Create( this, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer9->Add(m_pSpecificAreaStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( this, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT||wxALIGN_CENTER_VERTICAL|wxALL, 1);

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



bool CProjectListItemCtrl::SetTitle( wxString strTitle ) {
    if (m_pTitleStaticCtrl) m_pTitleStaticCtrl->SetLabel( strTitle );
    return true;
}


bool CProjectListItemCtrl::SetHome( wxString strHome ) {
    if (m_pHomeStaticCtrl) m_pHomeStaticCtrl->SetLabel( strHome );
    return true;
}


bool CProjectListItemCtrl::SetImage( wxBitmap bmpImage ) {
    if (m_pImageCtrl) m_pImageCtrl->SetBitmap( bmpImage );
    return true;
}


bool CProjectListItemCtrl::SetGeneralArea( wxString strGeneralArea ) {
    if (m_pGeneralAreaStaticCtrl) m_pGeneralAreaStaticCtrl->SetLabel( strGeneralArea );
    return true;
}


bool CProjectListItemCtrl::SetSpecificArea( wxString strSpecificArea ) {
    if (m_pSpecificAreaStaticCtrl) m_pSpecificAreaStaticCtrl->SetLabel( strSpecificArea );
    return true;
}


bool CProjectListItemCtrl::SetDescription( wxString strDescription ) {
    if (m_pDescriptionStaticCtrl) {
        m_pDescriptionStaticCtrl->SetLabel( strDescription );
        m_pDescriptionStaticCtrl->Wrap(300);
    }
    return true;
}


bool CProjectListItemCtrl::SetURL( wxString strURL ) {
    m_strURL = strURL;
    return true;
}


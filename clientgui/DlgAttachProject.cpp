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
#pragma implementation "DlgAttachProject.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"

////@begin includes
////@end includes

#include "DlgAttachProject.h"
#include "ValidateURL.h"
#include "ValidateAccountKey.h"

////@begin XPM images
////@end XPM images

/*!
 * CDlgAttachProject type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgAttachProject, wxDialog )

/*!
 * CDlgAttachProject event table definition
 */

BEGIN_EVENT_TABLE( CDlgAttachProject, wxDialog )

////@begin CDlgAttachProject event table entries
////@end CDlgAttachProject event table entries

END_EVENT_TABLE()

/*!
 * CDlgAttachProject constructors
 */

CDlgAttachProject::CDlgAttachProject( )
{
}

CDlgAttachProject::CDlgAttachProject( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgAttachProject creator
 */

bool CDlgAttachProject::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDlgAttachProject member initialisation
    m_ProjectAddressCtrl = NULL;
    m_ProjectAccountKeyCtrl = NULL;
////@end CDlgAttachProject member initialisation

////@begin CDlgAttachProject creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgAttachProject creation
    return TRUE;
}

/*!
 * Control creation for CDlgAttachProject
 */

void CDlgAttachProject::CreateControls()
{    
////@begin CDlgAttachProject content construction

    CDlgAttachProject* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText;
    itemStaticText6->Create( itemDialog1, wxID_STATIC, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_ProjectAddressCtrl = new wxTextCtrl;
    m_ProjectAddressCtrl->Create( itemDialog1, ID_PROJECTADDRESS, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer5->Add(m_ProjectAddressCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText;
    itemStaticText8->Create( itemDialog1, wxID_STATIC, _("Account Key:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_ProjectAccountKeyCtrl = new wxTextCtrl;
    m_ProjectAccountKeyCtrl->Create( itemDialog1, ID_PROJECTACCOUNTKEY, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer5->Add(m_ProjectAccountKeyCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText;
    itemStaticText10->Create( itemDialog1, wxID_STATIC, _("These are emailed to you when you create an account.\nGo to project web sites to create accounts.\nVisit http://boinc.berkeley.edu for a list of projects."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton12 = new wxButton;
    itemButton12->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton12->SetDefault();
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton;
    itemButton13->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Set validators
    m_ProjectAddressCtrl->SetValidator( CValidateURL(& m_strProjectAddress) );
    m_ProjectAccountKeyCtrl->SetValidator( CValidateAccountKey(& m_strProjectAccountKey) );
////@end CDlgAttachProject content construction
}

/*!
 * Should we show tooltips?
 */

bool CDlgAttachProject::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDlgAttachProject::GetBitmapResource( const wxString& )
{
    // Bitmap retrieval
////@begin CDlgAttachProject bitmap retrieval
    return wxNullBitmap;
////@end CDlgAttachProject bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgAttachProject::GetIconResource( const wxString& )
{
    // Icon retrieval
////@begin CDlgAttachProject icon retrieval
    return wxNullIcon;
////@end CDlgAttachProject icon retrieval
}

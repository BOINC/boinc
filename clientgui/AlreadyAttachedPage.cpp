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
#pragma implementation "AlreadyAttachedPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "AlreadyAttachedPage.h"


/*!
 * CErrAlreadyAttachedPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrAlreadyAttachedPage, wxWizardPageEx )

/*!
 * CErrAlreadyAttachedPage event table definition
 */

BEGIN_EVENT_TABLE( CErrAlreadyAttachedPage, wxWizardPageEx )

////@begin CErrAlreadyAttachedPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrAlreadyAttachedPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrAlreadyAttachedPage::OnCancel )

////@end CErrAlreadyAttachedPage event table entries

END_EVENT_TABLE()

/*!
 * CErrAlreadyAttachedPage constructors
 */

CErrAlreadyAttachedPage::CErrAlreadyAttachedPage( )
{
}

CErrAlreadyAttachedPage::CErrAlreadyAttachedPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}

/*!
 * WizardPage creator
 */

bool CErrAlreadyAttachedPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrAlreadyAttachedPage member initialisation
////@end CErrAlreadyAttachedPage member initialisation

////@begin CErrAlreadyAttachedPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrAlreadyAttachedPage creation
    return TRUE;
}

/*!
 * Control creation for WizardPage
 */

void CErrAlreadyAttachedPage::CreateControls()
{    
////@begin CErrAlreadyAttachedPage content construction
    CErrAlreadyAttachedPage* itemWizardPage101 = this;

    wxBoxSizer* itemBoxSizer102 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage101->SetSizer(itemBoxSizer102);

    wxStaticText* itemStaticText103 = new wxStaticText;
    itemStaticText103->Create( itemWizardPage101, wxID_STATIC, _("Already attached to project"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText103->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer102->Add(itemStaticText103, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer102->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText105 = new wxStaticText;
    itemStaticText105->Create( itemWizardPage101, wxID_STATIC, _("You are already attached to this project."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer102->Add(itemStaticText105, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrAlreadyAttachedPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPageEx* CErrAlreadyAttachedPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPageEx* CErrAlreadyAttachedPage::GetNext() const
{
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CErrAlreadyAttachedPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrAlreadyAttachedPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrAlreadyAttachedPage bitmap retrieval
    return wxNullBitmap;
////@end CErrAlreadyAttachedPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrAlreadyAttachedPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrAlreadyAttachedPage icon retrieval
    return wxNullIcon;
////@end CErrAlreadyAttachedPage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTALREADYATTACHED
 */

void CErrAlreadyAttachedPage::OnPageChanged( wxWizardExEvent& event ) {
    event.Skip();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTALREADYATTACHED
 */

void CErrAlreadyAttachedPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


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
#pragma implementation "AlreadyExistsPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "AlreadyExistsPage.h"


/*!
 * CErrAlreadyExistsPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrAlreadyExistsPage, wxWizardPageEx )
 
/*!
 * CErrAlreadyExistsPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrAlreadyExistsPage, wxWizardPageEx )
 
////@begin CErrAlreadyExistsPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrAlreadyExistsPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrAlreadyExistsPage::OnCancel )

////@end CErrAlreadyExistsPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrAlreadyExistsPage constructors
 */
 
CErrAlreadyExistsPage::CErrAlreadyExistsPage( )
{
}
 
CErrAlreadyExistsPage::CErrAlreadyExistsPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrAccountAlreadyExists creator
 */
 
bool CErrAlreadyExistsPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrAlreadyExistsPage member initialisation
////@end CErrAlreadyExistsPage member initialisation
 
////@begin CErrAlreadyExistsPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrAlreadyExistsPage creation

    return TRUE;
}
 
/*!
 * Control creation for CErrAccountAlreadyExists
 */
 
void CErrAlreadyExistsPage::CreateControls()
{    
////@begin CErrAlreadyExistsPage content construction
    CErrAlreadyExistsPage* itemWizardPage116 = this;

    wxBoxSizer* itemBoxSizer117 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage116->SetSizer(itemBoxSizer117);

    wxStaticText* itemStaticText118 = new wxStaticText;
    itemStaticText118->Create( itemWizardPage116, wxID_STATIC, _("Email address already in use"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText118->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer117->Add(itemStaticText118, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer117->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText120 = new wxStaticText;
    itemStaticText120->Create( itemWizardPage116, wxID_STATIC, _("An account with that email address already exists and has a\ndifferent password than the one you entered.\n\nPlease visit the project's web site and follow the instructions there."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer117->Add(itemStaticText120, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrAlreadyExistsPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CErrAlreadyExistsPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CErrAlreadyExistsPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrAlreadyExistsPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrAlreadyExistsPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrAlreadyExistsPage bitmap retrieval
    return wxNullBitmap;
////@end CErrAlreadyExistsPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CErrAlreadyExistsPage::GetIconResource( const wxString& name )
{
    // Icon retrieval

////@begin CErrAlreadyExistsPage icon retrieval
    return wxNullIcon;
////@end CErrAlreadyExistsPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRACCOUNTALREADYEXISTSPAGE
 */
 
void CErrAlreadyExistsPage::OnPageChanged( wxWizardExEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRACCOUNTALREADYEXISTSPAGE
 */
 
void CErrAlreadyExistsPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


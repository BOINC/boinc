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
#pragma implementation "NoInternetConnectionPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "NoInternetConnectionPage.h"


/*!
 * CErrNoInternetConnectionPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrNoInternetConnectionPage, wxWizardPageEx )

/*!
 * CErrNoInternetConnectionPage event table definition
 */

BEGIN_EVENT_TABLE( CErrNoInternetConnectionPage, wxWizardPageEx )

////@begin CErrNoInternetConnectionPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrNoInternetConnectionPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrNoInternetConnectionPage::OnCancel )

////@end CErrNoInternetConnectionPage event table entries

END_EVENT_TABLE()

/*!
 * CErrNoInternetConnectionPage constructors
 */

CErrNoInternetConnectionPage::CErrNoInternetConnectionPage( )
{
}

CErrNoInternetConnectionPage::CErrNoInternetConnectionPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}

/*!
 * CErrNoInternetConnectionPage creator
 */

bool CErrNoInternetConnectionPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrNoInternetConnectionPage member initialisation
////@end CErrNoInternetConnectionPage member initialisation

////@begin CErrNoInternetConnectionPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrNoInternetConnectionPage creation
    return TRUE;
}

/*!
 * Control creation for CErrNoInternetConnectionPage
 */

void CErrNoInternetConnectionPage::CreateControls()
{    
////@begin CErrNoInternetConnectionPage content construction
    CErrNoInternetConnectionPage* itemWizardPage106 = this;

    wxBoxSizer* itemBoxSizer107 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage106->SetSizer(itemBoxSizer107);

    wxStaticText* itemStaticText108 = new wxStaticText;
    itemStaticText108->Create( itemWizardPage106, wxID_STATIC, _("No Internet connection"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText108->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer107->Add(itemStaticText108, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer107->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText110 = new wxStaticText;
    itemStaticText110->Create( itemWizardPage106, wxID_STATIC, _("Please connect to the Internet and try again."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer107->Add(itemStaticText110, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrNoInternetConnectionPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPageEx* CErrNoInternetConnectionPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPageEx* CErrNoInternetConnectionPage::GetNext() const
{
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CErrNoInternetConnectionPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrNoInternetConnectionPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrNoInternetConnectionPage bitmap retrieval
    return wxNullBitmap;
////@end CErrNoInternetConnectionPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrNoInternetConnectionPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrNoInternetConnectionPage icon retrieval
    return wxNullIcon;
////@end CErrNoInternetConnectionPage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNOINTERNETCONNECTIONPAGE
 */

void CErrNoInternetConnectionPage::OnPageChanged( wxWizardExEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRNOINTERNETCONNECTIONPAGE
 */
 
void CErrNoInternetConnectionPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


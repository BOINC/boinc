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
#pragma implementation "NotFoundPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "NotFoundPage.h"


/*!
 * CErrNotFoundPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrNotFoundPage, wxWizardPageEx )
 
/*!
 * CErrNotFoundPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrNotFoundPage, wxWizardPageEx )

////@begin CErrNotFoundPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrNotFoundPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrNotFoundPage::OnCancel )

////@end CErrNotFoundPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrNotFoundPage constructors
 */
 
CErrNotFoundPage::CErrNotFoundPage( )
{
}
 
CErrNotFoundPage::CErrNotFoundPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrNoInternetConnection creator
 */
 
bool CErrNotFoundPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrNotFoundPage member initialisation
////@end CErrNotFoundPage member initialisation
 
////@begin CErrNotFoundPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrNotFoundPage creation

    return TRUE;
}
 
/*!
 * Control creation for CErrNoInternetConnection
 */
 
void CErrNotFoundPage::CreateControls()
{    
////@begin CErrNotFoundPage content construction
    CErrNotFoundPage* itemWizardPage111 = this;

    wxBoxSizer* itemBoxSizer112 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage111->SetSizer(itemBoxSizer112);

    wxStaticText* itemStaticText113 = new wxStaticText;
    itemStaticText113->Create( itemWizardPage111, wxID_STATIC, _("Account not found"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText113->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer112->Add(itemStaticText113, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer112->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_NotFoundDescriptionStaticCtrl = new wxStaticText;
    m_NotFoundDescriptionStaticCtrl->Create( itemWizardPage111, wxID_STATIC, _("Check the email address and password, and try again."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer112->Add(m_NotFoundDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrNotFoundPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CErrNotFoundPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */

wxWizardPageEx* CErrNotFoundPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrNotFoundPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrNotFoundPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrNotFoundPage bitmap retrieval
    return wxNullBitmap;
////@end CErrNotFoundPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */

wxIcon CErrNotFoundPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrNotFoundPage icon retrieval
    return wxNullIcon;
////@end CErrNotFoundPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNACCOUNTDOESNOTEXISTPAGE
 */

void CErrNotFoundPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    if (((CBOINCBaseWizard*)GetParent())->project_config.uses_username) {
        m_NotFoundDescriptionStaticCtrl->SetLabel(
            _("Check the username and password, and try again.")
        );
    } else {
        m_NotFoundDescriptionStaticCtrl->SetLabel(
            _("Check the email address and password, and try again.")
        );
    }
 
    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRNACCOUNTDOESNOTEXISTPAGE
 */

void CErrNotFoundPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


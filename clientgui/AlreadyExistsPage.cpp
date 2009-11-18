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
#pragma implementation "AlreadyExistsPage.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
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
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
////@end CErrAlreadyExistsPage member initialisation
 
////@begin CErrAlreadyExistsPage creation
    wxWizardPageEx::Create( parent, ID_ERRALREADYEXISTSPAGE );

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
    CErrAlreadyExistsPage* itemWizardPage96 = this;

    wxBoxSizer* itemBoxSizer97 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage96->SetSizer(itemBoxSizer97);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer97->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer97->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create( itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer97->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);
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
 
wxBitmap CErrAlreadyExistsPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CErrAlreadyExistsPage bitmap retrieval
    return wxNullBitmap;
////@end CErrAlreadyExistsPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CErrAlreadyExistsPage::GetIconResource( const wxString& WXUNUSED(name) )
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
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    if (((CBOINCBaseWizard*)GetParent())->project_config.uses_username) {
        m_pTitleStaticCtrl->SetLabel(
            _("Username already in use")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("An account with that username already exists and has a\ndifferent password than the one you entered.\n\nPlease visit the project's web site and follow the instructions there.")
        );
    } else {
        m_pTitleStaticCtrl->SetLabel(
            _("Email address already in use")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("An account with that email address already exists and has a\ndifferent password than the one you entered.\n\nPlease visit the project's web site and follow the instructions there.")
        );
    }

    Fit();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRACCOUNTALREADYEXISTSPAGE
 */
 
void CErrAlreadyExistsPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


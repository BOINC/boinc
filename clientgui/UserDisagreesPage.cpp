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
#pragma implementation "UserDisagreesPage.h"
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
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "UserDisagreesPage.h"


/*!
 * CErrUserDisagreesPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrUserDisagreesPage, wxWizardPageEx )
 
/*!
 * CErrUserDisagreesPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrUserDisagreesPage, wxWizardPageEx )
 
////@begin CErrUserDisagreesPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrUserDisagreesPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrUserDisagreesPage::OnCancel )

////@end CErrUserDisagreesPage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CErrUserDisagreesPage constructors
 */
 
CErrUserDisagreesPage::CErrUserDisagreesPage( )
{
}
  
CErrUserDisagreesPage::CErrUserDisagreesPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrUserDisagreesPage creator
 */
 
bool CErrUserDisagreesPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrUserDisagreesPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
////@end CErrUserDisagreesPage member initialisation
 
////@begin CErrUserDisagreesPage creation
    wxWizardPageEx::Create( parent, ID_ERRUSERDISAGREESPAGE );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrUserDisagreesPage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CErrUserDisagreesPage
 */
 
void CErrUserDisagreesPage::CreateControls()
{    
////@begin CErrUserDisagreesPage content construction
    CErrUserDisagreesPage* itemWizardPage96 = this;

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
////@end CErrUserDisagreesPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CErrUserDisagreesPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CErrUserDisagreesPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */

bool CErrUserDisagreesPage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrUserDisagreesPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
 
////@begin CErrUserDisagreesPage bitmap retrieval
    return wxNullBitmap;
////@end CErrUserDisagreesPage bitmap retrieval
}
  
/*!
 * Get icon resources
 */

wxIcon CErrUserDisagreesPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval

////@begin CErrUserDisagreesPage icon retrieval
    return wxNullIcon;
////@end CErrUserDisagreesPage icon retrieval
}
   
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRUSERDISAGREESPAGE
 */

void CErrUserDisagreesPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    if (IS_ATTACHTOPROJECTWIZARD()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Failed to add project")
        );
    } else if (IS_ACCOUNTMANAGERWIZARD()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Failed to add account manager")
        );
    } else {
        wxASSERT(FALSE);
    }
    
    m_pDirectionsStaticCtrl->SetLabel(
        _("You did not agree to the terms of use.\nClick Finish or Cancel to close.")
    );

    Fit();
}
 
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRUSERDISAGREESPAGE
 */

void CErrUserDisagreesPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


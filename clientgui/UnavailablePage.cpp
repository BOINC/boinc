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
#pragma implementation "UnavailablePage.h"
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
#include "UnavailablePage.h"


/*!
 * CErrUnavailablePage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrUnavailablePage, wxWizardPageEx )
 
/*!
 * CErrUnavailablePage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrUnavailablePage, wxWizardPageEx )
 
////@begin CErrUnavailablePage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrUnavailablePage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrUnavailablePage::OnCancel )

////@end CErrUnavailablePage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CErrUnavailablePage constructors
 */
 
CErrUnavailablePage::CErrUnavailablePage( )
{
}
  
CErrUnavailablePage::CErrUnavailablePage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrUnavailablePage creator
 */
 
bool CErrUnavailablePage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrUnavailablePage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
////@end CErrUnavailablePage member initialisation
 
////@begin CErrUnavailablePage creation
    wxWizardPageEx::Create( parent, ID_ERRUNAVAILABLEPAGE );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrUnavailablePage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CErrUnavailablePage
 */
 
void CErrUnavailablePage::CreateControls()
{    
////@begin CErrUnavailablePage content construction
    CErrUnavailablePage* itemWizardPage96 = this;

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
////@end CErrUnavailablePage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CErrUnavailablePage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CErrUnavailablePage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */

bool CErrUnavailablePage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrUnavailablePage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
 
////@begin CErrUnavailablePage bitmap retrieval
    return wxNullBitmap;
////@end CErrUnavailablePage bitmap retrieval
}
  
/*!
 * Get icon resources
 */

wxIcon CErrUnavailablePage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval

////@begin CErrUnavailablePage icon retrieval
    return wxNullIcon;
////@end CErrUnavailablePage icon retrieval
}
   
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTUNAVAILABLEPAGE
 */

void CErrUnavailablePage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    if (IS_ATTACHTOPROJECTWIZARD()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Project temporarily unavailable")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("The project is temporarily unavailable.\n\nPlease try again later.")
        );
    } else if (IS_ACCOUNTMANAGERWIZARD()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Account manager temporarily unavailable")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("The account manager is temporarily unavailable.\n\nPlease try again later.")
        );
    } else {
        wxASSERT(FALSE);
    }

    Fit();
}
 
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTUNAVAILABLEPAGE
 */

void CErrUnavailablePage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


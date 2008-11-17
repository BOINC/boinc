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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "AccountManagerInfoPage.h"
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
#include "hyperlink.h"
#include "ValidateURL.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "AccountManagerInfoPage.h"


/*!
 * CAccountManagerInfoPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAccountManagerInfoPage, wxWizardPageEx )

/*!
 * CAccountManagerInfoPage event table definition
 */

BEGIN_EVENT_TABLE( CAccountManagerInfoPage, wxWizardPageEx )

////@begin CAccountManagerInfoPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CAccountManagerInfoPage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CAccountManagerInfoPage::OnPageChanging )
    EVT_WIZARDEX_CANCEL( -1, CAccountManagerInfoPage::OnCancel )

////@end CAccountManagerInfoPage event table entries

END_EVENT_TABLE()

/*!
 * CAccountManagerInfoPage constructors
 */

CAccountManagerInfoPage::CAccountManagerInfoPage( )
{
}

CAccountManagerInfoPage::CAccountManagerInfoPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}

/*!
 * CProjectInfoPage creator
 */

bool CAccountManagerInfoPage::Create( CBOINCBaseWizard* parent )
{
////@begin CAccountManagerInfoPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDescriptionStaticCtrl = NULL;
    m_pDescription2StaticCtrl = NULL;
    m_pProjectUrlStaticCtrl = NULL;
    m_pProjectUrlCtrl = NULL;
    m_pBOINCPromoStaticCtrl = NULL;
    m_pBOINCPromoUrlCtrl = NULL;
////@end CAccountManagerInfoPage member initialisation

////@begin CAccountManagerInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, ID_ACCOUNTMANAGERINFOPAGE, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountManagerInfoPage creation
    return TRUE;
}

/*!
 * Control creation for CProjectInfoPage
 */

void CAccountManagerInfoPage::CreateControls()
{    
////@begin CAccountManagerInfoPage content construction
    CAccountManagerInfoPage* itemWizardPage23 = this;

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage23->SetSizer(itemBoxSizer24);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescription2StaticCtrl = new wxStaticText;
    m_pDescription2StaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pDescription2StaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer30 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer30->AddGrowableCol(1);
    itemBoxSizer24->Add(itemFlexGridSizer30, 0, wxALIGN_LEFT|wxALL, 5);

    m_pProjectUrlStaticCtrl = new wxStaticText;
    m_pProjectUrlStaticCtrl->Create( itemWizardPage23, ID_PROJECTURLSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(m_pProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectUrlCtrl = new wxTextCtrl;
    m_pProjectUrlCtrl->Create( itemWizardPage23, ID_PROJECTURLCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer30->Add(m_pProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pBOINCPromoStaticCtrl = new wxStaticText;
    m_pBOINCPromoStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pBOINCPromoStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pBOINCPromoUrlCtrl = new wxHyperLink;
    m_pBOINCPromoUrlCtrl->Create( itemWizardPage23, ID_BOINCHYPERLINK, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer24->Add(m_pBOINCPromoUrlCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_pProjectUrlCtrl->SetValidator( CValidateURL( & m_strProjectURL) );
////@end CAccountManagerInfoPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
 */

void CAccountManagerInfoPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pDescription2StaticCtrl);
    wxASSERT(m_pProjectUrlStaticCtrl);
    wxASSERT(m_pProjectUrlCtrl);
    wxASSERT(m_pBOINCPromoStaticCtrl);
    wxASSERT(m_pBOINCPromoUrlCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Account Manager URL")
    );
    m_pDescriptionStaticCtrl->SetLabel(
        _("Enter the URL of the account manager's web site.")
    );
    m_pDescription2StaticCtrl->SetLabel(
        _("You can copy and paste the URL from your browser's\n"
		  "address bar.")
    );
    m_pProjectUrlStaticCtrl->SetLabel(
        _("Account Manager &URL:")
    );
    m_pBOINCPromoStaticCtrl->SetLabel(
        _("For a list of BOINC-based account managers go to:")
    );
    m_pBOINCPromoUrlCtrl->SetLabel(
        wxT("http://boinc.berkeley.edu/")
    );

    Fit();
    m_pProjectUrlCtrl->SetFocus();
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
 */

void CAccountManagerInfoPage::OnPageChanging( wxWizardExEvent& event ) {
    event.Skip();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
 */

void CAccountManagerInfoPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

/*!
 * Gets the previous page.
 */

wxWizardPageEx* CAccountManagerInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}

/*!
 * Gets the next page.
 */

wxWizardPageEx* CAccountManagerInfoPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROPERTIESPAGE);
    }
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAccountManagerInfoPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAccountManagerInfoPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CAccountManagerInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountManagerInfoPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAccountManagerInfoPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CAccountManagerInfoPage icon retrieval
    return wxNullIcon;
////@end CAccountManagerInfoPage icon retrieval
}


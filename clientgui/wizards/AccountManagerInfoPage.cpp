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
#pragma implementation "AccountManagerInfoPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
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
    m_AccountManagerUrlStaticCtrl = NULL;
    m_AccountManagerUrlCtrl = NULL;
////@end CAccountManagerInfoPage member initialisation

////@begin CAccountManagerInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

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
    CAccountManagerInfoPage* itemWizardPage21 = this;

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage21->SetSizer(itemBoxSizer22);

    wxStaticText* itemStaticText23 = new wxStaticText;
    itemStaticText23->Create( itemWizardPage21, wxID_STATIC, _("Account Manager URL"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText23->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer22->Add(itemStaticText23, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText24 = new wxStaticText;
    itemStaticText24->Create( itemWizardPage21, wxID_STATIC, _("Enter the URL of the account manager's web site."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText24, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer22->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText;
    itemStaticText26->Create( itemWizardPage21, wxID_STATIC, _("You can copy and paste the URL from your browser’s address bar."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText26, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer22->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer28 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer28->AddGrowableCol(1);
    itemBoxSizer22->Add(itemFlexGridSizer28, 0, wxALIGN_LEFT|wxALL, 5);

    m_AccountManagerUrlStaticCtrl = new wxStaticText;
    m_AccountManagerUrlStaticCtrl->Create( itemWizardPage21, ID_PROJECTURLSTATICCTRL, _("Account Manager &URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer28->Add(m_AccountManagerUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountManagerUrlCtrl = new wxTextCtrl;
    m_AccountManagerUrlCtrl->Create( itemWizardPage21, ID_PROJECTURLCTRL, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer28->Add(m_AccountManagerUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer22->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText32 = new wxStaticText;
    itemStaticText32->Create( itemWizardPage21, wxID_STATIC, _("For a list of BOINC-based account managers go to:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText32, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink33 = new wxHyperLink;
    itemHyperLink33->Create( itemWizardPage21, ID_BOINCHYPERLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer22->Add(itemHyperLink33, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_AccountManagerUrlCtrl->SetValidator( CValidateURL( & m_strProjectURL) );
////@end CAccountManagerInfoPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
 */

void CAccountManagerInfoPage::OnPageChanged( wxWizardExEvent& event ) {
    event.Skip();
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

wxBitmap CAccountManagerInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAccountManagerInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountManagerInfoPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAccountManagerInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountManagerInfoPage icon retrieval
    return wxNullIcon;
////@end CAccountManagerInfoPage icon retrieval
}


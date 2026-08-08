// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "CompletionErrorPage.h"
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
#include "CompletionErrorPage.h"


/*!
 * CCompletionErrorPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCompletionErrorPage, wxWizardPageEx )

/*!
 * CCompletionErrorPage event table definition
 */

BEGIN_EVENT_TABLE( CCompletionErrorPage, wxWizardPageEx )

////@begin CCompletionErrorPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CCompletionErrorPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CCompletionErrorPage::OnCancel )

////@end CCompletionErrorPage event table entries

END_EVENT_TABLE()

/*!
 * CCompletionErrorPage constructors
 */

CCompletionErrorPage::CCompletionErrorPage( )
{
}

CCompletionErrorPage::CCompletionErrorPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}

/*!
 * CAccountResultPage creator
 */

bool CCompletionErrorPage::Create( CBOINCBaseWizard* parent )
{
////@begin CCompletionErrorPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
    m_pServerMessagesDescriptionCtrl = NULL;
    m_pServerMessagesStaticBoxSizerCtrl = NULL;
    m_pServerMessagesCtrl = NULL;
////@end CCompletionErrorPage member initialisation

////@begin CCompletionErrorPage creation
    wxWizardPageEx::Create( parent, ID_COMPLETIONERRORPAGE );

    CreateControls();
    GetSizer()->Fit(this);
////@end CCompletionErrorPage creation

    return TRUE;
}

/*!
 * Control creation for CAccountResultPage
 */

void CCompletionErrorPage::CreateControls()
{
////@begin CCompletionErrorPage content construction
    CCompletionErrorPage* itemWizardPage85 = this;

    wxBoxSizer* itemBoxSizer86 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage85->SetSizer(itemBoxSizer86);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage85, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer86->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer86->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create( itemWizardPage85, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer86->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer86->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pServerMessagesDescriptionCtrl = new wxStaticBox(itemWizardPage85, wxID_ANY, wxEmptyString);
    m_pServerMessagesStaticBoxSizerCtrl = new wxStaticBoxSizer(m_pServerMessagesDescriptionCtrl, wxVERTICAL);
    itemBoxSizer86->Add(m_pServerMessagesStaticBoxSizerCtrl, 0, wxGROW|wxALL, 5);

    m_pServerMessagesCtrl = new wxStaticText;
    m_pServerMessagesCtrl->Create( itemWizardPage85, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pServerMessagesStaticBoxSizerCtrl->Add(m_pServerMessagesCtrl, 0, wxGROW|wxALL, 5);
////@end CCompletionErrorPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPageEx* CCompletionErrorPage::GetPrev() const
{
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPageEx* CCompletionErrorPage::GetNext() const
{
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CCompletionErrorPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCompletionErrorPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval

////@begin CCompletionErrorPage bitmap retrieval
    return wxNullBitmap;
////@end CCompletionErrorPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCompletionErrorPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval

////@begin CCompletionErrorPage icon retrieval
    return wxNullIcon;
////@end CCompletionErrorPage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE
 */

void CCompletionErrorPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);
    wxASSERT(m_pServerMessagesDescriptionCtrl);
    wxASSERT(m_pServerMessagesStaticBoxSizerCtrl);
    wxASSERT(m_pServerMessagesCtrl);

    if (IS_ATTACHTOPROJECTWIZARD()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Failed to add project")
        );
    } else if (IS_ACCOUNTMANAGERWIZARD()) {
        if (IS_ACCOUNTMANAGERUPDATEWIZARD()) {
            m_pTitleStaticCtrl->SetLabel(
                _("Failed to update account manager")
            );
        } else if (IS_ACCOUNTMANAGERUPDATEWIZARD()) {
            m_pTitleStaticCtrl->SetLabel(
                _("Failed to remove account manager")
            );
        } else {
            m_pTitleStaticCtrl->SetLabel(
                _("Failed to add account manager")
            );
        }
    } else {
        wxASSERT(FALSE);
    }

    if (m_pServerMessagesCtrl->GetLabel().IsEmpty()) {
        m_pDirectionsStaticCtrl->SetLabel(
            _("Please try again later.\n\nClick Finish to close.")
        );
    } else {
        m_pDirectionsStaticCtrl->SetLabel(
            _("Click Finish to close.")
        );
    }

    if (CHECK_CLOSINGINPROGRESS() || m_pServerMessagesCtrl->GetLabel().IsEmpty()) {
        m_pServerMessagesDescriptionCtrl->Hide();
        m_pServerMessagesCtrl->Hide();
    } else {
        m_pServerMessagesDescriptionCtrl->SetLabel(
            _("Messages from server:")
        );
        const wxSize page_width = this->GetClientSize();
        const int minimum_size = page_width.x - 15;  // 15 seems to be needed to keep the right border visible.
        m_pServerMessagesCtrl->Wrap(minimum_size);
        m_pServerMessagesDescriptionCtrl->Show();
        m_pServerMessagesCtrl->Show();
    }

    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE
 */

void CCompletionErrorPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


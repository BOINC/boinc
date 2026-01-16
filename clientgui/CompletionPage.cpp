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
#pragma implementation "CompletionPage.h"
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
#include "CompletionPage.h"
#include "AccountInfoPage.h"


/*!
 * CCompletionPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCompletionPage, wxWizardPageEx )

/*!
 * CCompletionPage event table definition
 */

BEGIN_EVENT_TABLE( CCompletionPage, wxWizardPageEx )

////@begin CCompletionPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CCompletionPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CCompletionPage::OnCancel )
    EVT_WIZARDEX_FINISHED( ID_COMPLETIONPAGE, CCompletionPage::OnFinished )
////@end CCompletionPage event table entries

END_EVENT_TABLE()

/*!
 * CCompletionPage constructors
 */

CCompletionPage::CCompletionPage( )
{
}

CCompletionPage::CCompletionPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}

/*!
 * CCompletionPage creator
 */

bool CCompletionPage::Create( CBOINCBaseWizard* parent )
{
////@begin CCompletionPage member initialisation
    m_pCompletionTitle = NULL;
    m_pCompletionWelcome = NULL;
    m_pCompletionBrandedMessage = NULL;
    m_pCompletionMessage = NULL;
////@end CCompletionPage member initialisation

////@begin CCompletionPage creation
    wxWizardPageEx::Create( parent, ID_COMPLETIONPAGE );

    CreateControls();
    GetSizer()->Fit(this);
////@end CCompletionPage creation

    return TRUE;
}

/*!
 * Control creation for CCompletionPage
 */

void CCompletionPage::CreateControls()
{
////@begin CCompletionPage content construction
    CCompletionPage* itemWizardPage79 = this;

    wxBoxSizer* itemBoxSizer80 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage79->SetSizer(itemBoxSizer80);

    m_pCompletionTitle = new wxStaticText;
    m_pCompletionTitle->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pCompletionTitle->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer80->Add(m_pCompletionTitle, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionWelcome = new wxStaticText;
    m_pCompletionWelcome->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pCompletionWelcome->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE));
    itemBoxSizer80->Add(m_pCompletionWelcome, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionBrandedMessage = new wxStaticText;
    m_pCompletionBrandedMessage->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer80->Add(m_pCompletionBrandedMessage, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionMessage = new wxStaticText;
    m_pCompletionMessage->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer80->Add(m_pCompletionMessage, 0, wxALIGN_LEFT|wxALL, 5);
    ////@end CCompletionPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPageEx* CCompletionPage::GetPrev() const
{
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPageEx* CCompletionPage::GetNext() const
{
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CCompletionPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCompletionPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CCompletionPage bitmap retrieval
    return wxNullBitmap;
////@end CCompletionPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCompletionPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CCompletionPage icon retrieval
    return wxNullIcon;
////@end CCompletionPage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE
 */

void CCompletionPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    CWizardAttach* pWAP = ((CWizardAttach*)GetParent());
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();


    wxASSERT(pSkinAdvanced);
    wxASSERT(m_pCompletionTitle);
    wxASSERT(m_pCompletionWelcome);
    wxASSERT(m_pCompletionBrandedMessage);
    wxASSERT(m_pCompletionMessage);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    if (IS_ATTACHTOPROJECTWIZARD()) {
        m_pCompletionTitle->SetLabel(
            _("Project added")
        );

        m_pCompletionWelcome->Hide();

        m_pCompletionBrandedMessage->SetLabel(
            _("This project has been successfully added.")
        );

        if (pWAP->m_AccountInfoPage->m_pAccountCreateCtrl->GetValue()) {
            m_pCompletionMessage->SetLabel(
                _("When you click Finish, your web browser will go to a page where\nyou can set your account name and preferences.")
            );
        } else {
            m_pCompletionMessage->SetLabel(
                _("Click Finish to close.")
            );
        }
    } else if (IS_ACCOUNTMANAGERWIZARD()) {

        if (IS_ACCOUNTMANAGERUPDATEWIZARD()) {
            // Update completed
            wxString strTitle;
            if (pSkinAdvanced->IsBranded()) {
                strTitle.Printf(
                    _("Update from %s completed."),
                    pWAP->GetProjectName().c_str()
                );
            } else {
                strTitle = _("Update completed.");
            }

            m_pCompletionTitle->SetLabel( strTitle );

            m_pCompletionMessage->SetLabel(
                _("Click Finish to close.")
            );

        } else {
            // Attach Completed
            m_pCompletionTitle->SetLabel(_("Now using account manager"));

            if (pSkinAdvanced->IsBranded()) {
                wxString strWelcome;
                strWelcome.Printf(
                    _("Welcome to %s!"),
                    pWAP->GetProjectName().c_str()
                );

                m_pCompletionWelcome->Show();
                m_pCompletionWelcome->SetLabel( strWelcome );
            }

            wxString strBrandedMessage;
            if (pSkinAdvanced->IsBranded()) {
                strBrandedMessage.Printf(
                    _("You are now using %s to manage accounts."),
                    pWAP->GetProjectName().c_str()
                );
            } else {
                strBrandedMessage = _("You are now using this account manager.");
            }

            m_pCompletionBrandedMessage->SetLabel( strBrandedMessage );

            m_pCompletionMessage->SetLabel(
                _("Click Finish to close.")
            );
        }
    }

    Fit();
    int x, y, x1, y1, w, h;
    GetPosition(&x, &y);
    m_pCompletionBrandedMessage->GetPosition(&x1, &y1);
    pWAP->GetSize(&w, &h);
    m_pCompletionBrandedMessage->Wrap(w - x - x1 - 5);
    Fit();

    // Is this supposed to be completely automated?
    // If so, then go ahead and close the wizard down now.
    if (pWAP->IsCloseWhenCompleted()) {
        pWAP->SimulateNextButton();
    }
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE
 */

void CCompletionPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE
 */

void CCompletionPage::OnFinished( wxWizardExEvent& event ) {
    event.Skip();
}


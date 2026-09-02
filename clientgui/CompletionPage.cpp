// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "CompletionPage.h"
#include "AccountInfoPage.h"

IMPLEMENT_DYNAMIC_CLASS(CCompletionPage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CCompletionPage, CBOINCWizardPage)

EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CCompletionPage::OnPageChanged)
EVT_WIZARD_CANCEL(wxID_ANY, CCompletionPage::OnCancel)
EVT_WIZARD_FINISHED(wxID_ANY, CCompletionPage::OnFinished)

END_EVENT_TABLE()

CCompletionPage::CCompletionPage() {
}

CCompletionPage::CCompletionPage(CWizardAttach* parent) {
    Create(parent);
}

bool CCompletionPage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pCompletionTitle = nullptr;
    m_pCompletionWelcome = nullptr;
    m_pCompletionBrandedMessage = nullptr;
    m_pCompletionMessage = nullptr;
    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CCompletionPage::CreateControls() {
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
}

wxWizardPage* CCompletionPage::GetPrev() const {
    return nullptr;
}

wxWizardPage* CCompletionPage::GetNext() const {
    return nullptr;
}

void CCompletionPage::SetPrev(CBOINCWizardPage*) {
    // no prev expected here
}

bool CCompletionPage::HasNextPage() const {
    return false;
}

bool CCompletionPage::HasPrevPage() const {
    return false;
}

void CCompletionPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();


    wxASSERT(pSkinAdvanced);
    wxASSERT(m_pCompletionTitle);
    wxASSERT(m_pCompletionWelcome);
    wxASSERT(m_pCompletionBrandedMessage);
    wxASSERT(m_pCompletionMessage);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    if (m_pParent->GetIsAttachToProjectWizard()) {
        m_pCompletionTitle->SetLabel(
            _("Project added")
        );

        m_pCompletionWelcome->Hide();

        m_pCompletionBrandedMessage->SetLabel(
            _("This project has been successfully added.")
        );

        if (m_pParent->GetAccountInfoPage()->GetAccountCreateCtrlValue()) {
            m_pCompletionMessage->SetLabel(
                _("When you click Finish, your web browser will go to a page where\nyou can set your account name and preferences.")
            );
        } else {
            m_pCompletionMessage->SetLabel(
                _("Click Finish to close.")
            );
        }
    } else if (m_pParent->GetIsAccountManagerWizard()) {

        if (m_pParent->GetIsAccountManagerUpdateWizard()) {
            // Update completed
            wxString strTitle;
            if (pSkinAdvanced->IsBranded()) {
                strTitle.Printf(
                    _("Update from %s completed."),
                    m_pParent->GetProjectName().c_str()
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
                    m_pParent->GetProjectName().c_str()
                );

                m_pCompletionWelcome->Show();
                m_pCompletionWelcome->SetLabel( strWelcome );
            }

            wxString strBrandedMessage;
            if (pSkinAdvanced->IsBranded()) {
                strBrandedMessage.Printf(
                    _("You are now using %s to manage accounts."),
                    m_pParent->GetProjectName().c_str()
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
    m_pParent->GetSize(&w, &h);
    m_pCompletionBrandedMessage->Wrap(w - x - x1 - 5);
    Fit();

    m_pParent->DisableBackButton();

    // Is this supposed to be completely automated?
    // If so, then go ahead and close the wizard down now.
    if (m_pParent->IsCloseWhenCompleted()) {
        m_pParent->SimulateNextButton();
    }
}

void CCompletionPage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}

void CCompletionPage::OnFinished(wxWizardEvent& event) {
    event.Skip();
}

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
#include "CompletionErrorPage.h"


IMPLEMENT_DYNAMIC_CLASS(CCompletionErrorPage, CBOINCWizardPage)

BEGIN_EVENT_TABLE(CCompletionErrorPage, CBOINCWizardPage)

EVT_WIZARD_PAGE_CHANGED(wxID_ANY, CCompletionErrorPage::OnPageChanged)
EVT_WIZARD_CANCEL(wxID_ANY, CCompletionErrorPage::OnCancel)

END_EVENT_TABLE()

CCompletionErrorPage::CCompletionErrorPage() {
}

CCompletionErrorPage::CCompletionErrorPage(CWizardAttach* parent) {
    Create(parent);
}

bool CCompletionErrorPage::Create(CWizardAttach* parent) {
    m_pParent = parent;
    m_pTitleStaticCtrl = nullptr;
    m_pDirectionsStaticCtrl = nullptr;
    m_pServerMessagesDescriptionCtrl = nullptr;
    m_pServerMessagesStaticBoxSizerCtrl = nullptr;
    m_pServerMessagesCtrl = nullptr;

    wxWizardPage::Create(parent);

    CreateControls();
    GetSizer()->Fit(this);

    return true;
}

void CCompletionErrorPage::CreateControls() {
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
}

wxWizardPage* CCompletionErrorPage::GetPrev() const {
    return nullptr;
}

wxWizardPage* CCompletionErrorPage::GetNext() const {
    return nullptr;
}

bool CCompletionErrorPage::HasNextPage() const {
    return false;
}

bool CCompletionErrorPage::HasPrevPage() const {
    return false;
}

void CCompletionErrorPage::SetPrev(CBOINCWizardPage*) {
    // no prev page expected
    return;
}

void CCompletionErrorPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);
    wxASSERT(m_pServerMessagesDescriptionCtrl);
    wxASSERT(m_pServerMessagesStaticBoxSizerCtrl);
    wxASSERT(m_pServerMessagesCtrl);

    if (m_pParent->GetIsAttachToProjectWizard()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Failed to add project")
        );
    } else if (m_pParent->GetIsAccountManagerWizard()) {
        if (m_pParent->GetIsAccountManagerUpdateWizard()) {
            m_pTitleStaticCtrl->SetLabel(
                _("Failed to update account manager")
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

    if (m_pParent->IsCancelInProgress() || m_pServerMessagesCtrl->GetLabel().IsEmpty()) {
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

    m_pParent->DisableBackButton();
}

void CCompletionErrorPage::OnCancel(wxWizardEvent& event) {
    m_pParent->ProcessCancelEvent(event);
}

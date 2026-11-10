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
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef BOINC_ACCOUNTINFOPAGE_H
#define BOINC_ACCOUNTINFOPAGE_H

class CAccountInfoPage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS(CAccountInfoPage)

public:
    CAccountInfoPage() = default;
    CAccountInfoPage(CWizardAttach* parent);
    bool Create(CWizardAttach* parent);

    void CreateControls();

    void OnPageChanged(wxWizardEvent& event);
    void OnPageChanging(wxWizardEvent& event);
    void OnCancel(wxWizardEvent& event);
    void OnAccountCreateCtrlSelected(wxCommandEvent& event);
    void OnAccountUseExistingCtrlSelected(wxCommandEvent& event);

    wxWizardPage* GetPrev() const;
    wxWizardPage* GetNext() const;

    void SetPrev(CBOINCWizardPage *prev);

    bool HasNextPage() const;
    bool HasPrevPage() const;

    bool GetAccountCreateCtrlValue() const { return m_pAccountCreateCtrl->GetValue(); }

    bool Validate();

private:
    wxStaticText* m_pTitleStaticCtrl = nullptr;
    wxStaticText* m_pAccountQuestionStaticCtrl = nullptr;
    wxRadioButton* m_pAccountCreateCtrl = nullptr;
    wxRadioButton* m_pAccountUseExistingCtrl = nullptr;
    wxStaticText* m_pAccountInformationStaticCtrl = nullptr;
    wxStaticText* m_pAccountEmailAddressStaticCtrl = nullptr;
    wxTextCtrl* m_pAccountEmailAddressCtrl = nullptr;
    wxStaticText* m_pAccountUsernameStaticCtrl = nullptr;
    wxTextCtrl* m_pAccountUsernameCtrl = nullptr;
    wxStaticText* m_pAccountPasswordStaticCtrl = nullptr;
    wxTextCtrl* m_pAccountPasswordCtrl = nullptr;
    wxStaticText* m_pAccountConfirmPasswordStaticCtrl = nullptr;
    wxTextCtrl* m_pAccountConfirmPasswordCtrl = nullptr;
    wxStaticText* m_pAccountPasswordRequirmentsStaticCtrl = nullptr;
    wxStaticText* m_pAccountManagerLinkLabelStaticCtrl = nullptr;
    wxHyperlinkCtrl* m_pAccountForgotPasswordCtrl = nullptr;
    wxString m_strAccountEmailAddress;
    wxString m_strAccountUsername;

    CWizardAttach *m_pParent = nullptr;
    CBOINCWizardPage *m_pPrev = nullptr;
};

#endif

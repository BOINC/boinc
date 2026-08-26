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

#include "WizardAttach.h"
class CAccountInfoPage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS(CAccountInfoPage)
    DECLARE_EVENT_TABLE()

public:
    CAccountInfoPage();
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
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pAccountQuestionStaticCtrl;
    wxRadioButton* m_pAccountCreateCtrl;
    wxRadioButton* m_pAccountUseExistingCtrl;
    wxStaticText* m_pAccountInformationStaticCtrl;
    wxStaticText* m_pAccountEmailAddressStaticCtrl;
    wxTextCtrl* m_pAccountEmailAddressCtrl;
    wxStaticText* m_pAccountUsernameStaticCtrl;
    wxTextCtrl* m_pAccountUsernameCtrl;
    wxStaticText* m_pAccountPasswordStaticCtrl;
    wxTextCtrl* m_pAccountPasswordCtrl;
    wxStaticText* m_pAccountConfirmPasswordStaticCtrl;
    wxTextCtrl* m_pAccountConfirmPasswordCtrl;
    wxStaticText* m_pAccountPasswordRequirmentsStaticCtrl;
    wxStaticText* m_pAccountManagerLinkLabelStaticCtrl;
    wxHyperlinkCtrl* m_pAccountForgotPasswordCtrl;
    wxString m_strAccountEmailAddress;
    wxString m_strAccountUsername;

    CWizardAttach *m_pParent;
    CBOINCWizardPage *m_pPrev;
};

#endif

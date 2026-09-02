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
#ifndef BOINC_TERMSOFUSEPAGE_H
#define BOINC_TERMSOFUSEPAGE_H

class CTermsOfUsePage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS(CTermsOfUsePage)
    DECLARE_EVENT_TABLE()

public:
    CTermsOfUsePage();
    CTermsOfUsePage(CWizardAttach* parent);
    bool Create(CWizardAttach* parent);

    void CreateControls();

    void OnLinkClicked(wxHtmlLinkEvent & event);
    void OnPageChanged(wxWizardEvent& event);
    void OnPageChanging(wxWizardEvent& event);
    void OnCancel(wxWizardEvent& event);
    void OnTermsOfUseStatusChange(wxCommandEvent& event);

    wxWizardPage* GetPrev() const;
    wxWizardPage* GetNext() const;

    void SetPrev(CBOINCWizardPage *prev);

    bool HasNextPage() const;
    bool HasPrevPage() const;

    bool GetUserAgrees() const { return m_bUserAgrees ; }
    void SetUserAgrees(bool value) { m_bUserAgrees = value ; }

    bool GetCredentialsAlreadyAvailable() const { return m_bCredentialsAlreadyAvailable ; }
    void SetCredentialsAlreadyAvailable(bool value) { m_bCredentialsAlreadyAvailable = value ; }

private:
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
    wxHtmlWindow* m_pTermsOfUseCtrl;
    wxRadioButton* m_pAgreeCtrl;
    wxRadioButton* m_pDisagreeCtrl;
    bool m_bUserAgrees;
    bool m_bCredentialsAlreadyAvailable;

    CWizardAttach *m_pParent;
    CBOINCWizardPage *m_pPrev;
};

#endif

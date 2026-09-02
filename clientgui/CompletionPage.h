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
#ifndef BOINC_COMPLETIONPAGE_H
#define BOINC_COMPLETIONPAGE_H

class CCompletionPage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS(CCompletionPage)
    DECLARE_EVENT_TABLE()

public:
    CCompletionPage();
    CCompletionPage(CWizardAttach* parent);
    bool Create(CWizardAttach* parent);

    void CreateControls();

    void OnPageChanged(wxWizardEvent& event);
    void OnCancel(wxWizardEvent& event);
    void OnFinished(wxWizardEvent& event);

    wxWizardPage* GetPrev() const;
    wxWizardPage* GetNext() const;

    void SetPrev(CBOINCWizardPage *prev);

    bool HasNextPage() const;
    bool HasPrevPage() const;

private:
    wxStaticText* m_pCompletionTitle;
    wxStaticText* m_pCompletionWelcome;
    wxStaticText* m_pCompletionBrandedMessage;
    wxStaticText* m_pCompletionMessage;

    CWizardAttach *m_pParent;
};

#endif

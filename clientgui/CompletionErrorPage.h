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
#ifndef BOINC_COMPLETIONERRORPAGE_H
#define BOINC_COMPLETIONERRORPAGE_H

#include "WizardAttach.h"
class CCompletionErrorPage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS(CCompletionErrorPage)
    DECLARE_EVENT_TABLE()

public:
    CCompletionErrorPage();
    CCompletionErrorPage(CWizardAttach* parent);
    bool Create(CWizardAttach* parent);

    void CreateControls();

    void OnPageChanged(wxWizardEvent& event);
    void OnCancel(wxWizardEvent& event);

    wxWizardPage* GetPrev() const;
    wxWizardPage* GetNext() const;

    void SetPrev(CBOINCWizardPage *prev);

    bool HasNextPage() const;
    bool HasPrevPage() const;

    wxString GetServerMessagesCtrlLabel() const { return m_pServerMessagesCtrl->GetLabel(); }
    void SetServerMessagesCtrlLabel(const wxString& label) { m_pServerMessagesCtrl->SetLabel(label); }

private:
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
    wxStaticBox* m_pServerMessagesDescriptionCtrl;
    wxStaticBoxSizer* m_pServerMessagesStaticBoxSizerCtrl;
    wxStaticText* m_pServerMessagesCtrl;

    CWizardAttach *m_pParent;
};

#endif

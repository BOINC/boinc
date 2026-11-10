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
#ifndef BOINC_ACCOUNTMANAGERINFOPAGE_H
#define BOINC_ACCOUNTMANAGERINFOPAGE_H

class CAcctMgrListItem: public wxObject {
    DECLARE_DYNAMIC_CLASS(CAcctMgrListItem)
public:

    wxString GetURL() const { return m_strURL ; }
    void SetURL(wxString value) { m_strURL = value ; }

    wxString GetName() const { return m_strName ; }
    void SetName(wxString value) { m_strName = value ; }

    wxString GetImage() const { return m_strImage ; }
    void SetImage(wxString value) { m_strImage = value ; }

    wxString GetDescription() const { return m_strDescription ; }
    void SetDescription(wxString value) { m_strDescription = value ; }

private:
    wxString m_strURL;
    wxString m_strName;
    wxString m_strImage;
    wxString m_strDescription;
};

class CAccountManagerInfoPage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS(CAccountManagerInfoPage)

public:
    CAccountManagerInfoPage() = default;
    CAccountManagerInfoPage(CWizardAttach* parent);
    bool Create( CWizardAttach* parent );

    void CreateControls();

    void OnPageChanged( wxWizardEvent& event );
    void OnPageChanging( wxWizardEvent& event );
    void OnProjectSelected( wxCommandEvent& event );
    void OnURLChanged( wxCommandEvent& event );
    void OnProjectItemDisplay( wxCommandEvent& event );
    void OnCancel( wxWizardEvent& event );

    wxWizardPage* GetPrev() const;
    wxWizardPage* GetNext() const;

    void SetPrev(CBOINCWizardPage *prev);

    bool HasNextPage() const;
    bool HasPrevPage() const;

private:
    wxStaticText* m_pTitleStaticCtrl = nullptr;
    wxStaticText* m_pDescriptionStaticCtrl = nullptr;
    wxListBox* m_pProjectListCtrl = nullptr;
    wxStaticText* m_pProjectDetailsStaticCtrl = nullptr;
    wxTextCtrl* m_pProjectDetailsDescriptionCtrl = nullptr;
    wxButton* m_pOpenWebSiteButton = nullptr;
    wxStaticText* m_pProjectUrlStaticCtrl = nullptr;
    wxTextCtrl* m_pProjectUrlCtrl = nullptr;
    bool m_bAccountManagerListPopulated = false;

    CWizardAttach *m_pParent = nullptr;
    CBOINCWizardPage *m_pPrev = nullptr;
};

#endif

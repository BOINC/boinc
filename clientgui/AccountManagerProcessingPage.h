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
#ifndef BOINC_ACCOUNTMANAGERPROCESSINGPAGE_H
#define BOINC_ACCOUNTMANAGERPROCESSINGPAGE_H

class CAccountManagerProcessingPageEvent : public wxEvent {
public:
    CAccountManagerProcessingPageEvent(wxEventType evtType, wxWizardPage *parent)
        : wxEvent(-1, evtType) {
            SetEventObject(parent);
        }

    virtual wxEvent *Clone() const { return new CAccountManagerProcessingPageEvent(*this); }
};

wxDECLARE_EVENT(wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE, CAccountManagerProcessingPageEvent);

#define ATTACHACCTMGR_INIT                              0
#define ATTACHACCTMGR_ATTACHACCTMGR_BEGIN               1
#define ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE             2
#define ATTACHACCTMGR_CLEANUP                           3
#define ATTACHACCTMGR_END                               4

class CAccountManagerProcessingPage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS( CAccountManagerProcessingPage )

public:
    CAccountManagerProcessingPage() = default;
    CAccountManagerProcessingPage(CWizardAttach* parent);
    bool Create(CWizardAttach* parent);

    void CreateControls();

    void OnPageChanged(wxWizardEvent& event);
    void OnPageChanging(wxWizardEvent& event);
    void OnCancel(wxWizardEvent& event);
    void OnStateChange(CAccountManagerProcessingPageEvent& event);

    wxWizardPage* GetPrev() const;
    wxWizardPage* GetNext() const;

    void SetPrev(CBOINCWizardPage *prev);

    bool HasNextPage() const;
    bool HasPrevPage() const;

    wxBitmap GetBitmapResource(const wxString& name);

    bool GetProjectCommunicationsSucceeded() const { return m_bProjectCommunicationsSucceeded ; }
    void SetProjectCommunicationsSucceeded(bool value) { m_bProjectCommunicationsSucceeded = value ; }

    bool GetProjectUnavailable() const { return m_bProjectUnavailable ; }
    void SetProjectUnavailable(bool value) { m_bProjectUnavailable = value ; }

    bool GetProjectAccountAlreadyExists() const { return m_bProjectAccountAlreadyExists ; }
    void SetProjectAccountAlreadyExists(bool value) { m_bProjectAccountAlreadyExists = value ; }

    bool GetProjectAccountNotFound() const { return m_bProjectAccountNotFound ; }
    void SetProjectAccountNotFound(bool value) { m_bProjectAccountNotFound = value ; }

    bool GetProjectAttachSucceeded() const { return m_bProjectAttachSucceeded ; }
    void SetProjectAttachSucceeded(bool value) { m_bProjectAttachSucceeded = value ; }

    wxInt32 GetCurrentState() const { return m_iCurrentState ; }
    void SetNextState(wxInt32 value) { m_iCurrentState = value ; }

    void StartProgress(wxStaticBitmap* pBitmap);
    void IncrementProgress(wxStaticBitmap* pBitmap);
    void FinishProgress(wxStaticBitmap* pBitmap);

private:
    wxStaticText* m_pTitleStaticCtrl = nullptr;
    wxStaticText* m_pPleaseWaitStaticCtrl = nullptr;
    wxStaticBitmap* m_pProgressIndicator = nullptr;
    bool m_bProjectCommunicationsSucceeded = false;
    bool m_bProjectUnavailable = false;
    bool m_bProjectAccountNotFound = false;
    bool m_bProjectAccountAlreadyExists = false;
    bool m_bProjectAttachSucceeded = false;
    int m_iBitmapIndex = 0;
    int m_iCurrentState = 0;

    CWizardAttach *m_pParent = nullptr;
    CBOINCWizardPage *m_pPrev = nullptr;
};

#endif

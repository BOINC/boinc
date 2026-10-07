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
#ifndef BOINC_ACCOUNTMANAGERPROPERTIESPAGE_H
#define BOINC_ACCOUNTMANAGERPROPERTIESPAGE_H

class CAccountManagerPropertiesPageEvent : public wxEvent {
public:
    CAccountManagerPropertiesPageEvent(wxEventType evtType, wxWizardPage *parent)
        : wxEvent(-1, evtType) {
            SetEventObject(parent);
        }

    virtual wxEvent *Clone() const { return new CAccountManagerPropertiesPageEvent(*this); }
};

wxDECLARE_EVENT(wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE, CAccountManagerPropertiesPageEvent);

#define ACCTMGRPROP_INIT                                   0
#define ACCTMGRPROP_RETRPROJECTPROPERTIES_BEGIN            1
#define ACCTMGRPROP_RETRPROJECTPROPERTIES_EXECUTE          2
#define ACCTMGRPROP_DETERMINENETWORKSTATUS_BEGIN           3
#define ACCTMGRPROP_DETERMINENETWORKSTATUS_EXECUTE         4
#define ACCTMGRPROP_DETERMINEACCOUNTINFOSTATUS_BEGIN       5
#define ACCTMGRPROP_DETERMINEACCOUNTINFOSTATUS_EXECUTE     6
#define ACCTMGRPROP_CLEANUP                                7
#define ACCTMGRPROP_END                                    8

class CAccountManagerPropertiesPage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS(CAccountManagerPropertiesPage)

public:
    CAccountManagerPropertiesPage() = default;
    CAccountManagerPropertiesPage(CWizardAttach* parent);
    bool Create(CWizardAttach* parent);

    void CreateControls();

    void OnPageChanged(wxWizardEvent& event);
    void OnPageChanging(wxWizardEvent& event);
    void OnCancel(wxWizardEvent& event);
    void OnStateChange(CAccountManagerPropertiesPageEvent& event);

    wxWizardPage* GetPrev() const;
    wxWizardPage* GetNext() const;

    void SetPrev(CBOINCWizardPage *prev);

    bool HasNextPage() const;
    bool HasPrevPage() const;

    wxBitmap GetBitmapResource(const wxString& name);

    bool GetProjectPropertiesSucceeded() const { return m_bProjectPropertiesSucceeded ; }
    void SetProjectPropertiesSucceeded(bool value) { m_bProjectPropertiesSucceeded = value ; }

    bool GetProjectPropertiesURLFailure() const { return m_bProjectPropertiesURLFailure ; }
    void SetProjectPropertiesURLFailure(bool value) { m_bProjectPropertiesURLFailure = value ; }

    bool GetProjectPropertiesCommunicationFailure() const { return m_bProjectPropertiesCommunicationFailure ; }
    void SetProjectPropertiesCommunicationFailure(bool value) { m_bProjectPropertiesCommunicationFailure = value ; }

    bool GetProjectAccountCreationDisabled() const { return m_bProjectAccountCreationDisabled ; }
    void SetProjectAccountCreationDisabled(bool value) { m_bProjectAccountCreationDisabled = value ; }

    bool GetProjectClientAccountCreationDisabled() const { return m_bProjectClientAccountCreationDisabled ; }
    void SetProjectClientAccountCreationDisabled(bool value) { m_bProjectClientAccountCreationDisabled = value ; }

    bool GetNetworkConnectionNotDetected() const { return m_bNetworkConnectionNotDetected ; }
    void SetNetworkConnectionNotDetected(bool value) { m_bNetworkConnectionNotDetected = value ; }

    bool GetServerReportedError() const { return m_bServerReportedError ; }
    void SetServerReportedError(bool value) { m_bServerReportedError = value ; }

    bool GetTermsOfUseRequired() const { return m_bTermsOfUseRequired ; }
    void SetTermsOfUseRequired(bool value) { m_bTermsOfUseRequired = value ; }

    bool GetCredentialsAlreadyAvailable() const { return m_bCredentialsAlreadyAvailable ; }
    void SetCredentialsAlreadyAvailable(bool value) { m_bCredentialsAlreadyAvailable = value ; }

    wxInt32 GetCurrentState() const { return m_iCurrentState ; }
    void SetNextState(wxInt32 value) { m_iCurrentState = value ; }

    void StartProgress(wxStaticBitmap* pBitmap);
    void IncrementProgress(wxStaticBitmap* pBitmap);
    void FinishProgress(wxStaticBitmap* pBitmap);

private:
    wxStaticText* m_pTitleStaticCtrl = nullptr;
    wxStaticText* m_pPleaseWaitStaticCtrl = nullptr;
    wxStaticBitmap* m_pProgressIndicator = nullptr;
    bool m_bProjectPropertiesSucceeded = false;
    bool m_bProjectPropertiesURLFailure = false;
    bool m_bProjectPropertiesCommunicationFailure = false;
    bool m_bProjectAccountCreationDisabled = false;
    bool m_bProjectClientAccountCreationDisabled = false;
    bool m_bNetworkConnectionNotDetected = false;
    bool m_bServerReportedError = false;
    bool m_bTermsOfUseRequired = false;
    bool m_bCredentialsAlreadyAvailable = false;
    int m_iBitmapIndex = 0;
    int m_iCurrentState = 0;

    CWizardAttach *m_pParent = nullptr;
    CBOINCWizardPage *m_pPrev = nullptr;
};

#endif

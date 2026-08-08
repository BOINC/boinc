// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
#ifndef BOINC_PROJECTPROPERTIESPAGE_H
#define BOINC_PROJECTPROPERTIESPAGE_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ProjectPropertiesPage.cpp"
#endif

/*!
 * CProjectPropertiesPage custom events
 */

class CProjectPropertiesPageEvent : public wxEvent
{
public:
    CProjectPropertiesPageEvent(wxEventType evtType, wxWizardPageEx *parent)
        : wxEvent(-1, evtType)
        {
            SetEventObject(parent);
        }

    virtual wxEvent *Clone() const { return new CProjectPropertiesPageEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_PROJECTPROPERTIES_STATECHANGE, 11000 )
END_DECLARE_EVENT_TYPES()

#define EVT_PROJECTPROPERTIES_STATECHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_PROJECTPROPERTIES_STATECHANGE, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

/*!
 * CProjectPropertiesPage states
 */

#define PROJPROP_INIT                                   0
#define PROJPROP_RETRPROJECTPROPERTIES_BEGIN            1
#define PROJPROP_RETRPROJECTPROPERTIES_EXECUTE          2
#define PROJPROP_DETERMINENETWORKSTATUS_BEGIN           3
#define PROJPROP_DETERMINENETWORKSTATUS_EXECUTE         4
#define PROJPROP_DETERMINEACCOUNTINFOSTATUS_BEGIN       5
#define PROJPROP_DETERMINEACCOUNTINFOSTATUS_EXECUTE     6
#define PROJPROP_CLEANUP                                7
#define PROJPROP_END                                    8

/*!
 * CProjectPropertiesPage class declaration
 */

class CProjectPropertiesPage: public wxWizardPageEx
{
    DECLARE_DYNAMIC_CLASS( CProjectPropertiesPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectPropertiesPage( );

    CProjectPropertiesPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectPropertiesPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTPROPERTIESPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_PROJECTPROPERTIESPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CProjectPropertiesPage event handler declarations

    void OnStateChange( CProjectPropertiesPageEvent& event );

////@begin CProjectPropertiesPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CProjectPropertiesPage member function declarations

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

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Progress Image Support
    void StartProgress(wxStaticBitmap* pBitmap);
    void IncrementProgress(wxStaticBitmap* pBitmap);
    void FinishProgress(wxStaticBitmap* pBitmap);

////@begin CProjectPropertiesPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticBitmap* m_pProgressIndicator;
////@end CProjectPropertiesPage member variables
    bool m_bProjectPropertiesSucceeded;
    bool m_bProjectPropertiesURLFailure;
    bool m_bProjectPropertiesCommunicationFailure;
    bool m_bProjectAccountCreationDisabled;
    bool m_bProjectClientAccountCreationDisabled;
    bool m_bNetworkConnectionNotDetected;
    bool m_bServerReportedError;
    bool m_bTermsOfUseRequired;
    bool m_bCredentialsAlreadyAvailable;
    int m_iBitmapIndex;
    int m_iCurrentState;
};

#endif

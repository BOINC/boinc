// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
#ifndef _WIZ_ACCOUNTMANAGERPROPERTIESPAGE_H_
#define _WIZ_ACCOUNTMANAGERPROPERTIESPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "AccountManagerPropertiesPage.cpp"
#endif

/*!
 * CAccountManagerPropertiesPage custom events
 */

class CAccountManagerPropertiesPageEvent : public wxEvent
{
public:
    CAccountManagerPropertiesPageEvent(wxEventType evtType, wxWizardPageEx *parent)
        : wxEvent(-1, evtType)
        {
            SetEventObject(parent);
        }

    virtual wxEvent *Clone() const { return new CAccountManagerPropertiesPageEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE, 11000 )
END_DECLARE_EVENT_TYPES()

#define EVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

/*!
 * CAccountManagerPropertiesPage states
 */

#define ACCTMGRPROP_INIT                                   0
#define ACCTMGRPROP_RETRPROJECTPROPERTIES_BEGIN            1
#define ACCTMGRPROP_RETRPROJECTPROPERTIES_EXECUTE          2
#define ACCTMGRPROP_DETERMINENETWORKSTATUS_BEGIN           3
#define ACCTMGRPROP_DETERMINENETWORKSTATUS_EXECUTE         4
#define ACCTMGRPROP_CLEANUP                                5
#define ACCTMGRPROP_END                                    6

/*!
 * CAccountManagerPropertiesPage class declaration
 */

class CAccountManagerPropertiesPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CAccountManagerPropertiesPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountManagerPropertiesPage( );

    CAccountManagerPropertiesPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountManagerPropertiesPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTMANAGERPROPERTIESPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTMANAGERPROPERTIESPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CAccountManagerPropertiesPage event handler declarations

    void OnStateChange( CAccountManagerPropertiesPageEvent& event );

////@begin CAccountManagerPropertiesPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAccountManagerPropertiesPage member function declarations

    bool GetProjectPropertiesSucceeded() const { return m_bProjectPropertiesSucceeded ; }
    void SetProjectPropertiesSucceeded(bool value) { m_bProjectPropertiesSucceeded = value ; }

    bool GetProjectPropertiesURLFailure() const { return m_bProjectPropertiesURLFailure ; }
    void SetProjectPropertiesURLFailure(bool value) { m_bProjectPropertiesURLFailure = value ; }

    bool GetProjectAccountCreationDisabled() const { return m_bProjectAccountCreationDisabled ; }
    void SetProjectAccountCreationDisabled(bool value) { m_bProjectAccountCreationDisabled = value ; }

    bool GetProjectClientAccountCreationDisabled() const { return m_bProjectClientAccountCreationDisabled ; }
    void SetProjectClientAccountCreationDisabled(bool value) { m_bProjectClientAccountCreationDisabled = value ; }

    bool GetNetworkConnectionDetected() const { return m_bNetworkConnectionDetected ; }
    void SetNetworkConnectionDetected(bool value) { m_bNetworkConnectionDetected = value ; }

    wxInt32 GetCurrentState() const { return m_iCurrentState ; }
    void SetNextState(wxInt32 value) { m_iCurrentState = value ; }

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Progress Image Support
    void StartProgress(wxStaticBitmap* pBitmap);
    void IncrementProgress(wxStaticBitmap* pBitmap);
    void FinishProgress(wxStaticBitmap* pBitmap);

////@begin CAccountManagerPropertiesPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pPleaseWaitStaticCtrl;
    wxStaticBitmap* m_pProgressIndicator;
////@end CAccountManagerPropertiesPage member variables
    bool m_bProjectPropertiesSucceeded;
    bool m_bProjectPropertiesURLFailure;
    bool m_bProjectAccountCreationDisabled;
    bool m_bProjectClientAccountCreationDisabled;
    bool m_bNetworkConnectionDetected;
    int m_iBitmapIndex;
    int m_iCurrentState;
};

#endif // _WIZ_ACCOUNTMANAGERPROPERTIESPAGE_H_

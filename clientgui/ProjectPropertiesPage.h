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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#ifndef _WIZ_PROJECTPROPERTIESPAGE_H_
#define _WIZ_PROJECTPROPERTIESPAGE_H_

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
#define PROJPROP_COMMUNICATEYAHOO_BEGIN                 3
#define PROJPROP_COMMUNICATEYAHOO_EXECUTE               4
#define PROJPROP_COMMUNICATEGOOGLE_BEGIN                5
#define PROJPROP_COMMUNICATEGOOGLE_EXECUTE              6
#define PROJPROP_DETERMINENETWORKSTATUS_BEGIN           7
#define PROJPROP_DETERMINENETWORKSTATUS_EXECUTE         8
#define PROJPROP_CLEANUP                                9
#define PROJPROP_END                                    10

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

    bool GetProjectAccountCreationDisabled() const { return m_bProjectAccountCreationDisabled ; }
    void SetProjectAccountCreationDisabled(bool value) { m_bProjectAccountCreationDisabled = value ; }

    bool GetProjectClientAccountCreationDisabled() const { return m_bProjectClientAccountCreationDisabled ; }
    void SetProjectClientAccountCreationDisabled(bool value) { m_bProjectClientAccountCreationDisabled = value ; }

    bool GetProjectAlreadyAttached() const { return m_bProjectAlreadyAttached ; }
    void SetProjectAlreadyAttached(bool value) { m_bProjectAlreadyAttached = value ; }

    bool GetCommunicateYahooSucceeded() const { return m_bCommunicateYahooSucceeded ; }
    void SetCommunicateYahooSucceeded(bool value) { m_bCommunicateYahooSucceeded = value ; }

    bool GetCommunicateGoogleSucceeded() const { return m_bCommunicateGoogleSucceeded ; }
    void SetCommunicateGoogleSucceeded(bool value) { m_bCommunicateGoogleSucceeded = value ; }

    bool GetDeterminingConnectionStatusSucceeded() const { return m_bDeterminingConnectionStatusSucceeded ; }
    void SetDeterminingConnectionStatusSucceeded(bool value) { m_bDeterminingConnectionStatusSucceeded = value ; }

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
    bool m_bProjectAccountCreationDisabled;
    bool m_bProjectClientAccountCreationDisabled;
    bool m_bProjectAlreadyAttached;
    bool m_bCommunicateYahooSucceeded;
    bool m_bCommunicateGoogleSucceeded;
    bool m_bDeterminingConnectionStatusSucceeded;
    int m_iBitmapIndex;
    int m_iCurrentState;
};

#endif // _WIZ_PROJECTPROPERTIESPAGE_H_

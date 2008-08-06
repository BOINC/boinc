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
#ifndef _WIZ_ACCOUNTMANAGERPROCESSINGPAGE_H_
#define _WIZ_ACCOUNTMANAGERPROCESSINGPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "AccountManagerProcessingPage.cpp"
#endif

/*!
 * CAccountManagerProcessingPage custom events
 */

class CAccountManagerProcessingPageEvent : public wxEvent
{
public:
    CAccountManagerProcessingPageEvent(wxEventType evtType, wxWizardPageEx *parent)
        : wxEvent(-1, evtType)
        {
            SetEventObject(parent);
        }

    virtual wxEvent *Clone() const { return new CAccountManagerProcessingPageEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE, 11100 )
END_DECLARE_EVENT_TYPES()

#define EVT_ACCOUNTMANAGERPROCESSING_STATECHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_ACCOUNTMANAGERPROCESSING_STATECHANGE, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

/*!
 * CAccountManagerProcessingPage states
 */

#define ATTACHACCTMGR_INIT                              0
#define ATTACHACCTMGR_ATTACHACCTMGR_BEGIN               1
#define ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE             2
#define ATTACHACCTMGR_CLEANUP                           3
#define ATTACHACCTMGR_END                               4

/*!
 * CAccountManagerProcessingPage class declaration
 */

class CAccountManagerProcessingPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CAccountManagerProcessingPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountManagerProcessingPage( );

    CAccountManagerProcessingPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountManagerProcessingPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHACCOUNTMANAGERPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ATTACHACCOUNTMANAGERPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CAccountManagerProcessingPage event handler declarations

    void OnStateChange( CAccountManagerProcessingPageEvent& event );

////@begin CAccountManagerProcessingPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAccountManagerProcessingPage member function declarations

    bool GetProjectCommunitcationsSucceeded() const { return m_bProjectCommunitcationsSucceeded ; }
    void SetProjectCommunitcationsSucceeded(bool value) { m_bProjectCommunitcationsSucceeded = value ; }

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

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Progress Image Support
    void StartProgress(wxStaticBitmap* pBitmap);
    void IncrementProgress(wxStaticBitmap* pBitmap);
    void FinishProgress(wxStaticBitmap* pBitmap);

////@begin CAccountManagerProcessingPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pPleaseWaitStaticCtrl;
    wxStaticBitmap* m_pProgressIndicator;
////@end CAccountManagerProcessingPage member variables
    bool m_bProjectCommunitcationsSucceeded;
    bool m_bProjectUnavailable;
    bool m_bProjectAccountNotFound;
    bool m_bProjectAccountAlreadyExists;
    bool m_bProjectAttachSucceeded;
    int m_iBitmapIndex;
    int m_iCurrentState;
};

#endif // _WIZ_ACCOUNTMANAGERPROCESSINGPAGE_H_

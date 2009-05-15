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
#ifndef _WIZ_ACCOUNTMANAGER_H_
#define _WIZ_ACCOUNTMANAGER_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "WizardAccountManager.cpp"
#endif

/*!
 * Forward declarations
 */

////@begin forward declarations
class CAccountManagerInfoPage;
class CAccountManagerPropertiesPage;
class CAccountManagerProcessingPage;
////@end forward declarations

/*!
 * CWizardAccountManager class declaration
 */

class CWizardAccountManager: public CBOINCBaseWizard
{    
    DECLARE_DYNAMIC_CLASS( CWizardAccountManager )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWizardAccountManager( );
    CWizardAccountManager( wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDACCOUNTMANAGER_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDACCOUNTMANAGER_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CWizardAccountManager event handler declarations

    /// wxEVT_WIZARD_FINISHED event handler for ID_ATTACHACCOUNTMANAGERWIZARD
    void OnFinished( wxWizardEvent& event );

////@end CWizardAccountManager event handler declarations

////@begin CWizardAccountManager member function declarations

    /// Runs the wizard.
    bool Run(int action = ACCOUNTMANAGER_ATTACH);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWizardAccountManager member function declarations

    /// Overrides
    virtual bool HasNextPage( wxWizardPageEx* page );
    virtual bool HasPrevPage( wxWizardPageEx* page );

    /// Track page transitions
    wxWizardPageEx* _PopPageTransition();
    wxWizardPageEx* _PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID );

    /// Cancel Event Infrastructure
    void _ProcessCancelEvent( wxWizardExEvent& event );

    /// Finish Button Environment
    bool GetAccountCreatedSuccessfully() const { return account_created_successfully ; }
    void SetAccountCreatedSuccessfully(bool value) { account_created_successfully = value ; }

    bool GetAttachedToProjectSuccessfully() const { return attached_to_project_successfully ; }
    void SetAttachedToProjectSuccessfully(bool value) { attached_to_project_successfully = value ; }

    wxString GetProjectURL() const { return project_url ; }
    void SetProjectURL(wxString value) { project_url = value ; }

    wxString GetProjectAuthenticator() const { return project_authenticator ; }
    void SetProjectAuthenticator(wxString value) { project_authenticator = value ; }

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWizardAccountManager member variables
    CWelcomePage* m_WelcomePage;
    CAccountManagerInfoPage* m_AccountManagerInfoPage;
    CAccountManagerPropertiesPage* m_AccountManagerPropertiesPage;
    CAccountManagerProcessingPage* m_AccountManagerProcessingPage;
    CTermsOfUsePage* m_TermsOfUsePage;
    CAccountInfoPage* m_AccountInfoPage;
    CCompletionPage* m_CompletionPage;
    CCompletionErrorPage* m_CompletionErrorPage;
    CErrNotDetectedPage* m_ErrNotDetectedPage;
    CErrUnavailablePage* m_ErrUnavailablePage;
    CErrNoInternetConnectionPage* m_ErrNoInternetConnectionPage;
    CErrNotFoundPage* m_ErrNotFoundPage;
    CErrProxyInfoPage* m_ErrProxyInfoPage;
    CErrProxyPage* m_ErrProxyPage;
////@end CWizardAccountManager member variables
    wxString m_strProjectName;
    bool m_bCredentialsCached;
};

#endif // _WIZ_ACCOUNTMANAGER_H_

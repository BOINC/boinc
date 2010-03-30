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
#ifndef _WIZ_ATTACHPROJECT_H_
#define _WIZ_ATTACHPROJECT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "WizardAttachProject.cpp"
#endif

/*!
 * Forward declarations
 */

////@begin forward declarations
class CProjectInfoPage;
class CProjectPropertiesPage;
class CProjectProcessingPage;
class CAccountManagerInfoPage;
class CAccountManagerPropertiesPage;
class CAccountManagerProcessingPage;
////@end forward declarations

#define ACCOUNTMANAGER_ATTACH       0
#define ACCOUNTMANAGER_UPDATE       1
#define ACCOUNTMANAGER_DETACH       2


/*!
 * CWizardAttachProject class declaration
 */

class CWizardAttachProject: public CBOINCBaseWizard
{    
    DECLARE_DYNAMIC_CLASS( CWizardAttachProject )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWizardAttachProject( );
    CWizardAttachProject( wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDATTACHPROJECT_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDATTACHPROJECT_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CWizardAttachProject event handler declarations

    /// wxEVT_WIZARD_FINISHED event handler for ID_ATTACHPROJECTWIZARD
    void OnFinished( wxWizardExEvent& event );

////@end CWizardAttachProject event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_BACKWARD
    void OnWizardBack( wxCommandEvent& event );
    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_FORWARD
    void OnWizardNext( wxCommandEvent& event );

////@begin CWizardAttachProject member function declarations

    /// Runs the wizard.
    bool Run(
        wxString& strName, wxString& strURL, std::string& team_name,
        bool bCredentialsCached = true
    );
    
    // Synchronize to Account Manager
    bool SyncToAccountManager();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWizardAttachProject member function declarations

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

////@begin CWizardAttachProject member variables
    CWelcomePage* m_WelcomePage;
    CProjectInfoPage* m_ProjectInfoPage;
    CProjectPropertiesPage* m_ProjectPropertiesPage;
    CProjectProcessingPage* m_ProjectProcessingPage;
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
    CErrAlreadyExistsPage* m_ErrAlreadyExistsPage;
    CErrProxyInfoPage* m_ErrProxyInfoPage;
    CErrProxyPage* m_ErrProxyPage;
////@end CWizardAttachProject member variables
    bool m_bCredentialsCached;
    bool m_bCredentialsDetected;
    wxString m_strProjectName;
    wxString m_strReturnURL;
    bool m_bCookieRequired;
    wxString m_strCookieFailureURL;
    std::string team_name;
};

#endif // _WIZ_ATTACHPROJECT_H_

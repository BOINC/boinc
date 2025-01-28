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
#ifndef BOINC_WIZARDATTACH_H
#define BOINC_WIZARDATTACH_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "WizardAttach.cpp"
#endif


// Wizard Identifiers
//
#define ID_ATTACHWIZARD 10000
#define SYMBOL_CWIZARDATTACH_IDNAME ID_ATTACHWIZARD


// Page Identifiers
//

// Generic Pages
#define ID_ACCOUNTINFOPAGE 10102
#define ID_COMPLETIONPAGE 10103
#define ID_COMPLETIONERRORPAGE 10104
#define ID_ERRNOTDETECTEDPAGE 10105
#define ID_ERRUNAVAILABLEPAGE 10106
#define ID_ERRNOINTERNETCONNECTIONPAGE 10108
#define ID_ERRNOTFOUNDPAGE 10109
#define ID_ERRALREADYEXISTSPAGE 10110
#define ID_ERRPROXYINFOPAGE 10111
#define ID_ERRPROXYPAGE 10112
#define ID_TERMSOFUSEPAGE 10113

// Attach to Project Wizard Pages
#define ID_PROJECTINFOPAGE 10200
#define ID_PROJECTPROPERTIESPAGE 10201
#define ID_PROJECTPROCESSINGPAGE 10202
#define ID_PROJECTWELCOMEPAGE 10203

// Account Manager Wizard Pages
#define ID_ACCOUNTMANAGERINFOPAGE 10300
#define ID_ACCOUNTMANAGERPROPERTIESPAGE 10301
#define ID_ACCOUNTMANAGERPROCESSINGPAGE 10302


// Control Identifiers
//

// Bitmap Progress Control
#define ID_PROGRESSCTRL 11000

// BOINC Hyperlink Control
#define ID_BOINCHYPERLINK 11001

// Completion Error Page Multiline Text Control
#define ID_TEXTCTRL 11002

// Welcome Page Controls
#define ID_WELCOMESELECTWIZARDPROJECT 11100
#define ID_WELCOMESELECTWIZARDACCOUNTMGR 11101
#define ID_WELCOMECHANGEAPPS 11102

// Project Info/Account Manager Info Controls
#define ID_CATEGORIES 11200
#define ID_PROJECTS 11201
#define ID_PROJECTDESCRIPTION 11202
#define ID_PROJECTLISTCTRL 11203
#define ID_PROJECTURLSTATICCTRL 11204
#define ID_PROJECTURLCTRL 11205
#define ID_PROJECTWEBPAGECTRL 11206

// Terms Of Use Controls
#define ID_TERMSOFUSECTRL 11300
#define ID_TERMSOFUSEAGREECTRL 11301
#define ID_TERMSOFUSEDISAGREECTRL 11302

// Account Info Controls
#define ID_ACCOUNTCREATECTRL 11400
#define ID_ACCOUNTUSEEXISTINGCTRL 11401
#define ID_ACCOUNTEMAILADDRESSSTATICCTRL 11402
#define ID_ACCOUNTEMAILADDRESSCTRL 11403
#define ID_ACCOUNTUSERNAMESTATICCTRL 11404
#define ID_ACCOUNTUSERNAMECTRL 11405
#define ID_ACCOUNTPASSWORDSTATICCTRL 11406
#define ID_ACCOUNTPASSWORDCTRL 11407
#define ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL 11408
#define ID_ACCOUNTCONFIRMPASSWORDCTRL 11409
#define ID_ACCOUNTREQUIREMENTSSTATICCTRL 11410
#define ID_ACCOUNTLINKLABELSTATICCTRL 11411
#define ID_ACCOUNTFORGOTPASSWORDCTRL 11412
#define ID_ACCOUNTCOOKIEDETECTIONFAILEDCTRL 11413

// Proxy Page Controls
#define ID_PROXYHTTPSERVERSTATICCTRL 11500
#define ID_PROXYHTTPSERVERCTRL 11501
#define ID_PROXYHTTPPORTSTATICCTRL 11502
#define ID_PROXYHTTPPORTCTRL 11503
#define ID_PROXYHTTPUSERNAMESTATICCTRL 11504
#define ID_PROXYHTTPUSERNAMECTRL 11505
#define ID_PROXYHTTPPASSWORDSTATICCTRL 11506
#define ID_PROXYHTTPPASSWORDCTRL 11507
#define ID_PROXYHTTPAUTODETECTCTRL 11508
#define ID_PROXYSOCKSSERVERSTATICCTRL 11509
#define ID_PROXYSOCKSSERVERCTRL 11510
#define ID_PROXYSOCKSPORTSTATICCTRL 11511
#define ID_PROXYSOCKSPORTCTRL 11512
#define ID_PROXYSOCKSUSERNAMESTATICCTRL 11513
#define ID_PROXYSOCKSUSERNAMECTRL 11514
#define ID_PROXYSOCKSPASSWORDSTATICCTRL 11515
#define ID_PROXYSOCKSPASSWORDCTRL 11516

// Account Manager Status Controls
#define ID_ACCTMANAGERNAMECTRL 11600
#define ID_ACCTMANAGERLINKCTRL 11601
#define ID_ACCTMANAGERUPDATECTRL 11602
#define ID_ACCTMANAGERREMOVECTRL 11603


// Forward declare the generic page classes
//
class CAccountInfoPage;
class CTermsOfUsePage;
class CCompletionPage;
class CCompletionErrorPage;
class CErrNotDetectedPage;
class CErrUnavailablePage;
class CErrAlreadyAttachedPage;
class CErrNoInternetConnectionPage;
class CErrNotFoundPage;
class CErrAlreadyExistsPage;
class CErrProxyInfoPage;
class CErrProxyPage;
class CErrUserDisagreesPage;
class CProjectInfoPage;
class CProjectPropertiesPage;
class CProjectProcessingPage;
class CProjectWelcomePage;
class CAccountManagerInfoPage;
class CAccountManagerPropertiesPage;
class CAccountManagerProcessingPage;

// Forward declare PROJECT_INIT_STATUS
struct PROJECT_INIT_STATUS;


// Wizard Detection
//
#define IS_ATTACHTOPROJECTWIZARD() \
    ((CWizardAttach*)GetParent())->IsAttachToProjectWizard

#define IS_ACCOUNTMANAGERWIZARD() \
    ((CWizardAttach*)GetParent())->IsAccountManagerWizard

#define IS_ACCOUNTMANAGERUPDATEWIZARD() \
    ((CWizardAttach*)GetParent())->IsAccountManagerUpdateWizard

#define IS_CHANGEWCGAPPS() \
	((CWizardAttach*)GetParent())->IsChangeWCGApps


// Commonly defined macros
//
#define PAGE_TRANSITION_NEXT(id) \
    ((CWizardAttach*)GetParent())->PushPageTransition((wxWizardPageEx*)this, id)

#define PAGE_TRANSITION_BACK \
    ((CWizardAttach*)GetParent())->PopPageTransition()

#define PROCESS_CANCELEVENT(event) \
    ((CWizardAttach*)GetParent())->ProcessCancelEvent(event)

#define CHECK_CLOSINGINPROGRESS() \
    ((CWizardAttach*)GetParent())->IsCancelInProgress()


/*!
 * CWizardAttach class declaration
 */

class CWizardAttach: public CBOINCBaseWizard
{
    DECLARE_DYNAMIC_CLASS( CWizardAttach )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWizardAttach( );
    CWizardAttach( wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDATTACH_IDNAME, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, long style = wxDEFAULT_DIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDATTACH_IDNAME, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, long style = wxDEFAULT_DIALOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CWizardAttachProject event handler declarations

    /// wxEVT_WIZARD_FINISHED event handler for ID_ATTACHWIZARD
    void OnFinished( wxWizardExEvent& event );

////@end CWizardAttachProject event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_BACKWARD
    void OnWizardBack( wxCommandEvent& event );
    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_FORWARD
    void OnWizardNext( wxCommandEvent& event );

////@begin CWizardAttachProject member function declarations

    /// Runs the wizard.
    bool Run(
        wxString strProjectName,
        wxString strProjectURL,
        wxString strProjectAuthenticator,
        wxString strProjectInstitution,
        wxString strProjectDescription,
        wxString strProjectKnown,
        bool     bAccountKeyDetected,
        bool     bEmbedded
    );

    /// Synchronize to Account Manager
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
    wxWizardPageEx* TranslatePage(unsigned long ulPageID);
    wxWizardPageEx* PopPageTransition();
    wxWizardPageEx* PushPage( unsigned long ulPageID );
    wxWizardPageEx* PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID );

    /// Cancel Event Infrastructure
    void _ProcessCancelEvent( wxWizardExEvent& event );

    /// Finish Button Environment
    bool GetAccountCreatedSuccessfully() const { return account_created_successfully ; }
    void SetAccountCreatedSuccessfully(bool value) { account_created_successfully = value ; }

    bool GetAttachedToProjectSuccessfully() const { return attached_to_project_successfully ; }
    void SetAttachedToProjectSuccessfully(bool value) { attached_to_project_successfully = value ; }

    wxString GetProjectName() const { return m_strProjectName ; }
    void SetProjectName(wxString value) { m_strProjectName = value ; }

    wxString GetProjectURL() const { return m_strProjectUrl ; }
    void SetProjectURL(wxString value) { m_strProjectUrl = value ; }

    wxString GetProjectUserName() const { return m_strProjectUserName ; }
    void SetProjectUserName(wxString value) { m_strProjectUserName = value ; }

    wxString GetProjectAuthenticator() const { return m_strProjectAuthenticator ; }
    void SetProjectAuthenticator(wxString value) { m_strProjectAuthenticator = value ; }

    wxString GetProjectInstitution() const { return m_strProjectInstitution ; }
    void SetProjectInstitution(wxString value) { m_strProjectInstitution = value ; }

    wxString GetProjectDescription() const { return m_strProjectDescription ; }
    void SetProjectDescription(wxString value) { m_strProjectDescription = value ; }

    bool IsProjectKnown() const { return m_bProjectKnown ; }
    void SetProjectKnown(bool value) { m_bProjectKnown = value ; }

    wxString GetAccountEmailAddress() const { return m_strAccountEmailAddress ; }
    void SetAccountEmailAddress(wxString value) { m_strAccountEmailAddress = value ; }

    wxString GetAccountUsername() const { return m_strAccountUsername ; }
    void SetAccountUsername(wxString value) { m_strAccountUsername = value ; }

    wxString GetAccountPassword() const { return m_strAccountPassword ; }
    void SetAccountPassword(wxString value) { m_strAccountPassword = value ; }

    wxString GetAccountConfirmPassword() const { return m_strAccountConfirmPassword ; }
    void SetAccountConfirmPassword(wxString value) { m_strAccountConfirmPassword = value ; }

    bool GetConsentedToTerms() const { return m_bConsentedToTerms ; }
    void SetConsentedToTerms(bool value) { m_bConsentedToTerms = value ; }

    wxString GetReturnURL() const { return m_strReturnURL ; }
    void SetReturnURL(wxString value) { m_strReturnURL = value ; }

    bool IsCredentialsCached() const { return m_bCredentialsCached ; }
    void SetCredentialsCached(bool value) { m_bCredentialsCached = value ; }

    bool IsCredentialsDetected() const { return m_bCredentialsDetected ; }
    void SetCredentialsDetected(bool value) { m_bCredentialsDetected = value ; }

    bool IsCloseWhenCompleted() const { return m_bCloseWhenCompleted ; }
    void SetCloseWhenCompleted(bool value) { m_bCloseWhenCompleted = value ; }

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWizardAttachProject member variables
    CProjectInfoPage* m_ProjectInfoPage;
    CProjectPropertiesPage* m_ProjectPropertiesPage;
    CProjectProcessingPage* m_ProjectProcessingPage;
    CProjectWelcomePage* m_ProjectWelcomePage;
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
    CErrUserDisagreesPage* m_ErrUserDisagreesPage;
////@end CWizardAttachProject member variables

    /// Wizard Detection
    bool                IsAttachToProjectWizard;
    bool                IsAccountManagerWizard;
    bool                IsAccountManagerUpdateWizard;
	bool				IsChangeWCGApps;
	/// For WCG, need to not launch project page when first opened
	bool				IsFirstPass;

    /// Global Wizard Status
    PROJECT_CONFIG      project_config;
    ACCOUNT_IN          account_in;
    ACCOUNT_OUT         account_out;
    bool                account_created_successfully;
    bool                attached_to_project_successfully;
    bool                m_bCloseWhenCompleted;
    bool                m_bCredentialsCached;
    bool                m_bCredentialsDetected;
    wxString            m_strProjectName;
    wxString            m_strProjectUrl;
    wxString            m_strProjectAuthenticator;
    wxString            m_strProjectInstitution;
    wxString            m_strProjectDescription;
    wxString            m_strProjectUserName;
    bool                m_bProjectKnown;
    wxString            m_strAccountEmailAddress;
    wxString            m_strAccountUsername;
    wxString            m_strAccountPassword;
    wxString            m_strAccountConfirmPassword;
    bool                m_bConsentedToTerms;
    wxString            m_strReturnURL;
};

#endif

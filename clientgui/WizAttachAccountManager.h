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

#ifndef _WIZATTACHACCOUNTMANAGER_H_
#define _WIZATTACHACCOUNTMANAGER_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "WizAttachAccountManager.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/wizard.h"
#include "hyperlink.h"
#include "wx/valgen.h"
#include "wx/valtext.h"
////@end includes
#include "ValidateURL.h"
#include "ValidateAccountKey.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class CAMWelcomePage;
class CAMAccountManagerInfoPage;
class CAMAccountManagerPropertiesPage;
class CAMAccountInfoPage;
class CAMAttachAccountManagerPage;
class CAMCompletionPage;
class CAMCompletionErrorPage;
class CAMErrAccountManagerNotDetectedPage;
class CAMErrAccountManagerUnavailablePage;
class CAMErrNoInternetConnectionPage;
class CAMErrProxyPage;
class CAMErrRefCountPage;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ATTACHACCOUNTMANAGERWIZARD 10032
#define SYMBOL_CWIZATTACHACCOUNTMANAGER_IDNAME ID_ATTACHACCOUNTMANAGERWIZARD
#define ID_WELCOMEPAGE 10033
#define ID_ERRPROJECTPROPERTIES 10047
#define ID_ERRPROJECTPROPERTIESURL 10058
#define ID_ERRPROJECTATTACHFAILURE 10080
#define ID_ERRGOOGLECOMM 10053
#define ID_ERRYAHOOCOMM 10055
#define ID_ERRNETDETECTION 10057
#define ID_ACCOUNTMANAGERINFOPAGE 10034
#define ID_ACCOUNTMANAGERURLSTATICCTRL 10046
#define ID_ACCOUNTMANAGERURLCTRL 10035
#define ID_PROJECRINFOBOINCLINK 10036
#define ID_ACCOUNTMANAGERPROPERTIESPAGE 10017
#define ID_PROJECTPROPERTIESPROGRESS 10077
#define ID_ACCOUNTINFOPAGE 10037
#define ID_ACCOUNTEMAILADDRESSSTATICCTRL 10045
#define ID_ACCOUNTEMAILADDRESSCTRL 10040
#define ID_ACCOUNTPASSWORDSTATICCTRL 10044
#define ID_ACCOUNTPASSWORDCTRL 10041
#define ID_ATTACHACCOUNTMANAGERPAGE 10038
#define ID_ATTACHPROJECTPROGRESS 10003
#define ID_COMPLETIONPAGE 10048
#define ID_COMPLETIONERRORPAGE 10011
#define ID_ERRACCOUNTMANAGERNOTDETECTEDPAGE 10007
#define ID_ERRACCOUNTMANAGERUNAVAILABLEPAGE 10049
#define ID_ERRNOINTERNETCONNECTIONPAGE 10050
#define ID_ERRPROXYPAGE 10063
#define ID_PROXYHTTPSERVERSTATICCTRL 10002
#define ID_PROXYHTTPSERVERCTRL 10000
#define ID_PROXYHTTPPORTSTATICCTRL 10004
#define ID_PROXYHTTPPORTCTRL 10001
#define ID_PROXYHTTPUSERNAMESTATICCTRL 10005
#define ID_PROXYHTTPUSERNAMECTRL 10006
#define ID_PROXYHTTPPASSWORDSTATICCTRL 10009
#define ID_PROXYHTTPPASSWORDCTRL 10010
#define ID_PROXYHTTPAUTODETECTCTRL 10081
#define ID_PROXYSOCKSSERVERSTATICCTRL 10012
#define ID_PROXYSOCKSSERVERCTRL 10013
#define ID_PROXYSOCKSPORTSTATICCTRL 10014
#define ID_PROXYSOCKSPORTCTRL 10015
#define ID_PROXYSOCKSUSERNAMESTATICCTRL 10016
#define ID_PROXYSOCKSUSERNAMECTRL 10018
#define ID_PROXYSOCKSPASSWORDSTATICCTRL 10019
#define ID_PROXYSOCKSPASSWORDCTRL 10021
#define ID_ERRREFCOUNTPAGE 10075
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * CWizAttachAccountManager debug flags
 */

#define WIZDEBUG_ERRPROJECTPROPERTIES                 0x00000001
#define WIZDEBUG_ERRPROJECTPROPERTIESURL              0x00000002
#define WIZDEBUG_ERRYAHOOCOMM                         0x00000004
#define WIZDEBUG_ERRGOOGLECOMM                        0x00000008
#define WIZDEBUG_ERRNETDETECTION                      0x00000010
#define WIZDEBUG_ERRPROJECTCOMM                       0x00000020
#define WIZDEBUG_ERRACCOUNTNOTFOUND                   0x00000040
#define WIZDEBUG_ERRACCOUNTALREADYEXISTS              0x00000080
#define WIZDEBUG_ERRACCOUNTCREATIONDISABLED           0x00000100
#define WIZDEBUG_ERRCLIENTACCOUNTCREATIONDISABLED     0x00000200
#define WIZDEBUG_ERRPROJECTATTACH                     0x00000400
#define WIZDEBUG_ERRPROJECTALREADYATTACHED            0x00000800


/*!
 * CWizAttachAccountManager class declaration
 */

class CWizAttachAccountManager: public wxWizard
{    
    DECLARE_DYNAMIC_CLASS( CWizAttachAccountManager )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWizAttachAccountManager( );
    CWizAttachAccountManager( wxWindow* parent, wxWindowID id = SYMBOL_CWIZATTACHACCOUNTMANAGER_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWIZATTACHACCOUNTMANAGER_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CWizAttachAccountManager event handler declarations

    /// wxEVT_WIZARD_FINISHED event handler for ID_ATTACHACCOUNTMANAGERWIZARD
    void OnFinished( wxWizardEvent& event );

////@end CWizAttachAccountManager event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_BACKWARD
    void OnWizardBack( wxCommandEvent& event );
    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_FORWARD
    void OnWizardNext( wxCommandEvent& event );

////@begin CWizAttachAccountManager member function declarations

    /// Runs the wizard.
    bool Run();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWizAttachAccountManager member function declarations

    /// Overrides
    virtual bool HasNextPage( wxWizardPage* page );
    virtual bool HasPrevPage( wxWizardPage* page );

    // Accessors
    wxButton* GetBackButton() const { return m_pbtnBack ; }
    void SetBackButton(wxButton* value) { m_pbtnBack = value ; }

    wxButton* GetNextButton() const { return m_pbtnNext ; }
    void SetNextButton(wxButton* value) { m_pbtnNext = value ; }

    /// Diagnostics functions
    void SetDiagFlags( unsigned long ulFlags );
    bool IsDiagFlagsSet( unsigned long ulFlags );

    /// Track page transitions
    wxWizardPage* PopPageTransition();
    wxWizardPage* PushPageTransition( wxWizardPage* pCurrentPage, unsigned long ulPageID );

    /// Cancel Event Infrastructure
    bool IsCancelInProgress() const { return m_bCancelInProgress ; }
    void ProcessCancelEvent( wxWizardEvent& event );

    /// Button Simulation
    void SimulateNextButton();
    void EnableNextButton();
    void DisableNextButton();
    void SimulateBackButton();
    void EnableBackButton();
    void DisableBackButton();

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

////@begin CWizAttachAccountManager member variables
    CAMWelcomePage* m_WelcomePage;
    CAMAccountManagerInfoPage* m_AccountManagerInfoPage;
    CAMAccountManagerPropertiesPage* m_AccountManagerPropertiesPage;
    CAMAccountInfoPage* m_AccountInfoPage;
    CAMAttachAccountManagerPage* m_AttachAccountManagerPage;
    CAMCompletionPage* m_CompletionPage;
    CAMCompletionErrorPage* m_CompletionErrorPage;
    CAMErrAccountManagerNotDetectedPage* m_ErrAccountManagerNotDetectedPage;
    CAMErrAccountManagerUnavailablePage* m_ErrAccountManagerUnavailablePage;
    CAMErrNoInternetConnectionPage* m_ErrNoInternetConnectionPage;
    CAMErrProxyPage* m_ErrProxyPage;
    CAMErrRefCountPage* m_ErrRefCountPage;
////@end CWizAttachAccountManager member variables

    // Since the buttons are not publically exposed, we are going to cheat to get
    //   the pointers to them by trapping the click event and caching the button
    //   class pointers.
    wxButton*    m_pbtnBack;     // the "<Back" button
    wxButton*    m_pbtnNext;     // the "Next>" or "Finish" button

    // Wizard support
    unsigned long m_ulDiagFlags;
    std::stack<wxWizardPage*> m_PageTransition;

    // Cancel Checking
    bool m_bCancelInProgress;

    // Global Wizard Status
    PROJECT_CONFIG      project_config;
    ACCOUNT_IN          account_in;
    ACCOUNT_OUT         account_out;
    bool                account_created_successfully;
    bool                attached_to_project_successfully;
    wxString            project_url;
    wxString            project_authenticator;
};

/*!
 * CAMWelcomePage class declaration
 */

class CAMWelcomePage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMWelcomePage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMWelcomePage( );

    CAMWelcomePage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMWelcomePage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WELCOMEPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE
    void OnPageChanging( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMWelcomePage event handler declarations

////@begin CAMWelcomePage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMWelcomePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMWelcomePage member variables
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectPropertiesCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectPropertiesURLCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectAttachFailureCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrGoogleCommCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrYahooCommCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrNetDetectionCtrl;
#endif
////@end CAMWelcomePage member variables
};

/*!
 * CAMAccountManagerInfoPage class declaration
 */

class CAMAccountManagerInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMAccountManagerInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMAccountManagerInfoPage( );

    CAMAccountManagerInfoPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMAccountManagerInfoPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanging( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMAccountManagerInfoPage event handler declarations

////@begin CAMAccountManagerInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    wxString GetProjectURL() const { return m_strProjectURL ; }
    void SetProjectURL(wxString value) { m_strProjectURL = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMAccountManagerInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMAccountManagerInfoPage member variables
    wxStaticText* m_AccountManagerUrlStaticCtrl;
    wxTextCtrl* m_AccountManagerUrlCtrl;
    wxString m_strProjectURL;
////@end CAMAccountManagerInfoPage member variables
};

/*!
 * CAMAccountManagerPropertiesPage custom events
 */

class CAMAccountManagerPropertiesPageEvent : public wxEvent
{
public:
    CAMAccountManagerPropertiesPageEvent(wxEventType evtType, wxWizardPage *parent)
        : wxEvent(-1, evtType)
        {
            SetEventObject(parent);
        }

    virtual wxEvent *Clone() const { return new CAMAccountManagerPropertiesPageEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE, 11000 )
END_DECLARE_EVENT_TYPES()

#define EVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_ACCOUNTMANAGERPROPERTIES_STATECHANGE, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

/*!
 * CAMAccountManagerPropertiesPage states
 */

#define ACCTMGRPROP_INIT                                   0
#define ACCTMGRPROP_RETRPROJECTPROPERTIES_BEGIN            1
#define ACCTMGRPROP_RETRPROJECTPROPERTIES_EXECUTE          2
#define ACCTMGRPROP_COMMUNICATEYAHOO_BEGIN                 3
#define ACCTMGRPROP_COMMUNICATEYAHOO_EXECUTE               4
#define ACCTMGRPROP_COMMUNICATEGOOGLE_BEGIN                5
#define ACCTMGRPROP_COMMUNICATEGOOGLE_EXECUTE              6
#define ACCTMGRPROP_DETERMINENETWORKSTATUS_BEGIN           7
#define ACCTMGRPROP_DETERMINENETWORKSTATUS_EXECUTE         8
#define ACCTMGRPROP_CLEANUP                                9
#define ACCTMGRPROP_END                                    10

/*!
 * CAMAccountManagerPropertiesPage class declaration
 */

class CAMAccountManagerPropertiesPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMAccountManagerPropertiesPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMAccountManagerPropertiesPage( );

    CAMAccountManagerPropertiesPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMAccountManagerPropertiesPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTMANAGERPROPERTIESPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTMANAGERPROPERTIESPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMAccountManagerPropertiesPage event handler declarations

    void OnStateChange( CAMAccountManagerPropertiesPageEvent& event );

////@begin CAMAccountManagerPropertiesPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMAccountManagerPropertiesPage member function declarations

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

////@begin CAMAccountManagerPropertiesPage member variables
    wxStaticBitmap* m_ProjectPropertiesProgress;
////@end CAMAccountManagerPropertiesPage member variables

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

/*!
 * CAMAccountInfoPage class declaration
 */

class CAMAccountInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMAccountInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMAccountInfoPage( );

    CAMAccountInfoPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMAccountInfoPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
    void OnPageChanging( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTINFOPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMAccountInfoPage event handler declarations

////@begin CAMAccountInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    wxString GetAccountEmailAddress() const { return m_strAccountEmailAddress ; }
    void SetAccountEmailAddress(wxString value) { m_strAccountEmailAddress = value ; }

    wxString GetAccountPassword() const { return m_strAccountPassword ; }
    void SetAccountPassword(wxString value) { m_strAccountPassword = value ; }

    wxString GetAccountConfirmPassword() const { return m_strAccountConfirmPassword ; }
    void SetAccountConfirmPassword(wxString value) { m_strAccountConfirmPassword = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMAccountInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMAccountInfoPage member variables
    wxStaticText* m_AccountEmailAddressStaticCtrl;
    wxTextCtrl* m_AccountEmailAddressCtrl;
    wxStaticText* m_AccountPasswordStaticCtrl;
    wxTextCtrl* m_AccountPasswordCtrl;
    wxString m_strAccountEmailAddress;
    wxString m_strAccountPassword;
    wxString m_strAccountConfirmPassword;
////@end CAMAccountInfoPage member variables
};

/*!
 * CAMAttachAccountManagerPage custom events
 */

class CAMAttachAccountManagerPageEvent : public wxEvent
{
public:
    CAMAttachAccountManagerPageEvent(wxEventType evtType, wxWizardPage *parent)
        : wxEvent(-1, evtType)
        {
            SetEventObject(parent);
        }

    virtual wxEvent *Clone() const { return new CAMAttachAccountManagerPageEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_ATTACHACCOUNTMANAGER_STATECHANGE, 11100 )
END_DECLARE_EVENT_TYPES()

#define EVT_ATTACHACCOUNTMANAGER_STATECHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_ATTACHACCOUNTMANAGER_STATECHANGE, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

/*!
 * CAMAttachAccountManagerPage states
 */

#define ATTACHACCTMGR_INIT                              0
#define ATTACHACCTMGR_ATTACHACCTMGR_BEGIN               1
#define ATTACHACCTMGR_ATTACHACCTMGR_EXECUTE             2
#define ATTACHACCTMGR_CLEANUP                           3
#define ATTACHACCTMGR_END                               4

/*!
 * CAMAttachAccountManagerPage class declaration
 */

class CAMAttachAccountManagerPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMAttachAccountManagerPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMAttachAccountManagerPage( );

    CAMAttachAccountManagerPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMAttachAccountManagerPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHACCOUNTMANAGERPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ATTACHACCOUNTMANAGERPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMAttachAccountManagerPage event handler declarations

    void OnStateChange( CAMAttachAccountManagerPageEvent& event );

////@begin CAMAttachAccountManagerPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMAttachAccountManagerPage member function declarations

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

////@begin CAMAttachAccountManagerPage member variables
    wxStaticBitmap* m_AttachProjectProgress;
////@end CAMAttachAccountManagerPage member variables

    bool m_bProjectCommunitcationsSucceeded;
    bool m_bProjectUnavailable;
    bool m_bProjectAccountNotFound;
    bool m_bProjectAccountAlreadyExists;
    bool m_bProjectAttachSucceeded;
    int m_iBitmapIndex;
    int m_iCurrentState;
};

/*!
 * CAMCompletionPage class declaration
 */

class CAMCompletionPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMCompletionPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMCompletionPage( );

    CAMCompletionPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMCompletionPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE
    void OnCancel( wxWizardEvent& event );

    /// wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE
    void OnFinished( wxWizardEvent& event );

////@end CAMCompletionPage event handler declarations

////@begin CAMCompletionPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMCompletionPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMCompletionPage member variables
    wxStaticText* m_CompletionMessage;
////@end CAMCompletionPage member variables
};

/*!
 * CAMCompletionErrorPage class declaration
 */

class CAMCompletionErrorPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMCompletionErrorPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMCompletionErrorPage( );

    CAMCompletionErrorPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMCompletionErrorPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMCompletionErrorPage event handler declarations

////@begin CAMCompletionErrorPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMCompletionErrorPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMCompletionErrorPage member variables
////@end CAMCompletionErrorPage member variables
};

/*!
 * CAMErrAccountManagerNotDetectedPage class declaration
 */

class CAMErrAccountManagerNotDetectedPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMErrAccountManagerNotDetectedPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMErrAccountManagerNotDetectedPage( );

    CAMErrAccountManagerNotDetectedPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMErrAccountManagerNotDetectedPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRACCOUNTMANAGERNOTDETECTEDPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRACCOUNTMANAGERNOTDETECTEDPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMErrAccountManagerNotDetectedPage event handler declarations

////@begin CAMErrAccountManagerNotDetectedPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMErrAccountManagerNotDetectedPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMErrAccountManagerNotDetectedPage member variables
////@end CAMErrAccountManagerNotDetectedPage member variables
};

/*!
 * CAMErrAccountManagerUnavailablePage class declaration
 */

class CAMErrAccountManagerUnavailablePage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMErrAccountManagerUnavailablePage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMErrAccountManagerUnavailablePage( );

    CAMErrAccountManagerUnavailablePage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMErrAccountManagerUnavailablePage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRACCOUNTMANAGERUNAVAILABLEPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRACCOUNTMANAGERUNAVAILABLEPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMErrAccountManagerUnavailablePage event handler declarations

////@begin CAMErrAccountManagerUnavailablePage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMErrAccountManagerUnavailablePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMErrAccountManagerUnavailablePage member variables
////@end CAMErrAccountManagerUnavailablePage member variables
};

/*!
 * CAMErrNoInternetConnectionPage class declaration
 */

class CAMErrNoInternetConnectionPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMErrNoInternetConnectionPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMErrNoInternetConnectionPage( );

    CAMErrNoInternetConnectionPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMErrNoInternetConnectionPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNOINTERNETCONNECTIONPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRNOINTERNETCONNECTIONPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMErrNoInternetConnectionPage event handler declarations

////@begin CAMErrNoInternetConnectionPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMErrNoInternetConnectionPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMErrNoInternetConnectionPage member variables
////@end CAMErrNoInternetConnectionPage member variables
};

/*!
 * CAMErrProxyPage class declaration
 */

class CAMErrProxyPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMErrProxyPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMErrProxyPage( );

    CAMErrProxyPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMErrProxyPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ERRPROXYPAGE
    void OnPageChanging( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMErrProxyPage event handler declarations

////@begin CAMErrProxyPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    wxString GetProxyHTTPServer() const { return m_strProxyHTTPServer ; }
    void SetProxyHTTPServer(wxString value) { m_strProxyHTTPServer = value ; }

    wxString GetProxyHTTPPort() const { return m_strProxyHTTPPort ; }
    void SetProxyHTTPPort(wxString value) { m_strProxyHTTPPort = value ; }

    wxString GetProxyHTTPUsername() const { return m_strProxyHTTPUsername ; }
    void SetProxyHTTPUsername(wxString value) { m_strProxyHTTPUsername = value ; }

    wxString GetProxyHTTPPassword() const { return m_strProxyHTTPPassword ; }
    void SetProxyHTTPPassword(wxString value) { m_strProxyHTTPPassword = value ; }

    wxString GetProxySOCKSServer() const { return m_strProxySOCKSServer ; }
    void SetProxySOCKSServer(wxString value) { m_strProxySOCKSServer = value ; }

    wxString GetProxySOCKSPort() const { return m_strProxySOCKSPort ; }
    void SetProxySOCKSPort(wxString value) { m_strProxySOCKSPort = value ; }

    wxString GetProxySOCKSUsername() const { return m_strProxySOCKSUsername ; }
    void SetProxySOCKSUsername(wxString value) { m_strProxySOCKSUsername = value ; }

    wxString GetProxySOCKSPassword() const { return m_strProxySOCKSPassword ; }
    void SetProxySOCKSPassword(wxString value) { m_strProxySOCKSPassword = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMErrProxyPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMErrProxyPage member variables
    wxStaticText* m_ProxyHTTPServerStaticCtrl;
    wxTextCtrl* m_ProxyHTTPServerCtrl;
    wxStaticText* m_ProxyHTTPPortStaticCtrl;
    wxTextCtrl* m_ProxyHTTPPortCtrl;
    wxStaticText* m_ProxyHTTPUsernameStaticCtrl;
    wxTextCtrl* m_ProxyHTTPUsernameCtrl;
    wxStaticText* m_ProxyHTTPPasswordStaticCtrl;
    wxTextCtrl* m_ProxyHTTPPasswordCtrl;
    wxButton* m_ProxyHTTPAutodetectCtrl;
    wxStaticText* m_ProxySOCKSServerStaticCtrl;
    wxTextCtrl* m_ProxySOCKSServerCtrl;
    wxStaticText* m_ProxySOCKSPortStaticCtrl;
    wxTextCtrl* m_ProxySOCKSPortCtrl;
    wxStaticText* m_ProxySOCKSUsernameStaticCtrl;
    wxTextCtrl* m_ProxySOCKSUsernameCtrl;
    wxStaticText* m_ProxySOCKSPasswordStaticCtrl;
    wxTextCtrl* m_ProxySOCKSPasswordCtrl;
    wxString m_strProxyHTTPServer;
    wxString m_strProxyHTTPPort;
    wxString m_strProxyHTTPUsername;
    wxString m_strProxyHTTPPassword;
    wxString m_strProxySOCKSServer;
    wxString m_strProxySOCKSPort;
    wxString m_strProxySOCKSUsername;
    wxString m_strProxySOCKSPassword;
////@end CAMErrProxyPage member variables
};

/*!
 * CAMErrRefCountPage class declaration
 */

class CAMErrRefCountPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMErrRefCountPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMErrRefCountPage( );

    CAMErrRefCountPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMErrRefCountPage event handler declarations

////@end CAMErrRefCountPage event handler declarations

////@begin CAMErrRefCountPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMErrRefCountPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMErrRefCountPage member variables
////@end CAMErrRefCountPage member variables
};

#endif
    // _WIZATTACHACCOUNTMANAGER_H_

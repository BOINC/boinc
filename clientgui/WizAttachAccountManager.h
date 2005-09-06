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
class CAMErrProxyInfoPage;
class CAMErrProxyHTTPPage;
class CAMErrProxySOCKSPage;
class CAMErrProxyComplationPage;
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
#define ID_ERRPROXYINFOPAGE 10060
#define ID_ERRPROXYHTTPPAGE 10061
#define ID_HTTPAUTODETECT 10064
#define ID_PROXYHTTPSERVERSTATICCTRL 10065
#define ID_PROXYHTTPSERVERCTRL 10000
#define ID_PROXYHTTPPORTSTATICCTRL 10070
#define ID_PROXYHTTPPORTCTRL 10001
#define ID_PROXYHTTPUSERNAMESTATICCTRL 10068
#define ID_PROXYHTTPUSERNAMECTRL 10066
#define ID_PROXYHTTPPASSWORDSTATICCTRL 10069
#define ID_PROXYHTTPPASSWORDCTRL 10067
#define ID_ERRPROXYSOCKSPAGE 10062
#define ID_PROXYSOCKSSERVERSTATICCTRL 10071
#define ID_PROXYSOCKSSERVERCTRL 10002
#define ID_PROXYSOCKSPORTSTATICCTRL 10070
#define ID_PROXYSOCKSPORTCTRL 10001
#define ID_PROXYSOCKSUSERNAMESTATICCTRL 10072
#define ID_PROXYSOCKSUSERNAMECTRL 10004
#define ID_PROXYSOCKSPASSWORDSTATICCTRL 10069
#define ID_PROXYSOCKSPASSWORDCTRL 10005
#define ID_ERRPROXYCOMPLETIONPAGE 10063
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

////@begin CWizAttachAccountManager member function declarations

    /// Runs the wizard.
    bool Run();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWizAttachAccountManager member function declarations

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
    CAMErrProxyInfoPage* m_ErrProxyInfoPage;
    CAMErrProxyHTTPPage* m_ErrProxyHTTPPage;
    CAMErrProxySOCKSPage* m_ErrProxySOCKSPage;
    CAMErrProxyComplationPage* m_ErrProxyCompletionPage;
    CAMErrRefCountPage* m_ErrRefCountPage;
////@end CWizAttachAccountManager member variables
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

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMAccountManagerPropertiesPage member variables
    wxStaticBitmap* m_ProjectPropertiesProgress;
////@end CAMAccountManagerPropertiesPage member variables
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

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMAttachAccountManagerPage member variables
    wxStaticBitmap* m_AttachProjectProgress;
////@end CAMAttachAccountManagerPage member variables
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
 * CAMErrProxyInfoPage class declaration
 */

class CAMErrProxyInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMErrProxyInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMErrProxyInfoPage( );

    CAMErrProxyInfoPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMErrProxyInfoPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYINFOPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYINFOPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMErrProxyInfoPage event handler declarations

////@begin CAMErrProxyInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMErrProxyInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMErrProxyInfoPage member variables
////@end CAMErrProxyInfoPage member variables
};

/*!
 * CAMErrProxyHTTPPage class declaration
 */

class CAMErrProxyHTTPPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMErrProxyHTTPPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMErrProxyHTTPPage( );

    CAMErrProxyHTTPPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMErrProxyHTTPPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYHTTPPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYHTTPPAGE
    void OnCancel( wxWizardEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_HTTPAUTODETECT
    void OnAutodetectClick( wxCommandEvent& event );

////@end CAMErrProxyHTTPPage event handler declarations

////@begin CAMErrProxyHTTPPage member function declarations

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

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMErrProxyHTTPPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMErrProxyHTTPPage member variables
    wxStaticText* m_ProxyHTTPServerStaticCtrl;
    wxTextCtrl* m_ProxyHTTPServerCtrl;
    wxStaticText* m_ProxyHTTPPortStaticCtrl;
    wxTextCtrl* m_ProxyHTTPPortCtrl;
    wxStaticText* m_ProxyHTTPUsernameStaticCtrl;
    wxTextCtrl* m_ProxyHTTPUsernameCtrl;
    wxStaticText* m_ProxyHTTPPasswordStaticCtrl;
    wxTextCtrl* m_ProxyHTTPPasswordCtrl;
    wxString m_strProxyHTTPServer;
    wxString m_strProxyHTTPPort;
    wxString m_strProxyHTTPUsername;
    wxString m_strProxyHTTPPassword;
////@end CAMErrProxyHTTPPage member variables
};

/*!
 * CAMErrProxySOCKSPage class declaration
 */

class CAMErrProxySOCKSPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMErrProxySOCKSPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMErrProxySOCKSPage( );

    CAMErrProxySOCKSPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMErrProxySOCKSPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYSOCKSPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYSOCKSPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMErrProxySOCKSPage event handler declarations

////@begin CAMErrProxySOCKSPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

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
////@end CAMErrProxySOCKSPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMErrProxySOCKSPage member variables
    wxStaticText* m_ProxySOCKSServerStaticCtrl;
    wxTextCtrl* m_ProxySOCKSServerCtrl;
    wxStaticText* m_ProxySOCKSPortStaticCtrl;
    wxTextCtrl* m_ProxySOCKSPortCtrl;
    wxStaticText* m_ProxySOCKSUsernameStaticCtrl;
    wxTextCtrl* m_ProxySOCKSUsernameCtrl;
    wxStaticText* m_ProxySOCKSPasswordStaticCtrl;
    wxTextCtrl* m_ProxySOCKSPasswordCtrl;
    wxString m_strProxySOCKSServer;
    wxString m_strProxySOCKSPort;
    wxString m_strProxySOCKSUsername;
    wxString m_strProxySOCKSPassword;
////@end CAMErrProxySOCKSPage member variables
};

/*!
 * CAMErrProxyComplationPage class declaration
 */

class CAMErrProxyComplationPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAMErrProxyComplationPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAMErrProxyComplationPage( );

    CAMErrProxyComplationPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAMErrProxyComplationPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYCOMPLETIONPAGE
    void OnPageChanged( wxWizardEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYCOMPLETIONPAGE
    void OnCancel( wxWizardEvent& event );

////@end CAMErrProxyComplationPage event handler declarations

////@begin CAMErrProxyComplationPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAMErrProxyComplationPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAMErrProxyComplationPage member variables
////@end CAMErrProxyComplationPage member variables
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

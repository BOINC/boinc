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
#ifndef _WIZATTACHPROJECT_H_
#define _WIZATTACHPROJECT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "WizAttachProject.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/wizard.h"
#include "wx/valtext.h"
#include "hyperlink.h"
#include "wx/valgen.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CWelcomePage;
class CProjectInfoPage;
class CProjectPropertiesPage;
class CAccountKeyPage;
class CAccountInfoPage;
class CAccountCreationPage;
class CCompletionPage;
class CErrProjectNotDetectedPage;
class CErrProjectUnavailablePage;
class CErrNoInternetConnectionPage;
class CErrAccountAlreadyExistsPage;
class CErrAccountCreationDisabledPage;
class CErrProxyInfoPage;
class CErrProxyHTTPPage;
class CErrProxySOCKSPage;
class CErrProxyComplationPage;
class CErrRefCountPage;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ATTACHPROJECTWIZARD 10032
#define SYMBOL_CWIZATTACHPROJECT_IDNAME ID_ATTACHPROJECTWIZARD
#define ID_WELCOMEPAGE 10033
#define ID_ERRPROJECTPROPERTIES 10047
#define ID_ERRPROJECTCOMM 10052
#define ID_ERRPROJECTPROPERTIESURL 10058
#define ID_ERRACCOUNTCREATIONDISABLED 10059
#define ID_ERRACCOUNTALREADYEXISTS 10020
#define ID_ERRGOOGLECOMM 10053
#define ID_ERRYAHOOCOMM 10055
#define ID_ERRNETDETECTION 10057
#define ID_PROJECTINFOPAGE 10034
#define ID_PROJECTURLSTATICCTRL 10046
#define ID_PROJECTURLCTRL 10035
#define ID_PROJECRINFOBOINCLINK 10036
#define ID_PROJECTPROPERTIESPAGE 10017
#define ID_RETRPROJECTPROPERTIESIMAGECTRL 10018
#define ID_RETRPROJECTPROPERTIESCTRL 10019
#define ID_FINALPROJECTPROPERTIESTATUSCTRL 10026
#define ID_ACCOUNTKEYPAGE 10054
#define ID_ACCOUNTKEYSTATICCTRL 10074
#define ID_ACCOUNTKEYCTRL 10073
#define ID_ACCOUNTINFOPAGE 10037
#define ID_ACCOUNTCREATECTRL 10038
#define ID_ACCOUNTUSEEXISTINGCTRL 10039
#define ID_ACCOUNTEMAILADDRESSSTATICCTRL 10045
#define ID_ACCOUNTEMAILADDRESSCTRL 10040
#define ID_ACCOUNTPASSWORDSTATICCTRL 10044
#define ID_ACCOUNTPASSWORDCTRL 10041
#define ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL 10043
#define ID_ACCOUNTCONFIRMPASSWORDCTRL 10042
#define ID_ACCOUNTCREATIONPAGE 10038
#define ID_PROJECTCOMMUNICATIONSIMAGECTRL 10008
#define ID_PROJECTCOMMUNICATIONSCTRL 10009
#define ID_FINALACCOUNTCREATIONSTATUSCTRL 10016
#define ID_COMPLETIONPAGE 10048
#define ID_ERRPROJECTNOTDETECTEDPAGE 10007
#define ID_HYPERLINK 10010
#define ID_ERRPROJECTUNAVAILABLEPAGE 10049
#define ID_PROJECTUNAVAILABLEBOINCLINK 10003
#define ID_ERRNOINTERNETCONNECTIONPAGE 10050
#define ID_ERRACCOUNTALREADYEXISTSPAGE 10051
#define ID_ERRACCOUNTCREATIONDISABLEDPAGE 10056
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
#define ID_SOCKSAUTODETECT 10006
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
 * CWizAttachProject debug flags
 */

#define WIZDEBUG_ERRPROJECTPROPERTIES           0x00000001
#define WIZDEBUG_ERRPROJECTPROPERTIESURL        0x00000002
#define WIZDEBUG_ERRYAHOOCOMM                   0x00000004
#define WIZDEBUG_ERRGOOGLECOMM                  0x00000008
#define WIZDEBUG_ERRNETDETECTION                0x00000010
#define WIZDEBUG_ERRPROJECTCOMM                 0x00000020
#define WIZDEBUG_ERRACCOUNTALREADYEXISTS        0x00000040
#define WIZDEBUG_ERRACCOUNTCREATIONDISABLED     0x00000080


/*!
 * CWizAttachProject class declaration
 */

class CWizAttachProject: public wxWizard
{    
    DECLARE_DYNAMIC_CLASS( CWizAttachProject )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWizAttachProject( );
    CWizAttachProject( wxWindow* parent, wxWindowID id = SYMBOL_CWIZATTACHPROJECT_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWIZATTACHPROJECT_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CWizAttachProject event handler declarations

    /// wxEVT_WIZARD_CANCEL event handler for ID_ATTACHPROJECTWIZARD
    void OnWizardCancel( wxWizardEvent& event );

////@end CWizAttachProject event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_BACKWARD
    void OnWizardBack( wxCommandEvent& event );
    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_FORWARD
    void OnWizardNext( wxCommandEvent& event );

////@begin CWizAttachProject member function declarations

    /// Runs the wizard.
    bool Run();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWizAttachProject member function declarations

    /// Overrides
    virtual bool HasNextPage( wxWizardPage* page );
    virtual bool HasPrevPage( wxWizardPage* page );

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

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWizAttachProject member variables
    CWelcomePage* m_WelcomePage;
    CProjectInfoPage* m_ProjectInfoPage;
    CProjectPropertiesPage* m_ProjectPropertiesPage;
    CAccountKeyPage* m_AccountKeyPage;
    CAccountInfoPage* m_AccountInfoPage;
    CAccountCreationPage* m_AccountCreationPage;
    CCompletionPage* m_CompletionPage;
    CErrProjectNotDetectedPage* m_ErrProjectNotDetectedPage;
    CErrProjectUnavailablePage* m_ErrProjectUnavailablePage;
    CErrNoInternetConnectionPage* m_ErrNoInternetConnectionPage;
    CErrAccountAlreadyExistsPage* m_ErrAccountAlreadyExistsPage;
    CErrAccountCreationDisabledPage* m_ErrAccountCreationDisabledPage;
    CErrProxyInfoPage* m_ErrProxyInfoPage;
    CErrProxyHTTPPage* m_ErrProxyHTTPPage;
    CErrProxySOCKSPage* m_ErrProxySOCKSPage;
    CErrProxyComplationPage* m_ErrProxyCompletionPage;
    CErrRefCountPage* m_ErrRefCountPage;
////@end CWizAttachProject member variables

    // Since the buttons are not publically exposed, we are going to cheat to get
    //   the pointers to them by trapping the click event and caching the button
    //   class pointers.
    wxButton*    m_pbtnBack;     // the "<Back" button
    wxButton*    m_pbtnNext;     // the "Next>" or "Finish" button

    // Wizard support
    unsigned long m_ulDiagFlags;
    std::stack<wxWizardPage*> m_PageTransition;

    // Global wizard status
    PROJECT_CONFIG      project_config;
    GR_PROXY_INFO       proxy_info;
    ACCOUNT_IN          account_in;
    ACCOUNT_OUT         account_out;
};

/*!
 * CWelcomePage class declaration
 */

class CWelcomePage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CWelcomePage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWelcomePage( );

    CWelcomePage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CWelcomePage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE
    void OnPageChanging( wxWizardEvent& event );

////@end CWelcomePage event handler declarations

////@begin CWelcomePage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWelcomePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWelcomePage member variables
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectPropertiesCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectCommCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectPropertiesURLCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrAccountCreationDisabledCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrAccountAlreadyExistsCtrl;
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
////@end CWelcomePage member variables
};

/*!
 * CProjectInfoPage class declaration
 */

class CProjectInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CProjectInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectInfoPage( );

    CProjectInfoPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectInfoPage event handler declarations

////@end CProjectInfoPage event handler declarations

////@begin CProjectInfoPage member function declarations

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
////@end CProjectInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CProjectInfoPage member variables
    wxStaticText* m_ProjectUrlStaticCtrl;
    wxTextCtrl* m_ProjectUrlCtrl;
    wxString m_strProjectURL;
////@end CProjectInfoPage member variables
};

/*!
 * CProjectPropertiesPage custom events
 */

class CProjectPropertiesPageEvent : public wxEvent
{
public:
    CProjectPropertiesPageEvent(wxEventType evtType, wxWizardPage *parent)
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

class CProjectPropertiesPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CProjectPropertiesPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectPropertiesPage( );

    CProjectPropertiesPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectPropertiesPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTPROPERTIESPAGE
    void OnPageChanged( wxWizardEvent& event );

////@end CProjectPropertiesPage event handler declarations

    void OnStateChange( CProjectPropertiesPageEvent& event );

////@begin CProjectPropertiesPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

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

////@begin CProjectPropertiesPage member variables
    wxStaticBitmap* m_RetrProjectPropertiesImageCtrl;
    wxStaticText* m_RetrProjectPropertiesCtrl;
    wxStaticText* m_FinalProjectPropertiesStatusCtrl;
////@end CProjectPropertiesPage member variables

    bool m_bProjectPropertiesSucceeded;
    bool m_bProjectPropertiesURLFailure;
    bool m_bProjectAccountCreationDisabled;
    bool m_bCommunicateYahooSucceeded;
    bool m_bCommunicateGoogleSucceeded;
    bool m_bDeterminingConnectionStatusSucceeded;
    wxInt32 m_iCurrentState;
};

/*!
 * CAccountKeyPage class declaration
 */

class CAccountKeyPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAccountKeyPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountKeyPage( );

    CAccountKeyPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountKeyPage event handler declarations

////@end CAccountKeyPage event handler declarations

////@begin CAccountKeyPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    wxString GetAccountKey() const { return m_strAccountKey ; }
    void SetAccountKey(wxString value) { m_strAccountKey = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAccountKeyPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAccountKeyPage member variables
    wxStaticText* m_AccountKeyStaticCtrl;
    wxTextCtrl* m_AccountKeyCtrl;
    wxString m_strAccountKey;
////@end CAccountKeyPage member variables
};

/*!
 * CAccountInfoPage class declaration
 */

class CAccountInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAccountInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountInfoPage( );

    CAccountInfoPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountInfoPage event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTCREATECTRL
    void OnAccountCreateCtrlSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTUSEEXISTINGCTRL
    void OnAccountUseExistingCtrlSelected( wxCommandEvent& event );

////@end CAccountInfoPage event handler declarations

////@begin CAccountInfoPage member function declarations

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
////@end CAccountInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAccountInfoPage member variables
    wxRadioButton* m_AccountCreateCtrl;
    wxRadioButton* m_AccountUseExistingCtrl;
    wxStaticText* m_AccountEmailAddressStaticCtrl;
    wxTextCtrl* m_AccountEmailAddressCtrl;
    wxStaticText* m_AccountPasswordStaticCtrl;
    wxTextCtrl* m_AccountPasswordCtrl;
    wxStaticText* m_AccountConfirmPasswordStaticCtrl;
    wxTextCtrl* m_AccountConfirmPasswordCtrl;
    wxString m_strAccountEmailAddress;
    wxString m_strAccountPassword;
    wxString m_strAccountConfirmPassword;
////@end CAccountInfoPage member variables
};


/*!
 * CAccountCreationPage custom events
 */

class CAccountCreationPageEvent : public wxEvent
{
public:
    CAccountCreationPageEvent(wxEventType evtType, wxWizardPage *parent)
        : wxEvent(-1, evtType)
        {
            SetEventObject(parent);
        }

    virtual wxEvent *Clone() const { return new CAccountCreationPageEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_ACCOUNTCREATION_STATECHANGE, 11100 )
END_DECLARE_EVENT_TYPES()

#define EVT_ACCOUNTCREATION_STATECHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_ACCOUNTCREATION_STATECHANGE, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

/*!
 * CAccountCreationPage states
 */

#define ACCOUNTCREATION_INIT                            0
#define ACCOUNTCREATION_PROJECTCOMM_BEGIN               1
#define ACCOUNTCREATION_PROJECTCOMM_EXECUTE             2
#define ACCOUNTCREATION_CLEANUP                         3
#define ACCOUNTCREATION_END                             4

/*!
 * CAccountCreationPage class declaration
 */

class CAccountCreationPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CAccountCreationPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountCreationPage( );

    CAccountCreationPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountCreationPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTCREATIONPAGE
    void OnPageChanged( wxWizardEvent& event );

////@end CAccountCreationPage event handler declarations

    void OnStateChange( CAccountCreationPageEvent& event );

////@begin CAccountCreationPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAccountCreationPage member function declarations

    bool GetProjectCommunitcationsSucceeded() const { return m_bProjectCommunitcationsSucceeded ; }
    void SetProjectCommunitcationsSucceeded(bool value) { m_bProjectCommunitcationsSucceeded = value ; }

    bool GetProjectUnavailable() const { return m_bProjectUnavailable ; }
    void SetProjectUnavailable(bool value) { m_bProjectUnavailable = value ; }

    bool GetProjectAccountAlreadyExists() const { return m_bProjectAccountAlreadyExists ; }
    void SetProjectAccountAlreadyExists(bool value) { m_bProjectAccountAlreadyExists = value ; }

    wxInt32 GetCurrentState() const { return m_iCurrentState ; }
    void SetNextState(wxInt32 value) { m_iCurrentState = value ; }

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAccountCreationPage member variables
    wxStaticBitmap* m_ProjectCommunitcationsImageCtrl;
    wxStaticText* m_ProjectCommunitcationsCtrl;
    wxStaticText* m_FinalAccountCreationStatusCtrl;
////@end CAccountCreationPage member variables

    bool m_bProjectCommunitcationsSucceeded;
    bool m_bProjectUnavailable;
    bool m_bProjectAccountAlreadyExists;
    wxInt32 m_iCurrentState;
};


/*!
 * CCompletionPage class declaration
 */

class CCompletionPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CCompletionPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCompletionPage( );

    CCompletionPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCompletionPage event handler declarations

////@end CCompletionPage event handler declarations

////@begin CCompletionPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCompletionPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCompletionPage member variables
////@end CCompletionPage member variables
};

/*!
 * CErrProjectNotDetectedPage class declaration
 */

class CErrProjectNotDetectedPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrProjectNotDetectedPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrProjectNotDetectedPage( );

    CErrProjectNotDetectedPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrProjectNotDetectedPage event handler declarations

////@end CErrProjectNotDetectedPage event handler declarations

////@begin CErrProjectNotDetectedPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrProjectNotDetectedPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrProjectNotDetectedPage member variables
////@end CErrProjectNotDetectedPage member variables
};

/*!
 * CErrProjectUnavailablePage class declaration
 */

class CErrProjectUnavailablePage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrProjectUnavailablePage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrProjectUnavailablePage( );

    CErrProjectUnavailablePage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrProjectUnavailablePage event handler declarations

////@end CErrProjectUnavailablePage event handler declarations

////@begin CErrProjectUnavailablePage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrProjectUnavailablePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrProjectUnavailablePage member variables
////@end CErrProjectUnavailablePage member variables
};

/*!
 * CErrNoInternetConnectionPage class declaration
 */

class CErrNoInternetConnectionPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrNoInternetConnectionPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrNoInternetConnectionPage( );

    CErrNoInternetConnectionPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrNoInternetConnectionPage event handler declarations

////@end CErrNoInternetConnectionPage event handler declarations

////@begin CErrNoInternetConnectionPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrNoInternetConnectionPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrNoInternetConnectionPage member variables
////@end CErrNoInternetConnectionPage member variables
};

/*!
 * CErrAccountAlreadyExistsPage class declaration
 */

class CErrAccountAlreadyExistsPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrAccountAlreadyExistsPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrAccountAlreadyExistsPage( );

    CErrAccountAlreadyExistsPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrAccountAlreadyExistsPage event handler declarations

////@end CErrAccountAlreadyExistsPage event handler declarations

////@begin CErrAccountAlreadyExistsPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrAccountAlreadyExistsPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrAccountAlreadyExistsPage member variables
////@end CErrAccountAlreadyExistsPage member variables
};

/*!
 * CErrAccountCreationDisabledPage class declaration
 */

class CErrAccountCreationDisabledPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrAccountCreationDisabledPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrAccountCreationDisabledPage( );

    CErrAccountCreationDisabledPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrAccountCreationDisabledPage event handler declarations

////@end CErrAccountCreationDisabledPage event handler declarations

////@begin CErrAccountCreationDisabledPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrAccountCreationDisabledPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrAccountCreationDisabledPage member variables
////@end CErrAccountCreationDisabledPage member variables
};

/*!
 * CErrProxyInfoPage class declaration
 */

class CErrProxyInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrProxyInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrProxyInfoPage( );

    CErrProxyInfoPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrProxyInfoPage event handler declarations

////@end CErrProxyInfoPage event handler declarations

////@begin CErrProxyInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrProxyInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrProxyInfoPage member variables
////@end CErrProxyInfoPage member variables
};

/*!
 * CErrProxyHTTPPage class declaration
 */

class CErrProxyHTTPPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrProxyHTTPPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrProxyHTTPPage( );

    CErrProxyHTTPPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrProxyHTTPPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_HTTPAUTODETECT
    void OnAutodetectClick( wxCommandEvent& event );

////@end CErrProxyHTTPPage event handler declarations

////@begin CErrProxyHTTPPage member function declarations

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
////@end CErrProxyHTTPPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrProxyHTTPPage member variables
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
////@end CErrProxyHTTPPage member variables
};

/*!
 * CErrProxySOCKSPage class declaration
 */

class CErrProxySOCKSPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrProxySOCKSPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrProxySOCKSPage( );

    CErrProxySOCKSPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrProxySOCKSPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SOCKSAUTODETECT
    void OnAutodetectClick( wxCommandEvent& event );

////@end CErrProxySOCKSPage event handler declarations

////@begin CErrProxySOCKSPage member function declarations

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
////@end CErrProxySOCKSPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrProxySOCKSPage member variables
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
////@end CErrProxySOCKSPage member variables
};

/*!
 * CErrProxyComplationPage class declaration
 */

class CErrProxyComplationPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrProxyComplationPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrProxyComplationPage( );

    CErrProxyComplationPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrProxyComplationPage event handler declarations

////@end CErrProxyComplationPage event handler declarations

////@begin CErrProxyComplationPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrProxyComplationPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrProxyComplationPage member variables
////@end CErrProxyComplationPage member variables
};

/*!
 * CErrRefCountPage class declaration
 */

class CErrRefCountPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( CErrRefCountPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrRefCountPage( );

    CErrRefCountPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrRefCountPage event handler declarations

////@end CErrRefCountPage event handler declarations

////@begin CErrRefCountPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrRefCountPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrRefCountPage member variables
////@end CErrRefCountPage member variables
};

#endif
    // _WIZATTACHPROJECT_H_

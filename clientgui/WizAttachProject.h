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
class CAccountInfoPage;
class CAccountCreationPage;
class CCompletionPage;
class CErrProjectUnavailablePage;
class CErrNoInternetConnectionPage;
class CErrAccountAlreadyExistsPage;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ATTACHPROJECTWIZARD 10032
#define SYMBOL_CWIZATTACHPROJECT_IDNAME ID_ATTACHPROJECTWIZARD
#define ID_WELCOMEPAGE 10033
#define ID_PROJECTINFOPAGE 10034
#define ID_PROJECTURLSTATICCTRL 10046
#define ID_PROJECTURLCTRL 10035
#define ID_PROJECRINFOBOINCLINK 10036
#define ID_ACCOUNTINFOPAGE 10037
#define ID_ACCOUNTCREATECTRL 10038
#define ID_ACCOUNTUSEEXISTINGCTRL 10039
#define ID_ACCOUNTEMAILADDRESSSTATICCTRL 10045
#define ID_ACCOUNTEMAILADDRESSCTRL 10040
#define ID_ACCOUNTPASSWORDSTATICCTRL 10044
#define ID_ACCOUNTPASSWORDCTRL 10041
#define ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL 10043
#define ID_ACCOUNTCONFIRMPASSWORDCTRL 10042
#define ID_ACCOUNTCREATIONPAGE 10047
#define ID_COMMBOINCPROJECTIMAGECTRL 10052
#define ID_COMMBOINCPROJECTCTRL 10053
#define ID_COMMYAHOOIMAGECTRL 10054
#define ID_COMMYAHOOCTRL 10053
#define ID_COMMGOOGLEIMAGECTRL 10055
#define ID_COMMGOOGLECTRL 10056
#define ID_DETERMINECONNECTIONSTATUSIMAGECTRL 10057
#define ID_DETERMINECONNECTIONSTATUSCTRL 10058
#define ID_FINALACCOUNTCREATIONSTATUSCTRL 10059
#define ID_COMPLETIONPAGE 10048
#define ID_ERRPROJECTUNAVAILABLEPAGE 10049
#define ID_ERRNOINTERNETCONNECTIONPAGE 10050
#define ID_ERRACCOUNTALREADYEXISTSPAGE 10051
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

////@begin CWizAttachProject member function declarations

    /// Runs the wizard.
    bool Run();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWizAttachProject member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWizAttachProject member variables
    CWelcomePage* m_WelcomePage;
    CProjectInfoPage* m_ProjectInfoPage;
    CAccountInfoPage* m_AccountInfoPage;
    CAccountCreationPage* m_AccountCreationPage;
    CCompletionPage* m_CompletionPage;
    CErrProjectUnavailablePage* m_ErrProjectUnavailablePage;
    CErrNoInternetConnectionPage* m_ErrNoInternetConnectionPage;
    CErrAccountAlreadyExistsPage* m_ErrAccountAlreadyExistsPage;
////@end CWizAttachProject member variables
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

    wxString GetStrProjectURL() const { return m_strProjectURL ; }
    void SetStrProjectURL(wxString value) { m_strProjectURL = value ; }

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

    wxString GetAccountCreateNew() const { return m_bAccountCreateNew ; }
    void SetAccountCreateNew(wxString value) { m_bAccountCreateNew = value ; }

    wxString GetAccountUseExisting() const { return m_bAccountUseExisting ; }
    void SetAccountUseExisting(wxString value) { m_bAccountUseExisting = value ; }

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
    wxString m_bAccountCreateNew;
    wxString m_bAccountUseExisting;
////@end CAccountInfoPage member variables
};


/*!
 * CAccountCreatePage custom events
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
DECLARE_EVENT_TYPE( wxEVT_ACCOUNTCREATION_STATECHANGE, 10000 )
END_DECLARE_EVENT_TYPES()

#define EVT_ACCOUNTCREATION_STATECHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_ACCOUNTCREATION_STATECHANGE, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

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

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAccountCreationPage member variables
    wxStaticBitmap* m_CommBOINCProjectImageCtrl;
    wxStaticText* m_CommBOINCProjectCtrl;
    wxStaticBitmap* m_CommYahooImageCtrl;
    wxStaticText* m_CommYahooCtrl;
    wxStaticBitmap* m_CommGoogleImageCtrl;
    wxStaticText* m_CommGoogleCtrl;
    wxStaticBitmap* m_DetermineConnectionStatusImageCtrl;
    wxStaticText* m_DetermineConnectionStatusCtrl;
    wxStaticText* m_FinalAccountCreationStatusCtrl;
////@end CAccountCreationPage member variables
    wxInt32       m_iCurrentState;
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

#endif
    // _WIZATTACHPROJECT_H_

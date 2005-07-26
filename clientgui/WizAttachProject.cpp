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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "WizAttachProject.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"

////@begin includes
////@end includes

#include "WizAttachProject.h"

////@begin XPM images
#include "res/attachprojectwizard.xpm"
#include "res/wizquestion.xpm"
#include "res/wizsuccess.xpm"
#include "res/wizfailure.xpm"
////@end XPM images

/*!
 * CWizAttachProject type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CWizAttachProject, wxWizard )

/*!
 * CWizAttachProject event table definition
 */

BEGIN_EVENT_TABLE( CWizAttachProject, wxWizard )

////@begin CWizAttachProject event table entries
    EVT_WIZARD_CANCEL( ID_ATTACHPROJECTWIZARD, CWizAttachProject::OnWizardCancel )

////@end CWizAttachProject event table entries

END_EVENT_TABLE()

/*!
 * CWizAttachProject constructors
 */

CWizAttachProject::CWizAttachProject( )
{
}

CWizAttachProject::CWizAttachProject( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    Create(parent, id, pos);
}

/*!
 * CWizAttachProject creator
 */

bool CWizAttachProject::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
////@begin CWizAttachProject member initialisation
    m_WelcomePage = NULL;
    m_ProjectInfoPage = NULL;
    m_AccountInfoPage = NULL;
    m_AccountCreationPage = NULL;
    m_CompletionPage = NULL;
    m_ErrProjectUnavailablePage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrAccountAlreadyExistsPage = NULL;
////@end CWizAttachProject member initialisation

////@begin CWizAttachProject creation
    wxBitmap wizardBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    wxWizard::Create( parent, id, _("Attach to Project Wizard"), wizardBitmap, pos );

    CreateControls();
////@end CWizAttachProject creation
    return TRUE;
}

/*!
 * Control creation for CWizAttachProject
 */

void CWizAttachProject::CreateControls()
{    
////@begin CWizAttachProject content construction
    wxWizard* itemWizard1 = this;

    m_WelcomePage = new CWelcomePage;
    m_WelcomePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_WelcomePage);
    m_ProjectInfoPage = new CProjectInfoPage;
    m_ProjectInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ProjectInfoPage);
    m_AccountInfoPage = new CAccountInfoPage;
    m_AccountInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountInfoPage);
    m_AccountCreationPage = new CAccountCreationPage;
    m_AccountCreationPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountCreationPage);
    m_CompletionPage = new CCompletionPage;
    m_CompletionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionPage);
    m_ErrProjectUnavailablePage = new CErrProjectUnavailablePage;
    m_ErrProjectUnavailablePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProjectUnavailablePage);
    m_ErrNoInternetConnectionPage = new CErrNoInternetConnectionPage;
    m_ErrNoInternetConnectionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrNoInternetConnectionPage);
    m_ErrAccountAlreadyExistsPage = new CErrAccountAlreadyExistsPage;
    m_ErrAccountAlreadyExistsPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrAccountAlreadyExistsPage);
    wxWizardPageSimple* lastPage = NULL;
////@end CWizAttachProject content construction
}

/*!
 * Runs the wizard.
 */

bool CWizAttachProject::Run()
{
    wxWizardPage* startPage = NULL;
    wxWindowListNode* node = GetChildren().GetFirst();
    while (node)
    {
        wxWizardPage* startPage = wxDynamicCast(node->GetData(), wxWizardPage);
        if (startPage) return RunWizard(startPage);
        node = node->GetNext();
    }
    return FALSE;
}

/*!
 * Should we show tooltips?
 */

bool CWizAttachProject::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWizAttachProject::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWizAttachProject bitmap retrieval
    if (name == wxT("res/attachprojectwizard.xpm"))
    {
        wxBitmap bitmap(attachprojectwizard_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CWizAttachProject bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CWizAttachProject::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWizAttachProject icon retrieval
    return wxNullIcon;
////@end CWizAttachProject icon retrieval
}

void CWizAttachProject::OnWizardCancel( wxWizardEvent& event )
{
    if ( wxMessageBox( _("Do you really want to cancel?"), _("Question"),
         wxICON_QUESTION | wxYES_NO, this ) != wxYES
       )
    {
        // not confirmed
        event.Veto();
    }
}


/*!
 * CWelcomePage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CWelcomePage, wxWizardPage )

/*!
 * CWelcomePage event table definition
 */

BEGIN_EVENT_TABLE( CWelcomePage, wxWizardPage )

////@begin CWelcomePage event table entries
////@end CWelcomePage event table entries

END_EVENT_TABLE()

/*!
 * CWelcomePage constructors
 */

CWelcomePage::CWelcomePage( )
{
}

CWelcomePage::CWelcomePage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * WizardPage creator
 */

bool CWelcomePage::Create( wxWizard* parent )
{
////@begin CWelcomePage member initialisation
////@end CWelcomePage member initialisation

////@begin CWelcomePage creation
    wxBitmap wizardBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CWelcomePage creation
    return TRUE;
}

/*!
 * Control creation for WizardPage
 */

void CWelcomePage::CreateControls()
{    
////@begin CWelcomePage content construction
    CWelcomePage* itemWizardPage2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage2->SetSizer(itemBoxSizer3);

    wxStaticText* itemStaticText4 = new wxStaticText;
    itemStaticText4->Create( itemWizardPage2, wxID_STATIC, _("Welcome to the Attach to Project\nWizard"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText4->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText;
    itemStaticText5->Create( itemWizardPage2, wxID_STATIC, _("This wizard will guide you through the process of attaching to a project."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemWizardPage2, wxID_STATIC, _("This wizard will require access to the Internet in order to attach to\na project using an existing account or creating a new one."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 125, 0, wxALIGN_LEFT|wxALL|wxFIXED_MINSIZE, 5);

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create( itemWizardPage2, wxID_STATIC, _("To continue, click Next."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALL, 5);

////@end CWelcomePage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CWelcomePage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CWelcomePage::GetNext() const
{
    // TODO: return the next page
    return ((CWizAttachProject*)GetParent())->m_ProjectInfoPage;
}

/*!
 * Should we show tooltips?
 */

bool CWelcomePage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWelcomePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWelcomePage bitmap retrieval
    if (name == wxT("res/attachprojectwizard.xpm"))
    {
        wxBitmap bitmap(attachprojectwizard_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CWelcomePage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CWelcomePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWelcomePage icon retrieval
    return wxNullIcon;
////@end CWelcomePage icon retrieval
}

/*!
 * CProjectInfoPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CProjectInfoPage, wxWizardPage )

/*!
 * CProjectInfoPage event table definition
 */

BEGIN_EVENT_TABLE( CProjectInfoPage, wxWizardPage )

////@begin CProjectInfoPage event table entries
////@end CProjectInfoPage event table entries

END_EVENT_TABLE()

/*!
 * CProjectInfoPage constructors
 */

CProjectInfoPage::CProjectInfoPage( )
{
}

CProjectInfoPage::CProjectInfoPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * WizardPage creator
 */

bool CProjectInfoPage::Create( wxWizard* parent )
{
////@begin CProjectInfoPage member initialisation
    m_ProjectUrlStaticCtrl = NULL;
    m_ProjectUrlCtrl = NULL;
////@end CProjectInfoPage member initialisation

////@begin CProjectInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CProjectInfoPage creation
    return TRUE;
}

/*!
 * Control creation for WizardPage
 */

void CProjectInfoPage::CreateControls()
{    
////@begin CProjectInfoPage content construction
    CProjectInfoPage* itemWizardPage10 = this;

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage10->SetSizer(itemBoxSizer11);

    wxStaticText* itemStaticText12 = new wxStaticText;
    itemStaticText12->Create( itemWizardPage10, wxID_STATIC, _("Project Information"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText12->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer11->Add(itemStaticText12, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText;
    itemStaticText13->Create( itemWizardPage10, wxID_STATIC, _("Which project do you wish to attach to?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer11->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText;
    itemStaticText15->Create( itemWizardPage10, wxID_STATIC, _("The project URL is generally the project's homepage.  It is in the form of\na web address and can be found in your browsers address bar."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText15, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer16 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer16->AddGrowableCol(1);
    itemBoxSizer11->Add(itemFlexGridSizer16, 0, wxALIGN_LEFT|wxALL, 5);

    m_ProjectUrlStaticCtrl = new wxStaticText;
    m_ProjectUrlStaticCtrl->Create( itemWizardPage10, ID_PROJECTURLSTATICCTRL, _("Project URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(m_ProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProjectUrlCtrl = new wxTextCtrl;
    m_ProjectUrlCtrl->Create( itemWizardPage10, ID_PROJECTURLCTRL, _T(""), wxDefaultPosition, wxSize(250, -1), 0 );
    itemFlexGridSizer16->Add(m_ProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer11->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText20 = new wxStaticText;
    itemStaticText20->Create( itemWizardPage10, wxID_STATIC, _("For more information, and to see a list of some BOINC-based projects,\ngo to:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText20, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink21 = new wxHyperLink;
    itemHyperLink21->Create( itemWizardPage10, ID_PROJECRINFOBOINCLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer11->Add(itemHyperLink21, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_ProjectUrlCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProjectURL) );
////@end CProjectInfoPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CProjectInfoPage::GetPrev() const
{
    // TODO: return the previous page
    return ((CWizAttachProject*)GetParent())->m_WelcomePage;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CProjectInfoPage::GetNext() const
{
    // TODO: return the next page
    return ((CWizAttachProject*)GetParent())->m_AccountInfoPage;
}

/*!
 * Should we show tooltips?
 */

bool CProjectInfoPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CProjectInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProjectInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CProjectInfoPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CProjectInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProjectInfoPage icon retrieval
    return wxNullIcon;
////@end CProjectInfoPage icon retrieval
}

/*!
 * CAccountInfoPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAccountInfoPage, wxWizardPage )

/*!
 * CAccountInfoPage event table definition
 */

BEGIN_EVENT_TABLE( CAccountInfoPage, wxWizardPage )

////@begin CAccountInfoPage event table entries
    EVT_RADIOBUTTON( ID_ACCOUNTCREATECTRL, CAccountInfoPage::OnAccountCreateCtrlSelected )

    EVT_RADIOBUTTON( ID_ACCOUNTUSEEXISTINGCTRL, CAccountInfoPage::OnAccountUseExistingCtrlSelected )

////@end CAccountInfoPage event table entries

END_EVENT_TABLE()

/*!
 * CAccountInfoPage constructors
 */

CAccountInfoPage::CAccountInfoPage( )
{
}

CAccountInfoPage::CAccountInfoPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * AccountInfoPage creator
 */

bool CAccountInfoPage::Create( wxWizard* parent )
{
////@begin CAccountInfoPage member initialisation
    m_AccountCreateCtrl = NULL;
    m_AccountUseExistingCtrl = NULL;
    m_AccountEmailAddressStaticCtrl = NULL;
    m_AccountEmailAddressCtrl = NULL;
    m_AccountPasswordStaticCtrl = NULL;
    m_AccountPasswordCtrl = NULL;
    m_AccountConfirmPasswordStaticCtrl = NULL;
    m_AccountConfirmPasswordCtrl = NULL;
////@end CAccountInfoPage member initialisation

////@begin CAccountInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountInfoPage creation
    return TRUE;
}

/*!
 * Control creation for AccountInfoPage
 */

void CAccountInfoPage::CreateControls()
{    
////@begin CAccountInfoPage content construction
    CAccountInfoPage* itemWizardPage22 = this;

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage22->SetSizer(itemBoxSizer23);

    wxStaticText* itemStaticText24 = new wxStaticText;
    itemStaticText24->Create( itemWizardPage22, wxID_STATIC, _("Account Information"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText24->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer23->Add(itemStaticText24, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText25 = new wxStaticText;
    itemStaticText25->Create( itemWizardPage22, wxID_STATIC, _("Do you wish to use an existing account or create a new one?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText25, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer23->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText27 = new wxStaticText;
    itemStaticText27->Create( itemWizardPage22, wxID_STATIC, _("If this is the first time you have attempted to attach to this project then\nyou should create a new account.  If you already have an account you\nshould use your existing email address and password to attach to the\nproject."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText27, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer28 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer28->AddGrowableCol(1);
    itemBoxSizer23->Add(itemFlexGridSizer28, 0, wxGROW|wxALL, 5);

    m_AccountCreateCtrl = new wxRadioButton;
    m_AccountCreateCtrl->Create( itemWizardPage22, ID_ACCOUNTCREATECTRL, _("Create new account"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_AccountCreateCtrl->SetValue(FALSE);
    itemFlexGridSizer28->Add(m_AccountCreateCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountUseExistingCtrl = new wxRadioButton;
    m_AccountUseExistingCtrl->Create( itemWizardPage22, ID_ACCOUNTUSEEXISTINGCTRL, _("Use existing account"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AccountUseExistingCtrl->SetValue(FALSE);
    itemFlexGridSizer28->Add(m_AccountUseExistingCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer31 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer31->AddGrowableCol(1);
    itemBoxSizer23->Add(itemFlexGridSizer31, 0, wxGROW|wxALL, 5);

    m_AccountEmailAddressStaticCtrl = new wxStaticText;
    m_AccountEmailAddressStaticCtrl->Create( itemWizardPage22, ID_ACCOUNTEMAILADDRESSSTATICCTRL, _("Email address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer31->Add(m_AccountEmailAddressStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountEmailAddressCtrl = new wxTextCtrl;
    m_AccountEmailAddressCtrl->Create( itemWizardPage22, ID_ACCOUNTEMAILADDRESSCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer31->Add(m_AccountEmailAddressCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordStaticCtrl = new wxStaticText;
    m_AccountPasswordStaticCtrl->Create( itemWizardPage22, ID_ACCOUNTPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer31->Add(m_AccountPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordCtrl = new wxTextCtrl;
    m_AccountPasswordCtrl->Create( itemWizardPage22, ID_ACCOUNTPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer31->Add(m_AccountPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountConfirmPasswordStaticCtrl = new wxStaticText;
    m_AccountConfirmPasswordStaticCtrl->Create( itemWizardPage22, ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL, _("Confirm password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer31->Add(m_AccountConfirmPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountConfirmPasswordCtrl = new wxTextCtrl;
    m_AccountConfirmPasswordCtrl->Create( itemWizardPage22, ID_ACCOUNTCONFIRMPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer31->Add(m_AccountConfirmPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_AccountEmailAddressCtrl->SetValidator( wxGenericValidator(& m_strAccountEmailAddress) );
    m_AccountPasswordCtrl->SetValidator( wxGenericValidator(& m_strAccountPassword) );
    m_AccountConfirmPasswordCtrl->SetValidator( wxGenericValidator(& m_strAccountConfirmPassword) );
////@end CAccountInfoPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAccountInfoPage::GetPrev() const
{
    // TODO: return the previous page
    return ((CWizAttachProject*)GetParent())->m_ProjectInfoPage;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAccountInfoPage::GetNext() const
{
    // TODO: return the next page
    return ((CWizAttachProject*)GetParent())->m_AccountCreationPage;
}

/*!
 * Should we show tooltips?
 */

bool CAccountInfoPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAccountInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAccountInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountInfoPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAccountInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountInfoPage icon retrieval
    return wxNullIcon;
////@end CAccountInfoPage icon retrieval
}
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTUSEXISTINGBUTTON
 */

void CAccountInfoPage::OnAccountUseExistingCtrlSelected( wxCommandEvent& event )
{
    m_AccountConfirmPasswordStaticCtrl->Hide();
    m_AccountConfirmPasswordCtrl->Hide();
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ACCOUNTCREATEBUTTON
 */

void CAccountInfoPage::OnAccountCreateCtrlSelected( wxCommandEvent& event )
{
    m_AccountConfirmPasswordStaticCtrl->Show();
    m_AccountConfirmPasswordCtrl->Show();
}


/*!
 * CAccountCreatePage custom event definition
 */
DEFINE_EVENT_TYPE(wxEVT_ACCOUNTCREATION_STATECHANGE)


/*!
 * CAccountCreationPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAccountCreationPage, wxWizardPage )

/*!
 * CAccountCreationPage event table definition
 */

BEGIN_EVENT_TABLE( CAccountCreationPage, wxWizardPage )

    EVT_ACCOUNTCREATION_STATECHANGE( CAccountCreationPage::OnStateChange )

////@begin CAccountCreationPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAccountCreationPage::OnPageChanged )

////@end CAccountCreationPage event table entries

END_EVENT_TABLE()

/*!
 * CAccountCreationPage constructors
 */

CAccountCreationPage::CAccountCreationPage( )
{
}

CAccountCreationPage::CAccountCreationPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * WizardPage creator
 */

bool CAccountCreationPage::Create( wxWizard* parent )
{
////@begin CAccountCreationPage member initialisation
    m_CommBOINCProjectImageCtrl = NULL;
    m_CommBOINCProjectCtrl = NULL;
    m_CommYahooImageCtrl = NULL;
    m_CommYahooCtrl = NULL;
    m_CommGoogleImageCtrl = NULL;
    m_CommGoogleCtrl = NULL;
    m_DetermineConnectionStatusImageCtrl = NULL;
    m_DetermineConnectionStatusCtrl = NULL;
    m_FinalAccountCreationStatusCtrl = NULL;
////@end CAccountCreationPage member initialisation
    m_iCurrentState = 0;

////@begin CAccountCreationPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountCreationPage creation
    return TRUE;
}

/*!
 * Control creation for WizardPage
 */

void CAccountCreationPage::CreateControls()
{    
////@begin CAccountCreationPage content construction
    CAccountCreationPage* itemWizardPage38 = this;

    wxBoxSizer* itemBoxSizer39 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage38->SetSizer(itemBoxSizer39);

    wxStaticText* itemStaticText40 = new wxStaticText;
    itemStaticText40->Create( itemWizardPage38, wxID_STATIC, _("Account Creation"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText40->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer39->Add(itemStaticText40, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText41 = new wxStaticText;
    itemStaticText41->Create( itemWizardPage38, wxID_STATIC, _("This wizard is now attempting to create a new account or validate your\nexisting account."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer39->Add(itemStaticText41, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer39->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText43 = new wxStaticText;
    itemStaticText43->Create( itemWizardPage38, wxID_STATIC, _("If this wizard cannot reach the project server, it'll attempt to contact a\ncouple known good websites in an effort to help diagnose the problem."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer39->Add(itemStaticText43, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer44 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer39->Add(itemFlexGridSizer44, 0, wxALIGN_LEFT|wxALL, 5);

    wxBitmap m_CommBOINCProjectImageCtrlBitmap(itemWizardPage38->GetBitmapResource(wxT("res/wizquestion.xpm")));
    m_CommBOINCProjectImageCtrl = new wxStaticBitmap;
    m_CommBOINCProjectImageCtrl->Create( itemWizardPage38, ID_COMMBOINCPROJECTIMAGECTRL, m_CommBOINCProjectImageCtrlBitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemFlexGridSizer44->Add(m_CommBOINCProjectImageCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CommBOINCProjectCtrl = new wxStaticText;
    m_CommBOINCProjectCtrl->Create( itemWizardPage38, ID_COMMBOINCPROJECTCTRL, _("Communicating with BOINC project"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer44->Add(m_CommBOINCProjectCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmap m_CommYahooImageCtrlBitmap(itemWizardPage38->GetBitmapResource(wxT("res/wizsuccess.xpm")));
    m_CommYahooImageCtrl = new wxStaticBitmap;
    m_CommYahooImageCtrl->Create( itemWizardPage38, ID_COMMYAHOOIMAGECTRL, m_CommYahooImageCtrlBitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemFlexGridSizer44->Add(m_CommYahooImageCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CommYahooCtrl = new wxStaticText;
    m_CommYahooCtrl->Create( itemWizardPage38, ID_COMMYAHOOCTRL, _("Communicating with Yahoo"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer44->Add(m_CommYahooCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmap m_CommGoogleImageCtrlBitmap(itemWizardPage38->GetBitmapResource(wxT("res/wizfailure.xpm")));
    m_CommGoogleImageCtrl = new wxStaticBitmap;
    m_CommGoogleImageCtrl->Create( itemWizardPage38, ID_COMMGOOGLEIMAGECTRL, m_CommGoogleImageCtrlBitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemFlexGridSizer44->Add(m_CommGoogleImageCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CommGoogleCtrl = new wxStaticText;
    m_CommGoogleCtrl->Create( itemWizardPage38, ID_COMMGOOGLECTRL, _("Communicating with Google"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer44->Add(m_CommGoogleCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmap m_DetermineConnectionStatusImageCtrlBitmap(itemWizardPage38->GetBitmapResource(wxT("res/wizquestion.xpm")));
    m_DetermineConnectionStatusImageCtrl = new wxStaticBitmap;
    m_DetermineConnectionStatusImageCtrl->Create( itemWizardPage38, ID_DETERMINECONNECTIONSTATUSIMAGECTRL, m_DetermineConnectionStatusImageCtrlBitmap, wxDefaultPosition, wxSize(16, 16), 0 );
    itemFlexGridSizer44->Add(m_DetermineConnectionStatusImageCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DetermineConnectionStatusCtrl = new wxStaticText;
    m_DetermineConnectionStatusCtrl->Create( itemWizardPage38, ID_DETERMINECONNECTIONSTATUSCTRL, _("Determining connection status"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer44->Add(m_DetermineConnectionStatusCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FinalAccountCreationStatusCtrl = new wxStaticText;
    m_FinalAccountCreationStatusCtrl->Create( itemWizardPage38, ID_FINALACCOUNTCREATIONSTATUSCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer39->Add(m_FinalAccountCreationStatusCtrl, 0, wxALIGN_LEFT|wxALL, 5);

////@end CAccountCreationPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAccountCreationPage::GetPrev() const
{
    // TODO: return the previous page
    return ((CWizAttachProject*)GetParent())->m_AccountInfoPage;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAccountCreationPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAccountCreationPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAccountCreationPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAccountCreationPage bitmap retrieval
    if (name == wxT("res/wizquestion.xpm"))
    {
        wxBitmap bitmap(wizquestion_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizsuccess.xpm"))
    {
        wxBitmap bitmap(wizsuccess_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizfailure.xpm"))
    {
        wxBitmap bitmap(wizfailure_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CAccountCreationPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAccountCreationPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAccountCreationPage icon retrieval
    return wxNullIcon;
////@end CAccountCreationPage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTCREATIONPAGE
 */

void CAccountCreationPage::OnPageChanged( wxWizardEvent& event )
{
    if (event.GetDirection() == false) return;

    m_iCurrentState = 0;

    CAccountCreationPageEvent TransitionEvent(wxEVT_ACCOUNTCREATION_STATECHANGE, this);
    AddPendingEvent(TransitionEvent);
}

/*!
 * wxEVT_ACCOUNTCREATION_STATECHANGE event handler for ID_ACCOUNTCREATIONPAGE
 */

void CAccountCreationPage::OnStateChange( CAccountCreationPageEvent& event )
{
    bool bPostNewEvent = true;

    wxFont fontOriginal = m_FinalAccountCreationStatusCtrl->GetFont();
    wxFont fontBold = m_FinalAccountCreationStatusCtrl->GetFont();
    fontBold.SetWeight(wxBOLD);

    switch(m_iCurrentState) {
        case 0:
            // Set initial bitmaps to question marks since we don't yet know how
            //   things will turn out.
            m_CommBOINCProjectImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizquestion.xpm")));
            m_CommYahooImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizquestion.xpm")));
            m_CommGoogleImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizquestion.xpm")));
            m_DetermineConnectionStatusImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizquestion.xpm")));

            // Hide the additional diagnostics stuff until we really need it.
            m_CommYahooImageCtrl->Hide();
            m_CommYahooCtrl->Hide();
            m_CommGoogleImageCtrl->Hide();
            m_CommGoogleCtrl->Hide();
            m_DetermineConnectionStatusImageCtrl->Hide();
            m_DetermineConnectionStatusCtrl->Hide();

            // Clear out any text that might exist in the final status field
            m_FinalAccountCreationStatusCtrl->SetLabel(wxT(""));

            // Highlight the current activity by making it bold
            m_CommBOINCProjectCtrl->SetFont(fontBold);
            break;
        case 1:
            // Attempt to create an account on the project server or validate an existing account
            wxSleep(5);

            // demo purpose, fail it.
            m_CommBOINCProjectImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizfailure.xpm")));
            m_CommBOINCProjectCtrl->SetFont(fontOriginal);

            // Show the additional diagnostics stuff.
            m_CommYahooImageCtrl->Show();
            m_CommYahooCtrl->Show();
            m_CommGoogleImageCtrl->Show();
            m_CommGoogleCtrl->Show();
            m_DetermineConnectionStatusImageCtrl->Show();
            m_DetermineConnectionStatusCtrl->Show();

            break;
        case 2:

            // Highlight the current activity by making it bold
            m_CommYahooCtrl->SetFont(fontBold);
            break;
        case 3:
            // Attempt to successfully download the Yahoo homepage
            wxSleep(5);

            // demo purpose, fail it.
            m_CommYahooImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizfailure.xpm")));
            m_CommYahooCtrl->SetFont(fontOriginal);
            break;
        case 4:
            // Highlight the current activity by making it bold
            m_CommGoogleCtrl->SetFont(fontBold);

            break;
        case 5:
            // Attempt to successfully download the Google homepage
            wxSleep(5);

            // demo purpose, fail it.
            m_CommGoogleImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizfailure.xpm")));
            m_CommGoogleCtrl->SetFont(fontOriginal);
            break;
        case 6:
            // Highlight the current activity by making it bold
            m_DetermineConnectionStatusCtrl->SetFont(fontBold);

            break;
        case 7:
            // Attempt to determine if we are even connected to a network
            wxSleep(5);

            // demo purpose, success.
            m_DetermineConnectionStatusImageCtrl->SetBitmap(GetBitmapResource(wxT("res/wizsuccess.xpm")));
            m_DetermineConnectionStatusCtrl->SetFont(fontOriginal);

            // Say something useful to go with this condition
            m_FinalAccountCreationStatusCtrl->SetLabel(_("One or more problems detected, click Next to troubleshoot the\nproblem."));
            break;
        default:
            bPostNewEvent = false;
            break;
    }
    m_iCurrentState++;

    Update();

    if (bPostNewEvent) {
        CAccountCreationPageEvent TransitionEvent(wxEVT_ACCOUNTCREATION_STATECHANGE, this);
        AddPendingEvent(TransitionEvent);
    }
}

/*!
 * CCompletionPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCompletionPage, wxWizardPage )

/*!
 * CCompletionPage event table definition
 */

BEGIN_EVENT_TABLE( CCompletionPage, wxWizardPage )

////@begin CCompletionPage event table entries
////@end CCompletionPage event table entries

END_EVENT_TABLE()

/*!
 * CCompletionPage constructors
 */

CCompletionPage::CCompletionPage( )
{
}

CCompletionPage::CCompletionPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CAccountResultPage creator
 */

bool CCompletionPage::Create( wxWizard* parent )
{
////@begin CCompletionPage member initialisation
////@end CCompletionPage member initialisation

////@begin CCompletionPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
////@end CCompletionPage creation
    return TRUE;
}

/*!
 * Control creation for CAccountResultPage
 */

void CCompletionPage::CreateControls()
{    
////@begin CCompletionPage content construction
    CCompletionPage* itemWizardPage54 = this;

////@end CCompletionPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CCompletionPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CCompletionPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CCompletionPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCompletionPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCompletionPage bitmap retrieval
    return wxNullBitmap;
////@end CCompletionPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCompletionPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCompletionPage icon retrieval
    return wxNullIcon;
////@end CCompletionPage icon retrieval
}

/*!
 * CErrProjectUnavailablePage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrProjectUnavailablePage, wxWizardPage )

/*!
 * CErrProjectUnavailablePage event table definition
 */

BEGIN_EVENT_TABLE( CErrProjectUnavailablePage, wxWizardPage )

////@begin CErrProjectUnavailablePage event table entries
////@end CErrProjectUnavailablePage event table entries

END_EVENT_TABLE()

/*!
 * CErrProjectUnavailablePage constructors
 */

CErrProjectUnavailablePage::CErrProjectUnavailablePage( )
{
}

CErrProjectUnavailablePage::CErrProjectUnavailablePage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrProjectUnavailable creator
 */

bool CErrProjectUnavailablePage::Create( wxWizard* parent )
{
////@begin CErrProjectUnavailablePage member initialisation
////@end CErrProjectUnavailablePage member initialisation

////@begin CErrProjectUnavailablePage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
////@end CErrProjectUnavailablePage creation
    return TRUE;
}

/*!
 * Control creation for CErrProjectUnavailable
 */

void CErrProjectUnavailablePage::CreateControls()
{    
////@begin CErrProjectUnavailablePage content construction
    CErrProjectUnavailablePage* itemWizardPage55 = this;

////@end CErrProjectUnavailablePage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrProjectUnavailablePage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrProjectUnavailablePage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CErrProjectUnavailablePage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrProjectUnavailablePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProjectUnavailablePage bitmap retrieval
    return wxNullBitmap;
////@end CErrProjectUnavailablePage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrProjectUnavailablePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProjectUnavailablePage icon retrieval
    return wxNullIcon;
////@end CErrProjectUnavailablePage icon retrieval
}

/*!
 * CErrNoInternetConnectionPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrNoInternetConnectionPage, wxWizardPage )

/*!
 * CErrNoInternetConnectionPage event table definition
 */

BEGIN_EVENT_TABLE( CErrNoInternetConnectionPage, wxWizardPage )

////@begin CErrNoInternetConnectionPage event table entries
////@end CErrNoInternetConnectionPage event table entries

END_EVENT_TABLE()

/*!
 * CErrNoInternetConnectionPage constructors
 */

CErrNoInternetConnectionPage::CErrNoInternetConnectionPage( )
{
}

CErrNoInternetConnectionPage::CErrNoInternetConnectionPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrNoInternetConnection creator
 */

bool CErrNoInternetConnectionPage::Create( wxWizard* parent )
{
////@begin CErrNoInternetConnectionPage member initialisation
////@end CErrNoInternetConnectionPage member initialisation

////@begin CErrNoInternetConnectionPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
////@end CErrNoInternetConnectionPage creation
    return TRUE;
}

/*!
 * Control creation for CErrNoInternetConnection
 */

void CErrNoInternetConnectionPage::CreateControls()
{    
////@begin CErrNoInternetConnectionPage content construction
    CErrNoInternetConnectionPage* itemWizardPage56 = this;

////@end CErrNoInternetConnectionPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrNoInternetConnectionPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrNoInternetConnectionPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CErrNoInternetConnectionPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrNoInternetConnectionPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrNoInternetConnectionPage bitmap retrieval
    return wxNullBitmap;
////@end CErrNoInternetConnectionPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrNoInternetConnectionPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrNoInternetConnectionPage icon retrieval
    return wxNullIcon;
////@end CErrNoInternetConnectionPage icon retrieval
}

/*!
 * CErrAccountAlreadyExistsPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CErrAccountAlreadyExistsPage, wxWizardPage )

/*!
 * CErrAccountAlreadyExistsPage event table definition
 */

BEGIN_EVENT_TABLE( CErrAccountAlreadyExistsPage, wxWizardPage )

////@begin CErrAccountAlreadyExistsPage event table entries
////@end CErrAccountAlreadyExistsPage event table entries

END_EVENT_TABLE()

/*!
 * CErrAccountAlreadyExistsPage constructors
 */

CErrAccountAlreadyExistsPage::CErrAccountAlreadyExistsPage( )
{
}

CErrAccountAlreadyExistsPage::CErrAccountAlreadyExistsPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrAccountAlreadyExists creator
 */

bool CErrAccountAlreadyExistsPage::Create( wxWizard* parent )
{
////@begin CErrAccountAlreadyExistsPage member initialisation
////@end CErrAccountAlreadyExistsPage member initialisation

////@begin CErrAccountAlreadyExistsPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
////@end CErrAccountAlreadyExistsPage creation
    return TRUE;
}

/*!
 * Control creation for CErrAccountAlreadyExists
 */

void CErrAccountAlreadyExistsPage::CreateControls()
{    
////@begin CErrAccountAlreadyExistsPage content construction
    CErrAccountAlreadyExistsPage* itemWizardPage57 = this;

////@end CErrAccountAlreadyExistsPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CErrAccountAlreadyExistsPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CErrAccountAlreadyExistsPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CErrAccountAlreadyExistsPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CErrAccountAlreadyExistsPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrAccountAlreadyExistsPage bitmap retrieval
    return wxNullBitmap;
////@end CErrAccountAlreadyExistsPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CErrAccountAlreadyExistsPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrAccountAlreadyExistsPage icon retrieval
    return wxNullIcon;
////@end CErrAccountAlreadyExistsPage icon retrieval
}


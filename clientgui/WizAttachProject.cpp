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

    m_WelcomePage = new WelcomePage;
    m_WelcomePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_WelcomePage);
    m_ProjectInfoPage = new ProjectInfoPage;
    m_ProjectInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ProjectInfoPage);
    m_AccountInfoPage = new AccountInfoPage;
    m_AccountInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountInfoPage);
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

/*!
 * WelcomePage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WelcomePage, wxWizardPage )

/*!
 * WelcomePage event table definition
 */

BEGIN_EVENT_TABLE( WelcomePage, wxWizardPage )

////@begin WelcomePage event table entries
////@end WelcomePage event table entries

END_EVENT_TABLE()

/*!
 * WelcomePage constructors
 */

WelcomePage::WelcomePage( )
{
}

WelcomePage::WelcomePage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * WizardPage creator
 */

bool WelcomePage::Create( wxWizard* parent )
{
////@begin WelcomePage member initialisation
////@end WelcomePage member initialisation

////@begin WelcomePage creation
    wxBitmap wizardBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end WelcomePage creation
    return TRUE;
}

/*!
 * Control creation for WizardPage
 */

void WelcomePage::CreateControls()
{    
////@begin WelcomePage content construction
    WelcomePage* itemWizardPage2 = this;

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

////@end WelcomePage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* WelcomePage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* WelcomePage::GetNext() const
{
    // TODO: return the next page
    return ((CWizAttachProject*)GetParent())->m_ProjectInfoPage;
}

/*!
 * Should we show tooltips?
 */

bool WelcomePage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap WelcomePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WelcomePage bitmap retrieval
    if (name == wxT("res/attachprojectwizard.xpm"))
    {
        wxBitmap bitmap(attachprojectwizard_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end WelcomePage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WelcomePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WelcomePage icon retrieval
    return wxNullIcon;
////@end WelcomePage icon retrieval
}

/*!
 * ProjectInfoPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ProjectInfoPage, wxWizardPage )

/*!
 * ProjectInfoPage event table definition
 */

BEGIN_EVENT_TABLE( ProjectInfoPage, wxWizardPage )

////@begin ProjectInfoPage event table entries
////@end ProjectInfoPage event table entries

END_EVENT_TABLE()

/*!
 * ProjectInfoPage constructors
 */

ProjectInfoPage::ProjectInfoPage( )
{
}

ProjectInfoPage::ProjectInfoPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * WizardPage creator
 */

bool ProjectInfoPage::Create( wxWizard* parent )
{
////@begin ProjectInfoPage member initialisation
////@end ProjectInfoPage member initialisation

////@begin ProjectInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end ProjectInfoPage creation
    return TRUE;
}

/*!
 * Control creation for WizardPage
 */

void ProjectInfoPage::CreateControls()
{    
////@begin ProjectInfoPage content construction
    ProjectInfoPage* itemWizardPage10 = this;

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

    wxStaticText* itemStaticText17 = new wxStaticText;
    itemStaticText17->Create( itemWizardPage10, wxID_STATIC, _("Project URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(itemStaticText17, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl18 = new wxTextCtrl;
    itemTextCtrl18->Create( itemWizardPage10, ID_PROJECTURLCTRL, _T(""), wxDefaultPosition, wxSize(250, -1), 0 );
    itemFlexGridSizer16->Add(itemTextCtrl18, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer11->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText20 = new wxStaticText;
    itemStaticText20->Create( itemWizardPage10, wxID_STATIC, _("For more information, and to see a list of some BOINC-based projects,\ngo to:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText20, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink21 = new wxHyperLink;
    itemHyperLink21->Create( itemWizardPage10, ID_PROJECRINFOBOINCLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer11->Add(itemHyperLink21, 0, wxALIGN_LEFT|wxALL, 5);

////@end ProjectInfoPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* ProjectInfoPage::GetPrev() const
{
    // TODO: return the previous page
    return ((CWizAttachProject*)GetParent())->m_WelcomePage;
}

/*!
 * Gets the next page.
 */

wxWizardPage* ProjectInfoPage::GetNext() const
{
    // TODO: return the next page
    return ((CWizAttachProject*)GetParent())->m_AccountInfoPage;
}

/*!
 * Should we show tooltips?
 */

bool ProjectInfoPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap ProjectInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ProjectInfoPage bitmap retrieval
    return wxNullBitmap;
////@end ProjectInfoPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ProjectInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ProjectInfoPage icon retrieval
    return wxNullIcon;
////@end ProjectInfoPage icon retrieval
}

/*!
 * AccountInfoPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( AccountInfoPage, wxWizardPage )

/*!
 * AccountInfoPage event table definition
 */

BEGIN_EVENT_TABLE( AccountInfoPage, wxWizardPage )

////@begin AccountInfoPage event table entries
////@end AccountInfoPage event table entries

END_EVENT_TABLE()

/*!
 * AccountInfoPage constructors
 */

AccountInfoPage::AccountInfoPage( )
{
}

AccountInfoPage::AccountInfoPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * AccountInfoPage creator
 */

bool AccountInfoPage::Create( wxWizard* parent )
{
////@begin AccountInfoPage member initialisation
    m_AccountCreate = NULL;
    m_AccountUseExisting = NULL;
////@end AccountInfoPage member initialisation

////@begin AccountInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end AccountInfoPage creation
    return TRUE;
}

/*!
 * Control creation for AccountInfoPage
 */

void AccountInfoPage::CreateControls()
{    
////@begin AccountInfoPage content construction
    AccountInfoPage* itemWizardPage22 = this;

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage22->SetSizer(itemBoxSizer23);

    wxStaticText* itemStaticText24 = new wxStaticText;
    itemStaticText24->Create( itemWizardPage22, wxID_STATIC, _("Account Information"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText24->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer23->Add(itemStaticText24, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText25 = new wxStaticText;
    itemStaticText25->Create( itemWizardPage22, wxID_STATIC, _("Do you wish to use an existing account or create a new one?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText25, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    itemBoxSizer23->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText27 = new wxStaticText;
    itemStaticText27->Create( itemWizardPage22, wxID_STATIC, _("If this is the first time you have attempted to attach to this project then\nyou should create a new account.  If you already have an account you\nshould use your existing email address and password to attach to the\nproject."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText27, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    wxFlexGridSizer* itemFlexGridSizer28 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer28->AddGrowableCol(1);
    itemBoxSizer23->Add(itemFlexGridSizer28, 0, wxGROW|wxALL, 5);

    m_AccountCreate = new wxRadioButton;
    m_AccountCreate->Create( itemWizardPage22, ID_ACCOUNTCREATEBUTTON, _("Create new account"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_AccountCreate->SetValue(FALSE);
    itemFlexGridSizer28->Add(m_AccountCreate, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountUseExisting = new wxRadioButton;
    m_AccountUseExisting->Create( itemWizardPage22, ID_ACCOUNTUSEXISTINGBUTTON, _("Use existing account"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AccountUseExisting->SetValue(FALSE);
    itemFlexGridSizer28->Add(m_AccountUseExisting, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer23->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer32 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer32->AddGrowableCol(1);
    itemBoxSizer23->Add(itemFlexGridSizer32, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText33 = new wxStaticText;
    itemStaticText33->Create( itemWizardPage22, wxID_STATIC, _("Email address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl34 = new wxTextCtrl;
    itemTextCtrl34->Create( itemWizardPage22, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemTextCtrl34, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText35 = new wxStaticText;
    itemStaticText35->Create( itemWizardPage22, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText35, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl36 = new wxTextCtrl;
    itemTextCtrl36->Create( itemWizardPage22, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer32->Add(itemTextCtrl36, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText37 = new wxStaticText;
    itemStaticText37->Create( itemWizardPage22, wxID_STATIC, _("Confirm password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText37, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl38 = new wxTextCtrl;
    itemTextCtrl38->Create( itemWizardPage22, ID_TEXTCTRL2, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer32->Add(itemTextCtrl38, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end AccountInfoPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* AccountInfoPage::GetPrev() const
{
    // TODO: return the previous page
    return ((CWizAttachProject*)GetParent())->m_ProjectInfoPage;
}

/*!
 * Gets the next page.
 */

wxWizardPage* AccountInfoPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool AccountInfoPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap AccountInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin AccountInfoPage bitmap retrieval
    return wxNullBitmap;
////@end AccountInfoPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon AccountInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin AccountInfoPage icon retrieval
    return wxNullIcon;
////@end AccountInfoPage icon retrieval
}

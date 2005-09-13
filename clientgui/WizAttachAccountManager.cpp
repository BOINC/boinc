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
#pragma implementation "WizAttachAccountManager.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "WizAttachAccountManager.h"

////@begin XPM images
#include "res/attachprojectwizard.xpm"
#include "res/wizprogress01.xpm"
#include "res/wizprogress02.xpm"
#include "res/wizprogress03.xpm"
#include "res/wizprogress04.xpm"
#include "res/wizprogress05.xpm"
#include "res/wizprogress06.xpm"
#include "res/wizprogress07.xpm"
#include "res/wizprogress08.xpm"
#include "res/wizprogress09.xpm"
#include "res/wizprogress10.xpm"
#include "res/wizprogress11.xpm"
#include "res/wizprogress12.xpm"
////@end XPM images

/*!
 * CWizAttachAccountManager type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CWizAttachAccountManager, wxWizard )

/*!
 * CWizAttachAccountManager event table definition
 */

BEGIN_EVENT_TABLE( CWizAttachAccountManager, wxWizard )

////@begin CWizAttachAccountManager event table entries
    EVT_WIZARD_FINISHED( ID_ATTACHACCOUNTMANAGERWIZARD, CWizAttachAccountManager::OnFinished )

////@end CWizAttachAccountManager event table entries

END_EVENT_TABLE()

/*!
 * CWizAttachAccountManager constructors
 */

CWizAttachAccountManager::CWizAttachAccountManager( )
{
}

CWizAttachAccountManager::CWizAttachAccountManager( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
    Create(parent, id, pos);
}

/*!
 * CWizAttachAccountManager creator
 */

bool CWizAttachAccountManager::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
////@begin CWizAttachAccountManager member initialisation
    m_WelcomePage = NULL;
    m_AccountManagerInfoPage = NULL;
    m_AccountManagerPropertiesPage = NULL;
    m_AccountInfoPage = NULL;
    m_AttachAccountManagerPage = NULL;
    m_CompletionPage = NULL;
    m_CompletionErrorPage = NULL;
    m_ErrAccountManagerNotDetectedPage = NULL;
    m_ErrAccountManagerUnavailablePage = NULL;
    m_ErrNoInternetConnectionPage = NULL;
    m_ErrProxyPage = NULL;
    m_ErrRefCountPage = NULL;
////@end CWizAttachAccountManager member initialisation

////@begin CWizAttachAccountManager creation
    wxBitmap wizardBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    wxWizard::Create( parent, id, _("Attach to Account Manager"), wizardBitmap, pos );

    CreateControls();
////@end CWizAttachAccountManager creation
    return TRUE;
}

/*!
 * Control creation for CWizAttachAccountManager
 */

void CWizAttachAccountManager::CreateControls()
{    
////@begin CWizAttachAccountManager content construction
    wxWizard* itemWizard1 = this;

    m_WelcomePage = new CAMWelcomePage;
    m_WelcomePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_WelcomePage);
    m_AccountManagerInfoPage = new CAMAccountManagerInfoPage;
    m_AccountManagerInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountManagerInfoPage);
    m_AccountManagerPropertiesPage = new CAMAccountManagerPropertiesPage;
    m_AccountManagerPropertiesPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountManagerPropertiesPage);
    m_AccountInfoPage = new CAMAccountInfoPage;
    m_AccountInfoPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AccountInfoPage);
    m_AttachAccountManagerPage = new CAMAttachAccountManagerPage;
    m_AttachAccountManagerPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_AttachAccountManagerPage);
    m_CompletionPage = new CAMCompletionPage;
    m_CompletionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionPage);
    m_CompletionErrorPage = new CAMCompletionErrorPage;
    m_CompletionErrorPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_CompletionErrorPage);
    m_ErrAccountManagerNotDetectedPage = new CAMErrAccountManagerNotDetectedPage;
    m_ErrAccountManagerNotDetectedPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrAccountManagerNotDetectedPage);
    m_ErrAccountManagerUnavailablePage = new CAMErrAccountManagerUnavailablePage;
    m_ErrAccountManagerUnavailablePage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrAccountManagerUnavailablePage);
    m_ErrNoInternetConnectionPage = new CAMErrNoInternetConnectionPage;
    m_ErrNoInternetConnectionPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrNoInternetConnectionPage);
    m_ErrProxyPage = new CAMErrProxyPage;
    m_ErrProxyPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrProxyPage);
    m_ErrRefCountPage = new CAMErrRefCountPage;
    m_ErrRefCountPage->Create( itemWizard1 );

    itemWizard1->FitToPage(m_ErrRefCountPage);
    wxWizardPageSimple* lastPage = NULL;
////@end CWizAttachAccountManager content construction
}

/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_ATTACHACCOUNTMANAGERWIZARD
 */

void CWizAttachAccountManager::OnFinished( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_FINISHED event handler for ID_ATTACHACCOUNTMANAGERWIZARD in CWizAttachAccountManager.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_FINISHED event handler for ID_ATTACHACCOUNTMANAGERWIZARD in CWizAttachAccountManager. 
}

/*!
 * Runs the wizard.
 */

bool CWizAttachAccountManager::Run()
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

bool CWizAttachAccountManager::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWizAttachAccountManager::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWizAttachAccountManager bitmap retrieval
    if (name == wxT("res/attachprojectwizard.xpm"))
    {
        wxBitmap bitmap(attachprojectwizard_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CWizAttachAccountManager bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CWizAttachAccountManager::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWizAttachAccountManager icon retrieval
    return wxNullIcon;
////@end CWizAttachAccountManager icon retrieval
}

/*!
 * CAMWelcomePage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMWelcomePage, wxWizardPage )

/*!
 * CAMWelcomePage event table definition
 */

BEGIN_EVENT_TABLE( CAMWelcomePage, wxWizardPage )

////@begin CAMWelcomePage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMWelcomePage::OnPageChanged )
    EVT_WIZARD_PAGE_CHANGING( -1, CAMWelcomePage::OnPageChanging )
    EVT_WIZARD_CANCEL( -1, CAMWelcomePage::OnCancel )

////@end CAMWelcomePage event table entries

END_EVENT_TABLE()

/*!
 * CAMWelcomePage constructors
 */

CAMWelcomePage::CAMWelcomePage( )
{
}

CAMWelcomePage::CAMWelcomePage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CWelcomePage creator
 */

bool CAMWelcomePage::Create( wxWizard* parent )
{
////@begin CAMWelcomePage member initialisation
#if defined(__WXDEBUG__)
    m_ErrProjectPropertiesCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectPropertiesURLCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectAttachFailureCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrGoogleCommCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrYahooCommCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrNetDetectionCtrl = NULL;
#endif
////@end CAMWelcomePage member initialisation

////@begin CAMWelcomePage creation
    wxBitmap wizardBitmap(GetBitmapResource(wxT("res/attachprojectwizard.xpm")));
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMWelcomePage creation
    return TRUE;
}

/*!
 * Control creation for CWelcomePage
 */

void CAMWelcomePage::CreateControls()
{    
////@begin CAMWelcomePage content construction
    CAMWelcomePage* itemWizardPage2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage2->SetSizer(itemBoxSizer3);

    wxStaticText* itemStaticText4 = new wxStaticText;
    itemStaticText4->Create( itemWizardPage2, wxID_STATIC, _("Attach to account manager"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText4->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText;
    itemStaticText5->Create( itemWizardPage2, wxID_STATIC, _("We'll now guide you through the process of attaching to an\naccount manager."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

#if defined(__WXDEBUG__)
    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemWizardPage2, wxID_ANY, _("Debug Flags"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(-1, 2, 0, 0);
    itemFlexGridSizer8->AddGrowableCol(0);
    itemFlexGridSizer8->AddGrowableCol(1);
    itemStaticBoxSizer7->Add(itemFlexGridSizer8, 0, wxGROW|wxALL, 5);

    m_ErrProjectPropertiesCtrl = new wxCheckBox;
    m_ErrProjectPropertiesCtrl->Create( itemWizardPage2, ID_ERRPROJECTPROPERTIES, _("Project Properties Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectPropertiesCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectPropertiesCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectPropertiesURLCtrl = new wxCheckBox;
    m_ErrProjectPropertiesURLCtrl->Create( itemWizardPage2, ID_ERRPROJECTPROPERTIESURL, _("Project Properties URL Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectPropertiesURLCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectPropertiesURLCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectAttachFailureCtrl = new wxCheckBox;
    m_ErrProjectAttachFailureCtrl->Create( itemWizardPage2, ID_ERRPROJECTATTACHFAILURE, _("Project Attach Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectAttachFailureCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectAttachFailureCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrGoogleCommCtrl = new wxCheckBox;
    m_ErrGoogleCommCtrl->Create( itemWizardPage2, ID_ERRGOOGLECOMM, _("Google Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrGoogleCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrGoogleCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrYahooCommCtrl = new wxCheckBox;
    m_ErrYahooCommCtrl->Create( itemWizardPage2, ID_ERRYAHOOCOMM, _("Yahoo Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrYahooCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrYahooCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrNetDetectionCtrl = new wxCheckBox;
    m_ErrNetDetectionCtrl->Create( itemWizardPage2, ID_ERRNETDETECTION, _("Net Detection Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrNetDetectionCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrNetDetectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

#endif

    wxStaticText* itemStaticText20 = new wxStaticText;
    itemStaticText20->Create( itemWizardPage2, wxID_STATIC, _("To continue, click Next."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText20, 0, wxALIGN_LEFT|wxALL, 5);

////@end CAMWelcomePage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WELCOMEPAGE
 */

void CAMWelcomePage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WELCOMEPAGE in CWelcomePage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WELCOMEPAGE in CWelcomePage. 
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE
 */

void CAMWelcomePage::OnPageChanging( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE in CWelcomePage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE in CWelcomePage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE
 */

void CAMWelcomePage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE in CWelcomePage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE in CWelcomePage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMWelcomePage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMWelcomePage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMWelcomePage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMWelcomePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMWelcomePage bitmap retrieval
    if (name == wxT("res/attachprojectwizard.xpm"))
    {
        wxBitmap bitmap(attachprojectwizard_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CAMWelcomePage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMWelcomePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMWelcomePage icon retrieval
    return wxNullIcon;
////@end CAMWelcomePage icon retrieval
}

/*!
 * CAMAccountManagerInfoPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMAccountManagerInfoPage, wxWizardPage )

/*!
 * CAMAccountManagerInfoPage event table definition
 */

BEGIN_EVENT_TABLE( CAMAccountManagerInfoPage, wxWizardPage )

////@begin CAMAccountManagerInfoPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMAccountManagerInfoPage::OnPageChanged )
    EVT_WIZARD_PAGE_CHANGING( -1, CAMAccountManagerInfoPage::OnPageChanging )
    EVT_WIZARD_CANCEL( -1, CAMAccountManagerInfoPage::OnCancel )

////@end CAMAccountManagerInfoPage event table entries

END_EVENT_TABLE()

/*!
 * CAMAccountManagerInfoPage constructors
 */

CAMAccountManagerInfoPage::CAMAccountManagerInfoPage( )
{
}

CAMAccountManagerInfoPage::CAMAccountManagerInfoPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CProjectInfoPage creator
 */

bool CAMAccountManagerInfoPage::Create( wxWizard* parent )
{
////@begin CAMAccountManagerInfoPage member initialisation
    m_AccountManagerUrlStaticCtrl = NULL;
    m_AccountManagerUrlCtrl = NULL;
////@end CAMAccountManagerInfoPage member initialisation

////@begin CAMAccountManagerInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMAccountManagerInfoPage creation
    return TRUE;
}

/*!
 * Control creation for CProjectInfoPage
 */

void CAMAccountManagerInfoPage::CreateControls()
{    
////@begin CAMAccountManagerInfoPage content construction
    CAMAccountManagerInfoPage* itemWizardPage21 = this;

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage21->SetSizer(itemBoxSizer22);

    wxStaticText* itemStaticText23 = new wxStaticText;
    itemStaticText23->Create( itemWizardPage21, wxID_STATIC, _("Account Manager URL"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText23->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer22->Add(itemStaticText23, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText24 = new wxStaticText;
    itemStaticText24->Create( itemWizardPage21, wxID_STATIC, _("Enter the URL of the account manager's web site."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText24, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer22->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText;
    itemStaticText26->Create( itemWizardPage21, wxID_STATIC, _("You can copy and paste the URL from your browser’s address bar."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText26, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer22->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer28 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer28->AddGrowableCol(1);
    itemBoxSizer22->Add(itemFlexGridSizer28, 0, wxALIGN_LEFT|wxALL, 5);

    m_AccountManagerUrlStaticCtrl = new wxStaticText;
    m_AccountManagerUrlStaticCtrl->Create( itemWizardPage21, ID_ACCOUNTMANAGERURLSTATICCTRL, _("Account Manager URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer28->Add(m_AccountManagerUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountManagerUrlCtrl = new wxTextCtrl;
    m_AccountManagerUrlCtrl->Create( itemWizardPage21, ID_ACCOUNTMANAGERURLCTRL, _T(""), wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer28->Add(m_AccountManagerUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer22->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText32 = new wxStaticText;
    itemStaticText32->Create( itemWizardPage21, wxID_STATIC, _("For a list of BOINC-based account managers go to:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemStaticText32, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink33 = new wxHyperLink;
    itemHyperLink33->Create( itemWizardPage21, ID_PROJECRINFOBOINCLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer22->Add(itemHyperLink33, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_AccountManagerUrlCtrl->SetValidator( CValidateURL( & m_strProjectURL) );
////@end CAMAccountManagerInfoPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
 */

void CAMAccountManagerInfoPage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE in CProjectInfoPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE in CProjectInfoPage. 
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
 */

void CAMAccountManagerInfoPage::OnPageChanging( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE in CProjectInfoPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE in CProjectInfoPage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
 */

void CAMAccountManagerInfoPage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE in CProjectInfoPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE in CProjectInfoPage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMAccountManagerInfoPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMAccountManagerInfoPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMAccountManagerInfoPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMAccountManagerInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMAccountManagerInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CAMAccountManagerInfoPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMAccountManagerInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMAccountManagerInfoPage icon retrieval
    return wxNullIcon;
////@end CAMAccountManagerInfoPage icon retrieval
}

/*!
 * CAMAccountManagerPropertiesPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMAccountManagerPropertiesPage, wxWizardPage )

/*!
 * CAMAccountManagerPropertiesPage event table definition
 */

BEGIN_EVENT_TABLE( CAMAccountManagerPropertiesPage, wxWizardPage )

////@begin CAMAccountManagerPropertiesPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMAccountManagerPropertiesPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CAMAccountManagerPropertiesPage::OnCancel )

////@end CAMAccountManagerPropertiesPage event table entries

END_EVENT_TABLE()

/*!
 * CAMAccountManagerPropertiesPage constructors
 */

CAMAccountManagerPropertiesPage::CAMAccountManagerPropertiesPage( )
{
}

CAMAccountManagerPropertiesPage::CAMAccountManagerPropertiesPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CProjectPropertiesPage creator
 */

bool CAMAccountManagerPropertiesPage::Create( wxWizard* parent )
{
////@begin CAMAccountManagerPropertiesPage member initialisation
    m_ProjectPropertiesProgress = NULL;
////@end CAMAccountManagerPropertiesPage member initialisation

////@begin CAMAccountManagerPropertiesPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMAccountManagerPropertiesPage creation
    return TRUE;
}

/*!
 * Control creation for CProjectPropertiesPage
 */

void CAMAccountManagerPropertiesPage::CreateControls()
{    
////@begin CAMAccountManagerPropertiesPage content construction
    CAMAccountManagerPropertiesPage* itemWizardPage34 = this;

    wxBoxSizer* itemBoxSizer35 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage34->SetSizer(itemBoxSizer35);

    wxStaticText* itemStaticText36 = new wxStaticText;
    itemStaticText36->Create( itemWizardPage34, wxID_STATIC, _("Communicating with account manager \nPlease wait..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText36->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer35->Add(itemStaticText36, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer35->Add(5, 80, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer38 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer38->AddGrowableRow(0);
    itemFlexGridSizer38->AddGrowableCol(0);
    itemFlexGridSizer38->AddGrowableCol(1);
    itemFlexGridSizer38->AddGrowableCol(2);
    itemBoxSizer35->Add(itemFlexGridSizer38, 0, wxGROW|wxALL, 5);

    itemFlexGridSizer38->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

    wxBitmap m_ProjectPropertiesProgressBitmap(itemWizardPage34->GetBitmapResource(wxT("res/wizprogress01.xpm")));
    m_ProjectPropertiesProgress = new wxStaticBitmap;
    m_ProjectPropertiesProgress->Create( itemWizardPage34, ID_PROJECTPROPERTIESPROGRESS, m_ProjectPropertiesProgressBitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemFlexGridSizer38->Add(m_ProjectPropertiesProgress, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer38->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

////@end CAMAccountManagerPropertiesPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTPROPERTIESPAGE
 */

void CAMAccountManagerPropertiesPage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTPROPERTIESPAGE in CProjectPropertiesPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTPROPERTIESPAGE in CProjectPropertiesPage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTPROPERTIESPAGE
 */

void CAMAccountManagerPropertiesPage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_PROJECTPROPERTIESPAGE in CProjectPropertiesPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_PROJECTPROPERTIESPAGE in CProjectPropertiesPage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMAccountManagerPropertiesPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMAccountManagerPropertiesPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMAccountManagerPropertiesPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMAccountManagerPropertiesPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMAccountManagerPropertiesPage bitmap retrieval
    if (name == wxT("res/wizprogress01.xpm"))
    {
        wxBitmap bitmap(wizprogress01_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CAMAccountManagerPropertiesPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMAccountManagerPropertiesPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMAccountManagerPropertiesPage icon retrieval
    return wxNullIcon;
////@end CAMAccountManagerPropertiesPage icon retrieval
}

/*!
 * CAMAccountInfoPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMAccountInfoPage, wxWizardPage )

/*!
 * CAMAccountInfoPage event table definition
 */

BEGIN_EVENT_TABLE( CAMAccountInfoPage, wxWizardPage )

////@begin CAMAccountInfoPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMAccountInfoPage::OnPageChanged )
    EVT_WIZARD_PAGE_CHANGING( -1, CAMAccountInfoPage::OnPageChanging )
    EVT_WIZARD_CANCEL( -1, CAMAccountInfoPage::OnCancel )

////@end CAMAccountInfoPage event table entries

END_EVENT_TABLE()

/*!
 * CAMAccountInfoPage constructors
 */

CAMAccountInfoPage::CAMAccountInfoPage( )
{
}

CAMAccountInfoPage::CAMAccountInfoPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CAccountInfoPage creator
 */

bool CAMAccountInfoPage::Create( wxWizard* parent )
{
////@begin CAMAccountInfoPage member initialisation
    m_AccountEmailAddressStaticCtrl = NULL;
    m_AccountEmailAddressCtrl = NULL;
    m_AccountPasswordStaticCtrl = NULL;
    m_AccountPasswordCtrl = NULL;
////@end CAMAccountInfoPage member initialisation

////@begin CAMAccountInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMAccountInfoPage creation
    return TRUE;
}

/*!
 * Control creation for CAccountInfoPage
 */

void CAMAccountInfoPage::CreateControls()
{    
////@begin CAMAccountInfoPage content construction
    CAMAccountInfoPage* itemWizardPage42 = this;

    wxBoxSizer* itemBoxSizer43 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage42->SetSizer(itemBoxSizer43);

    wxStaticText* itemStaticText44 = new wxStaticText;
    itemStaticText44->Create( itemWizardPage42, wxID_STATIC, _("Account information"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText44->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer43->Add(itemStaticText44, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer43->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer46 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer46->AddGrowableCol(1);
    itemBoxSizer43->Add(itemFlexGridSizer46, 0, wxGROW|wxALL, 5);

    m_AccountEmailAddressStaticCtrl = new wxStaticText;
    m_AccountEmailAddressStaticCtrl->Create( itemWizardPage42, ID_ACCOUNTEMAILADDRESSSTATICCTRL, _("Email address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer46->Add(m_AccountEmailAddressStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountEmailAddressCtrl = new wxTextCtrl;
    m_AccountEmailAddressCtrl->Create( itemWizardPage42, ID_ACCOUNTEMAILADDRESSCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer46->Add(m_AccountEmailAddressCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordStaticCtrl = new wxStaticText;
    m_AccountPasswordStaticCtrl->Create( itemWizardPage42, ID_ACCOUNTPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer46->Add(m_AccountPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AccountPasswordCtrl = new wxTextCtrl;
    m_AccountPasswordCtrl->Create( itemWizardPage42, ID_ACCOUNTPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer46->Add(m_AccountPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_AccountEmailAddressCtrl->SetValidator( wxGenericValidator(& m_strAccountEmailAddress) );
    m_AccountPasswordCtrl->SetValidator( wxGenericValidator(& m_strAccountPassword) );
////@end CAMAccountInfoPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE
 */

void CAMAccountInfoPage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE in CAccountInfoPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTINFOPAGE in CAccountInfoPage. 
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE
 */

void CAMAccountInfoPage::OnPageChanging( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE in CAccountInfoPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTINFOPAGE in CAccountInfoPage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTINFOPAGE
 */

void CAMAccountInfoPage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTINFOPAGE in CAccountInfoPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTINFOPAGE in CAccountInfoPage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMAccountInfoPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMAccountInfoPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMAccountInfoPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMAccountInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMAccountInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CAMAccountInfoPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMAccountInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMAccountInfoPage icon retrieval
    return wxNullIcon;
////@end CAMAccountInfoPage icon retrieval
}

/*!
 * CAMAttachAccountManagerPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMAttachAccountManagerPage, wxWizardPage )

/*!
 * CAMAttachAccountManagerPage event table definition
 */

BEGIN_EVENT_TABLE( CAMAttachAccountManagerPage, wxWizardPage )

////@begin CAMAttachAccountManagerPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMAttachAccountManagerPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CAMAttachAccountManagerPage::OnCancel )

////@end CAMAttachAccountManagerPage event table entries

END_EVENT_TABLE()

/*!
 * CAMAttachAccountManagerPage constructors
 */

CAMAttachAccountManagerPage::CAMAttachAccountManagerPage( )
{
}

CAMAttachAccountManagerPage::CAMAttachAccountManagerPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CAttachProjectPage creator
 */

bool CAMAttachAccountManagerPage::Create( wxWizard* parent )
{
////@begin CAMAttachAccountManagerPage member initialisation
    m_AttachProjectProgress = NULL;
////@end CAMAttachAccountManagerPage member initialisation

////@begin CAMAttachAccountManagerPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMAttachAccountManagerPage creation
    return TRUE;
}

/*!
 * Control creation for CAttachProjectPage
 */

void CAMAttachAccountManagerPage::CreateControls()
{    
////@begin CAMAttachAccountManagerPage content construction
    CAMAttachAccountManagerPage* itemWizardPage51 = this;

    wxBoxSizer* itemBoxSizer52 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage51->SetSizer(itemBoxSizer52);

    wxStaticText* itemStaticText53 = new wxStaticText;
    itemStaticText53->Create( itemWizardPage51, wxID_STATIC, _("Communicating with account manager\nPlease wait..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText53->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer52->Add(itemStaticText53, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer52->Add(5, 80, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer55 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer55->AddGrowableRow(0);
    itemFlexGridSizer55->AddGrowableCol(0);
    itemFlexGridSizer55->AddGrowableCol(1);
    itemFlexGridSizer55->AddGrowableCol(2);
    itemBoxSizer52->Add(itemFlexGridSizer55, 0, wxGROW|wxALL, 5);

    itemFlexGridSizer55->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

    wxBitmap m_AttachProjectProgressBitmap(itemWizardPage51->GetBitmapResource(wxT("res/wizprogress01.xpm")));
    m_AttachProjectProgress = new wxStaticBitmap;
    m_AttachProjectProgress->Create( itemWizardPage51, ID_ATTACHPROJECTPROGRESS, m_AttachProjectProgressBitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemFlexGridSizer55->Add(m_AttachProjectProgress, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer55->Add(5, 5, 0, wxGROW|wxGROW|wxALL, 5);

////@end CAMAttachAccountManagerPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE
 */

void CAMAttachAccountManagerPage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE in CAttachProjectPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE in CAttachProjectPage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ATTACHPROJECTPAGE
 */

void CAMAttachAccountManagerPage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_ATTACHPROJECTPAGE in CAttachProjectPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_ATTACHPROJECTPAGE in CAttachProjectPage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMAttachAccountManagerPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMAttachAccountManagerPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMAttachAccountManagerPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMAttachAccountManagerPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMAttachAccountManagerPage bitmap retrieval
    if (name == wxT("res/wizprogress01.xpm"))
    {
        wxBitmap bitmap(wizprogress01_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CAMAttachAccountManagerPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMAttachAccountManagerPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMAttachAccountManagerPage icon retrieval
    return wxNullIcon;
////@end CAMAttachAccountManagerPage icon retrieval
}

/*!
 * CAMCompletionPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMCompletionPage, wxWizardPage )

/*!
 * CAMCompletionPage event table definition
 */

BEGIN_EVENT_TABLE( CAMCompletionPage, wxWizardPage )

////@begin CAMCompletionPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMCompletionPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CAMCompletionPage::OnCancel )
    EVT_WIZARD_FINISHED( ID_COMPLETIONPAGE, CAMCompletionPage::OnFinished )

////@end CAMCompletionPage event table entries

END_EVENT_TABLE()

/*!
 * CAMCompletionPage constructors
 */

CAMCompletionPage::CAMCompletionPage( )
{
}

CAMCompletionPage::CAMCompletionPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CCompletionPage creator
 */

bool CAMCompletionPage::Create( wxWizard* parent )
{
////@begin CAMCompletionPage member initialisation
    m_CompletionMessage = NULL;
////@end CAMCompletionPage member initialisation

////@begin CAMCompletionPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMCompletionPage creation
    return TRUE;
}

/*!
 * Control creation for CCompletionPage
 */

void CAMCompletionPage::CreateControls()
{    
////@begin CAMCompletionPage content construction
    CAMCompletionPage* itemWizardPage59 = this;

    wxBoxSizer* itemBoxSizer60 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage59->SetSizer(itemBoxSizer60);

    wxStaticText* itemStaticText61 = new wxStaticText;
    itemStaticText61->Create( itemWizardPage59, wxID_STATIC, _("Attached to account manager"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText61->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer60->Add(itemStaticText61, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText62 = new wxStaticText;
    itemStaticText62->Create( itemWizardPage59, wxID_STATIC, _("You are now successfully attached to this account manager."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer60->Add(itemStaticText62, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer60->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_CompletionMessage = new wxStaticText;
    m_CompletionMessage->Create( itemWizardPage59, wxID_STATIC, _("Click Finish to close."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer60->Add(m_CompletionMessage, 0, wxALIGN_LEFT|wxALL, 5);

////@end CAMCompletionPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE
 */

void CAMCompletionPage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE in CCompletionPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE in CCompletionPage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE
 */

void CAMCompletionPage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE in CCompletionPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE in CCompletionPage. 
}

/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE
 */

void CAMCompletionPage::OnFinished( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE in CCompletionPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE in CCompletionPage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMCompletionPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMCompletionPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMCompletionPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMCompletionPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMCompletionPage bitmap retrieval
    return wxNullBitmap;
////@end CAMCompletionPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMCompletionPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMCompletionPage icon retrieval
    return wxNullIcon;
////@end CAMCompletionPage icon retrieval
}

/*!
 * CAMCompletionErrorPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMCompletionErrorPage, wxWizardPage )

/*!
 * CAMCompletionErrorPage event table definition
 */

BEGIN_EVENT_TABLE( CAMCompletionErrorPage, wxWizardPage )

////@begin CAMCompletionErrorPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMCompletionErrorPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CAMCompletionErrorPage::OnCancel )

////@end CAMCompletionErrorPage event table entries

END_EVENT_TABLE()

/*!
 * CAMCompletionErrorPage constructors
 */

CAMCompletionErrorPage::CAMCompletionErrorPage( )
{
}

CAMCompletionErrorPage::CAMCompletionErrorPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CCompletionErrorPage creator
 */

bool CAMCompletionErrorPage::Create( wxWizard* parent )
{
////@begin CAMCompletionErrorPage member initialisation
////@end CAMCompletionErrorPage member initialisation

////@begin CAMCompletionErrorPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMCompletionErrorPage creation
    return TRUE;
}

/*!
 * Control creation for CCompletionErrorPage
 */

void CAMCompletionErrorPage::CreateControls()
{    
////@begin CAMCompletionErrorPage content construction
    CAMCompletionErrorPage* itemWizardPage65 = this;

    wxBoxSizer* itemBoxSizer66 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage65->SetSizer(itemBoxSizer66);

    wxStaticText* itemStaticText67 = new wxStaticText;
    itemStaticText67->Create( itemWizardPage65, wxID_STATIC, _("Failed to attach to account manager"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText67->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer66->Add(itemStaticText67, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer66->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText69 = new wxStaticText;
    itemStaticText69->Create( itemWizardPage65, wxID_STATIC, _("Click Finish to close."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer66->Add(itemStaticText69, 0, wxALIGN_LEFT|wxALL, 5);

////@end CAMCompletionErrorPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE
 */

void CAMCompletionErrorPage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE in CCompletionErrorPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE in CCompletionErrorPage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE
 */

void CAMCompletionErrorPage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE in CCompletionErrorPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE in CCompletionErrorPage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMCompletionErrorPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMCompletionErrorPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMCompletionErrorPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMCompletionErrorPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMCompletionErrorPage bitmap retrieval
    return wxNullBitmap;
////@end CAMCompletionErrorPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMCompletionErrorPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMCompletionErrorPage icon retrieval
    return wxNullIcon;
////@end CAMCompletionErrorPage icon retrieval
}

/*!
 * CAMErrAccountManagerNotDetectedPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMErrAccountManagerNotDetectedPage, wxWizardPage )

/*!
 * CAMErrAccountManagerNotDetectedPage event table definition
 */

BEGIN_EVENT_TABLE( CAMErrAccountManagerNotDetectedPage, wxWizardPage )

////@begin CAMErrAccountManagerNotDetectedPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMErrAccountManagerNotDetectedPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CAMErrAccountManagerNotDetectedPage::OnCancel )

////@end CAMErrAccountManagerNotDetectedPage event table entries

END_EVENT_TABLE()

/*!
 * CAMErrAccountManagerNotDetectedPage constructors
 */

CAMErrAccountManagerNotDetectedPage::CAMErrAccountManagerNotDetectedPage( )
{
}

CAMErrAccountManagerNotDetectedPage::CAMErrAccountManagerNotDetectedPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrProjectNotDetectedPage creator
 */

bool CAMErrAccountManagerNotDetectedPage::Create( wxWizard* parent )
{
////@begin CAMErrAccountManagerNotDetectedPage member initialisation
////@end CAMErrAccountManagerNotDetectedPage member initialisation

////@begin CAMErrAccountManagerNotDetectedPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMErrAccountManagerNotDetectedPage creation
    return TRUE;
}

/*!
 * Control creation for CErrProjectNotDetectedPage
 */

void CAMErrAccountManagerNotDetectedPage::CreateControls()
{    
////@begin CAMErrAccountManagerNotDetectedPage content construction
    CAMErrAccountManagerNotDetectedPage* itemWizardPage70 = this;

    wxBoxSizer* itemBoxSizer71 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage70->SetSizer(itemBoxSizer71);

    wxStaticText* itemStaticText72 = new wxStaticText;
    itemStaticText72->Create( itemWizardPage70, wxID_STATIC, _("Account manager not found"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText72->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer71->Add(itemStaticText72, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer71->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText74 = new wxStaticText;
    itemStaticText74->Create( itemWizardPage70, wxID_STATIC, _("The URL you supplied is not that of a BOINC-based account\nmanager."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer71->Add(itemStaticText74, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText75 = new wxStaticText;
    itemStaticText75->Create( itemWizardPage70, wxID_STATIC, _("Please check the URL and try again."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer71->Add(itemStaticText75, 0, wxALIGN_LEFT|wxALL, 5);

////@end CAMErrAccountManagerNotDetectedPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTNOTDETECTEDPAGE
 */

void CAMErrAccountManagerNotDetectedPage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTNOTDETECTEDPAGE in CErrProjectNotDetectedPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTNOTDETECTEDPAGE in CErrProjectNotDetectedPage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTNOTDETECTEDPAGE
 */

void CAMErrAccountManagerNotDetectedPage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTNOTDETECTEDPAGE in CErrProjectNotDetectedPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTNOTDETECTEDPAGE in CErrProjectNotDetectedPage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMErrAccountManagerNotDetectedPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMErrAccountManagerNotDetectedPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMErrAccountManagerNotDetectedPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMErrAccountManagerNotDetectedPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMErrAccountManagerNotDetectedPage bitmap retrieval
    return wxNullBitmap;
////@end CAMErrAccountManagerNotDetectedPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMErrAccountManagerNotDetectedPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMErrAccountManagerNotDetectedPage icon retrieval
    return wxNullIcon;
////@end CAMErrAccountManagerNotDetectedPage icon retrieval
}

/*!
 * CAMErrAccountManagerUnavailablePage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMErrAccountManagerUnavailablePage, wxWizardPage )

/*!
 * CAMErrAccountManagerUnavailablePage event table definition
 */

BEGIN_EVENT_TABLE( CAMErrAccountManagerUnavailablePage, wxWizardPage )

////@begin CAMErrAccountManagerUnavailablePage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMErrAccountManagerUnavailablePage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CAMErrAccountManagerUnavailablePage::OnCancel )

////@end CAMErrAccountManagerUnavailablePage event table entries

END_EVENT_TABLE()

/*!
 * CAMErrAccountManagerUnavailablePage constructors
 */

CAMErrAccountManagerUnavailablePage::CAMErrAccountManagerUnavailablePage( )
{
}

CAMErrAccountManagerUnavailablePage::CAMErrAccountManagerUnavailablePage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrProjectUnavailablePage creator
 */

bool CAMErrAccountManagerUnavailablePage::Create( wxWizard* parent )
{
////@begin CAMErrAccountManagerUnavailablePage member initialisation
////@end CAMErrAccountManagerUnavailablePage member initialisation

////@begin CAMErrAccountManagerUnavailablePage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMErrAccountManagerUnavailablePage creation
    return TRUE;
}

/*!
 * Control creation for CErrProjectUnavailablePage
 */

void CAMErrAccountManagerUnavailablePage::CreateControls()
{    
////@begin CAMErrAccountManagerUnavailablePage content construction
    CAMErrAccountManagerUnavailablePage* itemWizardPage76 = this;

    wxBoxSizer* itemBoxSizer77 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage76->SetSizer(itemBoxSizer77);

    wxStaticText* itemStaticText78 = new wxStaticText;
    itemStaticText78->Create( itemWizardPage76, wxID_STATIC, _("Account manager temporarily unavailable"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText78->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer77->Add(itemStaticText78, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer77->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText80 = new wxStaticText;
    itemStaticText80->Create( itemWizardPage76, wxID_STATIC, _("The account manager is currently unavailable.\n\nPlease try again later."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer77->Add(itemStaticText80, 0, wxALIGN_LEFT|wxALL, 5);

////@end CAMErrAccountManagerUnavailablePage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTUNAVAILABLEPAGE
 */

void CAMErrAccountManagerUnavailablePage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTUNAVAILABLEPAGE in CErrProjectUnavailablePage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTUNAVAILABLEPAGE in CErrProjectUnavailablePage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTUNAVAILABLEPAGE
 */

void CAMErrAccountManagerUnavailablePage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTUNAVAILABLEPAGE in CErrProjectUnavailablePage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTUNAVAILABLEPAGE in CErrProjectUnavailablePage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMErrAccountManagerUnavailablePage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMErrAccountManagerUnavailablePage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMErrAccountManagerUnavailablePage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMErrAccountManagerUnavailablePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMErrAccountManagerUnavailablePage bitmap retrieval
    return wxNullBitmap;
////@end CAMErrAccountManagerUnavailablePage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMErrAccountManagerUnavailablePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMErrAccountManagerUnavailablePage icon retrieval
    return wxNullIcon;
////@end CAMErrAccountManagerUnavailablePage icon retrieval
}

/*!
 * CAMErrNoInternetConnectionPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMErrNoInternetConnectionPage, wxWizardPage )

/*!
 * CAMErrNoInternetConnectionPage event table definition
 */

BEGIN_EVENT_TABLE( CAMErrNoInternetConnectionPage, wxWizardPage )

////@begin CAMErrNoInternetConnectionPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMErrNoInternetConnectionPage::OnPageChanged )
    EVT_WIZARD_CANCEL( -1, CAMErrNoInternetConnectionPage::OnCancel )

////@end CAMErrNoInternetConnectionPage event table entries

END_EVENT_TABLE()

/*!
 * CAMErrNoInternetConnectionPage constructors
 */

CAMErrNoInternetConnectionPage::CAMErrNoInternetConnectionPage( )
{
}

CAMErrNoInternetConnectionPage::CAMErrNoInternetConnectionPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrNoInternetConnectionPage creator
 */

bool CAMErrNoInternetConnectionPage::Create( wxWizard* parent )
{
////@begin CAMErrNoInternetConnectionPage member initialisation
////@end CAMErrNoInternetConnectionPage member initialisation

////@begin CAMErrNoInternetConnectionPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMErrNoInternetConnectionPage creation
    return TRUE;
}

/*!
 * Control creation for CErrNoInternetConnectionPage
 */

void CAMErrNoInternetConnectionPage::CreateControls()
{    
////@begin CAMErrNoInternetConnectionPage content construction
    CAMErrNoInternetConnectionPage* itemWizardPage81 = this;

    wxBoxSizer* itemBoxSizer82 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage81->SetSizer(itemBoxSizer82);

    wxStaticText* itemStaticText83 = new wxStaticText;
    itemStaticText83->Create( itemWizardPage81, wxID_STATIC, _("No Internet connection"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText83->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer82->Add(itemStaticText83, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer82->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText85 = new wxStaticText;
    itemStaticText85->Create( itemWizardPage81, wxID_STATIC, _("Please connect to the Internet and try again."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer82->Add(itemStaticText85, 0, wxALIGN_LEFT|wxALL, 5);

////@end CAMErrNoInternetConnectionPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNOINTERNETCONNECTIONPAGE
 */

void CAMErrNoInternetConnectionPage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNOINTERNETCONNECTIONPAGE in CErrNoInternetConnectionPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNOINTERNETCONNECTIONPAGE in CErrNoInternetConnectionPage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRNOINTERNETCONNECTIONPAGE
 */

void CAMErrNoInternetConnectionPage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_ERRNOINTERNETCONNECTIONPAGE in CErrNoInternetConnectionPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_ERRNOINTERNETCONNECTIONPAGE in CErrNoInternetConnectionPage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMErrNoInternetConnectionPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMErrNoInternetConnectionPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMErrNoInternetConnectionPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMErrNoInternetConnectionPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMErrNoInternetConnectionPage bitmap retrieval
    return wxNullBitmap;
////@end CAMErrNoInternetConnectionPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMErrNoInternetConnectionPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMErrNoInternetConnectionPage icon retrieval
    return wxNullIcon;
////@end CAMErrNoInternetConnectionPage icon retrieval
}

/*!
 * CAMErrProxyPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMErrProxyPage, wxWizardPage )

/*!
 * CAMErrProxyPage event table definition
 */

BEGIN_EVENT_TABLE( CAMErrProxyPage, wxWizardPage )

////@begin CAMErrProxyPage event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, CAMErrProxyPage::OnPageChanged )
    EVT_WIZARD_PAGE_CHANGING( -1, CAMErrProxyPage::OnPageChanging )
    EVT_WIZARD_CANCEL( -1, CAMErrProxyPage::OnCancel )

////@end CAMErrProxyPage event table entries

END_EVENT_TABLE()

/*!
 * CAMErrProxyPage constructors
 */

CAMErrProxyPage::CAMErrProxyPage( )
{
}

CAMErrProxyPage::CAMErrProxyPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrProxyPage creator
 */

bool CAMErrProxyPage::Create( wxWizard* parent )
{
////@begin CAMErrProxyPage member initialisation
    m_ProxyHTTPServerStaticCtrl = NULL;
    m_ProxyHTTPServerCtrl = NULL;
    m_ProxyHTTPPortStaticCtrl = NULL;
    m_ProxyHTTPPortCtrl = NULL;
    m_ProxyHTTPUsernameStaticCtrl = NULL;
    m_ProxyHTTPUsernameCtrl = NULL;
    m_ProxyHTTPPasswordStaticCtrl = NULL;
    m_ProxyHTTPPasswordCtrl = NULL;
    m_ProxyHTTPAutodetectCtrl = NULL;
    m_ProxySOCKSServerStaticCtrl = NULL;
    m_ProxySOCKSServerCtrl = NULL;
    m_ProxySOCKSPortStaticCtrl = NULL;
    m_ProxySOCKSPortCtrl = NULL;
    m_ProxySOCKSUsernameStaticCtrl = NULL;
    m_ProxySOCKSUsernameCtrl = NULL;
    m_ProxySOCKSPasswordStaticCtrl = NULL;
    m_ProxySOCKSPasswordCtrl = NULL;
////@end CAMErrProxyPage member initialisation

////@begin CAMErrProxyPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMErrProxyPage creation
    return TRUE;
}

/*!
 * Control creation for CErrProxyPage
 */

void CAMErrProxyPage::CreateControls()
{    
////@begin CAMErrProxyPage content construction
    CAMErrProxyPage* itemWizardPage86 = this;

    wxBoxSizer* itemBoxSizer87 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage86->SetSizer(itemBoxSizer87);

    wxStaticText* itemStaticText88 = new wxStaticText;
    itemStaticText88->Create( itemWizardPage86, wxID_STATIC, _("Proxy configuration"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText88->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer87->Add(itemStaticText88, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer87->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer90Static = new wxStaticBox(itemWizardPage86, wxID_ANY, _("HTTP proxy"));
    wxStaticBoxSizer* itemStaticBoxSizer90 = new wxStaticBoxSizer(itemStaticBoxSizer90Static, wxVERTICAL);
    itemBoxSizer87->Add(itemStaticBoxSizer90, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer91 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer91->AddGrowableCol(1);
    itemStaticBoxSizer90->Add(itemFlexGridSizer91, 0, wxGROW|wxALL, 2);

    m_ProxyHTTPServerStaticCtrl = new wxStaticText;
    m_ProxyHTTPServerStaticCtrl->Create( itemWizardPage86, ID_PROXYHTTPSERVERSTATICCTRL, _("Server:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer91->Add(m_ProxyHTTPServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer93 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer93->AddGrowableCol(0);
    itemFlexGridSizer91->Add(itemFlexGridSizer93, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ProxyHTTPServerCtrl = new wxTextCtrl;
    m_ProxyHTTPServerCtrl->Create( itemWizardPage86, ID_PROXYHTTPSERVERCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer93->Add(m_ProxyHTTPServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPortStaticCtrl = new wxStaticText;
    m_ProxyHTTPPortStaticCtrl->Create( itemWizardPage86, ID_PROXYHTTPPORTSTATICCTRL, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer93->Add(m_ProxyHTTPPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPortCtrl = new wxTextCtrl;
    m_ProxyHTTPPortCtrl->Create( itemWizardPage86, ID_PROXYHTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer93->Add(m_ProxyHTTPPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPUsernameStaticCtrl = new wxStaticText;
    m_ProxyHTTPUsernameStaticCtrl->Create( itemWizardPage86, ID_PROXYHTTPUSERNAMESTATICCTRL, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer91->Add(m_ProxyHTTPUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPUsernameCtrl = new wxTextCtrl;
    m_ProxyHTTPUsernameCtrl->Create( itemWizardPage86, ID_PROXYHTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer91->Add(m_ProxyHTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPasswordStaticCtrl = new wxStaticText;
    m_ProxyHTTPPasswordStaticCtrl->Create( itemWizardPage86, ID_PROXYHTTPPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer91->Add(m_ProxyHTTPPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPasswordCtrl = new wxTextCtrl;
    m_ProxyHTTPPasswordCtrl->Create( itemWizardPage86, ID_PROXYHTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer91->Add(m_ProxyHTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPAutodetectCtrl = new wxButton;
    m_ProxyHTTPAutodetectCtrl->Create( itemWizardPage86, ID_PROXYHTTPAUTODETECTCTRL, _("Autodetect"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer90->Add(m_ProxyHTTPAutodetectCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxStaticBox* itemStaticBoxSizer102Static = new wxStaticBox(itemWizardPage86, wxID_ANY, _("SOCKS proxy"));
    wxStaticBoxSizer* itemStaticBoxSizer102 = new wxStaticBoxSizer(itemStaticBoxSizer102Static, wxVERTICAL);
    itemBoxSizer87->Add(itemStaticBoxSizer102, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer103 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer103->AddGrowableCol(1);
    itemStaticBoxSizer102->Add(itemFlexGridSizer103, 0, wxGROW|wxALL, 2);

    m_ProxySOCKSServerStaticCtrl = new wxStaticText;
    m_ProxySOCKSServerStaticCtrl->Create( itemWizardPage86, ID_PROXYSOCKSSERVERSTATICCTRL, _("Server:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer103->Add(m_ProxySOCKSServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer105 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer105->AddGrowableCol(0);
    itemFlexGridSizer103->Add(itemFlexGridSizer105, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ProxySOCKSServerCtrl = new wxTextCtrl;
    m_ProxySOCKSServerCtrl->Create( itemWizardPage86, ID_PROXYSOCKSSERVERCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer105->Add(m_ProxySOCKSServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPortStaticCtrl = new wxStaticText;
    m_ProxySOCKSPortStaticCtrl->Create( itemWizardPage86, ID_PROXYSOCKSPORTSTATICCTRL, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer105->Add(m_ProxySOCKSPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPortCtrl = new wxTextCtrl;
    m_ProxySOCKSPortCtrl->Create( itemWizardPage86, ID_PROXYSOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer105->Add(m_ProxySOCKSPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSUsernameStaticCtrl = new wxStaticText;
    m_ProxySOCKSUsernameStaticCtrl->Create( itemWizardPage86, ID_PROXYSOCKSUSERNAMESTATICCTRL, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer103->Add(m_ProxySOCKSUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSUsernameCtrl = new wxTextCtrl;
    m_ProxySOCKSUsernameCtrl->Create( itemWizardPage86, ID_PROXYSOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer103->Add(m_ProxySOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPasswordStaticCtrl = new wxStaticText;
    m_ProxySOCKSPasswordStaticCtrl->Create( itemWizardPage86, ID_PROXYSOCKSPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer103->Add(m_ProxySOCKSPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPasswordCtrl = new wxTextCtrl;
    m_ProxySOCKSPasswordCtrl->Create( itemWizardPage86, ID_PROXYSOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer103->Add(m_ProxySOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    // Set validators
    m_ProxyHTTPServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPServer) );
    m_ProxyHTTPPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxyHTTPPort) );
    m_ProxyHTTPUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPUsername) );
    m_ProxyHTTPPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPPassword) );
    m_ProxySOCKSServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSServer) );
    m_ProxySOCKSPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxySOCKSPort) );
    m_ProxySOCKSUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSUsername) );
    m_ProxySOCKSPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSPassword) );
////@end CAMErrProxyPage content construction
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYPAGE
 */

void CAMErrProxyPage::OnPageChanged( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYPAGE in CErrProxyPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYPAGE in CErrProxyPage. 
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ERRPROXYPAGE
 */

void CAMErrProxyPage::OnPageChanging( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ERRPROXYPAGE in CErrProxyPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ERRPROXYPAGE in CErrProxyPage. 
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYPAGE
 */

void CAMErrProxyPage::OnCancel( wxWizardEvent& event )
{
////@begin wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYPAGE in CErrProxyPage.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYPAGE in CErrProxyPage. 
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMErrProxyPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMErrProxyPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMErrProxyPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMErrProxyPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMErrProxyPage bitmap retrieval
    return wxNullBitmap;
////@end CAMErrProxyPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMErrProxyPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMErrProxyPage icon retrieval
    return wxNullIcon;
////@end CAMErrProxyPage icon retrieval
}

/*!
 * CAMErrRefCountPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAMErrRefCountPage, wxWizardPage )

/*!
 * CAMErrRefCountPage event table definition
 */

BEGIN_EVENT_TABLE( CAMErrRefCountPage, wxWizardPage )

////@begin CAMErrRefCountPage event table entries
////@end CAMErrRefCountPage event table entries

END_EVENT_TABLE()

/*!
 * CAMErrRefCountPage constructors
 */

CAMErrRefCountPage::CAMErrRefCountPage( )
{
}

CAMErrRefCountPage::CAMErrRefCountPage( wxWizard* parent )
{
    Create( parent );
}

/*!
 * CErrRefCountPage creator
 */

bool CAMErrRefCountPage::Create( wxWizard* parent )
{
////@begin CAMErrRefCountPage member initialisation
////@end CAMErrRefCountPage member initialisation

////@begin CAMErrRefCountPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAMErrRefCountPage creation
    return TRUE;
}

/*!
 * Control creation for CErrRefCountPage
 */

void CAMErrRefCountPage::CreateControls()
{    
////@begin CAMErrRefCountPage content construction
    CAMErrRefCountPage* itemWizardPage113 = this;

    wxBoxSizer* itemBoxSizer114 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage113->SetSizer(itemBoxSizer114);

    wxStaticText* itemStaticText115 = new wxStaticText;
    itemStaticText115->Create( itemWizardPage113, wxID_STATIC, _("Ref Count Page"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText115->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer114->Add(itemStaticText115, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText116 = new wxStaticText;
    itemStaticText116->Create( itemWizardPage113, wxID_STATIC, _("This page should never be used in the wizard itself."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer114->Add(itemStaticText116, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer114->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText118 = new wxStaticText;
    itemStaticText118->Create( itemWizardPage113, wxID_STATIC, _("This page just increases the refcount of various bitmap resources\nso that DialogBlocks doesn't nuke the refences to them."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer114->Add(itemStaticText118, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer119 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer114->Add(itemBoxSizer119, 0, wxALIGN_LEFT|wxALL, 5);

    wxBitmap itemStaticBitmap120Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress01.xpm")));
    wxStaticBitmap* itemStaticBitmap120 = new wxStaticBitmap;
    itemStaticBitmap120->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap120Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap120->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap120, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap121Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress02.xpm")));
    wxStaticBitmap* itemStaticBitmap121 = new wxStaticBitmap;
    itemStaticBitmap121->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap121Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap121->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap121, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap122Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress03.xpm")));
    wxStaticBitmap* itemStaticBitmap122 = new wxStaticBitmap;
    itemStaticBitmap122->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap122Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap122->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap122, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap123Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress04.xpm")));
    wxStaticBitmap* itemStaticBitmap123 = new wxStaticBitmap;
    itemStaticBitmap123->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap123Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap123->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap123, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap124Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress05.xpm")));
    wxStaticBitmap* itemStaticBitmap124 = new wxStaticBitmap;
    itemStaticBitmap124->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap124Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap124->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap124, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap125Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress06.xpm")));
    wxStaticBitmap* itemStaticBitmap125 = new wxStaticBitmap;
    itemStaticBitmap125->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap125Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap125->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap125, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap126Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress07.xpm")));
    wxStaticBitmap* itemStaticBitmap126 = new wxStaticBitmap;
    itemStaticBitmap126->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap126Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap126->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap126, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap127Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress08.xpm")));
    wxStaticBitmap* itemStaticBitmap127 = new wxStaticBitmap;
    itemStaticBitmap127->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap127Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap127->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap127, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap128Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress09.xpm")));
    wxStaticBitmap* itemStaticBitmap128 = new wxStaticBitmap;
    itemStaticBitmap128->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap128Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap128->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap128, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap129Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress10.xpm")));
    wxStaticBitmap* itemStaticBitmap129 = new wxStaticBitmap;
    itemStaticBitmap129->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap129Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap129->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap129, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap130Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress11.xpm")));
    wxStaticBitmap* itemStaticBitmap130 = new wxStaticBitmap;
    itemStaticBitmap130->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap130Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap130->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap130, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmap itemStaticBitmap131Bitmap(itemWizardPage113->GetBitmapResource(wxT("res/wizprogress12.xpm")));
    wxStaticBitmap* itemStaticBitmap131 = new wxStaticBitmap;
    itemStaticBitmap131->Create( itemWizardPage113, wxID_STATIC, itemStaticBitmap131Bitmap, wxDefaultPosition, wxSize(184, 48), 0 );
    itemStaticBitmap131->Show(FALSE);
    itemBoxSizer119->Add(itemStaticBitmap131, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CAMErrRefCountPage content construction
}

/*!
 * Gets the previous page.
 */

wxWizardPage* CAMErrRefCountPage::GetPrev() const
{
    // TODO: return the previous page
    return NULL;
}

/*!
 * Gets the next page.
 */

wxWizardPage* CAMErrRefCountPage::GetNext() const
{
    // TODO: return the next page
    return NULL;
}

/*!
 * Should we show tooltips?
 */

bool CAMErrRefCountPage::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAMErrRefCountPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAMErrRefCountPage bitmap retrieval
    if (name == wxT("res/wizprogress01.xpm"))
    {
        wxBitmap bitmap(wizprogress01_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress02.xpm"))
    {
        wxBitmap bitmap(wizprogress02_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress03.xpm"))
    {
        wxBitmap bitmap(wizprogress03_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress04.xpm"))
    {
        wxBitmap bitmap(wizprogress04_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress05.xpm"))
    {
        wxBitmap bitmap(wizprogress05_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress06.xpm"))
    {
        wxBitmap bitmap(wizprogress06_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress07.xpm"))
    {
        wxBitmap bitmap(wizprogress07_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress08.xpm"))
    {
        wxBitmap bitmap(wizprogress08_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress09.xpm"))
    {
        wxBitmap bitmap(wizprogress09_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress10.xpm"))
    {
        wxBitmap bitmap(wizprogress10_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress11.xpm"))
    {
        wxBitmap bitmap(wizprogress11_xpm);
        return bitmap;
    }
    else if (name == wxT("res/wizprogress12.xpm"))
    {
        wxBitmap bitmap(wizprogress12_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CAMErrRefCountPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAMErrRefCountPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAMErrRefCountPage icon retrieval
    return wxNullIcon;
////@end CAMErrRefCountPage icon retrieval
}
const char *BOINC_RCSID_b0f884ae03="$Id$";

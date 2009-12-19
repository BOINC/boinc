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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "AccountManagerInfoPage.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "str_util.h"
#include "error_numbers.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "ValidateURL.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "AccountManagerInfoPage.h"
#include "ProjectListCtrl.h"


/*!
 * CAccountManagerInfoPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAccountManagerInfoPage, wxWizardPageEx )

/*!
 * CAccountManagerInfoPage event table definition
 */

BEGIN_EVENT_TABLE( CAccountManagerInfoPage, wxWizardPageEx )

////@begin CAccountManagerInfoPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CAccountManagerInfoPage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CAccountManagerInfoPage::OnPageChanging )
    EVT_PROJECTLIST_ITEM_CHANGE( CAccountManagerInfoPage::OnProjectItemChange )
    EVT_PROJECTLIST_ITEM_DISPLAY( CAccountManagerInfoPage::OnProjectItemDisplay )
    EVT_WIZARDEX_CANCEL( -1, CAccountManagerInfoPage::OnCancel )
////@end CAccountManagerInfoPage event table entries

END_EVENT_TABLE()


/*!
 * CAccountManagerInfoPage constructors
 */

CAccountManagerInfoPage::CAccountManagerInfoPage( )
{
}

CAccountManagerInfoPage::CAccountManagerInfoPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}


/*!
 * CAccountManagerInfoPage creator
 */

bool CAccountManagerInfoPage::Create( CBOINCBaseWizard* parent )
{
////@begin CAccountManagerInfoPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDescriptionStaticCtrl = NULL;
    m_pProjectListCtrl = NULL;
    m_pProjectUrlStaticCtrl = NULL;
    m_pProjectUrlCtrl = NULL;
////@end CAccountManagerInfoPage member initialisation
    m_bAccountManagerListPopulated = false;

////@begin CAccountManagerInfoPage creation
    wxWizardPageEx::Create( parent, ID_ACCOUNTMANAGERINFOPAGE );

    CreateControls();
    GetSizer()->Fit(this);
////@end CAccountManagerInfoPage creation
    return TRUE;
}


/*!
 * Control creation for CAccountManagerInfoPage
 */

void CAccountManagerInfoPage::CreateControls()
{    
////@begin CAccountManagerInfoPage content construction
    CAccountManagerInfoPage* itemWizardPage23 = this;

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage23->SetSizer(itemBoxSizer24);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    m_pProjectListCtrl = new CProjectListCtrl;
    m_pProjectListCtrl->Create( itemWizardPage23 );
    itemFlexGridSizer3->Add(m_pProjectListCtrl, 0, wxGROW|wxRIGHT, 10);

    wxFlexGridSizer* itemFlexGridSizer11 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer11->AddGrowableRow(0);
    itemFlexGridSizer11->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer11, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer11->Add(itemBoxSizer22, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer14 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer14->AddGrowableCol(1);
    itemBoxSizer24->Add(itemFlexGridSizer14, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT, 10);

    m_pProjectUrlStaticCtrl = new wxStaticText;
    m_pProjectUrlStaticCtrl->Create( itemWizardPage23, ID_PROJECTURLSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer14->Add(m_pProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectUrlCtrl = new wxTextCtrl;
    m_pProjectUrlCtrl->Create( itemWizardPage23, ID_PROJECTURLCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer14->Add(m_pProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_pProjectUrlCtrl->SetValidator( CValidateURL( & m_strProjectURL ) );
    
#ifdef __WXMAC__
    //Accessibility
    HIViewRef listView = (HIViewRef)m_pProjectListCtrl->GetHandle();
    HIObjectRef   theObject = (HIObjectRef)HIViewGetSuperview(listView);
    HIObjectSetAccessibilityIgnored(theObject, true);
#endif
    ////@end CAccountManagerInfoPage content construction
}


/*!
 * Gets the previous page.
 */

wxWizardPageEx* CAccountManagerInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}


/*!
 * Gets the next page.
 */

wxWizardPageEx* CAccountManagerInfoPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROPERTIESPAGE);
    }
    return NULL;
}


/*!
 * Should we show tooltips?
 */

bool CAccountManagerInfoPage::ShowToolTips()
{
    return TRUE;
}


/*!
 * Get bitmap resources
 */

wxBitmap CAccountManagerInfoPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CAccountManagerInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CAccountManagerInfoPage bitmap retrieval
}


/*!
 * Get icon resources
 */

wxIcon CAccountManagerInfoPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CAccountManagerInfoPage icon retrieval
    return wxNullIcon;
////@end CAccountManagerInfoPage icon retrieval
}


/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
 */

void CAccountManagerInfoPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;
    wxLogTrace(wxT("Function Start/End"), wxT("CAccountManagerInfoPage::OnPageChanged - Function Begin"));

    unsigned int      i;
    ALL_PROJECTS_LIST pl;
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pProjectUrlStaticCtrl);
    wxASSERT(m_pProjectUrlCtrl);


    m_pTitleStaticCtrl->SetLabel(
        _("Choose an account manager")
    );
    m_pDescriptionStaticCtrl->SetLabel(
        _("To choose an account manager, click its name or \ntype its URL below.")
    );
    m_pProjectUrlStaticCtrl->SetLabel(
        _("Account Manager &URL:")
    );

    // Populate the virtual list control with project information
    //
    if (!m_bAccountManagerListPopulated) {
        pDoc->rpc.get_all_projects_list(pl);
        for (i=0; i<pl.account_managers.size(); i++) {
            wxLogTrace(
                wxT("Function Status"),
                wxT("CAccountManagerInfoPage::OnPageChanged - Name: '%s', URL: '%s', Supported: '%d'"),
                wxString(pl.account_managers[i]->name.c_str(), wxConvUTF8).c_str(),
                wxString(pl.account_managers[i]->url.c_str(), wxConvUTF8).c_str(),
                true
            );

            m_pProjectListCtrl->Append(
                wxString(pl.account_managers[i]->url.c_str(), wxConvUTF8),
                wxString(pl.account_managers[i]->name.c_str(), wxConvUTF8),
                wxString(pl.account_managers[i]->image.c_str(), wxConvUTF8),
                wxString(pl.account_managers[i]->description.c_str(), wxConvUTF8),
                false,
                false,
                false,
                true
            );
        }
        m_bAccountManagerListPopulated = true;
    }

    Layout();
    FitInside();
    m_pProjectListCtrl->SetFocus();

    wxLogTrace(wxT("Function Start/End"), wxT("CAccountManagerInfoPage::OnPageChanged - Function End"));
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
 */

void CAccountManagerInfoPage::OnPageChanging( wxWizardExEvent& event ) {
    event.Skip();
}

/*!
 * wxEVT_PROJECTLIST_ITEM_CHANGE event handler for ID_PROJECTSELECTIONCTRL
 */

void CAccountManagerInfoPage::OnProjectItemChange( ProjectListCtrlEvent& event ) {
    SetProjectURL( event.GetURL() );
    SetProjectSupported( event.IsSupported() );
    TransferDataToWindow();
}

/*!
 * wxEVT_PROJECTLIST_ITEM_DISPLAY event handler for ID_PROJECTSELECTIONCTRL
 */

void CAccountManagerInfoPage::OnProjectItemDisplay( ProjectListCtrlEvent& event ) {
    wxLaunchDefaultBrowser( event.GetURL() );
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
 */

void CAccountManagerInfoPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

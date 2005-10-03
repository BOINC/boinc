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
#pragma implementation "CompletionPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "CompletionPage.h"
#include "AccountInfoPage.h"


/*!
 * CCompletionPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CCompletionPage, wxWizardPageEx )
 
/*!
 * CCompletionPage event table definition
 */
 
BEGIN_EVENT_TABLE( CCompletionPage, wxWizardPageEx )
 
////@begin CCompletionPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CCompletionPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CCompletionPage::OnCancel )
    EVT_WIZARDEX_FINISHED( ID_COMPLETIONPAGE, CCompletionPage::OnFinished )

////@end CCompletionPage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CCompletionPage constructors
 */
 
CCompletionPage::CCompletionPage( )
{
}
 
CCompletionPage::CCompletionPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CCompletionPage creator
 */
 
bool CCompletionPage::Create( CBOINCBaseWizard* parent )
{
////@begin CCompletionPage member initialisation
    m_CompletionMessage = NULL;
////@end CCompletionPage member initialisation
 
////@begin CCompletionPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CCompletionPage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CCompletionPage
 */
 
void CCompletionPage::CreateControls()
{    
////@begin CCompletionPage content construction
    CCompletionPage* itemWizardPage79 = this;

    wxBoxSizer* itemBoxSizer80 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage79->SetSizer(itemBoxSizer80);

    wxStaticText* itemStaticText81 = new wxStaticText;
    wxStaticText* itemStaticText82 = new wxStaticText;
    m_CompletionMessage = new wxStaticText;
    if (IS_ATTACHTOPROJECTWIZARD()) {
        itemStaticText81->Create( itemWizardPage79, wxID_STATIC, _("Attached to project"), wxDefaultPosition, wxDefaultSize, 0 );
        itemStaticText81->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
        itemBoxSizer80->Add(itemStaticText81, 0, wxALIGN_LEFT|wxALL, 5);

        itemStaticText82->Create( itemWizardPage79, wxID_STATIC, _("You are now successfully attached to this project."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer80->Add(itemStaticText82, 0, wxALIGN_LEFT|wxALL, 5);

        itemBoxSizer80->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

        m_CompletionMessage->Create( itemWizardPage79, wxID_STATIC, _("When you click Finish, your web browser will go to a page where\nyou can set your account name and preferences."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer80->Add(m_CompletionMessage, 0, wxALIGN_LEFT|wxALL, 5);
    }

    if (IS_ACCOUNTMANAGERWIZARD()) {
        itemStaticText81->Create( itemWizardPage79, wxID_STATIC, _("Attached to account manager"), wxDefaultPosition, wxDefaultSize, 0 );
        itemStaticText81->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
        itemBoxSizer80->Add(itemStaticText81, 0, wxALIGN_LEFT|wxALL, 5);

        itemStaticText82->Create( itemWizardPage79, wxID_STATIC, _("You are now successfully attached to this account manager."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer80->Add(itemStaticText82, 0, wxALIGN_LEFT|wxALL, 5);

        itemBoxSizer80->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

        m_CompletionMessage->Create( itemWizardPage79, wxID_STATIC, _("When you click Finish, your web browser will go to a page where\nyou can set your account name and preferences."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer80->Add(m_CompletionMessage, 0, wxALIGN_LEFT|wxALL, 5);
    }

////@end CCompletionPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CCompletionPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CCompletionPage::GetNext() const
{
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
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE
 */
 
void CCompletionPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    if (IS_ATTACHTOPROJECTWIZARD() && ((CWizardAttachProject*)GetParent())->m_AccountInfoPage->m_AccountCreateCtrl->GetValue()) {
        m_CompletionMessage->SetLabel(_("When you click Finish, your web browser will go to a page where\nyou can set your account name and preferences."));
    } else {
        m_CompletionMessage->SetLabel(_("Click Finish to close."));
    }
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE
 */
 
void CCompletionPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}
 
/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE
 */
 
void CCompletionPage::OnFinished( wxWizardExEvent& event ) {
    event.Skip();
}


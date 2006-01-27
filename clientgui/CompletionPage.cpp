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
    m_pCompletionTitle = NULL;
    m_pCompletionWelcome = NULL;
    m_pCompletionBrandedMessage = NULL;
    m_pCompletionMessage = NULL;
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

    m_pCompletionTitle = new wxStaticText;
    m_pCompletionTitle->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pCompletionTitle->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Verdana")));
    itemBoxSizer80->Add(m_pCompletionTitle, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionWelcome = new wxStaticText;
    m_pCompletionWelcome->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pCompletionWelcome->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE));
    itemBoxSizer80->Add(m_pCompletionWelcome, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionBrandedMessage = new wxStaticText;
    m_pCompletionBrandedMessage->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer80->Add(m_pCompletionBrandedMessage, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionMessage = new wxStaticText;
    m_pCompletionMessage->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer80->Add(m_pCompletionMessage, 0, wxALIGN_LEFT|wxALL, 5);
////@end CCompletionPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CCompletionPage::GetPrev() const
{
    return NULL;
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

    wxASSERT(m_pCompletionTitle);
    wxASSERT(m_pCompletionWelcome);
    wxASSERT(m_pCompletionBrandedMessage);
    wxASSERT(m_pCompletionMessage);

    if (IS_ATTACHTOPROJECTWIZARD()) {
        wxString strTitle;
        if (wxGetApp().GetBrand()->IsBranded() && 
            !wxGetApp().GetBrand()->GetAPWizardCompletionTitle().IsEmpty()) {
            strTitle = wxGetApp().GetBrand()->GetAPWizardCompletionTitle();
        } else {
            strTitle = _("Attached to project");
        }

        m_pCompletionTitle->SetLabel( strTitle );

        m_pCompletionWelcome->Hide();

        wxString strBrandedMessage;
        if (wxGetApp().GetBrand()->IsBranded() && 
            !wxGetApp().GetBrand()->GetAPWizardCompletionBrandedMessage().IsEmpty()) {
            strBrandedMessage = wxGetApp().GetBrand()->GetAPWizardCompletionBrandedMessage();
        } else {
            strBrandedMessage = _("You are now successfully attached to this project.");
        }

        m_pCompletionBrandedMessage->SetLabel( strBrandedMessage );

        wxString strMessage;
        if (wxGetApp().GetBrand()->IsBranded() && 
            !wxGetApp().GetBrand()->GetAPWizardCompletionMessage().IsEmpty()) {
            strMessage = wxGetApp().GetBrand()->GetAPWizardCompletionMessage();
        } else {
            strMessage = 
                _("When you click Finish, your web browser will go to a page where\n"
                  "you can set your account name and preferences.");
        }


        m_pCompletionMessage->SetLabel( strMessage );
    } else if (IS_ACCOUNTMANAGERWIZARD()) {
        wxString strTitle;
        if (wxGetApp().GetBrand()->IsBranded()) {
            // %s is the project name
            //    i.e. 'GridRepublic'
            strTitle.Printf(
                _("Attached to %s"),
                wxGetApp().GetBrand()->GetProjectName().c_str()
            );
        } else {
            strTitle = _("Attached to account manager");
        }

        m_pCompletionTitle->SetLabel( strTitle );

        if (wxGetApp().GetBrand()->IsBranded()) {
            // %s is the project name
            //    i.e. 'GridRepublic'
            wxString strWelcome;
            strWelcome.Printf(
                _("Welcome to %s!"),
                wxGetApp().GetBrand()->GetProjectName().c_str()
            );

            m_pCompletionWelcome->Show();
            m_pCompletionWelcome->SetLabel( strWelcome );
        }

        wxString strBrandedMessage;
        if (wxGetApp().GetBrand()->IsBranded()) {
            // 1st %s is the project name
            //    i.e. 'GridRepublic'
            // 2nd %s is the account manager success message
            strBrandedMessage.Printf(
                _("You are now successfully attached to the %s system.\n"
                  "%s"),
                wxGetApp().GetBrand()->GetProjectName().c_str(),
                wxGetApp().GetBrand()->GetAMWizardSuccessMessage().c_str()
            );
        } else {
            strBrandedMessage = _("You are now successfully attached to this account manager.");
        }

        m_pCompletionBrandedMessage->SetLabel( strBrandedMessage );

        m_pCompletionMessage->SetLabel(
            _("Click Finish to close.")
        );
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


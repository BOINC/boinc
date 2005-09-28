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
#pragma implementation "CompletionErrorPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "CompletionErrorPage.h"


/*!
 * CCompletionErrorPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CCompletionErrorPage, wxWizardPageEx )
 
/*!
 * CCompletionErrorPage event table definition
 */
 
BEGIN_EVENT_TABLE( CCompletionErrorPage, wxWizardPageEx )
 
////@begin CCompletionErrorPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CCompletionErrorPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CCompletionErrorPage::OnCancel )

////@end CCompletionErrorPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CCompletionErrorPage constructors
 */
 
CCompletionErrorPage::CCompletionErrorPage( )
{
}
 
CCompletionErrorPage::CCompletionErrorPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CAccountResultPage creator
 */
 
bool CCompletionErrorPage::Create( CBOINCBaseWizard* parent )
{
////@begin CCompletionErrorPage member initialisation
    m_ServerMessagesStaticBoxSizer = NULL;
    m_ServerMessages = NULL;
////@end CCompletionErrorPage member initialisation
 
////@begin CCompletionErrorPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CCompletionErrorPage creation

    return TRUE;
}
 
/*!
 * Control creation for CAccountResultPage
 */
 
void CCompletionErrorPage::CreateControls()
{    
////@begin CCompletionErrorPage content construction
    CCompletionErrorPage* itemWizardPage85 = this;

    wxBoxSizer* itemBoxSizer86 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage85->SetSizer(itemBoxSizer86);

    wxStaticText* itemStaticText87 = new wxStaticText;
    wxStaticText* itemStaticText89 = new wxStaticText;
    if (IS_ATTACHTOPROJECTWIZARD()) {
        itemStaticText87->Create( itemWizardPage85, wxID_STATIC, _("Failed to attach to project"), wxDefaultPosition, wxDefaultSize, 0 );
        itemStaticText87->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
        itemBoxSizer86->Add(itemStaticText87, 0, wxALIGN_LEFT|wxALL, 5);

        itemBoxSizer86->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

        itemStaticText89->Create( itemWizardPage85, wxID_STATIC, _("Click Finish to close."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer86->Add(itemStaticText89, 0, wxALIGN_LEFT|wxALL, 5);
    }

    if (IS_ACCOUNTMANAGERWIZARD()) {
        itemStaticText87->Create( itemWizardPage85, wxID_STATIC, _("Failed to attach to account manager"), wxDefaultPosition, wxDefaultSize, 0 );
        itemStaticText87->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
        itemBoxSizer86->Add(itemStaticText87, 0, wxALIGN_LEFT|wxALL, 5);

        itemBoxSizer86->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

        itemStaticText89->Create( itemWizardPage85, wxID_STATIC, _("Click Finish to close."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer86->Add(itemStaticText89, 0, wxALIGN_LEFT|wxALL, 5);
    }

    itemBoxSizer86->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemWizardPage85, wxID_ANY, _("Messages from server:"));
    m_ServerMessagesStaticBoxSizer = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer86->Add(m_ServerMessagesStaticBoxSizer, 0, wxGROW|wxALL, 5);

    m_ServerMessages = new wxStaticText;
    m_ServerMessages->Create( itemWizardPage85, wxID_STATIC, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_ServerMessagesStaticBoxSizer->Add(m_ServerMessages, 0, wxGROW|wxALL, 5);
////@end CCompletionErrorPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CCompletionErrorPage::GetPrev() const
{
    return NULL;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CCompletionErrorPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CCompletionErrorPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CCompletionErrorPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval

////@begin CCompletionErrorPage bitmap retrieval
    return wxNullBitmap;
////@end CCompletionErrorPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CCompletionErrorPage::GetIconResource( const wxString& name )
{
    // Icon retrieval

////@begin CCompletionErrorPage icon retrieval
    return wxNullIcon;
////@end CCompletionErrorPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE
 */
 
void CCompletionErrorPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    if (CHECK_CLOSINGINPROGRESS()) {
        m_ServerMessagesStaticBoxSizer->GetStaticBox()->Hide();
        m_ServerMessages->Hide();
    } else {
        m_ServerMessagesStaticBoxSizer->GetStaticBox()->Show();
        m_ServerMessages->Show();
    }
}
 
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE
 */
 
void CCompletionErrorPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


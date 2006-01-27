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
#pragma implementation "CompletionUpdatePage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "CompletionUpdatePage.h"
#include "AccountInfoPage.h"


/*!
 * CCompletionUpdatePage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CCompletionUpdatePage, wxWizardPageEx )
 
/*!
 * CCompletionUpdatePage event table definition
 */
 
BEGIN_EVENT_TABLE( CCompletionUpdatePage, wxWizardPageEx )
 
////@begin CCompletionUpdatePage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CCompletionUpdatePage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CCompletionUpdatePage::OnCancel )
    EVT_WIZARDEX_FINISHED( ID_COMPLETIONUPDATEPAGE, CCompletionUpdatePage::OnFinished )

////@end CCompletionUpdatePage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CCompletionUpdatePage constructors
 */
 
CCompletionUpdatePage::CCompletionUpdatePage( )
{
}
 
CCompletionUpdatePage::CCompletionUpdatePage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CCompletionUpdatePage creator
 */
 
bool CCompletionUpdatePage::Create( CBOINCBaseWizard* parent )
{
////@begin CCompletionUpdatePage member initialisation
    m_pCompletionTitle = NULL;
    m_pCompletionMessage = NULL;
////@end CCompletionUpdatePage member initialisation
 
////@begin CCompletionUpdatePage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CCompletionUpdatePage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CCompletionUpdatePage
 */
 
void CCompletionUpdatePage::CreateControls()
{    
////@begin CCompletionUpdatePage content construction
    CCompletionUpdatePage* itemWizardPage79 = this;

    wxBoxSizer* itemBoxSizer80 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage79->SetSizer(itemBoxSizer80);

    m_pCompletionTitle = new wxStaticText;
    m_pCompletionTitle->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pCompletionTitle->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Verdana")));
    itemBoxSizer80->Add(m_pCompletionTitle, 0, wxALIGN_LEFT|wxALL, 5);

    m_pCompletionMessage = new wxStaticText;
    m_pCompletionMessage->Create( itemWizardPage79, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer80->Add(m_pCompletionMessage, 0, wxALIGN_LEFT|wxALL, 5);
////@end CCompletionUpdatePage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CCompletionUpdatePage::GetPrev() const
{
    return NULL;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CCompletionUpdatePage::GetNext() const
{
    return NULL;
}
  
/*!
 * Should we show tooltips?
 */
 
bool CCompletionUpdatePage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CCompletionUpdatePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCompletionUpdatePage bitmap retrieval
    return wxNullBitmap;
////@end CCompletionUpdatePage bitmap retrieval
}
  
/*!
 * Get icon resources
 */
 
wxIcon CCompletionUpdatePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
 
////@begin CCompletionUpdatePage icon retrieval
    return wxNullIcon;
////@end CCompletionUpdatePage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_CompletionUpdatePage
 */
 
void CCompletionUpdatePage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pCompletionTitle);
    wxASSERT(m_pCompletionMessage);

    wxString strTitle;
    if (wxGetApp().GetBrand()->IsBranded()) {
        // %s is the project name
        //    i.e. 'GridRepublic'
        strTitle.Printf(
            _("Updating %s successful!"),
            wxGetApp().GetBrand()->GetProjectName().c_str()
        );
    } else {
        strTitle = _("Update successful!");
    }

    m_pCompletionTitle->SetLabel( strTitle );

    m_pCompletionMessage->SetLabel(
        _("Click Finish to close.")
    );

    Fit();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_CompletionUpdatePage
 */
 
void CCompletionUpdatePage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}
 
/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_CompletionUpdatePage
 */
 
void CCompletionUpdatePage::OnFinished( wxWizardExEvent& event ) {
    event.Skip();
}


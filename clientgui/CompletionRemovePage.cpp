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
#pragma implementation "CompletionRemovePage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "CompletionRemovePage.h"
#include "AccountInfoPage.h"


/*!
 * CCompletionRemovePage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CCompletionRemovePage, wxWizardPageEx )
 
/*!
 * CCompletionRemovePage event table definition
 */
 
BEGIN_EVENT_TABLE( CCompletionRemovePage, wxWizardPageEx )
 
////@begin CCompletionRemovePage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CCompletionRemovePage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CCompletionRemovePage::OnCancel )
    EVT_WIZARDEX_FINISHED( ID_COMPLETIONREMOVEPAGE, CCompletionRemovePage::OnFinished )

////@end CCompletionRemovePage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CCompletionRemovePage constructors
 */
 
CCompletionRemovePage::CCompletionRemovePage( )
{
}
 
CCompletionRemovePage::CCompletionRemovePage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CCompletionRemovePage creator
 */
 
bool CCompletionRemovePage::Create( CBOINCBaseWizard* parent )
{
////@begin CCompletionRemovePage member initialisation
    m_CompletionTitle = NULL;
    m_CompletionWelcome = NULL;
    m_CompletionBrandedMessage = NULL;
    m_CompletionMessage = NULL;
////@end CCompletionRemovePage member initialisation
 
////@begin CCompletionRemovePage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CCompletionRemovePage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CCompletionRemovePage
 */
 
void CCompletionRemovePage::CreateControls()
{    
////@begin CCompletionRemovePage content construction
    CCompletionRemovePage* itemWizardPage79 = this;

    wxBoxSizer* itemBoxSizer80 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage79->SetSizer(itemBoxSizer80);

    wxString strTitle;
    if (wxGetApp().GetBrand()->IsBranded()) {
        // %s is the project name
        //    i.e. 'GridRepublic'
        strTitle.Printf(
            _("Removal from %s successful!"),
            wxGetApp().GetBrand()->GetProjectName().c_str()
        );
    } else {
        strTitle = _("Removal succeeded!");
    }

    m_CompletionTitle = new wxStaticText;
    m_CompletionTitle->Create( itemWizardPage79, wxID_STATIC, strTitle, wxDefaultPosition, wxDefaultSize, 0 );
    m_CompletionTitle->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Verdana")));
    itemBoxSizer80->Add(m_CompletionTitle, 0, wxALIGN_LEFT|wxALL, 5);

    m_CompletionMessage = new wxStaticText;
    m_CompletionMessage->Create( itemWizardPage79, wxID_STATIC, _("Click Finish to close."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer80->Add(m_CompletionMessage, 0, wxALIGN_LEFT|wxALL, 5);

////@end CCompletionRemovePage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CCompletionRemovePage::GetPrev() const
{
    return NULL;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CCompletionRemovePage::GetNext() const
{
    return NULL;
}
  
/*!
 * Should we show tooltips?
 */
 
bool CCompletionRemovePage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CCompletionRemovePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCompletionRemovePage bitmap retrieval
    return wxNullBitmap;
////@end CCompletionRemovePage bitmap retrieval
}
  
/*!
 * Get icon resources
 */
 
wxIcon CCompletionRemovePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
 
////@begin CCompletionRemovePage icon retrieval
    return wxNullIcon;
////@end CCompletionRemovePage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_CompletionRemovePage
 */
 
void CCompletionRemovePage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_CompletionRemovePage
 */
 
void CCompletionRemovePage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}
 
/*!
 * wxEVT_WIZARD_FINISHED event handler for ID_CompletionRemovePage
 */
 
void CCompletionRemovePage::OnFinished( wxWizardExEvent& event ) {
    event.Skip();
}


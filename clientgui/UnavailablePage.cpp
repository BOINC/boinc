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
#pragma implementation "UnavailablePage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "UnavailablePage.h"


/*!
 * CErrUnavailablePage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrUnavailablePage, wxWizardPageEx )
 
/*!
 * CErrUnavailablePage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrUnavailablePage, wxWizardPageEx )
 
////@begin CErrUnavailablePage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrUnavailablePage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrUnavailablePage::OnCancel )

////@end CErrUnavailablePage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CErrUnavailablePage constructors
 */
 
CErrUnavailablePage::CErrUnavailablePage( )
{
}
  
CErrUnavailablePage::CErrUnavailablePage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrUnavailablePage creator
 */
 
bool CErrUnavailablePage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrUnavailablePage member initialisation
////@end CErrUnavailablePage member initialisation
 
////@begin CErrUnavailablePage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrUnavailablePage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CErrUnavailablePage
 */
 
void CErrUnavailablePage::CreateControls()
{    
////@begin CErrUnavailablePage content construction
    CErrUnavailablePage* itemWizardPage96 = this;

    wxBoxSizer* itemBoxSizer97 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage96->SetSizer(itemBoxSizer97);

    wxStaticText* itemStaticText98 = new wxStaticText;
    wxStaticText* itemStaticText100 = new wxStaticText;
    if (IS_ATTACHTOPROJECTWIZARD()) {
        itemStaticText98->Create( itemWizardPage96, wxID_STATIC, _("Project temporarily unavailable"), wxDefaultPosition, wxDefaultSize, 0 );
        itemStaticText98->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
        itemBoxSizer97->Add(itemStaticText98, 0, wxALIGN_LEFT|wxALL, 5);

        itemBoxSizer97->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

        itemStaticText100->Create( itemWizardPage96, wxID_STATIC, _("The project is temporarily unavailable.\n\nPlease try again later."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer97->Add(itemStaticText100, 0, wxALIGN_LEFT|wxALL, 5);
    }

    if (IS_ACCOUNTMANAGERATTACHWIZARD()) {
        itemStaticText98->Create( itemWizardPage96, wxID_STATIC, _("Account manager temporarily unavailable"), wxDefaultPosition, wxDefaultSize, 0 );
        itemStaticText98->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
        itemBoxSizer97->Add(itemStaticText98, 0, wxALIGN_LEFT|wxALL, 5);

        itemBoxSizer97->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

        itemStaticText100->Create( itemWizardPage96, wxID_STATIC, _("The account manager is temporarily unavailable.\n\nPlease try again later."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer97->Add(itemStaticText100, 0, wxALIGN_LEFT|wxALL, 5);
    }

////@end CErrUnavailablePage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CErrUnavailablePage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CErrUnavailablePage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */

bool CErrUnavailablePage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrUnavailablePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
 
////@begin CErrUnavailablePage bitmap retrieval
    return wxNullBitmap;
////@end CErrUnavailablePage bitmap retrieval
}
  
/*!
 * Get icon resources
 */

wxIcon CErrUnavailablePage::GetIconResource( const wxString& name )
{
    // Icon retrieval

////@begin CErrUnavailablePage icon retrieval
    return wxNullIcon;
////@end CErrUnavailablePage icon retrieval
}
   
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTUNAVAILABLEPAGE
 */

void CErrUnavailablePage::OnPageChanged( wxWizardExEvent& event ) {
    event.Skip();
}
 
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTUNAVAILABLEPAGE
 */

void CErrUnavailablePage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


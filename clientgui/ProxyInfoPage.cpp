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
#pragma implementation "ProxyInfoPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "ProxyInfoPage.h"


/*!
 * CErrProxyInfoPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrProxyInfoPage, wxWizardPageEx )
  
/*!
 * CErrProxyInfoPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrProxyInfoPage, wxWizardPageEx )
 
////@begin CErrProxyInfoPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrProxyInfoPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrProxyInfoPage::OnCancel )

////@end CErrProxyInfoPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrProxyInfoPage constructors
 */
 
CErrProxyInfoPage::CErrProxyInfoPage( )
{
}
 
CErrProxyInfoPage::CErrProxyInfoPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrProxyInfoPage creator
 */
 
bool CErrProxyInfoPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrProxyInfoPage member initialisation
////@end CErrProxyInfoPage member initialisation
  
////@begin CErrProxyInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProxyInfoPage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CErrProxyInfoPage
 */
 
void CErrProxyInfoPage::CreateControls()
{    
////@begin CErrProxyInfoPage content construction
    CErrProxyInfoPage* itemWizardPage126 = this;

    wxBoxSizer* itemBoxSizer127 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage126->SetSizer(itemBoxSizer127);

    wxStaticText* itemStaticText128 = new wxStaticText;
    itemStaticText128->Create( itemWizardPage126, wxID_STATIC, 
        _("Network communication failure"),
        wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText128->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer127->Add(itemStaticText128, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText130 = new wxStaticText;
    itemStaticText130->Create( itemWizardPage126, wxID_STATIC, 
        _("BOINC failed to communicate on the Internet. The most likely\n"
          "reasons are:\n"
          "\n"
          "1) Personal firewall software is blocking BOINC. You must\n"
          "configure your personal firewall to let BOINC communicate\n"
          "on port 80. Once this is fixed, click Back to try again.\n"
          "\n"
          "2) You are using a proxy server and BOINC needs to know\n"
          "about it.\n"
          "\n"
          "Click Next to configure BOINC's proxy settings.\n"),
          wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer127->Add(itemStaticText130, 0, wxALIGN_LEFT|wxALL, 5);

////@end CErrProxyInfoPage content construction
}
  
/*!
 * Gets the previous page.
 */

wxWizardPageEx* CErrProxyInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CErrProxyInfoPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_ERRPROXYPAGE);
    }
    return NULL;
}
  
/*!
 * Should we show tooltips?
 */
 
bool CErrProxyInfoPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrProxyInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProxyInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProxyInfoPage bitmap retrieval
}
  
/*!
 * Get icon resources
 */
 
wxIcon CErrProxyInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
 
////@begin CErrProxyInfoPage icon retrieval
    return wxNullIcon;
////@end CErrProxyInfoPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYINFOPAGE
 */

void CErrProxyInfoPage::OnPageChanged( wxWizardExEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYINFOPAGE
 */
 
void CErrProxyInfoPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


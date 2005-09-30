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
#pragma implementation "ProjectInfoPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "hyperlink.h"
#include "ValidateURL.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "ProjectInfoPage.h"


/*!
 * CProjectInfoPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectInfoPage, wxWizardPageEx )
 
/*!
 * CProjectInfoPage event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectInfoPage, wxWizardPageEx )
 
////@begin CProjectInfoPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CProjectInfoPage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CProjectInfoPage::OnPageChanging )
    EVT_WIZARDEX_CANCEL( -1, CProjectInfoPage::OnCancel )

////@end CProjectInfoPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CProjectInfoPage constructors
 */
 
CProjectInfoPage::CProjectInfoPage( )
{
}
 
CProjectInfoPage::CProjectInfoPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * WizardPage creator
 */
 
bool CProjectInfoPage::Create( CBOINCBaseWizard* parent )
{
////@begin CProjectInfoPage member initialisation
    m_ProjectUrlStaticCtrl = NULL;
    m_ProjectUrlCtrl = NULL;
////@end CProjectInfoPage member initialisation
 
////@begin CProjectInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CProjectInfoPage creation

    return TRUE;
}
 
/*!
 * Control creation for WizardPage
 */
 
void CProjectInfoPage::CreateControls()
{    
////@begin CProjectInfoPage content construction
    CProjectInfoPage* itemWizardPage23 = this;

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage23->SetSizer(itemBoxSizer24);

    wxStaticText* itemStaticText25 = new wxStaticText;
    itemStaticText25->Create( itemWizardPage23, wxID_STATIC, _("Project URL"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText25->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(itemStaticText25, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText;
    itemStaticText26->Create( itemWizardPage23, wxID_STATIC, _("Enter the URL of the project's web site."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(itemStaticText26, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText28 = new wxStaticText;
    itemStaticText28->Create( itemWizardPage23, wxID_STATIC, _("You can copy and paste the URL from your browser’s address bar."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(itemStaticText28, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer30 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer30->AddGrowableCol(1);
    itemBoxSizer24->Add(itemFlexGridSizer30, 0, wxALIGN_LEFT|wxALL, 5);

    m_ProjectUrlStaticCtrl = new wxStaticText;
    m_ProjectUrlStaticCtrl->Create( itemWizardPage23, ID_PROJECTURLSTATICCTRL, _("Project &URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(m_ProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProjectUrlCtrl = new wxTextCtrl;
    m_ProjectUrlCtrl->Create( itemWizardPage23, ID_PROJECTURLCTRL, _T(""), wxDefaultPosition, wxSize(250, -1), 0 );
    itemFlexGridSizer30->Add(m_ProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText34 = new wxStaticText;
    itemStaticText34->Create( itemWizardPage23, wxID_STATIC, _("For a list of BOINC-based projects go to:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(itemStaticText34, 0, wxALIGN_LEFT|wxALL, 5);

    wxHyperLink* itemHyperLink35 = new wxHyperLink;
    itemHyperLink35->Create( itemWizardPage23, ID_BOINCHYPERLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer24->Add(itemHyperLink35, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_ProjectUrlCtrl->SetValidator( CValidateURL( & m_strProjectURL) );
////@end CProjectInfoPage content construction
}
 
/*!
 * Gets the previous page.
 */
wxWizardPageEx* CProjectInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CProjectInfoPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CProjectInfoPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CProjectInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProjectInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CProjectInfoPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CProjectInfoPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProjectInfoPage icon retrieval
    return wxNullIcon;
////@end CProjectInfoPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanged( wxWizardExEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanging( wxWizardExEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


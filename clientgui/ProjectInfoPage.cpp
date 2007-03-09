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
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
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
    EVT_COMBOBOX( ID_PROJECTSELECTIONCTRL, CProjectInfoPage::OnProjectSelectionChanged )
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
    m_pTitleStaticCtrl = NULL;
    m_pDescriptionStaticCtrl = NULL;
    m_pDescription2StaticCtrl = NULL;
    m_pProjectSelectionCtrl = NULL;
    m_pProjectUrlStaticCtrl = NULL;
    m_pProjectUrlCtrl = NULL;
////@end CProjectInfoPage member initialisation
 
////@begin CProjectInfoPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, ID_PROJECTINFOPAGE, wizardBitmap );

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

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescription2StaticCtrl = new wxStaticText;
    m_pDescription2StaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pDescription2StaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer24->Add(itemBoxSizer8, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer8->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxString* strProjectSelectionCtrlStrings = NULL;
    m_pProjectSelectionCtrl = new wxComboBox;
    m_pProjectSelectionCtrl->Create( itemWizardPage23, ID_PROJECTSELECTIONCTRL, wxEmptyString, wxDefaultPosition, wxSize(320, -1), 0, strProjectSelectionCtrlStrings, wxCB_DROPDOWN | wxCB_READONLY);
    itemBoxSizer8->Add(m_pProjectSelectionCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer11 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer11->AddGrowableRow(0);
    itemFlexGridSizer11->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer11, 0, wxGROW|wxALL, 0);

    m_pProjectSelectionStaticCtrl = new wxStaticBox(itemWizardPage23, wxID_STATIC, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer12 = new wxStaticBoxSizer(m_pProjectSelectionStaticCtrl, wxVERTICAL);
    itemFlexGridSizer11->Add(itemStaticBoxSizer12, 0, wxALL, 0);

    m_pProjectSelectionDescriptionStaticCtrl = new wxStaticText;
    m_pProjectSelectionDescriptionStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(320, 115), wxST_NO_AUTORESIZE );
    itemStaticBoxSizer12->Add(m_pProjectSelectionDescriptionStaticCtrl, 0, wxALL, 5);
    m_pProjectSelectionDescriptionStaticCtrl->Wrap(320);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer11->Add(itemBoxSizer22, 0, wxGROW|wxALL, 0);

    m_pProjectUrlDescriptionStaticCtrl = new wxStaticText;
    m_pProjectUrlDescriptionStaticCtrl->Create( itemWizardPage23, ID_PROJECTURLDESCRIPTIONSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(m_pProjectUrlDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);
    m_pProjectUrlDescriptionStaticCtrl->Hide();

    wxFlexGridSizer* itemFlexGridSizer14 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer14->AddGrowableCol(1);
    itemBoxSizer22->Add(itemFlexGridSizer14, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pProjectUrlStaticCtrl = new wxStaticText;
    m_pProjectUrlStaticCtrl->Create( itemWizardPage23, ID_PROJECTURLSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer14->Add(m_pProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_pProjectUrlStaticCtrl->Hide();

    m_pProjectUrlCtrl = new wxTextCtrl;
    m_pProjectUrlCtrl->Create( itemWizardPage23, ID_PROJECTURLCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer14->Add(m_pProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_pProjectUrlCtrl->Hide();

    // Set validators
    m_pProjectUrlCtrl->SetValidator( CValidateURL( & m_strProjectURL) );
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
 
wxBitmap CProjectInfoPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CProjectInfoPage bitmap retrieval
    return wxNullBitmap;
////@end CProjectInfoPage bitmap retrieval
}


/*!
 * Get icon resources
 */
 
wxIcon CProjectInfoPage::GetIconResource( const wxString& WXUNUSED(name) )
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
    if (event.GetDirection() == false) return;

    unsigned int   i;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pDescription2StaticCtrl);
    wxASSERT(m_pProjectSelectionCtrl);
    wxASSERT(m_pProjectUrlStaticCtrl);
    wxASSERT(m_pProjectUrlCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Project Selection")
    );
    m_pDescriptionStaticCtrl->SetLabel(
        _("Choose which project you would like to participate in.")
    );
    m_pDescription2StaticCtrl->SetLabel(
        _("Project selection list:")
    );
    m_pProjectSelectionStaticCtrl->SetLabel(
        _("Project Description")
    );
    m_pProjectUrlDescriptionStaticCtrl->SetLabel(
        _("You can copy and paste the URL from your browser's\n"
		  "address bar.")
    );
    m_pProjectUrlStaticCtrl->SetLabel(
        _("Project &URL:")
    );


    // Populate the combo box with project information
    //
    pDoc->rpc.get_project_list(pl);
    for (i=0; i<pl.projects.size(); i++) {
        m_pProjectSelectionCtrl->Append(ConstructProjectTitle(i));
    }
    m_pProjectSelectionCtrl->Append(_("Other"));


    Fit();
    m_pProjectSelectionCtrl->SetValue(
        ConstructProjectTitle(0)
    );
    m_pProjectSelectionDescriptionStaticCtrl->SetLabel(
        wxString(pl.projects[0]->description.c_str(), wxConvUTF8)
    );
    m_pProjectUrlCtrl->SetValue(
        wxString(pl.projects[0]->url.c_str(), wxConvUTF8)
    );
    m_pProjectSelectionCtrl->SetFocus();
}


/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanging( wxWizardExEvent& event ) {
    event.Skip();
}


/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_PROJECTSELECTIONCTRL
 */

void CProjectInfoPage::OnProjectSelectionChanged( wxCommandEvent& /*event*/ ) {
    if (m_pProjectSelectionCtrl->GetValue() == _("Other")) {
        m_pProjectSelectionStaticCtrl->Hide();
        m_pProjectSelectionDescriptionStaticCtrl->Hide();
        m_pProjectUrlDescriptionStaticCtrl->Show();
        m_pProjectUrlStaticCtrl->Show();
        m_pProjectUrlCtrl->Show();

        m_pProjectUrlCtrl->SetValue(
            _T("")
        );
    } else {
        m_pProjectSelectionStaticCtrl->Show();
        m_pProjectSelectionDescriptionStaticCtrl->Show();
        m_pProjectUrlDescriptionStaticCtrl->Hide();
        m_pProjectUrlStaticCtrl->Hide();
        m_pProjectUrlCtrl->Hide();

        m_pProjectSelectionDescriptionStaticCtrl->SetLabel(
            wxString(pl.projects[m_pProjectSelectionCtrl->GetSelection()]->description.c_str(), wxConvUTF8)
        );
        m_pProjectUrlCtrl->SetValue(
            wxString(pl.projects[m_pProjectSelectionCtrl->GetSelection()]->url.c_str(), wxConvUTF8)
        );
    }
    Fit();
}


/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


/*!
 * Construct what the string should look like for a project in the
 *   drop down combo box.
 */

wxString CProjectInfoPage::ConstructProjectTitle( int iIndex ) {
    wxString strReturnValue;

    strReturnValue = wxString(pl.projects[iIndex]->name.c_str(), wxConvUTF8);

    return strReturnValue;
}


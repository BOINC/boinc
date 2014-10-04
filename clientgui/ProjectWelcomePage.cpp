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
#pragma implementation "ProjectWelcomePage.h"
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
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "ProjectWelcomePage.h"

////@begin XPM images
////@end XPM images


/*!
 * CProjectWelcomePage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectWelcomePage, wxWizardPageEx )
 
/*!
 * CProjectWelcomePage event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectWelcomePage, wxWizardPageEx )
 
////@begin CProjectWelcomePage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CProjectWelcomePage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CProjectWelcomePage::OnCancel )
////@end CProjectWelcomePage event table entries
 
END_EVENT_TABLE()

/*!
 * CProjectWelcomePage constructors
 */
 
CProjectWelcomePage::CProjectWelcomePage( )
{
}
 
CProjectWelcomePage::CProjectWelcomePage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * WizardPage creator
 */
 
bool CProjectWelcomePage::Create( CBOINCBaseWizard* parent )
{
////@begin CProjectWelcomePage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDescriptionStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
////@end CProjectWelcomePage member initialisation

	((CWizardAttach*)parent)->IsFirstPass = false;
 
////@begin CProjectWelcomePage creation
    wxWizardPageEx::Create( parent, ID_PROJECTWELCOMEPAGE );

    CreateControls();
    GetSizer()->Fit(this);
////@end CProjectWelcomePage creation

	return TRUE;
}
 
/*!
 * Control creation for WizardPage
 */
 
void CProjectWelcomePage::CreateControls()
{    
////@begin CWelcomePage content construction
    CProjectWelcomePage* itemWizardPage2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage2->SetSizer(itemBoxSizer3);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer3->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer62 = new wxFlexGridSizer(0, 1, 0, 0);
    itemFlexGridSizer62->AddGrowableCol(0);
    itemBoxSizer3->Add(itemFlexGridSizer62, 0, wxGROW|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemWizardPage2->SetSizer(itemBoxSizer3);

////@end CWelcomePage content construction
}

/*
 * Gets the previous page.
 */
 
wxWizardPageEx* CProjectWelcomePage::GetPrev() const
{
    return NULL;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CProjectWelcomePage::GetNext() const
{
    CWizardAttach* pWA  = ((CWizardAttach*)GetParent());

    wxASSERT(pWA);

    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (pWA->GetProjectAuthenticator().IsEmpty()) {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROCESSINGPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CProjectWelcomePage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CProjectWelcomePage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CWelcomePage bitmap retrieval
    return wxNullBitmap;
////@end CWelcomePage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CProjectWelcomePage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CWelcomePage icon retrieval
    return wxNullIcon;
////@end CWelcomePage icon retrieval
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTWELCOMEPAGE
 */
 
void CProjectWelcomePage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectWelcomePage::OnPageChanged - Function Begin"));

    CWizardAttach* pWA  = ((CWizardAttach*)GetParent());

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

	m_pTitleStaticCtrl->SetLabel(
        pWA->GetProjectName()
	);

    m_pDescriptionStaticCtrl->SetLabel(
        pWA->GetProjectURL()
    );

    m_pDirectionsStaticCtrl->SetLabel(
        _("To continue, click Next.")
    );

    Fit();
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectWelcomePage::OnPageChanged - Function End"));
}


/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTWELCOMEPAGE
 */
 
void CProjectWelcomePage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

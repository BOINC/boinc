// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.
//

// show a "welcome" dialog showing the user what project they're about to run,
// in the case where a project_init.xml was present on startup
//
// AFAIK no one uses this mechanism, so this may not be needed

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

    title_ctrl = new wxStaticText;
    title_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    title_ctrl->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer3->Add(title_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

    intro_ctrl = new wxStaticText;
    intro_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(intro_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* grid = new wxFlexGridSizer(5, 2, 0, 0);
    grid->AddGrowableCol(1);
    grid->SetFlexibleDirection(wxBOTH);
    itemBoxSizer3->Add(grid, 0, wxEXPAND|wxALL, 5);

    project_name1_ctrl = new wxStaticText;
    project_name1_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    grid->Add(project_name1_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

    project_name2_ctrl = new wxStaticText;
    project_name2_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    grid->Add(project_name2_ctrl, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

    project_url1_ctrl = new wxStaticText;
    project_url1_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    grid->Add(project_url1_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

    project_url2_ctrl = new wxStaticText;
    project_url2_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    grid->Add(project_url2_ctrl, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 5);

    continue_ctrl = new wxStaticText;
    continue_ctrl->Create( itemWizardPage2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(continue_ctrl, 0, wxALIGN_LEFT|wxALL, 5);

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
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
    }
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

    wxString buf;
    buf.Printf(_("Welcome to %s."), pWA->GetProjectName().c_str());
    title_ctrl->SetLabel(buf);

    intro_ctrl->SetLabel(_("You have volunteered to compute for this project:"));
    project_name1_ctrl->SetLabel(_("Name:"));
    project_name2_ctrl->SetLabel(pWA->GetProjectName());
    project_url1_ctrl->SetLabel(_("URL:"));
    project_url2_ctrl->SetLabel(pWA->GetProjectURL());

    continue_ctrl->SetLabel(
        _("To continue, click Next.")
    );

    Layout();
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectWelcomePage::OnPageChanged - Function End"));
}


/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTWELCOMEPAGE
 */

void CProjectWelcomePage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

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

#ifndef BOINC_PROJECTWELCOMEPAGE_H
#define BOINC_PROJECTWELCOMEPAGE_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ProjectWelcomePage.cpp"
#endif

/*!
 * CWelcomePage class declaration
 */

class CProjectWelcomePage: public wxWizardPageEx
{
    DECLARE_DYNAMIC_CLASS( CProjectWelcomePage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectWelcomePage( );

    CProjectWelcomePage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectWelcomePage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTWELCOMEPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_PROJECTWELCOMEPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CProjectWelcomePage event handler declarations

////@begin CProjectWelcomePage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

////@end CProjectWelcomePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CProjectWelcomePage member variables
    wxStaticText* title_ctrl;
    wxStaticText* intro_ctrl;
    wxStaticText* project_name1_ctrl;
    wxStaticText* project_name2_ctrl;
    wxStaticText* project_inst1_ctrl;
    wxStaticText* project_inst2_ctrl;
    wxStaticText* project_desc1_ctrl;
    wxStaticText* project_desc2_ctrl;
    wxStaticText* project_url1_ctrl;
    wxStaticText* project_url2_ctrl;
    wxStaticText* user_name1_ctrl;
    wxStaticText* user_name2_ctrl;

    wxStaticText* warning_ctrl;
    wxStaticText* continue_ctrl;

////@end CProjectWelcomePage member variables
};

#endif

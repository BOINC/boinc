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
#ifndef _WIZ_WELCOMEPAGE_H_
#define _WIZ_WELCOMEPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "WelcomePage.cpp"
#endif

/*!
 * CWelcomePage class declaration
 */

class CWelcomePage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CWelcomePage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWelcomePage( );

    CWelcomePage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CWelcomePage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WELCOMEPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for
    ///   ID_WELCOMESELECTWIZARDPROJECT or ID_WELCOMESELECTWIZARDACCOUNTMGR
    void OnWizardSelectionChanged( wxCommandEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE
    void OnCancel( wxWizardExEvent& event );

	/// wxEVT_BUTTON event handler for ID_WELCOMECHANGEAPPS
	void OnChangeApplications( wxCommandEvent& event );

////@end CWelcomePage event handler declarations

////@begin CWelcomePage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

////@end CWelcomePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWelcomePage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDescriptionStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
    wxRadioButton* m_pAttachToProjectCtrl;
    wxRadioButton* m_pAttachToAccountManagerCtrl;
	wxButton* m_pChangeApplicationsCtrl;
////@end CWelcomePage member variables
};

#endif // _WIZ_WELCOMEPAGE_H_

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
#ifndef _WIZ_TERMSOFUSEPAGE_H_
#define _WIZ_TERMSOFUSEPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "TermsOfUsePage.cpp"
#endif

/*!
 * CTermsOfUsePage class declaration
 */

class CTermsOfUsePage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CTermsOfUsePage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTermsOfUsePage( );

    CTermsOfUsePage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CTermsOfUsePage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_TERMSOFUSEPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_TERMSOFUSEPAGE
    void OnPageChanging( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_TERMSOFUSEPAGE
    void OnCancel( wxWizardExEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED  event handler for ID_TERMSOFUSEAGREECTRL
    ///   or ID_TERMSOFUSEDISAGREECTRL
    void OnTermsOfUseStatusChange( wxCommandEvent& event );

////@end CTermsOfUsePage event handler declarations

////@begin CTermsOfUsePage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTermsOfUsePage member function declarations

    bool GetUserAgrees() const { return m_bUserAgrees ; }
    void SetUserAgrees(bool value) { m_bUserAgrees = value ; }

    bool GetCredentialsAlreadyAvailable() const { return m_bCredentialsAlreadyAvailable ; }
    void SetCredentialsAlreadyAvailable(bool value) { m_bCredentialsAlreadyAvailable = value ; }

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTermsOfUsePage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
    wxTextCtrl* m_pTermsOfUseCtrl;
    wxRadioButton* m_pAgreeCtrl;
    wxRadioButton* m_pDisagreeCtrl;
////@end CTermsOfUsePage member variables
    bool m_bUserAgrees;
    bool m_bCredentialsAlreadyAvailable;
};

#endif

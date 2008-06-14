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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE
    void OnPageChanging( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE
    void OnCancel( wxWizardExEvent& event );

    /// wxEVT_SET_FOCUS event handler for ID_WELCOMEPAGE
    void OnSetFocus( wxFocusEvent& event );

    /// wxEVT_SHOW event handler for ID_WELCOMEPAGE
    void OnShow( wxShowEvent& event );

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
#if defined(__WXDEBUG__)
    wxStaticBox* m_pErrDescriptionCtrl; 
    wxCheckBox* m_pErrProjectPropertiesCtrl;
    wxCheckBox* m_pErrProjectCommCtrl;
    wxCheckBox* m_pErrProjectPropertiesURLCtrl;
    wxCheckBox* m_pErrAccountCreationDisabledCtrl;
    wxCheckBox* m_pErrClientAccountCreationDisabledCtrl;
    wxCheckBox* m_pErrAccountAlreadyExistsCtrl;
    wxCheckBox* m_pErrProjectAlreadyAttachedCtrl;
    wxCheckBox* m_pErrProjectAttachFailureCtrl;
    wxCheckBox* m_pErrGoogleCommCtrl;
    wxCheckBox* m_pErrNetDetectionCtrl;
#endif
////@end CWelcomePage member variables
};

#endif // _WIZ_WELCOMEPAGE_H_

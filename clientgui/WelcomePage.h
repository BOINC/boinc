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
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectPropertiesCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectCommCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectPropertiesURLCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrAccountCreationDisabledCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrClientAccountCreationDisabledCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrAccountAlreadyExistsCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectAlreadyAttachedCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrProjectAttachFailureCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrGoogleCommCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrYahooCommCtrl;
#endif
#if defined(__WXDEBUG__)
    wxCheckBox* m_ErrNetDetectionCtrl;
#endif
////@end CWelcomePage member variables
    int iWizardID;
};

#endif // _WIZ_WELCOMEPAGE_H_

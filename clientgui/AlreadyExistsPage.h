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
#ifndef _WIZ_ALREADYEXISTSPAGE_H_
#define _WIZ_ALREADYEXISTSPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "AlreadyExistsPage.cpp"
#endif

/*!
 * CErrAlreadyExistsPage class declaration
 */

class CErrAlreadyExistsPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CErrAlreadyExistsPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrAlreadyExistsPage( );

    CErrAlreadyExistsPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrAlreadyExistsPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRACCOUNTALREADYEXISTSPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRACCOUNTALREADYEXISTSPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CErrAlreadyExistsPage event handler declarations

////@begin CErrAlreadyExistsPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrAlreadyExistsPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrAlreadyExistsPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
////@end CErrAlreadyExistsPage member variables
};

#endif // _WIZ_ALREADYEXISTSPAGE_H_

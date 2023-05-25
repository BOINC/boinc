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
#ifndef BOINC_NOTFOUNDPAGE_H
#define BOINC_NOTFOUNDPAGE_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "NotFoundPage.cpp"
#endif

/*!
 * CErrNotFoundPage class declaration
 */

class CErrNotFoundPage: public wxWizardPageEx
{
    DECLARE_DYNAMIC_CLASS( CErrNotFoundPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CErrNotFoundPage( );

    CErrNotFoundPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CErrNotFoundPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRACCOUNTNOTFOUNDPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ERRACCOUNTNOTFOUNDPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CErrNotFoundPage event handler declarations

////@begin CErrNotFoundPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CErrNotFoundPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CErrNotFoundPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
////@end CErrNotFoundPage member variables
};


#endif

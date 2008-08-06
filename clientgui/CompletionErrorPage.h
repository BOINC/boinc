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
#ifndef _WIZ_COMPLETIONERRORPAGE_H_
#define _WIZ_COMPLETIONERRORPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "CompletionErrorPage.cpp"
#endif

/*!
 * CCompletionErrorPage class declaration
 */

class CCompletionErrorPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CCompletionErrorPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCompletionErrorPage( );

    CCompletionErrorPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCompletionErrorPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONERRORPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONERRORPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CCompletionErrorPage event handler declarations

////@begin CCompletionErrorPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCompletionErrorPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCompletionErrorPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
    wxStaticBox* m_pServerMessagesDescriptionCtrl;
    wxStaticBoxSizer* m_pServerMessagesStaticBoxSizerCtrl;
    wxStaticText* m_pServerMessagesCtrl;
////@end CCompletionErrorPage member variables
};

#endif // _WIZ_COMPLETIONERRORPAGE_H_

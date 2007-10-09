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
#ifndef _WIZ_COMPLETIONPAGE_H_
#define _WIZ_COMPLETIONPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "CompletionPage.cpp"
#endif

/*!
 * CCompletionPage class declaration
 */

class CCompletionPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CCompletionPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCompletionPage( );

    CCompletionPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCompletionPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_COMPLETIONPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_COMPLETIONPAGE
    void OnCancel( wxWizardExEvent& event );

    /// wxEVT_WIZARD_FINISHED event handler for ID_COMPLETIONPAGE
    void OnFinished( wxWizardExEvent& event );

////@end CCompletionPage event handler declarations

////@begin CCompletionPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCompletionPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCompletionPage member variables
    wxStaticText* m_pCompletionTitle;
    wxStaticText* m_pCompletionWelcome;
    wxStaticText* m_pCompletionBrandedMessage;
    wxStaticText* m_pCompletionMessage;
////@end CCompletionPage member variables
};

#endif // _WIZ_COMPLETIONPAGE_H_

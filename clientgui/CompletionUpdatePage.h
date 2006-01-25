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
#ifndef _WIZ_COMPLETIONUPDATEPAGE_H_
#define _WIZ_COMPLETIONUPDATEPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "CompletionUpdatePage.cpp"
#endif

/*!
 * CCompletionUpdatePage class declaration
 */

class CCompletionUpdatePage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CCompletionUpdatePage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCompletionUpdatePage( );

    CCompletionUpdatePage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCompletionUpdatePage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_CompletionUpdatePage
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_CompletionUpdatePage
    void OnCancel( wxWizardExEvent& event );

    /// wxEVT_WIZARD_FINISHED event handler for ID_CompletionUpdatePage
    void OnFinished( wxWizardExEvent& event );

////@end CCompletionUpdatePage event handler declarations

////@begin CCompletionUpdatePage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCompletionUpdatePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCompletionUpdatePage member variables
    wxStaticText* m_CompletionTitle;
    wxStaticText* m_CompletionWelcome;
    wxStaticText* m_CompletionBrandedMessage;
    wxStaticText* m_CompletionMessage;
////@end CCompletionUpdatePage member variables
};

#endif // _WIZ_COMPLETIONUPDATEPAGE_H_

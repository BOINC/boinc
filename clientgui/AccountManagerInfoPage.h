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
#ifndef _WIZ_ACCOUNTMANAGERINFOPAGE_H_
#define _WIZ_ACCOUNTMANAGERINFOPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "AccountManagerInfoPage.cpp"
#endif

class CProjectListCtrl;
class ProjectListCtrlEvent;

/*!
 * CAccountManagerInfoPage class declaration
 */

class CAccountManagerInfoPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CAccountManagerInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountManagerInfoPage( );

    CAccountManagerInfoPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountManagerInfoPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanging( wxWizardExEvent& event );

    /// wxEVT_PROJECTLIST_ITEM_CHANGE event handler for ID_PROJECTSELECTIONCTRL
    void OnProjectItemChange( ProjectListCtrlEvent& event );

    /// wxEVT_PROJECTLIST_ITEM_DISPLAY event handler for ID_PROJECTSELECTIONCTRL
    void OnProjectItemDisplay( ProjectListCtrlEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CAccountManagerInfoPage event handler declarations

////@begin CAccountManagerInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    wxString GetProjectURL() const { return m_strProjectURL ; }
    void SetProjectURL(wxString value) { m_strProjectURL = value ; }

    bool GetProjectSupported() const { return m_bProjectSupported ; }
    void SetProjectSupported(bool value) { m_bProjectSupported = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAccountManagerInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAccountManagerInfoPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDescriptionStaticCtrl;
    CProjectListCtrl* m_pProjectListCtrl;
    wxStaticText* m_pProjectUrlStaticCtrl;
    wxTextCtrl* m_pProjectUrlCtrl;
////@end CAccountManagerInfoPage member variables
    wxString m_strProjectURL;
    bool m_bProjectSupported;
    bool m_bAccountManagerListPopulated;
};

#endif // _WIZ_ACCOUNTMANAGERINFOPAGE_H_

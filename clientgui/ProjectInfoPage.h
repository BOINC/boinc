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
#ifndef _WIZ_PROJECTINFOPAGE_H_
#define _WIZ_PROJECTINFOPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ProjectInfoPage.cpp"
#endif

class CProjectListCtrl;
class ProjectListCtrlEvent;

/*!
 * CProjectInfoPage class declaration
 */

class CProjectInfoPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CProjectInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectInfoPage( );

    CProjectInfoPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectInfoPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
    void OnPageChanging( wxWizardExEvent& event );

    /// wxEVT_PROJECTLIST_ITEM_CHANGE event handler for ID_PROJECTSELECTIONCTRL
    void OnProjectItemChange( ProjectListCtrlEvent& event );

    /// wxEVT_PROJECTLIST_ITEM_DISPLAY event handler for ID_PROJECTSELECTIONCTRL
    void OnProjectItemDisplay( ProjectListCtrlEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CProjectInfoPage event handler declarations

////@begin CProjectInfoPage member function declarations

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
////@end CProjectInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CProjectInfoPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDescriptionStaticCtrl;
    CProjectListCtrl* m_pProjectListCtrl;
    wxStaticText* m_pProjectUrlStaticCtrl;
    wxTextCtrl* m_pProjectUrlCtrl;
////@end CProjectInfoPage member variables
    wxString m_strProjectURL;
    bool m_bProjectSupported;
    bool m_bProjectListPopulated;
};

#endif // _WIZ_PROJECTINFOPAGE_H_

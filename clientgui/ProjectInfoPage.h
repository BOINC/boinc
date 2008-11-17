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

    /// wxEVT_PROJECTLISTCTRL_SELECTION_CHANGED event handler for ID_PROJECTSELECTIONCTRL
    void OnProjectSelectionChanged( ProjectListCtrlEvent& event );

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
    wxString m_strProjectURL;
////@end CProjectInfoPage member variables
    bool                bProjectListPopulated;
    ALL_PROJECTS_LIST   pl;
};

#endif // _WIZ_PROJECTINFOPAGE_H_

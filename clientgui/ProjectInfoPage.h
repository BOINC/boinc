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


class CProjectInfo;


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

    /// Destructor
    ~CProjectInfoPage( );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectInfoPage event handler declarations

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_PROJECTCATEGORY
    void OnProjectCategorySelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_PROJECTS
    void OnProjectSelected( wxCommandEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
    void OnPageChanging( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
    void OnCancel( wxWizardExEvent& event );

    void EllipseStringIfNeeded(wxString& s, wxWindow *win);

////@end CProjectInfoPage event handler declarations

////@begin CProjectInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    wxString GetProjectURL() const { return m_strProjectURL ; }
    void SetProjectURL(wxString value) { m_strProjectURL = value ; }

    wxString GetProjectDetailsDescription() const { return m_strProjectDetailsDescription ; }
    void SetProjectDetailsDescription(wxString value) { m_strProjectDetailsDescription = value ; }

    wxString GetProjectDetailsResearchArea() const { return m_strProjectDetailsResearchArea ; }
    void SetProjectDetailsResearchArea(wxString value) { m_strProjectDetailsResearchArea = value ; }

    wxString GetProjectDetailsOrganization() const { return m_strProjectDetailsOrganization ; }
    void SetProjectDetailsOrganization(wxString value) { m_strProjectDetailsOrganization = value ; }

    wxString GetProjectDetailsURL() const { return m_strProjectDetailsURL ; }
    void SetProjectDetailsURL(wxString value) { m_strProjectDetailsURL = value ; }

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
    wxStaticText* m_pProjectCategoriesStaticCtrl;
    wxComboBox* m_pProjectCategoriesCtrl;
    wxStaticText* m_pProjectsStaticCtrl;
    wxListBox* m_pProjectsCtrl;
    wxStaticBox* m_pProjectDetailsStaticCtrl;
    wxTextCtrl* m_pProjectDetailsDescriptionCtrl;
    wxStaticText* m_pProjectDetailsResearchAreaStaticCtrl;
    wxStaticText* m_pProjectDetailsResearchAreaCtrl;
    wxStaticText* m_pProjectDetailsOrganizationStaticCtrl;
    wxStaticText* m_pProjectDetailsOrganizationCtrl;
    wxStaticText* m_pProjectDetailsURLStaticCtrl;
    wxHyperlinkCtrl* m_pProjectDetailsURLCtrl;
    wxStaticText* m_pProjectDetailsSupportedPlatformsStaticCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformWindowsCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformMacCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformLinuxCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformATICtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformNvidiaCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformBlankCtrl;
    wxStaticText* m_pProjectURLStaticCtrl;
    wxTextCtrl* m_pProjectURLCtrl;
////@end CProjectInfoPage member variables
private:
    wxString m_strProjectURL;
    wxString m_strProjectDetailsDescription;
    wxString m_strProjectDetailsResearchArea;
    wxString m_strProjectDetailsOrganization;
    wxString m_strProjectDetailsURL;
    std::vector<CProjectInfo*> m_Projects;
    bool m_bProjectSupported;
    bool m_bProjectListPopulated;
};

#endif // _WIZ_PROJECTINFOPAGE_H_

// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.
//
#ifndef BOINC_PROJECTINFOPAGE_H
#define BOINC_PROJECTINFOPAGE_H

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

    /// wxEVT_LIST_ITEM_SELECTED event handler for ID_PROJECTS
    void OnProjectSelected( wxListEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
    void OnPageChanging( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CProjectInfoPage event handler declarations

////@begin CProjectInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CProjectInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void EllipseStringIfNeeded(wxString& s, wxWindow *win);

    void RefreshPage();

    void TrimURL(std::string& purl);

////@begin CProjectInfoPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDescriptionStaticCtrl;
    wxStaticText* m_pProjectCategoriesStaticCtrl;
    wxComboBox* m_pProjectCategoriesCtrl;
    wxStaticText* m_pProjectsStaticCtrl;
    wxListCtrl* m_pProjectsCtrl;
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
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformAndroidCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformFreeBSDCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformLinuxArmCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformATICtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformNvidiaCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformIntelGPUCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformVirtualBoxCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformBlankCtrl;
    wxStaticText* m_pProjectURLStaticCtrl;
    wxTextCtrl* m_pProjectURLCtrl;
////@end CProjectInfoPage member variables
private:
    ALL_PROJECTS_LIST* m_apl;
    wxString m_strProjectURL;
    std::vector<CProjectInfo*> m_Projects;
    bool m_bProjectSupported;
    bool m_bProjectListPopulated;
    std::vector<std::string> m_pTrimmedURL;
    std::vector<std::string> m_pTrimmedURL_attached;
};

#endif

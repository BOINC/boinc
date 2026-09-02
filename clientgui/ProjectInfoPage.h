// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

class CProjectInfo;

class CProjectInfoPage: public CBOINCWizardPage {
    DECLARE_DYNAMIC_CLASS(CProjectInfoPage)
    DECLARE_EVENT_TABLE()

public:
    CProjectInfoPage();
    CProjectInfoPage(CWizardAttach* parent);
    ~CProjectInfoPage();
    bool Create(CWizardAttach* parent);

    void CreateControls();

    void OnProjectCategorySelected(wxCommandEvent& event);
    void OnProjectSelected(wxListEvent& event);
    void OnPageChanged(wxWizardEvent& event);
    void OnPageChanging(wxWizardEvent& event);
    void OnCancel(wxWizardEvent& event);

    wxWizardPage* GetPrev() const;
    wxWizardPage* GetNext() const;

    void SetPrev(CBOINCWizardPage *prev);

    bool HasNextPage() const;
    bool HasPrevPage() const;

    wxBitmap GetBitmapResource(const wxString& name);

    void EllipseStringIfNeeded(wxString& s, wxWindow *win);

    void RefreshPage();

    void TrimURL(std::string& purl);

private:
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
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformRaspberryPiCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformDockerCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformMetalCtrl;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformBlankCtrl;
    wxStaticText* m_pProjectURLStaticCtrl;
    wxTextCtrl* m_pProjectURLCtrl;
    ALL_PROJECTS_LIST* m_apl;
    wxString m_strProjectURL;
    std::vector<CProjectInfo*> m_Projects;
    bool m_bProjectSupported;
    bool m_bProjectListPopulated;
    std::vector<std::string> m_pTrimmedURL;
    std::vector<std::string> m_pTrimmedURL_attached;
    CWizardAttach *m_pParent;
    CBOINCWizardPage *m_pPrev;
};

#endif

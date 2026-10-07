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

public:
    CProjectInfoPage() = default;
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
    wxStaticText* m_pTitleStaticCtrl = nullptr;
    wxStaticText* m_pDescriptionStaticCtrl = nullptr;
    wxStaticText* m_pProjectCategoriesStaticCtrl = nullptr;
    wxComboBox* m_pProjectCategoriesCtrl = nullptr;
    wxStaticText* m_pProjectsStaticCtrl = nullptr;
    wxListCtrl* m_pProjectsCtrl = nullptr;
    wxStaticBox* m_pProjectDetailsStaticCtrl = nullptr;
    wxTextCtrl* m_pProjectDetailsDescriptionCtrl = nullptr;
    wxStaticText* m_pProjectDetailsResearchAreaStaticCtrl = nullptr;
    wxStaticText* m_pProjectDetailsResearchAreaCtrl = nullptr;
    wxStaticText* m_pProjectDetailsOrganizationStaticCtrl = nullptr;
    wxStaticText* m_pProjectDetailsOrganizationCtrl = nullptr;
    wxStaticText* m_pProjectDetailsURLStaticCtrl = nullptr;
    wxHyperlinkCtrl* m_pProjectDetailsURLCtrl = nullptr;
    wxStaticText* m_pProjectDetailsSupportedPlatformsStaticCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformWindowsCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformMacCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformLinuxCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformAndroidCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformFreeBSDCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformLinuxArmCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformATICtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformNvidiaCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformIntelGPUCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformVirtualBoxCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformRaspberryPiCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformDockerCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformMetalCtrl = nullptr;
    wxStaticBitmap* m_pProjectDetailsSupportedPlatformBlankCtrl = nullptr;
    wxStaticText* m_pProjectURLStaticCtrl = nullptr;
    wxTextCtrl* m_pProjectURLCtrl = nullptr;
    ALL_PROJECTS_LIST* m_apl = nullptr;
    wxString m_strProjectURL;
    std::vector<CProjectInfo*> m_Projects;
    bool m_bProjectSupported = false;
    bool m_bProjectListPopulated = false;
    std::vector<std::string> m_pTrimmedURL;
    std::vector<std::string> m_pTrimmedURL_attached;
    CWizardAttach *m_pParent = nullptr;
    CBOINCWizardPage *m_pPrev = nullptr;
};

#endif

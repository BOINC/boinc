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
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BOINC_SG_PROJECTPANEL_H
#define BOINC_SG_PROJECTPANEL_H

#include "sg_CustomControls.h"
#include "sg_PanelBase.h"
#include "sg_ProjectWebSitesPopup.h"
#include "sg_ProjectCommandPopup.h"

typedef struct {
    char project_url[256];
    double project_files_downloaded_time;
} ProjectSelectionData;

///////////////////////////////////////////////////////////////////////////////
/// Class CSimpleProjectPanel
///////////////////////////////////////////////////////////////////////////////

class CBOINCBitmapComboBox;

class CSimpleProjectPanel : public CSimplePanelBase
{
    DECLARE_DYNAMIC_CLASS( CSimpleProjectPanel )

	public:
        CSimpleProjectPanel() = default;
		CSimpleProjectPanel( wxWindow* parent);
		~CSimpleProjectPanel();

        ProjectSelectionData* GetProjectSelectionData();
        wxString GetSelectedProjectString() { return m_ProjectSelectionCtrl->GetValue(); }
        CBOINCBitmapComboBox* GetProjectSelectionCtrl() { return m_ProjectSelectionCtrl; }
        void UpdateInterface();
        void ReskinInterface();

	private:
        void OnProjectSelection(wxCommandEvent &event);
        void OnAddProject(wxCommandEvent& /*event*/);
        void OnWizardAttach(wxCommandEvent&);
        void OnWizardUpdate();
        void UpdateProjectList();
        std::string GetProjectIconLoc(char* project_url);
        wxBitmap* GetProjectSpecificBitmap(char* project_url);

	protected:
		CTransparentStaticText*             m_myProjectsLabel = nullptr;
		CBOINCBitmapComboBox*               m_ProjectSelectionCtrl = nullptr;
		CTransparentButton*                 m_TaskAddProjectButton = nullptr;
        CTransparentStaticText*             m_TotalCreditValue = nullptr;
		CSimpleProjectWebSitesPopupButton*  m_ProjectWebSitesButton = nullptr;
		CSimpleProjectCommandPopupButton*   m_ProjectCommandsButton = nullptr;
        wxString                            m_sAddProjectString;
        wxString                            m_sSynchronizeString;
        wxString                            m_sTotalWorkDoneString;
        int                                 m_UsingAccountManager = 0;
        char                                m_CurrentSelectedProjectURL[256];
        double                              m_Project_last_rpc_time = 0;
        wxString                            m_sAddProjectToolTip;
        wxString                            m_sSynchronizeToolTip;
        double                              m_fDisplayedCredit = 0;
};

#endif

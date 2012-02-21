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

#ifndef __sg_ProjectPanel__
#define __sg_ProjectPanel__

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

class CSimpleProjectPanel : public CSimplePanelBase 
{
    DECLARE_DYNAMIC_CLASS( CSimpleProjectPanel )
    DECLARE_EVENT_TABLE()

	public:
        CSimpleProjectPanel();
		CSimpleProjectPanel( wxWindow* parent);
		~CSimpleProjectPanel();
        
        ProjectSelectionData* GetProjectSelectionData();
        void UpdateInterface();

	private:
        void OnProjectSelection(wxCommandEvent &event);
        void OnProjectCommandButton(wxCommandEvent& /*event*/);
        void OnAddProject(wxCommandEvent& /*event*/);
        void OnWizardAttach();
        void OnWizardUpdate();
        void OnProjectWebSiteButton(wxCommandEvent& /*event*/);
        void UpdateProjectList();
        std::string GetProjectIconLoc(char* project_url);
        wxBitmap* GetProjectSpecificBitmap(char* project_url);

	protected:
		CTransparentStaticText*             m_myProjectsLabel;
		CBOINCBitmapComboBox*               m_ProjectSelectionCtrl;
		wxButton*                           m_TaskAddProjectButton;
        CTransparentStaticText*             m_TotalCreditValue;
		CSimpleProjectWebSitesPopupButton*  m_ProjectWebSitesButton;
		CSimpleProjectCommandPopupButton*   m_ProjectCommandsButton;
        wxString                            m_sAddProjectString;
        wxString                            m_sSynchronizeString;
        wxString                            m_sTotalWorkDoneString;
        int                                 m_UsingAccountManager;
        char                                m_CurrentSelectedProjectURL[256];
        double                              m_Project_last_rpc_time;
        wxString                            m_sAddProjectToolTip;
        wxString                            m_sSynchronizeToolTip;
        double                              m_fDisplayedCredit;
};

#endif //__sg_ProjectPanel__

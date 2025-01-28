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

#ifndef BOINC_SG_PROJECTCOMMANDPOPUP_H
#define BOINC_SG_PROJECTCOMMANDPOPUP_H

#include "sg_CustomControls.h"

class CSimpleProjectCommandPopupButton : public CTransparentButton
{
    DECLARE_DYNAMIC_CLASS( CSimpleProjectCommandPopupButton )
    DECLARE_EVENT_TABLE()

    public:
        CSimpleProjectCommandPopupButton();

		CSimpleProjectCommandPopupButton(wxWindow* parent, wxWindowID id,
        const wxString& label = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxT("ProjectCommandsPopupMenu"));

		~CSimpleProjectCommandPopupButton();

	private:
        void AddMenuItems();
        void OnProjectCommandsMouseDown(wxMouseEvent& event);
        void OnProjectCommandsKeyboardNav(wxCommandEvent& event);
        void ShowProjectCommandsMenu(wxPoint pos);
        void OnProjectUpdate(wxCommandEvent& event);
        void OnProjectSuspendResume(wxCommandEvent& event);
        void OnProjectNoNewWork(wxCommandEvent& event);
        void OnResetProject(wxCommandEvent& event);
        void OnProjectDetach(wxCommandEvent& event);
        void OnProjectShowProperties(wxCommandEvent& event);
        PROJECT* FindProjectIndexFromURL(char *project_url, int *index);

	protected:
        wxMenu*                     m_ProjectCommandsPopUpMenu;
        wxMenuItem*                 m_UpdateProjectMenuItem;
        wxMenuItem*                 m_SuspendResumeMenuItem;
        wxMenuItem*                 m_NoNewTasksMenuItem;
        wxMenuItem*                 m_ResetProjectMenuItem;
        wxMenuItem*                 m_RemoveProjectMenuItem;
        wxMenuItem*                 m_ShowPropertiesMenuItem;
};

#endif

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


#ifndef BOINC_SG_TASKCOMMANDPOPUP_H
#define BOINC_SG_TASKCOMMANDPOPUP_H

#include "sg_CustomControls.h"

class CSimpleTaskPopupButton : public CTransparentButton
{
    DECLARE_DYNAMIC_CLASS( CSimpleTaskPopupButton )

    public:
        CSimpleTaskPopupButton() = default;

		CSimpleTaskPopupButton(wxWindow* parent, wxWindowID id,
        const wxString& label = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxT("TaskCommandsPopupMenu"));

		~CSimpleTaskPopupButton();

	private:
        void AddMenuItems();
        void OnTaskCommandsMouseDown(wxMouseEvent& event);
        void OnTaskCommandsKeyboardNav(wxCommandEvent& event);
        void ShowTaskCommandsMenu(wxPoint pos);
        void OnTaskShowGraphics(wxCommandEvent& event);
        void OnTaskSuspendResume(wxCommandEvent& event);
        void OnTaskAbort(wxCommandEvent& event);
        void OnTaskShowProperties(wxCommandEvent& event);
        RESULT* lookup_result(char* url, char* name);

	protected:
        wxMenu*                     m_TaskCommandPopUpMenu = nullptr;
        wxMenuItem*                 m_ShowGraphicsMenuItem = nullptr;
        wxMenuItem*                 m_SuspendResumeMenuItem = nullptr;
        wxMenuItem*                 m_AbortMenuItem = nullptr;
        wxMenuItem*                 m_ShowPropertiesMenuItem = nullptr;
        bool                        m_TaskSuspendedViaGUI = false;
};

#endif

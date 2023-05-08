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


#ifndef BOINC_SG_TASKCOMMANDPOPUP_H
#define BOINC_SG_TASKCOMMANDPOPUP_H

#include "sg_CustomControls.h"

class CSimpleTaskPopupButton : public CTransparentButton
{
    DECLARE_DYNAMIC_CLASS( CSimpleTaskPopupButton )
    DECLARE_EVENT_TABLE()

    public:
        CSimpleTaskPopupButton();

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
        wxMenu*                     m_TaskCommandPopUpMenu;
        wxMenuItem*                 m_ShowGraphicsMenuItem;
        wxMenuItem*                 m_SuspendResumeMenuItem;
        wxMenuItem*                 m_AbortMenuItem;
        wxMenuItem*                 m_ShowPropertiesMenuItem;
        bool                        m_TaskSuspendedViaGUI;
};

#endif

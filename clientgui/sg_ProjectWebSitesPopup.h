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

#ifndef BOINC_SG_PROJECTWEBSITESPOPUP_H
#define BOINC_SG_PROJECTWEBSITESPOPUP_H

#include "sg_CustomControls.h"

class CSimpleProjectWebSitesPopupButton : public CTransparentButton
{
    DECLARE_DYNAMIC_CLASS( CSimpleProjectWebSitesPopupButton )
    DECLARE_EVENT_TABLE()

    public:
        CSimpleProjectWebSitesPopupButton();

		CSimpleProjectWebSitesPopupButton(wxWindow* parent, wxWindowID id,
        const wxString& label = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxT("ProjectWebsitesPopupMenu"));

		~CSimpleProjectWebSitesPopupButton();

        void RebuildMenu();

	private:
        void AddMenuItems();
        void OnProjectWebSitesMouseDown(wxMouseEvent& event);
        void OnProjectWebSitesKeyboardNav(wxCommandEvent& event);
        void ShowProjectWebSitesMenu(wxPoint pos);
        void OnMenuLinkClicked(wxCommandEvent& event);

	protected:
        wxMenu*                     m_ProjectWebSitesPopUpMenu;
};

#endif

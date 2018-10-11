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

#ifndef BOINC_DLGITEMPROPERTIES_H
#define BOINC_DLGITEMPROPERTIES_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgItemProperties.cpp"
#endif

#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>

#include "MainDocument.h"

#define ID_COPYSELECTED 10001
#define ID_COPYALL 10002

class CDlgItemProperties : public wxDialog {
	DECLARE_DYNAMIC_CLASS( CDlgItemProperties )
    DECLARE_EVENT_TABLE()
public:
	CDlgItemProperties(wxWindow* parent=NULL);//to act as standard constructor set a default value
	virtual ~CDlgItemProperties();
    //
	void renderInfos(PROJECT* project);
	void renderInfos(RESULT* result);
    void show_rsc(wxString rsc_name, RSC_DESC rsc_desc);
    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYSELECTED
    void OnCopySelected( wxCommandEvent& event );
    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPYALL
    void OnCopyAll( wxCommandEvent& event );

private:
       std::vector<struct ITEM> m_items;
	//formatting methods
	wxString FormatDiskSpace(double bytes);
	wxString FormatApplicationName(RESULT* result );
	//generic layout methods
    bool SaveState();
    bool RestoreState();
       void renderInfos();
	void addSection(const wxString& title);
	void addProperty(const wxString& name, const wxString& value);
    void copyTextToClipboard(const wxString& text);
    void OnMouseButtonEvent(wxMouseEvent& event);
protected:
        wxBoxSizer* m_bSizer1;
        wxButton* m_btnClose;
        wxButton* m_pCopySelectedButton;
        wxButton* m_pCopyAllButton;
        wxString m_strBaseConfigLocation;
        wxHtmlWindow* m_txtInformation;
};

#endif

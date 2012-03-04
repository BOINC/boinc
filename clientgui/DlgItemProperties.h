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
#ifndef _DLGITEMPROPERTIES_H_
#define _DLGITEMPROPERTIES_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgItemProperties.cpp"
#endif

#include <wx/intl.h>

#include <wx/gdicmn.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/dialog.h>

#include "MainDocument.h"

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
private:
	int m_current_row;
	//formatting methods
	wxString FormatDiskSpace(double bytes);
	wxString FormatApplicationName(RESULT* result );
	wxString FormatTime(float fBuffer);
	//generic layout methods
    bool SaveState();
    bool RestoreState();
	void addSection(const wxString& title);
	void addProperty(const wxString& name, const wxString& value);
protected:
        wxBoxSizer* m_bSizer1;
        wxScrolledWindow* m_scrolledWindow;
        wxGridBagSizer* m_gbSizer;
        wxButton* m_btnClose;
        wxString m_strBaseConfigLocation;
};

#endif // _DLGITEMPROPERTIES_H_


// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
public:
	CDlgItemProperties(wxWindow* parent=NULL);//to act as standard constructor set a default value
	virtual ~CDlgItemProperties();
	//
	void renderInfos(PROJECT* project);
	void renderInfos(RESULT* result);
private:
	int m_current_row;
	//formatting methods
	wxString FormatDiskSpace(double bytes);
	wxString FormatApplicationName(RESULT* result );
	wxString FormatStatus(RESULT* result);
	wxString FormatTime(float fBuffer);
	//generic layout methods
	void addSection(const wxString& title);
	void addProperty(const wxString& name, const wxString& value);
protected:
        wxBoxSizer* m_bSizer1;
        wxScrolledWindow* m_scrolledWindow;
        wxGridBagSizer* m_gbSizer;
        wxButton* m_btnClose;
};

#endif // _DLGITEMPROPERTIES_H_


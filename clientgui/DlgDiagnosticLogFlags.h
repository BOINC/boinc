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
#ifndef BOINC_DLGDIAGNOSTICLOGFLAGS_H
#define BOINC_DLGDIAGNOSTICLOGFLAGS_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgDiagnosticLogFlags.cpp"
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

#define ID_DEFAULTSBTN 10001

class CDlgDiagnosticLogFlags : public wxDialog {
	DECLARE_DYNAMIC_CLASS( CDlgDiagnosticLogFlags )
    DECLARE_EVENT_TABLE()
public:
	CDlgDiagnosticLogFlags(wxWindow* parent=NULL);
	virtual ~CDlgDiagnosticLogFlags();

    void OnSize(wxSizeEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnSetDefaults(wxCommandEvent& event);
    void OnApply(wxCommandEvent& event);
    void OnCheckBox(wxCommandEvent& event);
private:
	//generic layout methods
    bool SaveState();
    bool RestoreState();

    void CreateCheckboxes();
    void SaveFlags();

    LOG_FLAGS           log_flags;
    CC_CONFIG           m_cc_config;

    wxGridSizer*        m_headingSizer;
    std::vector <wxCheckBox*> m_checkbox_list;
    wxButton*           m_btnApply;

protected:
    wxString            m_headingText;
    wxStaticText*       m_heading;
    wxScrolledWindow*   m_scrolledWindow;
    wxGridSizer*        m_checkboxSizer;
};

#endif

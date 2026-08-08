// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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
#ifndef BOINC_DLGHIDDENCOLUMNS_H
#define BOINC_DLGHIDDENCOLUMNS_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgHiddenColumns.cpp"
#endif

#include "BOINCBaseView.h"

#define ID_DEFAULTSBTN 10001

class CDlgHiddenColumns : public wxDialog {
	DECLARE_DYNAMIC_CLASS( CDlgHiddenColumns )
    DECLARE_EVENT_TABLE()
public:
	CDlgHiddenColumns(wxWindow* parent=NULL);
	virtual ~CDlgHiddenColumns();

    void OnSize(wxSizeEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnSetDefaults(wxCommandEvent& event);
    void OnCheckboxClick(wxCommandEvent& event);

private:
	//generic layout methods
    bool SaveState();
    bool RestoreState();

    void CreateCheckboxes();

    wxButton*           m_btnOK;
    wxGridSizer*        m_headingSizer;
    std::vector <std::vector <wxCheckBox*>*> m_checkbox_list;
    std::vector <CBOINCBaseView*> m_pBOINCBaseView;

protected:
    wxString            m_headingText;
    wxStaticText*       m_heading;
    wxScrolledWindow*   m_scrolledWindow;
    wxBoxSizer*         m_scrolledSizer;
};

#endif

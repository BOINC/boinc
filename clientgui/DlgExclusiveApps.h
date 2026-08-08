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
#ifndef BOINC_DLGEXCLUSIVEAPPS_H
#define BOINC_DLGEXCLUSIVEAPPS_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgExclusiveApps.cpp"
#endif


#include "cc_config.h"

#define ID_LISTBOX_EXCLAPPS 20061
#define ID_ADDEXCLUSIVEAPPBUTTON 20062
#define ID_REMOVEEXCLUSIVEAPPBUTTON 20063
#define ID_LISTBOX_EXCLGPUAPPS 20064
#define ID_ADDEXCLUSIVEGPUAPPBUTTON 20065
#define ID_REMOVEEXCLUSIVEGPUAPPBUTTON 20066

class CDlgExclusiveApps : public wxDialog  {
	DECLARE_DYNAMIC_CLASS( CDlgExclusiveApps )
    DECLARE_EVENT_TABLE()
	void ReadPreferenceSettings();
	bool SavePreferencesSettings();
public:
	CDlgExclusiveApps(wxWindow* parent=NULL);
    virtual ~CDlgExclusiveApps();
    void OnExclusiveAppListEvent(wxCommandEvent& event);
    void OnExclusiveGPUAppListEvent(wxCommandEvent& event);
	void OnAddExclusiveApp(wxCommandEvent& event);
	void OnAddExclusiveGPUApp(wxCommandEvent& event);
    void AddToListBox(wxListBox * theListBox);
	void OnRemoveExclusiveApp(wxCommandEvent& event);
	void OnRemoveExclusiveGPUApp(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
private:
    LOG_FLAGS log_flags;
    CC_CONFIG config;
	bool m_bExclusiveAppsDataChanged;
	bool m_bInInit;

    wxListBox* m_exclusiveApsListBox;
    wxListBox* m_exclusiveGPUApsListBox;
    wxButton* m_addExclusiveAppButton;
    wxButton* m_removeExclusiveAppButton;
    wxButton* m_addExclusiveGPUAppButton;
    wxButton* m_removeExclusiveGPUAppButton;

    wxPanel* m_panelButtons;
    wxButton* m_btnOK;
    wxButton* m_btnCancel;
    wxButton* m_btnHelp;

};

#endif

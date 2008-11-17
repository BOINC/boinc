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
#ifndef _DLGADVPREFERENCES_H_
#define _DLGADVPREFERENCES_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgAdvPreferences.cpp"
#endif


#include "DlgAdvPreferencesBase.h"
#include "prefs.h"

#define TXT_PROC_TIME_TOOLTIP _("specify work start and stop hours in format HH:MM-HH:MM")
#define TXT_NET_TIME_TOOLTIP _("specify network usage start and stop hours in format HH:MM-HH:MM")

class CDlgAdvPreferences : public CDlgAdvPreferencesBase {
	DECLARE_DYNAMIC_CLASS( CDlgAdvPreferences )
    DECLARE_EVENT_TABLE()
	void ReadPreferenceSettings();
	bool SavePreferencesSettings();
	void UpdateControlStates();
	void SetSpecialTooltips();
    bool SaveState();
    bool RestoreState();
	bool ValidateInput();
	void SetValidators();
	bool IsValidFloatChar(const wxChar& ch);
	bool IsValidFloatValue(const wxString& value);
	bool IsValidTimeChar(const wxChar& ch);
	bool IsValidTimeValue(const wxString& value);
	bool IsValidTimeIntervalChar(const wxChar& ch);
	bool IsValidTimeIntervalValue(const wxString& value);
	void ShowErrorMessage(wxString& msg,wxTextCtrl* errorCtrl);
	bool EnsureTabPageVisible(wxTextCtrl* txtCtrl);
	bool ConfirmClear();
	wxString DoubleToTimeString(double dt);
	double TimeStringToDouble(wxString timeStr);
public:
	CDlgAdvPreferences(wxWindow* parent=NULL);//to act as standard constructor set a default value
	virtual ~CDlgAdvPreferences();
	//generic event handler
	void OnHandleCommandEvent(wxCommandEvent& ev);
	//
	void OnOK(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
	void OnClear(wxCommandEvent& event);
private:
    GLOBAL_PREFS      prefs;
    GLOBAL_PREFS_MASK mask;
	bool m_bDataChanged;
	bool m_bInInit;
	wxArrayInt m_arrTabPageIds;
};

#endif // _DLGADVPREFERENCES_H_


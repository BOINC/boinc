// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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
#ifndef BOINC_DLGADVPREFERENCES_H
#define BOINC_DLGADVPREFERENCES_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgAdvPreferences.cpp"
#endif


#include "DlgAdvPreferencesBase.h"
#include "prefs.h"
#include "cc_config.h"

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
    bool IsValidFloatValue(const wxString& value, bool allowNegative=false);
    bool IsValidFloatValueBetween(const wxString& value, double minVal, double maxVal);
    bool IsValidTimeChar(const wxChar& ch);
    bool IsValidTimeValue(const wxString& value);
    void ShowErrorMessage(wxString& msg,wxTextCtrl* errorCtrl);
    bool EnsureTabPageVisible(wxTextCtrl* txtCtrl);
    bool ConfirmClear();
    bool ConfirmSetLocal();
    wxString DoubleToTimeString(double dt);
    double TimeStringToDouble(wxString timeStr);
    double RoundToHundredths(double td);
public:
    CDlgAdvPreferences(wxWindow* parent=NULL);//to act as standard constructor set a default value
    virtual ~CDlgAdvPreferences();
    //generic event handler
    void OnHandleCommandEvent(wxCommandEvent& ev);
    void OnOK(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    void DisplayValue(double value, wxTextCtrl* textCtrl, wxCheckBox* checkBox=NULL);
    void EnableDisableInUseItem(wxTextCtrl* textCtrl, bool doEnable);
    void EnableDisableInUseItems();
    bool OKToShow() { return m_bOKToShow; }
private:
    GLOBAL_PREFS      prefs;
    GLOBAL_PREFS_MASK mask;
    GLOBAL_PREFS      defaultPrefs;
    LOG_FLAGS log_flags;
    CC_CONFIG config;
    wxArrayInt m_arrTabPageIds;
    wxTextValidator* m_vTimeValidator;
    wxCheckBox* procDayChks[7];
    wxTextCtrl* procDayStartTxts[7];
    wxTextCtrl* procDayStopTxts[7];
    wxCheckBox* netDayChks[7];
    wxTextCtrl* netDayStartTxts[7];
    wxTextCtrl* netDayStopTxts[7];
    bool m_bOKToShow;
    wxColour stdTextBkgdColor;
    wxTextCtrl* lastErrorCtrl;
};

#endif


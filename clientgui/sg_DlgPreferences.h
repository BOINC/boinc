// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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


#ifndef BOINC_SG_DLG_PREFERENCES_H
#define BOINC_SG_DLG_PREFERENCES_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_DlgPreferences.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valtext.h"
#include "wx/valgen.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DLGPREFERENCES 10000
#define SYMBOL_CDLGPREFERENCES_STYLE wxDEFAULT_DIALOG_STYLE
#define SYMBOL_CDLGPREFERENCES_TITLE wxT("")
#define SYMBOL_CDLGPREFERENCES_IDNAME ID_DLGPREFERENCES
#define SYMBOL_CDLGPREFERENCES_SIZE wxDefaultSize
#define SYMBOL_CDLGPREFERENCES_POSITION wxDefaultPosition

#define ID_SG_PREFS_START ID_DLGPREFERENCES+1

enum {
    ID_SGPREFERENCESCLEAR = ID_SG_PREFS_START,
    ID_CHKPROCONBATTERIES,
    ID_CHKPROCINUSE,
    ID_TXTPROCIDLEFOR,
    ID_CHKPROCEVERYDAY,
    ID_TXTPROCEVERYDAYSTART,
    ID_TXTPROCEVERYDAYSTOP,
    ID_TXTPROCUSECPUTIME,
    ID_CHKNETEVERYDAY,
    ID_TXTNETEVERYDAYSTART,
    ID_TXTNETEVERYDAYSTOP,
    ID_CHKDISKMAXSPACE,
    ID_TXTDISKMAXSPACE,
    ID_SG_PREFS_LAST
};

////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * CPanelPreferences class declaration
 */

class CPanelPreferences: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CPanelPreferences )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPanelPreferences( );
    CPanelPreferences( wxWindow* parent );

    ~CPanelPreferences( );

    /// Creation
    bool Create();

    /// Creates the controls and sizers
    void CreateControls();

    void MakeBackgroundBitmap();

////@begin CPanelPreferences event handler declarations
    /// wxEVT_ERASE_BACKGROUND event handler for ID_DLGPREFERENCES
    void OnEraseBackground( wxEraseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SIMPLE_HELP
    void OnButtonHelp( wxCommandEvent& event );

////@end CPanelPreferences event handler declarations

    void OnButtonClear();
    bool OnOK();
    bool ConfirmSetLocal();

    bool UpdateControlStates();

    bool ClearPreferenceSettings();
    wxString DoubleToTimeString(double dt);
    double TimeStringToDouble(wxString timeStr);
    double RoundToHundredths(double td);
    void DisplayValue(double value, wxTextCtrl* textCtrl, wxCheckBox* checkBox=NULL);
    bool ReadPreferenceSettings();
    bool SavePreferenceSettings();
    bool ValidateInput();
    bool m_bUsingLocalPrefs;
    void ShowErrorMessage(wxString& msg,wxTextCtrl* errorCtrl);
    bool IsValidFloatChar(const wxChar& ch);
    bool IsValidFloatValue(const wxString& value, bool allowNegative=false);
    bool IsValidFloatValueBetween(const wxString& value, double minVal, double maxVal);
    bool IsValidTimeChar(const wxChar& ch);
    bool IsValidTimeValue(const wxString& value);
    void OnHandleCheckboxEvent(wxCommandEvent& ev);

    void addNewRowToSizer(wxSizer* toSizer, wxString& toolTipText,
                wxWindow* first, wxWindow* second, wxWindow* third,
                wxWindow* fourth=NULL, wxWindow* fifth=NULL);
    wxSize getTextCtrlSize(wxString maxText);
    bool doesLocalPrefsFileExist();
    wxBitmap* GetBackgroundBmp() { return m_backgroundBitmap; }

    bool OKToShow() { return m_bOKToShow; }

private:
////@begin CPanelPreferences member variables
    wxCheckBox* m_chkProcOnBatteries;
    wxCheckBox* m_chkProcInUse;
    wxTextCtrl* m_txtProcIdleFor;
    wxCheckBox* m_chkProcEveryDay;
    wxTextCtrl* m_txtProcEveryDayStart;
    wxTextCtrl* m_txtProcEveryDayStop;
    wxTextCtrl* m_txtProcUseCPUTime;
    wxCheckBox* m_chkNetEveryDay;
    wxTextCtrl* m_txtNetEveryDayStart;
    wxTextCtrl* m_txtNetEveryDayStop;
    wxCheckBox* m_chkDiskMaxSpace;
    wxTextCtrl* m_txtDiskMaxSpace;

    wxTextValidator* m_vTimeValidator;
    wxButton* m_btnClear;
    wxString *web_prefs_url;

    wxBitmap* m_backgroundBitmap;

    bool m_bOKToShow;

    wxColour stdTextBkgdColor;
    wxTextCtrl* lastErrorCtrl;

////@end CPanelPreferences member variables
    GLOBAL_PREFS      global_preferences_working;
    GLOBAL_PREFS_MASK global_preferences_mask;
    GLOBAL_PREFS_MASK global_preferences_override_mask;
    GLOBAL_PREFS      defaultPrefs;

};


/*!
 * CDlgPreferences class declaration
 */

class CDlgPreferences: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CDlgPreferences )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgPreferences( );
    CDlgPreferences( wxWindow* parent, wxWindowID id = SYMBOL_CDLGPREFERENCES_IDNAME, const wxString& caption = SYMBOL_CDLGPREFERENCES_TITLE, const wxPoint& pos = SYMBOL_CDLGPREFERENCES_POSITION, const wxSize& size = SYMBOL_CDLGPREFERENCES_SIZE, long style = SYMBOL_CDLGPREFERENCES_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGPREFERENCES_IDNAME, const wxString& caption = SYMBOL_CDLGPREFERENCES_TITLE, const wxPoint& pos = SYMBOL_CDLGPREFERENCES_POSITION, const wxSize& size = SYMBOL_CDLGPREFERENCES_SIZE, long style = SYMBOL_CDLGPREFERENCES_STYLE );

    /// wxEVT_HELP event handler for ID_DLGPREFERENCES
    void OnHelp( wxHelpEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SGPREFERENCESCLEAR
    void OnButtonClear( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOK( wxCommandEvent& event );

    bool ConfirmClear();

    bool OKToShow() { return m_pBackgroundPanel->OKToShow(); }

    CPanelPreferences* GetPrefsPanel() { return m_pBackgroundPanel; }

private:
////@begin CDlgPreferences member variables

    CPanelPreferences* m_pBackgroundPanel;
};

#endif

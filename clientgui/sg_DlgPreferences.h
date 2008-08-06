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


#ifndef _DLG_PREFERENCES_H_ 
#define _DLG_PREFERENCES_H_ 

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
class CTransparentCheckBox;
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
#define ID_SKINSELECTOR 10001
#define ID_CUSTOMIZEPREFERENCES 10002
#define ID_WORKBETWEENBEGIN 10004
#define ID_WORKBETWEENEND 10006
#define ID_CONNECTBETWEENBEGIN 10007
#define ID_CONNECTBETWEENEND 10009
#define ID_MAXDISKUSAGE 10010
#define ID_MAXCPUUSAGE 10011
#define ID_WORKWHILEONBATTERY 10005
#define ID_WORKWHENIDLE 10012
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

    /// Creation
    bool Create();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CPanelPreferences event handler declarations
    /// wxEVT_ERASE_BACKGROUND event handler for ID_DLGPREFERENCES
    void OnEraseBackground( wxEraseEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CUSTOMIZEPREFERENCES
    void OnCustomizePreferencesClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_WORKBETWEENBEGIN
    void OnWorkBetweenBeginSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_CONNECTBETWEENBEGIN
    void OnConnectBetweenBeginSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SIMPLE_HELP
    void OnButtonHelp( wxCommandEvent& event );

////@end CPanelPreferences event handler declarations

////@begin CPanelPreferences member function declarations
    wxString GetSkinSelector() const { return m_strSkinSelector ; }
    void SetSkinSelector(wxString value) { m_strSkinSelector = value ; }

    wxString GetWorkBetweenBegin() const { return m_strWorkBetweenBegin ; }
    void SetWorkBetweenBegin(wxString value) { m_strWorkBetweenBegin = value ; }

    wxString GetWorkBetweenEnd() const { return m_strWorkBetweenEnd ; }
    void SetWorkBetweenEnd(wxString value) { m_strWorkBetweenEnd = value ; }

    wxString GetConnectBetweenBegin() const { return m_strConnectBetweenBegin ; }
    void SetConnectBetweenBegin(wxString value) { m_strConnectBetweenBegin = value ; }

    wxString GetConnectBetweenEnd() const { return m_strConnectBetweenEnd ; }
    void SetConnectBetweenEnd(wxString value) { m_strConnectBetweenEnd = value ; }

    wxString GetMaxDiskUsage() const { return m_strMaxDiskUsage ; }
    void SetMaxDiskUsage(wxString value) { m_strMaxDiskUsage = value ; }

    wxString GetMaxCPUUsage() const { return m_strMaxCPUUsage ; }
    void SetMaxCPUUsage(wxString value) { m_strMaxCPUUsage = value ; }

    bool GetWorkWhileOnBattery() const { return m_bWorkWhileOnBattery ; }
    void SetWorkWhileOnBattery(bool value) { m_bWorkWhileOnBattery = value ; }

    wxString GetWorkWhenIdle() const { return m_strWorkWhenIdle ; }
    void SetWorkWhenIdle(wxString value) { m_strWorkWhenIdle = value ; }

    bool GetCustomizedPreferences() const { return m_bCustomizedPreferences ; }
    void SetCustomizedPreferences(bool value) { m_bCustomizedPreferences = value ; }
////@end CPanelPreferences member function declarations

    void OnOK();

    bool UpdateControlStates();

    bool ClearPreferenceSettings();
    bool ReadPreferenceSettings();
    bool ReadSkinSettings();
    bool SavePreferenceSettings();
    bool SaveSkinSettings();

private:
////@begin CPanelPreferences member variables
    wxComboBox* m_SkinSelectorCtrl;
    wxCheckBox* m_CustomizePreferencesCtrl;
    wxComboBox* m_WorkBetweenBeginCtrl;
    wxComboBox* m_WorkBetweenEndCtrl;
    wxComboBox* m_ConnectBetweenBeginCtrl;
    wxComboBox* m_ConnectBetweenEndCtrl;
    wxComboBox* m_MaxDiskUsageCtrl;
    wxComboBox* m_MaxCPUUsageCtrl;
    wxCheckBox* m_WorkWhileOnBatteryCtrl;
    wxComboBox* m_WorkWhenIdleCtrl;
    wxString m_strSkinSelector;
    bool m_bCustomizedPreferences;
    wxString m_strWorkBetweenBegin;
    wxString m_strWorkBetweenEnd;
    wxString m_strConnectBetweenBegin;
    wxString m_strConnectBetweenEnd;
    wxString m_strMaxDiskUsage;
    wxString m_strMaxCPUUsage;
    bool m_bWorkWhileOnBattery;
    wxString m_strWorkWhenIdle;
////@end CPanelPreferences member variables
    GLOBAL_PREFS      global_preferences_working;
    GLOBAL_PREFS_MASK global_preferences_mask;
    GLOBAL_PREFS_MASK global_preferences_override_mask;
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

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOK( wxCommandEvent& event );

private:
////@begin CDlgPreferences member variables

    CPanelPreferences* m_pBackgroundPanel;
};



#endif  // end CDlgPreferences

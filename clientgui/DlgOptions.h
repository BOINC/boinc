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

#ifndef BOINC_DLGOPTIONS_H
#define BOINC_DLGOPTIONS_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgOptions.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/notebook.h"
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
#define ID_DIALOG 10000
#define SYMBOL_CDLGOPTIONS_STYLE wxDEFAULT_DIALOG_STYLE
#define SYMBOL_CDLGOPTIONS_TITLE wxT("")
#define SYMBOL_CDLGOPTIONS_IDNAME ID_DIALOG
#define SYMBOL_CDLGOPTIONS_SIZE wxDefaultSize
#define SYMBOL_CDLGOPTIONS_POSITION wxDefaultPosition
#define ID_NOTEBOOK 10001
#define ID_GENERAL 10002
#define ID_LANGUAGESELECTION 10004
#define ID_REMINDERFREQUENCY 10018
#define ID_ENABLEAUTOSTART 10031
#define ID_ENABLEEXITMESSAGE 10032
#define ID_ENABLERUNDAEMON 10033
#define ID_ENABLESHUTDOWNMESSAGE 10034
#define ID_ENABLEMENUBARICON 10035
#define ID_CONNECTONS 10019
#define ID_NETWORKAUTODETECT 10020
#define ID_NETWORKLAN 10021
#define ID_NETWORKDIALUP 10022
#define ID_DIALUPCONNECTIONS 10023
#define ID_DIALUPSETDEFAULT 10024
#define ID_DIALUPCLEARDEFAULT 10025
#define ID_DIALUPDEFAULTCONNECTIONTEXT 10027
#define ID_DIALUPDEFAULTCONNECTION 10026
#define ID_DIALUPPROMPTUSERNAMEPASSWORD 10030
#define ID_HTTPPROXY 10003
#define ID_ENABLEHTTPPROXYCTRL 10007
#define ID_HTTPADDRESSCTRL 10010
#define ID_HTTPPORTCTRL 10011
#define ID_HTTPUSERNAMECTRL 10008
#define ID_HTTPPASSWORDCTRL 10009
#define ID_SOCKSPROXY 10006
#define ID_ENABLESOCKSPROXYCTRL 10012
#define ID_SOCKSADDRESSCTRL 10013
#define ID_SOCKSPORTCTRL 10014
#define ID_SOCKSUSERNAMECTRL 10015
#define ID_SOCKSPASSWORDCTRL 10016
#define ID_HTTPNOPROXYCTRL 10017
#define ID_SOCKSNOPROXYCTRL 10018
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
 * CDlgOptions class declaration
 */

class CDlgOptions: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CDlgOptions )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgOptions( );
    CDlgOptions( wxWindow* parent, wxWindowID id = SYMBOL_CDLGOPTIONS_IDNAME, const wxString& caption = SYMBOL_CDLGOPTIONS_TITLE, const wxPoint& pos = SYMBOL_CDLGOPTIONS_POSITION, const wxSize& size = SYMBOL_CDLGOPTIONS_SIZE, long style = SYMBOL_CDLGOPTIONS_STYLE );

    /// Destructor
    ~CDlgOptions( );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGOPTIONS_IDNAME, const wxString& caption = SYMBOL_CDLGOPTIONS_TITLE, const wxPoint& pos = SYMBOL_CDLGOPTIONS_POSITION, const wxSize& size = SYMBOL_CDLGOPTIONS_SIZE, long style = SYMBOL_CDLGOPTIONS_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDlgOptions event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DIALUPSETDEFAULT
    void OnDialupSetDefaultClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DIALUPCLEARDEFAULT
    void OnDialupClearDefaultClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ENABLEHTTPPROXYCTRL
    void OnEnableHTTPProxyCtrlClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_ENABLEHTTPPROXYCTRL
    void OnEnableHTTPProxyCtrlUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ENABLESOCKSPROXYCTRL
    void OnEnableSOCKSProxyCtrlClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_ENABLESOCKSPROXYCTRL
    void OnEnableSOCKSProxyCtrlUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOK( wxCommandEvent& event );

////@end CDlgOptions event handler declarations

////@begin CDlgOptions member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgOptions member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxString GetDefaultDialupConnection() const;
    void SetDefaultDialupConnection(wxString value);

    bool ReadSettings();
    bool SaveSettings();

private:
////@begin CDlgOptions member variables
    wxComboBox* m_LanguageSelectionCtrl;
    wxComboBox* m_ReminderFrequencyCtrl;
    wxCheckBox* m_EnableBOINCManagerAutoStartCtrl;
    wxCheckBox* m_EnableBOINCManagerExitMessageCtrl;
    wxCheckBox* m_EnableBOINCClientShutdownMessageCtrl;
    wxCheckBox* m_EnableBOINCMenuBarIconCtrl;
    wxCheckBox* m_EnableRunDaemonCtrl;
    wxStaticBoxSizer* m_DialupStaticBoxCtrl;
    wxListBox* m_DialupConnectionsCtrl;
    wxButton* m_DialupSetDefaultCtrl;
    wxButton* m_DialupClearDefaultCtrl;
    wxStaticText* m_DialupDefaultConnectionTextCtrl;
    wxStaticText* m_DialupDefaultConnectionCtrl;
    wxCheckBox* m_EnableHTTPProxyCtrl;
    wxTextCtrl* m_HTTPAddressCtrl;
    wxTextCtrl* m_HTTPPortCtrl;
    wxTextCtrl* m_HTTPUsernameCtrl;
    wxTextCtrl* m_HTTPPasswordCtrl;
    wxCheckBox* m_EnableSOCKSProxyCtrl;
    wxTextCtrl* m_SOCKSAddressCtrl;
    wxTextCtrl* m_SOCKSPortCtrl;
    wxTextCtrl* m_SOCKSUsernameCtrl;
    wxTextCtrl* m_SOCKSPasswordCtrl;
    wxTextCtrl* m_HTTPNoProxiesCtrl;
    wxTextCtrl* m_SOCKSNoProxiesCtrl;
////@end CDlgOptions member variables
    bool m_bRetrievedProxyConfiguration;
};

#endif

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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _DLGOPTIONS_H_
#define _DLGOPTIONS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgOptions.cpp"
#endif


#define ID_TOOLSOPTIONSDIALOG 10000
#define SYMBOL_CDLGTOOLSOPTIONS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CDLGTOOLSOPTIONS_TITLE _("Options")
#define SYMBOL_CDLGTOOLSOPTIONS_IDNAME ID_TOOLSOPTIONSDIALOG
#define SYMBOL_CDLGTOOLSOPTIONS_SIZE wxDefaultSize
#define SYMBOL_CDLGTOOLSOPTIONS_POSITION wxDefaultPosition
#define ID_NOTEBOOK 10001
#define ID_GENERAL 10002
#define ID_LANGUAGESELECTION 10003
#define ID_HTTPPROXY 10004
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

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif


class CDlgOptions: public wxDialog
{    
    DECLARE_CLASS( CDlgOptions )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgOptions( );
    CDlgOptions( wxWindow* parent, wxWindowID id = SYMBOL_CDLGTOOLSOPTIONS_IDNAME, const wxString& caption = SYMBOL_CDLGTOOLSOPTIONS_TITLE, const wxPoint& pos = SYMBOL_CDLGTOOLSOPTIONS_POSITION, const wxSize& size = SYMBOL_CDLGTOOLSOPTIONS_SIZE, long style = SYMBOL_CDLGTOOLSOPTIONS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGTOOLSOPTIONS_IDNAME, const wxString& caption = SYMBOL_CDLGTOOLSOPTIONS_TITLE, const wxPoint& pos = SYMBOL_CDLGTOOLSOPTIONS_POSITION, const wxSize& size = SYMBOL_CDLGTOOLSOPTIONS_SIZE, long style = SYMBOL_CDLGTOOLSOPTIONS_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED event handler for ID_NOTEBOOK
    void OnNotebookPageChanged( wxNotebookEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_NOTEBOOK
    void OnNotebookUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ENABLEHTTPPROXYCTRL
    void OnEnableHTTPProxyCtrlClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_ENABLEHTTPPROXYCTRL
    void OnEnableHTTPProxyCtrlUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ENABLESOCKSPROXYCTRL
    void OnEnableSOCKSProxyCtrlClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_ENABLESOCKSPROXYCTRL
    void OnEnableSOCKSProxyCtrlUpdate( wxUpdateUIEvent& event );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxComboBox* m_LanguageSelectionCtrl;
    bool        m_bProxySectionConfigured;
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
};

#endif
    // _DLGOPTIONS_H_


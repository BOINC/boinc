// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

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
    void OnEnablehttpproxyctrlClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_ENABLEHTTPPROXYCTRL
    void OnEnablehttpproxyctrlUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ENABLESOCKSPROXYCTRL
    void OnEnablesocksproxyctrlClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_ENABLESOCKSPROXYCTRL
    void OnEnablesocksproxyctrlUpdate( wxUpdateUIEvent& event );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxCheckBox* m_EnableHTTPProxyCtrl;
    wxTextCtrl* m_HTTPAddressCtrl;
    wxTextCtrl* m_HTTPPortCtrl;
    wxTextCtrl* m_HTTPUsernameCtrl;
    wxTextCtrl* m_HTTPPasswordCtrl;
    wxCheckBox* m_EnableSOCKSProxyCtrl;
    wxTextCtrl* m_SOCKSAddressCtrl;
    wxTextCtrl* m_SOCKSPortCtrl;
    wxTextCtrl* m_SOCKSUsernameCtrl;
    wxTextCtrl* m_SOCKPasswordCtrl;
};

#endif
    // _DLGOPTIONS_H_

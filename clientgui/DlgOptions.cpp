// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgOptions.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "DlgOptions.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "BOINCDialupManager.h"

////@begin includes
////@end includes

////@begin XPM images
////@end XPM images

/*!
 * CDlgOptions type definition
 */

IMPLEMENT_DYNAMIC_CLASS(CDlgOptions, wxDialog)

/*!
 * CDlgOptions event table definition
 */

BEGIN_EVENT_TABLE(CDlgOptions, wxDialog)

////@begin CDlgOptions event table entries
#if defined(__WXMSW__)
    EVT_BUTTON( ID_DIALUPSETDEFAULT, CDlgOptions::OnDialupSetDefaultClick )
    EVT_BUTTON( ID_DIALUPCLEARDEFAULT, CDlgOptions::OnDialupClearDefaultClick )
#endif      // __WXMSW__
    EVT_CHECKBOX( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnableHTTPProxyCtrlClick )
    EVT_UPDATE_UI( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnableHTTPProxyCtrlUpdate )
    EVT_CHECKBOX( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnableSOCKSProxyCtrlClick )
    EVT_UPDATE_UI( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnableSOCKSProxyCtrlUpdate )
    EVT_BUTTON( wxID_OK, CDlgOptions::OnOK )
////@end CDlgOptions event table entries

END_EVENT_TABLE()

/*!
 * CDlgOptions constructors
 */

CDlgOptions::CDlgOptions() {
}

CDlgOptions::CDlgOptions(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
    Create(parent, id, caption, pos, size, style);
}


CDlgOptions::~CDlgOptions() {
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();

    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));

    wxGetApp().SaveState();
    pFrame->SaveState();

    wxConfigBase::Get(FALSE)->Flush();
}


/*!
 * CDlgToolsOptions creator
 */

bool CDlgOptions::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
////@begin CDlgOptions member initialisation
    m_LanguageSelectionCtrl = NULL;
    m_ReminderFrequencyCtrl = NULL;
    m_EnableBOINCManagerAutoStartCtrl = NULL;
    m_EnableRunDaemonCtrl = NULL;
    m_EnableBOINCManagerExitMessageCtrl = NULL;
    m_EnableBOINCClientShutdownMessageCtrl = NULL;
#if defined(__WXMAC__)
    m_EnableBOINCMenuBarIconCtrl = NULL;
#endif      // __WXMAC__
    m_DialupStaticBoxCtrl = NULL;
#if defined(__WXMSW__)
    m_DialupConnectionsCtrl = NULL;
    m_DialupSetDefaultCtrl = NULL;
    m_DialupClearDefaultCtrl = NULL;
    m_DialupDefaultConnectionTextCtrl = NULL;
    m_DialupDefaultConnectionCtrl = NULL;
#endif      // __WXMSW__
    m_EnableHTTPProxyCtrl = NULL;
    m_HTTPAddressCtrl = NULL;
    m_HTTPPortCtrl = NULL;
    m_HTTPUsernameCtrl = NULL;
    m_HTTPPasswordCtrl = NULL;
    m_EnableSOCKSProxyCtrl = NULL;
    m_SOCKSAddressCtrl = NULL;
    m_SOCKSPortCtrl = NULL;
    m_SOCKSUsernameCtrl = NULL;
    m_SOCKSPasswordCtrl = NULL;
	m_HTTPNoProxiesCtrl = NULL;
	m_SOCKSNoProxiesCtrl = NULL;
////@end CDlgOptions member initialisation
    m_bRetrievedProxyConfiguration = false;

    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
#if 1       // I don't think we want to include the application name here
        strCaption.Printf(_("Options"));
#else
        CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
        wxASSERT(pSkinAdvanced);
        wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        strCaption.Printf(_("Options"), pSkinAdvanced->GetApplicationShortName().c_str());
#endif
    }

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, strCaption, pos, size, style );

    CreateControls();

    ReadSettings();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    return TRUE;
}

/*!
 * Control creation for CDlgToolsOptions
 */

// NOTE: On MS Windows with wxWidgets 3.0, controls inside a wxStaticBox
// don't refresh properly unless they are children of the wxStaticBox!
//
void CDlgOptions::CreateControls() {
////@begin CDlgOptions content construction
    CDlgOptions* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxNotebook* itemNotebook3 = new wxNotebook;
    itemNotebook3->Create( itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_TOP );

    wxPanel* itemPanel4 = new wxPanel;
    itemPanel4->Create( itemNotebook3, ID_GENERAL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemPanel4->SetSizer(itemBoxSizer5);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(2, 0, 0);
    itemBoxSizer5->Add(itemFlexGridSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemPanel4, wxID_STATIC, _("Language:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    const std::vector<GUI_SUPPORTED_LANG>& langs = wxGetApp().GetSupportedLanguages();
    wxArrayString langLabels;
    for (const GUI_SUPPORTED_LANG& lang : langs) {
        langLabels.push_back(lang.Label);
    }
    m_LanguageSelectionCtrl = new wxComboBox;
    m_LanguageSelectionCtrl->Create( itemPanel4, ID_LANGUAGESELECTION, wxT(""), wxDefaultPosition, wxDefaultSize, langLabels, wxCB_READONLY );
    if (ShowToolTips())
        m_LanguageSelectionCtrl->SetToolTip(_("What language should BOINC use?"));
    itemFlexGridSizer6->Add(m_LanguageSelectionCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create( itemPanel4, wxID_STATIC, _("Notice reminder interval:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer6->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxString* m_ReminderFrequencyCtrlStrings = NULL;
    m_ReminderFrequencyCtrl = new wxComboBox;
    m_ReminderFrequencyCtrl->Create( itemPanel4, ID_REMINDERFREQUENCY, wxT(""), wxDefaultPosition, wxDefaultSize, 0, m_ReminderFrequencyCtrlStrings, wxCB_READONLY);
    if (ShowToolTips())
        m_ReminderFrequencyCtrl->SetToolTip(_("How often should BOINC remind you of new notices?"));
    itemFlexGridSizer6->Add(m_ReminderFrequencyCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifdef __WXMSW__
    wxStaticText* itemStaticText10 = new wxStaticText;
    itemStaticText10->Create( itemPanel4, wxID_STATIC, _("Run Manager at login?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EnableBOINCManagerAutoStartCtrl = new wxCheckBox;
    m_EnableBOINCManagerAutoStartCtrl->Create( itemPanel4, ID_ENABLEAUTOSTART, wxT(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    if (ShowToolTips())
        m_EnableBOINCManagerAutoStartCtrl->SetToolTip(_("Run the BOINC Manager when you log on."));
    itemFlexGridSizer6->Add(m_EnableBOINCManagerAutoStartCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

    wxStaticText* itemStaticText11 = new wxStaticText;
    itemStaticText11->Create( itemPanel4, wxID_STATIC, _("Run the client?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EnableRunDaemonCtrl = new wxCheckBox;
    m_EnableRunDaemonCtrl->Create( itemPanel4, ID_ENABLERUNDAEMON, wxT(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    if (ShowToolTips())
        m_EnableRunDaemonCtrl->SetToolTip(_("Run the client when launching the Manager."));
    itemFlexGridSizer6->Add(m_EnableRunDaemonCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText;
    itemStaticText12->Create( itemPanel4, wxID_STATIC, _("Enable Manager exit dialog?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EnableBOINCManagerExitMessageCtrl = new wxCheckBox;
    m_EnableBOINCManagerExitMessageCtrl->Create( itemPanel4, ID_ENABLEEXITMESSAGE, wxT(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    if (ShowToolTips())
        m_EnableBOINCManagerExitMessageCtrl->SetToolTip(_("Display the exit dialog when shutting down the Manager."));
    itemFlexGridSizer6->Add(m_EnableBOINCManagerExitMessageCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText;
    itemStaticText13->Create( itemPanel4, wxID_STATIC, _("Enable Client shutdown dialog?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EnableBOINCClientShutdownMessageCtrl = new wxCheckBox;
    m_EnableBOINCClientShutdownMessageCtrl->Create( itemPanel4, ID_ENABLESHUTDOWNMESSAGE, wxT(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    if (ShowToolTips())
        m_EnableBOINCClientShutdownMessageCtrl->SetToolTip(_("Display confirmation dialog when shutting down the connected client."));
    itemFlexGridSizer6->Add(m_EnableBOINCClientShutdownMessageCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifdef __WXMAC__
    wxStaticText* itemStaticText14 = new wxStaticText;
    itemStaticText14->Create( itemPanel4, wxID_STATIC, _("Show status in menu bar?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EnableBOINCMenuBarIconCtrl = new wxCheckBox;
    m_EnableBOINCMenuBarIconCtrl->Create( itemPanel4, ID_ENABLEMENUBARICON, wxT(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    if (ShowToolTips())
        m_EnableBOINCMenuBarIconCtrl->SetToolTip(_("Display a status icon in the system menu bar."));
    itemFlexGridSizer6->Add(m_EnableBOINCMenuBarIconCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif      // __WXMAC__

    itemNotebook3->AddPage(itemPanel4, _("General"));

#if defined(__WXMSW__)
    wxPanel* itemPanel11 = new wxPanel;
    itemPanel11->Create( itemNotebook3, ID_CONNECTONS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
    itemPanel11->SetSizer(itemBoxSizer12);

    wxStaticBox* itemStaticBox16 = new wxStaticBox(itemPanel11, wxID_ANY, _("Dial-up and Virtual Private Network settings"));
    m_DialupStaticBoxCtrl = new wxStaticBoxSizer(itemStaticBox16, wxVERTICAL);
    itemBoxSizer12->Add(m_DialupStaticBoxCtrl, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer17 = new wxFlexGridSizer(2, 0, 0);
    itemFlexGridSizer17->AddGrowableCol(0);
    m_DialupStaticBoxCtrl->Add(itemFlexGridSizer17, 0, wxGROW|wxALL, 5);
    wxString* m_DialupConnectionsCtrlStrings = NULL;
    m_DialupConnectionsCtrl = new wxListBox;
    m_DialupConnectionsCtrl->Create( itemStaticBox16, ID_DIALUPCONNECTIONS, wxDefaultPosition, wxSize(166, 185), 0, m_DialupConnectionsCtrlStrings, wxLB_SINGLE|wxLB_NEEDED_SB );
    itemFlexGridSizer17->Add(m_DialupConnectionsCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer17->Add(itemBoxSizer19, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_DialupSetDefaultCtrl = new wxButton;
    m_DialupSetDefaultCtrl->Create( itemStaticBox16, ID_DIALUPSETDEFAULT, _("&Set Default"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DialupSetDefaultCtrl->SetDefault();
    itemBoxSizer19->Add(m_DialupSetDefaultCtrl, 0, wxGROW|wxALL, 5);

    m_DialupClearDefaultCtrl = new wxButton;
    m_DialupClearDefaultCtrl->Create( itemStaticBox16, ID_DIALUPCLEARDEFAULT, _("&Clear Default"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer19->Add(m_DialupClearDefaultCtrl, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer22 = new wxFlexGridSizer(2, 0, 0);
    itemFlexGridSizer22->AddGrowableCol(1);
    m_DialupStaticBoxCtrl->Add(itemFlexGridSizer22, 0, wxGROW|wxALL, 5);
    m_DialupDefaultConnectionTextCtrl = new wxStaticText;
    m_DialupDefaultConnectionTextCtrl->Create( itemStaticBox16, ID_DIALUPDEFAULTCONNECTIONTEXT, _("Default Connection:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(m_DialupDefaultConnectionTextCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DialupDefaultConnectionCtrl = new wxStaticText;
    m_DialupDefaultConnectionCtrl->Create( itemStaticBox16, ID_DIALUPDEFAULTCONNECTION, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(m_DialupDefaultConnectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel11, _("Connections"));
#endif      // __WXMSW__

    wxPanel* itemPanel27 = new wxPanel;
    itemPanel27->Create( itemNotebook3, ID_HTTPPROXY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxVERTICAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    m_EnableHTTPProxyCtrl = new wxCheckBox;
    m_EnableHTTPProxyCtrl->Create( itemPanel27, ID_ENABLEHTTPPROXYCTRL, _("Connect via HTTP proxy server"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_EnableHTTPProxyCtrl->SetValue(FALSE);
    itemBoxSizer28->Add(m_EnableHTTPProxyCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBox30 = new wxStaticBox(itemPanel27, wxID_ANY, _("HTTP Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer30 = new wxStaticBoxSizer(itemStaticBox30, wxVERTICAL);
    itemBoxSizer28->Add(itemStaticBoxSizer30, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer31 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer30->Add(itemGridSizer31, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer32 = new wxFlexGridSizer(2, 0, 0);
    itemGridSizer31->Add(itemFlexGridSizer32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText33 = new wxStaticText;
    itemStaticText33->Create( itemStaticBox30, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HTTPAddressCtrl = new wxTextCtrl;
    m_HTTPAddressCtrl->Create( itemStaticBox30, ID_HTTPADDRESSCTRL, wxT(""), wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer32->Add(m_HTTPAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText35 = new wxStaticText;
    itemStaticText35->Create( itemStaticBox30, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText35, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HTTPPortCtrl = new wxTextCtrl;
    m_HTTPPortCtrl->Create( itemStaticBox30, ID_HTTPPORTCTRL, wxT(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer32->Add(m_HTTPPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText62 = new wxStaticText;
    itemStaticText62->Create( itemStaticBox30, wxID_STATIC, _("Don't use proxy for:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText62, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_HTTPNoProxiesCtrl = new wxTextCtrl;
	m_HTTPNoProxiesCtrl->Create(itemStaticBox30,ID_HTTPNOPROXYCTRL,wxT(""),wxDefaultPosition,wxSize(150,-1),0);
	itemFlexGridSizer32->Add(m_HTTPNoProxiesCtrl,0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBox37 = new wxStaticBox(itemStaticBox30, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer37 = new wxStaticBoxSizer(itemStaticBox37, wxVERTICAL);
    itemStaticBoxSizer30->Add(itemStaticBoxSizer37, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer38 = new wxFlexGridSizer(2, 0, 0);
    itemStaticBoxSizer37->Add(itemFlexGridSizer38, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText39 = new wxStaticText;
    itemStaticText39->Create( itemStaticBox37, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer38->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HTTPUsernameCtrl = new wxTextCtrl;
    m_HTTPUsernameCtrl->Create( itemStaticBox37, ID_HTTPUSERNAMECTRL, wxT(""), wxDefaultPosition, wxSize(175, -1), 0 );
    itemFlexGridSizer38->Add(m_HTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText41 = new wxStaticText;
    itemStaticText41->Create( itemStaticBox37, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer38->Add(itemStaticText41, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HTTPPasswordCtrl = new wxTextCtrl;
    m_HTTPPasswordCtrl->Create( itemStaticBox37, ID_HTTPPASSWORDCTRL, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer38->Add(m_HTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel27, _("HTTP Proxy"));

    wxPanel* itemPanel43 = new wxPanel;
    itemPanel43->Create( itemNotebook3, ID_SOCKSPROXY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer44 = new wxBoxSizer(wxVERTICAL);
    itemPanel43->SetSizer(itemBoxSizer44);

    m_EnableSOCKSProxyCtrl = new wxCheckBox;
    m_EnableSOCKSProxyCtrl->Create( itemPanel43, ID_ENABLESOCKSPROXYCTRL, _("Connect via SOCKS proxy server"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_EnableSOCKSProxyCtrl->SetValue(FALSE);
    itemBoxSizer44->Add(m_EnableSOCKSProxyCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBox46 = new wxStaticBox(itemPanel43, wxID_ANY, _("SOCKS Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer46 = new wxStaticBoxSizer(itemStaticBox46, wxVERTICAL);
    itemBoxSizer44->Add(itemStaticBoxSizer46, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer47 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer46->Add(itemGridSizer47, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer48 = new wxFlexGridSizer(2, 0, 0);
    itemGridSizer47->Add(itemFlexGridSizer48, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText49 = new wxStaticText;
    itemStaticText49->Create( itemStaticBox46, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer48->Add(itemStaticText49, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SOCKSAddressCtrl = new wxTextCtrl;
    m_SOCKSAddressCtrl->Create( itemStaticBox46, ID_SOCKSADDRESSCTRL, wxT(""), wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer48->Add(m_SOCKSAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText51 = new wxStaticText;
    itemStaticText51->Create( itemStaticBox46, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer48->Add(itemStaticText51, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SOCKSPortCtrl = new wxTextCtrl;
    m_SOCKSPortCtrl->Create( itemStaticBox46, ID_SOCKSPORTCTRL, wxT(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer48->Add(m_SOCKSPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText63 = new wxStaticText;
    itemStaticText63->Create( itemStaticBox46, wxID_STATIC, _("Don't use proxy for:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer48->Add(itemStaticText63, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_SOCKSNoProxiesCtrl = new wxTextCtrl;
	m_SOCKSNoProxiesCtrl->Create(itemStaticBox46,ID_SOCKSNOPROXYCTRL,wxT(""),wxDefaultPosition,wxSize(150,-1),0);
	itemFlexGridSizer48->Add(m_SOCKSNoProxiesCtrl,0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBox53 = new wxStaticBox(itemStaticBox46, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer53 = new wxStaticBoxSizer(itemStaticBox53, wxVERTICAL);
    itemStaticBoxSizer46->Add(itemStaticBoxSizer53, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer54 = new wxFlexGridSizer(2, 0, 0);
    itemStaticBoxSizer53->Add(itemFlexGridSizer54, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText55 = new wxStaticText;
    itemStaticText55->Create( itemStaticBox53, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer54->Add(itemStaticText55, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SOCKSUsernameCtrl = new wxTextCtrl;
    m_SOCKSUsernameCtrl->Create( itemStaticBox53, ID_SOCKSUSERNAMECTRL, wxT(""), wxDefaultPosition, wxSize(175, -1), 0 );
    itemFlexGridSizer54->Add(m_SOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText57 = new wxStaticText;
    itemStaticText57->Create( itemStaticBox53, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer54->Add(itemStaticText57, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SOCKSPasswordCtrl = new wxTextCtrl;
    m_SOCKSPasswordCtrl->Create( itemStaticBox53, ID_SOCKSPASSWORDCTRL, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer54->Add(m_SOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel43, _("SOCKS Proxy"));

    itemBoxSizer2->Add(itemNotebook3, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer59 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer59, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* btnSave = new wxButton;
    btnSave->Create( itemDialog1, wxID_OK, _("&Save"), wxDefaultPosition, wxDefaultSize, 0 );
    btnSave->SetToolTip( _("Save all values and close the dialog") );
    itemBoxSizer59->Add(btnSave, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* btnCancel = new wxButton;
    btnCancel->Create( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    btnCancel->SetToolTip( _("Close the dialog without saving") );
    btnCancel->SetDefault();
    itemBoxSizer59->Add(btnCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators

////@end CDlgOptions content construction
}


#if defined(__WXMSW__)
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DIALUPSETDEFAULT
 */

void CDlgOptions::OnDialupSetDefaultClick( wxCommandEvent& WXUNUSED(event) ) {
    m_DialupDefaultConnectionCtrl->SetLabel(m_DialupConnectionsCtrl->GetStringSelection());
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DIALUPCLEARDEFAULT
 */

void CDlgOptions::OnDialupClearDefaultClick( wxCommandEvent& WXUNUSED(event) ) {
    m_DialupDefaultConnectionCtrl->SetLabel(wxEmptyString);
}
#endif      // __WXMSW__


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ENABLEHTTPPROXYCTRL
 */

void CDlgOptions::OnEnableHTTPProxyCtrlClick(wxCommandEvent& event) {
    if (event.IsChecked()) {
        m_HTTPAddressCtrl->Enable(true);
        m_HTTPPortCtrl->Enable(true);
        m_HTTPUsernameCtrl->Enable(true);
        m_HTTPPasswordCtrl->Enable(true);
		m_HTTPNoProxiesCtrl->Enable(true);
    } else {
        m_HTTPAddressCtrl->Enable(false);
        m_HTTPPortCtrl->Enable(false);
        m_HTTPUsernameCtrl->Enable(false);
        m_HTTPPasswordCtrl->Enable(false);
		m_HTTPNoProxiesCtrl->Enable(false);
    }

    event.Skip();
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_ENABLEHTTPPROXYCTRL
 */

void CDlgOptions::OnEnableHTTPProxyCtrlUpdate(wxUpdateUIEvent& event) {
    if (m_EnableHTTPProxyCtrl->IsEnabled() && m_EnableHTTPProxyCtrl->IsChecked()) {
        m_HTTPAddressCtrl->Enable(true);
        m_HTTPPortCtrl->Enable(true);
        m_HTTPUsernameCtrl->Enable(true);
        m_HTTPPasswordCtrl->Enable(true);
		m_HTTPNoProxiesCtrl->Enable(true);
    } else {
        m_HTTPAddressCtrl->Enable(false);
        m_HTTPPortCtrl->Enable(false);
        m_HTTPUsernameCtrl->Enable(false);
        m_HTTPPasswordCtrl->Enable(false);
		m_HTTPNoProxiesCtrl->Enable(false);
    }
    event.Skip();
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ENABLESOCKSPROXYCTRL
 */

void CDlgOptions::OnEnableSOCKSProxyCtrlClick(wxCommandEvent& event) {
    if (event.IsChecked()) {
        m_SOCKSAddressCtrl->Enable(true);
        m_SOCKSPortCtrl->Enable(true);
        m_SOCKSUsernameCtrl->Enable(true);
        m_SOCKSPasswordCtrl->Enable(true);
		m_SOCKSNoProxiesCtrl->Enable(true);
    } else {
        m_SOCKSAddressCtrl->Enable(false);
        m_SOCKSPortCtrl->Enable(false);
        m_SOCKSUsernameCtrl->Enable(false);
        m_SOCKSPasswordCtrl->Enable(false);
		m_SOCKSNoProxiesCtrl->Enable(false);
    }
    event.Skip();
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_ENABLESOCKSPROXYCTRL
 */

void CDlgOptions::OnEnableSOCKSProxyCtrlUpdate(wxUpdateUIEvent& event) {
    if (m_EnableSOCKSProxyCtrl->IsEnabled() && m_EnableSOCKSProxyCtrl->IsChecked()) {
        m_SOCKSAddressCtrl->Enable(true);
        m_SOCKSPortCtrl->Enable(true);
        m_SOCKSUsernameCtrl->Enable(true);
        m_SOCKSPasswordCtrl->Enable(true);
		m_SOCKSNoProxiesCtrl->Enable(true);
    } else {
        m_SOCKSAddressCtrl->Enable(false);
        m_SOCKSPortCtrl->Enable(false);
        m_SOCKSUsernameCtrl->Enable(false);
        m_SOCKSPasswordCtrl->Enable(false);
		m_SOCKSNoProxiesCtrl->Enable(false);
    }
    event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CDlgOptions::OnOK( wxCommandEvent& WXUNUSED(event) ) {
    SaveSettings();
    EndModal(wxID_OK);
}


/*!
 * Should we show tooltips?
 */

bool CDlgOptions::ShowToolTips() {
    return TRUE;
}


/*!
 * Get bitmap resources
 */

wxBitmap CDlgOptions::GetBitmapResource( const wxString& WXUNUSED(name) ) {
    return wxNullBitmap;
}


/*!
 * Get icon resources
 */

wxIcon CDlgOptions::GetIconResource( const wxString& WXUNUSED(name) ) {
    return wxNullIcon;
}


#ifdef __WXMSW__

wxString CDlgOptions::GetDefaultDialupConnection() const {
    return m_DialupDefaultConnectionCtrl->GetLabel();
}

void CDlgOptions::SetDefaultDialupConnection(wxString value) {
    m_DialupDefaultConnectionCtrl->SetLabel(value);
}

#endif


bool CDlgOptions::ReadSettings() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();
    wxString            strBuffer = wxEmptyString;
#ifdef __WXMSW__
    wxArrayString       astrDialupConnections;
#endif

    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));

    // General Tab
    if (wxGetApp().UseDefaultLocale()) {
        // CBOINCGUIApp::InitSupportedLanguages() ensures "Auto" is the first item in the list
        m_LanguageSelectionCtrl->SetSelection(0);
    } else {
        const wxLanguageInfo* pLI = wxLocale::FindLanguageInfo(wxGetApp().GetISOLanguageCode());
        if (pLI) {
            const std::vector<GUI_SUPPORTED_LANG>& langs = wxGetApp().GetSupportedLanguages();
            for (std::vector<GUI_SUPPORTED_LANG>::const_iterator foundit = langs.begin();
                 foundit != langs.end(); ++foundit) {
                const GUI_SUPPORTED_LANG& item = *foundit;
                if (item.Language == pLI->Language) {
                    int selLangIdx = std::distance(langs.begin(), foundit);
                    m_LanguageSelectionCtrl->SetSelection(selLangIdx);
                    break;
                }
            }
        }
    }

    m_ReminderFrequencyCtrl->Append(_("always"));
    m_ReminderFrequencyCtrl->Append(_("1 hour"));
    m_ReminderFrequencyCtrl->Append(_("6 hours"));
    m_ReminderFrequencyCtrl->Append(_("1 day"));
    m_ReminderFrequencyCtrl->Append(_("1 week"));
    m_ReminderFrequencyCtrl->Append(_("never"));

    switch(pFrame->GetReminderFrequency()) {
        case 1:
            m_ReminderFrequencyCtrl->SetSelection(0);
            break;
        case 60:
            m_ReminderFrequencyCtrl->SetSelection(1);
            break;
        case 360:
            m_ReminderFrequencyCtrl->SetSelection(2);
            break;
        case 1440:
            m_ReminderFrequencyCtrl->SetSelection(3);
            break;
        case 10080:
            m_ReminderFrequencyCtrl->SetSelection(4);
            break;
        case 0:
            m_ReminderFrequencyCtrl->SetSelection(5);
            break;
    }

    //m_ReminderFrequencyCtrl->SetValue(m_iReminderFrequency);

    m_EnableBOINCManagerExitMessageCtrl->SetValue(wxGetApp().GetBOINCMGRDisplayExitMessage() != 0);
    m_EnableBOINCClientShutdownMessageCtrl->SetValue(wxGetApp().GetBOINCMGRDisplayShutdownConnectedClientMessage() != 0);
#ifdef __WXMAC__
    m_EnableBOINCMenuBarIconCtrl->SetValue(wxGetApp().GetBOINCMGRHideMenuBarIcon() == 0);
#endif
#ifdef __WXMSW__
    m_EnableBOINCManagerAutoStartCtrl->SetValue(!wxGetApp().GetBOINCMGRDisableAutoStart());

    // Connection Tab
    if (pFrame) {
        pFrame->GetDialupManager()->GetISPNames(astrDialupConnections);

        m_DialupConnectionsCtrl->Append(astrDialupConnections);
        SetDefaultDialupConnection(pFrame->GetDialupConnectionName());
    } else {
        m_DialupSetDefaultCtrl->Disable();
        m_DialupClearDefaultCtrl->Disable();
    }
#endif
    m_EnableRunDaemonCtrl->SetValue(wxGetApp().GetRunDaemon());

    // Proxy Tabs
    m_bRetrievedProxyConfiguration = (0 == pDoc->GetProxyConfiguration());
    if(!m_bRetrievedProxyConfiguration) {
        // We were unable to get the proxy configuration, so disable
        //   the controls
        m_EnableHTTPProxyCtrl->Enable(false);
        m_EnableSOCKSProxyCtrl->Enable(false);
    } else {
        m_EnableHTTPProxyCtrl->Enable(true);
        m_EnableSOCKSProxyCtrl->Enable(true);
    }

    m_EnableHTTPProxyCtrl->SetValue(pDoc->proxy_info.use_http_proxy);
    m_HTTPAddressCtrl->SetValue(wxString(pDoc->proxy_info.http_server_name.c_str(), wxConvUTF8));
    m_HTTPUsernameCtrl->SetValue(wxString(pDoc->proxy_info.http_user_name.c_str(), wxConvUTF8));
    m_HTTPPasswordCtrl->SetValue(wxString(pDoc->proxy_info.http_user_passwd.c_str(), wxConvUTF8));
	m_HTTPNoProxiesCtrl->SetValue(wxString(pDoc->proxy_info.noproxy_hosts.c_str(), wxConvUTF8));
    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.http_server_port);
    m_HTTPPortCtrl->SetValue(strBuffer);

    m_EnableSOCKSProxyCtrl->SetValue(pDoc->proxy_info.use_socks_proxy);
    m_SOCKSAddressCtrl->SetValue(wxString(pDoc->proxy_info.socks_server_name.c_str(), wxConvUTF8));
    m_SOCKSUsernameCtrl->SetValue(wxString(pDoc->proxy_info.socks5_user_name.c_str(), wxConvUTF8));
    m_SOCKSPasswordCtrl->SetValue(wxString(pDoc->proxy_info.socks5_user_passwd.c_str(), wxConvUTF8));
	m_SOCKSNoProxiesCtrl->SetValue(wxString(pDoc->proxy_info.noproxy_hosts.c_str(),wxConvUTF8));
    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.socks_server_port);
    m_SOCKSPortCtrl->SetValue(strBuffer);

    return true;
}


bool CDlgOptions::SaveSettings() {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CBOINCBaseFrame*    pFrame = wxGetApp().GetFrame();
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    long                lBuffer = 0;
    wxString            strBuffer = wxEmptyString;
    const wxLanguageInfo *newLanguageInfo = NULL;

    wxASSERT(pDoc);
    wxASSERT(pFrame);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    // General Tab
    wxString oldLangCode = wxGetApp().GetISOLanguageCode();
    wxString newLangCode = oldLangCode;
    int selLangIdx = m_LanguageSelectionCtrl->GetSelection();
    if (selLangIdx == 0) {
        // CBOINCGUIApp::InitSupportedLanguages() ensures "Auto" is the first item in the list
        newLanguageInfo = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);
        // wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT) may return NULL on Macintosh
        newLangCode = wxEmptyString;
    } else if (selLangIdx > 0) {
        const std::vector<GUI_SUPPORTED_LANG>& langs = wxGetApp().GetSupportedLanguages();
        if (selLangIdx < langs.size()) {
            const GUI_SUPPORTED_LANG& selLang = langs[selLangIdx];
            newLanguageInfo = wxLocale::GetLanguageInfo(selLang.Language);
        }
    }
    if (newLanguageInfo) {
        newLangCode = newLanguageInfo->CanonicalName;
    }
    if (newLangCode != oldLangCode) {
        wxString strDialogTitle;
        wxString strDialogMessage;

        // %s is the application name
        //    i.e. 'BOINC Manager', 'GridRepublic Manager'
        strDialogTitle.Printf(
            _("%s - Language Selection"),
            pSkinAdvanced->GetApplicationName().c_str()
        );

        // %s is the application name
        //    i.e. 'BOINC Manager', 'GridRepublic Manager'
        strDialogMessage.Printf(
            _("The %s's language has been changed.  In order for this change to take effect, you must restart the %s."),
            pSkinAdvanced->GetApplicationName().c_str(),
            pSkinAdvanced->GetApplicationName().c_str()
        );

        pFrame->ShowAlert(
            strDialogTitle,
            strDialogMessage,
            wxOK | wxICON_INFORMATION
        );
    }

    wxGetApp().SetISOLanguageCode(newLangCode);
    wxGetApp().SetUseDefaultLocale(selLangIdx == 0);

    switch(m_ReminderFrequencyCtrl->GetSelection()) {
        case 0:
            pFrame->SetReminderFrequency(1);
            break;
        case 1:
            pFrame->SetReminderFrequency(60);
            break;
        case 2:
            pFrame->SetReminderFrequency(360);
            break;
        case 3:
            pFrame->SetReminderFrequency(1440);
            break;
        case 4:
            pFrame->SetReminderFrequency(10080);
            break;
        case 5:
            pFrame->SetReminderFrequency(0);
            break;
    }

    wxGetApp().SetBOINCMGRDisplayExitMessage(m_EnableBOINCManagerExitMessageCtrl->GetValue());
    wxGetApp().SetBOINCMGRDisplayShutdownConnectedClientMessage(m_EnableBOINCClientShutdownMessageCtrl->GetValue());
#ifdef __WXMAC__
    wxGetApp().SetBOINCMGRHideMenuBarIcon(!m_EnableBOINCMenuBarIconCtrl->GetValue());
#endif
#ifdef __WXMSW__
    wxGetApp().SetBOINCMGRDisableAutoStart(!m_EnableBOINCManagerAutoStartCtrl->GetValue());

    // Connection Tab
    pFrame->SetDialupConnectionName(GetDefaultDialupConnection());
#endif
    wxGetApp().SetRunDaemon(m_EnableRunDaemonCtrl->GetValue());

    // Proxy Tabs
    if (m_bRetrievedProxyConfiguration) {
        pDoc->proxy_info.use_http_proxy = m_EnableHTTPProxyCtrl->GetValue();
        pDoc->proxy_info.http_server_name = (const char*)m_HTTPAddressCtrl->GetValue().mb_str();
        pDoc->proxy_info.http_user_name = (const char*)m_HTTPUsernameCtrl->GetValue().mb_str();
        pDoc->proxy_info.http_user_passwd = (const char*)m_HTTPPasswordCtrl->GetValue().mb_str();
		if(pDoc->proxy_info.use_http_proxy) {
			pDoc->proxy_info.noproxy_hosts = (const char*)m_HTTPNoProxiesCtrl->GetValue().mb_str();
		}
        strBuffer = m_HTTPPortCtrl->GetValue();
        strBuffer.ToLong((long*)&lBuffer);
        pDoc->proxy_info.http_server_port = lBuffer;

        pDoc->proxy_info.use_socks_proxy = m_EnableSOCKSProxyCtrl->GetValue();
        pDoc->proxy_info.socks_server_name = (const char*)m_SOCKSAddressCtrl->GetValue().mb_str();
        pDoc->proxy_info.socks5_user_name = (const char*)m_SOCKSUsernameCtrl->GetValue().mb_str();
        pDoc->proxy_info.socks5_user_passwd = (const char*)m_SOCKSPasswordCtrl->GetValue().mb_str();
		if(pDoc->proxy_info.use_socks_proxy) {
			pDoc->proxy_info.noproxy_hosts = (const char*)m_SOCKSNoProxiesCtrl->GetValue().mb_str();
		}
        strBuffer = m_SOCKSPortCtrl->GetValue();
        strBuffer.ToLong((long*)&lBuffer);
        pDoc->proxy_info.socks_server_port = lBuffer;

        pDoc->SetProxyConfiguration();
    }

    return true;
}


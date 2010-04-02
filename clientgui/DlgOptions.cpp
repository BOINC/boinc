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
    EVT_NOTEBOOK_PAGE_CHANGED( ID_NOTEBOOK, CDlgOptions::OnNotebookPageChanged )
    EVT_UPDATE_UI( ID_NOTEBOOK, CDlgOptions::OnNotebookUpdate )
#if defined(__WXMSW__)
    EVT_BUTTON( ID_DIALUPSETDEFAULT, CDlgOptions::OnDialupSetDefaultClick )
    EVT_BUTTON( ID_DIALUPCLEARDEFAULT, CDlgOptions::OnDialupClearDefaultClick )
#endif      // __WXMSW__
    EVT_CHECKBOX( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnableHTTPProxyCtrlClick )
    EVT_UPDATE_UI( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnableHTTPProxyCtrlUpdate )
    EVT_CHECKBOX( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnableSOCKSProxyCtrlClick )
    EVT_UPDATE_UI( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnableSOCKSProxyCtrlUpdate )
////@end CDlgOptions event table entries

END_EVENT_TABLE()

/*!
 * CDlgOptions constructors
 */

CDlgOptions::CDlgOptions()
{
}

CDlgOptions::CDlgOptions(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgToolsOptions creator
 */

bool CDlgOptions::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
////@begin CDlgOptions member initialisation
    m_LanguageSelectionCtrl = NULL;
    m_ReminderFrequencyCtrl = NULL;
    m_EnableBOINCManagerAutoStartCtrl = NULL;
    m_EnableBOINCManagerExitMessageCtrl = NULL;
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

    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
        wxASSERT(pSkinAdvanced);
        wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        strCaption.Printf(_("%s - Options"), pSkinAdvanced->GetApplicationName().c_str());
    }

////@begin CDlgOptions creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, strCaption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgOptions creation
    return TRUE;
}

/*!
 * Control creation for CDlgToolsOptions
 */

void CDlgOptions::CreateControls()
{    
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

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(3, 2, 0, 0);
    itemBoxSizer5->Add(itemFlexGridSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemPanel4, wxID_STATIC, _("Language:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxString* m_LanguageSelectionCtrlStrings = NULL;
    m_LanguageSelectionCtrl = new wxComboBox;
    m_LanguageSelectionCtrl->Create( itemPanel4, ID_LANGUAGESELECTION, _T(""), wxDefaultPosition, wxDefaultSize, 0, m_LanguageSelectionCtrlStrings, wxCB_READONLY );
    if (ShowToolTips())
        m_LanguageSelectionCtrl->SetToolTip(_("What language should BOINC use?"));
    itemFlexGridSizer6->Add(m_LanguageSelectionCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create( itemPanel4, wxID_STATIC, _("Notice reminder interval:\n(minutes; 0 means no reminders)"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer6->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ReminderFrequencyCtrl = new wxSlider;
    m_ReminderFrequencyCtrl->Create( itemPanel4, ID_REMINDERFREQUENCY, 60, 0, 120, wxDefaultPosition,
#if defined(__WXMSW__) || defined(__WXMAC__)        
                                     wxDefaultSize, 
#else
                                     wxSize(150, 40),
#endif
                                     wxSL_HORIZONTAL|wxSL_LABELS);
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
    itemStaticText11->Create( itemPanel4, wxID_STATIC, _("Enable Manager exit dialog?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EnableBOINCManagerExitMessageCtrl = new wxCheckBox;
    m_EnableBOINCManagerExitMessageCtrl->Create( itemPanel4, ID_ENABLEEXITMESSAGE, wxT(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    if (ShowToolTips())
        m_EnableBOINCManagerExitMessageCtrl->SetToolTip(_("Display the exit dialog when shutting down the Manager."));
    itemFlexGridSizer6->Add(m_EnableBOINCManagerExitMessageCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel4, _("General"));

#if defined(__WXMSW__)
    wxPanel* itemPanel11 = new wxPanel;
    itemPanel11->Create( itemNotebook3, ID_CONNECTONS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
    itemPanel11->SetSizer(itemBoxSizer12);

    wxStaticBox* itemStaticBoxSizer16Static = new wxStaticBox(itemPanel11, wxID_ANY, _("Dial-up and Virtual Private Network settings"));
    m_DialupStaticBoxCtrl = new wxStaticBoxSizer(itemStaticBoxSizer16Static, wxVERTICAL);
    itemBoxSizer12->Add(m_DialupStaticBoxCtrl, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer17 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer17->AddGrowableCol(0);
    m_DialupStaticBoxCtrl->Add(itemFlexGridSizer17, 0, wxGROW|wxALL, 5);
    wxString* m_DialupConnectionsCtrlStrings = NULL;
    m_DialupConnectionsCtrl = new wxListBox;
    m_DialupConnectionsCtrl->Create( itemPanel11, ID_DIALUPCONNECTIONS, wxDefaultPosition, wxSize(166, 185), 0, m_DialupConnectionsCtrlStrings, wxLB_SINGLE|wxLB_NEEDED_SB );
    itemFlexGridSizer17->Add(m_DialupConnectionsCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer17->Add(itemBoxSizer19, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_DialupSetDefaultCtrl = new wxButton;
    m_DialupSetDefaultCtrl->Create( itemPanel11, ID_DIALUPSETDEFAULT, _("&Set Default"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DialupSetDefaultCtrl->SetDefault();
    itemBoxSizer19->Add(m_DialupSetDefaultCtrl, 0, wxGROW|wxALL, 5);

    m_DialupClearDefaultCtrl = new wxButton;
    m_DialupClearDefaultCtrl->Create( itemPanel11, ID_DIALUPCLEARDEFAULT, _("&Clear Default"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer19->Add(m_DialupClearDefaultCtrl, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer22 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer22->AddGrowableCol(1);
    m_DialupStaticBoxCtrl->Add(itemFlexGridSizer22, 0, wxGROW|wxALL, 5);
    m_DialupDefaultConnectionTextCtrl = new wxStaticText;
    m_DialupDefaultConnectionTextCtrl->Create( itemPanel11, ID_DIALUPDEFAULTCONNECTIONTEXT, _("Default Connection:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(m_DialupDefaultConnectionTextCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DialupDefaultConnectionCtrl = new wxStaticText;
    m_DialupDefaultConnectionCtrl->Create( itemPanel11, ID_DIALUPDEFAULTCONNECTION, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
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

    wxStaticBox* itemStaticBoxSizer30Static = new wxStaticBox(itemPanel27, wxID_ANY, _("HTTP Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer30 = new wxStaticBoxSizer(itemStaticBoxSizer30Static, wxVERTICAL);
    itemBoxSizer28->Add(itemStaticBoxSizer30, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer31 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer30->Add(itemGridSizer31, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer32 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer31->Add(itemFlexGridSizer32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText33 = new wxStaticText;
    itemStaticText33->Create( itemPanel27, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HTTPAddressCtrl = new wxTextCtrl;
    m_HTTPAddressCtrl->Create( itemPanel27, ID_HTTPADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer32->Add(m_HTTPAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText35 = new wxStaticText;
    itemStaticText35->Create( itemPanel27, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText35, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HTTPPortCtrl = new wxTextCtrl;
    m_HTTPPortCtrl->Create( itemPanel27, ID_HTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer32->Add(m_HTTPPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText62 = new wxStaticText;
    itemStaticText62->Create( itemPanel27, wxID_STATIC, _("Don't use proxy for:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText62, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_HTTPNoProxiesCtrl = new wxTextCtrl;
	m_HTTPNoProxiesCtrl->Create(itemPanel27,ID_HTTPNOPROXYCTRL,_T(""),wxDefaultPosition,wxSize(150,-1),0);
	itemFlexGridSizer32->Add(m_HTTPNoProxiesCtrl,0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer37Static = new wxStaticBox(itemPanel27, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer37 = new wxStaticBoxSizer(itemStaticBoxSizer37Static, wxVERTICAL);
    itemStaticBoxSizer30->Add(itemStaticBoxSizer37, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer38 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer37->Add(itemFlexGridSizer38, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText39 = new wxStaticText;
    itemStaticText39->Create( itemPanel27, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer38->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HTTPUsernameCtrl = new wxTextCtrl;
    m_HTTPUsernameCtrl->Create( itemPanel27, ID_HTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    itemFlexGridSizer38->Add(m_HTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText41 = new wxStaticText;
    itemStaticText41->Create( itemPanel27, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer38->Add(itemStaticText41, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HTTPPasswordCtrl = new wxTextCtrl;
    m_HTTPPasswordCtrl->Create( itemPanel27, ID_HTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
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

    wxStaticBox* itemStaticBoxSizer46Static = new wxStaticBox(itemPanel43, wxID_ANY, _("SOCKS Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer46 = new wxStaticBoxSizer(itemStaticBoxSizer46Static, wxVERTICAL);
    itemBoxSizer44->Add(itemStaticBoxSizer46, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer47 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer46->Add(itemGridSizer47, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer48 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer47->Add(itemFlexGridSizer48, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText49 = new wxStaticText;
    itemStaticText49->Create( itemPanel43, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer48->Add(itemStaticText49, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SOCKSAddressCtrl = new wxTextCtrl;
    m_SOCKSAddressCtrl->Create( itemPanel43, ID_SOCKSADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer48->Add(m_SOCKSAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText51 = new wxStaticText;
    itemStaticText51->Create( itemPanel43, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer48->Add(itemStaticText51, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SOCKSPortCtrl = new wxTextCtrl;
    m_SOCKSPortCtrl->Create( itemPanel43, ID_SOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer48->Add(m_SOCKSPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText63 = new wxStaticText;
    itemStaticText63->Create( itemPanel43, wxID_STATIC, _("Don't use proxy for:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer48->Add(itemStaticText63, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_SOCKSNoProxiesCtrl = new wxTextCtrl;
	m_SOCKSNoProxiesCtrl->Create(itemPanel43,ID_SOCKSNOPROXYCTRL,_T(""),wxDefaultPosition,wxSize(150,-1),0);
	itemFlexGridSizer48->Add(m_SOCKSNoProxiesCtrl,0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer53Static = new wxStaticBox(itemPanel43, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer53 = new wxStaticBoxSizer(itemStaticBoxSizer53Static, wxVERTICAL);
    itemStaticBoxSizer46->Add(itemStaticBoxSizer53, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer54 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer53->Add(itemFlexGridSizer54, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText55 = new wxStaticText;
    itemStaticText55->Create( itemPanel43, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer54->Add(itemStaticText55, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SOCKSUsernameCtrl = new wxTextCtrl;
    m_SOCKSUsernameCtrl->Create( itemPanel43, ID_SOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    itemFlexGridSizer54->Add(m_SOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText57 = new wxStaticText;
    itemStaticText57->Create( itemPanel43, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer54->Add(itemStaticText57, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SOCKSPasswordCtrl = new wxTextCtrl;
    m_SOCKSPasswordCtrl->Create( itemPanel43, ID_SOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer54->Add(m_SOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel43, _("SOCKS Proxy"));

    itemBoxSizer2->Add(itemNotebook3, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer59 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer59, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton60 = new wxButton;
    itemButton60->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton60->SetDefault();
    itemBoxSizer59->Add(itemButton60, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton61 = new wxButton;
    itemButton61->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer59->Add(itemButton61, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CDlgOptions content construction
}


/*!
 * wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED event handler for ID_NOTEBOOK
 */

void CDlgOptions::OnNotebookPageChanged(wxNotebookEvent& event)
{
    event.Skip();
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_NOTEBOOK
 */

void CDlgOptions::OnNotebookUpdate(wxUpdateUIEvent& event)
{
    event.Skip();
}


#if defined(__WXMSW__)
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DIALUPSETDEFAULT
 */

void CDlgOptions::OnDialupSetDefaultClick( wxCommandEvent& WXUNUSED(event) )
{
    m_DialupDefaultConnectionCtrl->SetLabel(m_DialupConnectionsCtrl->GetStringSelection());
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DIALUPCLEARDEFAULT
 */

void CDlgOptions::OnDialupClearDefaultClick( wxCommandEvent& WXUNUSED(event) )
{
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


#if defined(__WXMSW__)
wxString CDlgOptions::GetDefaultDialupConnection() const
{
    return m_DialupDefaultConnectionCtrl->GetLabel(); 
}


void CDlgOptions::SetDefaultDialupConnection(wxString value)
{
    m_DialupDefaultConnectionCtrl->SetLabel(value);
}
#endif      // __WXMSW__


/*!
 * Should we show tooltips?
 */

bool CDlgOptions::ShowToolTips()
{
    return TRUE;
}


/*!
 * Get bitmap resources
 */

wxBitmap CDlgOptions::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CDlgOptions bitmap retrieval
    return wxNullBitmap;
////@end CDlgOptions bitmap retrieval
}


/*!
 * Get icon resources
 */

wxIcon CDlgOptions::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CDlgOptions icon retrieval
    return wxNullIcon;
////@end CDlgOptions icon retrieval
}


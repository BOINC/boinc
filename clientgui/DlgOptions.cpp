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
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgOptions.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "DlgOptions.h"

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
    EVT_RADIOBUTTON( ID_NETWORKAUTODETECT, CDlgOptions::OnNetworkAutoDetectSelected )
    EVT_UPDATE_UI( ID_NETWORKAUTODETECT, CDlgOptions::OnNetworkAutoDetectUpdate )
#endif

#if defined(__WXMSW__)
    EVT_RADIOBUTTON( ID_NETWORKLAN, CDlgOptions::OnNetworkLANSelected )
    EVT_UPDATE_UI( ID_NETWORKLAN, CDlgOptions::OnNetworkLANUpdate )
#endif

#if defined(__WXMSW__)
    EVT_RADIOBUTTON( ID_NETWORKDIALUP, CDlgOptions::OnNetworkDialupSelected )
    EVT_UPDATE_UI( ID_NETWORKDIALUP, CDlgOptions::OnNetworkDialupUpdate )
#endif

#if defined(__WXMSW__)
    EVT_BUTTON( ID_DIALUPSETDEFAULT, CDlgOptions::OnDialupSetDefaultClick )
#endif

#if defined(__WXMSW__)
    EVT_BUTTON( ID_DIALUPCLEARDEFAULT, CDlgOptions::OnDialupClearDefaultClick )
#endif

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
#if defined(__WXMSW__)
    m_NetworkAutomaticDetectionCtrl = NULL;
#endif
#if defined(__WXMSW__)
    m_NetworkUseLANCtrl = NULL;
#endif
#if defined(__WXMSW__)
    m_NetworkUseDialupCtrl = NULL;
#endif
#if defined(__WXMSW__)
    m_DialupStaticBoxCtrl = NULL;
#endif
#if defined(__WXMSW__)
    m_DialupConnectionsCtrl = NULL;
#endif
#if defined(__WXMSW__)
    m_DialupSetDefaultCtrl = NULL;
#endif
#if defined(__WXMSW__)
    m_DialupClearDefaultCtrl = NULL;
#endif
#if defined(__WXMSW__)
    m_DialupDefaultConnectionTextCtrl = NULL;
#endif
#if defined(__WXMSW__)
    m_DialupDefaultConnectionCtrl = NULL;
#endif
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
////@end CDlgOptions member initialisation

////@begin CDlgOptions creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

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
#if !wxCHECK_VERSION(2,5,2)
    wxNotebookSizer* itemNotebook3Sizer = new wxNotebookSizer(itemNotebook3);
#endif

    wxPanel* itemPanel4 = new wxPanel;
    itemPanel4->Create( itemNotebook3, ID_GENERAL, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemPanel4->SetSizer(itemBoxSizer5);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer5->Add(itemFlexGridSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemPanel4, wxID_STATIC, _("Language Selection:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString* m_LanguageSelectionCtrlStrings = NULL;
    m_LanguageSelectionCtrl = new wxComboBox;
    m_LanguageSelectionCtrl->Create( itemPanel4, ID_LANGUAGESELECTION, _T(""), wxDefaultPosition, wxDefaultSize, 0, m_LanguageSelectionCtrlStrings, wxCB_READONLY );
    if (ShowToolTips())
        m_LanguageSelectionCtrl->SetToolTip(_("What language should the manager display by default."));
    itemFlexGridSizer6->Add(m_LanguageSelectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create( itemPanel4, wxID_STATIC, _("Reminder Frequency:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_ReminderFrequencyCtrl = new wxSlider;
    m_ReminderFrequencyCtrl->Create( itemPanel4, ID_REMINDERFREQUENCY, 60, 0, 120, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
    if (ShowToolTips())
        m_ReminderFrequencyCtrl->SetToolTip(_("How often, in minutes, should the manager remind you of possible connection events."));
    itemFlexGridSizer6->Add(m_ReminderFrequencyCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel4, _("General"));

#if defined(__WXMSW__)
    wxPanel* itemPanel11 = new wxPanel;
    itemPanel11->Create( itemNotebook3, ID_CONNECTONS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
    itemPanel11->SetSizer(itemBoxSizer12);

    m_NetworkAutomaticDetectionCtrl = new wxRadioButton;
    m_NetworkAutomaticDetectionCtrl->Create( itemPanel11, ID_NETWORKAUTODETECT, _("&Automatically detect network connection settings"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_NetworkAutomaticDetectionCtrl->SetValue(TRUE);
    itemBoxSizer12->Add(m_NetworkAutomaticDetectionCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_NetworkUseLANCtrl = new wxRadioButton;
    m_NetworkUseLANCtrl->Create( itemPanel11, ID_NETWORKLAN, _("Use my &Local Area Network(LAN) connection"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NetworkUseLANCtrl->SetValue(FALSE);
    itemBoxSizer12->Add(m_NetworkUseLANCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_NetworkUseDialupCtrl = new wxRadioButton;
    m_NetworkUseDialupCtrl->Create( itemPanel11, ID_NETWORKDIALUP, _("Use my &Dial-up and Virtual Private Network connection"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NetworkUseDialupCtrl->SetValue(FALSE);
    itemBoxSizer12->Add(m_NetworkUseDialupCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer16Static = new wxStaticBox(itemPanel11, wxID_ANY, _("Dial-up and Virtual Private Network settings"));
    m_DialupStaticBoxCtrl = new wxStaticBoxSizer(itemStaticBoxSizer16Static, wxVERTICAL);
    itemBoxSizer12->Add(m_DialupStaticBoxCtrl, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer17 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer17->AddGrowableCol(0);
    m_DialupStaticBoxCtrl->Add(itemFlexGridSizer17, 0, wxGROW|wxALL, 5);
    wxString* m_DialupConnectionsCtrlStrings = NULL;
    m_DialupConnectionsCtrl = new wxListBox;
    m_DialupConnectionsCtrl->Create( itemPanel11, ID_DIALUPCONNECTIONS, wxDefaultPosition, wxDefaultSize, 0, m_DialupConnectionsCtrlStrings, wxLB_SINGLE|wxLB_NEEDED_SB );
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
    itemFlexGridSizer22->Add(m_DialupDefaultConnectionTextCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_DialupDefaultConnectionCtrl = new wxStaticText;
    m_DialupDefaultConnectionCtrl->Create( itemPanel11, ID_DIALUPDEFAULTCONNECTION, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(m_DialupDefaultConnectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

#endif

#if defined(__WXMSW__)
    itemNotebook3->AddPage(itemPanel11, _("Connections"));
#endif

    wxPanel* itemPanel25 = new wxPanel;
    itemPanel25->Create( itemNotebook3, ID_HTTPPROXY, wxDefaultPosition, wxSize(99, 150), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxVERTICAL);
    itemPanel25->SetSizer(itemBoxSizer26);

    m_EnableHTTPProxyCtrl = new wxCheckBox;
    m_EnableHTTPProxyCtrl->Create( itemPanel25, ID_ENABLEHTTPPROXYCTRL, _("Connect via HTTP proxy server"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_EnableHTTPProxyCtrl->SetValue(FALSE);
    itemBoxSizer26->Add(m_EnableHTTPProxyCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer28Static = new wxStaticBox(itemPanel25, wxID_ANY, _("HTTP Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer28 = new wxStaticBoxSizer(itemStaticBoxSizer28Static, wxVERTICAL);
    itemBoxSizer26->Add(itemStaticBoxSizer28, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer29 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer28->Add(itemGridSizer29, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer30 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer29->Add(itemFlexGridSizer30, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText31 = new wxStaticText;
    itemStaticText31->Create( itemPanel25, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(itemStaticText31, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPAddressCtrl = new wxTextCtrl;
    m_HTTPAddressCtrl->Create( itemPanel25, ID_HTTPADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer30->Add(m_HTTPAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText33 = new wxStaticText;
    itemStaticText33->Create( itemPanel25, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(itemStaticText33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPPortCtrl = new wxTextCtrl;
    m_HTTPPortCtrl->Create( itemPanel25, ID_HTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer30->Add(m_HTTPPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer35Static = new wxStaticBox(itemPanel25, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer35 = new wxStaticBoxSizer(itemStaticBoxSizer35Static, wxVERTICAL);
    itemStaticBoxSizer28->Add(itemStaticBoxSizer35, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer36 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer35->Add(itemFlexGridSizer36, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText37 = new wxStaticText;
    itemStaticText37->Create( itemPanel25, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer36->Add(itemStaticText37, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPUsernameCtrl = new wxTextCtrl;
    m_HTTPUsernameCtrl->Create( itemPanel25, ID_HTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    itemFlexGridSizer36->Add(m_HTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText39 = new wxStaticText;
    itemStaticText39->Create( itemPanel25, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer36->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPPasswordCtrl = new wxTextCtrl;
    m_HTTPPasswordCtrl->Create( itemPanel25, ID_HTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer36->Add(m_HTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel25, _("HTTP Proxy"));

    wxPanel* itemPanel41 = new wxPanel;
    itemPanel41->Create( itemNotebook3, ID_SOCKSPROXY, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer42 = new wxBoxSizer(wxVERTICAL);
    itemPanel41->SetSizer(itemBoxSizer42);

    m_EnableSOCKSProxyCtrl = new wxCheckBox;
    m_EnableSOCKSProxyCtrl->Create( itemPanel41, ID_ENABLESOCKSPROXYCTRL, _("Connect via SOCKS proxy server"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_EnableSOCKSProxyCtrl->SetValue(FALSE);
    itemBoxSizer42->Add(m_EnableSOCKSProxyCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer44Static = new wxStaticBox(itemPanel41, wxID_ANY, _("SOCKS Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer44 = new wxStaticBoxSizer(itemStaticBoxSizer44Static, wxVERTICAL);
    itemBoxSizer42->Add(itemStaticBoxSizer44, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer45 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer44->Add(itemGridSizer45, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer46 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer45->Add(itemFlexGridSizer46, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText47 = new wxStaticText;
    itemStaticText47->Create( itemPanel41, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer46->Add(itemStaticText47, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSAddressCtrl = new wxTextCtrl;
    m_SOCKSAddressCtrl->Create( itemPanel41, ID_SOCKSADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer46->Add(m_SOCKSAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText49 = new wxStaticText;
    itemStaticText49->Create( itemPanel41, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer46->Add(itemStaticText49, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSPortCtrl = new wxTextCtrl;
    m_SOCKSPortCtrl->Create( itemPanel41, ID_SOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer46->Add(m_SOCKSPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer51Static = new wxStaticBox(itemPanel41, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer51 = new wxStaticBoxSizer(itemStaticBoxSizer51Static, wxVERTICAL);
    itemStaticBoxSizer44->Add(itemStaticBoxSizer51, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer52 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer51->Add(itemFlexGridSizer52, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText53 = new wxStaticText;
    itemStaticText53->Create( itemPanel41, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer52->Add(itemStaticText53, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSUsernameCtrl = new wxTextCtrl;
    m_SOCKSUsernameCtrl->Create( itemPanel41, ID_SOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    itemFlexGridSizer52->Add(m_SOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText55 = new wxStaticText;
    itemStaticText55->Create( itemPanel41, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer52->Add(itemStaticText55, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSPasswordCtrl = new wxTextCtrl;
    m_SOCKSPasswordCtrl->Create( itemPanel41, ID_SOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer52->Add(m_SOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel41, _("SOCKS Proxy"));

#if !wxCHECK_VERSION(2,5,2)
    itemBoxSizer2->Add(itemNotebook3Sizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
#else
    itemBoxSizer2->Add(itemNotebook3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
#endif

    wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer57, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton58 = new wxButton;
    itemButton58->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton58->SetDefault();
    itemBoxSizer57->Add(itemButton58, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton59 = new wxButton;
    itemButton59->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer57->Add(itemButton59, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
#if defined(__WXMSW__)
    m_DialupDefaultConnectionCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strDefaultDialupConnection) );
#endif
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
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_NETWORKAUTODETECT
 */

void CDlgOptions::OnNetworkAutoDetectSelected( wxCommandEvent& event )
{
    if (event.IsChecked()) {
        m_DialupConnectionsCtrl->Enable(false);
        m_DialupSetDefaultCtrl->Enable(false);
        m_DialupClearDefaultCtrl->Enable(false);
        m_DialupDefaultConnectionTextCtrl->Enable(false);
        m_DialupDefaultConnectionCtrl->Enable(false);
    }
}

#endif


#if defined(__WXMSW__)
/*!
 * wxEVT_UPDATE_UI event handler for ID_NETWORKAUTODETECT
 */

void CDlgOptions::OnNetworkAutoDetectUpdate( wxUpdateUIEvent& event )
{
    if (m_NetworkAutomaticDetectionCtrl->GetValue()) {
        m_DialupConnectionsCtrl->Enable(false);
        m_DialupSetDefaultCtrl->Enable(false);
        m_DialupClearDefaultCtrl->Enable(false);
        m_DialupDefaultConnectionTextCtrl->Enable(false);
        m_DialupDefaultConnectionCtrl->Enable(false);
    }
}
#endif


#if defined(__WXMSW__)
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_NETWORKLAN
 */

void CDlgOptions::OnNetworkLANSelected( wxCommandEvent& event )
{
    if (event.IsChecked()) {
        m_DialupConnectionsCtrl->Enable(false);
        m_DialupSetDefaultCtrl->Enable(false);
        m_DialupClearDefaultCtrl->Enable(false);
        m_DialupDefaultConnectionTextCtrl->Enable(false);
        m_DialupDefaultConnectionCtrl->Enable(false);
    }
}

#endif


#if defined(__WXMSW__)
/*!
 * wxEVT_UPDATE_UI event handler for ID_NETWORKLAN
 */

void CDlgOptions::OnNetworkLANUpdate( wxUpdateUIEvent& event )
{
    if (m_NetworkUseLANCtrl->GetValue()) {
        m_DialupConnectionsCtrl->Enable(false);
        m_DialupSetDefaultCtrl->Enable(false);
        m_DialupClearDefaultCtrl->Enable(false);
        m_DialupDefaultConnectionTextCtrl->Enable(false);
        m_DialupDefaultConnectionCtrl->Enable(false);
    }
}
#endif


#if defined(__WXMSW__)
/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_NETWORKDIALUP
 */

void CDlgOptions::OnNetworkDialupSelected( wxCommandEvent& event )
{
    if (event.IsChecked()) {
        m_DialupConnectionsCtrl->Enable(true);
        m_DialupSetDefaultCtrl->Enable(true);
        m_DialupClearDefaultCtrl->Enable(true);
        m_DialupDefaultConnectionTextCtrl->Enable(true);
        m_DialupDefaultConnectionCtrl->Enable(true);
    }
}

#endif


#if defined(__WXMSW__)
/*!
 * wxEVT_UPDATE_UI event handler for ID_NETWORKDIALUP
 */

void CDlgOptions::OnNetworkDialupUpdate( wxUpdateUIEvent& event )
{
    if (m_NetworkUseDialupCtrl->GetValue()) {
        m_DialupConnectionsCtrl->Enable(true);
        m_DialupSetDefaultCtrl->Enable(true);
        m_DialupClearDefaultCtrl->Enable(true);
        m_DialupDefaultConnectionTextCtrl->Enable(true);
        m_DialupDefaultConnectionCtrl->Enable(true);
    }
}
#endif


#if defined(__WXMSW__)
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DIALUPSETDEFAULT
 */

void CDlgOptions::OnDialupSetDefaultClick( wxCommandEvent& event )
{
    m_DialupDefaultConnectionCtrl->SetLabel(m_DialupConnectionsCtrl->GetStringSelection());
}
#endif


#if defined(__WXMSW__)
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DIALUPCLEARDEFAULT
 */

void CDlgOptions::OnDialupClearDefaultClick( wxCommandEvent& event )
{
    m_DialupDefaultConnectionCtrl->SetLabel(wxEmptyString);
}
#endif


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ENABLEHTTPPROXYCTRL
 */

void CDlgOptions::OnEnableHTTPProxyCtrlClick(wxCommandEvent& event) {
    if (event.IsChecked()) {
        m_HTTPAddressCtrl->Enable(true);
        m_HTTPPortCtrl->Enable(true);
        m_HTTPUsernameCtrl->Enable(true);
        m_HTTPPasswordCtrl->Enable(true);
    } else {
        m_HTTPAddressCtrl->Enable(false);
        m_HTTPPortCtrl->Enable(false);
        m_HTTPUsernameCtrl->Enable(false);
        m_HTTPPasswordCtrl->Enable(false);
    }

    event.Skip();
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_ENABLEHTTPPROXYCTRL
 */

void CDlgOptions::OnEnableHTTPProxyCtrlUpdate(wxUpdateUIEvent& event) {
    if (m_bProxySectionConfigured) {
        m_EnableHTTPProxyCtrl->Enable(true);
        if (m_EnableHTTPProxyCtrl->IsChecked()) {
            m_HTTPAddressCtrl->Enable(true);
            m_HTTPPortCtrl->Enable(true);
            m_HTTPUsernameCtrl->Enable(true);
            m_HTTPPasswordCtrl->Enable(true);
        } else {
            m_HTTPAddressCtrl->Enable(false);
            m_HTTPPortCtrl->Enable(false);
            m_HTTPUsernameCtrl->Enable(false);
            m_HTTPPasswordCtrl->Enable(false);
        }
    } else {
        m_EnableHTTPProxyCtrl->Enable(false);
        m_HTTPAddressCtrl->Enable(false);
        m_HTTPPortCtrl->Enable(false);
        m_HTTPUsernameCtrl->Enable(false);
        m_HTTPPasswordCtrl->Enable(false);
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
    } else {
        m_SOCKSAddressCtrl->Enable(false);
        m_SOCKSPortCtrl->Enable(false);
        m_SOCKSUsernameCtrl->Enable(false);
        m_SOCKSPasswordCtrl->Enable(false);
    }
    event.Skip();
}


/*!
 * wxEVT_UPDATE_UI event handler for ID_ENABLESOCKSPROXYCTRL
 */

void CDlgOptions::OnEnableSOCKSProxyCtrlUpdate(wxUpdateUIEvent& event) {
    if (m_bProxySectionConfigured) {
        m_EnableSOCKSProxyCtrl->Enable(true);
        if (m_EnableSOCKSProxyCtrl->IsChecked()) {
            m_SOCKSAddressCtrl->Enable(true);
            m_SOCKSPortCtrl->Enable(true);
            m_SOCKSUsernameCtrl->Enable(true);
            m_SOCKSPasswordCtrl->Enable(true);
        } else {
            m_SOCKSAddressCtrl->Enable(false);
            m_SOCKSPortCtrl->Enable(false);
            m_SOCKSUsernameCtrl->Enable(false);
            m_SOCKSPasswordCtrl->Enable(false);
        }
    } else {
        m_EnableSOCKSProxyCtrl->Enable(false);
        m_SOCKSAddressCtrl->Enable(false);
        m_SOCKSPortCtrl->Enable(false);
        m_SOCKSUsernameCtrl->Enable(false);
        m_SOCKSPasswordCtrl->Enable(false);
    }
    event.Skip();
}


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

wxBitmap CDlgOptions::GetBitmapResource(const wxString&)
{
    // Bitmap retrieval
////@begin CDlgOptions bitmap retrieval
    return wxNullBitmap;
////@end CDlgOptions bitmap retrieval
}


/*!
 * Get icon resources
 */

wxIcon CDlgOptions::GetIconResource(const wxString&)
{
    // Icon retrieval
////@begin CDlgOptions icon retrieval
    return wxNullIcon;
////@end CDlgOptions icon retrieval
}


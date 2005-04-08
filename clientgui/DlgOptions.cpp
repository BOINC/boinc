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
    EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK, CDlgOptions::OnNotebookPageChanged)
    EVT_UPDATE_UI(ID_NOTEBOOK, CDlgOptions::OnNotebookUpdate)

    EVT_CHECKBOX(ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnablehttpproxyctrlClick)
    EVT_UPDATE_UI(ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnablehttpproxyctrlUpdate)

    EVT_CHECKBOX(ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnablesocksproxyctrlClick)
    EVT_UPDATE_UI(ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnablesocksproxyctrlUpdate)

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
    wxDialog::Create(parent, id, caption, pos, size, style);

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
    itemNotebook3->Create(itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxSize(300, 265), wxNB_TOP);
#if !wxCHECK_VERSION(2,5,2)
    wxNotebookSizer* itemNotebook3Sizer = new wxNotebookSizer(itemNotebook3);
#endif

    wxPanel* itemPanel4 = new wxPanel;
    itemPanel4->Create(itemNotebook3, ID_GENERAL, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL);
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemPanel4->SetSizer(itemBoxSizer5);

    wxGridSizer* itemGridSizer6 = new wxGridSizer(2, 2, 0, 0);
    itemBoxSizer5->Add(itemGridSizer6, 0, wxGROW|wxALL, 3);
    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create(itemPanel4, wxID_STATIC, _("Language Selection:"), wxDefaultPosition, wxDefaultSize, 0);
    itemGridSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_LanguageSelectionCtrl = new wxComboBox;
    m_LanguageSelectionCtrl->Create(itemPanel4, ID_COMBOBOX, _("(Automatic Detection)"), wxDefaultPosition, wxDefaultSize, wxGetApp().GetSupportedLanguagesCount(), wxGetApp().GetSupportedLanguages(), wxCB_READONLY);
    m_LanguageSelectionCtrl->SetStringSelection(_("(Automatic Detection)"));
    itemGridSizer6->Add(m_LanguageSelectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel4, _("General"));

    wxPanel* itemPanel9 = new wxPanel;
    itemPanel9->Create(itemNotebook3, ID_HTTPPROXY, wxDefaultPosition, wxSize(99, 150), wxTAB_TRAVERSAL);
    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
    itemPanel9->SetSizer(itemBoxSizer10);

    m_EnableHTTPProxyCtrl = new wxCheckBox;
    m_EnableHTTPProxyCtrl->Create(itemPanel9, ID_ENABLEHTTPPROXYCTRL, _("Connect via HTTP proxy server"), wxDefaultPosition, wxDefaultSize, 0);
    m_EnableHTTPProxyCtrl->SetValue(FALSE);
    itemBoxSizer10->Add(m_EnableHTTPProxyCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer12Static = new wxStaticBox(itemPanel9, wxID_ANY, _("HTTP Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer12 = new wxStaticBoxSizer(itemStaticBoxSizer12Static, wxVERTICAL);
    itemBoxSizer10->Add(itemStaticBoxSizer12, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer13 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer12->Add(itemGridSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer14 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer13->Add(itemFlexGridSizer14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText15 = new wxStaticText;
    itemStaticText15->Create(itemPanel9, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer14->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPAddressCtrl = new wxTextCtrl;
    m_HTTPAddressCtrl->Create(itemPanel9, ID_HTTPADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer14->Add(m_HTTPAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText;
    itemStaticText17->Create(itemPanel9, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer14->Add(itemStaticText17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPPortCtrl = new wxTextCtrl;
    m_HTTPPortCtrl->Create(itemPanel9, ID_HTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0);
    itemFlexGridSizer14->Add(m_HTTPPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer19Static = new wxStaticBox(itemPanel9, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer19 = new wxStaticBoxSizer(itemStaticBoxSizer19Static, wxVERTICAL);
    itemStaticBoxSizer12->Add(itemStaticBoxSizer19, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer20 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer19->Add(itemFlexGridSizer20, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText21 = new wxStaticText;
    itemStaticText21->Create(itemPanel9, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer20->Add(itemStaticText21, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPUsernameCtrl = new wxTextCtrl;
    m_HTTPUsernameCtrl->Create(itemPanel9, ID_HTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0);
    itemFlexGridSizer20->Add(m_HTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText23 = new wxStaticText;
    itemStaticText23->Create(itemPanel9, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer20->Add(itemStaticText23, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPPasswordCtrl = new wxTextCtrl;
    m_HTTPPasswordCtrl->Create(itemPanel9, ID_HTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    itemFlexGridSizer20->Add(m_HTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel9, _("HTTP Proxy"));

    wxPanel* itemPanel25 = new wxPanel;
    itemPanel25->Create(itemNotebook3, ID_SOCKSPROXY, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL);
    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxVERTICAL);
    itemPanel25->SetSizer(itemBoxSizer26);

    m_EnableSOCKSProxyCtrl = new wxCheckBox;
    m_EnableSOCKSProxyCtrl->Create(itemPanel25, ID_ENABLESOCKSPROXYCTRL, _("Connect via SOCKS proxy server"), wxDefaultPosition, wxDefaultSize, 0);
    m_EnableSOCKSProxyCtrl->SetValue(FALSE);
    itemBoxSizer26->Add(m_EnableSOCKSProxyCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer28Static = new wxStaticBox(itemPanel25, wxID_ANY, _("SOCKS Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer28 = new wxStaticBoxSizer(itemStaticBoxSizer28Static, wxVERTICAL);
    itemBoxSizer26->Add(itemStaticBoxSizer28, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer29 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer28->Add(itemGridSizer29, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer30 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer29->Add(itemFlexGridSizer30, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText31 = new wxStaticText;
    itemStaticText31->Create(itemPanel25, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer30->Add(itemStaticText31, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSAddressCtrl = new wxTextCtrl;
    m_SOCKSAddressCtrl->Create(itemPanel25, ID_SOCKSADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer30->Add(m_SOCKSAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText33 = new wxStaticText;
    itemStaticText33->Create(itemPanel25, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer30->Add(itemStaticText33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSPortCtrl = new wxTextCtrl;
    m_SOCKSPortCtrl->Create(itemPanel25, ID_SOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0);
    itemFlexGridSizer30->Add(m_SOCKSPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer35Static = new wxStaticBox(itemPanel25, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer35 = new wxStaticBoxSizer(itemStaticBoxSizer35Static, wxVERTICAL);
    itemStaticBoxSizer28->Add(itemStaticBoxSizer35, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer36 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer35->Add(itemFlexGridSizer36, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText37 = new wxStaticText;
    itemStaticText37->Create(itemPanel25, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer36->Add(itemStaticText37, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSUsernameCtrl = new wxTextCtrl;
    m_SOCKSUsernameCtrl->Create(itemPanel25, ID_SOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0);
    itemFlexGridSizer36->Add(m_SOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText39 = new wxStaticText;
    itemStaticText39->Create(itemPanel25, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer36->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSPasswordCtrl = new wxTextCtrl;
    m_SOCKSPasswordCtrl->Create(itemPanel25, ID_SOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    itemFlexGridSizer36->Add(m_SOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel25, _("SOCKS Proxy"));

#if !wxCHECK_VERSION(2,5,2)
    itemBoxSizer2->Add(itemNotebook3Sizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
#else
    itemBoxSizer2->Add(itemNotebook3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
#endif

    wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer41, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton42 = new wxButton;
    itemButton42->Create(itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
    itemButton42->SetDefault();
    itemBoxSizer41->Add(itemButton42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton43 = new wxButton;
    itemButton43->Create(itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer41->Add(itemButton43, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CDlgOptions content construction
}

/*!
 * wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED event handler for ID_NOTEBOOK
 */

void CDlgOptions::OnNotebookPageChanged(wxNotebookEvent& event)
{
////@begin wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED event handler for ID_NOTEBOOK in CDlgToolsOptions.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED event handler for ID_NOTEBOOK in CDlgToolsOptions. 
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_NOTEBOOK
 */

void CDlgOptions::OnNotebookUpdate(wxUpdateUIEvent& event)
{
////@begin wxEVT_UPDATE_UI event handler for ID_NOTEBOOK in CDlgToolsOptions.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_UPDATE_UI event handler for ID_NOTEBOOK in CDlgToolsOptions. 
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_ENABLEHTTPPROXYCTRL
 */

void CDlgOptions::OnEnablehttpproxyctrlClick(wxCommandEvent& event) {
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

void CDlgOptions::OnEnablehttpproxyctrlUpdate(wxUpdateUIEvent& event) {
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

void CDlgOptions::OnEnablesocksproxyctrlClick(wxCommandEvent& event) {
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

void CDlgOptions::OnEnablesocksproxyctrlUpdate(wxUpdateUIEvent& event) {
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

wxBitmap CDlgOptions::GetBitmapResource(const wxString&) {
    // Bitmap retrieval
////@begin CDlgOptions bitmap retrieval
    return wxNullBitmap;
////@end CDlgOptions bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgOptions::GetIconResource(const wxString&) {
    // Icon retrieval
////@begin CDlgOptions icon retrieval
    return wxNullIcon;
////@end CDlgOptions icon retrieval
}

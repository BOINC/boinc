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

    EVT_CHECKBOX( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnablehttpproxyctrlClick )
    EVT_UPDATE_UI( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnablehttpproxyctrlUpdate )

    EVT_CHECKBOX( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnablesocksproxyctrlClick )
    EVT_UPDATE_UI( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnablesocksproxyctrlUpdate )

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
    itemNotebook3->Create( itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxSize(350, 265), wxNB_TOP );
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
    m_LanguageSelectionCtrl->Create( itemPanel4, ID_COMBOBOX, _("(Automatic Detection)"), wxDefaultPosition, wxDefaultSize, wxGetApp().GetSupportedLanguagesCount(), wxGetApp().GetSupportedLanguages(), wxCB_READONLY );
    m_LanguageSelectionCtrl->SetStringSelection(_("(Automatic Detection)"));
    if (ShowToolTips())
        m_LanguageSelectionCtrl->SetToolTip(_("What language should the manager display by default."));
    itemFlexGridSizer6->Add(m_LanguageSelectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create( itemPanel4, wxID_STATIC, _("Reminder Frequency:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxSlider* itemSlider10 = new wxSlider;
    itemSlider10->Create( itemPanel4, ID_SLIDER, 60, 0, 240, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
    if (ShowToolTips())
        itemSlider10->SetToolTip(_("How often, in minutes, should the manager remind you of possible connection events."));
    itemFlexGridSizer6->Add(itemSlider10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel4, _("General"));

    wxPanel* itemPanel11 = new wxPanel;
    itemPanel11->Create( itemNotebook3, ID_HTTPPROXY, wxDefaultPosition, wxSize(99, 150), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
    itemPanel11->SetSizer(itemBoxSizer12);

    m_EnableHTTPProxyCtrl = new wxCheckBox;
    m_EnableHTTPProxyCtrl->Create( itemPanel11, ID_ENABLEHTTPPROXYCTRL, _("Connect via HTTP proxy server"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnableHTTPProxyCtrl->SetValue(FALSE);
    itemBoxSizer12->Add(m_EnableHTTPProxyCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox(itemPanel11, wxID_ANY, _("HTTP Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer14 = new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxVERTICAL);
    itemBoxSizer12->Add(itemStaticBoxSizer14, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer15 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer14->Add(itemGridSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer16 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer15->Add(itemFlexGridSizer16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText17 = new wxStaticText;
    itemStaticText17->Create( itemPanel11, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(itemStaticText17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPAddressCtrl = new wxTextCtrl;
    m_HTTPAddressCtrl->Create( itemPanel11, ID_HTTPADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer16->Add(m_HTTPAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText19 = new wxStaticText;
    itemStaticText19->Create( itemPanel11, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPPortCtrl = new wxTextCtrl;
    m_HTTPPortCtrl->Create( itemPanel11, ID_HTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer16->Add(m_HTTPPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer21Static = new wxStaticBox(itemPanel11, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer21 = new wxStaticBoxSizer(itemStaticBoxSizer21Static, wxVERTICAL);
    itemStaticBoxSizer14->Add(itemStaticBoxSizer21, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer22 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer21->Add(itemFlexGridSizer22, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText23 = new wxStaticText;
    itemStaticText23->Create( itemPanel11, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(itemStaticText23, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPUsernameCtrl = new wxTextCtrl;
    m_HTTPUsernameCtrl->Create( itemPanel11, ID_HTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    itemFlexGridSizer22->Add(m_HTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText25 = new wxStaticText;
    itemStaticText25->Create( itemPanel11, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(itemStaticText25, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_HTTPPasswordCtrl = new wxTextCtrl;
    m_HTTPPasswordCtrl->Create( itemPanel11, ID_HTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer22->Add(m_HTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel11, _("HTTP Proxy"));

    wxPanel* itemPanel27 = new wxPanel;
    itemPanel27->Create( itemNotebook3, ID_SOCKSPROXY, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxVERTICAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    m_EnableSOCKSProxyCtrl = new wxCheckBox;
    m_EnableSOCKSProxyCtrl->Create( itemPanel27, ID_ENABLESOCKSPROXYCTRL, _("Connect via SOCKS proxy server"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnableSOCKSProxyCtrl->SetValue(FALSE);
    itemBoxSizer28->Add(m_EnableSOCKSProxyCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer30Static = new wxStaticBox(itemPanel27, wxID_ANY, _("SOCKS Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer30 = new wxStaticBoxSizer(itemStaticBoxSizer30Static, wxVERTICAL);
    itemBoxSizer28->Add(itemStaticBoxSizer30, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer31 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer30->Add(itemGridSizer31, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer32 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer31->Add(itemFlexGridSizer32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText33 = new wxStaticText;
    itemStaticText33->Create( itemPanel27, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSAddressCtrl = new wxTextCtrl;
    m_SOCKSAddressCtrl->Create( itemPanel27, ID_SOCKSADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer32->Add(m_SOCKSAddressCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText35 = new wxStaticText;
    itemStaticText35->Create( itemPanel27, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer32->Add(itemStaticText35, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSPortCtrl = new wxTextCtrl;
    m_SOCKSPortCtrl->Create( itemPanel27, ID_SOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer32->Add(m_SOCKSPortCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer37Static = new wxStaticBox(itemPanel27, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer37 = new wxStaticBoxSizer(itemStaticBoxSizer37Static, wxVERTICAL);
    itemStaticBoxSizer30->Add(itemStaticBoxSizer37, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer38 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer37->Add(itemFlexGridSizer38, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText39 = new wxStaticText;
    itemStaticText39->Create( itemPanel27, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer38->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSUsernameCtrl = new wxTextCtrl;
    m_SOCKSUsernameCtrl->Create( itemPanel27, ID_SOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    itemFlexGridSizer38->Add(m_SOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText41 = new wxStaticText;
    itemStaticText41->Create( itemPanel27, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer38->Add(itemStaticText41, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_SOCKSPasswordCtrl = new wxTextCtrl;
    m_SOCKSPasswordCtrl->Create( itemPanel27, ID_SOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer38->Add(m_SOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel27, _("SOCKS Proxy"));

#if !wxCHECK_VERSION(2,5,2)
    itemBoxSizer2->Add(itemNotebook3Sizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
#else
    itemBoxSizer2->Add(itemNotebook3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
#endif

    wxBoxSizer* itemBoxSizer43 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer43, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton44 = new wxButton;
    itemButton44->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton44->SetDefault();
    itemBoxSizer43->Add(itemButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton45 = new wxButton;
    itemButton45->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer43->Add(itemButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

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

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


#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgOptions.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "DlgOptions.h"


IMPLEMENT_CLASS( CDlgOptions, wxDialog )

BEGIN_EVENT_TABLE( CDlgOptions, wxDialog )

    EVT_NOTEBOOK_PAGE_CHANGED( ID_NOTEBOOK, CDlgOptions::OnNotebookPageChanged )
    EVT_UPDATE_UI( ID_NOTEBOOK, CDlgOptions::OnNotebookUpdate )

    EVT_CHECKBOX( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnableHTTPProxyCtrlClick )
    EVT_UPDATE_UI( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnableHTTPProxyCtrlUpdate )

    EVT_CHECKBOX( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnableSOCKSProxyCtrlClick )
    EVT_UPDATE_UI( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnableSOCKSProxyCtrlUpdate )

END_EVENT_TABLE()


CDlgOptions::CDlgOptions( )
{
}


CDlgOptions::CDlgOptions( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}


bool CDlgOptions::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_bProxySectionConfigured = false;
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

    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    return TRUE;
}


void CDlgOptions::CreateControls()
{    

    CDlgOptions* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);
    itemDialog1->SetAutoLayout(TRUE);
    wxNotebook* itemNotebook3 = new wxNotebook;
    itemNotebook3->Create( itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxSize(300, 265), wxNB_TOP );
    wxNotebookSizer* itemNotebook3Sizer = new wxNotebookSizer(itemNotebook3);
    wxPanel* itemPanel4 = new wxPanel;
    itemPanel4->Create( itemNotebook3, ID_GENERAL, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemPanel4->SetSizer(itemBoxSizer5);
    itemPanel4->SetAutoLayout(TRUE);
    wxGridSizer* itemGridSizer6 = new wxGridSizer(2, 2, 0, 0);
    itemBoxSizer5->Add(itemGridSizer6, 0, wxGROW|wxALL, 3);
    wxStaticText* itemStaticText7 = new wxStaticText;
    itemStaticText7->Create( itemPanel4, wxID_STATIC, _("Language Selection:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxComboBox* itemComboBox8 = new wxComboBox;
    itemComboBox8->Create( itemPanel4, ID_LANGUAGESELECTION, wxT(""), wxDefaultPosition, wxDefaultSize, wxGetApp().GetSupportedLanguagesCount(), wxGetApp().GetSupportedLanguages(), wxCB_READONLY );
    m_LanguageSelectionCtrl = itemComboBox8;
    itemComboBox8->SetStringSelection(_("(Automatic Detection)"));
    itemGridSizer6->Add(itemComboBox8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel4, _("General"));
    wxPanel* itemPanel9 = new wxPanel;
    itemPanel9->Create( itemNotebook3, ID_HTTPPROXY, wxDefaultPosition, wxSize(99, 150), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
    itemPanel9->SetSizer(itemBoxSizer10);
    itemPanel9->SetAutoLayout(TRUE);
    wxCheckBox* itemCheckBox11 = new wxCheckBox;
    itemCheckBox11->Create( itemPanel9, ID_ENABLEHTTPPROXYCTRL, _("Connect via HTTP proxy server"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnableHTTPProxyCtrl = itemCheckBox11;
    itemCheckBox11->SetValue(FALSE);
    itemBoxSizer10->Add(itemCheckBox11, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer12Static = new wxStaticBox(itemPanel9, wxID_ANY, _("HTTP Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer12 = new wxStaticBoxSizer(itemStaticBoxSizer12Static, wxVERTICAL);
    itemBoxSizer10->Add(itemStaticBoxSizer12, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer13 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer12->Add(itemGridSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer14 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer13->Add(itemFlexGridSizer14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText15 = new wxStaticText;
    itemStaticText15->Create( itemPanel9, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer14->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl16 = new wxTextCtrl;
    itemTextCtrl16->Create( itemPanel9, ID_HTTPADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    m_HTTPAddressCtrl = itemTextCtrl16;
    itemFlexGridSizer14->Add(itemTextCtrl16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText;
    itemStaticText17->Create( itemPanel9, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer14->Add(itemStaticText17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl18 = new wxTextCtrl;
    itemTextCtrl18->Create( itemPanel9, ID_HTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    m_HTTPPortCtrl = itemTextCtrl18;
    itemFlexGridSizer14->Add(itemTextCtrl18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer19Static = new wxStaticBox(itemPanel9, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer19 = new wxStaticBoxSizer(itemStaticBoxSizer19Static, wxVERTICAL);
    itemStaticBoxSizer12->Add(itemStaticBoxSizer19, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer20 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer19->Add(itemFlexGridSizer20, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText21 = new wxStaticText;
    itemStaticText21->Create( itemPanel9, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer20->Add(itemStaticText21, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl22 = new wxTextCtrl;
    itemTextCtrl22->Create( itemPanel9, ID_HTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    m_HTTPUsernameCtrl = itemTextCtrl22;
    itemFlexGridSizer20->Add(itemTextCtrl22, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText23 = new wxStaticText;
    itemStaticText23->Create( itemPanel9, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer20->Add(itemStaticText23, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl24 = new wxTextCtrl;
    itemTextCtrl24->Create( itemPanel9, ID_HTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    m_HTTPPasswordCtrl = itemTextCtrl24;
    itemFlexGridSizer20->Add(itemTextCtrl24, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel9, _("HTTP Proxy"));
    wxPanel* itemPanel25 = new wxPanel;
    itemPanel25->Create( itemNotebook3, ID_SOCKSPROXY, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxVERTICAL);
    itemPanel25->SetSizer(itemBoxSizer26);
    itemPanel25->SetAutoLayout(TRUE);
    wxCheckBox* itemCheckBox27 = new wxCheckBox;
    itemCheckBox27->Create( itemPanel25, ID_ENABLESOCKSPROXYCTRL, _("Connect via SOCKS proxy server"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnableSOCKSProxyCtrl = itemCheckBox27;
    itemCheckBox27->SetValue(FALSE);
    itemBoxSizer26->Add(itemCheckBox27, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer28Static = new wxStaticBox(itemPanel25, wxID_ANY, _("SOCKS Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer28 = new wxStaticBoxSizer(itemStaticBoxSizer28Static, wxVERTICAL);
    itemBoxSizer26->Add(itemStaticBoxSizer28, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer29 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer28->Add(itemGridSizer29, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer30 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer29->Add(itemFlexGridSizer30, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText31 = new wxStaticText;
    itemStaticText31->Create( itemPanel25, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(itemStaticText31, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl32 = new wxTextCtrl;
    itemTextCtrl32->Create( itemPanel25, ID_SOCKSADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    m_SOCKSAddressCtrl = itemTextCtrl32;
    itemFlexGridSizer30->Add(itemTextCtrl32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText33 = new wxStaticText;
    itemStaticText33->Create( itemPanel25, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer30->Add(itemStaticText33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl34 = new wxTextCtrl;
    itemTextCtrl34->Create( itemPanel25, ID_SOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    m_SOCKSPortCtrl = itemTextCtrl34;
    itemFlexGridSizer30->Add(itemTextCtrl34, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer35Static = new wxStaticBox(itemPanel25, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer35 = new wxStaticBoxSizer(itemStaticBoxSizer35Static, wxVERTICAL);
    itemStaticBoxSizer28->Add(itemStaticBoxSizer35, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer36 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer35->Add(itemFlexGridSizer36, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText37 = new wxStaticText;
    itemStaticText37->Create( itemPanel25, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer36->Add(itemStaticText37, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl38 = new wxTextCtrl;
    itemTextCtrl38->Create( itemPanel25, ID_SOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    m_SOCKSUsernameCtrl = itemTextCtrl38;
    itemFlexGridSizer36->Add(itemTextCtrl38, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText39 = new wxStaticText;
    itemStaticText39->Create( itemPanel25, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer36->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl40 = new wxTextCtrl;
    itemTextCtrl40->Create( itemPanel25, ID_SOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    m_SOCKSPasswordCtrl = itemTextCtrl40;
    itemFlexGridSizer36->Add(itemTextCtrl40, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel25, _("SOCKS Proxy"));
    itemBoxSizer2->Add(itemNotebook3Sizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer41, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton42 = new wxButton;
    itemButton42->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton42->SetDefault();
    itemBoxSizer41->Add(itemButton42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton43 = new wxButton;
    itemButton43->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer41->Add(itemButton43, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}


void CDlgOptions::OnNotebookPageChanged( wxNotebookEvent& event )
{
    // Insert custom code here
    event.Skip();
}


void CDlgOptions::OnNotebookUpdate( wxUpdateUIEvent& event )
{
    // Insert custom code here
    event.Skip();
}


void CDlgOptions::OnEnableHTTPProxyCtrlClick( wxCommandEvent& event )
{
    if ( event.IsChecked() )
    {
        m_HTTPAddressCtrl->Enable(true);
        m_HTTPPortCtrl->Enable(true);
        m_HTTPUsernameCtrl->Enable(true);
        m_HTTPPasswordCtrl->Enable(true);
    }
    else
    {
        m_HTTPAddressCtrl->Enable(false);
        m_HTTPPortCtrl->Enable(false);
        m_HTTPUsernameCtrl->Enable(false);
        m_HTTPPasswordCtrl->Enable(false);
    }
    event.Skip();
}


void CDlgOptions::OnEnableHTTPProxyCtrlUpdate( wxUpdateUIEvent& event )
{
    if ( m_bProxySectionConfigured )
    {
        m_EnableHTTPProxyCtrl->Enable(true);
        if ( m_EnableHTTPProxyCtrl->IsChecked() )
        {
            m_HTTPAddressCtrl->Enable(true);
            m_HTTPPortCtrl->Enable(true);
            m_HTTPUsernameCtrl->Enable(true);
            m_HTTPPasswordCtrl->Enable(true);
        }
        else
        {
            m_HTTPAddressCtrl->Enable(false);
            m_HTTPPortCtrl->Enable(false);
            m_HTTPUsernameCtrl->Enable(false);
            m_HTTPPasswordCtrl->Enable(false);
        }
    }
    else
    {
        m_EnableHTTPProxyCtrl->Enable(false);
        m_HTTPAddressCtrl->Enable(false);
        m_HTTPPortCtrl->Enable(false);
        m_HTTPUsernameCtrl->Enable(false);
        m_HTTPPasswordCtrl->Enable(false);
    }
    event.Skip();
}


void CDlgOptions::OnEnableSOCKSProxyCtrlClick( wxCommandEvent& event )
{
    if ( event.IsChecked() )
    {
        m_SOCKSAddressCtrl->Enable(true);
        m_SOCKSPortCtrl->Enable(true);
        m_SOCKSUsernameCtrl->Enable(true);
        m_SOCKSPasswordCtrl->Enable(true);
    }
    else
    {
        m_SOCKSAddressCtrl->Enable(false);
        m_SOCKSPortCtrl->Enable(false);
        m_SOCKSUsernameCtrl->Enable(false);
        m_SOCKSPasswordCtrl->Enable(false);
    }
    event.Skip();
}


void CDlgOptions::OnEnableSOCKSProxyCtrlUpdate( wxUpdateUIEvent& event )
{
    if ( m_bProxySectionConfigured )
    {
        m_EnableSOCKSProxyCtrl->Enable(true);
        if ( m_EnableSOCKSProxyCtrl->IsChecked() )
        {
            m_SOCKSAddressCtrl->Enable(true);
            m_SOCKSPortCtrl->Enable(true);
            m_SOCKSUsernameCtrl->Enable(true);
            m_SOCKSPasswordCtrl->Enable(true);
        }
        else
        {
            m_SOCKSAddressCtrl->Enable(false);
            m_SOCKSPortCtrl->Enable(false);
            m_SOCKSUsernameCtrl->Enable(false);
            m_SOCKSPasswordCtrl->Enable(false);
        }
    }
    else
    {
        m_EnableSOCKSProxyCtrl->Enable(false);
        m_SOCKSAddressCtrl->Enable(false);
        m_SOCKSPortCtrl->Enable(false);
        m_SOCKSUsernameCtrl->Enable(false);
        m_SOCKSPasswordCtrl->Enable(false);
    }
    event.Skip();
}


bool CDlgOptions::ShowToolTips()
{
    return TRUE;
}


const char *BOINC_RCSID_18c9f4f9ba = "$Id$";

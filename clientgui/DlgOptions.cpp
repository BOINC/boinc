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
    wxPanel* itemPanel4 = new wxPanel;
    itemPanel4->Create( itemNotebook3, ID_GENERAL, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemPanel4->SetSizer(itemBoxSizer5);
    itemPanel4->SetAutoLayout(TRUE);
    wxStaticText* itemStaticText6 = new wxStaticText;
    itemStaticText6->Create( itemPanel4, wxID_STATIC, _("This page is intentionally left blank"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    itemBoxSizer5->Add(itemStaticText6, 1, wxGROW, 5);

    itemNotebook3->AddPage(itemPanel4, _("General"));
    wxPanel* itemPanel7 = new wxPanel;
    itemPanel7->Create( itemNotebook3, ID_HTTPPROXY, wxDefaultPosition, wxSize(99, 150), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemPanel7->SetSizer(itemBoxSizer8);
    itemPanel7->SetAutoLayout(TRUE);
    wxCheckBox* itemCheckBox9 = new wxCheckBox;
    itemCheckBox9->Create( itemPanel7, ID_ENABLEHTTPPROXYCTRL, _("Connect via HTTP proxy server"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnableHTTPProxyCtrl = itemCheckBox9;
    itemCheckBox9->SetValue(FALSE);
    itemBoxSizer8->Add(itemCheckBox9, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer10Static = new wxStaticBox(itemPanel7, wxID_ANY, _("HTTP Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer10 = new wxStaticBoxSizer(itemStaticBoxSizer10Static, wxVERTICAL);
    itemBoxSizer8->Add(itemStaticBoxSizer10, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer11 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer10->Add(itemGridSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer12 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer11->Add(itemFlexGridSizer12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText13 = new wxStaticText;
    itemStaticText13->Create( itemPanel7, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer12->Add(itemStaticText13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl14 = new wxTextCtrl;
    itemTextCtrl14->Create( itemPanel7, ID_HTTPADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    m_HTTPAddressCtrl = itemTextCtrl14;
    itemFlexGridSizer12->Add(itemTextCtrl14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText;
    itemStaticText15->Create( itemPanel7, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer12->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl16 = new wxTextCtrl;
    itemTextCtrl16->Create( itemPanel7, ID_HTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    m_HTTPPortCtrl = itemTextCtrl16;
    itemFlexGridSizer12->Add(itemTextCtrl16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer17Static = new wxStaticBox(itemPanel7, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer17 = new wxStaticBoxSizer(itemStaticBoxSizer17Static, wxVERTICAL);
    itemStaticBoxSizer10->Add(itemStaticBoxSizer17, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer18 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer17->Add(itemFlexGridSizer18, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText19 = new wxStaticText;
    itemStaticText19->Create( itemPanel7, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer18->Add(itemStaticText19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl20 = new wxTextCtrl;
    itemTextCtrl20->Create( itemPanel7, ID_HTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    m_HTTPUsernameCtrl = itemTextCtrl20;
    itemFlexGridSizer18->Add(itemTextCtrl20, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText21 = new wxStaticText;
    itemStaticText21->Create( itemPanel7, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer18->Add(itemStaticText21, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl22 = new wxTextCtrl;
    itemTextCtrl22->Create( itemPanel7, ID_HTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    m_HTTPPasswordCtrl = itemTextCtrl22;
    itemFlexGridSizer18->Add(itemTextCtrl22, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel7, _("HTTP Proxy"));
    wxPanel* itemPanel23 = new wxPanel;
    itemPanel23->Create( itemNotebook3, ID_SOCKSPROXY, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemPanel23->SetSizer(itemBoxSizer24);
    itemPanel23->SetAutoLayout(TRUE);
    wxCheckBox* itemCheckBox25 = new wxCheckBox;
    itemCheckBox25->Create( itemPanel23, ID_ENABLESOCKSPROXYCTRL, _("Connect via SOCKS proxy server"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnableSOCKSProxyCtrl = itemCheckBox25;
    itemCheckBox25->SetValue(FALSE);
    itemBoxSizer24->Add(itemCheckBox25, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer26Static = new wxStaticBox(itemPanel23, wxID_ANY, _("SOCKS Proxy Server Configuration"));
    wxStaticBoxSizer* itemStaticBoxSizer26 = new wxStaticBoxSizer(itemStaticBoxSizer26Static, wxVERTICAL);
    itemBoxSizer24->Add(itemStaticBoxSizer26, 0, wxGROW|wxALL, 5);
    wxGridSizer* itemGridSizer27 = new wxGridSizer(2, 1, 0, 0);
    itemStaticBoxSizer26->Add(itemGridSizer27, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer28 = new wxFlexGridSizer(2, 2, 0, 0);
    itemGridSizer27->Add(itemFlexGridSizer28, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText29 = new wxStaticText;
    itemStaticText29->Create( itemPanel23, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer28->Add(itemStaticText29, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl30 = new wxTextCtrl;
    itemTextCtrl30->Create( itemPanel23, ID_SOCKSADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    m_SOCKSAddressCtrl = itemTextCtrl30;
    itemFlexGridSizer28->Add(itemTextCtrl30, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText31 = new wxStaticText;
    itemStaticText31->Create( itemPanel23, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer28->Add(itemStaticText31, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl32 = new wxTextCtrl;
    itemTextCtrl32->Create( itemPanel23, ID_SOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    m_SOCKSPortCtrl = itemTextCtrl32;
    itemFlexGridSizer28->Add(itemTextCtrl32, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer33Static = new wxStaticBox(itemPanel23, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* itemStaticBoxSizer33 = new wxStaticBoxSizer(itemStaticBoxSizer33Static, wxVERTICAL);
    itemStaticBoxSizer26->Add(itemStaticBoxSizer33, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* itemFlexGridSizer34 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer33->Add(itemFlexGridSizer34, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* itemStaticText35 = new wxStaticText;
    itemStaticText35->Create( itemPanel23, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer34->Add(itemStaticText35, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl36 = new wxTextCtrl;
    itemTextCtrl36->Create( itemPanel23, ID_SOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    m_SOCKSUsernameCtrl = itemTextCtrl36;
    itemFlexGridSizer34->Add(itemTextCtrl36, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText37 = new wxStaticText;
    itemStaticText37->Create( itemPanel23, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer34->Add(itemStaticText37, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* itemTextCtrl38 = new wxTextCtrl;
    itemTextCtrl38->Create( itemPanel23, ID_SOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    m_SOCKSPasswordCtrl = itemTextCtrl38;
    itemFlexGridSizer34->Add(itemTextCtrl38, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel23, _("SOCKS Proxy"));
    itemBoxSizer2->Add(itemNotebook3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer39 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer39, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton40 = new wxButton;
    itemButton40->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton40->SetDefault();
    itemBoxSizer39->Add(itemButton40, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton41 = new wxButton;
    itemButton41->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer39->Add(itemButton41, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

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

// $Id$
//
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
// Revision History:
//
// $Log$
// Revision 1.4  2004/05/17 22:15:09  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgOptions.h"
#endif

#include "stdwx.h"
#include "DlgOptions.h"


IMPLEMENT_CLASS( CDlgOptions, wxDialog )

BEGIN_EVENT_TABLE( CDlgOptions, wxDialog )

    EVT_NOTEBOOK_PAGE_CHANGED( ID_NOTEBOOK, CDlgOptions::OnNotebookPageChanged )
    EVT_UPDATE_UI( ID_NOTEBOOK, CDlgOptions::OnNotebookUpdate )

    EVT_CHECKBOX( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnablehttpproxyctrlClick )
    EVT_UPDATE_UI( ID_ENABLEHTTPPROXYCTRL, CDlgOptions::OnEnablehttpproxyctrlUpdate )

    EVT_CHECKBOX( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnablesocksproxyctrlClick )
    EVT_UPDATE_UI( ID_ENABLESOCKSPROXYCTRL, CDlgOptions::OnEnablesocksproxyctrlUpdate )

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
    m_EnableHTTPProxyCtrl = NULL;
    m_HTTPAddressCtrl = NULL;
    m_HTTPPortCtrl = NULL;
    m_HTTPUsernameCtrl = NULL;
    m_HTTPPasswordCtrl = NULL;
    m_EnableSOCKSProxyCtrl = NULL;
    m_SOCKSAddressCtrl = NULL;
    m_SOCKSPortCtrl = NULL;
    m_SOCKSUsernameCtrl = NULL;
    m_SOCKPasswordCtrl = NULL;

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
    CDlgOptions* item1 = this;

    wxBoxSizer* item2 = new wxBoxSizer(wxVERTICAL);
    item1->SetSizer(item2);
    item1->SetAutoLayout(TRUE);

    wxNotebook* item3 = new wxNotebook;
    item3->Create( item1, ID_NOTEBOOK, wxDefaultPosition, wxSize(300, 265), wxNB_TOP );
    wxPanel* item4 = new wxPanel;
    item4->Create( item3, ID_GENERAL, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL );
    item3->AddPage(item4, _("General"));
    wxPanel* item5 = new wxPanel;
    item5->Create( item3, ID_HTTPPROXY, wxDefaultPosition, wxSize(99, 150), wxTAB_TRAVERSAL );
    wxBoxSizer* item6 = new wxBoxSizer(wxVERTICAL);
    item5->SetSizer(item6);
    item5->SetAutoLayout(TRUE);
    wxCheckBox* item7 = new wxCheckBox;
    item7->Create( item5, ID_ENABLEHTTPPROXYCTRL, _("Connect via HTTP proxy server"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnableHTTPProxyCtrl = item7;
    item7->SetValue(FALSE);
    item6->Add(item7, 0, wxGROW|wxALL, 5);
    wxStaticBox* item8Static = new wxStaticBox(item5, wxID_ANY, _("HTTP Proxy Server Configuration"));
    wxStaticBoxSizer* item8 = new wxStaticBoxSizer(item8Static, wxVERTICAL);
    item6->Add(item8, 0, wxGROW|wxALL, 5);
    wxGridSizer* item9 = new wxGridSizer(2, 1, 0, 0);
    item8->Add(item9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* item10 = new wxFlexGridSizer(2, 2, 0, 0);
    item9->Add(item10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* item11 = new wxStaticText;
    item11->Create( item5, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    item10->Add(item11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
    wxTextCtrl* item12 = new wxTextCtrl;
    item12->Create( item5, ID_HTTPADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    m_HTTPAddressCtrl = item12;
    item10->Add(item12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* item13 = new wxStaticText;
    item13->Create( item5, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    item10->Add(item13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
    wxTextCtrl* item14 = new wxTextCtrl;
    item14->Create( item5, ID_HTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    m_HTTPPortCtrl = item14;
    item10->Add(item14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticBox* item15Static = new wxStaticBox(item5, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* item15 = new wxStaticBoxSizer(item15Static, wxVERTICAL);
    item8->Add(item15, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* item16 = new wxFlexGridSizer(2, 2, 0, 0);
    item15->Add(item16, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* item17 = new wxStaticText;
    item17->Create( item5, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    item16->Add(item17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
    wxTextCtrl* item18 = new wxTextCtrl;
    item18->Create( item5, ID_HTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    m_HTTPUsernameCtrl = item18;
    item16->Add(item18, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* item19 = new wxStaticText;
    item19->Create( item5, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    item16->Add(item19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
    wxTextCtrl* item20 = new wxTextCtrl;
    item20->Create( item5, ID_HTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    m_HTTPPasswordCtrl = item20;
    item16->Add(item20, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    item3->AddPage(item5, _("HTTP Proxy"));
    wxPanel* item21 = new wxPanel;
    item21->Create( item3, ID_SOCKSPROXY, wxDefaultPosition, wxSize(99, 80), wxTAB_TRAVERSAL );
    wxBoxSizer* item22 = new wxBoxSizer(wxVERTICAL);
    item21->SetSizer(item22);
    item21->SetAutoLayout(TRUE);
    wxCheckBox* item23 = new wxCheckBox;
    item23->Create( item21, ID_ENABLESOCKSPROXYCTRL, _("Connect via SOCKS proxy server"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnableSOCKSProxyCtrl = item23;
    item23->SetValue(FALSE);
    item22->Add(item23, 0, wxGROW|wxALL, 5);
    wxStaticBox* item24Static = new wxStaticBox(item21, wxID_ANY, _("SOCKS Proxy Server Configuration"));
    wxStaticBoxSizer* item24 = new wxStaticBoxSizer(item24Static, wxVERTICAL);
    item22->Add(item24, 0, wxGROW|wxALL, 5);
    wxGridSizer* item25 = new wxGridSizer(2, 1, 0, 0);
    item24->Add(item25, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxFlexGridSizer* item26 = new wxFlexGridSizer(2, 2, 0, 0);
    item25->Add(item26, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* item27 = new wxStaticText;
    item27->Create( item21, wxID_STATIC, _("Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    item26->Add(item27, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
    wxTextCtrl* item28 = new wxTextCtrl;
    item28->Create( item21, ID_SOCKSADDRESSCTRL, _T(""), wxDefaultPosition, wxSize(150, -1), 0 );
    m_SOCKSAddressCtrl = item28;
    item26->Add(item28, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* item29 = new wxStaticText;
    item29->Create( item21, wxID_STATIC, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    item26->Add(item29, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
    wxTextCtrl* item30 = new wxTextCtrl;
    item30->Create( item21, ID_SOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    m_SOCKSPortCtrl = item30;
    item26->Add(item30, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticBox* item31Static = new wxStaticBox(item21, wxID_ANY, _("Leave these blank if not needed"));
    wxStaticBoxSizer* item31 = new wxStaticBoxSizer(item31Static, wxVERTICAL);
    item24->Add(item31, 0, wxGROW|wxALL, 5);
    wxFlexGridSizer* item32 = new wxFlexGridSizer(2, 2, 0, 0);
    item31->Add(item32, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* item33 = new wxStaticText;
    item33->Create( item21, wxID_STATIC, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    item32->Add(item33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
    wxTextCtrl* item34 = new wxTextCtrl;
    item34->Create( item21, ID_SOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxSize(175, -1), 0 );
    m_SOCKSUsernameCtrl = item34;
    item32->Add(item34, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* item35 = new wxStaticText;
    item35->Create( item21, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    item32->Add(item35, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
    wxTextCtrl* item36 = new wxTextCtrl;
    item36->Create( item21, ID_SOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    m_SOCKPasswordCtrl = item36;
    item32->Add(item36, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    item3->AddPage(item21, _("SOCKS Proxy"));
    item2->Add(item3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* item37 = new wxBoxSizer(wxHORIZONTAL);
    item2->Add(item37, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* item38 = new wxButton;
    item38->Create( item1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item38->SetDefault();
    item37->Add(item38, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* item39 = new wxButton;
    item39->Create( item1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item37->Add(item39, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
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


void CDlgOptions::OnEnablehttpproxyctrlClick( wxCommandEvent& event )
{
    // Insert custom code here
    event.Skip();
}


void CDlgOptions::OnEnablehttpproxyctrlUpdate( wxUpdateUIEvent& event )
{
    // Insert custom code here
    event.Skip();
}


void CDlgOptions::OnEnablesocksproxyctrlClick( wxCommandEvent& event )
{
    // Insert custom code here
    event.Skip();
}


void CDlgOptions::OnEnablesocksproxyctrlUpdate( wxUpdateUIEvent& event )
{
    // Insert custom code here
    event.Skip();
}


bool CDlgOptions::ShowToolTips()
{
    return TRUE;
}


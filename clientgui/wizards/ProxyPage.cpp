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
#pragma implementation "ProxyPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "ProxyPage.h"


/*!
 * CErrProxyPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrProxyPage, wxWizardPageEx )
 
/*!
 * CErrProxyPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrProxyPage, wxWizardPageEx )
 
////@begin CErrProxyPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrProxyPage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CErrProxyPage::OnPageChanging )
    EVT_WIZARDEX_CANCEL( -1, CErrProxyPage::OnCancel )

////@end CErrProxyPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrProxyPage constructors
 */
 
CErrProxyPage::CErrProxyPage( )
{
}
 
CErrProxyPage::CErrProxyPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrProxyComplationPage creator
 */
 
bool CErrProxyPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrProxyPage member initialisation
    m_ProxyHTTPServerStaticCtrl = NULL;
    m_ProxyHTTPServerCtrl = NULL;
    m_ProxyHTTPPortStaticCtrl = NULL;
    m_ProxyHTTPPortCtrl = NULL;
    m_ProxyHTTPUsernameStaticCtrl = NULL;
    m_ProxyHTTPUsernameCtrl = NULL;
    m_ProxyHTTPPasswordStaticCtrl = NULL;
    m_ProxyHTTPPasswordCtrl = NULL;
    m_ProxyHTTPAutodetectCtrl = NULL;
    m_ProxySOCKSServerStaticCtrl = NULL;
    m_ProxySOCKSServerCtrl = NULL;
    m_ProxySOCKSPortStaticCtrl = NULL;
    m_ProxySOCKSPortCtrl = NULL;
    m_ProxySOCKSUsernameStaticCtrl = NULL;
    m_ProxySOCKSUsernameCtrl = NULL;
    m_ProxySOCKSPasswordStaticCtrl = NULL;
    m_ProxySOCKSPasswordCtrl = NULL;
////@end CErrProxyPage member initialisation
 
////@begin CErrProxyPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProxyPage creation
 
    return TRUE;
}
 
/*!
 * Control creation for CErrProxyComplationPage
 */
 
void CErrProxyPage::CreateControls()
{    
////@begin CErrProxyPage content construction
    CErrProxyPage* itemWizardPage121 = this;

    wxBoxSizer* itemBoxSizer122 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage121->SetSizer(itemBoxSizer122);

    wxStaticText* itemStaticText123 = new wxStaticText;
    itemStaticText123->Create( itemWizardPage121, wxID_STATIC, _("Proxy configuration"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText123->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer122->Add(itemStaticText123, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer122->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer125Static = new wxStaticBox(itemWizardPage121, wxID_ANY, _("HTTP proxy"));
    wxStaticBoxSizer* itemStaticBoxSizer125 = new wxStaticBoxSizer(itemStaticBoxSizer125Static, wxVERTICAL);
    itemBoxSizer122->Add(itemStaticBoxSizer125, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer126 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer126->AddGrowableCol(1);
    itemStaticBoxSizer125->Add(itemFlexGridSizer126, 0, wxGROW|wxALL, 2);

    m_ProxyHTTPServerStaticCtrl = new wxStaticText;
    m_ProxyHTTPServerStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPSERVERSTATICCTRL, _("Server:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_ProxyHTTPServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer128 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer128->AddGrowableCol(0);
    itemFlexGridSizer126->Add(itemFlexGridSizer128, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ProxyHTTPServerCtrl = new wxTextCtrl;
    m_ProxyHTTPServerCtrl->Create( itemWizardPage121, ID_PROXYHTTPSERVERCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer128->Add(m_ProxyHTTPServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPortStaticCtrl = new wxStaticText;
    m_ProxyHTTPPortStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPPORTSTATICCTRL, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer128->Add(m_ProxyHTTPPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPortCtrl = new wxTextCtrl;
    m_ProxyHTTPPortCtrl->Create( itemWizardPage121, ID_PROXYHTTPPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer128->Add(m_ProxyHTTPPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPUsernameStaticCtrl = new wxStaticText;
    m_ProxyHTTPUsernameStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPUSERNAMESTATICCTRL, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_ProxyHTTPUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPUsernameCtrl = new wxTextCtrl;
    m_ProxyHTTPUsernameCtrl->Create( itemWizardPage121, ID_PROXYHTTPUSERNAMECTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_ProxyHTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPasswordStaticCtrl = new wxStaticText;
    m_ProxyHTTPPasswordStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_ProxyHTTPPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxyHTTPPasswordCtrl = new wxTextCtrl;
    m_ProxyHTTPPasswordCtrl->Create( itemWizardPage121, ID_PROXYHTTPPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer126->Add(m_ProxyHTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

#if 0
    m_ProxyHTTPAutodetectCtrl = new wxButton;
    m_ProxyHTTPAutodetectCtrl->Create( itemWizardPage121, ID_PROXYHTTPAUTODETECTCTRL, _("Autodetect"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer125->Add(m_ProxyHTTPAutodetectCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
#endif

    wxStaticBox* itemStaticBoxSizer137Static = new wxStaticBox(itemWizardPage121, wxID_ANY, _("SOCKS proxy"));
    wxStaticBoxSizer* itemStaticBoxSizer137 = new wxStaticBoxSizer(itemStaticBoxSizer137Static, wxVERTICAL);
    itemBoxSizer122->Add(itemStaticBoxSizer137, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer138 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer138->AddGrowableCol(1);
    itemStaticBoxSizer137->Add(itemFlexGridSizer138, 0, wxGROW|wxALL, 2);

    m_ProxySOCKSServerStaticCtrl = new wxStaticText;
    m_ProxySOCKSServerStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSSERVERSTATICCTRL, _("Server:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_ProxySOCKSServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer140 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer140->AddGrowableCol(0);
    itemFlexGridSizer138->Add(itemFlexGridSizer140, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ProxySOCKSServerCtrl = new wxTextCtrl;
    m_ProxySOCKSServerCtrl->Create( itemWizardPage121, ID_PROXYSOCKSSERVERCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer140->Add(m_ProxySOCKSServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPortStaticCtrl = new wxStaticText;
    m_ProxySOCKSPortStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPORTSTATICCTRL, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer140->Add(m_ProxySOCKSPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPortCtrl = new wxTextCtrl;
    m_ProxySOCKSPortCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPORTCTRL, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer140->Add(m_ProxySOCKSPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSUsernameStaticCtrl = new wxStaticText;
    m_ProxySOCKSUsernameStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSUSERNAMESTATICCTRL, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_ProxySOCKSUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSUsernameCtrl = new wxTextCtrl;
    m_ProxySOCKSUsernameCtrl->Create( itemWizardPage121, ID_PROXYSOCKSUSERNAMECTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_ProxySOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPasswordStaticCtrl = new wxStaticText;
    m_ProxySOCKSPasswordStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPASSWORDSTATICCTRL, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_ProxySOCKSPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ProxySOCKSPasswordCtrl = new wxTextCtrl;
    m_ProxySOCKSPasswordCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPASSWORDCTRL, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer138->Add(m_ProxySOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    // Set validators
    m_ProxyHTTPServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPServer) );
    m_ProxyHTTPPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxyHTTPPort) );
    m_ProxyHTTPUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPUsername) );
    m_ProxyHTTPPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPPassword) );
    m_ProxySOCKSServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSServer) );
    m_ProxySOCKSPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxySOCKSPort) );
    m_ProxySOCKSUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSUsername) );
    m_ProxySOCKSPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSPassword) );
////@end CErrProxyPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CErrProxyPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CErrProxyPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (IS_ATTACHTOPROJECTWIZARD()) {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
    } else if (IS_ACCOUNTMANAGERWIZARD()) {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROPERTIESPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrProxyPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrProxyPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProxyPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProxyPage bitmap retrieval
}
  
/*!
 * Get icon resources
 */
 
wxIcon CErrProxyPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProxyPage icon retrieval
    return wxNullIcon;
////@end CErrProxyPage icon retrieval
}
 
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYCOMPLETIONPAGE
 */
 
void CErrProxyPage::OnPageChanged( wxWizardExEvent& event ) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString       strBuffer = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (event.GetDirection() == true) {
        // Moving from the previous page, get state
        pDoc->GetProxyConfiguration();
        m_ProxyHTTPServerCtrl->SetValue(pDoc->proxy_info.http_server_name.c_str());
        m_ProxyHTTPUsernameCtrl->SetValue(pDoc->proxy_info.http_user_name.c_str());
        m_ProxyHTTPPasswordCtrl->SetValue(pDoc->proxy_info.http_user_passwd.c_str());

        strBuffer.Printf(wxT("%d"), pDoc->proxy_info.http_server_port);
        m_ProxyHTTPPortCtrl->SetValue(strBuffer);

        m_ProxySOCKSServerCtrl->SetValue(pDoc->proxy_info.socks_server_name.c_str());
        m_ProxySOCKSUsernameCtrl->SetValue(pDoc->proxy_info.socks5_user_name.c_str());
        m_ProxySOCKSPasswordCtrl->SetValue(pDoc->proxy_info.socks5_user_passwd.c_str());

        strBuffer.Printf(wxT("%d"), pDoc->proxy_info.socks_server_port);
        m_ProxySOCKSPortCtrl->SetValue(strBuffer);
    }
}
 
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ERRPROXYPAGE
 */

void CErrProxyPage::OnPageChanging( wxWizardExEvent& event ) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString       strBuffer = wxEmptyString;
    int            iBuffer = 0;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (event.GetDirection() == true) {
        // Moving to the next page, save state
        pDoc->proxy_info.use_http_proxy = (m_ProxyHTTPServerCtrl->GetValue().Length() > 0);
        pDoc->proxy_info.http_server_name = m_ProxyHTTPServerCtrl->GetValue().c_str();
        pDoc->proxy_info.http_user_name = m_ProxyHTTPUsernameCtrl->GetValue().c_str();
        pDoc->proxy_info.http_user_passwd = m_ProxyHTTPPasswordCtrl->GetValue().c_str();

        strBuffer = m_ProxyHTTPPortCtrl->GetValue();
        strBuffer.ToLong((long*)&iBuffer);
        pDoc->proxy_info.http_server_port = iBuffer;

        pDoc->proxy_info.use_socks_proxy = (m_ProxySOCKSServerCtrl->GetValue().Length() > 0);
        pDoc->proxy_info.socks_server_name = m_ProxySOCKSServerCtrl->GetValue().c_str();
        pDoc->proxy_info.socks5_user_name = m_ProxySOCKSUsernameCtrl->GetValue().c_str();
        pDoc->proxy_info.socks5_user_passwd = m_ProxySOCKSPasswordCtrl->GetValue().c_str();

        strBuffer = m_ProxySOCKSPortCtrl->GetValue();
        strBuffer.ToLong((long*)&iBuffer);
        pDoc->proxy_info.socks_server_port = iBuffer;

        pDoc->SetProxyConfiguration();
    }
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYCOMPLETIONPAGE
 */
 
void CErrProxyPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}


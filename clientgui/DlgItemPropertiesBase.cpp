///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "DlgItemPropertiesBase.h"

///////////////////////////////////////////////////////////////////////////

CDlgItemPropertiesBase::CDlgItemPropertiesBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_html = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO|wxSIMPLE_BORDER );
	bSizer11->Add( m_html, 1, wxALL|wxEXPAND, 5 );
	
	m_btnClose = new wxButton( this, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btnClose->SetDefault(); 
	bSizer11->Add( m_btnClose, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	this->SetSizer( bSizer11 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

CDlgItemPropertiesBase::~CDlgItemPropertiesBase()
{
}

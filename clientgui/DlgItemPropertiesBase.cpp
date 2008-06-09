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
	
	m_bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_scrolledWindow = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledWindow->SetScrollRate( 5, 5 );
	wxBoxSizer* m_bSizer2;
	m_bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	m_gbSizer = new wxGridBagSizer( 0, 0 );
	m_gbSizer->AddGrowableCol( 1 );
	m_gbSizer->SetFlexibleDirection( wxBOTH );
	m_gbSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_bSizer2->Add( m_gbSizer, 1, wxEXPAND, 5 );
	
	m_scrolledWindow->SetSizer( m_bSizer2 );
	m_scrolledWindow->Layout();
	m_bSizer2->Fit( m_scrolledWindow );
	m_bSizer1->Add( m_scrolledWindow, 1, wxEXPAND | wxALL, 5 );
	
	m_btnClose = new wxButton( this, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btnClose->SetDefault(); 
	m_bSizer1->Add( m_btnClose, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	this->SetSizer( m_bSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

CDlgItemPropertiesBase::~CDlgItemPropertiesBase()
{
}

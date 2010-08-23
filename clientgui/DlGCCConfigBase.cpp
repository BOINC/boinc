///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "DlgCCConfigBase.h"

///////////////////////////////////////////////////////////////////////////

DlgCCConfigBase::DlgCCConfigBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxStaticBoxSizer* sbSizerBase;
	sbSizerBase = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("label") ), wxVERTICAL );
	
	m_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_panelLogging = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizerLogging;
	bSizerLogging = new wxBoxSizer( wxVERTICAL );
	
	m_panelLogging->SetSizer( bSizerLogging );
	m_panelLogging->Layout();
	bSizerLogging->Fit( m_panelLogging );
	m_notebook->AddPage( m_panelLogging, wxT("Logging"), false );
	m_panelOptions = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizerOptions;
	bSizerOptions = new wxBoxSizer( wxVERTICAL );
	
	m_panelOptions->SetSizer( bSizerOptions );
	m_panelOptions->Layout();
	bSizerOptions->Fit( m_panelOptions );
	m_notebook->AddPage( m_panelOptions, wxT("Options"), false );
	
	sbSizerBase->Add( m_notebook, 1, wxEXPAND | wxALL, 5 );
	
	m_sdbSizerButtons = new wxStdDialogButtonSizer();
	m_sdbSizerButtonsOK = new wxButton( this, wxID_OK );
	m_sdbSizerButtons->AddButton( m_sdbSizerButtonsOK );
	m_sdbSizerButtonsCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizerButtons->AddButton( m_sdbSizerButtonsCancel );
	m_sdbSizerButtons->Realize();
	sbSizerBase->Add( m_sdbSizerButtons, 0, wxEXPAND, 5 );
	
	this->SetSizer( sbSizerBase );
	this->Layout();
}

DlgCCConfigBase::~DlgCCConfigBase()
{
}

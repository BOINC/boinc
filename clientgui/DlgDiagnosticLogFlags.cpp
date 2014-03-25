// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgDiagnosticLogFlags.cpp"
#endif

#include "stdwx.h"
#include "util.h"
#include "DlgDiagnosticLogFlags.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "Events.h"
#include "error_numbers.h"
#include "gui_rpc_client.h" // For SET_LOCALE

IMPLEMENT_DYNAMIC_CLASS(CDlgDiagnosticLogFlags, wxDialog)

BEGIN_EVENT_TABLE(CDlgDiagnosticLogFlags, wxDialog)
    EVT_SIZE(CDlgDiagnosticLogFlags::OnSize)
    EVT_BUTTON(wxID_OK,CDlgDiagnosticLogFlags::OnOK)
    EVT_BUTTON(ID_DEFAULTSBTN,CDlgDiagnosticLogFlags::OnSetDefaults)

END_EVENT_TABLE()

/* Constructor */
CDlgDiagnosticLogFlags::CDlgDiagnosticLogFlags(wxWindow* parent) :
    wxDialog( parent, ID_ANYDIALOG, wxEmptyString, wxDefaultPosition, 
                wxSize( 503,480 ), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ) {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(pFrame);
    if (!pFrame) return;

    SetSizeHints( wxDefaultSize, wxDefaultSize );
    SetExtraStyle( GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY );
    
    wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );
    
    m_desctext = _("BOINC's tech support assistants may ask you to set some of these on or off to help diagnose certain types of problems.");
    m_desc = new wxStaticText(this, wxID_ANY, m_desctext);
    bSizer1->AddSpacer(7);
    bSizer1->Add(m_desc, 0, wxLEFT | wxRIGHT, 25);
    bSizer1->AddSpacer(7);

    m_scrolledWindow = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    m_scrolledWindow->SetScrollRate( 5, 5 );

    CreateCheckboxes();
    
    bSizer1->Add( m_scrolledWindow, 1, wxEXPAND | wxALL, 5 );
    
    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    wxButton* btnOK = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    btnOK->SetToolTip( _("save all values and close the dialog") );
    buttonSizer->Add( btnOK, 0, wxALL, 5 );

    wxButton* btnCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    btnCancel->SetToolTip( _("close the dialog without saving") );
    buttonSizer->Add( btnCancel, 0, wxALL, 5 );

    wxButton* btnDefaults = new wxButton( this, ID_DEFAULTSBTN, _("Defaults"), wxDefaultPosition, wxDefaultSize, 0 );
    btnDefaults->SetToolTip( _("Restore default settings") );
    buttonSizer->Add( btnDefaults, 0, wxALL, 5 );

    btnCancel->SetDefault();
    bSizer1->Add( buttonSizer, 0, wxALL, 5 );
    
    SetSizer( bSizer1 );
    
    RestoreState();
    FitInside();
    Center( wxBOTH );
}

// destructor
CDlgDiagnosticLogFlags::~CDlgDiagnosticLogFlags() {
    SaveState();
}


void CDlgDiagnosticLogFlags::CreateCheckboxes() {
    SET_LOCALE sl;
    char buf[64000];
    MIOFILE mf;
    bool val;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    m_checkbox_list.clear();
    
    // Get cc_config.xml file flags
    log_flags.init();
    config.defaults();
    pDoc->rpc.get_cc_config(config, log_flags);
    
    mf.init_buf_write(buf, sizeof(buf));
    config.write(mf, log_flags);

    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer( wxVERTICAL );
    
    mf.init_buf_read(buf);
    XML_PARSER xp(&mf);

    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("log_flags")) break;
    }
    
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            continue;
        }
        if (xp.match_tag("/log_flags")) break;
        wxString label = wxString(xp.parsed_tag);
        xp.parse_bool(xp.parsed_tag, val);

        wxCheckBox* ckbox = new wxCheckBox(m_scrolledWindow, wxID_ANY, label);
        bSizer2->Add(ckbox, 0, wxLEFT, 25);
        bSizer2->AddSpacer(3);
        m_checkbox_list.push_back(ckbox);
        ckbox->SetValue(val);
    }

    m_scrolledWindow->SetSizer( bSizer2 );
    m_scrolledWindow->Layout();
    bSizer2->Fit( m_scrolledWindow );
}


/* saves dialog size and (on Mac) position */
bool CDlgDiagnosticLogFlags::SaveState() {
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);
    if (!pConfig) return false;

    pConfig->SetPath("/DlgDiagnosticLogFlags/");
    pConfig->Write(wxT("Width"), GetSize().GetWidth());
    pConfig->Write(wxT("Height"), GetSize().GetHeight());
#ifdef __WXMAC__
    pConfig->Write(wxT("XPos"), GetPosition().x);
    pConfig->Write(wxT("YPos"), GetPosition().y);
#endif

    pConfig->Flush();
    
    return true;
}

/* restores former dialog size and (on Mac) position */
bool CDlgDiagnosticLogFlags::RestoreState() {
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    int                iWidth, iHeight;

    wxASSERT(pConfig);
    if (!pConfig) return false;

    pConfig->SetPath("/DlgDiagnosticLogFlags/");

    pConfig->Read(wxT("Width"), &iWidth, wxDefaultCoord);
    pConfig->Read(wxT("Height"), &iHeight, wxDefaultCoord);

    // Guard against a rare situation where registry values are zero
    if ((iWidth < 50) && (iWidth != wxDefaultCoord)) iWidth = wxDefaultCoord;
    if ((iHeight < 50) && (iHeight != wxDefaultCoord)) iHeight = wxDefaultCoord;

#ifndef __WXMAC__
    // Set size to saved values or defaults if no saved values
    SetSize(iWidth, iHeight);    
#else
    int                iTop, iLeft;
    
    pConfig->Read(wxT("YPos"), &iTop, wxDefaultCoord);
    pConfig->Read(wxT("XPos"), &iLeft, wxDefaultCoord);
    
    // If either co-ordinate is less then 0 then set it equal to 0 to ensure
    // it displays on the screen.
    if ((iLeft < 0) && (iLeft != wxDefaultCoord)) iLeft = 30;
    if ((iTop < 0) && (iTop != wxDefaultCoord)) iTop = 30;

    // Set size and position to saved values or defaults if no saved values
    SetSize(iLeft, iTop, iWidth, iHeight, wxSIZE_USE_EXISTING);

    // Now make sure window is on screen
    GetScreenPosition(&iLeft, &iTop);
    GetSize(&iWidth, &iHeight);

    if (!IsWindowOnScreen(iLeft, iTop, iWidth, iHeight)) {
        iTop = iLeft = 30;
        SetSize(iLeft, iTop, iWidth, iHeight, wxSIZE_USE_EXISTING);
    }
#endif

    return true;
}


void CDlgDiagnosticLogFlags::OnSize(wxSizeEvent& event) {
    m_desc->SetLabel(m_desctext);
    m_desc->Wrap(GetSize().GetWidth()-50);
    FitInside();
    SaveState();
    
    event.Skip();
}


void CDlgDiagnosticLogFlags::OnOK(wxCommandEvent& event) {
    SET_LOCALE sl;
    char buf[64000];
    MIOFILE mf;
    bool val;
    int i;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    mf.init_buf_write(buf, sizeof(buf));
    for (i=0; i<m_checkbox_list.size(); ++i) {
        wxCheckBox* ckbox = m_checkbox_list[i];
        val = ckbox->GetValue();
        mf.printf("        <%s>%d</%s>\n", (const char*)ckbox->GetLabel().ToAscii(), (int)val, (const char*)ckbox->GetLabel().ToAscii());
    }
    mf.printf("    </log_flags>\n");

    XML_PARSER xp(&mf);
    mf.init_buf_read(buf);
    log_flags.parse(xp);
    
    int retval = pDoc->rpc.set_cc_config(config, log_flags);
    if (!retval) {
        pDoc->rpc.read_cc_config();
    }
    event.Skip();
}


void CDlgDiagnosticLogFlags::OnSetDefaults(wxCommandEvent& event) {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    log_flags.init();
    int retval = pDoc->rpc.set_cc_config(config, log_flags);
    if (!retval) {
        pDoc->rpc.read_cc_config();
    }
    this->EndModal(wxID_CANCEL);
}

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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgConnection.h"
#endif

#include "stdwx.h"
#include "DlgConnection.h"


IMPLEMENT_CLASS( CDlgConnection, wxDialog )

BEGIN_EVENT_TABLE( CDlgConnection, wxDialog )

END_EVENT_TABLE()


CDlgConnection::CDlgConnection( )
{
}


CDlgConnection::CDlgConnection( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}


bool CDlgConnection::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{

    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    return TRUE;
}


void CDlgConnection::CreateControls()
{    
    CDlgConnection* item1 = this;

    wxBoxSizer* item2 = new wxBoxSizer(wxVERTICAL);
    item1->SetSizer(item2);
    item1->SetAutoLayout(TRUE);

    wxBoxSizer* item3 = new wxBoxSizer(wxHORIZONTAL);
    item2->Add(item3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* item4 = new wxBoxSizer(wxVERTICAL);
    item3->Add(item4, 0, wxALIGN_TOP|wxALL, 5);

    wxStaticText* item5 = new wxStaticText;
    item5->Create( item1, wxID_STATIC, _("BOINC needs to connect to the network.\nMay it do so now?"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add(item5, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* item6 = new wxBoxSizer(wxVERTICAL);
    item3->Add(item6, 0, wxALIGN_TOP|wxALL, 5);

    wxButton* item7 = new wxButton;
    item7->Create( item1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->SetDefault();
    item6->Add(item7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* item8 = new wxButton;
    item8->Create( item1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item6->Add(item8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}


bool CDlgConnection::ShowToolTips()
{
    return TRUE;
}


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


const char *BOINC_RCSID_9ed9f07f0a = "$Id$";

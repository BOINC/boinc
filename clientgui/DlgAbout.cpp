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
#pragma implementation "DlgAbout.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "DlgAbout.h"
#include "win_config.h"

#include "res/boincsm.xpm"


IMPLEMENT_CLASS( CDlgAbout, wxDialog )

BEGIN_EVENT_TABLE( CDlgAbout, wxDialog )
END_EVENT_TABLE()


CDlgAbout::CDlgAbout( )
{
}

CDlgAbout::CDlgAbout( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}


bool CDlgAbout::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    return TRUE;
}


void CDlgAbout::CreateControls()
{    

    CDlgAbout* item1 = this;

    wxBoxSizer* item2 = new wxBoxSizer(wxVERTICAL);
    item1->SetSizer(item2);
    item1->SetAutoLayout(TRUE);

    wxStaticText* item3 = new wxStaticText;
    item3->Create( item1, wxID_STATIC, wxGetApp().GetAppName(), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    item3->SetFont(wxFont(24, wxDEFAULT, wxNORMAL, wxBOLD, FALSE, _T("")));
    item2->Add(item3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* item4 = new wxBoxSizer(wxHORIZONTAL);
    item2->Add(item4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* item5 = new wxBoxSizer(wxVERTICAL);
    item4->Add(item5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmap item6Bitmap(boincsm_xpm);
    wxStaticBitmap* item6 = new wxStaticBitmap;
    item6->Create( item1, wxID_STATIC, item6Bitmap, wxDefaultPosition, wxSize(50, 50), 0 );
    item5->Add(item6, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* item7 = new wxFlexGridSizer(0, 2, 0, 0);
    item4->Add(item7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* item8 = new wxStaticText;
    item8->Create( item1, wxID_STATIC, _T("Version:"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item9 = new wxStaticText;
    item9->Create( item1, wxID_STATIC, _T( BOINC_VERSION_STRING ), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item10 = new wxStaticText;
    item10->Create( item1, wxID_STATIC, _T("Licence type:"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item11 = new wxStaticText;
    item11->Create( item1, wxID_STATIC, _T("BOINC Public License"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item12 = new wxStaticText;
    item12->Create( item1, wxID_STATIC, _T("Copyright:"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item12, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item13 = new wxStaticText;
    item13->Create( item1, wxID_STATIC, _T("(C) 2004 University of California at Berkeley.\nAll Rights Reserved."), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item14 = new wxStaticText;
    item14->Create( item1, wxID_STATIC, _T("Berkeley Open Infrastructure for Network Computing"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add(item14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* item15 = new wxStaticText;
    item15->Create( item1, wxID_STATIC, _T("A software platform for distributed computing using volunteered computer resources"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add(item15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* item16 = new wxStaticText;
    item16->Create( item1, wxID_STATIC, _T("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add(item16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticLine* item17 = new wxStaticLine;
    item17->Create( item1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    item2->Add(item17, 0, wxGROW|wxALL, 5);

    wxButton* item18 = new wxButton;
    item18->Create( item1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item18->SetDefault();
    item2->Add(item18, 0, wxALIGN_RIGHT|wxALL, 5);

}


bool CDlgAbout::ShowToolTips()
{
    return TRUE;
}


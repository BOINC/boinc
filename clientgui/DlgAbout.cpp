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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgAbout.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "DlgAbout.h"

////@begin XPM images
#include "boincsm.xpm"
////@end XPM images

/*!
 * CDlgHelpAbout type definition
 */

IMPLEMENT_CLASS( CDlgHelpAbout, wxDialog )

/*!
 * CDlgHelpAbout event table definition
 */

BEGIN_EVENT_TABLE( CDlgHelpAbout, wxDialog )

////@begin CDlgHelpAbout event table entries
////@end CDlgHelpAbout event table entries

END_EVENT_TABLE()

/*!
 * CDlgHelpAbout constructors
 */

CDlgHelpAbout::CDlgHelpAbout( )
{
}

CDlgHelpAbout::CDlgHelpAbout( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgHelpAbout creator
 */

bool CDlgHelpAbout::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDlgHelpAbout member initialisation
////@end CDlgHelpAbout member initialisation

////@begin CDlgHelpAbout creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgHelpAbout creation
    return TRUE;
}

/*!
 * Control creation for CDlgHelpAbout
 */

void CDlgHelpAbout::CreateControls()
{    
////@begin CDlgHelpAbout content construction

    CDlgHelpAbout* item1 = this;

    wxBoxSizer* item2 = new wxBoxSizer(wxVERTICAL);
    item1->SetSizer(item2);
    item1->SetAutoLayout(TRUE);

    wxStaticText* item3 = new wxStaticText;
    item3->Create( item1, wxID_STATIC, _("BOINC Core Client"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
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
    item8->Create( item1, wxID_STATIC, _("Version:"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item9 = new wxStaticText;
    item9->Create( item1, wxID_STATIC, _("2.28"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item10 = new wxStaticText;
    item10->Create( item1, wxID_STATIC, _("Licence type:"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item11 = new wxStaticText;
    item11->Create( item1, wxID_STATIC, _("BOINC Public License"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item12 = new wxStaticText;
    item12->Create( item1, wxID_STATIC, _("Copyright:"), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item12, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item13 = new wxStaticText;
    item13->Create( item1, wxID_STATIC, _("(C) 2004 University of California at Berkeley.\nAll Rights Reserved."), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add(item13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* item14 = new wxStaticText;
    item14->Create( item1, wxID_STATIC, _("Berkeley Open Infrastructure for Network Computing"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add(item14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* item15 = new wxStaticText;
    item15->Create( item1, wxID_STATIC, _("A software platform for distributed computing using volunteered computer resources"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add(item15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* item16 = new wxStaticText;
    item16->Create( item1, wxID_STATIC, _("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add(item16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticLine* item17 = new wxStaticLine;
    item17->Create( item1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    item2->Add(item17, 0, wxGROW|wxALL, 5);

    wxButton* item18 = new wxButton;
    item18->Create( item1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item18->SetDefault();
    item2->Add(item18, 0, wxALIGN_RIGHT|wxALL, 5);

////@end CDlgHelpAbout content construction
}

/*!
 * Should we show tooltips?
 */

bool CDlgHelpAbout::ShowToolTips()
{
    return TRUE;
}

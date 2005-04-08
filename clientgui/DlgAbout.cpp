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

#ifdef __WXMSW__
#include "version.h"
#else
#include "config.h"
#endif


////@begin XPM images
#include "res/boincsm.xpm"
////@end XPM images

/*!
 * CDlgAbout type definition
 */

IMPLEMENT_DYNAMIC_CLASS(CDlgAbout, wxDialog)

/*!
 * CDlgAbout event table definition
 */

BEGIN_EVENT_TABLE(CDlgAbout, wxDialog)

////@begin CDlgAbout event table entries
////@end CDlgAbout event table entries

END_EVENT_TABLE()

/*!
 * CDlgAbout constructors
 */

CDlgAbout::CDlgAbout() {}

CDlgAbout::CDlgAbout(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgHelpAbout creator
 */

bool CDlgAbout::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
////@begin CDlgAbout member initialisation
    m_strVersion = BOINC_VERSION_STRING;
////@end CDlgAbout member initialisation

////@begin CDlgAbout creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgAbout creation
    return TRUE;
}

/*!
 * Control creation for CDlgHelpAbout
 */

void CDlgAbout::CreateControls() {    
////@begin CDlgAbout content construction

    CDlgAbout* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText;
    itemStaticText3->Create(itemDialog1, wxID_STATIC, _("BOINC Manager"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
    itemStaticText3->SetFont(wxFont(16, wxDEFAULT, wxNORMAL, wxBOLD, FALSE, _T("")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText;
    itemStaticText8->Create(itemDialog1, wxID_STATIC, _("Version:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer7->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create(itemDialog1, wxID_STATIC, _T(""), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer7->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText12 = new wxStaticText;
    itemStaticText12->Create(itemDialog1, wxID_STATIC, _("Copyright:"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer7->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText13 = new wxStaticText;
    itemStaticText13->Create(itemDialog1, wxID_STATIC, _("(C) 2005 University of California at Berkeley.\nAll Rights Reserved."), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer7->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText14 = new wxStaticText;
    itemStaticText14->Create(itemDialog1, wxID_STATIC, _("Berkeley Open Infrastructure for Network Computing"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(itemStaticText14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText16 = new wxStaticText;
    itemStaticText16->Create(itemDialog1, wxID_STATIC, _("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticLine* itemStaticLine17 = new wxStaticLine;
    itemStaticLine17->Create(itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    itemBoxSizer2->Add(itemStaticLine17, 0, wxGROW|wxALL, 5);

    wxButton* itemButton18 = new wxButton;
    itemButton18->Create(itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
    itemButton18->SetDefault();
    itemBoxSizer2->Add(itemButton18, 0, wxALIGN_RIGHT|wxALL, 5);

    // Set validators
    itemStaticText9->SetValidator(wxGenericValidator(& m_strVersion));
////@end CDlgAbout content construction
}

/*!
 * Should we show tooltips?
 */

bool CDlgAbout::ShowToolTips() {
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDlgAbout::GetBitmapResource(const wxString& name) {
    // Bitmap retrieval
////@begin CDlgAbout bitmap retrieval
    if (name == wxT("res/boincsm.xpm")) {
        wxBitmap bitmap(boincsm_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CDlgAbout bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgAbout::GetIconResource(const wxString& name) {
    // Icon retrieval
////@begin CDlgAbout icon retrieval
    return wxNullIcon;
////@end CDlgAbout icon retrieval
}

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
#pragma implementation "DlgAttachProject.h"
#endif

#include "stdwx.h"
#include "DlgAttachProject.h"
#include "ValidateURL.h"
#include "ValidateAccountKey.h"


IMPLEMENT_CLASS( CDlgAttachProject, wxDialog )

BEGIN_EVENT_TABLE( CDlgAttachProject, wxDialog )

END_EVENT_TABLE()


CDlgAttachProject::CDlgAttachProject( )
{
}


CDlgAttachProject::CDlgAttachProject( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}


bool CDlgAttachProject::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_ProjectAddressCtrl = NULL;
    m_ProjectAccountKeyCtrl = NULL;

    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    return TRUE;
}


void CDlgAttachProject::CreateControls()
{    

    CDlgAttachProject* item1 = this;

    wxBoxSizer* item2 = new wxBoxSizer(wxVERTICAL);
    item1->SetSizer(item2);
    item1->SetAutoLayout(TRUE);

    wxBoxSizer* item3 = new wxBoxSizer(wxHORIZONTAL);
    item2->Add(item3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* item4 = new wxFlexGridSizer(2, 2, 0, 0);
    item3->Add(item4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* item5 = new wxStaticText;
    item5->Create( item1, wxID_STATIC, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add(item5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* item6 = new wxTextCtrl;
    item6->Create( item1, ID_PROJECTADDRESS, wxT(""), wxDefaultPosition, wxSize(200, -1), 0, CValidateURL(&m_strProjectAddress) );
    m_ProjectAddressCtrl = item6;
    item4->Add(item6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* item7 = new wxStaticText;
    item7->Create( item1, wxID_STATIC, _("Account Key:"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add(item7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxTextCtrl* item8 = new wxTextCtrl;
    item8->Create( item1, ID_PROJECTACCOUNTKEY, wxT(""), wxDefaultPosition, wxSize(200, -1), 0, CValidateAccountKey(&m_strProjectAccountKey) );
    m_ProjectAccountKeyCtrl = item8;
    item4->Add(item8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* item9 = new wxBoxSizer(wxVERTICAL);
    item3->Add(item9, 0, wxALIGN_TOP|wxALL, 5);

    wxButton* item10 = new wxButton;
    item10->Create( item1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item10->SetDefault();
    item9->Add(item10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* item11 = new wxButton;
    item11->Create( item1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item9->Add(item11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

}

bool CDlgAttachProject::ShowToolTips()
{
    return TRUE;
}


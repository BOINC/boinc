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

#include "stdwx.h"
#include ".\workview.h"


IMPLEMENT_DYNAMIC_CLASS(CWorkView, CBaseListCtrlView)


CWorkView::CWorkView(void)
{
}


CWorkView::CWorkView(wxNotebook* pNotebook) :
    CBaseListCtrlView(pNotebook)
{
    InsertColumn(0, _("Project"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(1, _("Application"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(2, _("Name"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(3, _("CPU time"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Progress"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("To Completetion"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Report Deadline"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Status"), wxLIST_FORMAT_LEFT, -1);
}


CWorkView::~CWorkView(void)
{
}


wxString CWorkView::GetViewName(void)
{
    return wxString(_("Work"));
}

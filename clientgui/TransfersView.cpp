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
// $Log$
// Revision 1.6  2004/05/17 22:15:09  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "TransfersView.h"
#endif

#include "stdwx.h"
#include "TransfersView.h"

#include "res/xfer.xpm"


IMPLEMENT_DYNAMIC_CLASS(CTransfersView, CBaseListCtrlView)


CTransfersView::CTransfersView(void)
{
}


CTransfersView::CTransfersView(wxNotebook* pNotebook) :
    CBaseListCtrlView(pNotebook)
{
    InsertColumn(0, _("Project"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(1, _("File"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(2, _("Progress"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(3, _("Size"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Time"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Speed"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(4, _("Status"), wxLIST_FORMAT_LEFT, -1);
}


CTransfersView::~CTransfersView(void)
{
}


wxString CTransfersView::GetViewName(void)
{
    return wxString(_("Transfers"));
}


char** CTransfersView::GetViewIcon(void)
{
    return xfer_xpm;
}


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
// Revision 1.13  2004/09/01 04:59:32  rwalton
// *** empty log message ***
//
// Revision 1.12  2004/08/11 23:52:12  rwalton
// *** empty log message ***
//
// Revision 1.11  2004/07/13 05:56:01  rwalton
// Hooked up the Project and Work tab for the new GUI.
//
// Revision 1.10  2004/05/29 00:09:40  rwalton
// *** empty log message ***
//
// Revision 1.9  2004/05/27 06:17:57  rwalton
// *** empty log message ***
//
// Revision 1.8  2004/05/24 23:50:14  rwalton
// *** empty log message ***
//
// Revision 1.7  2004/05/21 06:27:15  rwalton
// *** empty log message ***
//
// Revision 1.6  2004/05/17 22:15:09  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "MessagesView.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "MessagesView.h"
#include "Events.h"

#include "res/mess.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2


IMPLEMENT_DYNAMIC_CLASS(CMessagesView, CBaseListCtrlView)

BEGIN_EVENT_TABLE(CMessagesView, CBaseListCtrlView)
    EVT_LIST_CACHE_HINT(ID_LIST_MESSAGESVIEW, CMessagesView::OnCacheHint)
END_EVENT_TABLE()


CMessagesView::CMessagesView()
{
}


CMessagesView::CMessagesView(wxNotebook* pNotebook) :
    CBaseListCtrlView(pNotebook, ID_LIST_MESSAGESVIEW)
{
    InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, -1);
    InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, -1);
}


CMessagesView::~CMessagesView()
{
}


wxString CMessagesView::GetViewName()
{
    return wxString(_("Messages"));
}


char** CMessagesView::GetViewIcon()
{
    return mess_xpm;
}


void CMessagesView::OnCacheHint ( wxListEvent& event ) {
    m_iCacheFrom = event.GetCacheFrom();
    m_iCacheTo = event.GetCacheTo();
}


void CMessagesView::OnRender(wxTimerEvent &event) {
    wxLogTrace("CMessagesView::OnRender - Function Begining");
    wxLogTrace("CMessagesView::OnRender - Function Ending");
}


wxString CMessagesView::OnGetItemText(long item, long column) const {
    wxString strBuffer;
    switch(column) {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) wxGetApp().GetDocument()->CachedStateLock();
            strBuffer = wxGetApp().GetDocument()->GetMessageProjectName(item);
            break;
        case COLUMN_TIME:
            strBuffer = wxGetApp().GetDocument()->GetMessageTime(item);
            break;
        case COLUMN_MESSAGE:
            strBuffer = wxGetApp().GetDocument()->GetMessageMessage(item);
            if (item == m_iCacheTo) wxGetApp().GetDocument()->CachedStateUnlock();
            break;
    }
    return strBuffer;
}


int CMessagesView::OnGetItemImage(long item) const {
    return -1;
}


wxListItemAttr* CMessagesView::OnGetItemAttr(long item) const {

    return NULL;
}


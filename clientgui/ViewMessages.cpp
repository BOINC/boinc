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
// Revision 1.2  2004/09/24 02:01:50  rwalton
// *** empty log message ***
//
// Revision 1.1  2004/09/21 01:26:25  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewMessages.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "ViewMessages.h"
#include "Events.h"

#include "res/mess.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2


IMPLEMENT_DYNAMIC_CLASS(CViewMessages, CBOINCBaseView)

BEGIN_EVENT_TABLE(CViewMessages, CBOINCBaseView)
END_EVENT_TABLE()


CViewMessages::CViewMessages()
{
    wxLogTrace("CViewMessages::CViewMessages - Function Begining");
    wxLogTrace("CViewMessages::CViewMessages - Function Ending");
}


CViewMessages::CViewMessages(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_MESSAGESVIEW, ID_LIST_MESSAGESVIEW)
{
    m_bProcessingRenderEvent = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, -1);
}


CViewMessages::~CViewMessages()
{
    wxLogTrace("CViewMessages::~CViewMessages - Function Begining");
    wxLogTrace("CViewMessages::~CViewMessages - Function Ending");
}


wxString CViewMessages::GetViewName()
{
    return wxString(_("Messages"));
}


char** CViewMessages::GetViewIcon()
{
    return mess_xpm;
}


void CViewMessages::OnRender(wxTimerEvent &event)
{
    wxLogTrace("CViewMessages::OnRender - Processing Render Event...");
    if (!m_bProcessingRenderEvent)
    {
        m_bProcessingRenderEvent = true;

        wxInt32 iProjectCount = wxGetApp().GetDocument()->GetMessageCount();
        wxASSERT(NULL != m_pListPane);
        m_pListPane->SetItemCount(iProjectCount);

        m_bProcessingRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


wxString CViewMessages::OnGetItemText(long item, long column) const
{
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


int CViewMessages::OnGetItemImage(long item) const
{
    return -1;
}


wxListItemAttr* CViewMessages::OnGetItemAttr(long item) const
{
    return NULL;
}


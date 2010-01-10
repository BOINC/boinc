// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewNotifications.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "ViewNotifications.h"
#include "Events.h"
#include "error_numbers.h"


#include "res/mess.xpm"


IMPLEMENT_DYNAMIC_CLASS(CViewNotifications, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewNotifications, CBOINCBaseView)
END_EVENT_TABLE ()


CViewNotifications::CViewNotifications()
{}


CViewNotifications::CViewNotifications(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook)
{
    CTaskItemGroup* pGroup = NULL;
    CTaskItem*      pItem = NULL;

    //
    // Setup View
    //
    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);

    m_pTaskPane = new CBOINCTaskCtrl(this, ID_TASK_NOTIFICATIONSVIEW, DEFAULT_TASK_FLAGS);
    wxASSERT(m_pTaskPane);

	m_pHtmlPane = new wxHtmlWindow(this, ID_HTML_NOTIFICATIONSVIEW, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO | wxHSCROLL | wxVSCROLL);
	wxASSERT(m_pHtmlPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pHtmlPane, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();

    m_iOldNoticeCount = 0;

    pGroup = new CTaskItemGroup(_("News Feeds"));
    m_TaskGroups.push_back(pGroup);

    pItem = new CTaskItem(
        _("BOINC"),
        _("Display the latest news about BOINC"),
        ID_TASK_NEWS_BOINC
    );
    pGroup->m_Tasks.push_back(pItem);

    m_TaskGroups.push_back(pGroup);

    // Create Task Pane Items
    m_pTaskPane->UpdateControls();
}


CViewNotifications::~CViewNotifications() {
}


wxString& CViewNotifications::GetViewName() {
    static wxString strViewName(wxT("Notices"));
    return strViewName;
}


wxString& CViewNotifications::GetViewDisplayName() {
    static wxString strViewName(_("Notices"));
    return strViewName;
}


const char** CViewNotifications::GetViewIcon() {
    return mess_xpm;
}


const int CViewNotifications::GetViewRefreshRate() {
    return 10;
}

const int CViewNotifications::GetViewCurrentViewPage() {
     return VW_NOTIF;
}


bool CViewNotifications::OnSaveState(wxConfigBase* WXUNUSED(pConfig)) {
    return true;
}


bool CViewNotifications::OnRestoreState(wxConfigBase* WXUNUSED(pConfig)) {
    return true;
}


void CViewNotifications::OnListRender(wxTimerEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewNotifications::OnListRender - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    wxString strHTML;
    wxString strItems;
    wxString strTemp;
    wxDateTime dtBuffer;
    int n = 0;
    unsigned int i = 0;
    static bool s_bInProgress = false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
	wxASSERT(m_pHtmlPane);

    if (s_bInProgress) return;
    s_bInProgress = true;

    n = pDoc->GetNoticeCount();
    if (n == -1) {
        strItems +=   _("Retrieving notices...");
    } else {
        // Update display only if there is something new
        if (n == m_iOldNoticeCount) {
            goto done;
        }
        m_iOldNoticeCount = n;

        // Pre-allocate buffer size so string concat is much faster
        strItems.Alloc(4096*n);

        for (i=0; i < (unsigned int)n; i++) {
            NOTICE* np = pDoc->notice(i);
            if (!np) continue;
            char tbuf[512];
            if (strlen(np->title)) {
                sprintf(tbuf, "<b>%s</b><br>", np->title);
                strItems += wxString(tbuf, wxConvUTF8);

            }
            strItems += wxString(np->description.c_str(), wxConvUTF8);
            strItems += wxT("<br><font size=-2 color=#8f8f8f>");
            dtBuffer.Set((time_t)np->arrival_time);
            strItems += dtBuffer.Format();
            strItems += wxT("</font><hr>\n");
        }
    }
    strHTML  = wxT("<html>\n<body>\n");
    strHTML += strItems;
    //strHTML += wxT("<br><img src=http://boinc.berkeley.edu/logo/www_logo.gif>\n");
    strHTML += wxT("</body>\n</html>\n");
    m_pHtmlPane->SetFonts(wxT("Sans Serif"), wxT("Courier"), 0);
    m_pHtmlPane->SetPage(strHTML);

done:
    s_bInProgress = false;

    wxLogTrace(wxT("Function Start/End"), wxT("CViewNotifications::OnListRender - Function End"));
}

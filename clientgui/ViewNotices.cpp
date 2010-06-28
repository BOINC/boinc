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
#pragma implementation "ViewNotices.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCTaskCtrl.h"
#include "ViewNotices.h"
#include "NoticeListCtrl.h"
#include "Events.h"
#include "error_numbers.h"


#include "res/mess.xpm"


IMPLEMENT_DYNAMIC_CLASS(CViewNotices, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewNotices, CBOINCBaseView)
    EVT_NOTICELIST_ITEM_DISPLAY(CViewNotices::OnLinkClicked)
END_EVENT_TABLE ()


CViewNotices::CViewNotices()
{}


CViewNotices::CViewNotices(wxNotebook* pNotebook) :
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

	m_pHtmlListPane = new CNoticeListCtrl(this);
	wxASSERT(m_pHtmlListPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pHtmlListPane, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();

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


CViewNotices::~CViewNotices() {
}


wxString& CViewNotices::GetViewName() {
    static wxString strViewName(wxT("Notices"));
    return strViewName;
}


wxString& CViewNotices::GetViewDisplayName() {
    static wxString strViewName(_("Notices"));
    return strViewName;
}


const char** CViewNotices::GetViewIcon() {
    return mess_xpm;
}


const int CViewNotices::GetViewRefreshRate() {
    return 10;
}

const int CViewNotices::GetViewCurrentViewPage() {
     return VW_NOTIF;
}


bool CViewNotices::OnSaveState(wxConfigBase* WXUNUSED(pConfig)) {
    return true;
}


bool CViewNotices::OnRestoreState(wxConfigBase* WXUNUSED(pConfig)) {
    return true;
}


void CViewNotices::OnListRender(wxTimerEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewNotices::OnListRender - Function Begin"));

    CMainDocument*  pDoc   = wxGetApp().GetDocument();
    wxString strTitle;
    wxString strDescription;
    wxString strCategory;
    wxString strProjectName;
    wxString strURL;
    wxString strArrivalTime;
    wxDateTime dtBuffer;
    int n = 0;
    unsigned int i = 0;
    static bool s_bInProgress = false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
	wxASSERT(m_pHtmlListPane);

    if (s_bInProgress) return;
    s_bInProgress = true;

    n = pDoc->GetNoticeCount();
    if (n != -1) {

        m_pHtmlListPane->Freeze();
        m_pHtmlListPane->FlagAllItemsForDelete();

        for (i = n; i != 0; i--) {
            NOTICE* np = pDoc->notice(i-1);

            if (!np) continue;

            if (!m_pHtmlListPane->Exists(np->seqno)) {

                strProjectName = wxString(np->project_name, wxConvUTF8);
                strURL = wxString(np->link, wxConvUTF8);
                strTitle = wxString(process_client_message(np->title), wxConvUTF8);
                strDescription = wxString(process_client_message(np->description.c_str()), wxConvUTF8);
                strCategory = wxString(np->category, wxConvUTF8);

                dtBuffer.Set((time_t)np->arrival_time);
                strArrivalTime = dtBuffer.Format();

                m_pHtmlListPane->Add(
                    np->seqno,
                    strProjectName,
                    strURL, 
                    strTitle,
                    strDescription,
                    strCategory,
                    strArrivalTime
                );

            } else {

                dtBuffer.Set((time_t)np->arrival_time);
                strArrivalTime = dtBuffer.Format();

                m_pHtmlListPane->Update(
                    np->seqno,
                    strArrivalTime
                );

            }
        }

        m_pHtmlListPane->DeleteAllFlagedItems();
        m_pHtmlListPane->Thaw();
    }

    s_bInProgress = false;

    wxLogTrace(wxT("Function Start/End"), wxT("CViewNotices::OnListRender - Function End"));
}


void CViewNotices::OnLinkClicked( NoticeListCtrlEvent& event ) {
    if (event.GetURL().StartsWith(wxT("http://"))) {
		wxLaunchDefaultBrowser(event.GetURL());
    }
}


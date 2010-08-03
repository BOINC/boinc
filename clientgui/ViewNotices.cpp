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

    static bool s_bInProgress = false;
    static wxString strLastMachineName = wxEmptyString;
    wxString strNewMachineName = wxEmptyString;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
	wxASSERT(m_pHtmlListPane);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (s_bInProgress) return;
    s_bInProgress = true;

    if (pDoc->IsConnected()) {
        pDoc->GetConnectedComputerName(strNewMachineName);
        if (strLastMachineName != strNewMachineName) {
            strLastMachineName = strNewMachineName;
            m_pHtmlListPane->Clear();
        }
    }

    m_pHtmlListPane->Freeze();
    m_pHtmlListPane->UpdateUI();
    m_pHtmlListPane->Thaw();

    pDoc->UpdateUnreadNoticeState();

    s_bInProgress = false;

    wxLogTrace(wxT("Function Start/End"), wxT("CViewNotices::OnListRender - Function End"));
}


void CViewNotices::OnLinkClicked( NoticeListCtrlEvent& event ) {
    if (event.GetURL().StartsWith(wxT("http://"))) {
		wxLaunchDefaultBrowser(event.GetURL());
    }
}


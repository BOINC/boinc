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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCBaseView.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCBaseView.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "Events.h"

#include "res/boinc.xpm"


IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseView, wxPanel)


CBOINCBaseView::CBOINCBaseView() {}

CBOINCBaseView::CBOINCBaseView(wxNotebook* pNotebook) :
    wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
    wxASSERT(pNotebook);

    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    m_bForceUpdateSelection = true;

    //
    // Setup View
    //
    m_pTaskPane = NULL;
    m_pListPane = NULL;

    SetName(GetViewName());

    SetAutoLayout(TRUE);
}


CBOINCBaseView::CBOINCBaseView(
    wxNotebook* pNotebook, wxWindowID iTaskWindowID, int iTaskWindowFlags, wxWindowID iListWindowID, int iListWindowFlags) :
    wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
    wxASSERT(pNotebook);

    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    m_bForceUpdateSelection = true;

    //
    // Setup View
    //
    m_pTaskPane = NULL;
    m_pListPane = NULL;

    SetName(GetViewName());

    SetAutoLayout(TRUE);

    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);
    
    m_pTaskPane = new CBOINCTaskCtrl(this, iTaskWindowID, iTaskWindowFlags);
    wxASSERT(m_pTaskPane);

    m_pListPane = new CBOINCListCtrl(this, iListWindowID, iListWindowFlags);
    wxASSERT(m_pListPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pListPane, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();
}


CBOINCBaseView::~CBOINCBaseView() {}


// The user friendly name of the view.
//   If it has not been defined by the view "Undefined" is returned.
//
wxString& CBOINCBaseView::GetViewName() {
    static wxString strViewName(wxT("Undefined"));
    return strViewName;
}


// The user friendly icon of the view.
//   If it has not been defined by the view the BOINC icon is returned.
//
const char** CBOINCBaseView::GetViewIcon() {
    wxASSERT(boinc_xpm);
    return boinc_xpm;
}


bool CBOINCBaseView::FireOnSaveState(wxConfigBase* pConfig) {
    return OnSaveState(pConfig);
}


bool CBOINCBaseView::FireOnRestoreState(wxConfigBase* pConfig) {
    return OnRestoreState(pConfig);
}


int CBOINCBaseView::GetListRowCount() {
    wxASSERT(m_pListPane);
    return m_pListPane->GetItemCount();
}


void CBOINCBaseView::FireOnListRender(wxTimerEvent& event) {
    OnListRender(event);
}


void CBOINCBaseView::FireOnListSelected(wxListEvent& event) {
    OnListSelected(event);
}


void CBOINCBaseView::FireOnListDeselected(wxListEvent& event) {
    OnListDeselected(event);
}


wxString CBOINCBaseView::FireOnListGetItemText(long item, long column) const {
    return OnListGetItemText(item, column);
}


int CBOINCBaseView::FireOnListGetItemImage(long item) const {
    return OnListGetItemImage(item);
}


wxListItemAttr* CBOINCBaseView::FireOnListGetItemAttr(long item) const {
    return OnListGetItemAttr(item);
}


void CBOINCBaseView::OnListRender(wxTimerEvent& event) {
    if (!m_bProcessingListRenderEvent) {
        m_bProcessingListRenderEvent = true;

        wxASSERT(m_pListPane);

        int iDocCount = GetDocCount();
        int iCacheCount = GetCacheCount();
        if (iDocCount != iCacheCount) {
            if (0 >= iDocCount) {
                EmptyCache();
                m_pListPane->DeleteAllItems();
            } else {
                int iIndex = 0;
                int iReturnValue = -1;
                if (iDocCount > iCacheCount) {
                    for (iIndex = 0; iIndex < (iDocCount - iCacheCount); iIndex++
                    ) {
                        iReturnValue = AddCacheElement();
                        wxASSERT(!iReturnValue);
                    }
                    wxASSERT(GetDocCount() == GetCacheCount());
                } else {
                    for (iIndex = 0; iIndex < (iCacheCount - iDocCount); iIndex++
                    ) {
                        iReturnValue = RemoveCacheElement();
                        wxASSERT(!iReturnValue);
                    }
                    wxASSERT(GetDocCount() == GetCacheCount());
                }

                m_pListPane->SetItemCount(iDocCount);
            }
        }

        if (iDocCount) {
            SyncronizeCache();
        }

        if (_EnsureLastItemVisible() && (iDocCount != iCacheCount)) {
            m_pListPane->EnsureVisible(iDocCount - 1);
        }

        // If no item has been selected yet, select the first item.
#ifdef __WXMSW__
         if ((m_pListPane->GetSelectedItemCount() == 0) &&
            (m_pListPane->GetItemCount() >= 1)) {

            long desiredstate = wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED;
            m_pListPane->SetItemState(0, desiredstate, desiredstate);
        }
#else
         if ((m_pListPane->GetFirstSelected() < 0) &&
            (m_pListPane->GetItemCount() >= 1))
            m_pListPane->SetItemState(0, wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED, 
                                            wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED);
#endif

        UpdateSelection();

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
}


bool CBOINCBaseView::OnSaveState(wxConfigBase* pConfig) {
    bool bReturnValue = true;

    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    if (!m_pListPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    return bReturnValue;
}


bool CBOINCBaseView::OnRestoreState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }

    if (!m_pListPane->OnRestoreState(pConfig)) {
        return false;
    }
    return true;
}


void CBOINCBaseView::OnListSelected(wxListEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListSelected - Function Begin"));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListSelected - Function End"));
}


void CBOINCBaseView::OnListDeselected(wxListEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListDeselected - Function Begin"));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListDeselected - Function End"));
}


wxString CBOINCBaseView::OnListGetItemText(
    long WXUNUSED(item), long WXUNUSED(column)
) const {
    return wxString("Undefined");
}


int CBOINCBaseView::OnListGetItemImage(long WXUNUSED(item)) const {
    return -1;
}


wxListItemAttr* CBOINCBaseView::OnListGetItemAttr(long WXUNUSED(item)) const {
    return NULL;
}


int CBOINCBaseView::GetDocCount() {
    return 0;
}


wxString CBOINCBaseView::OnDocGetItemText(long WXUNUSED(item), long WXUNUSED(column)) const {
    return wxString("Undefined");
}


wxString CBOINCBaseView::OnDocGetItemImage(long WXUNUSED(item)) const {
    return wxString("Undefined");
}


wxString CBOINCBaseView::OnDocGetItemAttr(long WXUNUSED(item)) const {
    return wxString("Undefined");
}


int CBOINCBaseView::AddCacheElement() {
    return -1;
}

    
int CBOINCBaseView::EmptyCache() {
    return -1;
}


int CBOINCBaseView::GetCacheCount() {
    return -1;
}


int CBOINCBaseView::RemoveCacheElement() {
    return -1;
}


int CBOINCBaseView::SyncronizeCache() {
    int         iRowIndex        = 0;
    int         iRowTotal        = 0;
    int         iColumnIndex     = 0;
    int         iColumnTotal     = 0;
    wxString    strDocumentText  = wxEmptyString;
    wxString    strListPaneText  = wxEmptyString;
    bool        bNeedRefreshData = false;

    iRowTotal = GetDocCount();
    iColumnTotal = m_pListPane->GetColumnCount();

    for (iRowIndex = 0; iRowIndex < iRowTotal; iRowIndex++) {
        bNeedRefreshData = false;

        for (iColumnIndex = 0; iColumnIndex < iColumnTotal; iColumnIndex++) {
            strDocumentText.Empty();
            strListPaneText.Empty();

            strDocumentText = OnDocGetItemText(iRowIndex, iColumnIndex);
            strListPaneText = OnListGetItemText(iRowIndex, iColumnIndex);

            if (!strDocumentText.IsSameAs(strListPaneText)) {
                if (0 != UpdateCache(iRowIndex, iColumnIndex, strDocumentText)) {
                    wxASSERT(FALSE);
                }
                bNeedRefreshData = true;
            }
        }

        if (bNeedRefreshData) {
            m_pListPane->RefreshItem(iRowIndex);
        }
    }

    return 0;
}


int CBOINCBaseView::UpdateCache(
    long WXUNUSED(item), long WXUNUSED(column), wxString& WXUNUSED(strNewData)
) {
    return -1;
}


void CBOINCBaseView::EmptyTasks() {
    unsigned int i;
    unsigned int j;
    for (i=0; i<m_TaskGroups.size(); i++) {
        for (j=0; j<m_TaskGroups[i]->m_Tasks.size(); j++) {
            delete m_TaskGroups[i]->m_Tasks[j];
        }
        m_TaskGroups[i]->m_Tasks.clear();
        delete m_TaskGroups[i];
    }
    m_TaskGroups.clear();
}

    
void CBOINCBaseView::PreUpdateSelection(){
}


void CBOINCBaseView::UpdateSelection(){
}


void CBOINCBaseView::PostUpdateSelection(){
    wxASSERT(m_pTaskPane);
    m_pTaskPane->UpdateControls();
    Layout();
}


void CBOINCBaseView::UpdateWebsiteSelection(long lControlGroup, PROJECT* project){
    unsigned int        i;
    CTaskItemGroup*     pGroup = NULL;
    CTaskItem*          pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    // Update the websites list
    //
    if (m_bForceUpdateSelection) {
        if (m_TaskGroups.size() > 1) {

            // Delete task group, objects, and controls.
            pGroup = m_TaskGroups[lControlGroup];

            m_pTaskPane->DeleteTaskGroupAndTasks(pGroup);
            for (i=0; i<pGroup->m_Tasks.size(); i++) {
                delete pGroup->m_Tasks[i];
            }
            pGroup->m_Tasks.clear();
            delete pGroup;

            pGroup = NULL;

            m_TaskGroups.erase( m_TaskGroups.begin() + 1 );
        }

        // If something is selected create the tasks and controls
        if (m_pListPane->GetSelectedItemCount()) {
            if (project) {
                // Create the web sites task group
  	            pGroup = new CTaskItemGroup( _("Web sites") );
	            m_TaskGroups.push_back( pGroup );

                // Default project url
                pItem = new CTaskItem(
                    project->project_name.c_str(), 
                    wxT(""), 
                    project->master_url.c_str(),
                    ID_TASK_PROJECT_WEB_PROJDEF_MIN
                );
                pGroup->m_Tasks.push_back(pItem);


                // Project defined urls
                for (i=0;(i<project->gui_urls.size())&&(i<=ID_TASK_PROJECT_WEB_PROJDEF_MAX);i++) {
                    pItem = new CTaskItem(
                        _(project->gui_urls[i].name.c_str()),
                        _(project->gui_urls[i].description.c_str()),
                        project->gui_urls[i].url.c_str(),
                        ID_TASK_PROJECT_WEB_PROJDEF_MIN + 1 + i
                    );
                    pGroup->m_Tasks.push_back(pItem);
                }
            }
        }

        m_bForceUpdateSelection = false;
    }

}


bool CBOINCBaseView::_EnsureLastItemVisible() {
    return EnsureLastItemVisible();
}


bool CBOINCBaseView::EnsureLastItemVisible() {
    return false;
}


void CBOINCBaseView::append_to_status(wxString& existing, const wxChar* additional) {
    if (existing.size() == 0) {
        existing = additional;
    } else {
        existing = existing + ", " + additional;
    }
}


const char *BOINC_RCSID_0a1bd38a5a = "$Id$";

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
#pragma implementation "ViewResources.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewResources.h"
#include "Events.h"


#include "res/usage.xpm"


#define COLUMN_PROJECT              0
#define COLUMN_DISKSPACE            1


CResource::CResource() {
    m_strProjectName = wxEmptyString;
    m_strDiskSpace = wxEmptyString;
}


CResource::~CResource() {
    m_strProjectName.Clear();
    m_strDiskSpace.Clear();
}


IMPLEMENT_DYNAMIC_CLASS(CViewResources, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewResources, CBOINCBaseView)
END_EVENT_TABLE ()


CViewResources::CViewResources() {}


CViewResources::CViewResources(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_TASK_RESOURCEUTILIZATIONVIEW, DEFAULT_TASK_FLAGS, ID_LIST_RESOURCEUTILIZATIONVIEW, DEFAULT_LIST_SINGLE_SEL_FLAGS)
{
	CTaskItemGroup* pGroup = NULL;
	CTaskItem*      pItem = NULL;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    //
    // Setup View
    //

    // Create Task Pane Items
    m_pTaskPane->CreateTaskControls();

    // Create List Pane Items
    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_DISKSPACE, _("Disk Space"), wxLIST_FORMAT_LEFT, -1);
}


CViewResources::~CViewResources() {
    EmptyCache();
    EmptyTasks();
}


wxString CViewResources::GetViewName() {
    return wxString(_("Disk"));
}


const char** CViewResources::GetViewIcon() {
    return usage_xpm;
}


wxInt32 CViewResources::GetDocCount() {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetResourceCount();
}


wxString CViewResources::OnListGetItemText(long item, long column) const {
    CResource* resource   = m_ResourceCache.at(item);
    wxString   strBuffer  = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            strBuffer = resource->m_strProjectName;
            break;
        case COLUMN_DISKSPACE:
            strBuffer = resource->m_strDiskSpace;
            break;
    }

    return strBuffer;
}


wxString CViewResources::OnDocGetItemText(long item, long column) const {
    wxString       strBuffer = wxEmptyString;

    switch(column) {
    case COLUMN_PROJECT:
        FormatProjectName(item, strBuffer);
        break;
    case COLUMN_DISKSPACE:
        FormatDiskSpace(item, strBuffer);
        break;
    }
    return strBuffer;
}


wxInt32 CViewResources::AddCacheElement() {
    CResource* pItem = new CResource();
    wxASSERT(NULL != pItem);
    if (NULL != pItem) {
        m_ResourceCache.push_back(pItem);
        return 0;
    }
    return -1;
}


wxInt32 CViewResources::EmptyCache()
{
    unsigned int i;
    for (i=0; i<m_ResourceCache.size(); i++) {
        delete m_ResourceCache[i];
    }
    m_ResourceCache.clear();
    return 0;
}


wxInt32 CViewResources::GetCacheCount() {
    return m_ResourceCache.size();
}


wxInt32 CViewResources::RemoveCacheElement() {
    delete m_ResourceCache.back();
    m_ResourceCache.erase(m_ResourceCache.end() - 1);
    return 0;
}


wxInt32 CViewResources::UpdateCache(long item, long column, wxString& strNewData) {
    CResource* resource   = m_ResourceCache.at(item);

    switch(column) {
    case COLUMN_PROJECT:
        resource->m_strProjectName = strNewData;
        break;
    case COLUMN_DISKSPACE:
        resource->m_strDiskSpace = strNewData;
        break;
    }
    return 0;
}


void CViewResources::UpdateSelection() {
}


void CViewResources::UpdateTaskPane() {
}


wxInt32 CViewResources::FormatProjectName(wxInt32 item, wxString& strBuffer) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetResourceProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewResources::FormatDiskSpace(wxInt32 item, wxString& strBuffer) const {
    float          fBuffer = 0;
    double         xTera = 1099511627776.0;
    double         xGiga = 1073741824.0;
    double         xMega = 1048576.0;
    double         xKilo = 1024.0;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetResourceDiskspace(item, fBuffer);

    if (fBuffer >= xTera) {
        strBuffer.Printf(wxT("%0.2f TB"), fBuffer/xTera);
    } else if (fBuffer >= xGiga) {
        strBuffer.Printf(wxT("%0.2f GB"), fBuffer/xGiga);
    } else if (fBuffer >= xMega) {
        strBuffer.Printf(wxT("%0.2f MB"), fBuffer/xMega);
    } else if (fBuffer >= xKilo) {
        strBuffer.Printf(wxT("%0.2f KB"), fBuffer/xKilo);
    } else {
        strBuffer.Printf(wxT("%0.0f bytes"), fBuffer);
    }

    return 0;
}


const char *BOINC_RCSID_5a37b46a6e = "$Id$";

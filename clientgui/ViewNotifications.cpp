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


#include "res/xfer.xpm"


IMPLEMENT_DYNAMIC_CLASS(CViewNotifications, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewNotifications, CBOINCBaseView)
    EVT_BUTTON(ID_TASK_NEWS_BOINC, CViewNotifications::OnNewsBOINC)
    EVT_BUTTON(ID_TASK_NEWS_BOINCWEBSITE, CViewNotifications::OnNewsBOINCWebsite)
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

	m_pHtmlPane = new wxHtmlWindow(this, ID_HTML_NOTIFICATIONSVIEW, wxDefaultPosition, wxSize(640, -1), wxHW_SCROLLBAR_AUTO | wxHSCROLL | wxVSCROLL);
	wxASSERT(m_pHtmlPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pHtmlPane, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();

	pGroup = new CTaskItemGroup( _("News Feeds") );
	m_TaskGroups.push_back( pGroup );

	pItem = new CTaskItem(
        _("BOINC"),
        _("Display the latest news about BOINC"),
        ID_TASK_NEWS_BOINC 
    );
    pGroup->m_Tasks.push_back( pItem );

	pItem = new CTaskItem(
        _("BOINC Website"),
        _("Display the latest news about BOINC from the BOINC website"),
        ID_TASK_NEWS_BOINCWEBSITE
    );
    pGroup->m_Tasks.push_back( pItem );

    m_TaskGroups.push_back( pGroup );

    // Create Task Pane Items
    m_pTaskPane->UpdateControls();

    // Display the BOINC website by default.
    wxCommandEvent evt;
    OnNewsBOINC(evt);
}


CViewNotifications::~CViewNotifications() {
}


wxString& CViewNotifications::GetViewName() {
    static wxString strViewName(wxT("Notifications"));
    return strViewName;
}


wxString& CViewNotifications::GetViewDisplayName() {
    static wxString strViewName(_("Notifications"));
    return strViewName;
}


const char** CViewNotifications::GetViewIcon() {
    return xfer_xpm;
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


void CViewNotifications::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
}


void CViewNotifications::OnNewsBOINC( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewNotifications::OnNewsBOINC - Function Begin"));

    wxString strHTML = wxEmptyString;
    wxString strNewsFile = 
        wxGetApp().GetRootDirectory() + wxFileName::GetPathSeparator() + wxT("news.html");
    
    wxFFile f(strNewsFile.c_str());
    if (f.IsOpened()) {
        f.ReadAll(&strHTML);
        f.Close();
    } else {
        strHTML.Printf (
            wxT("<HTML><HEAD></HEAD><BODY>Could not open %s</BODY></HTML>"),
            strNewsFile.c_str()
        );
    }

	wxASSERT(m_pHtmlPane);
    m_pHtmlPane->SetPage(strHTML);

    wxLogTrace(wxT("Function Start/End"), wxT("CViewNotifications::OnNewsBOINC - Function End"));
}


void CViewNotifications::OnNewsBOINCWebsite( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CViewNotifications::OnNewsBOINCWebsite - Function Begin"));

	wxASSERT(m_pHtmlPane);
    m_pHtmlPane->LoadPage(wxT("http://boinc.berkeley.edu/"));

    wxLogTrace(wxT("Function Start/End"), wxT("CViewNotifications::OnNewsBOINCWebsite - Function End"));
}


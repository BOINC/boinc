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
// Revision 1.1  2004/09/21 01:26:24  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCBaseView.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "res/boinc.xpm"


IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseView, wxPanel)


CBOINCBaseView::CBOINCBaseView()
{
    wxLogTrace("CBOINCBaseView::CBOINCBaseView - Function Begining");
    wxLogTrace("CBOINCBaseView::CBOINCBaseView - Function Ending");
}


CBOINCBaseView::CBOINCBaseView(wxNotebook* pNotebook, wxWindowID iHtmlWindowID, wxWindowID iListWindowID) :
    wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
    wxLogTrace("CBOINCBaseView::CBOINCBaseView - Function Begining");
    wxASSERT(NULL != pNotebook);

    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(NULL != itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);
    
    SetSizer(itemFlexGridSizer);
    SetAutoLayout(TRUE);

    m_pTaskPane = new CBOINCTaskCtrl(this, iHtmlWindowID);
    wxASSERT(NULL != m_pTaskPane);

    m_pListPane = new CBOINCListCtrl(this, iListWindowID);
    wxASSERT(NULL != m_pListPane);

    itemFlexGridSizer->Add(m_pTaskPane, 0, wxGROW|wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pListPane, 0, wxGROW|wxGROW|wxALL, 1);
    
    wxLogTrace("CBOINCBaseView::CBOINCBaseView - Function Ending");
}


CBOINCBaseView::~CBOINCBaseView()
{
    wxLogTrace("CBOINCBaseView::~CBOINCBaseView - Function Begining");
    wxLogTrace("CBOINCBaseView::~CBOINCBaseView - Function Ending");
}


// The user friendly name of the view.
//   If it has not been defined by the view "Undefined" is returned.
//
wxString CBOINCBaseView::GetViewName()
{
    wxLogTrace("CBOINCBaseView::GetViewName - Function Begining");
    wxLogTrace("CBOINCBaseView::GetViewName - Function Ending");
    return wxString(_T("Undefined"));
}


// The user friendly icon of the view.
//   If it has not been defined by the view the BOINC icon is returned.
//
char** CBOINCBaseView::GetViewIcon()
{
    wxLogTrace("CBOINCBaseView::GetViewIcon - Function Begining");

    wxASSERT(NULL != boinc_xpm);

    wxLogTrace("CBOINCBaseView::GetViewIcon - Function Ending");
    return boinc_xpm;
}


void CBOINCBaseView::UpdateTaskPane()
{
    wxLogTrace("CBOINCBaseView::UpdateTaskPane - Function Begining");
    wxLogTrace("CBOINCBaseView::UpdateTaskPane - Function Ending");
}


void CBOINCBaseView::OnRender (wxTimerEvent& event)
{
    wxLogTrace("CBOINCBaseView::OnRender - Function Begining");
    wxLogTrace("CBOINCBaseView::OnRender - Function Ending");
}


bool CBOINCBaseView::OnSaveState( wxConfigBase* pConfig )
{
    wxLogTrace("CBOINCBaseView::OnSaveState - Function Begining");

    bool bReturnValue = true;

    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if (!m_pTaskPane->OnSaveState( pConfig ))
        bReturnValue = false;

    if (!m_pListPane->OnSaveState( pConfig ))
        bReturnValue = false;

    wxLogTrace("CBOINCBaseView::OnSaveState - Function Ending");
    return bReturnValue;
}


bool CBOINCBaseView::OnRestoreState( wxConfigBase* pConfig ) {
    wxLogTrace("CBOINCBaseView::OnRestoreState - Function Begining");

    bool bReturnValue = true;

    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if (!m_pTaskPane->OnRestoreState( pConfig ))
        bReturnValue = false;

    if (!m_pListPane->OnRestoreState( pConfig ))
        bReturnValue = false;

    wxLogTrace("CBOINCBaseView::OnRestoreState - Function Ending");
    return bReturnValue;
}


void CBOINCBaseView::OnCacheHint ( wxListEvent& event )
{
    m_iCacheFrom = event.GetCacheFrom();
    m_iCacheTo = event.GetCacheTo();
}


wxString CBOINCBaseView::OnGetItemText(long item, long column) const
{
    return wxString("Undefined");
}


int CBOINCBaseView::OnGetItemImage(long item) const
{
    return -1;
}


wxListItemAttr* CBOINCBaseView::OnGetItemAttr(long item) const
{
    return NULL;
}


void CBOINCBaseView::OnLinkClicked( const wxHtmlLinkInfo& link )
{
    wxLogTrace("CBOINCBaseView::OnLinkClicked - Function Begining");
    wxLogTrace("CBOINCBaseView::OnLinkClicked - Function Ending");
}


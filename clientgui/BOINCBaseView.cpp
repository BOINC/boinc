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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCBaseView.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCBaseView.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"

#include "res/boinc.xpm"


const wxString LINK_DEFAULT             = wxT("default");


IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseView, wxPanel)


CBOINCBaseView::CBOINCBaseView()
{
}


CBOINCBaseView::CBOINCBaseView(wxNotebook* pNotebook, wxWindowID iHtmlWindowID, wxWindowID iListWindowID) :
    wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
    wxASSERT(NULL != pNotebook);

    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    m_iCacheFrom = 0;
    m_iCacheTo = 0;

    m_iCount = 0;

    m_bItemSelected = false;

    m_strQuickTip = wxEmptyString;
    m_strQuickTipText = wxEmptyString;

    m_pTaskPane = NULL;
    m_pListPane = NULL;

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
}


CBOINCBaseView::~CBOINCBaseView()
{
}


// The user friendly name of the view.
//   If it has not been defined by the view "Undefined" is returned.
//
wxString CBOINCBaseView::GetViewName()
{
    return wxString(_T("Undefined"));
}


// The user friendly icon of the view.
//   If it has not been defined by the view the BOINC icon is returned.
//
char** CBOINCBaseView::GetViewIcon()
{
    wxASSERT(NULL != boinc_xpm);
    return boinc_xpm;
}


wxInt32 CBOINCBaseView::_GetListRowCount()
{
    return GetListRowCount();
}


wxInt32 CBOINCBaseView::GetListRowCount()
{
    return 0;
}


void CBOINCBaseView::FireOnTaskRender (wxTimerEvent& event)
{
    OnTaskRender( event );
}


void CBOINCBaseView::FireOnListRender (wxTimerEvent& event)
{
    OnListRender( event );
}


void CBOINCBaseView::OnTaskRender (wxTimerEvent& event)
{
    if (!m_bProcessingTaskRenderEvent)
    {
        m_bProcessingTaskRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        if ( ( 0 == m_pListPane->GetSelectedItemCount() ) && m_bItemSelected )
        {
            UpdateSelection();
        }

        m_bProcessingTaskRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


void CBOINCBaseView::OnListRender (wxTimerEvent& event)
{
    if (!m_bProcessingListRenderEvent)
    {
        m_bProcessingListRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        wxInt32 iCount = _GetListRowCount();
        if ( iCount != m_iCount )
        {
            m_iCount = iCount;
            if ( 0 >= iCount )
                m_pListPane->DeleteAllItems();
            else
                m_pListPane->SetItemCount(iCount);
        }
        else
        {
            if ( 1 <= m_iCacheTo )
            {
                wxInt32         iRowIndex        = 0;
                wxInt32         iColumnIndex     = 0;
                wxInt32         iColumnTotal     = 0;
                wxString        strDocumentText  = wxEmptyString;
                wxString        strListPaneText  = wxEmptyString;
                bool            bNeedRefreshData = false;
                wxListItem      liItem;

                liItem.SetMask(wxLIST_MASK_TEXT);
                iColumnTotal = m_pListPane->GetColumnCount();

                for ( iRowIndex = m_iCacheFrom; iRowIndex <= m_iCacheTo; iRowIndex++ )
                {
                    bNeedRefreshData = false;
                    liItem.SetId(iRowIndex);

                    for ( iColumnIndex = 0; iColumnIndex < iColumnTotal; iColumnIndex++ )
                    {
                        strDocumentText.Empty();
                        strListPaneText.Empty();

                        strDocumentText = OnListGetItemText( iRowIndex, iColumnIndex );

                        liItem.SetColumn(iColumnIndex);
                        m_pListPane->GetItem(liItem);
                        strListPaneText = liItem.GetText();

                        if ( !strDocumentText.IsSameAs(strListPaneText) )
                            bNeedRefreshData = true;
                    }

                    if ( bNeedRefreshData )
                    {
                        m_pListPane->RefreshItem( iRowIndex );
                    }
                }
            }
        }

        m_bProcessingListRenderEvent = false;
    }

    m_pListPane->Refresh();

    event.Skip();
}


bool CBOINCBaseView::FireOnSaveState( wxConfigBase* pConfig )
{
    return OnSaveState( pConfig );
}


bool CBOINCBaseView::FireOnRestoreState( wxConfigBase* pConfig )
{
    return OnRestoreState( pConfig );
}


bool CBOINCBaseView::OnSaveState( wxConfigBase* pConfig )
{
    bool bReturnValue = true;

    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if (!m_pTaskPane->OnSaveState( pConfig ))
        bReturnValue = false;

    if (!m_pListPane->OnSaveState( pConfig ))
        bReturnValue = false;

    return bReturnValue;
}


bool CBOINCBaseView::OnRestoreState( wxConfigBase* pConfig )
{
    bool bReturnValue = true;

    wxASSERT(NULL != pConfig);
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if (!m_pTaskPane->OnRestoreState( pConfig ))
        bReturnValue = false;

    if (!m_pListPane->OnRestoreState( pConfig ))
        bReturnValue = false;

    return bReturnValue;
}


void CBOINCBaseView::FireOnListCacheHint( wxListEvent& event )
{
    OnListCacheHint( event );
}


void CBOINCBaseView::FireOnListSelected( wxListEvent& event )
{
    OnListSelected( event );
}


void CBOINCBaseView::FireOnListDeselected( wxListEvent& event )
{
    OnListDeselected( event );
}


wxString CBOINCBaseView::FireOnListGetItemText(long item, long column) const
{
    return OnListGetItemText( item, column );
}


int CBOINCBaseView::FireOnListGetItemImage(long item) const
{
    return OnListGetItemImage( item );
}


wxListItemAttr* CBOINCBaseView::FireOnListGetItemAttr(long item) const
{
    return OnListGetItemAttr( item );
}


void CBOINCBaseView::OnListCacheHint( wxListEvent& event )
{
    m_iCacheFrom = event.GetCacheFrom();
    m_iCacheTo = event.GetCacheTo();
}


void CBOINCBaseView::OnListSelected( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


void CBOINCBaseView::OnListDeselected( wxListEvent& event )
{
    SetCurrentQuickTip(
        LINK_DEFAULT, 
        wxT("")
    );

    UpdateSelection();
    event.Skip();
}


wxString CBOINCBaseView::OnListGetItemText(long item, long column) const
{
    return wxString("Undefined");
}


int CBOINCBaseView::OnListGetItemImage(long item) const
{
    return -1;
}


wxListItemAttr* CBOINCBaseView::OnListGetItemAttr(long item) const
{
    return NULL;
}


void CBOINCBaseView::FireOnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    OnTaskLinkClicked( link );
}


void CBOINCBaseView::FireOnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    OnTaskCellMouseHover( cell, x, y );
}


void CBOINCBaseView::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
}


void CBOINCBaseView::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
}


wxString CBOINCBaseView::GetCurrentQuickTip()
{
    return m_strQuickTip;
}


wxString CBOINCBaseView::GetCurrentQuickTipText()
{
    return m_strQuickTipText;
}


void CBOINCBaseView::SetCurrentQuickTip( const wxString& strQuickTip, const wxString& strQuickTipText )
{
    m_strQuickTip = strQuickTip;
    m_strQuickTipText = strQuickTipText;
}


bool CBOINCBaseView::UpdateQuickTip( const wxString& strCurrentLink, const wxString& strQuickTip, const wxString& strQuickTipText )
{
    bool bRetVal;

    bRetVal = false;

    if ( (strCurrentLink == strQuickTip) && (strQuickTip != GetCurrentQuickTip()) )
    {
        SetCurrentQuickTip( strQuickTip, strQuickTipText );
        bRetVal = true;
    }

    return bRetVal;
}


void CBOINCBaseView::UpdateSelection()
{
}


void CBOINCBaseView::UpdateTaskPane()
{
}


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

#include "res/boinc.xpm"


IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseView, wxPanel)


CBOINCBaseView::CBOINCBaseView()
{
}


CBOINCBaseView::CBOINCBaseView( wxNotebook* pNotebook, wxWindowID iHtmlWindowID, wxInt32 iHtmlWindowFlags, wxWindowID iListWindowID, wxInt32 iListWindowFlags ) :
    wxPanel( pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    wxASSERT(NULL != pNotebook);

    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    m_bItemSelected = false;

    m_strQuickTip = wxEmptyString;
    m_strQuickTipText = wxEmptyString;

    //
    // Globalization/Localization
    //
    LINK_DEFAULT             = wxT("default");

    //
    // Setup View
    //
    m_pTaskPane = NULL;
    m_pListPane = NULL;

    SetAutoLayout(TRUE);

    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(NULL != itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);
    
    m_pTaskPane = new CBOINCTaskCtrl( this, iHtmlWindowID, iHtmlWindowFlags );
    wxASSERT(NULL != m_pTaskPane);

    m_pListPane = new CBOINCListCtrl( this, iListWindowID, iListWindowFlags );
    wxASSERT(NULL != m_pListPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pListPane, 1, wxGROW|wxALL, 1);

    SetSizerAndFit(itemFlexGridSizer);
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


wxInt32 CBOINCBaseView::GetListRowCount()
{
    wxASSERT(NULL != m_pListPane);
    return m_pListPane->GetItemCount();
}


void CBOINCBaseView::FireOnListRender ( wxTimerEvent& event )
{
    OnListRender( event );
}


bool CBOINCBaseView::FireOnSaveState( wxConfigBase* pConfig )
{
    return OnSaveState( pConfig );
}


bool CBOINCBaseView::FireOnRestoreState( wxConfigBase* pConfig )
{
    return OnRestoreState( pConfig );
}


void CBOINCBaseView::FireOnChar( wxKeyEvent& event )
{
    OnChar( event );
}


void CBOINCBaseView::FireOnListSelected( wxListEvent& event )
{
    OnListSelected( event );
}


void CBOINCBaseView::FireOnListDeselected( wxListEvent& event )
{
    OnListDeselected( event );
}


wxString CBOINCBaseView::FireOnListGetItemText( long item, long column ) const
{
    return OnListGetItemText( item, column );
}


int CBOINCBaseView::FireOnListGetItemImage( long item ) const
{
    return OnListGetItemImage( item );
}


wxListItemAttr* CBOINCBaseView::FireOnListGetItemAttr( long item ) const
{
    return OnListGetItemAttr( item );
}


void CBOINCBaseView::FireOnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    OnTaskLinkClicked( link );
}


void CBOINCBaseView::FireOnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    OnTaskCellMouseHover( cell, x, y );
}


wxInt32 CBOINCBaseView::GetDocCount()
{
    return 0;
}


void CBOINCBaseView::OnListRender ( wxTimerEvent& event )
{
    if (!m_bProcessingListRenderEvent)
    {
        m_bProcessingListRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        wxInt32 iDocCount = GetDocCount();
        wxInt32 iCacheCount = GetCacheCount();
        if ( iDocCount != iCacheCount )
        {
            if ( 0 >= iDocCount )
            {
                EmptyCache();
                m_pListPane->DeleteAllItems();
            }
            else
            {
                wxInt32 iIndex = 0;
                wxInt32 iReturnValue = -1;
                if ( iDocCount > iCacheCount )
                {
                    for ( iIndex = 0; iIndex < ( iDocCount - iCacheCount ); iIndex++ )
                    {
                        iReturnValue = AddCacheElement();
                        wxASSERT( 0 == iReturnValue );
                    }
                }
                else
                {
                    for ( iIndex = 0; iIndex < ( iCacheCount - iDocCount ); iIndex++ )
                    {
                        iReturnValue = RemoveCacheElement();
                        wxASSERT( 0 == iReturnValue );
                    }
                }

                m_pListPane->SetItemCount( iDocCount );
            }
        }

        if ( iCacheCount )
            SyncronizeCache();

        if ( EnsureLastItemVisible() )
            m_pListPane->EnsureVisible( iDocCount );

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
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


void CBOINCBaseView::OnChar( wxKeyEvent& event )
{
    event.Skip();
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


wxString CBOINCBaseView::OnListGetItemText( long WXUNUSED(item), long WXUNUSED(column) ) const
{
    return wxString("Undefined");
}


int CBOINCBaseView::OnListGetItemImage( long WXUNUSED(item) ) const
{
    return -1;
}


wxListItemAttr* CBOINCBaseView::OnListGetItemAttr( long WXUNUSED(item) ) const
{
    return NULL;
}


wxString CBOINCBaseView::OnDocGetItemText( long WXUNUSED(item), long WXUNUSED(column) ) const
{
    return wxString("Undefined");
}


wxString CBOINCBaseView::OnDocGetItemImage( long WXUNUSED(item) ) const
{
    return wxString("Undefined");
}


wxString CBOINCBaseView::OnDocGetItemAttr( long WXUNUSED(item) ) const
{
    return wxString("Undefined");
}


void CBOINCBaseView::OnTaskLinkClicked( const wxHtmlLinkInfo& WXUNUSED(link) )
{
}


void CBOINCBaseView::OnTaskCellMouseHover( wxHtmlCell* WXUNUSED(cell), wxCoord WXUNUSED(x) , wxCoord WXUNUSED(y) )
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


wxInt32 CBOINCBaseView::AddCacheElement()
{
    return -1;
}

    
wxInt32 CBOINCBaseView::EmptyCache()
{
    return -1;
}


wxInt32 CBOINCBaseView::GetCacheCount()
{
    return -1;
}


wxInt32 CBOINCBaseView::RemoveCacheElement()
{
    return -1;
}


wxInt32 CBOINCBaseView::SyncronizeCache()
{
    wxInt32         iRowIndex        = 0;
    wxInt32         iRowTotal        = 0;
    wxInt32         iColumnIndex     = 0;
    wxInt32         iColumnTotal     = 0;
    wxString        strDocumentText  = wxEmptyString;
    wxString        strListPaneText  = wxEmptyString;
    bool            bNeedRefreshData = false;

    iRowTotal = GetDocCount();
    iColumnTotal = m_pListPane->GetColumnCount();

    for ( iRowIndex = 0; iRowIndex < iRowTotal; iRowIndex++ )
    {
        bNeedRefreshData = false;

        for ( iColumnIndex = 0; iColumnIndex < iColumnTotal; iColumnIndex++ )
        {
            strDocumentText.Empty();
            strListPaneText.Empty();

            strDocumentText = OnDocGetItemText( iRowIndex, iColumnIndex );
            strListPaneText = OnListGetItemText( iRowIndex, iColumnIndex );

            if ( !strDocumentText.IsSameAs(strListPaneText) )
            {
                if ( 0 != UpdateCache( iRowIndex, iColumnIndex, strDocumentText ) )
                {
                    wxASSERT( FALSE );
                }
                bNeedRefreshData = true;
            }
        }

        if ( bNeedRefreshData )
        {
            m_pListPane->RefreshItem( iRowIndex );
        }
    }

    return 0;
}


wxInt32 CBOINCBaseView::UpdateCache( long WXUNUSED(item), long WXUNUSED(column), wxString& WXUNUSED(strNewData) )
{
    return -1;
}


bool CBOINCBaseView::EnsureLastItemVisible()
{
    return false;
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


const char *BOINC_RCSID_0a1bd38a5a = "$Id$";

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
#pragma implementation "BOINCListCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCListCtrl.h"
#include "ViewProjects.h"
#include "ViewWork.h"
#include "ViewTransfers.h"
#include "ViewMessages.h"
#include "ViewResources.h"


IMPLEMENT_DYNAMIC_CLASS(CBOINCListCtrl, wxListView)


CBOINCListCtrl::CBOINCListCtrl()
{
    wxLogTrace("CBOINCListCtrl::CBOINCListCtrl - Function Begining");
    wxLogTrace("CBOINCListCtrl::CBOINCListCtrl - Function Ending");
}


CBOINCListCtrl::CBOINCListCtrl( CBOINCBaseView* pView, wxWindowID iListWindowID ) :
    wxListView( pView, iListWindowID, wxDefaultPosition, wxSize(-1, -1), wxLC_REPORT | wxLC_VIRTUAL, wxDefaultValidator )
{
    wxLogTrace("CBOINCListCtrl::CBOINCListCtrl - Function Begining");

    m_pParentView = pView;

    wxLogTrace("CBOINCListCtrl::CBOINCListCtrl - Function Ending");
}


CBOINCListCtrl::~CBOINCListCtrl()
{
    wxLogTrace("CBOINCListCtrl::~CBOINCListCtrl - Function Begining");
    wxLogTrace("CBOINCListCtrl::~CBOINCListCtrl - Function Ending");
}


void CBOINCListCtrl::OnRender ( wxTimerEvent& event ) {
    wxLogTrace("CBOINCListCtrl::OnRender - Function Begining");
    wxLogTrace("CBOINCListCtrl::OnRender - Function Ending");
}


bool CBOINCListCtrl::OnSaveState( wxConfigBase* pConfig ) {
    wxLogTrace("CBOINCListCtrl::OnSaveState - Function Begining");

    wxString    strBaseConfigLocation = wxString(_T(""));
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;


    wxASSERT(NULL != pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + "/";

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask( wxLIST_MASK_TEXT |
                          wxLIST_MASK_WIDTH |
                          wxLIST_MASK_FORMAT );

    // Cycle through the columns recording anything interesting
    for ( iIndex = 0; iIndex <= iColumnCount; iIndex++ ) {

        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

        pConfig->Write(_T("Width"), liColumnInfo.GetWidth());
        pConfig->Write(_T("Format"), liColumnInfo.GetAlign());

    }


    wxLogTrace("CBOINCListCtrl::OnSaveState - Function Ending");
    return true;
}


bool CBOINCListCtrl::OnRestoreState( wxConfigBase* pConfig ) {
    wxLogTrace("CBOINCListCtrl::OnRestoreState - Function Begining");

    wxString    strBaseConfigLocation = wxString(_T(""));
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;
    wxInt32     iTempValue = 0;


    wxASSERT(NULL != pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + "/";

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask( wxLIST_MASK_TEXT |
                          wxLIST_MASK_WIDTH |
                          wxLIST_MASK_FORMAT );

    // Cycle through the columns recording anything interesting
    for ( iIndex = 0; iIndex <= iColumnCount; iIndex++ ) {

        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

        pConfig->Read(_T("Width"), &iTempValue, 80);
        liColumnInfo.SetWidth(iTempValue);

        pConfig->Read(_T("Format"), &iTempValue, 0);
        liColumnInfo.SetAlign((wxListColumnFormat)iTempValue);

        SetColumn(iIndex, liColumnInfo);

    }


    wxLogTrace("CBOINCListCtrl::OnRestoreState - Function Ending");
    return true;
}


wxString CBOINCListCtrl::OnGetItemText( long item, long column ) const
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    if        (wxDynamicCast(m_pParentView, CViewProjects)) {
        return FireOnGetItemTextEvent(wxDynamicCast(m_pParentView, CViewProjects), item, column);
    } else if (wxDynamicCast(m_pParentView, CViewWork)) {
        return FireOnGetItemTextEvent(wxDynamicCast(m_pParentView, CViewWork), item, column);
    } else if (wxDynamicCast(m_pParentView, CViewTransfers)) {
        return FireOnGetItemTextEvent(wxDynamicCast(m_pParentView, CViewTransfers), item, column);
    } else if (wxDynamicCast(m_pParentView, CViewMessages)) {
        return FireOnGetItemTextEvent(wxDynamicCast(m_pParentView, CViewMessages), item, column);
    } else if (wxDynamicCast(m_pParentView, CViewResources)) {
        return FireOnGetItemTextEvent(wxDynamicCast(m_pParentView, CViewResources), item, column);
    }
    return FireOnGetItemTextEvent(wxDynamicCast(m_pParentView, CBOINCBaseView), item, column);
}


template < class T >
wxString CBOINCListCtrl::FireOnGetItemTextEvent( T pView, long item, long column) const
{
    return pView->OnGetItemText( item, column );
}


int CBOINCListCtrl::OnGetItemImage( long item ) const
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    if        (wxDynamicCast(m_pParentView, CViewProjects)) {
        return FireOnGetItemImageEvent(wxDynamicCast(m_pParentView, CViewProjects), item);
    } else if (wxDynamicCast(m_pParentView, CViewWork)) {
        return FireOnGetItemImageEvent(wxDynamicCast(m_pParentView, CViewWork), item);
    } else if (wxDynamicCast(m_pParentView, CViewTransfers)) {
        return FireOnGetItemImageEvent(wxDynamicCast(m_pParentView, CViewTransfers), item);
    } else if (wxDynamicCast(m_pParentView, CViewMessages)) {
        return FireOnGetItemImageEvent(wxDynamicCast(m_pParentView, CViewMessages), item);
    } else if (wxDynamicCast(m_pParentView, CViewResources)) {
        return FireOnGetItemImageEvent(wxDynamicCast(m_pParentView, CViewResources), item);
    }
    return FireOnGetItemImageEvent(wxDynamicCast(m_pParentView, CBOINCBaseView), item);
}


template < class T >
int CBOINCListCtrl::FireOnGetItemImageEvent( T pView, long item ) const
{
    return pView->OnGetItemImage( item );
}


wxListItemAttr* CBOINCListCtrl::OnGetItemAttr( long item ) const
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    if        (wxDynamicCast(m_pParentView, CViewProjects)) {
        return FireOnGetItemAttrEvent(wxDynamicCast(m_pParentView, CViewProjects), item);
    } else if (wxDynamicCast(m_pParentView, CViewWork)) {
        return FireOnGetItemAttrEvent(wxDynamicCast(m_pParentView, CViewWork), item);
    } else if (wxDynamicCast(m_pParentView, CViewTransfers)) {
        return FireOnGetItemAttrEvent(wxDynamicCast(m_pParentView, CViewTransfers), item);
    } else if (wxDynamicCast(m_pParentView, CViewMessages)) {
        return FireOnGetItemAttrEvent(wxDynamicCast(m_pParentView, CViewMessages), item);
    } else if (wxDynamicCast(m_pParentView, CViewResources)) {
        return FireOnGetItemAttrEvent(wxDynamicCast(m_pParentView, CViewResources), item);
    }
    return FireOnGetItemAttrEvent(wxDynamicCast(m_pParentView, CBOINCBaseView), item);
}


template < class T >
wxListItemAttr* CBOINCListCtrl::FireOnGetItemAttrEvent( T pView, long item ) const
{
    return pView->OnGetItemAttr( item );
}


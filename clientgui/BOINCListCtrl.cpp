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
#pragma implementation "BOINCListCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCListCtrl.h"
#include "Events.h"


IMPLEMENT_DYNAMIC_CLASS(CBOINCListCtrl, wxListView)


CBOINCListCtrl::CBOINCListCtrl()
{
}


CBOINCListCtrl::CBOINCListCtrl( CBOINCBaseView* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags ) :
    wxListView( pView, iListWindowID, wxDefaultPosition, wxSize(-1, -1), iListWindowFlags )
{
    m_pParentView = pView;
}


CBOINCListCtrl::~CBOINCListCtrl()
{
}


void CBOINCListCtrl::OnRender ( wxTimerEvent& event )
{
}


bool CBOINCListCtrl::OnSaveState( wxConfigBase* pConfig )
{
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;


    wxASSERT(NULL != pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask( wxLIST_MASK_TEXT |
                          wxLIST_MASK_WIDTH |
                          wxLIST_MASK_FORMAT );

    // Cycle through the columns recording anything interesting
    for ( iIndex = 0; iIndex <= iColumnCount; iIndex++ )
    {
        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

        pConfig->Write(wxT("Width"), liColumnInfo.GetWidth());
    }


    return true;
}


bool CBOINCListCtrl::OnRestoreState( wxConfigBase* pConfig )
{
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;
    wxInt32     iTempValue = 0;


    wxASSERT(NULL != pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask( wxLIST_MASK_TEXT |
                          wxLIST_MASK_WIDTH |
                          wxLIST_MASK_FORMAT );

    // Cycle through the columns recording anything interesting
    for ( iIndex = 0; iIndex <= iColumnCount; iIndex++ )
    {
        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

        pConfig->Read(wxT("Width"), &iTempValue, 80);
        liColumnInfo.SetWidth(iTempValue);

        pConfig->Read(wxT("Format"), &iTempValue, 0);
        liColumnInfo.SetAlign((wxListColumnFormat)iTempValue);

        SetColumn(iIndex, liColumnInfo);
    }

    return true;
}


wxString CBOINCListCtrl::OnGetItemText( long item, long column ) const
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemText( item, column );
}


int CBOINCListCtrl::OnGetItemImage( long item ) const
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemImage( item );
}


wxListItemAttr* CBOINCListCtrl::OnGetItemAttr( long item ) const
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemAttr( item );
}


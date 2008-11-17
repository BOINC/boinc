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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCListCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCListCtrl.h"
#include "Events.h"


IMPLEMENT_DYNAMIC_CLASS(CBOINCListCtrl, wxListView)


CBOINCListCtrl::CBOINCListCtrl() {}


CBOINCListCtrl::CBOINCListCtrl(
    CBOINCBaseView* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags
) : wxListView(
    pView, iListWindowID, wxDefaultPosition, wxSize(-1, -1), iListWindowFlags
) {
    m_pParentView = pView;

    m_bIsSingleSelection = (iListWindowFlags & wxLC_SINGLE_SEL) ? true : false ;

    Connect(
        iListWindowID, 
        wxEVT_COMMAND_LEFT_CLICK, 
        (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &CBOINCListCtrl::OnClick
    );
}


CBOINCListCtrl::~CBOINCListCtrl()
{
}


bool CBOINCListCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;


    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask(
        wxLIST_MASK_TEXT |
        wxLIST_MASK_WIDTH |
        wxLIST_MASK_FORMAT
    );

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex <= iColumnCount; iIndex++) {
        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

        pConfig->Write(wxT("Width"), liColumnInfo.GetWidth());
        
#if (defined(__WXMAC__) &&  wxCHECK_VERSION(2,8,0))
        pConfig->Write(wxT("Width"), GetColumnWidth(iIndex)); // Work around bug in wxMac-2.8.0 wxListCtrl::SetColumn()
#endif
    }


    return true;
}


bool CBOINCListCtrl::OnRestoreState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;
    wxInt32     iTempValue = 0;


    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask(
        wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH | wxLIST_MASK_FORMAT
    );

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex <= iColumnCount; iIndex++) {
        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

        pConfig->Read(wxT("Width"), &iTempValue, -1);
        if (-1 != iTempValue) {
            liColumnInfo.SetWidth(iTempValue);
#if (defined(__WXMAC__) &&  wxCHECK_VERSION(2,8,0))
            SetColumnWidth(iIndex,iTempValue); // Work around bug in wxMac-2.8.0 wxListCtrl::SetColumn()
#endif
        }

        pConfig->Read(wxT("Format"), &iTempValue, -1);
        if (-1 != iTempValue) {
            liColumnInfo.SetAlign((wxListColumnFormat)iTempValue);
        }

        SetColumn(iIndex, liColumnInfo);
    }

    return true;
}


void CBOINCListCtrl::OnClick(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCListCtrl::OnClick - Function Begin"));

    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    wxListEvent leDeselectedEvent(wxEVT_COMMAND_LIST_ITEM_DESELECTED, m_windowId);
    leDeselectedEvent.SetEventObject(this);

    if (m_bIsSingleSelection) {
        if (GetFocusedItem() != GetFirstSelected()) {
            wxLogTrace(wxT("Function Status"), wxT("CBOINCListCtrl::OnClick - GetFocusedItem() '%d' != GetFirstSelected() '%d'"), GetFocusedItem(), GetFirstSelected());

            if (-1 == GetFirstSelected()) {
                wxLogTrace(wxT("Function Status"), wxT("CBOINCListCtrl::OnClick - Force Selected State"));

                long desiredstate = wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED;
                SetItemState(GetFocusedItem(), desiredstate, desiredstate);
            } else {
                m_pParentView->FireOnListDeselected(leDeselectedEvent);
            }
        }
    } else {
        if (-1 == GetFirstSelected()) {
            m_pParentView->FireOnListDeselected(leDeselectedEvent);
        }
    }

    event.Skip();
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCListCtrl::OnClick - Function End"));
}


wxString CBOINCListCtrl::OnGetItemText(long item, long column) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemText(item, column);
}


int CBOINCListCtrl::OnGetItemImage(long item) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemImage(item);
}


wxListItemAttr* CBOINCListCtrl::OnGetItemAttr(long item) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemAttr(item);
}


const char *BOINC_RCSID_5cf411daa0 = "$Id: BOINCListCtrl.cpp 13804 2007-10-09 11:35:47Z fthomas $";

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
#pragma implementation "sg_SGUIListControl.h"
#endif

#include "stdwx.h"
#include "sg_DlgMessages.h"
#include "sg_SGUIListControl.h"


IMPLEMENT_DYNAMIC_CLASS(CSGUIListCtrl, wxListView)


CSGUIListCtrl::CSGUIListCtrl() {}

CSGUIListCtrl::CSGUIListCtrl(
	CDlgMessages* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags
)
: wxListView(
    pView, iListWindowID,  wxPoint(15,15), wxSize(510, 370), iListWindowFlags
) 
{
    m_pParentView = pView;

    m_bIsSingleSelection = (iListWindowFlags & wxLC_SINGLE_SEL) ? true : false ;

    Connect(
        iListWindowID, 
        wxEVT_COMMAND_LEFT_CLICK, 
        (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &CSGUIListCtrl::OnClick
    );
}


CSGUIListCtrl::~CSGUIListCtrl()
{
}


bool CSGUIListCtrl::OnSaveState(wxConfigBase* pConfig) {
    

    return true;
}


bool CSGUIListCtrl::OnRestoreState(wxConfigBase* pConfig) {

    return true;
}


void CSGUIListCtrl::OnClick(wxCommandEvent& event) {

}


wxString CSGUIListCtrl::OnGetItemText(long item, long column) const {
    wxASSERT(m_pParentView);
	wxASSERT(wxDynamicCast(m_pParentView, CDlgMessages));

    return m_pParentView->OnListGetItemText(item, column);
}


int CSGUIListCtrl::OnGetItemImage(long item) const {

    return 1;
}


wxListItemAttr* CSGUIListCtrl::OnGetItemAttr(long item) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CDlgMessages));

    return m_pParentView->OnListGetItemAttr(item);
}
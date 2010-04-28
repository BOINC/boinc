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
#pragma implementation "DlgEventLogListCtrl.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "DlgEventLogListCtrl.h"
#include "DlgEventLog.h"


IMPLEMENT_DYNAMIC_CLASS(CDlgEventLogListCtrl, DLG_LISTCTRL_BASE)

BEGIN_EVENT_TABLE(CDlgEventLogListCtrl, DLG_LISTCTRL_BASE)
    EVT_LEFT_UP(CDlgEventLogListCtrl::OnMouseUp)
END_EVENT_TABLE()


CDlgEventLogListCtrl::CDlgEventLogListCtrl() {}

CDlgEventLogListCtrl::CDlgEventLogListCtrl(CDlgEventLog* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags)
    : DLG_LISTCTRL_BASE(pView, iListWindowID, wxDefaultPosition, wxDefaultSize, iListWindowFlags) 
{
    m_pParentView = pView;

    m_bIsSingleSelection = (iListWindowFlags & wxLC_SINGLE_SEL) ? true : false ;

#ifdef __WXMAC__
    SetupMacAccessibilitySupport();
#endif
}


#ifdef __WXMAC__
CDlgEventLogListCtrl::~CDlgEventLogListCtrl()
{
    RemoveMacAccessibilitySupport();
}
#endif


wxString CDlgEventLogListCtrl::OnGetItemText(long item, long column) const {
    wxASSERT(m_pParentView);
	wxASSERT(wxDynamicCast(m_pParentView, CDlgEventLog));

    return m_pParentView->OnListGetItemText(item, column);
}


int CDlgEventLogListCtrl::OnGetItemImage(long /* item */) const {
    return 1;
}


wxListItemAttr* CDlgEventLogListCtrl::OnGetItemAttr(long item) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CDlgEventLog));

    return m_pParentView->OnListGetItemAttr(item);
}


wxColour CDlgEventLogListCtrl::GetBackgroundColour() {
    return *wxWHITE;
}


void CDlgEventLogListCtrl::OnMouseUp(wxMouseEvent& event) {
    m_pParentView->UpdateButtons();
    event.Skip();
}

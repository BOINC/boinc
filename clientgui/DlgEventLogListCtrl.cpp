// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

#ifdef __WXGTK__
IMPLEMENT_DYNAMIC_CLASS(MyEvtLogEvtHandler, wxEvtHandler)

BEGIN_EVENT_TABLE(MyEvtLogEvtHandler, wxEvtHandler)
    EVT_PAINT(MyEvtLogEvtHandler::OnPaint)
END_EVENT_TABLE()

MyEvtLogEvtHandler::MyEvtLogEvtHandler() {
    m_view_startX = 0;
}

MyEvtLogEvtHandler::MyEvtLogEvtHandler(wxGenericListCtrl *theListControl) {
    m_listCtrl = theListControl;
    m_view_startX = 0;
}

void MyEvtLogEvtHandler::OnPaint(wxPaintEvent & event)
{
    if (m_listCtrl) {
        // Work around a wxWidgets 3.0 bug in wxGenericListCtrl (Linux
        // only) which causes headers to be misaligned after horizontal
        // scrolling due to wxListHeaderWindow::OnPaint() calling
        // parent->GetViewStart() before the parent window has been
        // scrolled to the new position.
        int view_startX;
        ((CDlgEventLogListCtrl*)m_listCtrl)->savedHandler->ProcessEvent(event);
        m_listCtrl->GetViewStart( &view_startX, NULL );
        if (view_startX != m_view_startX) {
            m_view_startX = view_startX;
            ((wxWindow *)m_listCtrl->m_headerWin)->Refresh();
            ((wxWindow *)m_listCtrl->m_headerWin)->Update();
        }
    } else {
        event.Skip();
   }
}
#endif


IMPLEMENT_DYNAMIC_CLASS(CDlgEventLogListCtrl, DLG_LISTCTRL_BASE)

BEGIN_EVENT_TABLE(CDlgEventLogListCtrl, DLG_LISTCTRL_BASE)
    EVT_LEFT_UP(CDlgEventLogListCtrl::OnMouseUp)
#ifdef __WXMAC__
	EVT_SIZE(CDlgEventLogListCtrl::OnSize)
#endif

END_EVENT_TABLE()


CDlgEventLogListCtrl::CDlgEventLogListCtrl() {}

CDlgEventLogListCtrl::CDlgEventLogListCtrl(CDlgEventLog* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags)
    : DLG_LISTCTRL_BASE(pView, iListWindowID, wxDefaultPosition, wxDefaultSize, iListWindowFlags)
{
    m_pParentView = pView;

    m_bIsSingleSelection = (iListWindowFlags & wxLC_SINGLE_SEL) ? true : false ;

#ifdef __WXGTK__
    savedHandler = GetMainWin()->GetEventHandler();
    GetMainWin()->PushEventHandler(new MyEvtLogEvtHandler(this));
#endif

#ifdef __WXMAC__
    m_fauxHeaderView = NULL;
    m_fauxBodyView = NULL;
    SetupMacAccessibilitySupport();
#endif
}


CDlgEventLogListCtrl::~CDlgEventLogListCtrl()
{
#ifdef __WXGTK__
    GetMainWin()->PopEventHandler(true);
#endif

#ifdef __WXMAC__
    RemoveMacAccessibilitySupport();
#endif
}


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


void CDlgEventLogListCtrl::OnMouseUp(wxMouseEvent& event) {
    m_pParentView->UpdateButtons();
    event.Skip();
}


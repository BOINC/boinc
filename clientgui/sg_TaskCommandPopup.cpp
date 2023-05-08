// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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


#include "stdwx.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "DlgItemProperties.h"
#include "sg_TaskPanel.h"
#include "sg_TaskCommandPopup.h"

IMPLEMENT_DYNAMIC_CLASS(CSimpleTaskPopupButton, CTransparentButton)

BEGIN_EVENT_TABLE(CSimpleTaskPopupButton, CTransparentButton)
    EVT_LEFT_DOWN(CSimpleTaskPopupButton::OnTaskCommandsMouseDown)
    EVT_MENU(ID_TASK_WORK_SHOWGRAPHICS, CSimpleTaskPopupButton::OnTaskShowGraphics)
    EVT_MENU(ID_TASK_WORK_SUSPEND, CSimpleTaskPopupButton::OnTaskSuspendResume)
    EVT_MENU(ID_TASK_WORK_ABORT, CSimpleTaskPopupButton::OnTaskAbort)
    EVT_MENU(ID_TASK_SHOW_PROPERTIES, CSimpleTaskPopupButton::OnTaskShowProperties)
END_EVENT_TABLE()

CSimpleTaskPopupButton::CSimpleTaskPopupButton() {
}

CSimpleTaskPopupButton::CSimpleTaskPopupButton(wxWindow* parent, wxWindowID id,
        const wxString& label, const wxPoint& pos, const wxSize& size,
        long style, const wxValidator& validator, const wxString& name) :
        CTransparentButton(parent, id, label, pos, size, style, validator, name)
    {

    m_TaskSuspendedViaGUI = false;
    m_TaskCommandPopUpMenu = new wxMenu();
    AddMenuItems();
    Connect(
        id,
        wxEVT_BUTTON,
        (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &CSimpleTaskPopupButton::OnTaskCommandsKeyboardNav
    );

}


CSimpleTaskPopupButton::~CSimpleTaskPopupButton() {
    delete m_TaskCommandPopUpMenu;
}


void CSimpleTaskPopupButton::AddMenuItems() {
    m_ShowGraphicsMenuItem = m_TaskCommandPopUpMenu->Append(
        ID_TASK_WORK_SHOWGRAPHICS,
        _("Show graphics"),
        _("Show application graphics in a window.")
    );

    m_SuspendResumeMenuItem = m_TaskCommandPopUpMenu->Append(
        ID_TASK_WORK_SUSPEND,
        _("Suspend"),
        _("Suspend this task.")
    );

    m_AbortMenuItem = m_TaskCommandPopUpMenu->Append(
        ID_TASK_WORK_ABORT,
        _("Abort"),
        _("Abandon this task. You will get no credit for it.")
    );

    m_ShowPropertiesMenuItem = m_TaskCommandPopUpMenu->Append(
        ID_TASK_SHOW_PROPERTIES,
        _("Properties"),
        _("Show task details.")
    );
}


void CSimpleTaskPopupButton::OnTaskCommandsMouseDown(wxMouseEvent&) {
    ShowTaskCommandsMenu(ScreenToClient(wxGetMousePosition()));
}


void CSimpleTaskPopupButton::OnTaskCommandsKeyboardNav(wxCommandEvent&) {
    ShowTaskCommandsMenu(wxPoint(GetSize().GetWidth()/2, GetSize().GetHeight()/2));
}


void CSimpleTaskPopupButton::ShowTaskCommandsMenu(wxPoint pos) {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
    bool                enableShowGraphics = true;
    bool                enableAbort = true;
    CC_STATUS           status;
    wxString            strMachineName;

    wxASSERT(pDoc);

    TaskSelectionData* selData = ((CSimpleTaskPanel*)GetParent())->GetTaskSelectionData();
    if (selData == NULL) return;

    RESULT* result = lookup_result(selData->project_url, selData->result_name);

    if (!result) return;

    if (result->suspended_via_gui) {
        m_TaskSuspendedViaGUI = true;
        m_SuspendResumeMenuItem->SetItemLabel(_("Resume"));
        m_SuspendResumeMenuItem->SetHelp(_("Resume work for this task."));
    } else {
        m_TaskSuspendedViaGUI = false;
        m_SuspendResumeMenuItem->SetItemLabel(_("Suspend"));
        m_SuspendResumeMenuItem->SetHelp(_("Suspend work for this task."));
    }

    pDoc->GetCoreClientStatus(status);
    if (status.task_suspend_reason & ~(SUSPEND_REASON_CPU_THROTTLE)) {
        enableShowGraphics = false;
    }

    pDoc->GetConnectedComputerName(strMachineName);
    if (!pDoc->IsComputerNameLocal(strMachineName)) {
        enableShowGraphics = false;
    }

    // Disable Show Graphics button if selected task can't display graphics
    if (!strlen(result->web_graphics_url) && !strlen(result->graphics_exec_path)) {
        enableShowGraphics = false;
    }

    if (result->suspended_via_gui ||
        result->project_suspended_via_gui ||
        (result->scheduler_state != CPU_SCHED_SCHEDULED)
    ) {
        enableShowGraphics = false;
    }

    if (pDoc->GetRunningGraphicsApp(result) != NULL) {
        m_ShowGraphicsMenuItem->SetItemLabel(_("Stop graphics"));
        m_ShowGraphicsMenuItem->SetHelp(_("Close application graphics window."));
        // Graphics might still be running even if task is suspended
        enableShowGraphics = true;

    } else {
        m_ShowGraphicsMenuItem->SetItemLabel(_("Show graphics"));
        m_ShowGraphicsMenuItem->SetHelp(_("Show application graphics in a window."));
    }

    m_ShowGraphicsMenuItem->Enable(enableShowGraphics);

    // Disable Abort button if any selected task already aborted
    if (
        result->active_task_state == PROCESS_ABORT_PENDING ||
        result->active_task_state == PROCESS_ABORTED ||
        result->state == RESULT_ABORTED
    ) {
        enableAbort = false;
    }

    m_AbortMenuItem->Enable(enableAbort);

#ifdef __WXMAC__
    // Disable tooltips on Mac while menus are popped up because they cover menus
    wxToolTip::Enable(false);
#endif

	PopupMenu(m_TaskCommandPopUpMenu, pos.x, pos.y);


#if TESTBIGICONPOPUP
/*** CAF *** FOR TESTING ONLY ***/
    static int i;
    wxString s;

    if (i > 9) i = 0;
    if ( i < 5) {
        s = (wxT("This is a very very very and extremely long label."));
    } else {
        s = (wxT("short."));
    }

    switch (i++) {
        case 0:
        case 5:
            UpdateStaticText(&m_TaskProjectName, s);
            break;
        case 1:
        case 6:
            UpdateStaticText(&m_TaskApplicationName, s);
            break;
        case 2:
        case 7:
            UpdateStaticText(&m_ElapsedTimeValue, s);
            break;
        case 3:
        case 8:
            UpdateStaticText(&m_TimeRemainingValue, s);
            break;
        case 4:
        case 9:
            UpdateStaticText(&m_StatusValueText, s);
            break;
    }

	m_ProgressBar->SetValue( i * 10 );
    int sel = i % 3;
//    m_TaskSelectionCtrl->SetStringSelection(tempArray[sel]);
    m_TaskSelectionCtrl->SetSelection(sel);
#endif
}


void CSimpleTaskPopupButton::OnTaskShowGraphics(wxCommandEvent& WXUNUSED(event)) {
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    TaskSelectionData* selData = ((CSimpleTaskPanel*)GetParent())->GetTaskSelectionData();
    if (selData == NULL) return;

    RESULT* result = lookup_result(selData->project_url, selData->result_name);
    if (result) {
        pDoc->WorkShowGraphics(result);
    }
}


void CSimpleTaskPopupButton::OnTaskSuspendResume(wxCommandEvent& WXUNUSED(event)) {
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    TaskSelectionData* selData = ((CSimpleTaskPanel*)GetParent())->GetTaskSelectionData();
    if (selData == NULL) return;

    if (m_TaskSuspendedViaGUI) {
        pDoc->WorkResume(selData->project_url, selData->result_name);
    } else {
        pDoc->WorkSuspend(selData->project_url, selData->result_name);
    }
}


void CSimpleTaskPopupButton::OnTaskAbort(wxCommandEvent& WXUNUSED(event)) {
    wxInt32  iAnswer        = 0;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())  // Probably no longer relevant
        return;

    TaskSelectionData* selData = ((CSimpleTaskPanel*)GetParent())->GetTaskSelectionData();
    if (selData == NULL) return;

    RESULT* result = lookup_result(selData->project_url, selData->result_name);
    if (result) {
#if SELECTBYRESULTNAME
        wxString name = wxString(selData->result_name, wxConvUTF8, strlen(selData->result_name));
#else
        wxString name = ((CSimpleTaskPanel*)GetParent())->GetSelectedTaskString();
#endif
        strMessage.Printf(
           _("Are you sure you want to abort this task '%s'?\n(Progress: %.1lf%%, Status: %s)"),
           name.c_str(), result->fraction_done * 100.0, result_description(result, false).c_str());

        iAnswer = wxGetApp().SafeMessageBox(
            strMessage,
            _("Abort task"),
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES != iAnswer) {
            return;
        }

        pDoc->WorkAbort(result->project_url, result->name);
    }
}


void CSimpleTaskPopupButton::OnTaskShowProperties(wxCommandEvent& WXUNUSED(event)) {
    TaskSelectionData* selData = ((CSimpleTaskPanel*)GetParent())->GetTaskSelectionData();
    if (selData == NULL) return;

    RESULT* result = lookup_result(selData->project_url, selData->result_name);
    if (result) {
        CDlgItemProperties dlg(this);
        dlg.renderInfos(result);
        dlg.ShowModal();
    }
}


// CMainDocument::state.lookup_result() does not yield current scheduler_state;
// we must use CMainDocument::result() for that.
RESULT* CSimpleTaskPopupButton::lookup_result(char* url, char* name) {
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->result(wxString(name, wxConvUTF8), wxString(url, wxConvUTF8));
}

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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
#pragma implementation "BOINCTaskCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCTaskCtrl.h"
#include "MainDocument.h"

const int taskPaneWidth = 200;
const int taskButtonWidth = taskPaneWidth - 55;

IMPLEMENT_DYNAMIC_CLASS(CBOINCTaskCtrl, wxScrolledWindow)

#ifdef __WXMSW__
BEGIN_EVENT_TABLE (CBOINCTaskCtrl, CBOINCBaseView)
    EVT_CHILD_FOCUS(CBOINCTaskCtrl::OnChildFocus)
END_EVENT_TABLE ()
#endif


CBOINCTaskCtrl::CBOINCTaskCtrl() {}


CBOINCTaskCtrl::CBOINCTaskCtrl(CBOINCBaseView* pView, wxWindowID iTaskWindowID, wxInt32 iTaskWindowFlags) :
    wxScrolledWindow(pView, iTaskWindowID, wxDefaultPosition, wxSize(taskPaneWidth, -1), iTaskWindowFlags)
{
    m_pParent = pView;
    m_pSizer = NULL;

    SetVirtualSize( taskPaneWidth, 1000 );
    EnableScrolling(false, true);
    SetScrollRate( 0, 10 );
}


CBOINCTaskCtrl::~CBOINCTaskCtrl() {}


wxInt32 CBOINCTaskCtrl::DeleteTaskGroupAndTasks( CTaskItemGroup* pGroup ) {
    unsigned int i;
    CTaskItem*   pItem = NULL;

    for (i=0; i < pGroup->m_Tasks.size(); i++) {
        pItem = pGroup->m_Tasks[i];
        DeleteTask(pGroup, pItem);
    }
    if (pGroup->m_pStaticBoxSizer) {
        m_pSizer->Detach(pGroup->m_pStaticBoxSizer);
        pGroup->m_pStaticBoxSizer->Detach(pGroup->m_pStaticBox);

        delete pGroup->m_pStaticBox;
        delete pGroup->m_pStaticBoxSizer;

        pGroup->m_pStaticBox = NULL;
        pGroup->m_pStaticBoxSizer = NULL;
    }

    return 0;
}


wxInt32 CBOINCTaskCtrl::DisableTaskGroupTasks( CTaskItemGroup* pGroup ) {
    unsigned int i;
    CTaskItem*   pItem = NULL;

    if (pGroup) {
        for (i=0; i < pGroup->m_Tasks.size(); i++) {
            pItem = pGroup->m_Tasks[i];
            DisableTask(pItem);
        }
    }

    return 0;
}


wxInt32 CBOINCTaskCtrl::EnableTaskGroupTasks( CTaskItemGroup* pGroup ) {
    unsigned int i;
    CTaskItem*   pItem = NULL;

    if (pGroup) {
        for (i=0; i < pGroup->m_Tasks.size(); i++) {
            pItem = pGroup->m_Tasks[i];
            EnableTask(pItem);
        }
    }

    return 0;
}


wxInt32 CBOINCTaskCtrl::DeleteTask( CTaskItemGroup* pGroup, CTaskItem* pItem ) {
    if (pItem->m_pButton) {
        pGroup->m_pStaticBoxSizer->Detach(pItem->m_pButton);
        delete pItem->m_pButton;
        pItem->m_pButton = NULL;
    }
    return 0;
}


wxInt32 CBOINCTaskCtrl::DisableTask( CTaskItem* pItem ) {
    if (pItem->m_pButton) {
        pItem->m_pButton->Disable();
    }
    return 0;
}


wxInt32 CBOINCTaskCtrl::EnableTask( CTaskItem* pItem ) {
    if (pItem->m_pButton) {
        pItem->m_pButton->Enable();
    }
    return 0;
}


wxInt32 CBOINCTaskCtrl::UpdateTask( CTaskItem* pItem, wxString strName, wxString strDescription ) {
    if (pItem->m_pButton) {
        if (!pItem->m_strName.Cmp(strName) &&
            !pItem->m_strDescription.Cmp(strDescription)) {
            return 0;
        }
        pItem->m_strName = strName;
        pItem->m_strNameEllipsed = pItem->m_strName;
        EllipseStringIfNeeded(pItem->m_strNameEllipsed);
        pItem->m_strDescription = strDescription;
        pItem->m_pButton->SetLabel( pItem->m_strNameEllipsed );
        pItem->m_pButton->SetHelpText( strDescription );
#if wxUSE_TOOLTIPS
        pItem->m_pButton->SetToolTip(pItem->m_strDescription);
#endif
    }
    return 0;
}


wxInt32 CBOINCTaskCtrl::UpdateControls() {
    unsigned int        i;
    unsigned int        j;
    bool                bCreateMainSizer = false;
    int                 layoutChanged = 0;
    CTaskItemGroup*     pGroup = NULL;
    CTaskItem*          pItem = NULL;


    bCreateMainSizer = !GetSizer();
    if (bCreateMainSizer) {
        SetAutoLayout(TRUE);
        m_pSizer = new wxBoxSizer( wxVERTICAL  );
        m_pSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
        layoutChanged = 1;
    }


    // Create static boxes and sizers if they don't exist
    for (i=0; i < m_pParent->m_TaskGroups.size(); i++) {
        pGroup = m_pParent->m_TaskGroups[i];
        if (!pGroup->m_pStaticBoxSizer) {
            pGroup->m_pStaticBox = new wxStaticBox(this, wxID_ANY, pGroup->m_strName);
            pGroup->m_pStaticBoxSizer = new wxStaticBoxSizer(pGroup->m_pStaticBox, wxVERTICAL);
            m_pSizer->Add(pGroup->m_pStaticBoxSizer, 0, wxEXPAND|wxALL, 5);
            layoutChanged = 1;
        }
    }

    // Create buttons if they don't exist
    for (i=0; i < m_pParent->m_TaskGroups.size(); i++) {
        pGroup = m_pParent->m_TaskGroups[i];
        for (j=0; j < pGroup->m_Tasks.size(); j++) {
            pItem = pGroup->m_Tasks[j];
            if (!pItem->m_pButton) {
                pItem->m_pButton = new wxButton;
                pItem->m_strNameEllipsed = pItem->m_strName;
                EllipseStringIfNeeded(pItem->m_strNameEllipsed);
#ifdef __WXMSW__
                // On Windows with wxWidgets 2.9.4, buttons don't refresh properly unless
                // they are children of the wxStaticBox, but on Mac the layout is wrong
                // unless the buttons are children of the parent of the wxStaticBox.
                // ToDo: merge these cases when these bugs are fixed in wxWidgets.
                pItem->m_pButton->Create(pGroup->m_pStaticBox, pItem->m_iEventID, pItem->m_strNameEllipsed, wxDefaultPosition, wxSize(taskButtonWidth, -1), 0);
#else
                pItem->m_pButton->Create(this, pItem->m_iEventID, pItem->m_strNameEllipsed, wxDefaultPosition, wxSize(taskButtonWidth, -1), 0);
#endif
                pItem->m_pButton->SetHelpText(pItem->m_strDescription);
#if wxUSE_TOOLTIPS
                pItem->m_pButton->SetToolTip(pItem->m_strDescription);
#endif
                pGroup->m_pStaticBoxSizer->Add(pItem->m_pButton, 0, wxEXPAND|wxALL, 5);
                layoutChanged = 1;
            }
        }
    }

    if (bCreateMainSizer) {
        SetSizer(m_pSizer);
    }

    // Force update layout and scrollbars, since nothing we do here
    // necessarily generates a size event which would do it for us.
    if (layoutChanged) {
        Fit ();
    }

    return layoutChanged;
}


bool CBOINCTaskCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;

    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    pConfig->SetPath(strBaseConfigLocation + wxT("TaskCtrl/"));

    //WriteCustomization(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    return true;
}


bool CBOINCTaskCtrl::OnRestoreState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;

    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    pConfig->SetPath(strBaseConfigLocation + wxT("TaskCtrl/"));

    //ReadCustomization(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    return true;
}


#ifdef __WXMSW__
// Work around a problem on Windows where clicking on a button
// in the web sites Task Item Group sometimes causes the task
// control panel to scroll that button out of view but does not
// send the button clicked event.  This is because the task
// control panel is the parent of the Task Item Group's
// wxStaticBox, which is the parent of the button; if we have
// scroll bars the Child Focus Event scrolls the task control
// panel to make the wxStaticBox fully visible. To prevent this,
// we intercept the Child Focus Event, scroll only enough to
// make the button visible if it is not already visible, and
// do not call event.Skip.
void CBOINCTaskCtrl::OnChildFocus(wxChildFocusEvent&) {
    int stepx, stepy;
    int startx, starty;
    int diff = 0;

    wxWindow* theButton = wxWindow::FindFocus();
    if (!theButton) return;

    // Get button position relative to Task Control's viewing area
    wxRect buttonRect(
		ScreenToClient(theButton->GetScreenPosition()), theButton->GetSize()
	);

    const wxRect viewRect(GetClientRect());
    if (viewRect.Contains(buttonRect)){
        return; // Already fully visible
    }

    GetScrollPixelsPerUnit(&stepx, &stepy);

    GetViewStart(&startx, &starty);

    if (buttonRect.GetTop() < 0) {
        diff = buttonRect.GetTop();
    } else if (buttonRect.GetBottom() > viewRect.GetHeight()) {
        diff = buttonRect.GetBottom() - viewRect.GetHeight() + 1;
        // round up to next scroll step if we can't get exact position,
        // so that the button is fully visible
        diff += stepy - 1;
    }

    starty = (starty * stepy + diff) / stepy;
    Scroll(startx, starty);
}
#endif


void CBOINCTaskCtrl::EllipseStringIfNeeded(wxString& s) {
    int w, h;
    const int maxWidth = taskButtonWidth - 10;

    GetTextExtent(s, &w, &h);

    // Adapted from ellipis code in wxRendererGeneric::DrawHeaderButtonContents()
    if (w > maxWidth) {
        int ellipsisWidth;
        GetTextExtent( wxT("..."), &ellipsisWidth, NULL);
        if (ellipsisWidth > maxWidth) {
            s.Clear();
            w = 0;
        } else {
            do {
                s.Truncate( s.length() - 1 );
                GetTextExtent( s, &w, &h);
            } while (((w + ellipsisWidth) > maxWidth) && s.length() );
            s.append( wxT("...") );
        }
    }
}

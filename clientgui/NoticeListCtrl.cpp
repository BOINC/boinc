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
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "NoticeListCtrl.h"
#endif

#include "stdwx.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "NoticeListCtrl.h"

////@begin XPM images
////@end XPM images

#if wxUSE_ACCESSIBILITY || defined(__WXMAC__)

#ifdef __WXMAC__

CNoticeListCtrlAccessible::CNoticeListCtrlAccessible(wxWindow* win) {
    mp_win = win;
    SetupMacAccessibilitySupport();
}


CNoticeListCtrlAccessible::~CNoticeListCtrlAccessible() {
    RemoveMacAccessibilitySupport();
}

#endif

// Gets the name of the specified object.
wxAccStatus CNoticeListCtrlAccessible::GetName(int childId, wxString* name) {
    static wxString strBuffer;

    if (childId == wxACC_SELF) {
        *name = _("Notice List");
    } else {
        CMainDocument* pDoc = wxDynamicCast(wxGetApp().GetDocument(), CMainDocument);
        strBuffer = wxEmptyString;

        if (pDoc) {
            strBuffer = wxString(pDoc->notice(childId-1)->title, wxConvUTF8);
            pDoc->LocalizeNoticeText(strBuffer, true);
            strBuffer = StripHTMLTags(strBuffer);
            *name = strBuffer.c_str();
        }
    }
    return wxACC_OK;
}


// Can return either a child object, or an integer
// representing the child element, starting from 1.
wxAccStatus CNoticeListCtrlAccessible::HitTest(const wxPoint& pt, int* childId, wxAccessible** /*childObject*/) {
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl) {
        *childId = pCtrl->HitTest(pt);
        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Returns the rectangle for this object (id = 0) or a child element (id > 0).
wxAccStatus CNoticeListCtrlAccessible::GetLocation(wxRect& rect, int elementId) {
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl && (0 == elementId)) {
        // List control
        rect.SetPosition(pCtrl->GetScreenPosition());
        rect.SetWidth(pCtrl->GetSize().GetWidth());
        rect.SetHeight(pCtrl->GetSize().GetHeight());
        return wxACC_OK;
    } else if (pCtrl && (0 != elementId)) {
        pCtrl->GetItemRect(elementId - 1, rect);
        pCtrl->ClientToScreen(&rect.x, &rect.y);

        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_FALSE;
}


// Gets the number of children.
wxAccStatus CNoticeListCtrlAccessible::GetChildCount(int* childCount) {
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl) {
        *childCount = (int)pCtrl->GetItemCount();
        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Performs the default action. childId is 0 (the action for this object)
// or > 0 (the action for a child).
// Return wxACC_NOT_SUPPORTED if there is no default action for this
// window (e.g. an edit control).
wxAccStatus CNoticeListCtrlAccessible::DoDefaultAction(int childId) {
#if ALLOW_NOTICES_SELECTION

    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    CMainDocument* pDoc = wxDynamicCast(wxGetApp().GetDocument(), CMainDocument);
    if (pCtrl && (childId != wxACC_SELF)) {
        // Zero-based array index
        int iRealChildId = childId - 1;

        pCtrl->SetSelection(iRealChildId);

        // Fire Event 
        NoticeListCtrlEvent evt( 
            wxEVT_NOTICELIST_ITEM_CHANGE, 
            pDoc->notice(iRealChildId)->seqno,  
            wxString(pDoc->notice(iRealChildId)->link, wxConvUTF8)
        ); 
#ifdef __WXMAC__
        evt.SetEventObject(pCtrl); 
#else
        evt.SetEventObject(this); 
#endif

        pCtrl->GetParent()->AddPendingEvent( evt ); 

        return wxACC_OK;
    }
    
#endif      // ALLOW_NOTICES_SELECTION

    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}

// Returns the description for this object or a child.
wxAccStatus CNoticeListCtrlAccessible::GetDescription(int childId, wxString* description) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    static wxString strBuffer;
    wxDateTime dtBuffer;
    wxString strDescription = wxEmptyString;
    wxString strProjectName = wxEmptyString;
    wxString strArrivalTime = wxEmptyString;

    if (pDoc && (childId != wxACC_SELF)) {
        strBuffer = wxEmptyString;
        
        strProjectName = wxString(pDoc->notice(childId-1)->project_name, wxConvUTF8);

        strDescription = wxString(pDoc->notice(childId-1)->description.c_str(), wxConvUTF8);
        pDoc->LocalizeNoticeText(strDescription, true);

        dtBuffer.Set((time_t)pDoc->notice(childId-1)->arrival_time);
        strArrivalTime = dtBuffer.Format();

        if (strProjectName.IsEmpty()) {
            strBuffer.Printf(_("%s; received on %s"), strDescription.c_str(), strArrivalTime.c_str());
        } else {
            strBuffer.Printf(_("%s; received from %s; on %s"), strDescription.c_str(), strProjectName.c_str(), strArrivalTime.c_str());
        }

        strBuffer = StripHTMLTags(strBuffer);
        *description = strBuffer.c_str();
        
        return wxACC_OK;
    }

    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


wxString CNoticeListCtrlAccessible::StripHTMLTags(wxString inBuf) {
    wxString outBuf = wxEmptyString;
    wxString tempBuf = inBuf;

    while (!tempBuf.IsEmpty()) {
        outBuf += tempBuf.BeforeFirst(wxT('<'));
        tempBuf = tempBuf.AfterFirst(wxT('<'));
        if (tempBuf.IsEmpty()) break;
        tempBuf = tempBuf.AfterFirst(wxT('>'));
    }

    return outBuf;
}

#ifndef __WXMAC__

// Navigates from fromId to toId/toObject.
wxAccStatus CNoticeListCtrlAccessible::Navigate(
    wxNavDir navDir, int fromId, int* toId, wxAccessible** toObject
) {

    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    *toObject = NULL;

    if (0 != fromId) {
        switch (navDir) {
        case wxNAVDIR_PREVIOUS:
        case wxNAVDIR_UP:
            if (1 == fromId){
                return wxACC_FALSE;
            } else {
                *toId = fromId - 1;
                return wxACC_OK;
            }
            break;
        case wxNAVDIR_NEXT:
        case wxNAVDIR_DOWN:
            if ((int)pCtrl->GetItemCount() == fromId) {
                return wxACC_FALSE;
            } else {
                *toId = fromId + 1;
                return wxACC_OK;
            }
            return wxACC_FALSE;
            break;
        case wxNAVDIR_LEFT:
            return wxACC_FALSE;
            break;
        case wxNAVDIR_RIGHT:
            return wxACC_FALSE;           
            break;
        case wxNAVDIR_FIRSTCHILD:
            if (1 == fromId) {
                return wxACC_FALSE;
            } else {
                *toId = 1;
                return wxACC_OK;
            }
            break;
        case wxNAVDIR_LASTCHILD:
            if ((int)pCtrl->GetItemCount() == fromId) {
                return wxACC_FALSE;
            } else {
                *toId = (int)pCtrl->GetItemCount();
                return wxACC_OK;
            }
            break;
        }
    }
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Gets the default action for this object (0) or > 0 (the action for a child).
// Return wxACC_OK even if there is no action. actionName is the action, or the empty
// string if there is no action.
// The retrieved string describes the action that is performed on an object,
// not what the object does as a result. For example, a toolbar button that prints
// a document has a default action of "Press" rather than "Prints the current document."
wxAccStatus CNoticeListCtrlAccessible::GetDefaultAction(int childId, wxString* actionName) {
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl && (childId != wxACC_SELF)) {
        *actionName = _("Click");
        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Returns a role constant.
wxAccStatus CNoticeListCtrlAccessible::GetRole(int childId, wxAccRole* role) {
    if (childId == wxACC_SELF) {
        *role = wxROLE_SYSTEM_LIST;
    } else {
        *role = wxROLE_SYSTEM_LISTITEM;
    }
    return wxACC_OK;
}


// Returns a role constant.
wxAccStatus CNoticeListCtrlAccessible::GetState(int childId, long* state) {
    if (childId == wxACC_SELF) {
        *state = wxACC_STATE_SYSTEM_DEFAULT;
    } else {
        CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
        if (pCtrl && (pCtrl->IsSelected(childId - 1))) {
            *state = wxACC_STATE_SYSTEM_SELECTABLE |
                     wxACC_STATE_SYSTEM_FOCUSABLE | 
                     wxACC_STATE_SYSTEM_SELECTED | 
                     wxACC_STATE_SYSTEM_FOCUSED;
        } else if (pCtrl && (pCtrl->IsVisible(childId - 1))) {
            *state = wxACC_STATE_SYSTEM_SELECTABLE |
                     wxACC_STATE_SYSTEM_FOCUSABLE;
        } else {
            *state = wxACC_STATE_SYSTEM_SELECTABLE |
                     wxACC_STATE_SYSTEM_FOCUSABLE |
                     wxACC_STATE_SYSTEM_OFFSCREEN |
                     wxACC_STATE_SYSTEM_INVISIBLE;
        }
    }
    return wxACC_OK;
}


// Selects the object or child.
wxAccStatus CNoticeListCtrlAccessible::Select(int , wxAccSelectionFlags ) {
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Gets a variant representing the selected children
// of this object.
// Acceptable values:
// - a null variant (IsNull() returns true)
// - a list variant (GetType() == wxT("list"))
// - an integer representing the selected child element,
//   or 0 if this object is selected (GetType() == wxT("long"))
// - a "void*" pointer to a wxAccessible child object
wxAccStatus CNoticeListCtrlAccessible::GetSelections(wxVariant* ) {
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}
#endif      // ifndef __WXMAC__

#endif      // wxUSE_ACCESSIBILITY || defined(__WXMAC__)


/*!
 * CNoticeListCtrl event definitions
 */
DEFINE_EVENT_TYPE( wxEVT_NOTICELIST_ITEM_CHANGE )
DEFINE_EVENT_TYPE( wxEVT_NOTICELIST_ITEM_DISPLAY )


/*!
 * CNoticeListCtrl type definition
 */
IMPLEMENT_DYNAMIC_CLASS( CNoticeListCtrl, CBOINCHtmlListBox )
IMPLEMENT_DYNAMIC_CLASS( NoticeListCtrlEvent, wxNotifyEvent )


/*!
 * CNoticeListCtrl event table definition
 */
 
BEGIN_EVENT_TABLE( CNoticeListCtrl, CBOINCHtmlListBox )

////@begin CNoticeListCtrl event table entries
    EVT_LISTBOX(ID_LIST_NOTIFICATIONSVIEW, CNoticeListCtrl::OnSelected)
    EVT_LISTBOX_DCLICK(ID_LIST_NOTIFICATIONSVIEW, CNoticeListCtrl::OnDClicked)
    EVT_HTML_CELL_CLICKED(ID_LIST_NOTIFICATIONSVIEW, CNoticeListCtrl::OnClicked)
    EVT_HTML_LINK_CLICKED(ID_LIST_NOTIFICATIONSVIEW, CNoticeListCtrl::OnLinkClicked)
////@end CNoticeListCtrl event table entries
 
END_EVENT_TABLE()
 
/*!
 * CNoticeListCtrl constructors
 */
 
CNoticeListCtrl::CNoticeListCtrl( )
{
}
 
CNoticeListCtrl::CNoticeListCtrl( wxWindow* parent )
{
    Create( parent );
}
 
 
CNoticeListCtrl::~CNoticeListCtrl( )
{
#ifdef __WXMAC__
    if (m_accessible) {
        delete m_accessible;
    }
#endif
}


/*!
 * CNoticeListCtrl creator
 */
 
bool CNoticeListCtrl::Create( wxWindow* parent )
{
////@begin CNoticeListCtrl member initialisation
////@end CNoticeListCtrl member initialisation

////@begin CNoticeListCtrl creation
    CBOINCHtmlListBox::Create( parent, ID_LIST_NOTIFICATIONSVIEW, wxDefaultPosition, wxDefaultSize,
        wxSUNKEN_BORDER | wxTAB_TRAVERSAL );

#if wxUSE_ACCESSIBILITY
    SetAccessible(new CNoticeListCtrlAccessible(this));
#endif
#ifdef __WXMAC__
    // Enable accessibility only after drawing the page 
    // to avoid a mysterious crash bug
    m_accessible = NULL;
#endif
////@end CNoticeListCtrl creation

    // Display the empty notice notification until we have some
    // notices to display.
    m_bDisplayEmptyNotice = true;
    m_bComputerChanged = true;

    return TRUE;
}


void CNoticeListCtrl::OnSelected( wxCommandEvent& event )
{
    // Fire Event 
    NoticeListCtrlEvent evt( 
        wxEVT_NOTICELIST_ITEM_CHANGE, 
        event.GetInt(),
        wxEmptyString
    ); 
    evt.SetEventObject(this); 

    GetParent()->AddPendingEvent( evt ); 
}


void CNoticeListCtrl::OnClicked( wxHtmlCellEvent& event )
{
    event.Skip();
}


void CNoticeListCtrl::OnDClicked( wxCommandEvent& event )
{
    event.Skip();
}


void CNoticeListCtrl::OnLinkClicked( wxHtmlLinkEvent& event )
{
    // Fire Event 
    NoticeListCtrlEvent evt( 
        wxEVT_NOTICELIST_ITEM_DISPLAY, 
        event.GetInt(),
        event.GetLinkInfo().GetHref()
    ); 
    evt.SetEventObject(this); 

    GetParent()->AddPendingEvent( evt ); 
}


wxString CNoticeListCtrl::OnGetItem(size_t i) const {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString strTitle = wxEmptyString;
    wxString strDescription = wxEmptyString;
    wxString strProjectName = wxEmptyString;
    wxString strURL = wxEmptyString;
    wxString create_time = wxEmptyString;
    wxString strBuffer = wxEmptyString;
    wxString strTemp = wxEmptyString;
    wxDateTime dtBuffer;
    char buf[1024];

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (m_bDisplayEmptyNotice) {
        strBuffer = wxT("<table border=0 cellpadding=5><tr><td>");
        strBuffer += _("There are no notices at this time.");
        strBuffer += wxT("</font></td></tr></table><hr>");
    } else 
    if (pDoc->IsConnected()) {
        NOTICE* np = pDoc->notice((unsigned int)i);

        strURL = wxString(np->link, wxConvUTF8);

        if (!strcmp(np->category, "client")) {
            if (strlen(np->project_name)) {
                sprintf(buf, "%s: %s", np->project_name, "_(\"Notice from BOINC\")");
            } else {
                strcpy(buf, "_(\"Notice from BOINC\")");
            }
        } else if (!strcmp(np->category, "scheduler")) {
            sprintf(buf, "%s: %s", np->project_name, "_(\"Notice from server\")");
        } else {
            if (strlen(np->project_name)) {
                sprintf(buf, "%s: %s", np->project_name, np->title);
            } else {
                strcpy(buf, np->title);
            }
        }
        strTitle = wxString(buf, wxConvUTF8);
        pDoc->LocalizeNoticeText(strTitle, true);

        strDescription = wxString(np->description.c_str(), wxConvUTF8);
        pDoc->LocalizeNoticeText(strDescription, true);

        dtBuffer.Set((time_t)np->create_time);
        create_time = dtBuffer.Format();

        strBuffer = wxT("<table border=0 cellpadding=5><tr><td>");

        if (!strTitle.IsEmpty()) {
            strTemp.Printf(
                wxT("<b>%s</b><br>"),
                strTitle.c_str()
            );
            strBuffer += strTemp;
        }

        strBuffer += strDescription;

        strBuffer += wxT("<br><font size=-2 color=#8f8f8f>");

        strBuffer += create_time;

        if (!strURL.IsEmpty()) {
            strTemp.Printf(
                wxT(" &middot; <a target=_new href=%s>%s</a> "),
                strURL.c_str(),
                _("more...")
            );
            strBuffer += strTemp;
        }

        strBuffer += wxT("</font></td></tr></table><hr>");
    }

    return strBuffer;
}


void CNoticeListCtrl::Clear() {
    SetItemCount(0);
    m_bComputerChanged = true;
}


/*!
 * Update the UI.
 */

bool CNoticeListCtrl::UpdateUI()
{
    static bool bAlreadyRunning = false;
    CMainDocument*  pDoc   = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Call Freeze() / Thaw() only when actually needed; 
    // otherwise it causes unnecessary redraws
    if ((pDoc->GetNoticeCount() <= 0) || (!pDoc->IsConnected()) || m_bComputerChanged) {
        m_bDisplayEmptyNotice = true;
        m_bComputerChanged = false;
        Freeze();
        SetItemCount(1);
        Thaw();
        return true;
    }
    
    // We must prevent re-entry because our asynchronous 
    // Internet access on Windows calls Yield() which can 
    // allow this to be called again.
    if (!bAlreadyRunning) {
        bAlreadyRunning = true;
        if (
            pDoc->IsConnected() &&
            (pDoc->notices.complete ||
            ((int)GetItemCount() != pDoc->GetNoticeCount()) ||
            ((pDoc->GetNoticeCount() > 0) && (m_bDisplayEmptyNotice == true)))
        ) {
            pDoc->notices.complete = false;
            m_bDisplayEmptyNotice = false;
            Freeze();
            SetItemCount(pDoc->GetNoticeCount());
            Thaw();
        }

#ifdef __WXMAC__
        // Enable accessibility only after drawing the page 
        // to avoid a mysterious crash bug
        if (m_accessible == NULL) {
            m_accessible = new CNoticeListCtrlAccessible(this);
        }
#endif

        bAlreadyRunning = false;
    }
        
    return true;
}

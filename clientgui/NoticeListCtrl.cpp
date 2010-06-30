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
#include "NoticeListCtrl.h"

////@begin XPM images
////@end XPM images


#ifdef __WXMAC__

CNoticeListCtrlAccessible::CNoticeListCtrlAccessible(wxWindow* win) {
    mp_win = win;
    SetupMacAccessibilitySupport();
}


CNoticeListCtrlAccessible::~CNoticeListCtrlAccessible() {
    RemoveMacAccessibilitySupport();
}

#endif

#if wxUSE_ACCESSIBILITY || defined(__WXMAC__)

// Gets the name of the specified object.
wxAccStatus CNoticeListCtrlAccessible::GetName(int childId, wxString* name)
{
    if (childId == wxACC_SELF)
    {
        *name = _("Notice List");
    }
    else
    {
        CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
        if (pCtrl)
        {
            *name = pCtrl->GetItem(childId - 1)->GetTitle().c_str();
        }
    }
    return wxACC_OK;
}


// Can return either a child object, or an integer
// representing the child element, starting from 1.
wxAccStatus CNoticeListCtrlAccessible::HitTest(const wxPoint& pt, int* childId, wxAccessible** /*childObject*/)
{
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl)
    {
        *childId = pCtrl->HitTest(pt);
        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Returns the rectangle for this object (id = 0) or a child element (id > 0).
wxAccStatus CNoticeListCtrlAccessible::GetLocation(wxRect& rect, int elementId)
{
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl && (0 == elementId))
    {
        // List control
        rect.SetPosition(pCtrl->GetScreenPosition());
        rect.SetWidth(pCtrl->GetSize().GetWidth());
        rect.SetHeight(pCtrl->GetSize().GetHeight());
        return wxACC_OK;
    }
    else if (pCtrl && (0 != elementId))
    {
        // List item
        wxSize cCtrlSize = pCtrl->GetClientSize();
        int    iItemWidth = cCtrlSize.GetWidth();
        int    iItemHeight = pCtrl->GetTotalClientHeight() / (int)pCtrl->GetItemCount();

        // Set the initial control postition to the absolute coords of the upper
        //   left hand position of the control
        rect.SetPosition(pCtrl->GetScreenPosition());
        rect.width = iItemWidth - 1;
        rect.height = iItemHeight - 1;

        if (1 == elementId)
        {
            // First child
        }
        else
        {
            // Other children
            rect.SetTop(rect.GetTop() + ((elementId - 1) * iItemHeight) + 1);
            rect.height -= 1;
        }
        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_FALSE;
}


// Gets the number of children.
wxAccStatus CNoticeListCtrlAccessible::GetChildCount(int* childCount)
{
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl)
    {
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
wxAccStatus CNoticeListCtrlAccessible::DoDefaultAction(int childId)
{
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl && (childId != wxACC_SELF))
    {
        // Zero-based array index
        int iRealChildId = childId - 1;

        pCtrl->SetSelection(iRealChildId);

        // Fire Event 
        NoticeListCtrlEvent evt( 
            wxEVT_NOTICELIST_ITEM_CHANGE, 
            pCtrl->GetItem(iRealChildId)->GetSeqNo(),  
            pCtrl->GetItem(iRealChildId)->GetURL() 
        ); 
#ifdef __WXMAC__
        evt.SetEventObject(pCtrl); 
#else
        evt.SetEventObject(this); 
#endif

        pCtrl->GetParent()->AddPendingEvent( evt ); 

        return wxACC_OK;
    }

    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Returns the description for this object or a child.
wxAccStatus CNoticeListCtrlAccessible::GetDescription(int childId, wxString* description)
{
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl && (childId != wxACC_SELF))
    {
        *description = pCtrl->GetItem(childId - 1)->GetDescription().c_str();
        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


#ifndef __WXMAC__

// Navigates from fromId to toId/toObject.
wxAccStatus CNoticeListCtrlAccessible::Navigate(
    wxNavDir navDir, int fromId, int* toId, wxAccessible** toObject
) {

    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    *toObject = NULL;

    if (0 != fromId)
    {
        switch (navDir)
        {
        case wxNAVDIR_PREVIOUS:
        case wxNAVDIR_UP:
            if (1 == fromId)
            {
                return wxACC_FALSE;
            }
            else
            {
                *toId = fromId - 1;
                return wxACC_OK;
            }
            break;
        case wxNAVDIR_NEXT:
        case wxNAVDIR_DOWN:
            if ((int)pCtrl->GetItemCount() == fromId)
            {
                return wxACC_FALSE;
            }
            else
            {
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
            if (1 == fromId)
            {
                return wxACC_FALSE;
            }
            else
            {
                *toId = 1;
                return wxACC_OK;
            }
            break;
        case wxNAVDIR_LASTCHILD:
            if ((int)pCtrl->GetItemCount() == fromId)
            {
                return wxACC_FALSE;
            }
            else
            {
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
wxAccStatus CNoticeListCtrlAccessible::GetDefaultAction(int childId, wxString* actionName)
{
    CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
    if (pCtrl && (childId != wxACC_SELF))
    {
        *actionName = _("Click");
        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Returns a role constant.
wxAccStatus CNoticeListCtrlAccessible::GetRole(int childId, wxAccRole* role)
{
    if (childId == wxACC_SELF)
    {
        *role = wxROLE_SYSTEM_LIST;
    }
    else
    {
        *role = wxROLE_SYSTEM_LISTITEM;
    }
    return wxACC_OK;
}


// Returns a role constant.
wxAccStatus CNoticeListCtrlAccessible::GetState(int childId, long* state)
{
    if (childId == wxACC_SELF)
    {
        *state = wxACC_STATE_SYSTEM_DEFAULT;
    }
    else
    {
        CNoticeListCtrl* pCtrl = wxDynamicCast(GetWindow(), CNoticeListCtrl);
        if (pCtrl && (pCtrl->IsSelected(childId - 1)))
        {
            *state = wxACC_STATE_SYSTEM_SELECTABLE |
                     wxACC_STATE_SYSTEM_FOCUSABLE | 
                     wxACC_STATE_SYSTEM_SELECTED | 
                     wxACC_STATE_SYSTEM_FOCUSED;
        }
        else if (pCtrl && (pCtrl->IsVisible(childId - 1)))
        {
            *state = wxACC_STATE_SYSTEM_SELECTABLE |
                     wxACC_STATE_SYSTEM_FOCUSABLE;
        }
        else
        {
            *state = wxACC_STATE_SYSTEM_SELECTABLE |
                     wxACC_STATE_SYSTEM_FOCUSABLE |
                     wxACC_STATE_SYSTEM_OFFSCREEN |
                     wxACC_STATE_SYSTEM_INVISIBLE;
        }
    }
    return wxACC_OK;
}


// Selects the object or child.
wxAccStatus CNoticeListCtrlAccessible::Select(int , wxAccSelectionFlags )
{
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
wxAccStatus CNoticeListCtrlAccessible::GetSelections(wxVariant* )
{
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}
#endif      // ifndef __WXMAC__
#endif      // wxUSE_ACCESSIBILITY || defined(__WXMAC__)


/*!
 * CNoticeListItem type definition
 */
IMPLEMENT_DYNAMIC_CLASS( CNoticeListItem, wxObject )


/*!
 * CNoticeListCtrl event definitions
 */
DEFINE_EVENT_TYPE( wxEVT_NOTICELIST_ITEM_CHANGE )
DEFINE_EVENT_TYPE( wxEVT_NOTICELIST_ITEM_DISPLAY )


/*!
 * CNoticeListCtrl type definition
 */
IMPLEMENT_DYNAMIC_CLASS( CNoticeListCtrl, wxHtmlListBox )
IMPLEMENT_DYNAMIC_CLASS( NoticeListCtrlEvent, wxNotifyEvent )


/*!
 * CNoticeListCtrl event table definition
 */
 
BEGIN_EVENT_TABLE( CNoticeListCtrl, wxHtmlListBox )

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
    Clear();

#ifdef __WXMAC__
    if (m_accessible) {
        delete m_accessible;
    }
#endif
}


/*!
 * Remove all entries from the project list.
 */
 
void CNoticeListCtrl::Clear()
{
    std::vector<CNoticeListItem*>::iterator iter;
    CNoticeListItem* pItem = NULL;

    iter = m_Items.begin();
    while (iter != m_Items.end()) {
        pItem = *iter;
        iter = m_Items.erase(iter);
        delete pItem;
    }
}

/*!
 * CNoticeListCtrl creator
 */
 
bool CNoticeListCtrl::Create( wxWindow* parent )
{
////@begin CNoticeListCtrl member initialisation
    m_bNeedsRefresh = false;
////@end CNoticeListCtrl member initialisation

////@begin CNoticeListCtrl creation
    wxHtmlListBox::Create( parent, ID_LIST_NOTIFICATIONSVIEW, wxDefaultPosition, wxDefaultSize,
        wxSUNKEN_BORDER | wxTAB_TRAVERSAL );

#if wxUSE_ACCESSIBILITY
    SetAccessible(new CNoticeListCtrlAccessible(this));
#endif
#ifdef __WXMAC__
    m_accessible = new CNoticeListCtrlAccessible(this);
#endif
////@end CNoticeListCtrl creation

    return TRUE;
}


void CNoticeListCtrl::OnSelected( wxCommandEvent& event )
{
    // Fire Event 
    NoticeListCtrlEvent evt( 
        wxEVT_NOTICELIST_ITEM_CHANGE, 
        event.GetInt(),  
        m_Items[event.GetInt()]->GetURL() 
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
        m_Items[event.GetInt()]->GetURL() 
    ); 
    evt.SetEventObject(this); 

    GetParent()->AddPendingEvent( evt ); 
}


wxString CNoticeListCtrl::OnGetItem(size_t i) const
{
    wxString strBuffer = wxEmptyString;
    wxString strTemp = wxEmptyString;

    if (!m_Items[i]->GetTitle().IsEmpty()) {
        strTemp.Printf(
            wxT("<b>%s</b><br>"),
            m_Items[i]->GetTitle().c_str()
        );
        strBuffer += strTemp;
    }

    strBuffer += m_Items[i]->GetDescription();

    strBuffer += wxT("<br><font size=-2 color=#8f8f8f>");

    if (!m_Items[i]->GetProjectName().IsEmpty()) {
        strTemp.Printf(
            wxT("%s %s<br>"),
            _("From"),
            m_Items[i]->GetProjectName().c_str()
        );
        strBuffer += strTemp;
    }

    strBuffer += m_Items[i]->GetArrivalTime();

    if (!m_Items[i]->GetURL().IsEmpty()) {
        strTemp.Printf(
            wxT(" &middot; <a target=_new href=%s>%s</a> "),
            m_Items[i]->GetURL().c_str(),
            _("more...")
        );
        strBuffer += strTemp;
    }

    strBuffer += wxT("</font><hr>\n");
    
    return strBuffer;
}


/*!
 * Append a new entry to the project list.
 */
 
bool CNoticeListCtrl::Add(
    int iSeqNo,
    wxString strProjectName,
    wxString strURL, 
    wxString strTitle,
    wxString strDescription,
    wxString strCategory,
    wxString strArrivalTime
)
{
    CNoticeListItem* pItem = new CNoticeListItem();

    pItem->SetSeqNo( iSeqNo );
    pItem->SetProjectName( strProjectName );
    pItem->SetURL( strURL );
    pItem->SetTitle( strTitle );
    pItem->SetDescription( strDescription );
    pItem->SetCategory( strCategory );
    pItem->SetArrivalTime( strArrivalTime );
    pItem->SetDeletionFlag( false );

    m_bNeedsRefresh = true;

    m_Items.insert(m_Items.begin(), pItem);
    return true;
}


/*!
 * Update an existing entry in the project list.
 */
 
bool CNoticeListCtrl::Update(
    int iSeqNo
)
{
    bool bRetVal = false;

    unsigned int n = (unsigned int)m_Items.size();
    for (unsigned int i = 0; i < n; i++) {
        if (iSeqNo == m_Items[i]->GetSeqNo()) {
            m_Items[i]->SetDeletionFlag( false );
            bRetVal = true;
        }
    }

    return bRetVal;
}


/*!
 * Check to see if the requested entry is already in the control.
 */
 
bool CNoticeListCtrl::Exists( int iSeqNo )
{
    bool bRetVal = false;

    unsigned int n = (unsigned int)m_Items.size();
    for (unsigned int i = 0; i < n; i++) {
        if (iSeqNo == m_Items[i]->GetSeqNo()) {
            bRetVal = true;
        }
    }

    return bRetVal;
}


/*!
 * Flag all entries for delete.
 */
 
void CNoticeListCtrl::FlagAllItemsForDelete()
{
    unsigned int n = (unsigned int)m_Items.size();
    for (unsigned int i = 0; i < n; i++) {
        m_Items[i]->SetDeletionFlag(true);
    }
}


/*!
 * Purge deleted items.
 */
 
void CNoticeListCtrl::DeleteAllFlagedItems()
{
    std::vector<CNoticeListItem*>::iterator iter;
    CNoticeListItem* pItem = NULL;

    iter = m_Items.begin();
    while (iter != m_Items.end()) {
        pItem = *iter;
        if (pItem->GetDeletionFlag()) {

            iter = m_Items.erase(iter);
            delete pItem;

            m_bNeedsRefresh = true;

        } else {
            iter++;
        }
    }

    if (m_bNeedsRefresh) {
        UpdateUI();
        m_bNeedsRefresh = false;
    }
}


/*!
 * Update the UI.
 */
 
bool CNoticeListCtrl::UpdateUI()
{
    SetItemCount(m_Items.size());
    return true;
}


/*!
 * Return the project list entry at a given index.
 */
 
CNoticeListItem* CNoticeListCtrl::GetItem( 
    int iIndex
)
{
    return m_Items[iIndex];
}


/*!
 * Return the total height of all the client items.
 */
 
wxCoord CNoticeListCtrl::GetTotalClientHeight()
{
    return EstimateTotalHeight();
}


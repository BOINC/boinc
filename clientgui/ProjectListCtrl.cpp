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
#pragma implementation "ProjectListCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "ProjectListCtrl.h"

////@begin XPM images
#include "res/externalweblink.xpm"
#include "res/nvidiaicon.xpm"
#include "res/atiicon.xpm"
#include "res/multicore.xpm"
////@end XPM images


#ifdef __WXMAC__

CProjectListCtrlAccessible::CProjectListCtrlAccessible(wxWindow* win) {
    mp_win = win;
    SetupMacAccessibilitySupport();
}


CProjectListCtrlAccessible::~CProjectListCtrlAccessible() {
    RemoveMacAccessibilitySupport();
}

#endif

#if wxUSE_ACCESSIBILITY || defined(__WXMAC__)

// Gets the name of the specified object.
wxAccStatus CProjectListCtrlAccessible::GetName(int childId, wxString* name)
{
    if (childId == wxACC_SELF)
    {
        *name = wxT("Project List");
    }
    else
    {
        CProjectListCtrl* pCtrl = wxDynamicCast(GetWindow(), CProjectListCtrl);
        if (pCtrl)
        {
            *name = pCtrl->GetItem(childId - 1)->GetTitle().c_str();
        }
    }
    return wxACC_OK;
}


// Can return either a child object, or an integer
// representing the child element, starting from 1.
wxAccStatus CProjectListCtrlAccessible::HitTest(const wxPoint& pt, int* childId, wxAccessible** /*childObject*/)
{
    CProjectListCtrl* pCtrl = wxDynamicCast(GetWindow(), CProjectListCtrl);
    if (pCtrl)
    {
        *childId = pCtrl->HitTest(pt);
        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Returns the rectangle for this object (id = 0) or a child element (id > 0).
wxAccStatus CProjectListCtrlAccessible::GetLocation(wxRect& rect, int elementId)
{
    CProjectListCtrl* pCtrl = wxDynamicCast(GetWindow(), CProjectListCtrl);
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
wxAccStatus CProjectListCtrlAccessible::GetChildCount(int* childCount)
{
    CProjectListCtrl* pCtrl = wxDynamicCast(GetWindow(), CProjectListCtrl);
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
wxAccStatus CProjectListCtrlAccessible::DoDefaultAction(int childId)
{
    CProjectListCtrl* pCtrl = wxDynamicCast(GetWindow(), CProjectListCtrl);
    if (pCtrl && (childId != wxACC_SELF))
    {
        // Zero-based array index
        int iRealChildId = childId - 1;

        pCtrl->SetSelection(iRealChildId);

        // Fire Event 
        ProjectListCtrlEvent evt( 
            wxEVT_PROJECTLIST_ITEM_CHANGE, 
            pCtrl->GetItem(iRealChildId)->GetTitle(),  
            pCtrl->GetItem(iRealChildId)->GetURL(), 
            true 
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
wxAccStatus CProjectListCtrlAccessible::GetDescription(int childId, wxString* description)
{
    CProjectListCtrl* pCtrl = wxDynamicCast(GetWindow(), CProjectListCtrl);
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
wxAccStatus CProjectListCtrlAccessible::Navigate(
    wxNavDir navDir, int fromId, int* toId, wxAccessible** toObject
) {

    CProjectListCtrl* pCtrl = wxDynamicCast(GetWindow(), CProjectListCtrl);
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
wxAccStatus CProjectListCtrlAccessible::GetDefaultAction(int childId, wxString* actionName)
{
    CProjectListCtrl* pCtrl = wxDynamicCast(GetWindow(), CProjectListCtrl);
    if (pCtrl && (childId != wxACC_SELF))
    {
        *actionName = _("Click");
        return wxACC_OK;
    }
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}


// Returns a role constant.
wxAccStatus CProjectListCtrlAccessible::GetRole(int childId, wxAccRole* role)
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
wxAccStatus CProjectListCtrlAccessible::GetState(int childId, long* state)
{
    if (childId == wxACC_SELF)
    {
        *state = wxACC_STATE_SYSTEM_DEFAULT;
    }
    else
    {
        CProjectListCtrl* pCtrl = wxDynamicCast(GetWindow(), CProjectListCtrl);
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
wxAccStatus CProjectListCtrlAccessible::Select(int , wxAccSelectionFlags )
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
wxAccStatus CProjectListCtrlAccessible::GetSelections(wxVariant* )
{
    // Let the framework handle the other cases.
    return wxACC_NOT_IMPLEMENTED;
}
#endif      // ifndef __WXMAC__
#endif      // wxUSE_ACCESSIBILITY || defined(__WXMAC__)


/*!
 * CProjectListItem type definition
 */
IMPLEMENT_DYNAMIC_CLASS( CProjectListItem, wxObject )


/*!
 * CProjectListCtrl event definitions
 */
DEFINE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_CHANGE )
DEFINE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_DISPLAY )


/*!
 * CProjectListCtrl type definition
 */
IMPLEMENT_DYNAMIC_CLASS( CProjectListCtrl, wxHtmlListBox )
IMPLEMENT_DYNAMIC_CLASS( ProjectListCtrlEvent, wxNotifyEvent )


/*!
 * CProjectListCtrl event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectListCtrl, wxHtmlListBox )

////@begin CProjectListCtrl event table entries
    EVT_LISTBOX(ID_PROJECTLISTCTRL, CProjectListCtrl::OnSelected)
    EVT_HTML_CELL_CLICKED( ID_PROJECTLISTCTRL, CProjectListCtrl::OnClicked )
    EVT_LISTBOX_DCLICK(ID_PROJECTLISTCTRL, CProjectListCtrl::OnDClicked)
    EVT_HTML_LINK_CLICKED( ID_PROJECTLISTCTRL, CProjectListCtrl::OnLinkClicked )
    EVT_HTML_CELL_HOVER( ID_PROJECTLISTCTRL, CProjectListCtrl::OnHover )
////@end CProjectListCtrl event table entries
 
END_EVENT_TABLE()
 
/*!
 * CProjectListCtrl constructors
 */
 
CProjectListCtrl::CProjectListCtrl( )
{
}
 
CProjectListCtrl::CProjectListCtrl( wxWindow* parent )
{
    Create( parent );
}
 
 
 #ifdef __WXMAC__
CProjectListCtrl::~CProjectListCtrl( )
{
    if (m_accessible) {
        delete m_accessible;
    }
}
#endif

/*!
 * CProjectList creator
 */
 
bool CProjectListCtrl::Create( wxWindow* parent )
{
////@begin CProjectListCtrl member initialisation
////@end CProjectListCtrl member initialisation

////@begin CProjectListCtrl creation
    wxHtmlListBox::Create( parent, ID_PROJECTLISTCTRL, wxDefaultPosition, wxDefaultSize,
        wxSUNKEN_BORDER | wxTAB_TRAVERSAL );

#if wxUSE_ACCESSIBILITY
    SetAccessible(new CProjectListCtrlAccessible(this));
#endif
#ifdef __WXMAC__
    m_accessible = new CProjectListCtrlAccessible(this);
#endif

    wxMemoryFSHandler::AddFile(wxT("webexternallink.xpm"), wxBitmap(externalweblink_xpm), wxBITMAP_TYPE_XPM);
    wxMemoryFSHandler::AddFile(wxT("nvidiaicon.xpm"), wxBitmap(nvidiaicon_xpm), wxBITMAP_TYPE_XPM);
    wxMemoryFSHandler::AddFile(wxT("atiicon.xpm"), wxBitmap(atiicon_xpm), wxBITMAP_TYPE_XPM);
    wxMemoryFSHandler::AddFile(wxT("multicore.xpm"), wxBitmap(multicore_xpm), wxBITMAP_TYPE_XPM);
////@end CProjectListCtrl creation

    return TRUE;
}


void CProjectListCtrl::OnSelected( wxCommandEvent& event )
{
    // Fire Event 
    ProjectListCtrlEvent evt( 
        wxEVT_PROJECTLIST_ITEM_CHANGE, 
        m_Items[event.GetInt()]->GetTitle(),  
        m_Items[event.GetInt()]->GetURL(), 
        m_Items[event.GetInt()]->IsPlatformSupported() 
    ); 
    evt.SetEventObject(this); 

    GetParent()->AddPendingEvent( evt ); 
}


void CProjectListCtrl::OnClicked( wxHtmlCellEvent& event )
{
    event.Skip();
}


void CProjectListCtrl::OnDClicked( wxCommandEvent& event )
{
    event.Skip();
}


void CProjectListCtrl::OnLinkClicked( wxHtmlLinkEvent& event )
{
    // Fire Event 
    ProjectListCtrlEvent evt( 
        wxEVT_PROJECTLIST_ITEM_DISPLAY, 
        wxEmptyString,  
        event.GetLinkInfo().GetHref(), 
        true 
    ); 
    evt.SetEventObject(this); 

    GetParent()->AddPendingEvent( evt ); 
}


void CProjectListCtrl::OnHover( wxHtmlCellEvent& event )
{
    long i = 0;
    wxHtmlCell* pCell = event.GetCell();
    wxHtmlCell* pRootCell = pCell->GetRootCell();
    wxString strMulticoreIcon = wxT("multicore");
    wxString strNvidiaIcon = wxT("nvidiaicon");
    wxString strATIIcon = wxT("atiicon");
    wxString strWebsiteIcon = wxT("website");
    wxString strTooltip = wxEmptyString;

    wxHtmlCell* pAnchor = pCell->GetParent()->GetFirstChild();

    if (pAnchor->Find(wxHTML_COND_ISANCHOR, &strMulticoreIcon)) {
        strTooltip = _("Multicore CPU Supported");
    } else if (pAnchor->Find(wxHTML_COND_ISANCHOR, &strNvidiaIcon)) {
        strTooltip = _("Nvidia GPU Supported");
    } else if (pAnchor->Find(wxHTML_COND_ISANCHOR, &strATIIcon)) {
        strTooltip = _("ATI GPU Supported");
    } else if (pAnchor->Find(wxHTML_COND_ISANCHOR, &strWebsiteIcon)) {
        strTooltip = _("Project Website");
    } else {
        // Convert current HTML cell into an array index
        pRootCell->GetId().ToLong(&i);

        strTooltip = m_Items[i]->GetDescription();
    }

    // Set Tooltip to the item currently being hovered over
    SetToolTip(strTooltip);
}


wxString CProjectListCtrl::OnGetItem(size_t i) const
{
    wxString strTopRow = wxEmptyString;
    wxString strBuffer = wxEmptyString;


    //
    // Top Row
    // 
    strTopRow += wxT("<table cellpadding=0 cellspacing=1>");

    strTopRow += wxT("<tr>");

    strBuffer.Printf(
        wxT("<td width=100%%>%s</td>"),
        m_Items[i]->GetTitle().c_str()
    );
    strTopRow += strBuffer;
    
    if (m_Items[i]->IsMulticoreSupported()) {
        strTopRow += wxT("<td><a name=\"multicore\"><img height=16 width=16 src=\"memory:multicore.xpm\"></a></td>");
    }

    if (m_Items[i]->IsNvidiaGPUSupported()) {
        strTopRow += wxT("<td><a name=\"nvidiaicon\"><img height=16 width=16 src=\"memory:nvidiaicon.xpm\"></a></td>");
    }

    if (m_Items[i]->IsATIGPUSupported()) {
        strTopRow += wxT("<td><a name=\"atiicon\"><img height=16 width=16 src=\"memory:atiicon.xpm\"></a></td>");
    }

    strBuffer.Printf(
        wxT("<td><a name=\"website\"href=\"%s\"><img height=16 width=16 src=\"memory:webexternallink.xpm\"></a></td>"),
        m_Items[i]->GetURL().c_str()
    );
    strTopRow += strBuffer;

    strTopRow += wxT("</tr>");
    strTopRow += wxT("</table>");

    return strTopRow;
}


/*!
 * Append a new entry to the project list.
 */
 
bool CProjectListCtrl::Append(
    wxString strURL,
    wxString strTitle,
    wxString strImage,
    wxString strDescription,
    bool bNvidiaGPUSupported,
    bool bATIGPUSupported,
    bool bMulticoreSupported,
    bool bSupported
)
{
    CProjectListItem* pItem = new CProjectListItem();

    pItem->SetURL( strURL );
    pItem->SetTitle( strTitle );
    pItem->SetImage( strImage );
    pItem->SetDescription( strDescription );
    pItem->SetNvidiaGPUSupported( bNvidiaGPUSupported );
    pItem->SetATIGPUSupported( bATIGPUSupported );
    pItem->SetMulticoreSupported( bMulticoreSupported );
    pItem->SetPlatformSupported( bSupported );

    m_Items.push_back(pItem);
    SetItemCount(m_Items.size());

    return true;
}


/*!
 * Return the project list entry at a given index.
 */
 
CProjectListItem* CProjectListCtrl::GetItem( 
    int iIndex
)
{
    return m_Items[iIndex];
}


/*!
 * Return the total height of all the client items.
 */
 
wxCoord CProjectListCtrl::GetTotalClientHeight()
{
    return EstimateTotalHeight();
}


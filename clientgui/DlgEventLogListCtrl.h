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

#ifndef _DLGEVENTLOGLISTCTRL_H_
#define _DLGEVENTLOGLISTCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgEventLogListCtrl.cpp"
#endif

#ifdef __WXMAC__
#include "macAccessiblity.h"
#define DLG_LISTCTRL_BASE wxGenericListCtrl
#else
#define DLG_LISTCTRL_BASE wxListView

#endif

class CDlgEventLog;

class CDlgEventLogListCtrl : public DLG_LISTCTRL_BASE
{
    DECLARE_DYNAMIC_CLASS(CDlgEventLogListCtrl)
    DECLARE_EVENT_TABLE()

public:
    CDlgEventLogListCtrl();
    CDlgEventLogListCtrl(CDlgEventLog* pView, wxWindowID iListWindowID, int iListWindowFlags);

#ifdef __WXMAC__
    ~CDlgEventLogListCtrl();
#endif

private:
    
    virtual wxString        OnGetItemText(long item, long column) const;
    virtual int             OnGetItemImage(long item) const;
    virtual wxListItemAttr* OnGetItemAttr(long item) const;
    virtual wxColour        GetBackgroundColour();
    void                    OnMouseUp(wxMouseEvent& event);

    bool                    m_bIsSingleSelection;

    CDlgEventLog*           m_pParentView;

#ifdef __WXMAC__
    void                    SetupMacAccessibilitySupport();
    void                    RemoveMacAccessibilitySupport();

    ListAccessData   accessibilityHandlerData;
    
    EventHandlerRef         m_pHeaderAccessibilityEventHandlerRef;
    EventHandlerRef         m_pBodyAccessibilityEventHandlerRef;
#endif
};

#endif

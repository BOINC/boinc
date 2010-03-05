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

#ifndef _BOINCLISTCTRL_H_
#define _BOINCLISTCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCListCtrl.cpp"
#endif

#ifdef __WXMSW__
#define USE_NATIVE_LISTCONTROL 1
#else
#define USE_NATIVE_LISTCONTROL 0
#endif

#if USE_NATIVE_LISTCONTROL
#define LISTCTRL_BASE wxListCtrl
#include "wx/listctrl.h"
#else
// In wxMac-2.8.7, default wxListCtrl::RefreshItem() does not work
// so use traditional generic implementation.
// This has been fixed in wxMac-2.8.8, but the Mac native implementation:
//  - takes 3 times the CPU time as the Mac generic version.
//  - seems to always redraw entire control even if asked to refresh only one row.
//  - causes major flicker of progress bars, (probably due to full redraws.)
#define LISTCTRL_BASE wxGenericListCtrl
#include "wx/generic/listctrl.h"
#endif

#ifdef __WXMAC__
#include "macAccessiblity.h"
#endif

#include "BOINCBaseView.h"


class CBOINCBaseView;
class CDrawProgressBarEvent;

class CBOINCListCtrl : public LISTCTRL_BASE {
    DECLARE_DYNAMIC_CLASS(CBOINCListCtrl)

public:
    CBOINCListCtrl();
    CBOINCListCtrl(CBOINCBaseView* pView, wxWindowID iListWindowID, int iListWindowFlags);

    ~CBOINCListCtrl();

    virtual bool            OnSaveState(wxConfigBase* pConfig);
    virtual bool            OnRestoreState(wxConfigBase* pConfig);

    long                    GetFocusedItem() { return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED); }
    long                    GetFirstSelected() { return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); }
    long                    GetNextSelected(int i) { return GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); }
    void                    SelectRow(int row, bool setSelected);
    void                    AddPendingProgressBar(int row);
    void                    RefreshCell(int row, int col);
    
    bool                    m_bIsSingleSelection;

private:
    virtual void            OnClick(wxCommandEvent& event);

    virtual wxString        OnGetItemText(long item, long column) const;
    virtual int             OnGetItemImage(long item) const;
#if BASEVIEW_STRIPES
    virtual wxListItemAttr* OnGetItemAttr(long item) const;
#endif

    CBOINCBaseView*         m_pParentView;
    wxArrayInt              m_iRowsNeedingProgressBars;

#if USE_NATIVE_LISTCONTROL
public:
   void                     PostDrawProgressBarEvent();
private:
    void                    OnDrawProgressBar(CDrawProgressBarEvent& event);
    void                    DrawProgressBars(void);
    
    bool                    m_bProgressBarEventPending;

    DECLARE_EVENT_TABLE()
#else
 public:
    void                    DrawProgressBars(void);
    wxScrolledWindow*       GetMainWin(void) { return (wxScrolledWindow*) m_mainWin; }
    wxCoord                 GetHeaderHeight(void) { return m_headerHeight; }
#ifdef __WXMAC__
    void                    SetupMacAccessibilitySupport();
    void                    RemoveMacAccessibilitySupport();

    struct ListAccessData   accessibilityHandlerData;
    
    EventHandlerRef         m_pHeaderAccessibilityEventHandlerRef;
    EventHandlerRef         m_pBodyAccessibilityEventHandlerRef;
#endif
#endif
};

class CDrawProgressBarEvent : public wxEvent
{
public:
    CDrawProgressBarEvent(wxEventType evtType, CBOINCListCtrl* myCtrl)
        : wxEvent(-1, evtType)
        {
            SetEventObject(myCtrl);
        }

    virtual wxEvent *       Clone() const { return new CDrawProgressBarEvent(*this); }
};

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_DRAW_PROGRESSBAR, 12000 )
END_DECLARE_EVENT_TYPES()

#define EVT_DRAW_PROGRESSBAR(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_DRAW_PROGRESSBAR, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


// Define a custom event handler
class MyEvtHandler : public wxEvtHandler
{
public:
    MyEvtHandler(CBOINCListCtrl *theListControl) { m_listCtrl = theListControl; }
    void                    OnPaint(wxPaintEvent & event);

private:
    CBOINCListCtrl *        m_listCtrl;

    DECLARE_EVENT_TABLE()
};

#endif


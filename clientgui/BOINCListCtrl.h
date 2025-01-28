// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

#ifndef BOINC_BOINCLISTCTRL_H
#define BOINC_BOINCLISTCTRL_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCListCtrl.cpp"
#endif

#ifdef __WXMSW__
#define USE_NATIVE_LISTCONTROL 1
#else
#define USE_NATIVE_LISTCONTROL 0
#endif


// Virtual wxListCtrl does not reliably generate selection and
// deselection events, so we must check for these differently.
// We get more events than we need using EVT_LIST_CACHE_HINT,
// so testing on mouse events is more efficient, but it doesn't
// work on Windows.
#ifdef __WXMSW__
// On Windows, check for selection / deselection on EVT_LIST_CACHE_HINT.
#define USE_LIST_CACHE_HINT 1
#else
// On Mac & Linux, check for selection / deselection on EVT_LEFT_DOWN.
#define USE_LIST_CACHE_HINT 0
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

    void                    TokenizedStringToArray(wxString tokenized, const char * delimiters, wxArrayString* array);
    void                    SetListColumnOrder(wxArrayString& orderArray);
    void                    SetStandardColumnOrder();
    bool                    IsColumnOrderStandard();
    void                    SetDefaultColumnDisplay();

    long                    GetFocusedItem() { return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED); }
    long                    GetFirstSelected() { return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); }
    long                    GetNextSelected(int i) { return GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); }
    void                    SelectRow(int row, bool setSelected);
    void                    AddPendingProgressBar(int row);
    void                    RefreshCell(int row, int col);

private:
    virtual wxString        OnGetItemText(long item, long column) const;
    virtual int             OnGetItemImage(long item) const;
#if BASEVIEW_STRIPES
    virtual wxListItemAttr* OnGetItemAttr(long item) const;
#endif

    CBOINCBaseView*         m_pParentView;
    wxArrayInt              m_iRowsNeedingProgressBars;

#if ! USE_LIST_CACHE_HINT
    void                    OnMouseDown(wxMouseEvent& event);
#endif

#if USE_NATIVE_LISTCONTROL
public:
   void                     PostDrawProgressBarEvent();
private:
    void                    OnDrawProgressBar(CDrawProgressBarEvent& event);
    void                    DrawProgressBars(void);

    bool                    m_bProgressBarEventPending;
#else
 public:
    void                    DrawProgressBars(void);
    wxScrolledWindow*       GetMainWin(void) { return (wxScrolledWindow*) m_mainWin; }
    wxCoord                 GetHeaderHeight(void) { return ((wxWindow *)m_headerWin)->GetSize().y; }
    wxEvtHandler*           savedHandler;
#ifdef __WXMAC__
    void                    SetupMacAccessibilitySupport();
    void                    RemoveMacAccessibilitySupport();
    void                    OnSize( wxSizeEvent &event );

    void*                   m_fauxHeaderView;
    void*                   m_fauxBodyView;
#endif
#endif

    DECLARE_EVENT_TABLE()
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

class CCheckSelectionChangedEvent : public wxEvent
{
public:
    CCheckSelectionChangedEvent(wxEventType evtType, CBOINCListCtrl* myCtrl)
        : wxEvent(-1, evtType)
        {
            SetEventObject(myCtrl);
        }

    virtual wxEvent *       Clone() const { return new CCheckSelectionChangedEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_DRAW_PROGRESSBAR, 12000 )
DECLARE_EVENT_TYPE( wxEVT_CHECK_SELECTION_CHANGED, 12002 )
END_DECLARE_EVENT_TYPES()

#define EVT_DRAW_PROGRESSBAR(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_DRAW_PROGRESSBAR, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

#define EVT_CHECK_SELECTION_CHANGED(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_CHECK_SELECTION_CHANGED, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


// Define a custom event handler
class MyEvtHandler : public wxEvtHandler
{
    DECLARE_DYNAMIC_CLASS(MyEvtHandler)

public:
    MyEvtHandler();
    MyEvtHandler(CBOINCListCtrl *theListControl);
    void                    OnPaint(wxPaintEvent & event);

private:
    CBOINCListCtrl *        m_listCtrl;

#if !USE_NATIVE_LISTCONTROL
#ifdef __WXGTK__
    int                     m_view_startX;
#endif
#endif

    DECLARE_EVENT_TABLE()
};

#endif

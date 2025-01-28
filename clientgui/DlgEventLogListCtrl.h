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

#ifndef BOINC_DLGEVENTLOGLISTCTRL_H
#define BOINC_DLGEVENTLOGLISTCTRL_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgEventLogListCtrl.cpp"
#endif

#if (defined(__WXMAC__) || defined(__WXGTK__))
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

    ~CDlgEventLogListCtrl();

#ifdef __WXGTK__
    wxEvtHandler*           savedHandler;
    wxScrolledWindow*       GetMainWin(void) { return (wxScrolledWindow*) m_mainWin; }
#endif

private:

    virtual wxString        OnGetItemText(long item, long column) const;
    virtual int             OnGetItemImage(long item) const;
    virtual wxListItemAttr* OnGetItemAttr(long item) const;
    void                    OnMouseUp(wxMouseEvent& event);

    void                    OnShow( wxShowEvent& event );

    bool                    m_bIsSingleSelection;

    CDlgEventLog*           m_pParentView;

#ifdef __WXMAC__
    void                    SetupMacAccessibilitySupport();
    void                    RemoveMacAccessibilitySupport();
    void                    OnSize( wxSizeEvent &event );

    void*                   m_fauxHeaderView;
    void*                   m_fauxBodyView;
#endif
};

#ifdef __WXGTK__
// Define a custom event handler
class MyEvtLogEvtHandler : public wxEvtHandler
{
    DECLARE_DYNAMIC_CLASS(MyEvtLogEvtHandler)

public:
    MyEvtLogEvtHandler();
    MyEvtLogEvtHandler(wxGenericListCtrl *theListControl);
    void                    OnPaint(wxPaintEvent & event);

private:
    wxGenericListCtrl *     m_listCtrl;
    int                     m_view_startX;

    DECLARE_EVENT_TABLE()
};
#endif

#endif

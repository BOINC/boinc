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
#ifndef _NOTICELISTCTRL_H_
#define _NOTICELISTCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "NoticeListCtrl.cpp"
#endif

#include "BOINCHtmlLBox.h"

#if wxUSE_ACCESSIBILITY || defined(__WXMAC__)

#ifdef __WXMAC__
#define wxACC_SELF              0
#define wxACC_OK                noErr
#define wxAccStatus             OSStatus
#define wxACC_NOT_IMPLEMENTED   eventNotHandledErr
#define wxACC_FALSE             eventNotHandledErr
#define wxAccessible            wxObject
#endif


#ifndef __WXMAC__
class CNoticeListCtrlAccessible : public wxWindowAccessible
#else
class CNoticeListCtrlAccessible
#endif
{
public:

#ifndef __WXMAC__
    CNoticeListCtrlAccessible(wxWindow* win): wxWindowAccessible(win) {}
#else
    CNoticeListCtrlAccessible(wxWindow* win);
    virtual ~CNoticeListCtrlAccessible();
#endif

    virtual wxAccStatus GetName(int childId, wxString* name);
    virtual wxAccStatus HitTest(const wxPoint& pt, int* childId, wxAccessible** childObject);
    virtual wxAccStatus GetLocation(wxRect& rect, int elementId);
    virtual wxAccStatus GetChildCount(int* childCount);
    virtual wxAccStatus DoDefaultAction(int childId);
    virtual wxAccStatus GetDescription(int childId, wxString* description);
    wxString StripHTMLTags(wxString inBuf);

#ifndef __WXMAC__
    virtual wxAccStatus Navigate(wxNavDir navDir, int fromId, int* toId, wxAccessible** toObject);
    virtual wxAccStatus GetDefaultAction(int childId, wxString* actionName);
    virtual wxAccStatus GetRole(int childId, wxAccRole* role);
    virtual wxAccStatus GetState(int childId, long* state);
    virtual wxAccStatus Select(int childId, wxAccSelectionFlags selectFlags);
    virtual wxAccStatus GetSelections(wxVariant* selections);
#endif

#ifdef __WXMAC__
    wxWindow                *mp_win;
    HIViewRef               m_listView;
    EventHandlerRef         m_plistAccessibilityEventHandlerRef;
    
    wxWindow *GetWindow() { return mp_win; }
    void SetupMacAccessibilitySupport();
    void RemoveMacAccessibilitySupport();
#endif
};

#endif


/*!
 * CNoticeListCtrl class declaration
 */

class CNoticeListCtrl: public CBOINCHtmlListBox
{    
    DECLARE_DYNAMIC_CLASS( CNoticeListCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CNoticeListCtrl( );
    CNoticeListCtrl( wxWindow* parent );
    ~CNoticeListCtrl();

    /// Creation
    bool Create( wxWindow* parent );

////@begin CNoticeListCtrl event handler declarations

    void OnSelected( wxCommandEvent& event );
    void OnDClicked( wxCommandEvent& event );
    void OnClicked( wxHtmlCellEvent& event );
    void OnLinkClicked( wxHtmlLinkEvent& event );

////@end CNoticeListCtrl event handler declarations

    virtual wxString OnGetItem(size_t i) const;

    void Clear();
    bool UpdateUI();

    int GetItemHeight(size_t i) { return (int)OnGetItemHeight(i); }

    bool    m_bDisplayFetchingNotices;
    bool    m_bDisplayEmptyNotice;
private:
    bool    m_bComputerChanged;
#ifdef __WXMAC__
    CNoticeListCtrlAccessible*    m_accessible;
#endif
};


/*!
 * NoticeListCtrlEvent class declaration
 */

class NoticeListCtrlEvent : public wxNotifyEvent
{
public:
    NoticeListCtrlEvent( wxEventType evtType = wxEVT_NULL, int iSeqNo = 0, wxString strURL = wxEmptyString ) :
      wxNotifyEvent( evtType, wxID_ANY )
    {
        m_iSeqNo = iSeqNo;
        m_strURL = strURL;
    } 

    int GetSeqNo() { return m_iSeqNo; };
    wxString GetURL() { return m_strURL; };

    virtual wxNotifyEvent* Clone() const { return new NoticeListCtrlEvent(*this); }

private:
    int      m_iSeqNo;
    wxString m_strURL;

    DECLARE_DYNAMIC_CLASS(NoticeListCtrlEvent)
};

// ----------------------------------------------------------------------------
// macros for handling ProjectListCtrlEvent
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE( wxEVT_NOTICELIST_ITEM_CHANGE, 100000 )
    DECLARE_EVENT_TYPE( wxEVT_NOTICELIST_ITEM_DISPLAY, 100001 )
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*NoticeListCtrlEventFunction)(NoticeListCtrlEvent&);

#define NoticeListCtrlEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(NoticeListCtrlEventFunction, &func)

#define wx__DECLARE_NOTICELISTEVT(evt, fn) \
    wx__DECLARE_EVT0(wxEVT_NOTICELIST_ ## evt, NoticeListCtrlEventHandler(fn))

#define EVT_NOTICELIST_ITEM_CHANGE(fn) wx__DECLARE_NOTICELISTEVT(ITEM_CHANGE, fn)
#define EVT_NOTICELIST_ITEM_DISPLAY(fn) wx__DECLARE_NOTICELISTEVT(ITEM_DISPLAY, fn)


#endif // _NOTICELISTCTRL_H_

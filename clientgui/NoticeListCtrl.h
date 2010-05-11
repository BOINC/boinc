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


/*!
 * CNoticeListItem class declaration
 */

class CNoticeListItem: public wxObject
{    
    DECLARE_DYNAMIC_CLASS( CNoticeListItem )
public:
private:
};

#if wxUSE_ACCESSIBILITY || defined(__WXMAC__)

#ifdef __WXMAC__

#define wxACC_SELF              0
#define wxACC_OK                noErr
#define wxAccStatus             OSStatus
#define wxACC_NOT_IMPLEMENTED   eventNotHandledErr
#define wxACC_FALSE             eventNotHandledErr
#define wxAccessible            wxObject

class CProjectListCtrlAccessible
#else
class CProjectListCtrlAccessible: public wxWindowAccessible
#endif
{
public:

#ifdef __WXMAC__
    CProjectListCtrlAccessible(wxWindow* win);
    virtual ~CProjectListCtrlAccessible();
#else
    CProjectListCtrlAccessible(wxWindow* win): wxWindowAccessible(win) {}
#endif

    virtual wxAccStatus GetName(int childId, wxString* name);
    virtual wxAccStatus HitTest(const wxPoint& pt, int* childId, wxAccessible** childObject);
    virtual wxAccStatus GetLocation(wxRect& rect, int elementId);
    virtual wxAccStatus GetChildCount(int* childCount);
    virtual wxAccStatus DoDefaultAction(int childId);
    virtual wxAccStatus GetDescription(int childId, wxString* description);
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
 * CProjectListCtrl class declaration
 */

class CProjectListCtrl: public wxHtmlListBox
{    
    DECLARE_DYNAMIC_CLASS( CProjectListCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectListCtrl( );

    CProjectListCtrl( wxWindow* parent );
#ifdef __WXMAC__
    ~CProjectListCtrl();
#endif

    /// Creation
    bool Create( wxWindow* parent );

////@begin CProjectListCtrl event handler declarations

    void OnSelected( wxCommandEvent& event );
    void OnClicked( wxHtmlCellEvent& event );
    void OnDClicked( wxCommandEvent& event );
    void OnLinkClicked( wxHtmlLinkEvent& event );
    void OnHover( wxHtmlCellEvent& event );

////@end CProjectListCtrl event handler declarations

    virtual wxString OnGetItem(size_t i) const;

    /// Methods
    bool Append(
        wxString strURL,
        wxString strTitle,
        wxString strImage,
        wxString strDescription,
        bool bNvidiaGPUSupported,
        bool bATIGPUSupported,
        bool bMulticoreSupported,
        bool bSupported
    );

    CProjectListItem* GetItem( 
        int iIndex
    );

    wxCoord GetTotalClientHeight();

private:
    std::vector<CProjectListItem*> m_Items;

#ifdef __WXMAC__
    CProjectListCtrlAccessible*    m_accessible;
#endif
};


/*!
 * ProjectListCtrlEvent class declaration
 */

class ProjectListCtrlEvent : public wxNotifyEvent
{
public:
    ProjectListCtrlEvent( wxEventType evtType = wxEVT_NULL, wxString strName = wxEmptyString, wxString strURL = wxEmptyString, bool bSupported = false ) :
      wxNotifyEvent( evtType, wxID_ANY )
    {
        m_strName = strName;
        m_strURL = strURL;
        m_bSupported = bSupported;
    } 

    wxString GetName() { return m_strName; };
    wxString GetURL() { return m_strURL; };
    bool IsSupported() { return m_bSupported; };

    virtual wxNotifyEvent* Clone() const { return new ProjectListCtrlEvent(*this); }

private:
    wxString m_strName;
    wxString m_strURL;
    bool m_bSupported;

    DECLARE_DYNAMIC_CLASS(ProjectListCtrlEvent)
};

// ----------------------------------------------------------------------------
// macros for handling ProjectListCtrlEvent
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_CHANGE, 100000 )
    DECLARE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_DISPLAY, 100001 )
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*ProjectListCtrlEventFunction)(ProjectListCtrlEvent&);

#define ProjectListCtrlEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(ProjectListCtrlEventFunction, &func)

#define wx__DECLARE_PROJECTLISTEVT(evt, fn) \
    wx__DECLARE_EVT0(wxEVT_PROJECTLIST_ ## evt, ProjectListCtrlEventHandler(fn))

#define EVT_PROJECTLIST_ITEM_CHANGE(fn) wx__DECLARE_PROJECTLISTEVT(ITEM_CHANGE, fn)
#define EVT_PROJECTLIST_ITEM_DISPLAY(fn) wx__DECLARE_PROJECTLISTEVT(ITEM_DISPLAY, fn)


#endif // _NOTICELISTCTRL_H_

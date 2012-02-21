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

#ifndef _BOINCBASEVIEW_H_
#define _BOINCBASEVIEW_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCBaseView.cpp"
#endif

#define BASEVIEW_STRIPES 1
#define BASEVIEW_RULES 1

#define DEFAULT_TASK_FLAGS             wxTAB_TRAVERSAL | wxADJUST_MINSIZE | wxFULL_REPAINT_ON_RESIZE

#if BASEVIEW_RULES
#define DEFAULT_LIST_SINGLE_SEL_FLAGS  wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_SINGLE_SEL
#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES
#else
#define DEFAULT_LIST_SINGLE_SEL_FLAGS  wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL
#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL
#endif


class CBOINCTaskCtrl;
class CBOINCListCtrl;
struct PROJECT;


class CTaskItem : wxObject {
public:
	CTaskItem();
	CTaskItem( wxString strName, wxString strDescription, wxInt32 iEventID ) :
		m_strName(strName), m_strDescription(strDescription), m_iEventID(iEventID),
        m_pButton(NULL), m_strWebSiteLink(wxT("")) {};
	CTaskItem( wxString strName, wxString strDescription, wxString strWebSiteLink, wxInt32 iEventID ) :
		m_strName(strName), m_strDescription(strDescription), m_iEventID(iEventID),  
        m_pButton(NULL), m_strWebSiteLink(strWebSiteLink) {};
    ~CTaskItem() {};

    wxString                m_strName;
    wxString                m_strNameEllipsed;
    wxString                m_strDescription;
    wxInt32                 m_iEventID;

    wxButton*               m_pButton;
    wxString                m_strWebSiteLink;
};


class CTaskItemGroup : wxObject {
public:
	CTaskItemGroup();
	CTaskItemGroup( wxString strName ) :
            m_strName(strName), m_pStaticBox(NULL), m_pStaticBoxSizer(NULL) {
            m_Tasks.clear();
#ifdef __WXMAC__
            m_pTaskGroupAccessibilityEventHandlerRef = NULL;
#endif
        };
    ~CTaskItemGroup() {
#ifdef __WXMAC__
        RemoveMacAccessibilitySupport();
#endif
    };
    wxButton* button(int i) {return m_Tasks[i]->m_pButton;}

    wxString                m_strName;

    wxStaticBox*            m_pStaticBox;
    wxStaticBoxSizer*       m_pStaticBoxSizer;

    std::vector<CTaskItem*> m_Tasks;

#ifdef __WXMAC__
    void                    SetupMacAccessibilitySupport();
    void                    RemoveMacAccessibilitySupport();
    
private:
    EventHandlerRef         m_pTaskGroupAccessibilityEventHandlerRef;
#endif
};

typedef bool     (*ListSortCompareFunc)(int, int);


class CBOINCBaseView : public wxPanel {
    DECLARE_DYNAMIC_CLASS( CBOINCBaseView )

public:

    CBOINCBaseView();
    CBOINCBaseView(
        wxNotebook* pNotebook
    );
    CBOINCBaseView(
        wxNotebook* pNotebook,
        wxWindowID iTaskWindowID,
        int iTaskWindowFlags,
        wxWindowID iListWindowID,
        int iListWindowFlags
    );

    ~CBOINCBaseView();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();
    virtual const int       GetViewRefreshRate();
    virtual const int       GetViewCurrentViewPage();

    virtual wxString        GetKeyValue1(int iRowIndex);
    virtual wxString        GetKeyValue2(int iRowIndex);
    virtual int             FindRowIndexByKeyValues(wxString& key1, wxString& key2);

    bool                    FireOnSaveState( wxConfigBase* pConfig );
    bool                    FireOnRestoreState( wxConfigBase* pConfig );

    virtual int             GetListRowCount();
    void                    FireOnListRender( wxTimerEvent& event );
    void                    FireOnListSelected( wxListEvent& event );
    void                    FireOnListDeselected( wxListEvent& event );
    wxString                FireOnListGetItemText( long item, long column ) const;
    int                     FireOnListGetItemImage( long item ) const;
#if BASEVIEW_STRIPES
    wxListItemAttr*         FireOnListGetItemAttr( long item ) const;
#endif

    int                     GetProgressColumn() { return m_iProgressColumn; }
    virtual double          GetProgressValue(long item);
    virtual wxString        GetProgressText( long item);

    void                    InitSort();
    
	void                    SaveSelections();
	void                    RestoreSelections();
	void                    ClearSavedSelections();
	void                    ClearSelections();
    void                    RefreshTaskPane();

#ifdef __WXMAC__
    CBOINCListCtrl*         GetListCtrl() { return m_pListPane; }
#endif    
 
    std::vector<CTaskItemGroup*> m_TaskGroups;

    int                     m_iSortColumn;
    bool                    m_bReverseSort;

private:

	wxArrayString           m_arrSelectedKeys1;     //array for remembering the current selected rows by primary key column value
	wxArrayString           m_arrSelectedKeys2;     //array for remembering the current selected rows by secondary key column value

protected:

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            OnListRender( wxTimerEvent& event );
    virtual void            OnListSelected( wxListEvent& event );
    virtual void            OnListDeselected( wxListEvent& event );
    virtual void            OnCacheHint(wxListEvent& event);
    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual int             OnListGetItemImage( long item ) const;

    void                    OnColClick(wxListEvent& event);
    
    virtual int             GetDocCount();
    virtual wxString        OnDocGetItemImage( long item ) const;
    virtual wxString        OnDocGetItemAttr( long item ) const;

    virtual int             AddCacheElement();
    virtual int             EmptyCache();
    virtual int             GetCacheCount();
    virtual int             RemoveCacheElement();
    virtual int             SynchronizeCache();
    virtual bool            SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex);
    void                    sortData();

    virtual void            EmptyTasks();

    virtual void            PreUpdateSelection();
    virtual void            UpdateSelection();
    virtual void            PostUpdateSelection();

    virtual void            UpdateWebsiteSelection(long lControlGroup, PROJECT* project);


    bool                    _IsSelectionManagementNeeded();
    virtual bool            IsSelectionManagementNeeded();

    bool                    _EnsureLastItemVisible();
    virtual bool            EnsureLastItemVisible();

    static  void            append_to_status(wxString& existing, const wxChar* additional);
    static  wxString        HtmlEntityEncode(wxString strRaw);
    static  wxString        HtmlEntityDecode(wxString strRaw);

#if BASEVIEW_STRIPES
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    wxListItemAttr*         m_pWhiteBackgroundAttr;
    wxListItemAttr*         m_pGrayBackgroundAttr;
#endif

    bool                    m_bProcessingTaskRenderEvent;
    bool                    m_bProcessingListRenderEvent;

    bool                    m_bForceUpdateSelection;
    bool                    m_bIgnoreUIEvents;
    bool                    m_bNeedSort;
    
    int                     m_iProgressColumn;

    wxImageList *           m_SortArrows;
    ListSortCompareFunc     m_funcSortCompare;
    wxArrayInt              m_iSortedIndexes;

    CBOINCTaskCtrl*         m_pTaskPane;
    CBOINCListCtrl*         m_pListPane;
};


#endif


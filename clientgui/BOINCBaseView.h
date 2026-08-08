// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#ifndef BOINC_BOINCBASEVIEW_H
#define BOINC_BOINCBASEVIEW_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCBaseView.cpp"
#endif


#define DEFAULT_TASK_FLAGS             wxTAB_TRAVERSAL | wxFULL_REPAINT_ON_RESIZE
#define DEFAULT_LIST_FLAGS             wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES

class CBOINCTaskCtrl;
class CBOINCListCtrl;
class CCheckSelectionChangedEvent;
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
        };
    ~CTaskItemGroup() {};
    wxButton* button(int i) {return m_Tasks[i]->m_pButton;}

    wxString                m_strName;

    wxStaticBox*            m_pStaticBox;
    wxStaticBoxSizer*       m_pStaticBoxSizer;

    std::vector<CTaskItem*> m_Tasks;
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
    virtual int             GetViewRefreshRate();
    virtual int             GetViewCurrentViewPage();

    virtual wxString        GetKeyValue1(int iRowIndex);
    virtual wxString        GetKeyValue2(int iRowIndex);
    virtual int             FindRowIndexByKeyValues(wxString& key1, wxString& key2);

    bool                    FireOnSaveState( wxConfigBase* pConfig );
    bool                    FireOnRestoreState( wxConfigBase* pConfig );

    virtual int             GetListRowCount();
    void                    FireOnListRender();
    void                    FireOnListSelected( wxListEvent& event );
    void                    FireOnListDeselected( wxListEvent& event );
    wxString                FireOnListGetItemText( long item, long column ) const;
    int                     FireOnListGetItemImage( long item ) const;

    int                     GetProgressColumn() { return m_iProgressColumn; }
    void                    SetProgressColumn(int col) { m_iProgressColumn = col; }
    virtual double          GetProgressValue(long item);
    virtual wxString        GetProgressText( long item);
    virtual void            AppendColumn(int columnID);

    void                    InitSort();
    void                    SetSortColumn(int newSortColIndex);
	void                    SaveSelections();
	void                    RestoreSelections();
	void                    ClearSavedSelections();
	void                    ClearSelections();
    void                    RefreshTaskPane();

    CBOINCListCtrl*         GetListCtrl() { return m_pListPane; }

#ifdef __WXMAC__
    void                    OnKeyPressed(wxKeyEvent &event);
#endif

    std::vector<CTaskItemGroup*> m_TaskGroups;

    int                     m_iSortColumnID;  // ColumnID of sort column
    bool                    m_bReverseSort;
    wxArrayString*          m_aStdColNameOrder;
    wxArrayInt              m_iStdColWidthOrder;
    wxArrayInt              m_iColumnIndexToColumnID;
    wxArrayInt              m_iColumnIDToColumnIndex;
    int*                    m_iDefaultShownColumns;
    int                     m_iNumDefaultShownColumns;


private:

	wxArrayString           m_arrSelectedKeys1;     //array for remembering the current selected rows by primary key column value
	wxArrayString           m_arrSelectedKeys2;     //array for remembering the current selected rows by secondary key column value

protected:

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            OnListRender();
    virtual void            OnListSelected( wxListEvent& event );
    virtual void            OnListDeselected( wxListEvent& event );
    virtual void            OnCacheHint(wxListEvent& event);
    virtual void            OnCheckSelectionChanged(CCheckSelectionChangedEvent& event);
    virtual void            CheckSelectionChanged();
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

    static  void            append_to_status(wxString& existing, const wxString& additional);
    static  wxString        HtmlEntityEncode(wxString strRaw);
    static  wxString        HtmlEntityDecode(wxString strRaw);

    bool                    m_bProcessingTaskRenderEvent;
    bool                    m_bProcessingListRenderEvent;

    bool                    m_bForceUpdateSelection;
    bool                    m_bIgnoreUIEvents;
    bool                    m_bNeedSort;

    int                     m_iPreviousSelectionCount;
    long                    m_lPreviousFirstSelection;
    int                     m_iProgressColumn;

    wxImageList *           m_SortArrows;
    ListSortCompareFunc     m_funcSortCompare;
    wxArrayInt              m_iSortedIndexes;

    CBOINCTaskCtrl*         m_pTaskPane;
    CBOINCListCtrl*         m_pListPane;
};

#endif

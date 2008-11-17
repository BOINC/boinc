// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _BOINCBASEVIEW_H_
#define _BOINCBASEVIEW_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCBaseView.cpp"
#endif

#define DEFAULT_TASK_FLAGS             wxTAB_TRAVERSAL | wxADJUST_MINSIZE
#define DEFAULT_LIST_SINGLE_SEL_FLAGS  wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL
#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL


class CBOINCTaskCtrl;
class CBOINCListCtrl;
class PROJECT;


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
    wxString                m_strDescription;
    wxInt32                 m_iEventID;

	wxButton*				m_pButton;
    wxString                m_strWebSiteLink;
};


class CTaskItemGroup : wxObject {
public:
	CTaskItemGroup();
	CTaskItemGroup( wxString strName ) :
		m_strName(strName), m_pStaticBox(NULL), m_pStaticBoxSizer(NULL) { m_Tasks.clear(); };
    ~CTaskItemGroup() {};
    wxButton* button(int i) {return m_Tasks[i]->m_pButton;}

    wxString                m_strName;

    wxStaticBox*            m_pStaticBox;
    wxStaticBoxSizer*       m_pStaticBoxSizer;

	std::vector<CTaskItem*> m_Tasks;
};


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

    bool                    FireOnSaveState( wxConfigBase* pConfig );
    bool                    FireOnRestoreState( wxConfigBase* pConfig );

    virtual int             GetListRowCount();
    void                    FireOnListRender( wxTimerEvent& event );
    void                    FireOnListSelected( wxListEvent& event );
    void                    FireOnListDeselected( wxListEvent& event );
    wxString                FireOnListGetItemText( long item, long column ) const;
    int                     FireOnListGetItemImage( long item ) const;
    wxListItemAttr*         FireOnListGetItemAttr( long item ) const;

    std::vector<CTaskItemGroup*> m_TaskGroups;

protected:

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            OnListRender( wxTimerEvent& event );
    virtual void            OnListSelected( wxListEvent& event );
    virtual void            OnListDeselected( wxListEvent& event );
    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual int             OnListGetItemImage( long item ) const;
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    virtual void            OnGridSelectCell( wxGridEvent& event );
    virtual void            OnGridSelectRange( wxGridRangeSelectEvent& event );

    virtual int             GetDocCount();
    virtual wxString        OnDocGetItemText( long item, long column ) const;
    virtual wxString        OnDocGetItemImage( long item ) const;
    virtual wxString        OnDocGetItemAttr( long item ) const;

    virtual int             AddCacheElement();
    virtual int             EmptyCache();
    virtual int             GetCacheCount();
    virtual int             RemoveCacheElement();
    virtual int             SyncronizeCache();
    virtual int             UpdateCache( long item, long column, wxString& strNewData );

    virtual void            EmptyTasks();

    virtual void            PreUpdateSelection();
    virtual void            UpdateSelection();
    virtual void            PostUpdateSelection();

    virtual void            UpdateWebsiteSelection(long lControlGroup, PROJECT* project);

    bool                    _EnsureLastItemVisible();
    virtual bool            EnsureLastItemVisible();

    static  void            append_to_status(wxString& existing, const wxChar* additional);
	static  wxString        HtmlEntityEncode(wxString strRaw);
	static  wxString        HtmlEntityDecode(wxString strRaw);

    bool                    m_bProcessingTaskRenderEvent;
    bool                    m_bProcessingListRenderEvent;

    bool                    m_bForceUpdateSelection;
    bool                    m_bIgnoreUIEvents;

    CBOINCTaskCtrl*         m_pTaskPane;
    CBOINCListCtrl*         m_pListPane;
};


#endif


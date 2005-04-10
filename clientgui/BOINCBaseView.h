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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _BOINCBASEVIEW_H_
#define _BOINCBASEVIEW_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCBaseView.cpp"
#endif

#define DEFAULT_HTML_FLAGS             wxHW_SCROLLBAR_AUTO | wxHSCROLL | wxVSCROLL
#define DEFAULT_LIST_SINGLE_SEL_FLAGS  wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL
#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL


class CBOINCTaskCtrl;
class CBOINCListCtrl;

class CTaskItem : wxObject {
public:
    wxString    m_strTaskName;
    wxString    m_strTaskDescription;
    wxString    m_strLink;
    bool        m_bIsHidden;
    bool        m_bIsHovering;
    bool        m_bIsClicked;
};

class CBOINCBaseView : public wxPanel {
    DECLARE_DYNAMIC_CLASS( CBOINCBaseView )

public:

    CBOINCBaseView();
    CBOINCBaseView(
        wxNotebook* pNotebook,
        wxWindowID iHtmlWindowID,
        int iHtmlWindowFlags,
        wxWindowID iListWindowID,
        int iListWindowFlags,
		bool donothing=false
    );

    ~CBOINCBaseView();

    virtual wxString        GetViewName();
    virtual const char**    GetViewIcon();
    virtual int         GetListRowCount();

    void                    FireOnListRender( wxTimerEvent& event );
    bool                    FireOnSaveState( wxConfigBase* pConfig );
    bool                    FireOnRestoreState( wxConfigBase* pConfig );
    void                    FireOnChar( wxKeyEvent& event );
    void                    FireOnListSelected( wxListEvent& event );
    void                    FireOnListDeselected( wxListEvent& event );
    wxString                FireOnListGetItemText( long item, long column ) const;
    int                     FireOnListGetItemImage( long item ) const;
    wxListItemAttr*         FireOnListGetItemAttr( long item ) const;
    void                    FireOnTaskLinkClicked( const wxHtmlLinkInfo& link );
    void                    FireOnTaskCellClicked( wxHtmlCell* cell, wxCoord x, wxCoord y, const wxMouseEvent& event );
    void                    FireOnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y );

    virtual void            UpdateTaskPane();

protected:

    virtual int         GetDocCount();

    virtual void            OnListRender( wxTimerEvent& event );

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            OnChar( wxKeyEvent& event );
    virtual void            OnListSelected( wxListEvent& event );
    virtual void            OnListDeselected( wxListEvent& event );

    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual int             OnListGetItemImage( long item ) const;
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    virtual wxString        OnDocGetItemText( long item, long column ) const;
    virtual wxString        OnDocGetItemImage( long item ) const;
    virtual wxString        OnDocGetItemAttr( long item ) const;

    virtual void            OnTaskLinkClicked( const wxHtmlLinkInfo& link );
    virtual void            OnTaskCellClicked( wxHtmlCell* cell, wxCoord x, wxCoord y, const wxMouseEvent& event );
    virtual void            OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y );

    wxString                GetCurrentQuickTip();
    wxString                GetCurrentQuickTipText();
    void                    SetCurrentQuickTip( const wxString& strQuickTip, const wxString& strQuickTipText );

    virtual int         AddCacheElement();
    virtual int         EmptyCache();
    virtual int         GetCacheCount();
    virtual int         RemoveCacheElement();
    virtual int         SyncronizeCache();
    virtual int         UpdateCache( long item, long column, wxString& strNewData );

    bool                    _EnsureLastItemVisible();
    virtual bool            EnsureLastItemVisible();

    virtual bool            UpdateQuickTip( const wxString& strCurrentLink, const wxString& strQuickTip, const wxString& strQuickTipText );
    virtual void            UpdateSelection();

    std::vector<CTaskItem*> m_DefinedTasks;

    bool                    m_bProcessingTaskRenderEvent;
    bool                    m_bProcessingListRenderEvent;

    bool                    m_bItemSelected;

    wxString                m_strQuickTip;
    wxString                m_strQuickTipText;

    CBOINCTaskCtrl*         m_pTaskPane;
    CBOINCListCtrl*         m_pListPane;

    //
    // Globalization/Localization
    //
    wxString                LINK_DEFAULT;

};


#endif


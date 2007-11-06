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

#ifndef _TASKVIEWBASE_H_
#define _TASKVIEWBASE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "TaskViewBase.cpp"
#endif

#include "BOINCBaseView.h"

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

// Base for all views including a task pane.
class CTaskViewBase : public CBOINCBaseView {
    DECLARE_DYNAMIC_CLASS( CTaskViewBase )

public:

    CTaskViewBase();
    CTaskViewBase(wxNotebook* pNotebook);

    ~CTaskViewBase();

    bool                    FireOnSaveState( wxConfigBase* pConfig );
    bool                    FireOnRestoreState( wxConfigBase* pConfig );

    std::vector<CTaskItemGroup*> m_TaskGroups;

protected:

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );


    virtual void            EmptyTasks();

    virtual void            PostUpdateSelection();

    virtual void            UpdateWebsiteSelection(long lControlGroup, PROJECT* project);

    virtual void            DemandLoadView(
                                wxWindowID iTaskWindowID,
                                int iTaskWindowFlags,
                                wxWindowID iListWindowID,
                                int iListWindowFlags
                            );

    CBOINCTaskCtrl*         m_pTaskPane;

    bool                    m_bTaskPaneVisible;
};


#endif



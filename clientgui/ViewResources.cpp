// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//
// $Log$
// Revision 1.5  2004/10/05 02:55:26  rwalton
// *** empty log message ***
//
// Revision 1.4  2004/09/25 21:33:23  rwalton
// *** empty log message ***
//
// Revision 1.3  2004/09/24 22:19:00  rwalton
// *** empty log message ***
//
// Revision 1.2  2004/09/24 02:01:53  rwalton
// *** empty log message ***
//
// Revision 1.1  2004/09/21 01:26:25  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewResources.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewResources.h"
#include "Events.h"

#include "res/usage.xpm"
#include "res/task.xpm"
#include "res/tips.xpm"


#define VIEW_HEADER                 "resources"

#define SECTION_TASK                VIEW_HEADER "task"
#define SECTION_TIPS                VIEW_HEADER "tips"

#define BITMAP_RESOURCES            VIEW_HEADER ".xpm"
#define BITMAP_TASKHEADER           SECTION_TASK ".xpm"
#define BITMAP_TIPSHEADER           SECTION_TIPS ".xpm"

#define LINK_DEFAULT                "default"

#define COLUMN_PROJECT              0
#define COLUMN_DISKSPACE            1


IMPLEMENT_DYNAMIC_CLASS(CViewResources, CBOINCBaseView)


CViewResources::CViewResources()
{
}


CViewResources::CViewResources(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_RESOURCEUTILIZATIONVIEW, ID_LIST_RESOURCEUTILIZATIONVIEW)
{
    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxBitmap bmpResources(usage_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpResources.SetMask(new wxMask(bmpResources, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(wxT(BITMAP_RESOURCES), bmpResources, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TASKHEADER), bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(wxT(BITMAP_TIPSHEADER), bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_DISKSPACE, _("Disk Space"), wxLIST_FORMAT_LEFT, -1);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        wxT(LINK_DEFAULT), 
        _("No available options currently defined.")
    );

    UpdateSelection();
}


CViewResources::~CViewResources()
{
}


wxString CViewResources::GetViewName()
{
    return wxString(_("Disk"));
}


char** CViewResources::GetViewIcon()
{
    return usage_xpm;
}


void CViewResources::OnTaskRender(wxTimerEvent &event)
{
    if (!m_bProcessingTaskRenderEvent)
    {
        m_bProcessingTaskRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        if ( ( 0 == m_pListPane->GetSelectedItemCount() ) && m_bItemSelected )
        {
            UpdateSelection();
        }

        m_bProcessingTaskRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


void CViewResources::OnListRender(wxTimerEvent &event)
{
    if (!m_bProcessingListRenderEvent)
    {
        m_bProcessingListRenderEvent = true;

        wxASSERT(NULL != m_pListPane);

        //wxInt32 iCount = wxGetApp().GetDocument()->GetMessageCount();
        //if ( iCount != m_iCount )
        //{
        //    m_iCount = iCount;
        //    m_pListPane->SetItemCount(iCount);
        //}
        //else
        //{
        //    m_pListPane->RefreshItems(m_iCacheFrom, m_iCacheTo);
        //}

        m_bProcessingListRenderEvent = false;
    }
    else
    {
        event.Skip();
    }
}


void CViewResources::OnListSelected ( wxListEvent& event )
{
    UpdateSelection();
    event.Skip();
}


void CViewResources::OnListDeselected ( wxListEvent& event )
{
    UpdateSelection();
    event.Skip();
}


wxString CViewResources::OnListGetItemText( long item, long column ) const
{
    wxString strBuffer;
    return strBuffer;
}


void CViewResources::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxString strMessage;

    if ( link.GetHref() == wxT(SECTION_TASK) )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;

    if ( link.GetHref() == wxT(SECTION_TIPS) )
        m_bTipsHeaderHidden ? m_bTipsHeaderHidden = false : m_bTipsHeaderHidden = true;


    UpdateSelection();
}


void CViewResources::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    if ( NULL != cell->GetLink() )
    {
        bool        bUpdateSelection = false;
        wxString    strLink;

        strLink = cell->GetLink()->GetHref();

        if      ( wxT("test") == strLink )
        {
            if  ( wxT("test") != GetCurrentQuickTip() )
            {
                SetCurrentQuickTip(
                    wxT("test"), 
                    wxT("test")
                );

                bUpdateSelection = true;
            }
        }
        else
        {
            if ( 0 == m_pListPane->GetSelectedItemCount() )
            {
                if  ( wxT(LINK_DEFAULT) != GetCurrentQuickTip() )
                {
                    SetCurrentQuickTip(
                        wxT(LINK_DEFAULT), 
                        _("No avaiable options currently defined.")
                    );

                    bUpdateSelection = true;
                }
            }
        }

        if ( bUpdateSelection )
        {
            UpdateSelection();
        }
    }
}


void CViewResources::UpdateSelection()
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if ( 0 == m_pListPane->GetSelectedItemCount() )
    {
        m_bTaskHeaderHidden = true;

        m_bItemSelected = false;
    }
    else
    {
        m_bTaskHeaderHidden = true;

        m_bItemSelected = true;
    }
    UpdateTaskPane();
}


void CViewResources::UpdateTaskPane()
{
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( wxT(SECTION_TASK), wxT(BITMAP_TASKHEADER), m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip(wxT(SECTION_TIPS), wxT(BITMAP_TIPSHEADER), GetCurrentQuickTipText(), m_bTipsHeaderHidden);

    m_pTaskPane->EndTaskPage();
}


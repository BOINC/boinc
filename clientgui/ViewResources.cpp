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


#define VIEW_HEADER                 wxT("resources")

#define SECTION_TASK                wxT(VIEW_HEADER "task")
#define SECTION_TIPS                wxT(VIEW_HEADER "tips")

#define BITMAP_RESOURCES            wxT(VIEW_HEADER ".xpm")
#define BITMAP_TASKHEADER           wxT(SECTION_TASK ".xpm")
#define BITMAP_TIPSHEADER           wxT(SECTION_TIPS ".xpm")

#define COLUMN_PROJECT              0
#define COLUMN_DISKSPACE            1


const wxString LINKDESC_DEFAULT         = 
     _("No available options currently defined.");


IMPLEMENT_DYNAMIC_CLASS(CViewResources, CBOINCBaseView)


CViewResources::CViewResources()
{
}


CViewResources::CViewResources(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_RESOURCEUTILIZATIONVIEW, ID_LIST_RESOURCEUTILIZATIONVIEW)
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxBitmap bmpResources(usage_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpResources.SetMask(new wxMask(bmpResources, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(wxT(BITMAP_RESOURCES), bmpResources, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, -1);
    m_pListPane->InsertColumn(COLUMN_DISKSPACE, _("Disk Space"), wxLIST_FORMAT_LEFT, -1);

    m_bTipsHeaderHidden = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
        LINKDESC_DEFAULT
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


wxInt32 CViewResources::GetListRowCount()
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetResourceCount();
}


wxString CViewResources::OnListGetItemText( long item, long column ) const
{
    wxString       strBuffer = wxEmptyString;
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    switch(column)
    {
        case COLUMN_PROJECT:
            if (item == m_iCacheFrom) pDoc->CachedStateLock();
            FormatProjectName( item, strBuffer );
            break;
        case COLUMN_DISKSPACE:
            FormatDiskSpace( item, strBuffer );
            if (item == m_iCacheTo) pDoc->CachedStateUnlock();
            break;
    }
    return strBuffer;
}


void CViewResources::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxString strMessage;

    if ( link.GetHref() == wxT(SECTION_TASK) )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;
    else if ( link.GetHref() == wxT(SECTION_TIPS) )
        m_bTipsHeaderHidden ? m_bTipsHeaderHidden = false : m_bTipsHeaderHidden = true;

    UpdateSelection();
    m_pListPane->Refresh();
}


void CViewResources::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    if ( NULL != cell->GetLink() )
    {
        bool        bUpdateSelection = false;
        wxString    strLink;

        strLink = cell->GetLink()->GetHref();

        if      ( UpdateQuickTip( strLink, wxT("test"), wxT("test") ) )
            bUpdateSelection = true;
        else
        {
            if ( 0 == m_pListPane->GetSelectedItemCount() )
            {
                if  ( wxT(LINK_DEFAULT) != GetCurrentQuickTip() )
                {
                    SetCurrentQuickTip(
                        LINK_DEFAULT, 
                        LINKDESC_DEFAULT
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

        if ( m_bItemSelected )
        {
            SetCurrentQuickTip(
                LINK_DEFAULT, 
                wxT("")
            );
        }
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

    m_pTaskPane->BeginTaskSection( SECTION_TASK, BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip( SECTION_TIPS, BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden);

    m_pTaskPane->EndTaskPage();
}


wxInt32 CViewResources::FormatProjectName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetResourceProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewResources::FormatDiskSpace( wxInt32 item, wxString& strBuffer ) const
{
    float          fBuffer = 0;
    double         xTera = 1099511627776.0;
    double         xGiga = 1073741824.0;
    double         xMega = 1048576.0;
    double         xKilo = 1024.0;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetResourceDiskspace( item, fBuffer );

    if      ( fBuffer >= xTera )
    {
        strBuffer.Printf( wxT("%0.2f TB"), fBuffer/xTera);
    }
    else if ( fBuffer >= xGiga )
    {
        strBuffer.Printf( wxT("%0.2f GB"), fBuffer/xGiga);
    }
    else if ( fBuffer >= xMega )
    {
        strBuffer.Printf( wxT("%0.2f MB"), fBuffer/xMega);
    }
    else if ( fBuffer >= xKilo )
    {
        strBuffer.Printf( wxT("%0.2f KB"), fBuffer/xKilo);
    }
    else
    {
        strBuffer.Printf( wxT("%0.0f bytes"), fBuffer);
    }

    return 0;
}


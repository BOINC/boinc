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
// Revision 1.1  2004/09/21 01:26:24  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCTaskCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCTaskCtrl.h"
#include "ViewProjects.h"
#include "ViewWork.h"
#include "ViewTransfers.h"
#include "ViewMessages.h"
#include "ViewResources.h"

#include "res/visibleheader.xpm"
#include "res/hiddenheader.xpm"


IMPLEMENT_DYNAMIC_CLASS(CBOINCTaskCtrl, wxHtmlWindow)


CBOINCTaskCtrl::CBOINCTaskCtrl()
{
    wxLogTrace("CBOINCTaskCtrl::CBOINCTaskCtrl - Function Begining");
    wxLogTrace("CBOINCTaskCtrl::CBOINCTaskCtrl - Function Ending");
}


CBOINCTaskCtrl::CBOINCTaskCtrl(CBOINCBaseView* pView, wxWindowID iHtmlWindowID) :
    wxHtmlWindow( pView, iHtmlWindowID, wxDefaultPosition, wxSize(225, -1), wxHW_SCROLLBAR_AUTO|wxHSCROLL|wxVSCROLL )
{
    wxLogTrace("CBOINCListCtrl::CBOINCListCtrl - Function Begining");

    m_pParentView = pView;

    wxLogTrace("CBOINCListCtrl::CBOINCListCtrl - Function Ending");
}


CBOINCTaskCtrl::~CBOINCTaskCtrl()
{
    wxLogTrace("CBOINCTaskCtrl::~CBOINCTaskCtrl - Function Begining");
    wxLogTrace("CBOINCTaskCtrl::~CBOINCTaskCtrl - Function Ending");
}


void CBOINCTaskCtrl::BeginTaskPage()
{
    wxLogTrace("CBOINCTaskCtrl::BeginTaskPage - Function Begining");

    m_strTaskPage.Clear();

    m_strTaskPage += "<html><body bgcolor=\"#0080FF\">";

    wxLogTrace("CBOINCTaskCtrl::BeginTaskPage - Function Ending");
}


void CBOINCTaskCtrl::BeginTaskSection( const wxString& strTaskHeaderFilename, bool bHidden )
{
    wxLogTrace("CBOINCTaskCtrl::BeginTaskSection - Function Begining");

    wxString strModifiedTaskHeaderFilename;

    if ( bHidden )
        strModifiedTaskHeaderFilename = strTaskHeaderFilename + wxT(".hidden");
    else
        strModifiedTaskHeaderFilename = strTaskHeaderFilename + wxT(".visible");

    m_strTaskPage += "<table border=\"0\" bgcolor=\"White\" width=\"100%\" cellpadding=\"0\" cellspacing=\"0\">";
    m_strTaskPage += "  <tr bgcolor=\"#0080FF\">";
    m_strTaskPage += "    <td width=\"100%\">";
    m_strTaskPage += "      <img src=\"memory:" + strModifiedTaskHeaderFilename + "\">";
    m_strTaskPage += "    </td>";
    m_strTaskPage += "  </tr>";
    m_strTaskPage += "  <tr>";
    m_strTaskPage += "    <td>&nbsp;</td>";
    m_strTaskPage += "  </tr>";
    m_strTaskPage += "  <tr>";
    m_strTaskPage += "    <td align=\"right\">";
    m_strTaskPage += "      <table border=\"0\" width=\"90%\" cellpadding=\"0\" cellspacing=\"0\">";

    wxLogTrace("CBOINCTaskCtrl::BeginTaskSection - Function Ending");
}


void CBOINCTaskCtrl::CreateTask( const wxString& strTaskIconFilename, const wxString& strTaskName, const wxString& strTaskDescription )
{
    wxLogTrace("CBOINCTaskCtrl::CreateTask - Function Begining");

    m_strTaskPage += "        <tr>";
    m_strTaskPage += "          <td valign=\"center\" width=\"100%\">";
    m_strTaskPage += "            <img src=\"memory:" + strTaskIconFilename + "\">";
    m_strTaskPage += "            &nbsp;&nbsp;";
    m_strTaskPage += "            <font color=\"#000000\">" + strTaskName + "</font>";
    m_strTaskPage += "          </td>";
    m_strTaskPage += "        </tr>";

    wxLogTrace("CBOINCTaskCtrl::CreateTask - Function Ending");
}


void CBOINCTaskCtrl::EndTaskSection()
{
    wxLogTrace("CBOINCTaskCtrl::EndTaskSection - Function Begining");

    m_strTaskPage += "      </table>";
    m_strTaskPage += "    </td>";
    m_strTaskPage += "  </tr>";
    m_strTaskPage += "  <tr>";
    m_strTaskPage += "    <td width=\"100%\">&nbsp;</td>";
    m_strTaskPage += "  </tr>";
    m_strTaskPage += "</table>";
    m_strTaskPage += "<p></p>";

    wxLogTrace("CBOINCTaskCtrl::EndTaskSection - Function Ending");
}


void CBOINCTaskCtrl::CreateQuickTip( const wxString& strTip )
{
    wxLogTrace("CBOINCTaskCtrl::CreateQuickTip - Function Begining");

    m_strTaskPage += "        <tr>";
    m_strTaskPage += "          <td width=\"100%\">";
    m_strTaskPage += "            " + strTip;
    m_strTaskPage += "          </td>";
    m_strTaskPage += "        </tr>";

    wxLogTrace("CBOINCTaskCtrl::CreateQuickTip - Function Ending");
}


void CBOINCTaskCtrl::EndTaskPage()
{
    wxLogTrace("CBOINCTaskCtrl::EndTaskPage - Function Begining");

    m_strTaskPage += "</body></html>";
    SetPage(m_strTaskPage);

    wxLogTrace("CBOINCTaskCtrl::EndTaskPage - Function Ending");
}


void CBOINCTaskCtrl::CreateTaskHeader( const wxString& strFilename, const wxBitmap& itemTaskBitmap, const wxString& strTaskName )
{
    wxLogTrace("CBOINCTaskCtrl::CreateTaskHeader - Function Begining");

    wxMemoryDC dc;
    wxBitmap   bmpHeaderVisible(visibleheader_xpm);
    wxBitmap   bmpHeaderHidden(hiddenheader_xpm);
    wxBitmap   bmpTaskIcon(itemTaskBitmap);

    bmpHeaderVisible.SetMask(new wxMask(bmpHeaderVisible, wxColour(255, 0, 255)));
    bmpHeaderHidden.SetMask(new wxMask(bmpHeaderHidden, wxColour(255, 0, 255)));
    bmpTaskIcon.SetMask(new wxMask(bmpTaskIcon, wxColour(255, 0, 255)));


    dc.SelectObject(bmpHeaderVisible);
    dc.DrawBitmap(bmpTaskIcon, 17, 9, true);

    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetTextForeground(wxColour(255, 255, 255));
    dc.DrawText(strTaskName, 44, 12);

    dc.SelectObject( wxNullBitmap );

    dc.SelectObject(bmpHeaderHidden);
    dc.DrawBitmap(bmpTaskIcon, 17, 9, true);

    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetTextForeground(wxColour(255, 255, 255));
    dc.DrawText(strTaskName, 44, 12);

    dc.SelectObject( wxNullBitmap );

    wxImage imgHeaderVisible = bmpHeaderVisible.ConvertToImage();
    wxImage imgHeaderHidden = bmpHeaderHidden.ConvertToImage();

    AddVirtualFile(strFilename + wxT(".visible"), bmpHeaderVisible, wxBITMAP_TYPE_XPM);
    AddVirtualFile(strFilename + wxT(".hidden"), bmpHeaderHidden, wxBITMAP_TYPE_XPM);

    wxLogTrace("CBOINCTaskCtrl::CreateTaskHeader - Function Ending");
}


void CBOINCTaskCtrl::AddVirtualFile( const wxString& strFilename, wxImage& itemImage, long lType )
{
    wxLogTrace("CBOINCTaskCtrl::AddVirtualFile - Function Begining");

    wxMemoryFSHandler::AddFile( strFilename, itemImage, lType );

    wxLogTrace("CBOINCTaskCtrl::AddVirtualFile - Function Ending");
}


void CBOINCTaskCtrl::AddVirtualFile( const wxString& strFilename, const wxBitmap& itemBitmap, long lType )
{
    wxLogTrace("CBOINCTaskCtrl::AddVirtualFile - Function Begining");

    wxMemoryFSHandler::AddFile( strFilename, itemBitmap, lType );

    wxLogTrace("CBOINCTaskCtrl::AddVirtualFile - Function Ending");
}


void CBOINCTaskCtrl::RemoveVirtualFile( const wxString& strFilename )
{
    wxLogTrace("CBOINCTaskCtrl::RemoveVirtualFile - Function Begining");

    wxMemoryFSHandler::RemoveFile( strFilename );

    wxLogTrace("CBOINCTaskCtrl::RemoveVirtualFile - Function Ending");
}


void CBOINCTaskCtrl::OnRender( wxTimerEvent& event )
{
    wxLogTrace("CBOINCTaskCtrl::OnRender - Function Begining");
    wxLogTrace("CBOINCTaskCtrl::OnRender - Function Ending");
}


bool CBOINCTaskCtrl::OnSaveState( wxConfigBase* pConfig )
{
    wxLogTrace("CBOINCTaskCtrl::OnSaveState - Function Begining");
    wxLogTrace("CBOINCTaskCtrl::OnSaveState - Function Ending");
    return true;
}


bool CBOINCTaskCtrl::OnRestoreState( wxConfigBase* pConfig )
{
    wxLogTrace("CBOINCTaskCtrl::OnRestoreState - Function Begining");
    wxLogTrace("CBOINCTaskCtrl::OnRestoreState - Function Ending");
    return true;
}


void CBOINCTaskCtrl::OnLinkClicked( const wxHtmlLinkInfo& link )
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    if        (wxDynamicCast(m_pParentView, CViewProjects)) {
        FireOnLinkClickedEvent(wxDynamicCast(m_pParentView, CViewProjects), link);
    } else if (wxDynamicCast(m_pParentView, CViewWork)) {
        FireOnLinkClickedEvent(wxDynamicCast(m_pParentView, CViewWork), link);
    } else if (wxDynamicCast(m_pParentView, CViewTransfers)) {
        FireOnLinkClickedEvent(wxDynamicCast(m_pParentView, CViewTransfers), link);
    } else if (wxDynamicCast(m_pParentView, CViewMessages)) {
        FireOnLinkClickedEvent(wxDynamicCast(m_pParentView, CViewMessages), link);
    } else if (wxDynamicCast(m_pParentView, CViewResources)) {
        FireOnLinkClickedEvent(wxDynamicCast(m_pParentView, CViewResources), link);
    } else if (wxDynamicCast(m_pParentView, CViewResources)) {
        FireOnLinkClickedEvent(wxDynamicCast(m_pParentView, CBOINCBaseView), link);
    }
}


template < class T >
void CBOINCTaskCtrl::FireOnLinkClickedEvent( T pView, const wxHtmlLinkInfo& link )
{
    return pView->OnLinkClicked( link );
}


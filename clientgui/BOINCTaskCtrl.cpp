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
// Revision 1.3  2004/09/23 08:28:50  rwalton
// *** empty log message ***
//
// Revision 1.2  2004/09/22 21:53:02  rwalton
// *** empty log message ***
//
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
}


CBOINCTaskCtrl::CBOINCTaskCtrl(CBOINCBaseView* pView, wxWindowID iHtmlWindowID) :
    wxHtmlWindow( pView, iHtmlWindowID, wxDefaultPosition, wxSize(225, -1), wxHW_SCROLLBAR_AUTO|wxHSCROLL|wxVSCROLL )
{
    m_pParentView = pView;
}


CBOINCTaskCtrl::~CBOINCTaskCtrl()
{
}


void CBOINCTaskCtrl::BeginTaskPage()
{
    m_strTaskPage.Clear();
    m_strTaskPage += wxT("<html><body bgcolor=\"#0080FF\">");
}


void CBOINCTaskCtrl::BeginTaskSection( const wxString& strLink, const wxString& strTaskHeaderFilename, bool bHidden )
{
    wxString strModifiedTaskHeaderFilename;

    if ( bHidden )
        strModifiedTaskHeaderFilename = strTaskHeaderFilename + wxT(".hidden");
    else
        strModifiedTaskHeaderFilename = strTaskHeaderFilename + wxT(".visible");

    m_strTaskPage += wxT("<table border=\"0\" bgcolor=\"White\" width=\"100%\" cellpadding=\"0\" cellspacing=\"0\">");
    m_strTaskPage += wxT("  <tr bgcolor=\"#0080FF\">");
    m_strTaskPage += wxT("    <td width=\"100%\">");

    if ( !strLink.empty() )
        m_strTaskPage += wxT("      <a href=\"") + strLink + wxT("\">");

    m_strTaskPage += wxT("        <img src=\"memory:") + strModifiedTaskHeaderFilename + wxT("\">");

    if ( !strLink.empty() )
        m_strTaskPage += wxT("      </a>");

    m_strTaskPage += wxT("    </td>");
    m_strTaskPage += wxT("  </tr>");

    if ( !bHidden )
    {
        m_strTaskPage += wxT("  <tr>");
        m_strTaskPage += wxT("    <td>&nbsp;</td>");
        m_strTaskPage += wxT("  </tr>");
        m_strTaskPage += wxT("  <tr>");
        m_strTaskPage += wxT("    <td align=\"right\">");
        m_strTaskPage += wxT("      <table border=\"0\" width=\"90%\" cellpadding=\"0\" cellspacing=\"0\">");
    }
}


void CBOINCTaskCtrl::CreateTask( const wxString& strLink, const wxString& strTaskIconFilename, const wxString& strTaskName )
{
    m_strTaskPage += wxT("        <tr>");
    m_strTaskPage += wxT("          <td valign=\"center\" width=\"100%\">");

    if ( !strLink.empty() )
        m_strTaskPage += wxT("            <a href=\"") + strLink + wxT("\">");

    m_strTaskPage += wxT("              <img src=\"memory:") + strTaskIconFilename + wxT("\">");

    if ( !strLink.empty() )
        m_strTaskPage += wxT("            </a>");

    m_strTaskPage += wxT("            &nbsp;&nbsp;");

    if ( !strLink.empty() )
        m_strTaskPage += wxT("            <a href=\"") + strLink + wxT("\">");

    m_strTaskPage += wxT("              <font color=\"#000000\">") + strTaskName + wxT("</font>");

    if ( !strLink.empty() )
        m_strTaskPage += wxT("            </a>");

    m_strTaskPage += wxT("          </td>");
    m_strTaskPage += wxT("        </tr>");
}


void CBOINCTaskCtrl::EndTaskSection( bool bHidden )
{
    if ( !bHidden )
    {
        m_strTaskPage += wxT("      </table>");
        m_strTaskPage += wxT("    </td>");
        m_strTaskPage += wxT("  </tr>");
        m_strTaskPage += wxT("  <tr>");
        m_strTaskPage += wxT("    <td width=\"100%\">&nbsp;</td>");
        m_strTaskPage += wxT("  </tr>");
    }

    m_strTaskPage += wxT("</table>");
    m_strTaskPage += wxT("<p></p>");
}


void CBOINCTaskCtrl::UpdateQuickTip( const wxString& strLink, const wxString& strIconFilename, const wxString& strTip, bool bHidden )
{
    if (!strTip.empty())
    {
        BeginTaskSection(
            strLink,
            strIconFilename,
            bHidden
        );
        if (!bHidden)
        {
            m_strTaskPage += wxT("        <tr>");
            m_strTaskPage += wxT("          <td width=\"100%\">");
            m_strTaskPage += wxT("            ") + strTip;
            m_strTaskPage += wxT("          </td>");
            m_strTaskPage += wxT("        </tr>");
        }
        EndTaskSection(bHidden);
    }
}


void CBOINCTaskCtrl::EndTaskPage()
{
    m_strTaskPage += wxT("</body></html>");
    SetPage(m_strTaskPage);
}


void CBOINCTaskCtrl::CreateTaskHeader( const wxString& strFilename, const wxBitmap& itemTaskBitmap, const wxString& strTaskName )
{
    wxMemoryDC dc;
    wxBitmap   bmpHeaderVisible(visibleheader_xpm);
    wxBitmap   bmpHeaderHidden(hiddenheader_xpm);

    bmpHeaderVisible.SetMask(new wxMask(bmpHeaderVisible, wxColour(255, 0, 255)));
    bmpHeaderHidden.SetMask(new wxMask(bmpHeaderHidden, wxColour(255, 0, 255)));


    dc.SelectObject(bmpHeaderVisible);
    dc.DrawBitmap(itemTaskBitmap, 17, 9, true);

    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetTextForeground(wxColour(255, 255, 255));
    dc.DrawText(strTaskName, 44, 12);

    dc.SelectObject( wxNullBitmap );

    dc.SelectObject(bmpHeaderHidden);
    dc.DrawBitmap(itemTaskBitmap, 17, 9, true);

    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetTextForeground(wxColour(255, 255, 255));
    dc.DrawText(strTaskName, 44, 12);

    dc.SelectObject( wxNullBitmap );

    wxImage imgHeaderVisible = bmpHeaderVisible.ConvertToImage();
    wxImage imgHeaderHidden = bmpHeaderHidden.ConvertToImage();

    AddVirtualFile(strFilename + wxT(".visible"), bmpHeaderVisible, wxBITMAP_TYPE_XPM);
    AddVirtualFile(strFilename + wxT(".hidden"), bmpHeaderHidden, wxBITMAP_TYPE_XPM);
}


void CBOINCTaskCtrl::AddVirtualFile( const wxString& strFilename, wxImage& itemImage, long lType )
{
    wxMemoryFSHandler::AddFile( strFilename, itemImage, lType );
}


void CBOINCTaskCtrl::AddVirtualFile( const wxString& strFilename, const wxBitmap& itemBitmap, long lType )
{
    wxMemoryFSHandler::AddFile( strFilename, itemBitmap, lType );
}


void CBOINCTaskCtrl::RemoveVirtualFile( const wxString& strFilename )
{
    wxMemoryFSHandler::RemoveFile( strFilename );
}


void CBOINCTaskCtrl::OnRender( wxTimerEvent& event )
{
}


bool CBOINCTaskCtrl::OnSaveState( wxConfigBase* pConfig )
{
    return true;
}


bool CBOINCTaskCtrl::OnRestoreState( wxConfigBase* pConfig )
{
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


void CBOINCTaskCtrl::OnCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    if        (wxDynamicCast(m_pParentView, CViewProjects)) {
        FireOnCellMouseHoverEvent(wxDynamicCast(m_pParentView, CViewProjects), cell, x, y );
    } else if (wxDynamicCast(m_pParentView, CViewWork)) {
        FireOnCellMouseHoverEvent(wxDynamicCast(m_pParentView, CViewWork), cell, x, y );
    } else if (wxDynamicCast(m_pParentView, CViewTransfers)) {
        FireOnCellMouseHoverEvent(wxDynamicCast(m_pParentView, CViewTransfers), cell, x, y );
    } else if (wxDynamicCast(m_pParentView, CViewMessages)) {
        FireOnCellMouseHoverEvent(wxDynamicCast(m_pParentView, CViewMessages), cell, x, y );
    } else if (wxDynamicCast(m_pParentView, CViewResources)) {
        FireOnCellMouseHoverEvent(wxDynamicCast(m_pParentView, CViewResources), cell, x, y );
    } else if (wxDynamicCast(m_pParentView, CViewResources)) {
        FireOnCellMouseHoverEvent(wxDynamicCast(m_pParentView, CBOINCBaseView), cell, x, y );
    }
}


wxHtmlOpeningStatus CBOINCTaskCtrl::OnOpeningURL( wxHtmlURLType type, const wxString& url, wxString *redirect )
{
    wxHtmlOpeningStatus retval;

    retval = wxHTML_BLOCK;

    if ( wxHTML_URL_IMAGE == type )
        retval = wxHTML_OPEN;

    return retval;
}


template < class T >
void CBOINCTaskCtrl::FireOnLinkClickedEvent( T pView, const wxHtmlLinkInfo& link )
{
    return pView->OnLinkClicked( link );
}


template < class T >
void CBOINCTaskCtrl::FireOnCellMouseHoverEvent( T pView, wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    return pView->OnCellMouseHover( cell, x, y );
}


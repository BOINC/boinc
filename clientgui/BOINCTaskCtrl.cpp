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


CBOINCTaskCtrl::CBOINCTaskCtrl( CBOINCBaseView* pView, wxWindowID iHtmlWindowID, wxInt32 iHtmlWindowFlags ) :
    wxHtmlWindow( pView, iHtmlWindowID, wxDefaultPosition, wxSize(225, -1), iHtmlWindowFlags )
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


void CBOINCTaskCtrl::CreateTask( const wxString& strLink, const wxString& strTaskIconFilename, const wxString& strTaskName, bool  bHidden )
{
    if ( !bHidden )
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


bool CBOINCTaskCtrl::OnSaveState( wxConfigBase* WXUNUSED(pConfig) )
{
    return true;
}


bool CBOINCTaskCtrl::OnRestoreState( wxConfigBase* WXUNUSED(pConfig) )
{
    return true;
}


void CBOINCTaskCtrl::OnLinkClicked( const wxHtmlLinkInfo& link )
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    m_pParentView->FireOnTaskLinkClicked( link );
}


void CBOINCTaskCtrl::OnCellMouseHover( wxHtmlCell* cell, wxCoord x, wxCoord y )
{
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    m_pParentView->FireOnTaskCellMouseHover( cell, x, y );
}


wxHtmlOpeningStatus CBOINCTaskCtrl::OnOpeningURL( wxHtmlURLType type, const wxString& WXUNUSED(url), wxString* WXUNUSED(redirect) )
{
    wxHtmlOpeningStatus retval;

    retval = wxHTML_BLOCK;

    if ( wxHTML_URL_IMAGE == type )
        retval = wxHTML_OPEN;

    return retval;
}


#ifdef __GNUC__
static volatile const char  __attribute__((unused)) *BOINCrcsid="$Id$";
#else
static volatile const char *BOINCrcsid="$Id$";
#endif

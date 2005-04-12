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

#define BGCOLOR "#ffffff"


IMPLEMENT_DYNAMIC_CLASS(CBOINCTaskCtrl, wxHtmlWindow)


CBOINCTaskCtrl::CBOINCTaskCtrl() {}


CBOINCTaskCtrl::CBOINCTaskCtrl(CBOINCBaseView* pView, wxWindowID iHtmlWindowID, wxInt32 iHtmlWindowFlags) :
    wxHtmlWindow(pView, iHtmlWindowID, wxDefaultPosition, wxSize(225, -1), iHtmlWindowFlags)
{
    m_pParentView = pView;
}


CBOINCTaskCtrl::~CBOINCTaskCtrl() {}


void CBOINCTaskCtrl::BeginTaskPage() {
    m_strTaskPage.Clear();
    m_strTaskPage += wxT("<html>");
    m_strTaskPage += wxT("<head>");
    if (wxLocale::GetSystemEncodingName().size() > 0) {
        m_strTaskPage += wxT("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=");
        m_strTaskPage += wxLocale::GetSystemEncodingName();
        m_strTaskPage += wxT("\">");
    }
    m_strTaskPage += wxT("</head>");

    m_strTaskPage += wxT("<body bgcolor=" BGCOLOR ">");
}


void CBOINCTaskCtrl::BeginTaskSection(const wxString& strTaskHeaderFilename, bool bHidden) {
    wxString strModifiedTaskHeaderFilename;

    if (bHidden)
        strModifiedTaskHeaderFilename = strTaskHeaderFilename + wxT(".hidden");
    else
        strModifiedTaskHeaderFilename = strTaskHeaderFilename + wxT(".visible");

    m_strTaskPage += wxT("<table border=0 width=100% cellpadding=0 cellspacing=0>");
    m_strTaskPage += wxT("  <tr>");
    m_strTaskPage += wxT("    <td width=100%>");
    m_strTaskPage += wxT("        <img src=\"memory:") + strModifiedTaskHeaderFilename + wxT("\">");
    m_strTaskPage += wxT("    </td>");
    m_strTaskPage += wxT("  </tr>");

    if (!bHidden) {
        m_strTaskPage += wxT(
            "    <tr><td>"
            "     <table border=1 cellpadding=0 cellspacing=0>"
            "      <tr><td>"
            "       <table width=100% border=0 cellpadding=0 cellspacing=0>"
            "        <tr><td>&nbsp;</td></tr>"
       );
    }
}

void CBOINCTaskCtrl::EndTaskSection(bool bHidden) {
    if (!bHidden) {
        m_strTaskPage += wxT(
            "       <tr><td>&nbsp;</td></tr>"
            "      </table>"
            "     </td></tr>"
            "   </table>"
       );
    }

    m_strTaskPage += wxT("</table>");
    m_strTaskPage += wxT("<p></p>");
}

void CBOINCTaskCtrl::CreateTaskSeparator(bool  bHidden) {
    if (!bHidden) {
        m_strTaskPage += wxT("<tr><td><hr></td></tr>");
    }
}

void CBOINCTaskCtrl::CreateTask(const wxString& strLink, const wxString& strTaskName, bool  bHidden) {
    if (!bHidden) {
        m_strTaskPage += wxT("        <tr>");
        m_strTaskPage += wxT("          <td valign=\"center\" width=\"100%\">&nbsp;&nbsp;");

        if (strLink.empty()) {
            m_strTaskPage += wxT("<font color=\"#000000\">") + strTaskName + wxT("</font>");
        } else {
            m_strTaskPage += wxT(
                "<a href=\"") + strLink + wxT("\">"
                  "<font color=\"#000000\">") + strTaskName + wxT("</font>"
                "</a>"
           );
        }

        m_strTaskPage += wxT("          </td>");
        m_strTaskPage += wxT("        </tr>");
    }
}


void CBOINCTaskCtrl::CreateTask(const wxString& strLink, const wxString& strTaskIconFilename, const wxString& strTaskName, bool  bHidden) {
    if (!bHidden) {
        m_strTaskPage += wxT("        <tr>");
        m_strTaskPage += wxT("          <td valign=\"center\" width=\"100%\">");

        if (!strLink.empty())
            m_strTaskPage += wxT("            <a href=\"") + strLink + wxT("\">");

        m_strTaskPage += wxT("              <img src=\"memory:") + strTaskIconFilename + wxT("\">");

        if (!strLink.empty())
            m_strTaskPage += wxT("            </a>");

        m_strTaskPage += wxT("            &nbsp;&nbsp;");

        if (strLink.empty()) {
            m_strTaskPage += wxT("<font color=\"#000000\">") + strTaskName + wxT("</font>");
        } else {
            m_strTaskPage += wxT(
                "<a href=\"") + strLink + wxT("\">"
                "  <font color=\"#000000\">") + strTaskName + wxT("</font>"
                "</a>"
           );
        }
        m_strTaskPage += wxT("          </td>");
        m_strTaskPage += wxT("        </tr>");
    }
}




void CBOINCTaskCtrl::UpdateQuickTip(const wxString& strIconFilename, const wxString& strTip, bool bHidden) {
    if (!strTip.empty()) {
        BeginTaskSection(strIconFilename, bHidden);
        if (!bHidden) {
            m_strTaskPage += wxT(
                "<tr>"
                " <td width=\"100%\">"
                "  <table border=0 cellpadding=0 cellspacing=0>"
                "   <tr>"
                "    <td width=1%><nobr>&nbsp;&nbsp;</nobr></td>"
                "    <td>"
           );
            m_strTaskPage += strTip;
            m_strTaskPage += wxT(
                "    </td>"
                "   </tr>"
                "  </table>"
                " </td>"
                "</tr>"
           );
        }
        EndTaskSection(bHidden);
    }
}

void CBOINCTaskCtrl::EndTaskPage()
{
    m_strTaskPage += wxT("</body></html>");

    // ok, now get the corresponding encoding
    wxFontEncoding fontenc = wxFontMapper::Get()->CharsetToEncoding( wxLocale::GetSystemEncodingName(), false );
    if ( (NULL == fontenc) && !wxFontMapper::Get()->IsEncodingAvailable(fontenc) ) {
        // try to find some similar encoding:
        wxFontEncoding encAlt;
        if ( wxFontMapper::Get()->GetAltForEncoding(fontenc, &encAlt) ) {
            wxEncodingConverter conv;

            if (conv.Init(fontenc, encAlt)) {
                fontenc = encAlt;
                m_strTaskPage.Replace(
                    wxLocale::GetSystemEncodingName(),
                    wxFontMapper::Get()->GetEncodingName(fontenc)
                );
                m_strTaskPage = conv.Convert(m_strTaskPage);
            } else {
                wxLogTrace(wxT("Cannot convert from '%s' to '%s'."),
                             wxFontMapper::GetEncodingDescription(fontenc).c_str(),
                             wxFontMapper::GetEncodingDescription(encAlt).c_str());
            }
        } else {
            wxLogTrace(wxT("No fonts for encoding '%s' on this system."),
                         wxFontMapper::GetEncodingDescription(fontenc).c_str());
        }
    }

    wxFont profont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxEmptyString, fontenc);
    wxFont fixedfont(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxEmptyString, fontenc);

    if ( !profont.Ok() || !fixedfont.Ok() ) {
        wxASSERT( false );
    }

    SetFonts(profont.GetFaceName(), fixedfont.GetFaceName(), NULL);
    SetPage(m_strTaskPage);
}


void CBOINCTaskCtrl::CreateTaskHeader(const wxString& strFilename, const wxBitmap& itemTaskBitmap, const wxString& strTaskName)
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

    dc.SelectObject(wxNullBitmap);

    dc.SelectObject(bmpHeaderHidden);
    dc.DrawBitmap(itemTaskBitmap, 17, 9, true);

    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetTextForeground(wxColour(255, 255, 255));
    dc.DrawText(strTaskName, 44, 12);

    dc.SelectObject(wxNullBitmap);

    wxImage imgHeaderVisible = bmpHeaderVisible.ConvertToImage();
    wxImage imgHeaderHidden = bmpHeaderHidden.ConvertToImage();

    AddVirtualFile(strFilename + wxT(".visible"), bmpHeaderVisible, wxBITMAP_TYPE_XPM);
    AddVirtualFile(strFilename + wxT(".hidden"), bmpHeaderHidden, wxBITMAP_TYPE_XPM);
}


void CBOINCTaskCtrl::AddVirtualFile(const wxString& strFilename, wxImage& itemImage, long lType) {
    wxMemoryFSHandler::AddFile(strFilename, itemImage, lType);
}


void CBOINCTaskCtrl::AddVirtualFile(const wxString& strFilename, const wxBitmap& itemBitmap, long lType) {
    wxMemoryFSHandler::AddFile(strFilename, itemBitmap, lType);
}


void CBOINCTaskCtrl::RemoveVirtualFile(const wxString& strFilename) {
    wxMemoryFSHandler::RemoveFile(strFilename);
}


bool CBOINCTaskCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;

    wxASSERT(NULL != pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    pConfig->SetPath(strBaseConfigLocation + wxT("TaskCtrl/"));

    WriteCustomization(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    return true;
}


bool CBOINCTaskCtrl::OnRestoreState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;

    wxASSERT(NULL != pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    pConfig->SetPath(strBaseConfigLocation + wxT("TaskCtrl/"));

    ReadCustomization(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    // Aparently reading the customizations back in from the registry
    // delete the contents of the page, so lets force an update
    m_pParentView->UpdateTaskPane();

    return true;
}


void CBOINCTaskCtrl::OnLinkClicked(const wxHtmlLinkInfo& link) {
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    m_pParentView->FireOnTaskLinkClicked(link);
}


void CBOINCTaskCtrl::OnCellClicked(wxHtmlCell* cell, wxCoord x, wxCoord y, const wxMouseEvent& event) {
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    m_pParentView->FireOnTaskCellClicked(cell, x, y, event);
    wxHtmlWindow::OnCellClicked(cell, x, y, event);
}


void CBOINCTaskCtrl::OnCellMouseHover(wxHtmlCell* cell, wxCoord x, wxCoord y) {
    wxASSERT(NULL != m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    m_pParentView->FireOnTaskCellMouseHover(cell, x, y);
}


wxHtmlOpeningStatus CBOINCTaskCtrl::OnOpeningURL(wxHtmlURLType type, const wxString& WXUNUSED(url), wxString* WXUNUSED(redirect)) {
    wxHtmlOpeningStatus retval;

    retval = wxHTML_BLOCK;

    if (wxHTML_URL_IMAGE == type)
        retval = wxHTML_OPEN;

    return retval;
}


const char *BOINC_RCSID_125ef3d14d = "$Id$";

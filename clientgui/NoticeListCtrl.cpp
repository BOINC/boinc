// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "NoticeListCtrl.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "NoticeListCtrl.h"

////@begin XPM images
////@end XPM images

 /* CNoticeListCtrl type definition
 */
IMPLEMENT_DYNAMIC_CLASS( CNoticeListCtrl, wxWindow )


/*!
 * CNoticeListCtrl event table definition
 */

BEGIN_EVENT_TABLE( CNoticeListCtrl, wxWindow )

////@begin CNoticeListCtrl event table entries
#if wxUSE_WEBVIEW
    EVT_WEBVIEW_NAVIGATING(ID_LIST_NOTIFICATIONSVIEW, CNoticeListCtrl::OnLinkClicked)
    EVT_WEBVIEW_ERROR(ID_LIST_NOTIFICATIONSVIEW, CNoticeListCtrl::OnWebViewError)
#else
    EVT_HTML_LINK_CLICKED(ID_LIST_NOTIFICATIONSVIEW, CNoticeListCtrl::OnLinkClicked)
#endif
////@end CNoticeListCtrl event table entries

END_EVENT_TABLE()

/*!
 * CNoticeListCtrl constructors
 */

CNoticeListCtrl::CNoticeListCtrl( ) {
}

CNoticeListCtrl::CNoticeListCtrl( wxWindow* parent ) {
    Create( parent );
}


CNoticeListCtrl::~CNoticeListCtrl( ) {
}


/*!
 * CNoticeListCtrl creator
 */

bool CNoticeListCtrl::Create( wxWindow* parent ) {
////@begin CNoticeListCtrl member initialisation
////@end CNoticeListCtrl member initialisation

////@begin CNoticeListCtrl creation
    wxWindow::Create( parent, ID_LIST_NOTIFICATIONSVIEW, wxDefaultPosition, wxDefaultSize,
        wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
#if wxUSE_WEBVIEW
    m_browser = wxWebView::New( this, ID_LIST_NOTIFICATIONSVIEW );
#else
    m_browser = new wxHtmlWindow( this, ID_LIST_NOTIFICATIONSVIEW );
#endif
////@end CNoticeListCtrl creation

    wxBoxSizer *topsizer;
    topsizer = new wxBoxSizer(wxVERTICAL);

    topsizer->Add(m_browser, 1, wxEXPAND);
    SetAutoLayout(true);
    SetSizer(topsizer);

    m_itemCount = 0;
    bool isWindowsDarkMode = false;
#ifdef __WXMSW__
    const wxSystemAppearance appearance = wxSystemSettings::GetAppearance();
    isWindowsDarkMode = appearance.IsSystemDark();
#endif
    if (wxGetApp().GetIsDarkMode() || isWindowsDarkMode){
#if wxUSE_WEBVIEW
        m_noticesBody = wxT("<html><style>body{background-color:black;color:white;}</style><head></head><body></body></html>");
#else
        m_noticesBody = wxT("<html><head></head><body bgcolor=black></body></html>");
#endif
    } else {
        m_noticesBody = wxT("<html><head></head><body></body></html>");
    }

    // In Dark Mode, paint the window black immediately
#if wxUSE_WEBVIEW
    m_browser->SetPage(m_noticesBody, wxEmptyString);
#else
    m_browser->SetPage(m_noticesBody);
#endif

    // Display the fetching notices message until we have notices
    // to display or have determined that there are no notices.
    m_bDisplayFetchingNotices = false;
    m_bDisplayEmptyNotice = true;
    m_bNeedsReloading = false;

    return TRUE;
}


int CNoticeListCtrl::GetItemCount() {
    return m_itemCount;
}


void CNoticeListCtrl::SetItemCount(int newCount) {
    int i;

    CMainDocument* pDoc = wxGetApp().GetDocument();
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString strTitle = wxEmptyString;
    wxString strDescription = wxEmptyString;
    wxString strProjectName = wxEmptyString;
    wxString strURL = wxEmptyString;
    wxString strCreateTime = wxEmptyString;
    wxString strCategory = wxEmptyString;
    wxString strBuffer = wxEmptyString;
    wxString strTemp = wxEmptyString;
    wxDateTime dtBuffer;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    m_itemCount = newCount;
    bool isWindowsDarkMode = false;
#ifdef __WXMSW__
    const wxSystemAppearance appearance = wxSystemSettings::GetAppearance();
    isWindowsDarkMode = appearance.IsSystemDark();
#endif
    if (wxGetApp().GetIsDarkMode() || isWindowsDarkMode){
#if wxUSE_WEBVIEW
        m_noticesBody =  wxT("<html><style>body{background-color:black;color:white;}</style><head></head><body><font face=helvetica>");
#else
        m_noticesBody =  wxT("<html><head></head><body bgcolor=black><font face=helvetica color=white bgcolor=black>");
#endif
    } else {
        m_noticesBody =  wxT("<html><head></head><body><font face=helvetica>");
    }

    for (i=0; i<newCount; ++i) {
        if (pDoc->IsConnected()) {
            NOTICE* np = pDoc->notice((unsigned int)i);

            strCategory = wxString(np->category, wxConvUTF8);

            strProjectName = wxString(np->project_name, wxConvUTF8);

            strURL = wxString(np->link, wxConvUTF8);

            strTitle = wxString(np->title, wxConvUTF8);

            // Fix-up title
            if (strCategory == wxT("client")) {
                strBuffer.Printf(
                    wxT("_(\"Notice from %s\")"),
                    pSkinAdvanced->GetApplicationShortName().c_str()
                );
                if (strProjectName.size()) {
                    strTemp.Printf(wxT("%s: %s"), strProjectName.c_str(), strBuffer.c_str());
                } else {
                    strTemp.Printf(wxT("%s"), strBuffer.c_str());
                }
            } else if (strCategory == wxT("scheduler")) {
                strTemp.Printf(wxT("%s: %s"), strProjectName.c_str(), wxT("_(\"Notice from server\")"));
            } else {
                if (strProjectName.size()) {
                    strTemp.Printf(wxT("%s: %s"), strProjectName.c_str(), strTitle.c_str());
                } else {
                    strTemp = strTitle;
                }
            }

            strTitle = strTemp;
            eol_to_br(strTitle);
            localize(strTitle);

            strDescription = wxString(np->description.c_str(), wxConvUTF8);
            eol_to_br(strDescription);
            localize(strDescription);

            // RSS feeds and web pages may use protocol-relative (scheme-relative)
            // URLs, such as <img src="//sample.com/test.jpg"/>
            // Since the html comes from a web server via http, the scheme is
            // assumed to also be http.  But we have cached the html in a local
            // file, so it is no longer associated with the http protocol / scheme.
            // Therefore all our URLs must explicitly specify the http protocol.
            //
            // The second argument to wxWebView::SetPage is supposed to take care
            // of this automatically, but fails to do so under Windows, so we do
            // it here explicitly.
            strDescription.Replace(wxT("\"//"), wxT("\"http://"));
			strDescription.Replace(wxT("</a>"), wxT("</a> "));

            // Apparently attempting to follow links with other targets specified
            // fails to fire our event handler.  For now we will just strip out
            // the special _blank/_new target which is supposed to open a new
            // browser window anyways.
            strDescription.Replace(wxT("target=\"_blank\""), wxT(""));
            strDescription.Replace(wxT("target=\"_new\""), wxT(""));

            dtBuffer.Set((time_t)np->create_time);
            strCreateTime = dtBuffer.Format();

            // Put dividers between notices, but not before first or after last
            if (i == 0) {
                strBuffer = wxEmptyString;
            } else {
                strBuffer = wxT("<hr>");
            }

            strBuffer += wxT("<table border=0 cellpadding=5><tr><td>");

            if (!strTitle.IsEmpty()) {
                strTemp.Printf(
                    wxT("<b>%s</b><br>"),
                    strTitle.c_str()
                );
                strBuffer += strTemp;
            }

            strBuffer += strDescription;

            strBuffer += wxT("<br><font size=-2 color=#8f8f8f>");

            strBuffer += strCreateTime;

            if (!strURL.IsEmpty()) {
                strTemp.Printf(
                    wxT(" &middot; <a href=%s>%s</a> "),
                    strURL.c_str(),
                    _("more...")
                );
                strBuffer += strTemp;
            }

            strBuffer += wxT("</font></td></tr></table>");
        }
        m_noticesBody += strBuffer;
    }
    m_noticesBody += wxT("</font></body></html>");
    // baseURL is not needed here (see comments above) and it
    // must be an empty string for this to work under OS 10.12.4
#if wxUSE_WEBVIEW
    m_browser->SetPage(m_noticesBody, wxEmptyString);
#else
    m_browser->SetPage(m_noticesBody);
#endif
}


void CNoticeListCtrl::Clear() {
    m_bNeedsReloading = true;
    UpdateUI();
}

#if wxUSE_WEBVIEW
void CNoticeListCtrl::OnLinkClicked( wxWebViewEvent& event ) {
    if (event.GetURL().StartsWith(wxT("http://")) || event.GetURL().StartsWith(wxT("https://"))) {
        event.Veto();   // Tell wxWebView not to follow link
		wxLaunchDefaultBrowser(event.GetURL());
    } else {
        event.Skip();
    }
}


void CNoticeListCtrl::OnWebViewError( wxWebViewEvent& event ) {
   fprintf(stderr, "wxWebView error: target=%s, URL=%s\n",
            (event.GetTarget().ToStdString()).c_str(), (event.GetURL().ToStdString()).c_str());

    event.Skip();
}
#else
void CNoticeListCtrl::OnLinkClicked( wxHtmlLinkEvent& event ) {
    wxString url = event.GetLinkInfo().GetHref();
    if (url.StartsWith(wxT("http://")) || url.StartsWith(wxT("https://"))) {
        // wxHtmlLinkEvent doesn't have Veto(), but only loads the page if you
	      // call Skip().
		    wxLaunchDefaultBrowser(url);
    } else {
        event.Skip();
    }
 }
#endif

/*!
 * Update the UI.
 */
bool CNoticeListCtrl::UpdateUI() {
    static bool bAlreadyRunning = false;
    CMainDocument*  pDoc   = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Call Freeze() / Thaw() only when actually needed;
    // otherwise it causes unnecessary redraws
    int noticeCount = pDoc->GetNoticeCount();
    if ((noticeCount < 0) || (!pDoc->IsConnected()) || m_bNeedsReloading) {
        if (GetItemCount()) {
            SetItemCount(0);
            Refresh();
        }
        // Display "Fetching Notices" text only when connected
        m_bDisplayFetchingNotices = pDoc->IsConnected();
        m_bDisplayEmptyNotice = false;
        m_bNeedsReloading = false;
        return true;
    }

    if (noticeCount == 0) {
        if (GetItemCount()) {
            SetItemCount(0);
            Refresh();
        }
        m_bDisplayFetchingNotices = false;
        m_bDisplayEmptyNotice = true;
        m_bNeedsReloading = false;
        return true;
    }

    if (!bAlreadyRunning) {
        bAlreadyRunning = true;
        if (
            pDoc->IsConnected() &&
            (pDoc->notices.complete ||
            ((int)GetItemCount() != noticeCount))
        ) {
            pDoc->notices.complete = false;
            SetItemCount(noticeCount);
            m_bDisplayFetchingNotices = false;
            m_bDisplayEmptyNotice = false;
            Refresh();
        }

        bAlreadyRunning = false;
    }

    return true;
}

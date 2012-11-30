// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "stdwx.h"
#include "BOINCHtmlLBox.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

const wxChar BOINCHtmlListBoxNameStr[] = wxT("BOINCHtmlListBox");

// ============================================================================
// implementation
// ============================================================================

IMPLEMENT_ABSTRACT_CLASS(CBOINCHtmlListBox, wxHtmlListBox)


// ----------------------------------------------------------------------------
// CBOINCHtmlListBox creation
// ----------------------------------------------------------------------------

CBOINCHtmlListBox::CBOINCHtmlListBox()
    : wxHtmlListBox()
{
}

CBOINCHtmlListBox::~CBOINCHtmlListBox() {}

// normal constructor which calls Create() internally
CBOINCHtmlListBox::CBOINCHtmlListBox(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name)
    : wxHtmlListBox(parent, id, pos, size, style, name)
{
}

wxHtmlOpeningStatus
CBOINCHtmlListBox::OnHTMLOpeningURL(wxHtmlURLType (type),
                                const wxString& (url),
                                wxString *(redirect)) const
{
    // RSS feeds and web pages may use protocol-relative (scheme-relative) 
    // URLs, such as <img src="//sample.com/test.jpg"/>
    // Since the html comes from a web server via http, the scheme is
    // assumed to also be http.  But we have cached the html in a local 
    // file, so it is no longer associated with the http protocol / scheme.
    // Therefore all our URLs must explicity specify the http protocol.
    //
    // If the html is <img src="//sample.com/test.jpg" alt="" border="1" width="80" height="80" />
    // then the url argument passed here will be: //sample.com/test.jpg
    // so we need to prepend http:
    if (type == wxHTML_URL_IMAGE) {
        if (url.Left(2).IsSameAs(wxT("//"))) {
            *redirect = wxString(wxT("http:") + url);
            return wxHTML_REDIRECT;
        }
    }
    
    return wxHTML_OPEN;
}

wxCoord CBOINCHtmlListBox::OnMeasureItem(size_t n) const {
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    
    size_t x = pDoc->notices.notices.size();
    if (n < x) {
        NOTICE *ntc = pDoc->notices.notices[n];
        if (ntc == NULL) return 0;
        return wxHtmlListBox::OnMeasureItem(n);
    }
    return 0;
}


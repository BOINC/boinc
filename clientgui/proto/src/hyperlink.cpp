//////////////////////////////////////////////////////////////////////////////
// File:        hyperlink.cpp
// Purpose:     myHyperlink control
// Maintainer:  Wyo
// Created:     2003-04-07
// RCS-ID:      $Id$
// Copyright:   (c) wxGuide
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// information
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all 'standard' wxWindows headers)
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

// wxWindows headers
#include <wx/mimetype.h> // mimetype support

// hyperlink headers
#include "hyperlink.h"   // myHyperlink control


//----------------------------------------------------------------------------
// resources
//----------------------------------------------------------------------------


//============================================================================
// declarations
//============================================================================


//============================================================================
// implementation
//============================================================================

//----------------------------------------------------------------------------
// myHyperLink
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS (myHyperLink, wxStaticText)

BEGIN_EVENT_TABLE (myHyperLink, wxStaticText)
    EVT_ENTER_WINDOW (myHyperLink::OnWindowEnter)
    EVT_LEAVE_WINDOW (myHyperLink::OnWindowLeave)
    EVT_LEFT_DCLICK  (myHyperLink::OnLinkActivate)
    EVT_LEFT_DOWN    (myHyperLink::OnLinkActivate)
    EVT_LEFT_UP      (myHyperLink::OnLinkActivate)
END_EVENT_TABLE()

bool myHyperLink::Create (wxWindow *parent,
                          wxWindowID id,
                          const wxString &label,
                          const wxPoint &pos,
                          const wxSize &size,
                          long style,
                          const wxString &name) {
    bool okay = FALSE;

    // create static text
    okay = wxStaticText::Create (parent, id, label, pos, size, style, name);
    wxASSERT_MSG (okay, _("Failed to create wxStaticText, needed by myHyperLink!"));

    // initialize variables
    m_URL = wxEmptyString;
    m_Marked = false;
    m_Visited = false;
    m_MarkedColour = wxColour ("DARK GREY");
    m_NormalColour = wxColour ("BLUE");
    m_VisitedColour = wxColour ("PURPLE");
    m_HoverCursor = wxCursor (wxCURSOR_HAND);

    // set foreground colour
    SetForegroundColour (m_NormalColour);
    wxFont font = GetFont();
    font.SetUnderlined (true);
    SetFont (font);

    // get background colour
    m_BackgroundColour = GetBackgroundColour ();

    return okay;
} // Create

//----------------------------------------------------------------------------
// event handlers

void myHyperLink::OnWindowEnter (wxMouseEvent &WXUNUSED(event)) {
    SetCursor (m_HoverCursor);
    Refresh();
}

void myHyperLink::OnWindowLeave (wxMouseEvent &WXUNUSED(event)) {
    SetCursor (wxNullCursor);
    Refresh();
}

void myHyperLink::OnLinkActivate (wxMouseEvent &WXUNUSED(event)) {
    m_Visited = TRUE;
    SetForegroundColour (m_VisitedColour);
    SetBackgroundColour (m_BackgroundColour);
    Refresh();
    if (m_URL.IsEmpty()) {
       ExecuteLink (GetLabel());
    }else{
       ExecuteLink (m_URL);
    }
}

//----------------------------------------------------------------------------
// settings functions

wxCursor myHyperLink::GetHoverCursor () {
    return m_HoverCursor;
}

void myHyperLink::SetHoverCursor (wxCursor cursor) {
    m_HoverCursor = cursor;
}

wxColour myHyperLink::GetMarkedColour () {
    return m_MarkedColour;
}

void myHyperLink::SetMarkedColour (wxColour colour) {
    m_MarkedColour = colour;
}

wxColour myHyperLink::GetNormalColour () {
    return m_NormalColour;
}

void myHyperLink::SetNormalColour (wxColour colour) {
    m_NormalColour = colour;
    if (!m_Visited) {
        SetForegroundColour (m_NormalColour);
    }else{
        SetForegroundColour (m_VisitedColour);
    }
    Refresh();
}

wxColour myHyperLink::GetVisitedColour () {
    return m_VisitedColour;
}

void myHyperLink::SetVisitedColour (wxColour colour) {
    m_VisitedColour = colour;
    if (!m_Visited) {
        SetForegroundColour (m_NormalColour);
    }else{
        SetForegroundColour (m_VisitedColour);
    }
    Refresh();
}

wxString myHyperLink::GetURL () {
    return m_URL;
}

void myHyperLink::SetURL (const wxString &url) {
    m_URL = url;
}

//----------------------------------------------------------------------------
// private functions

void myHyperLink::ExecuteLink (const wxString &link) {
    wxString mimetype = wxEmptyString;
    if (link.StartsWith (_T("http://"))) {
        mimetype = _T("text/html");
    }else if (link.StartsWith (_T("ftp://"))) {
        mimetype = _T("text/html");
    }else if (link.StartsWith (_T("mailto:"))) {
        mimetype = _T("message/rfc822");
    }else{
        return;
    }
    wxFileType *filetype = wxTheMimeTypesManager->GetFileTypeFromMimeType (mimetype);
    if (filetype) {
        wxString cmd;
        if (filetype->GetOpenCommand (&cmd, wxFileType::MessageParameters (link))) {
            cmd.Replace(_T("file://"), wxEmptyString);
            ::wxExecute(cmd);
        }
    }
}


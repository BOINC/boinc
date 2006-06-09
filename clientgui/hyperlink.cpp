//////////////////////////////////////////////////////////////////////////////
// File:        hyperlink.cpp
// Purpose:     wxHyperLink control
// Maintainer:  Wyo
// Created:     2003-04-07
// RCS-ID:      $Id$
// Copyright:   (c) 2004 wxCode
// Licence:     wxWindows
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// information
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "hyperlink.h"   // wxHyperLink control

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
// wxHyperLink
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS (wxHyperLink, wxStaticText)

BEGIN_EVENT_TABLE    (wxHyperLink, wxStaticText)
    EVT_ENTER_WINDOW (wxHyperLink::OnWindowEnter)
    EVT_LEAVE_WINDOW (wxHyperLink::OnWindowLeave)
    EVT_LEFT_DCLICK  (wxHyperLink::OnLinkActivate)
    EVT_LEFT_DOWN    (wxHyperLink::OnLinkActivate)
END_EVENT_TABLE()

bool wxHyperLink::Create (wxWindow *parent,
                          wxWindowID id,
                          const wxString &label,
                          const wxPoint &pos,
                          const wxSize &size,
                          long style,
                          const wxString &name) {
    bool okay = FALSE;

    // create static text
    okay = wxStaticText::Create (parent, id, label, pos, size, style, name);
    wxASSERT_MSG (okay, wxT("Failed to create wxStaticText, needed by wxHyperLink!"));

    // initialize variables
    m_URL = wxEmptyString;
    m_Marked = false;
    m_Visited = false;
    m_MarkedColour = wxColour (wxT("DARK GREY"));
    m_NormalColour = wxColour (wxT("BLUE"));
    m_VisitedColour = wxColour (wxT("PURPLE"));
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

void wxHyperLink::OnWindowEnter (wxMouseEvent &WXUNUSED(event)) {
    SetCursor (m_HoverCursor);
    Refresh();
}

void wxHyperLink::OnWindowLeave (wxMouseEvent &WXUNUSED(event)) {
    SetCursor (wxNullCursor);
    Refresh();
}

void wxHyperLink::OnLinkActivate (wxMouseEvent &WXUNUSED(event)) {
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

wxCursor wxHyperLink::GetHoverCursor () {
    return m_HoverCursor;
}

void wxHyperLink::SetHoverCursor (wxCursor cursor) {
    m_HoverCursor = cursor;
}

wxColour wxHyperLink::GetMarkedColour () {
    return m_MarkedColour;
}

void wxHyperLink::SetMarkedColour (wxColour colour) {
    m_MarkedColour = colour;
}

wxColour wxHyperLink::GetNormalColour () {
    return m_NormalColour;
}

void wxHyperLink::SetNormalColour (wxColour colour) {
    m_NormalColour = colour;
    if (!m_Visited) {
        SetForegroundColour (m_NormalColour);
    }else{
        SetForegroundColour (m_VisitedColour);
    }
    Refresh();
}

wxColour wxHyperLink::GetVisitedColour () {
    return m_VisitedColour;
}

void wxHyperLink::SetVisitedColour (wxColour colour) {
    m_VisitedColour = colour;
    if (!m_Visited) {
        SetForegroundColour (m_NormalColour);
    }else{
        SetForegroundColour (m_VisitedColour);
    }
    Refresh();
}

wxString wxHyperLink::GetURL () {
    return m_URL;
}

void wxHyperLink::SetURL (const wxString &url) {
    m_URL = url;
}

//----------------------------------------------------------------------------
// private functions

void wxHyperLink::ExecuteLink (const wxString &strLink) {
    wxString cmd;
    wxString strMimeType = wxEmptyString;
    bool mime_type_found = false;

    if      (strLink.StartsWith(wxT("http://")))
        strMimeType = wxT("text/html");
    else if (strLink.StartsWith(wxT("ftp://")))
        strMimeType = wxT("text/html");
    else if (strLink.StartsWith(wxT("mailto:")))
        strMimeType = wxT("message/rfc822");
    else
        return;

    wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromMimeType(strMimeType);
    if (ft) {
        if (ft->GetOpenCommand(&cmd, wxFileType::MessageParameters(strLink))) {
#ifdef __WXMAC__
#if wxABI_VERSION < 20603   // These are needed for wxMac-2.6.2 but don't work with wxMac-2.6.3
            cmd.Replace(wxT(" <"), "\'");
            cmd.Prepend(wxT("open \'"));
#endif
#else
            cmd.Replace(wxT("file://"), wxEmptyString);
#endif
            mime_type_found = true;
            ::wxExecute(cmd);
        }
        delete ft;
    }

#if defined(__WXGTK__) || defined(__WXMOTIF__)
    if (!mime_type_found) {
        cmd = ::wxGetenv(wxT("BROWSER"));
        if(cmd.IsEmpty()) {
            wxString strDialogTitle = wxEmptyString;
            wxString strDialogMessage = wxEmptyString;

            // %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            strDialogTitle.Printf(
                _("%s - Can't find web browser"),
                wxGetApp().GetBrand()->GetApplicationName().c_str()
            );

            // 1st %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            // 2nd %s is the URL that the browser is supposed to
            //    open.
            // 3rd %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            strDialogMessage.Printf(
                _("%s tried to display the web page\n"
                "\t%s\n"
                "but couldn't find a web browser.\n"
                "To fix this, set the environment variable\n"
                "BROWSER to the path of your web browser,\n"
                "then restart the %s."),
                wxGetApp().GetBrand()->GetApplicationName().c_str(),
                strLink.c_str(),
                wxGetApp().GetBrand()->GetApplicationName().c_str()
            );

            ::wxMessageBox(
                strDialogMessage,
                strDialogTitle,
                wxOK | wxICON_INFORMATION
            );
        } else {
            cmd += wxT(" ") + strLink;
			::wxExecute(cmd,wxEXEC_ASYNC);
        }
    }
#endif
}

const char *BOINC_RCSID_d587835b7e="$Id$";

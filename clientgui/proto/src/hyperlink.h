//////////////////////////////////////////////////////////////////////////////
// File:        hyperlink.h
// Purpose:     myHyperlink control
// Maintainer:  Wyo
// Created:     2003-04-07
// RCS-ID:      $Id$
// Copyright:   (c) wxGuide
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef _MY_HYPERLINK_H_
#define _MY_HYPERLINK_H_

#ifdef __GNUG__
    #pragma implementation "hyperlink.h"
#endif

//----------------------------------------------------------------------------
// information
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------
//! wxWindows headers


//============================================================================
// declarations
//============================================================================

//----------------------------------------------------------------------------
//!


//----------------------------------------------------------------------------
//! myHyperLink
class myHyperLink: public wxStaticText {

DECLARE_DYNAMIC_CLASS (myHyperLink)

public:

    //! default constructor
    myHyperLink () {}

    //! create constructor
    myHyperLink (wxWindow *parent,
                 wxWindowID id,
                 const wxString &label = wxEmptyString,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize,
                 long style = 0,
                 const wxString &name = _T("HyperLink")) {
        Create (parent, id, label, pos, size, style, name);
    }

    // function create
    bool Create (wxWindow *parent,
                 wxWindowID id,
                 const wxString &label,
                 const wxPoint &pos,
                 const wxSize &size,
                 long style,
                 const wxString &name);

    // event handlers
    void OnWindowEnter (wxMouseEvent& event);
    void OnWindowLeave (wxMouseEvent& event);
    void OnLinkActivate (wxMouseEvent& event);

    // get/set settings
    wxCursor GetHoverCursor ();
    void SetHoverCursor (wxCursor cursor);
    wxColour GetMarkedColour ();
    void SetMarkedColour (wxColour colour);
    wxColour GetNormalColour ();
    void SetNormalColour (wxColour colour);
    wxColour GetVisitedColour ();
    void SetVisitedColour (wxColour colour);
    wxString GetURL ();
    void SetURL (const wxString &url);

private:

    //! hypertext variables
    wxString m_URL;
    bool m_Marked;
    bool m_Visited;

    //! style settings
    wxCursor m_HoverCursor;
    wxColour m_MarkedColour;
    wxColour m_NormalColour;
    wxColour m_VisitedColour;
    wxColour m_BackgroundColour;

    //! execute according to mimetype
    void ExecuteLink (const wxString &link);

    DECLARE_EVENT_TABLE()
};

#endif // _MY_HYPERLINK_H_


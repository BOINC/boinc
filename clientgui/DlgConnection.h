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


#ifndef _DLGCONNECTION_H_
#define _DLGCONNECTION_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgConnection.cpp"
#endif


#define ID_DIALOG 10000
#define SYMBOL_CDLGCONNECTION_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CDLGCONNECTION_TITLE _("Connection")
#define SYMBOL_CDLGCONNECTION_IDNAME ID_DIALOG
#define SYMBOL_CDLGCONNECTION_SIZE wxSize(400, 300)
#define SYMBOL_CDLGCONNECTION_POSITION wxDefaultPosition

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif


class CDlgConnection: public wxDialog
{    
    DECLARE_CLASS( CDlgConnection )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgConnection( );
    CDlgConnection( wxWindow* parent, wxWindowID id = SYMBOL_CDLGCONNECTION_IDNAME, const wxString& caption = SYMBOL_CDLGCONNECTION_TITLE, const wxPoint& pos = SYMBOL_CDLGCONNECTION_POSITION, const wxSize& size = SYMBOL_CDLGCONNECTION_SIZE, long style = SYMBOL_CDLGCONNECTION_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGCONNECTION_IDNAME, const wxString& caption = SYMBOL_CDLGCONNECTION_TITLE, const wxPoint& pos = SYMBOL_CDLGCONNECTION_POSITION, const wxSize& size = SYMBOL_CDLGCONNECTION_SIZE, long style = SYMBOL_CDLGCONNECTION_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Should we show tooltips?
    static bool ShowToolTips();
};

#endif
    // _DLGCONNECTION_H_


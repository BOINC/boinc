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

#ifndef _DLGABOUT_H_
#define _DLGABOUT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgAbout.cpp"
#endif


#define ID_DIALOG 10000
#define SYMBOL_CDLGHELPABOUT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CDLGHELPABOUT_TITLE _("About BOINC")
#define SYMBOL_CDLGHELPABOUT_IDNAME ID_DIALOG
#define SYMBOL_CDLGHELPABOUT_SIZE wxSize(400, 300)
#define SYMBOL_CDLGHELPABOUT_POSITION wxDefaultPosition

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif


class CDlgAbout: public wxDialog
{    
    DECLARE_CLASS( CDlgAbout )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgAbout( );
    CDlgAbout( wxWindow* parent, wxWindowID id = SYMBOL_CDLGHELPABOUT_IDNAME, const wxString& caption = SYMBOL_CDLGHELPABOUT_TITLE, const wxPoint& pos = SYMBOL_CDLGHELPABOUT_POSITION, const wxSize& size = SYMBOL_CDLGHELPABOUT_SIZE, long style = SYMBOL_CDLGHELPABOUT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGHELPABOUT_IDNAME, const wxString& caption = SYMBOL_CDLGHELPABOUT_TITLE, const wxPoint& pos = SYMBOL_CDLGHELPABOUT_POSITION, const wxSize& size = SYMBOL_CDLGHELPABOUT_SIZE, long style = SYMBOL_CDLGHELPABOUT_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Should we show tooltips?
    static bool ShowToolTips();
};

#endif
    // _DLGABOUT_H_


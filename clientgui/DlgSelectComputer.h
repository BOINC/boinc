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
//
#ifndef BOINC_DLGSELECTCOMPUTER_H
#define BOINC_DLGSELECTCOMPUTER_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgSelectComputer.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_CDLGSELECTCOMPUTER_STYLE wxDEFAULT_DIALOG_STYLE
#define SYMBOL_CDLGSELECTCOMPUTER_TITLE wxT("")
#define SYMBOL_CDLGSELECTCOMPUTER_IDNAME ID_DIALOG
#define SYMBOL_CDLGSELECTCOMPUTER_SIZE wxSize(400, 300)
#define SYMBOL_CDLGSELECTCOMPUTER_POSITION wxDefaultPosition
#define ID_SELECTCOMPUTERNAME 10001
#define ID_SELECTCOMPUTERPASSWORD 10002
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * CDlgSelectComputer class declaration
 */

class CDlgSelectComputer: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CDlgSelectComputer )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgSelectComputer( );
    CDlgSelectComputer( wxWindow* parent, bool required = false, wxWindowID id = SYMBOL_CDLGSELECTCOMPUTER_IDNAME, const wxString& caption = SYMBOL_CDLGSELECTCOMPUTER_TITLE, const wxPoint& pos = SYMBOL_CDLGSELECTCOMPUTER_POSITION, const wxSize& size = SYMBOL_CDLGSELECTCOMPUTER_SIZE, long style = SYMBOL_CDLGSELECTCOMPUTER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, bool required = false, wxWindowID id = SYMBOL_CDLGSELECTCOMPUTER_IDNAME, const wxString& caption = SYMBOL_CDLGSELECTCOMPUTER_TITLE, const wxPoint& pos = SYMBOL_CDLGSELECTCOMPUTER_POSITION, const wxSize& size = SYMBOL_CDLGSELECTCOMPUTER_SIZE, long style = SYMBOL_CDLGSELECTCOMPUTER_STYLE );

    /// Creates the controls and sizers
    void CreateControls(bool required);

////@begin CDlgSelectComputer event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SELECTCOMPUTERNAME
    void OnComputerNameUpdated( wxCommandEvent& event );

////@end CDlgSelectComputer event handler declarations

////@begin CDlgSelectComputer member function declarations

    wxString GetComputerName() const { return m_strComputerName ; }
    void SetComputerName(wxString value) { m_strComputerName = value ; }

    wxString GetComputerPassword() const { return m_strComputerPassword ; }
    void SetComputerPassword(wxString value) { m_strComputerPassword = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgSelectComputer member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDlgSelectComputer member variables
    wxComboBox* m_ComputerNameCtrl;
    wxTextCtrl* m_ComputerPasswordCtrl;
    wxString m_strComputerName;
    wxString m_strComputerPassword;
////@end CDlgSelectComputer member variables

#ifdef __WXMAC__
protected:
    wxAcceleratorEntry  m_Shortcuts[3];     // For Copy, Cut & Paste keyboard shortcuts
    wxAcceleratorTable* m_pAccelTable;
#endif
};

#endif

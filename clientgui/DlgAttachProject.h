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

#ifndef _DLGATTACHPROJECT_H_
#define _DLGATTACHPROJECT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgAttachProject.cpp"
#endif


#define ID_DIALOG 10000
#define SYMBOL_CDLGATTACHPROJECT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CDLGATTACHPROJECT_TITLE _("Attach to Project")
#define SYMBOL_CDLGATTACHPROJECT_IDNAME ID_DIALOG
#define SYMBOL_CDLGATTACHPROJECT_SIZE wxSize(400, 300)
#define SYMBOL_CDLGATTACHPROJECT_POSITION wxDefaultPosition
#define ID_PROJECTADDRESS 10001
#define ID_PROJECTACCOUNTKEY 10002

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif


class CDlgAttachProject: public wxDialog
{    
    DECLARE_CLASS( CDlgAttachProject )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgAttachProject( );
    CDlgAttachProject( wxWindow* parent, wxWindowID id = SYMBOL_CDLGATTACHPROJECT_IDNAME, const wxString& caption = SYMBOL_CDLGATTACHPROJECT_TITLE, const wxPoint& pos = SYMBOL_CDLGATTACHPROJECT_POSITION, const wxSize& size = SYMBOL_CDLGATTACHPROJECT_SIZE, long style = SYMBOL_CDLGATTACHPROJECT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGATTACHPROJECT_IDNAME, const wxString& caption = SYMBOL_CDLGATTACHPROJECT_TITLE, const wxPoint& pos = SYMBOL_CDLGATTACHPROJECT_POSITION, const wxSize& size = SYMBOL_CDLGATTACHPROJECT_SIZE, long style = SYMBOL_CDLGATTACHPROJECT_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxTextCtrl* m_ProjectAddressCtrl;
    wxTextCtrl* m_ProjectAccountKeyCtrl;
};

#endif
    // _DLGATTACHPROJECT_H_


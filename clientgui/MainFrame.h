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

#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "MainFrame.cpp"
#endif


class CMainFrame : public wxFrame
{
    DECLARE_DYNAMIC_CLASS(CMainFrame)

public:
    CMainFrame();
    CMainFrame(wxString strTitle);

    ~CMainFrame(void);

    void OnClose(wxCloseEvent &event);
    void OnIdle(wxIdleEvent &event);

    void OnExit(wxCommandEvent &event);
    void OnCommandsAttachProject(wxCommandEvent &event);
    void OnToolsOptions(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
    void OnStatusbar(wxCommandEvent &event);

    void OnToolbarsUI(wxUpdateUIEvent &event);
    void OnStatusbarUI(wxUpdateUIEvent &event);

private:

    wxMenuBar*      m_pMenubar;
    wxNotebook*     m_pNotebook;
    wxStatusBar*    m_pStatusbar;

    wxString        m_strStatusMessage;

    // menu bar
    bool            CreateMenu();
    bool            DeleteMenu();

    // notebook
    bool            CreateNotebook();
    bool            CreateNotebookPage(wxWindow* pwndNewNotebookPage);
    bool            DeleteNotebook();

    // status bar
    bool            CreateStatusbar();
    bool            DeleteStatusbar();

    DECLARE_EVENT_TABLE()
};


#endif


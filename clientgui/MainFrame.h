// $Id$
//
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
// Revision History:
//
// $Log$
// Revision 1.9  2004/07/13 05:56:01  rwalton
// Hooked up the Project and Work tab for the new GUI.
//
// Revision 1.8  2004/05/27 06:17:57  rwalton
// *** empty log message ***
//
// Revision 1.7  2004/05/21 06:27:15  rwalton
// *** empty log message ***
//
// Revision 1.6  2004/05/17 22:15:09  rwalton
// *** empty log message ***
//
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

    void OnExit(wxCommandEvent &event);
    void OnClose(wxCloseEvent &event);

    void OnCommandsAttachProject(wxCommandEvent &event);
    void OnToolsOptions(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);

    void OnFrameRender(wxTimerEvent &event);

private:

    wxMenuBar*      m_pMenubar;
    wxNotebook*     m_pNotebook;
    wxStatusBar*    m_pStatusbar;
    wxTimer*        m_pFrameRenderTimer;

    wxString        m_strStatusMessage;

    // menu bar
    bool            CreateMenu();
    bool            DeleteMenu();

    // notebook
    bool            CreateNotebook();
    template < class T >
        bool        CreateNotebookPage( T pwndNewNotebookPage );
    bool            DeleteNotebook();

    // status bar
    bool            CreateStatusbar();
    bool            DeleteStatusbar();

    // state management
    bool            SaveState();
    template < class T >
        bool        FireSaveStateEvent( T pPage, wxConfigBase* pConfig );

    bool            RestoreState();
    template < class T >
        bool        FireRestoreStateEvent( T pPage, wxConfigBase* pConfig );

    // Render management
    template < class T >
        void        FireRenderEvent( T pPage, wxTimerEvent &event );


    DECLARE_EVENT_TABLE()
};


#endif


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

    bool UpdateStatusbar( const wxString& strStatusbarText );

    void OnExit( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );

    void OnToolsOptions( wxCommandEvent& event );
    void OnAbout( wxCommandEvent& event );

    void OnIdle ( wxIdleEvent& event );

    void OnNotebookSelectionChanged( wxNotebookEvent& event );

    void OnListCacheHint( wxListEvent& event );
    void OnListSelected( wxListEvent& event );
    void OnListDeselected( wxListEvent& event );

    void OnListPanelRender( wxTimerEvent& event );
    void OnTaskPanelRender( wxTimerEvent& event );

private:

    wxMenuBar*      m_pMenubar;
    wxNotebook*     m_pNotebook;
    wxStatusBar*    m_pStatusbar;
    wxTimer*        m_pFrameTaskPanelRenderTimer;
    wxTimer*        m_pFrameListPanelRenderTimer;


    bool            CreateMenu();
    bool            DeleteMenu();

    bool            CreateNotebook();
    template < class T >
        bool        CreateNotebookPage( T pwndNewNotebookPage );
    bool            DeleteNotebook();

    bool            CreateStatusbar();
    bool            DeleteStatusbar();

    bool            SaveState();
    template < class T >
        bool        FireSaveStateEvent( T pPage, wxConfigBase* pConfig );

    bool            RestoreState();
    template < class T >
        bool        FireRestoreStateEvent( T pPage, wxConfigBase* pConfig );

    template < class T >
        void        FireListOnCacheHintEvent( T pView, wxListEvent& event );
    template < class T >
        void        FireListOnSelectedEvent( T pView, wxListEvent& event );
    template < class T >
        void        FireListOnDeselectedEvent( T pView, wxListEvent& event );
    template < class T >
        void        FireListPanelRenderEvent( T pPage, wxTimerEvent& event );

    template < class T >
        void        FireTaskPanelRenderEvent( T pPage, wxTimerEvent& event );


    DECLARE_EVENT_TABLE()
};


#endif


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


#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "MainFrame.cpp"
#endif

class CStatusBar : public wxStatusBar
{
    DECLARE_DYNAMIC_CLASS(CStatusBar)

public:
    CStatusBar();
    CStatusBar(wxWindow *parent);
    ~CStatusBar();

    void OnSize(wxSizeEvent& event);

    wxStaticBitmap* m_pbmpConnected;
    wxStaticText*   m_ptxtConnected;
    wxStaticBitmap* m_pbmpDisconnect;
    wxStaticText*   m_ptxtDisconnect;

private:
    DECLARE_EVENT_TABLE()
};


class CBOINCDialUpManager;


class CMainFrame : public CBOINCBaseFrame
{
    DECLARE_DYNAMIC_CLASS(CMainFrame)

public:
    CMainFrame();
    CMainFrame(wxString title, wxIcon* icon);

    ~CMainFrame(void);

    void OnActivitySelection( wxCommandEvent& event );
    void OnNetworkSelection( wxCommandEvent& event );
    void OnRunBenchmarks( wxCommandEvent& event );
    void OnSelectComputer( wxCommandEvent& event );
    void OnSwitchGUI( wxCommandEvent& event );
    void OnExit( wxCommandEvent& event );

    void OnCommandsRetryCommunications( wxCommandEvent& event );

    void OnProjectsAttachToAccountManager( wxCommandEvent& event );
    void OnAccountManagerUpdate( wxCommandEvent& event );
    void OnAccountManagerDetach( wxCommandEvent& event );
    void OnProjectsAttachToProject( wxCommandEvent& event );

    void OnOptionsOptions( wxCommandEvent& event );

    void OnHelp( wxHelpEvent& event );
    void OnHelpBOINCManager( wxCommandEvent& event );
    void OnHelpBOINCWebsite( wxCommandEvent& event );
    void OnHelpAbout( wxCommandEvent& event );

    void OnShow( wxShowEvent& event );
    void SetWindowDimensions();
    void GetWindowDimensions();

    void OnRefreshState( wxTimerEvent& event );
    void OnFrameRender( wxTimerEvent& event );
    void OnListPanelRender( wxTimerEvent& event );

    void OnNotebookSelectionChanged( wxNotebookEvent& event );

    void OnRefreshView( CFrameEvent& event );
    void OnConnect( CFrameEvent& event );
    void OnUpdateStatus( CFrameEvent& event );

    void ResetReminderTimers();

    void SetFrameListPanelRenderTimerRate();  // TODO: refactor out of the frame and put the
                                              //   relevent code in OnPageChanged function
                                              //   and the base/statistics view.

#ifdef __WXMAC__
    bool Show( bool show = true );
#endif

    wxTimer*        m_pRefreshStateTimer;
    wxTimer*        m_pFrameRenderTimer;
    wxTimer*        m_pFrameListPanelRenderTimer;

private:

    wxMenuBar*      m_pMenubar;
    wxNotebook*     m_pNotebook;
    CStatusBar*     m_pStatusbar;
    CBOINCDialUpManager* m_pDialupManager;

    wxString        m_strBaseTitle;

    long            m_Top;
    long            m_Left;
    long            m_Height;
    long            m_Width;

    bool            CreateMenu();
    bool            DeleteMenu();

    bool            CreateNotebook();
    template < class T >
        bool        CreateNotebookPage( T pwndNewNotebookPage );
    bool            DeleteNotebook();

    bool            CreateStatusbar();
    bool            DeleteStatusbar();

    bool            SaveState();
    bool            RestoreState();

    DECLARE_EVENT_TABLE()
};


#endif


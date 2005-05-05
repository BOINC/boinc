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

class CMainFrameEvent;

class CMainFrame : public wxFrame
{
    DECLARE_DYNAMIC_CLASS(CMainFrame)

public:
    CMainFrame();
    CMainFrame(wxString strTitle);

    ~CMainFrame(void);

    void OnHide( wxCommandEvent& event );
    void OnActivitySelection( wxCommandEvent& event );
    void OnNetworkSelection( wxCommandEvent& event );
    void OnRunBenchmarks( wxCommandEvent& event );
    void OnSelectComputer( wxCommandEvent& event );
    void OnExit( wxCommandEvent& event );

    void OnToolsUpdateAccounts( wxCommandEvent& event );
    void OnToolsOptions( wxCommandEvent& event );

    void OnHelpBOINCManager( wxHelpEvent& event );
    void OnHelpBOINCWebsite( wxCommandEvent& event );
    void OnHelpAbout( wxCommandEvent& event );

    void OnClose( wxCloseEvent& event );

    void OnRefreshState( wxTimerEvent& event );
    void OnFrameRender( wxTimerEvent& event );
    void OnListPanelRender( wxTimerEvent& event );
    void OnDocumentPoll( wxTimerEvent& event );

    void OnNotebookSelectionChanged( wxNotebookEvent& event );

    void OnConnect( CMainFrameEvent& event );
    void OnConnectError( CMainFrameEvent& event );
    void OnConnectErrorAuthentication( CMainFrameEvent& event );
    void OnInitialized( CMainFrameEvent& event );
    void OnRefreshView( CMainFrameEvent& event );

    void UpdateStatusText( const wxChar* szStatus );

    void FireConnect();
    void FireConnectError();
    void FireConnectErrorAuthentication();
    void FireRefreshView();

    void ExecuteBrowserLink( const wxString& strLink );

private:

    wxMenuBar*      m_pMenubar;
    wxNotebook*     m_pNotebook;
    CStatusBar*     m_pStatusbar;
    wxTimer*        m_pRefreshStateTimer;
    wxTimer*        m_pFrameRenderTimer;
    wxTimer*        m_pFrameListPanelRenderTimer;
    wxTimer*        m_pDocumentPollTimer;

    wxString        m_strBaseTitle;

    wxInt32         m_iSelectedLanguage;
    wxArrayString   m_aSelectedComputerMRU;


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


class CMainFrameEvent : public wxEvent
{
public:
    CMainFrameEvent(wxEventType evtType, CMainFrame *frame)
        : wxEvent(-1, evtType)
        {
            SetEventObject(frame);
        }

    virtual wxEvent *Clone() const { return new CMainFrameEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_MAINFRAME_CONNECT, 10000 )
DECLARE_EVENT_TYPE( wxEVT_MAINFRAME_CONNECT_ERROR, 10001 )
DECLARE_EVENT_TYPE( wxEVT_MAINFRAME_CONNECT_ERROR_AUTHENTICATION, 10002 )
DECLARE_EVENT_TYPE( wxEVT_MAINFRAME_INITIALIZED, 10003 )
DECLARE_EVENT_TYPE( wxEVT_MAINFRAME_REFRESHVIEW, 10004 )
END_DECLARE_EVENT_TYPES()

#define EVT_MAINFRAME_CONNECT(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_MAINFRAME_CONNECT, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_MAINFRAME_CONNECT_ERROR(fn)      DECLARE_EVENT_TABLE_ENTRY(wxEVT_MAINFRAME_CONNECT_ERROR, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_MAINFRAME_CONNECT_ERROR_AUTHENTICATION(fn) \
                                             DECLARE_EVENT_TABLE_ENTRY(wxEVT_MAINFRAME_CONNECT_ERROR_AUTHENTICATION, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_MAINFRAME_INITIALIZED(fn)        DECLARE_EVENT_TABLE_ENTRY(wxEVT_MAINFRAME_INITIALIZED, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_MAINFRAME_REFRESH(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_MAINFRAME_REFRESHVIEW, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


#endif


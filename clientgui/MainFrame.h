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

class CMainFrameEvent;
class CMainFrameAlertEvent;

enum MainFrameAlertEventType {
    AlertNormal = 0,
    AlertProcessResponse
};

class CMainFrame : public wxFrame
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

    void OnClose( wxCloseEvent& event );

    void OnRefreshState( wxTimerEvent& event );
    void OnFrameRender( wxTimerEvent& event );
    void OnListPanelRender( wxTimerEvent& event );
    void OnDocumentPoll( wxTimerEvent& event );

    void OnNotebookSelectionChanged( wxNotebookEvent& event );

    void OnAlert( CMainFrameAlertEvent& event );
    void OnInitialized( CMainFrameEvent& event );
    void OnRefreshView( CMainFrameEvent& event );
    void OnConnect( CMainFrameEvent& event );

    void SetFrameListPanelRenderTimerRate();
    void UpdateStatusText( const wxChar* szStatus );

    void FireInitialize();
    void FireRefreshView();
    void FireConnect();

    int       GetReminderFrequency() { return m_iReminderFrequency; }
    wxString  GetDialupConnectionName() { return m_strNetworkDialupConnectionName; }
    bool      GetDialupPromptForCredentials() { return m_bNetworkDialupPromptCredentials; }

    void ShowConnectionBadPasswordAlert();
    void ShowConnectionFailedAlert();
    void ShowNotCurrentlyConnectedAlert();
    void ShowAlert( 
        const wxString title,
        const wxString message,
        const int style,
        const bool notification_only = false,
        const MainFrameAlertEventType alert_event_type = AlertNormal
    );

    void ExecuteBrowserLink( const wxString& strLink );

#ifdef __WXMAC__
    bool Show( bool show = true );
#endif

    wxTimer*        m_pRefreshStateTimer;
    wxTimer*        m_pFrameRenderTimer;
    wxTimer*        m_pFrameListPanelRenderTimer;
    wxTimer*        m_pDocumentPollTimer;

private:

    wxMenuBar*      m_pMenubar;
    wxNotebook*     m_pNotebook;
    CStatusBar*     m_pStatusbar;
#ifdef __WXMSW__
    wxDynamicLibrary m_WININET;
    wxDynamicLibrary m_RASAPI32;
    CBOINCDialUpManager* m_pDialupManager;
#endif
    wxString        m_strBaseTitle;

    int             m_iSelectedLanguage;
    int             m_iReminderFrequency;
    int             m_iDisplayExitWarning;

    int             m_iNetworkConnectionType;
    wxString        m_strNetworkDialupConnectionName;
    bool            m_bNetworkDialupPromptCredentials;

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


class CMainFrameAlertEvent : public wxEvent
{
public:
    CMainFrameAlertEvent(wxEventType evtType, CMainFrame *frame, wxString title, wxString message, int style, bool notification_only, MainFrameAlertEventType alert_event_type)
        : wxEvent(-1, evtType), m_title(title), m_message(message), m_style(style), m_notification_only(notification_only), m_alert_event_type(alert_event_type)
        {
            SetEventObject(frame);
        }

    CMainFrameAlertEvent(wxEventType evtType, CMainFrame *frame, wxString title, wxString message, int style, bool notification_only)
        : wxEvent(-1, evtType), m_title(title), m_message(message), m_style(style), m_notification_only(notification_only)
        {
            SetEventObject(frame);
            m_alert_event_type = AlertNormal;
        }

    CMainFrameAlertEvent(const CMainFrameAlertEvent& event)
        : wxEvent(event)
        {
            m_title = event.m_title;
            m_message = event.m_message;
            m_style = event.m_style;
            m_notification_only = event.m_notification_only;
            m_alert_event_type = event.m_alert_event_type;
        }

    virtual wxEvent *Clone() const { return new CMainFrameAlertEvent(*this); }
    virtual void     ProcessResponse(const int response) const;

    wxString                m_title;
    wxString                m_message;
    int                     m_style;
    bool                    m_notification_only;
    MainFrameAlertEventType m_alert_event_type;
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_MAINFRAME_ALERT, 10000 )
DECLARE_EVENT_TYPE( wxEVT_MAINFRAME_CONNECT, 10001 )
DECLARE_EVENT_TYPE( wxEVT_MAINFRAME_INITIALIZED, 10004 )
DECLARE_EVENT_TYPE( wxEVT_MAINFRAME_REFRESHVIEW, 10005 )
END_DECLARE_EVENT_TYPES()

#define EVT_MAINFRAME_ALERT(fn)              DECLARE_EVENT_TABLE_ENTRY(wxEVT_MAINFRAME_ALERT, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_MAINFRAME_CONNECT(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_MAINFRAME_CONNECT, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_MAINFRAME_INITIALIZED(fn)        DECLARE_EVENT_TABLE_ENTRY(wxEVT_MAINFRAME_INITIALIZED, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_MAINFRAME_REFRESH(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_MAINFRAME_REFRESHVIEW, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


#endif


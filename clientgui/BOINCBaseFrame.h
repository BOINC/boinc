// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// CBOINCBaseFrame: Base class for the 2 variants of the main window,
// CBOINCAdvancedFrame and CBOINCSimpleFrame

#ifndef BOINC_BOINCBASEFRAME_H
#define BOINC_BOINCBASEFRAME_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCBaseFrame.cpp"
#endif

class CFrameEvent;
class CFrameAlertEvent;
class CBOINCDialUpManager;

enum FrameAlertEventType {
    AlertNormal = 0,
    AlertProcessResponse
};

class CBOINCBaseFrame : public wxFrame {

    DECLARE_DYNAMIC_CLASS( CBOINCBaseFrame )

public:

    CBOINCBaseFrame();
    CBOINCBaseFrame(
        wxWindow *parent,
        const wxWindowID id,
        const wxString& title,
        const wxPoint& pos,
        const wxSize& size,
        const long style
    );

    ~CBOINCBaseFrame();

    void                OnPeriodicRPC( wxTimerEvent& event );
    void                OnDocumentPoll( wxTimerEvent& event );
    void                OnAlertPoll( wxTimerEvent& event );
    virtual void        OnRefreshView( CFrameEvent& event );

    void                OnInitialized( CFrameEvent& event );

    virtual void        OnAlert( CFrameAlertEvent& event );
    virtual void        OnActivate( wxActivateEvent& event );
    virtual void        OnClose( wxCloseEvent& event );
    virtual void        OnCloseWindow( wxCommandEvent& event );
    virtual void        OnExit( wxCommandEvent& event );
    virtual void        OnDarkModeChanged( wxSysColourChangedEvent& event );

    void                OnWizardAttachProject( wxCommandEvent& event );
    void                OnWizardUpdate( wxCommandEvent& event );
    void                OnWizardDetach( wxCommandEvent& event );

    int                 GetCurrentViewPage();
    wxString            GetDialupConnectionName() { return m_strNetworkDialupConnectionName; }
    void                SetDialupConnectionName(wxString val) { m_strNetworkDialupConnectionName = val; }
    CBOINCDialUpManager* GetDialupManager() { return m_pDialupManager; }
    int                 GetReminderFrequency() { return m_iReminderFrequency; }
    void                SetReminderFrequency(int val) { m_iReminderFrequency = val; }

    void                FireInitialize();
    void                FireRefreshView();
    void                FireConnect();
    void                FireReloadSkin();
    void                FireNotification();

    bool                SelectComputer(wxString& hostName, int& portNum, wxString& password, bool required = false);
    void                ShowConnectionBadPasswordAlert( bool bUsedDefaultPassword, int iReadGUIRPCAuthFailure, std::string);
    void                ShowConnectionFailedAlert();
    void                ShowDaemonStartFailedAlert();
    void                ShowNotCurrentlyConnectedAlert();

    void                StartTimersBase();
    void                StopTimersBase();
    virtual void        StartTimers() {}
    virtual void        StopTimers() {}
    virtual void        UpdateRefreshTimerInterval();

    void                ShowAlert(
                            const wxString title,
                            const wxString message,
                            const int style,
                            const bool notification_only = false,
                            const FrameAlertEventType alert_event_type = AlertNormal
                        );

    bool                Show( bool bShow = true );

    virtual bool        RestoreState();
    virtual bool        SaveState();
    virtual bool        CreateMenus(){return true;}
    void                ResetReminderTimers();

protected:

    CBOINCDialUpManager* m_pDialupManager;

    wxTimer*            m_pDocumentPollTimer;
    wxTimer*            m_pAlertPollTimer;
    wxTimer*            m_pPeriodicRPCTimer;

    int                 m_iReminderFrequency;
    int                 m_iFrameRefreshRate;

    wxString            m_strNetworkDialupConnectionName;

    wxArrayString       m_aSelectedComputerMRU;

    bool                m_bShowConnectionFailedAlert;

    virtual int         _GetCurrentViewPage();

    wxPoint             m_ptFramePos;


    DECLARE_EVENT_TABLE()
};


class CFrameEvent : public wxEvent
{
public:
    CFrameEvent(wxEventType evtType, CBOINCBaseFrame *frame)
        : wxEvent(-1, evtType)
        {
            SetEventObject(frame);
        }

    CFrameEvent(wxEventType evtType, CBOINCBaseFrame *frame, wxString message)
        : wxEvent(-1, evtType), m_message(message)
        {
            SetEventObject(frame);
        }

    virtual wxEvent *Clone() const { return new CFrameEvent(*this); }

    wxString                m_message;
};


class CFrameAlertEvent : public wxEvent
{
public:
    CFrameAlertEvent(wxEventType evtType, CBOINCBaseFrame *frame, wxString title, wxString message, int style, bool notification_only, FrameAlertEventType alert_event_type)
        : wxEvent(-1, evtType), m_title(title), m_message(message), m_style(style), m_notification_only(notification_only), m_alert_event_type(alert_event_type)
        {
            SetEventObject(frame);
        }

    CFrameAlertEvent(wxEventType evtType, CBOINCBaseFrame *frame, wxString title, wxString message, int style, bool notification_only)
        : wxEvent(-1, evtType), m_title(title), m_message(message), m_style(style), m_notification_only(notification_only)
        {
            SetEventObject(frame);
            m_alert_event_type = AlertNormal;
        }

    CFrameAlertEvent(const CFrameAlertEvent& event)
        : wxEvent(event)
        {
            m_title = event.m_title;
            m_message = event.m_message;
            m_style = event.m_style;
            m_notification_only = event.m_notification_only;
            m_alert_event_type = event.m_alert_event_type;
        }

    virtual wxEvent *Clone() const { return new CFrameAlertEvent(*this); }
    virtual void     ProcessResponse(const int response) const;

    wxString                m_title;
    wxString                m_message;
    int                     m_style;
    bool                    m_notification_only;
    FrameAlertEventType m_alert_event_type;
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_FRAME_ALERT, 10000 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_CONNECT, 10001 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_INITIALIZED, 10004 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_REFRESHVIEW, 10005 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_UPDATESTATUS, 10006 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_RELOADSKIN, 10007 )
DECLARE_EVENT_TYPE( wxEVT_FRAME_NOTIFICATION, 10007 )
END_DECLARE_EVENT_TYPES()

#define EVT_FRAME_ALERT(fn)              DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_ALERT, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_CONNECT(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_CONNECT, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_INITIALIZED(fn)        DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_INITIALIZED, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_REFRESH(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_REFRESHVIEW, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_UPDATESTATUS(fn)       DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_UPDATESTATUS, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_RELOADSKIN(fn)         DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_RELOADSKIN, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),
#define EVT_FRAME_NOTIFICATION(fn)       DECLARE_EVENT_TABLE_ENTRY(wxEVT_FRAME_NOTIFICATION, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

#endif

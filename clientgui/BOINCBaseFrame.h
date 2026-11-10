// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

    CBOINCBaseFrame() = default;
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

    CBOINCDialUpManager* m_pDialupManager = nullptr;

    wxTimer*            m_pDocumentPollTimer = nullptr;
    wxTimer*            m_pAlertPollTimer = nullptr;
    wxTimer*            m_pPeriodicRPCTimer = nullptr;

    int                 m_iReminderFrequency = 0;
    int                 m_iFrameRefreshRate = 0;

    wxString            m_strNetworkDialupConnectionName;

    wxArrayString       m_aSelectedComputerMRU;

    bool                m_bShowConnectionFailedAlert = false;

    virtual int         _GetCurrentViewPage();

    wxPoint             m_ptFramePos;
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
        : wxEvent(-1, evtType), m_title(std::move(title)), m_message(std::move(message)), m_style(style), m_notification_only(notification_only), m_alert_event_type(alert_event_type)
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


wxDECLARE_EVENT(wxEVT_FRAME_ALERT, CFrameAlertEvent);
wxDECLARE_EVENT(wxEVT_FRAME_INITIALIZED, CFrameEvent);
wxDECLARE_EVENT(wxEVT_FRAME_REFRESHVIEW, CFrameEvent);
wxDECLARE_EVENT(wxEVT_FRAME_UPDATESTATUS, CFrameEvent);
wxDECLARE_EVENT(wxEVT_FRAME_CONNECT, CFrameEvent);
wxDECLARE_EVENT(wxEVT_FRAME_RELOADSKIN, CFrameEvent);
wxDECLARE_EVENT(wxEVT_FRAME_NOTIFICATION, CFrameEvent);

#endif

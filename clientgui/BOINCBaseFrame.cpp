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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCBaseFrame.h"
#endif

#include "stdwx.h"
#include "hyperlink.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "BOINCDialupManager.h"
#include "Events.h"


DEFINE_EVENT_TYPE(wxEVT_FRAME_ALERT)
DEFINE_EVENT_TYPE(wxEVT_FRAME_CONNECT)
DEFINE_EVENT_TYPE(wxEVT_FRAME_INITIALIZED)
DEFINE_EVENT_TYPE(wxEVT_FRAME_REFRESHVIEW)
DEFINE_EVENT_TYPE(wxEVT_FRAME_UPDATESTATUS)


IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseFrame, wxFrame)

BEGIN_EVENT_TABLE (CBOINCBaseFrame, wxFrame)
    EVT_TIMER(ID_DOCUMENTPOLLTIMER, CBOINCBaseFrame::OnDocumentPoll)
    EVT_TIMER(ID_ALERTPOLLTIMER, CBOINCBaseFrame::OnAlertPoll)
    EVT_FRAME_INITIALIZED(CBOINCBaseFrame::OnInitialized)
    EVT_FRAME_ALERT(CBOINCBaseFrame::OnAlert)
    EVT_CLOSE(CBOINCBaseFrame::OnClose)
END_EVENT_TABLE ()


CBOINCBaseFrame::CBOINCBaseFrame()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Default Constructor Function End"));
}


CBOINCBaseFrame::CBOINCBaseFrame(wxWindow* parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long style) :
    wxFrame(parent, id, title, pos, size, style) 
{
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Function Begin"));

    // Configuration Settings
    m_iSelectedLanguage = 0;
    m_iReminderFrequency = 0;
    m_iDisplayExitWarning = 1;

    m_strNetworkDialupConnectionName = wxEmptyString;

    m_aSelectedComputerMRU.Clear();


    m_pDialupManager = new CBOINCDialUpManager();
    wxASSERT(m_pDialupManager->IsOk());


    m_pDocumentPollTimer = new wxTimer(this, ID_DOCUMENTPOLLTIMER);
    wxASSERT(m_pDocumentPollTimer);

    m_pDocumentPollTimer->Start(250);                // Send event every 250 milliseconds

    m_pAlertPollTimer = new wxTimer(this, ID_ALERTPOLLTIMER);
    wxASSERT(m_pAlertPollTimer);

    m_pAlertPollTimer->Start(1000);                  // Send event every 1000 milliseconds


    // Limit the number of times the UI can update itself to two times a second
    //   NOTE: Linux and Mac were updating several times a second and eating
    //         CPU time
    wxUpdateUIEvent::SetUpdateInterval(500);

    // The second half of the initialization process picks up in the OnFrameRender()
    //   routine since the menus' and status bars' are drawn in the frameworks
    //   on idle routines, on idle events are sent in between the end of the
    //   constructor and the first call to OnFrameRender
    //
    // Look for the 'if (!bAlreadyRunOnce) {' statement

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::CBOINCBaseFrame - Function End"));
}


CBOINCBaseFrame::~CBOINCBaseFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::~CBOINCBaseFrame - Function Begin"));

    wxASSERT(m_pAlertPollTimer);
    wxASSERT(m_pDocumentPollTimer);

    if (m_pAlertPollTimer) {
        m_pAlertPollTimer->Stop();
        delete m_pAlertPollTimer;
    }

    if (m_pDocumentPollTimer) {
        m_pDocumentPollTimer->Stop();
        delete m_pDocumentPollTimer;
    }

    if (m_pDialupManager)
        delete m_pDialupManager;

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::~CBOINCBaseFrame - Function End"));
}


void CBOINCBaseFrame::OnDocumentPoll(wxTimerEvent& WXUNUSED(event)) {
    static bool        bAlreadyRunOnce = false;
    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!bAlreadyRunOnce) {
        // Complete any remaining initialization that has to happen after we are up
        //   and running
        FireInitialize();
        bAlreadyRunOnce = true;
    }

    pDoc->OnPoll();
}


void CBOINCBaseFrame::OnAlertPoll(wxTimerEvent& WXUNUSED(event)) {
    static bool       bAlreadyRunningLoop = false;
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        // Update idle detection if needed.
        wxGetApp().UpdateSystemIdleDetection();

        // Check to see if there is anything that we need to do from the
        //   dial up user perspective.
        if (pDoc && m_pDialupManager) {
            if (pDoc->IsConnected()) {
                m_pDialupManager->poll();
            }
        }

        bAlreadyRunningLoop = false;
    }
}


void CBOINCBaseFrame::OnInitialized(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnInitialized - Function Begin"));

    CMainDocument*     pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsConnected()) {
        pDoc->Connect(wxT("localhost"), wxEmptyString, TRUE, TRUE);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnInitialized - Function End"));
}


void CBOINCBaseFrame::OnAlert(CFrameAlertEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnAlert - Function Begin"));

#ifdef __WXMSW__
    CTaskBarIcon* pTaskbar = wxGetApp().GetTaskBarIcon();
    wxASSERT(pTaskbar);

    if ((IsShown() && !event.m_notification_only) || (IsShown() && !pTaskbar->IsBalloonsSupported())) {
        if (!event.m_notification_only) {
            int retval = 0;

            if (!IsShown()) {
                Show();
            }

            retval = ::wxMessageBox(event.m_message, event.m_title, event.m_style, this);
            if (event.m_alert_event_type == AlertProcessResponse) {
                event.ProcessResponse(retval);
            }
        }
    } else {
        // If the main window is hidden or minimzed use the system tray ballon
        //   to notify the user instead.  This keeps dialogs from interfering
        //   with people typing email messages or any other activity where they
        //   do not want keyboard focus changed to another window while typing.
        unsigned int  icon_type;

        if (wxICON_ERROR & event.m_style) {
            icon_type = NIIF_ERROR;
        } else if (wxICON_WARNING & event.m_style) {
            icon_type = NIIF_WARNING;
        } else if (wxICON_INFORMATION & event.m_style) {
            icon_type = NIIF_INFO;
        } else {
            icon_type = NIIF_NONE;
        }

        pTaskbar->SetBalloon(
            pTaskbar->m_iconTaskBarIcon,
            event.m_title,
            event.m_message,
            5000,
            icon_type
        );
    }
#else
    // Notification only events on platforms other than Windows are currently
    //   discarded.  On the Mac a model dialog box for a hidden window causes
    //   the menus to be locked and the application to become unresponsive. On
    //   Linux the application is restored and input focus is set on the
    //   notification which interrupts whatever the user was up to.
    if (IsShown() && !event.m_notification_only) {
        if (!event.m_notification_only) {
            int retval = 0;

            if (!IsShown()) {
                Show();
            }

            retval = ::wxMessageBox(event.m_message, event.m_title, event.m_style, this);
            if (event.m_alert_event_type == AlertProcessResponse) {
                event.ProcessResponse(retval);
            }
        }
    }
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnAlert - Function End"));
}


void CBOINCBaseFrame::OnClose(wxCloseEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnClose - Function Begin"));

    if (!event.CanVeto()) {
        Destroy();
    } else {
        Hide();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::OnClose - Function End"));
}


void CBOINCBaseFrame::FireInitialize() {
    CFrameEvent event(wxEVT_FRAME_INITIALIZED, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::FireRefreshView() {
    CFrameEvent event(wxEVT_FRAME_REFRESHVIEW, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::FireConnect() {
    CFrameEvent event(wxEVT_FRAME_CONNECT, this);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::ShowConnectionBadPasswordAlert() {
    wxString            strDialogTitle = wxEmptyString;

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionBadPasswordAlert - Function Begin"));

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Error"),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );

    ShowAlert(
        strDialogTitle,
        _("The password you have provided is incorrect, please try again."),
        wxOK | wxICON_ERROR
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionBadPasswordAlert - Function End"));
}


void CBOINCBaseFrame::ShowConnectionFailedAlert() {
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionFailedAlert - Function Begin"));

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Failed"),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2st %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s is not able to connect to a %s client.\n"
          "Would you like to try to connect again?"),
        wxGetApp().GetBrand()->GetApplicationName().c_str(),
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );

    ShowAlert(
        strDialogTitle,
        strDialogMessage,
        wxYES_NO | wxICON_QUESTION,
        false,
        AlertProcessResponse
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowConnectionFailedAlert - Function End"));
}


void CBOINCBaseFrame::ShowNotCurrentlyConnectedAlert() {
    wxString            strDialogTitle = wxEmptyString;
    wxString            strDialogMessage = wxEmptyString;

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowNotCurrentlyConnectedAlert - Function Begin"));

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strDialogTitle.Printf(
        _("%s - Connection Status"),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );

    // 1st %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    // 2nd %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    // 3nd %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strDialogMessage.Printf(
        _("%s is not currently connected to a %s client.\n"
          "Please use the 'File\\Select Computer...' menu option to connect up to a %s client.\n"
          "To connect up to your local computer please use 'localhost' as the host name."),
        wxGetApp().GetBrand()->GetApplicationName().c_str(),
        wxGetApp().GetBrand()->GetProjectName().c_str(),
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );
    ShowAlert(
        strDialogTitle,
        strDialogMessage,
        wxOK | wxICON_ERROR
    );

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::ShowNotCurrentlyConnectedAlert - Function End"));
}


void CBOINCBaseFrame::UpdateStatusText(const wxChar* szStatus) {
    CFrameEvent event(wxEVT_FRAME_UPDATESTATUS, this, szStatus);
    ProcessEvent(event);
}


void CBOINCBaseFrame::ShowAlert( const wxString title, const wxString message, const int style, const bool notification_only, const FrameAlertEventType alert_event_type ) {
    CFrameAlertEvent event(wxEVT_FRAME_ALERT, this, title, message, style, notification_only, alert_event_type);
    AddPendingEvent(event);
}


void CBOINCBaseFrame::ExecuteBrowserLink(const wxString &strLink) {
    wxHyperLink::ExecuteLink(strLink);
}


bool CBOINCBaseFrame::SaveState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::SaveState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;
    int             iIndex;
    int             iItemCount;


    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("Language"), m_iSelectedLanguage);
    pConfig->Write(wxT("ReminderFrequency"), m_iReminderFrequency);
    pConfig->Write(wxT("DisplayExitWarning"), m_iDisplayExitWarning);

    pConfig->Write(wxT("NetworkDialupConnectionName"), m_strNetworkDialupConnectionName);


    //
    // Save Computer MRU list
    //
    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + wxT("ComputerMRU");

    pConfig->SetPath(strConfigLocation);

    iItemCount = m_aSelectedComputerMRU.GetCount() - 1;
    for (iIndex = 0; iIndex <= iItemCount; iIndex++) {
        strBuffer.Printf(wxT("%d"), iIndex);
        pConfig->Write(
            strBuffer,
            m_aSelectedComputerMRU.Item(iIndex)
        );
    }

    pConfig->SetPath(strPreviousLocation);


    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::SaveState - Function End"));
    return true;
}


bool CBOINCBaseFrame::RestoreState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::RestoreState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;
    wxString        strValue;
    long            iIndex;
    bool            bKeepEnumerating = false;


    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("Language"), &m_iSelectedLanguage, 0L);
    pConfig->Read(wxT("ReminderFrequency"), &m_iReminderFrequency, 60L);
    pConfig->Read(wxT("DisplayExitWarning"), &m_iDisplayExitWarning, 1L);

    pConfig->Read(wxT("NetworkDialupConnectionName"), &m_strNetworkDialupConnectionName, wxEmptyString);


    //
    // Restore Computer MRU list
    //
    strPreviousLocation = pConfig->GetPath();
    strConfigLocation = strPreviousLocation + wxT("ComputerMRU");

    pConfig->SetPath(strConfigLocation);

    m_aSelectedComputerMRU.Clear();
    bKeepEnumerating = pConfig->GetFirstEntry(strBuffer, iIndex);
    while (bKeepEnumerating) {
        pConfig->Read(strBuffer, &strValue);

        m_aSelectedComputerMRU.Add(strValue);
        bKeepEnumerating = pConfig->GetNextEntry(strBuffer, iIndex);
    }

    pConfig->SetPath(strPreviousLocation);


    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseFrame::RestoreState - Function End"));
    return true;
}


void CFrameAlertEvent::ProcessResponse(const int response) const {
    CMainDocument*      pDoc = wxGetApp().GetDocument();
   
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if ((AlertProcessResponse == m_alert_event_type) && (wxYES == response)) {
        pDoc->Reconnect();
    }
}


const char *BOINC_RCSID_0a1bd38a5b = "$Id$";

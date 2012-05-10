// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgItemProperties.h"
#endif

#include "stdwx.h"
#include "util.h"
#include "DlgItemProperties.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "Events.h"
#include "error_numbers.h"

IMPLEMENT_DYNAMIC_CLASS(CDlgItemProperties, wxDialog)

BEGIN_EVENT_TABLE(CDlgItemProperties, wxDialog)

END_EVENT_TABLE()

/* Constructor */
CDlgItemProperties::CDlgItemProperties(wxWindow* parent) : 
    wxDialog( parent, ID_ANYDIALOG, wxEmptyString, wxDefaultPosition, 
                wxSize( 503,480 ), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ) {
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    wxASSERT(pFrame);
    if (!pFrame) return;

    SetSizeHints( wxDefaultSize, wxDefaultSize );
    SetExtraStyle( GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY );
    
    m_bSizer1 = new wxBoxSizer( wxVERTICAL );
    
    m_scrolledWindow = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    m_scrolledWindow->SetScrollRate( 5, 5 );
    wxBoxSizer* m_bSizer2;
    m_bSizer2 = new wxBoxSizer( wxVERTICAL );
    
    m_gbSizer = new wxGridBagSizer( 0, 0 );
    m_gbSizer->AddGrowableCol( 1 );
    m_gbSizer->SetFlexibleDirection( wxBOTH );
    m_gbSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_bSizer2->Add( m_gbSizer, 1, wxEXPAND, 5 );
    
    m_scrolledWindow->SetSizer( m_bSizer2 );
    m_scrolledWindow->Layout();
    m_bSizer2->Fit( m_scrolledWindow );
    m_bSizer1->Add( m_scrolledWindow, 1, wxEXPAND | wxALL, 5 );
    
    m_btnClose = new wxButton( this, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnClose->SetDefault(); 
    m_bSizer1->Add( m_btnClose, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
    
    SetSizer( m_bSizer1 );
    Layout();
    
    Centre( wxBOTH );

    m_current_row=0;

    int currentTabView = pFrame->GetCurrentViewPage();
    switch(currentTabView) {
    case VW_PROJ:
        m_strBaseConfigLocation = wxString(wxT("/DlgProjectProperties/"));
        break;
    case VW_TASK:
        m_strBaseConfigLocation = wxString(wxT("/DlgTaskProperties/"));
        break;
    default:
        m_strBaseConfigLocation = wxString(wxT("/DlgProperties/"));
        break;
    }

    RestoreState();
}

// destructor
CDlgItemProperties::~CDlgItemProperties() {
    SaveState();
}

/* saves dialog size and (on Mac) position */
bool CDlgItemProperties::SaveState() {
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);
    if (!pConfig) return false;

    pConfig->SetPath(m_strBaseConfigLocation);
    pConfig->Write(wxT("Width"), GetSize().GetWidth());
    pConfig->Write(wxT("Height"), GetSize().GetHeight());
#ifdef __WXMAC__
    pConfig->Write(wxT("XPos"), GetPosition().x);
    pConfig->Write(wxT("YPos"), GetPosition().y);
#endif
    return true;
}

/* restores former dialog size and (on Mac) position */
bool CDlgItemProperties::RestoreState() {
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    int                iWidth, iHeight;

    wxASSERT(pConfig);
    if (!pConfig) return false;

    pConfig->SetPath(m_strBaseConfigLocation);

    pConfig->Read(wxT("Width"), &iWidth, wxDefaultCoord);
    pConfig->Read(wxT("Height"), &iHeight, wxDefaultCoord);

#ifndef __WXMAC__
    // Set size to saved values or defaults if no saved values
    SetSize(iWidth, iHeight);    
#else
    int                iTop, iLeft;
    
    pConfig->Read(wxT("YPos"), &iTop, wxDefaultCoord);
    pConfig->Read(wxT("XPos"), &iLeft, wxDefaultCoord);
    
    // If either co-ordinate is less then 0 then set it equal to 0 to ensure
    // it displays on the screen.
    if ((iLeft < 0) && (iLeft != wxDefaultCoord)) iLeft = 30;
    if ((iTop < 0) && (iTop != wxDefaultCoord)) iTop = 30;

    // Set size and position to saved values or defaults if no saved values
    SetSize(iLeft, iTop, iWidth, iHeight, wxSIZE_USE_EXISTING);

    // Now make sure window is on screen
    GetScreenPosition(&iLeft, &iTop);
    GetSize(&iWidth, &iHeight);

    if (!IsWindowOnScreen(iLeft, iTop, iWidth, iHeight)) {
        iTop = iLeft = 30;
        SetSize(iLeft, iTop, iWidth, iHeight, wxSIZE_USE_EXISTING);
    }
#endif

    return true;
}

void CDlgItemProperties::show_rsc(wxString rsc_name, RSC_DESC rsc_desc) {
    if (rsc_desc.no_rsc_pref) {
        addProperty(_("Don't fetch tasks for ") + rsc_name, _("Project preference"));
    }
    if (rsc_desc.no_rsc_ams) {
        addProperty(_("Don't fetch tasks for ") + rsc_name, _("Account manager preference"));
    }
    if (rsc_desc.no_rsc_apps) {
        addProperty(_("Don't fetch tasks for ") + rsc_name, _("Project has no apps for ") + rsc_name);
    }
    if (rsc_desc.no_rsc_config) {
        addProperty(_("Don't fetch tasks for ") + rsc_name, _("Client configuration excludes ") + rsc_name);
    }
    double x = rsc_desc.backoff_time - dtime();
    if (x<0) x = 0;
    addProperty(rsc_name + _(" work fetch deferred for"), FormatTime(x));
    addProperty(rsc_name + _(" work fetch deferral interval"), FormatTime(rsc_desc.backoff_interval));
}

// show project properties
//
void CDlgItemProperties::renderInfos(PROJECT* project_in) {
    std::string projectname;
    //collecting infos
    project_in->get_name(projectname);
    //disk usage needs additional lookups
    CMainDocument* pDoc = wxGetApp().GetDocument();
    pDoc->CachedDiskUsageUpdate();
    
    // CachedDiskUsageUpdate() may have invalidated our project 
    // pointer, so get an updated pointer to this project
    PROJECT* project = pDoc->project(project_in->master_url);
    if (!project) return;     // TODO: display some sort of error alert?

    std::vector<PROJECT*> dp = pDoc->disk_usage.projects;
    double diskusage=0.0;    
    for (unsigned int i=0; i< dp.size(); i++) {
        PROJECT* tp = dp[i];        
        std::string tname;        
        tp->get_name(tname);
        wxString t1(wxString(tname.c_str(), wxConvUTF8));
        if(t1.IsSameAs(wxString(projectname.c_str(), wxConvUTF8)) || t1.IsSameAs(wxString(project->master_url, wxConvUTF8))) {
            diskusage = tp->disk_usage;
            break;
        }
    }
    //set dialog title
    wxString wxTitle = _("Properties of project ");
    wxTitle.append(wxString(projectname.c_str(),wxConvUTF8));
    SetTitle(wxTitle);
    //layout controls
    addSection(_("General"));
    addProperty(_("Master URL"), wxString(project->master_url, wxConvUTF8));
    addProperty(_("User name"), wxString(project->user_name.c_str(), wxConvUTF8));
    addProperty(_("Team name"), wxString(project->team_name.c_str(), wxConvUTF8));
    addProperty(_("Resource share"), wxString::Format(wxT("%0.0f"), project->resource_share));
    if (project->min_rpc_time > dtime()) {
        addProperty(_("Scheduler RPC deferred for"), FormatTime(project->min_rpc_time - dtime()));
    }
    if (project->download_backoff) {
        addProperty(_("File downloads deferred for"), FormatTime(project->download_backoff));
    }
    if (project->upload_backoff) {
        addProperty(_("File uploads deferred for"), FormatTime(project->upload_backoff));
    }
    addProperty(_("Disk usage"), FormatDiskSpace(diskusage));
    addProperty(_("Computer ID"), wxString::Format(wxT("%d"), project->hostid));
    if (project->non_cpu_intensive) {
        addProperty(_("Non CPU intensive"), _("yes"));
    }
    addProperty(_("Suspended via GUI"), project->suspended_via_gui ? _("yes") : _("no"));
    addProperty(_("Don't request more work"), project->dont_request_more_work ? _("yes") : _("no"));
    if (project->scheduler_rpc_in_progress) {
        addProperty(_("Scheduler call in progress"), _("yes"));
    }
    if (project->trickle_up_pending) {
        addProperty(_("Trickle-up pending"), _("yes"));
    }
    if (strlen(project->venue)) {
        addProperty(_("Host location"), wxString(project->venue, wxConvUTF8));
    } else {
        addProperty(_("Host location"), _("default"));
    }

    if (project->attached_via_acct_mgr) {
        addProperty(_("Added via account manager"), _("yes"));
    }
    if (project->detach_when_done) {
        addProperty(_("Remove when tasks done"), _("yes"));
    }
    if (project->ended) {
        addProperty(_("Ended"), _("yes"));
    }
    addSection(_("Credit"));
    addProperty(_("User"),
        wxString::Format(
            wxT("%0.2f total, %0.2f average"),
            project->user_total_credit,
            project->user_expavg_credit
        )
    );
    addProperty(_("Host"),
        wxString::Format(
            wxT("%0.2f total, %0.2f average"),
            project->host_total_credit,
            project->host_expavg_credit
        )
    );
    
    if (!project->non_cpu_intensive) {
        addSection(_("Scheduling"));
        addProperty(_("Scheduling priority"), wxString::Format(wxT("%0.2f"), project->sched_priority));
        show_rsc(_("CPU"), project->rsc_desc_cpu);
        if (pDoc->state.have_nvidia) {
            show_rsc(_("NVIDIA GPU"), project->rsc_desc_nvidia);
        }
        if (pDoc->state.have_ati) {
            show_rsc(_("ATI GPU"), project->rsc_desc_ati);
        }
        addProperty(_("Duration correction factor"), wxString::Format(wxT("%0.4f"), project->duration_correction_factor));
    }
    m_gbSizer->Layout();
    m_scrolledWindow->FitInside();
}

// show task properties
//
void CDlgItemProperties::renderInfos(RESULT* result) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxDateTime dt;
    wxString wxTitle = _("Properties of task ");
    wxTitle.append(wxString(result->name, wxConvUTF8));
    SetTitle(wxTitle);

    APP_VERSION* avp = NULL;
    WORKUNIT* wup = NULL;
    RESULT* r = pDoc->state.lookup_result(result->project_url, result->name);
    if (r) {
        avp = r->avp;
        wup = r->wup;
    }
    
    addProperty(_("Application"), FormatApplicationName(result));
    addProperty(_("Workunit name"), wxString(result->wu_name, wxConvUTF8));
    addProperty(_("State"), result_description(result, false));
    if (result->received_time) {
        dt.Set((time_t)result->received_time);
        addProperty(_("Received"), dt.Format());
    }
    dt.Set((time_t)result->report_deadline);
    addProperty(_("Report deadline"), dt.Format());
    if (strlen(result->resources)) {
        addProperty(_("Resources"), wxString(result->resources, wxConvUTF8));
    }
    if (wup) {
        addProperty(_("Estimated computation size"), wxString::Format(wxT("%.0f GFLOPs"), wup->rsc_fpops_est/1e9));
    }
    if (result->active_task) {
        addProperty(_("CPU time at last checkpoint"), FormatTime(result->checkpoint_cpu_time));
        addProperty(_("CPU time"), FormatTime(result->current_cpu_time));
        if (result->elapsed_time >= 0) {
            addProperty(_("Elapsed time"), FormatTime(result->elapsed_time));
        }
        addProperty(_("Estimated time remaining"), FormatTime(result->estimated_cpu_time_remaining));
        addProperty(_("Fraction done"), wxString::Format(wxT("%.3f %%"), result->fraction_done*100));
        addProperty(_("Virtual memory size"), FormatDiskSpace(result->swap_size));
        addProperty(_("Working set size"), FormatDiskSpace(result->working_set_size_smoothed));
        if (result->slot >= 0) {
            addProperty(_("Directory"), wxString::Format(wxT("slots/%d"), result->slot));
        }
        if (result->pid) {
            addProperty(_("Process ID"), wxString::Format(wxT("%d"), result->pid));
        }
    } else if (result->state >= RESULT_COMPUTE_ERROR) {
        addProperty(_("CPU time"), FormatTime(result->final_cpu_time));
        addProperty(_("Elapsed time"), FormatTime(result->final_elapsed_time));
    }
    m_gbSizer->Layout();
    m_scrolledWindow->FitInside();
}

//
wxString CDlgItemProperties::FormatDiskSpace(double bytes) {    
    double         xTera = 1099511627776.0;
    double         xGiga = 1073741824.0;
    double         xMega = 1048576.0;
    double         xKilo = 1024.0;
    wxString strBuffer= wxEmptyString;

    if (bytes >= xTera) {
        strBuffer.Printf(wxT("%0.2f TB"), bytes/xTera);
    } else if (bytes >= xGiga) {
        strBuffer.Printf(wxT("%0.2f GB"), bytes/xGiga);
    } else if (bytes >= xMega) {
        strBuffer.Printf(wxT("%0.2f MB"), bytes/xMega);
    } else if (bytes >= xKilo) {
        strBuffer.Printf(wxT("%0.2f KB"), bytes/xKilo);
    } else {
        strBuffer.Printf(wxT("%0.0f bytes"), bytes);
    }
    return strBuffer;
}

//
wxString CDlgItemProperties::FormatApplicationName(RESULT* result ) {
    wxString       strBuffer = wxEmptyString;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT*        state_result = NULL;
    wxString       strAppBuffer = wxEmptyString;
    wxString       strClassBuffer = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (result) {
        state_result = pDoc->state.lookup_result(result->project_url, result->name);
        if (!state_result) {
            pDoc->ForceCacheUpdate();
            state_result = pDoc->state.lookup_result(result->project_url, result->name);
        }
        wxASSERT(state_result);

        if (!state_result) return strBuffer;
        WORKUNIT* wup = state_result->wup;
        if (!wup) return strBuffer;
        APP* app = wup->app;
        if (!app) return strBuffer;
        APP_VERSION* avp = state_result->avp;
        if (!avp) return strBuffer;

        if (strlen(app->user_friendly_name)) {
            strAppBuffer = wxString(state_result->app->user_friendly_name, wxConvUTF8);
        } else {
            strAppBuffer = wxString(state_result->avp->app_name, wxConvUTF8);
        }

        if (strlen(avp->plan_class)) {
            strClassBuffer.Printf(
                wxT(" (%s)"),
                wxString(avp->plan_class, wxConvUTF8).c_str()
            );
        }

        strBuffer.Printf(
            wxT("%s%s %d.%02d %s"),
            state_result->project->anonymous_platform?_("Local: "):wxT(""),
            strAppBuffer.c_str(),
            state_result->avp->version_num / 100,
            state_result->avp->version_num % 100,
            strClassBuffer.c_str()
        );
    }
    return strBuffer;
}

// 
wxString CDlgItemProperties::FormatTime(float fBuffer) {
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
    wxString strBuffer= wxEmptyString;

    if (0 >= fBuffer) {
        strBuffer = wxT("---");
    } else {
        iHour = (wxInt32)(fBuffer / (60 * 60));
        iMin  = (wxInt32)(fBuffer / 60) % 60;
        iSec  = (wxInt32)(fBuffer) % 60;

        ts = wxTimeSpan(iHour, iMin, iSec);

        strBuffer = ts.Format();
    }

    return strBuffer;
}

// adds a title section label to the dialog 
void CDlgItemProperties::addSection(const wxString& title) {
    wxStaticText* staticText = new wxStaticText(m_scrolledWindow, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, 0);
    staticText->Wrap(-1);
    staticText->SetFont(wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString));    
    m_gbSizer->Add(staticText, wxGBPosition( m_current_row, 0), wxGBSpan(1, 2), wxALL|wxEXPAND, 3);
    m_current_row++;
}

// adds a property row to the dialog 
void CDlgItemProperties::addProperty(const wxString& name, const wxString& value) {
    
    wxStaticText* staticText = new wxStaticText( m_scrolledWindow, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, 0 );
    staticText->Wrap( -1 );
    m_gbSizer->Add( staticText, wxGBPosition( m_current_row, 0 ), wxGBSpan( 1, 1 ), wxALL, 3 );
    
    staticText = new wxStaticText( m_scrolledWindow, wxID_ANY, value, wxDefaultPosition, wxDefaultSize, 0 );
    staticText->Wrap( -1 );
    m_gbSizer->Add( staticText, wxGBPosition( m_current_row, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
    m_current_row++;
}

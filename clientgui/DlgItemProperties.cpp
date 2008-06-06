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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgItemProperties.h"
#endif

#include "stdwx.h"
#include "DlgItemProperties.h"
#include "BOINCGUIApp.h"
#include "Events.h"
#include "error_numbers.h"

IMPLEMENT_DYNAMIC_CLASS(CDlgItemProperties, wxDialog)

BEGIN_EVENT_TABLE(CDlgItemProperties, wxDialog)
	//buttons
	EVT_BUTTON(wxID_OK,CDlgItemProperties::OnOK)
END_EVENT_TABLE()

/* Constructor */
CDlgItemProperties::CDlgItemProperties(wxWindow* parent) : CDlgItemPropertiesBase(parent,ID_ANYDIALOG) {
}

void CDlgItemProperties::OnOK(wxCommandEvent& ev) {
	ev.Skip();
}

CDlgItemProperties::~CDlgItemProperties() {
}

void CDlgItemProperties::renderInfos(PROJECT* project) {
	//CMainDocument* pDoc = wxGetApp().GetDocument();
	this->SetTitle(_("BOINC Manager - project properties"));
	wxString html = wxEmptyString;	
	wxString htmltemplate = "<html><body><h3>%s %s</h3><table width=\"100%%\" border=\"0\"> \
							<tr><td colspan=\"2\" bgcolor=\"#EEEEEE\">%s</td><tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%0.0f</td></tr> \
							<tr><td colspan=\"2\" bgcolor=\"#EEEEEE\">%s</td><tr> \
							<tr><td>%s</td><td>%0.2f</td></tr> \
							<tr><td>%s</td><td>%0.2f</td></tr> \
							<tr><td>%s</td><td>%0.2f</td></tr> \
							<tr><td>%s</td><td>%0.2f</td></tr> \
							<tr><td colspan=\"2\" bgcolor=\"#EEEEEE\">%s</td><tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td colspan=\"2\" bgcolor=\"#EEEEEE\">%s</td><tr> \
							<tr><td>%s</td><td>%0.2f</td></tr> \
							<tr><td>%s</td><td>%0.2f</td></tr> \
							<tr><td>%s</td><td>%0.4f</td></tr> \
							<tr><td colspan=\"2\" bgcolor=\"#EEEEEE\">%s</td><tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							</table></body></html>";
	std::string name;
	//collecting infos
	project->get_name(name);
	
	html.Printf(htmltemplate,
			_("project infos for "),name.c_str(),
			_("general infos"),
			_("master url:"),project->master_url.c_str(),
			_("user name:"),project->user_name.c_str(),
			_("team name:"),project->team_name.c_str(),
			_("resource share"),project->resource_share,
			_("credit infos"),
			_("user total credit:"),project->user_total_credit,
			_("user average credit:"),project->user_expavg_credit,
			_("host total credit:"),project->host_total_credit,
			_("host average credit:"),project->host_expavg_credit,
			_("disk usage infos"),
			_("disk usage:"),FormatDiskSpace(project->disk_usage).c_str(),
			_("scheduling infos"),
			_("short term debt:"),project->short_term_debt,
			_("long term debt:"),project->long_term_debt,
			_("duration correction factor:"),project->duration_correction_factor,
			_("diverse infos"),
			_("non cpu intensive:"),project->non_cpu_intensive ? _("yes") : _("no"),
			_("suspended via gui:"),project->suspended_via_gui ? _("yes") : _("no"),
			_("don't request more work:"),project->dont_request_more_work ? _("yes") : _("no"),
			_("scheduler rpc in progress:"),project->scheduler_rpc_in_progress ? _("yes") : _("no"),
			_("attached via account mgr:"),project->attached_via_acct_mgr ? _("yes") : _("no"),
			_("detach when done:"),project->detach_when_done ? _("yes") : _("no"),
			_("ended:"),project->ended ? _("yes") : _("no")
			);
	this->m_html->SetPage(html);
}

void CDlgItemProperties::renderInfos(RESULT* result) {
	this->SetTitle(_("BOINC Manager - task properties"));
	wxString html = wxEmptyString;	
	wxString html1= wxEmptyString;	
	wxString html2= wxEmptyString;	
	wxString html3= wxT("</table></body></html>");
	
	wxString htmltemplate1 = wxT("<html><body><h3>%s %s</h3><table width=\"100%%\" border=\"0\"> \
							<tr><td colspan=\"2\" bgcolor=\"#EEEEEE\">%s</td><tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td>%s</td><td>%s</td></tr> \
							<tr><td colspan=\"2\" bgcolor=\"#EEEEEE\">%s</td><tr> \
							<tr><td>%s</td><td>%s</td></tr>");
							
	wxString htmltemplate2 = wxT("<tr><td colspan=\"2\" bgcolor=\"#EEEEEE\">%s</td><tr> \
							 <tr><td>%s</td><td>%s</td></tr> \
							 <tr><td>%s</td><td>%s</td></tr> \
							 <tr><td>%s</td><td>%s</td></tr> \
							 <tr><td>%s</td><td>%.3f %%</td></tr> \
							 ");

	//first part
	wxString appname = this->FormatApplicationName(result);
	html1.Printf(htmltemplate1,
			_("task infos for "),result->name.c_str(),
			_("general infos"),
			_("application name"),appname.c_str(),
			_("workunit name"),result->wu_name.c_str(), 
			_("state infos"),
			_("state"),FormatStatus(result)
			);
	//second part (only for active tasks available)
	if(result->active_task) {
		html2.Printf(htmltemplate2,
			_("calculation infos"),
			_("checkpoint cpu time"),FormatTime(result->checkpoint_cpu_time),
			_("current cpu time"),FormatTime(result->current_cpu_time),
			_("est. cpu time remaining"),FormatTime(result->estimated_cpu_time_remaining),
			_("fraction done"),floor(result->fraction_done * 100000)/1000
			);
	}
	//concat all parts 
	html.append(html1);
	html.append(html2);
	html.append(html3);

	this->m_html->SetPage(html);
}

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

wxString CDlgItemProperties::FormatApplicationName(RESULT* result ) {
	wxString strBuffer = wxEmptyString;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT* state_result = NULL;
    wxString strLocalBuffer;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (result) {
        state_result = pDoc->state.lookup_result(result->project_url, result->name);
        if (!state_result) {
            pDoc->ForceCacheUpdate();
            state_result = pDoc->state.lookup_result(result->project_url, result->name);
        }
        wxASSERT(state_result);

        wxString strLocale = wxString(setlocale(LC_NUMERIC, NULL), wxConvUTF8);
        setlocale(LC_NUMERIC, "C");
        if (state_result->wup->app->user_friendly_name.size()) {
            strLocalBuffer = wxString(state_result->app->user_friendly_name.c_str(), wxConvUTF8);
        } else {
            strLocalBuffer = wxString(state_result->wup->avp->app_name.c_str(), wxConvUTF8);
        }
        char buf[256];
        if (state_result->wup->avp->plan_class.size()) {
            sprintf(buf, " (%s)", state_result->wup->avp->plan_class.c_str());
        } else {
            strcpy(buf, "");
        }
        strBuffer.Printf(
            wxT(" %s %.2f%s"), 
            strLocalBuffer.c_str(),
            state_result->wup->avp->version_num/100.0,
            buf
        );
        setlocale(LC_NUMERIC, (const char*)strLocale.mb_str());
    }
    return strBuffer;
}

wxString CDlgItemProperties::FormatStatus(RESULT* result) {
	wxString strBuffer= wxEmptyString;
    CMainDocument* doc = wxGetApp().GetDocument();    
    CC_STATUS      status;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    doc->GetCoreClientStatus(status);
    
	int throttled = status.task_suspend_reason & SUSPEND_REASON_CPU_USAGE_LIMIT;
    switch(result->state) {
    case RESULT_NEW:
        strBuffer = _("New"); 
        break;
    case RESULT_FILES_DOWNLOADING:
        if (result->ready_to_report) {
            strBuffer = _("Download failed");
        } else {
            strBuffer = _("Downloading");
        }
        break;
    case RESULT_FILES_DOWNLOADED:
        if (result->project_suspended_via_gui) {
            strBuffer = _("Project suspended by user");
        } else if (result->suspended_via_gui) {
            strBuffer = _("Task suspended by user");
        } else if (status.task_suspend_reason && !throttled) {
            strBuffer = _("Suspended");
            if (status.task_suspend_reason & SUSPEND_REASON_BATTERIES) {
                strBuffer += _(" - on batteries");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_USER_ACTIVE) {
                strBuffer += _(" - user active");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_USER_REQ) {
                strBuffer += _(" - computation suspended");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_TIME_OF_DAY) {
                strBuffer += _(" - time of day");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_BENCHMARKS) {
                strBuffer += _(" - CPU benchmarks");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_DISK_SIZE) {
                strBuffer += _(" - need disk space");
            }
        } else if (result->active_task) {
            if (result->too_large) {
                strBuffer = _("Waiting for memory");
            } else if (result->needs_shmem) {
                strBuffer = _("Waiting for shared memory");
            } else if (result->scheduler_state == CPU_SCHED_SCHEDULED) {
                if (result->edf_scheduled) {
                    strBuffer = _("Running, high priority");
                } else {
                    strBuffer = _("Running");
                }
            } else if (result->scheduler_state == CPU_SCHED_PREEMPTED) {
                strBuffer = _("Waiting to run");
            } else if (result->scheduler_state == CPU_SCHED_UNINITIALIZED) {
                strBuffer = _("Ready to start");
            }
        } else {
            strBuffer = _("Ready to start");
        }
        break;
    case RESULT_COMPUTE_ERROR:
        strBuffer = _("Computation error");
        break;
    case RESULT_FILES_UPLOADING:
        if (result->ready_to_report) {
            strBuffer = _("Upload failed");
        } else {
            strBuffer = _("Uploading");
        }
        break;
    case RESULT_ABORTED:
        switch(result->exit_status) {
        case ERR_ABORTED_VIA_GUI:
            strBuffer = _("Aborted by user");
            break;
        case ERR_ABORTED_BY_PROJECT:
            strBuffer = _("Aborted by project");
            break;
        default:
            strBuffer = _("Aborted");
        }
        break;
    default:
        if (result->got_server_ack) {
            strBuffer = _("Acknowledged");
        } else if (result->ready_to_report) {
            strBuffer = _("Ready to report");
        } else {
            strBuffer.Format(_("Error: invalid state '%d'"), result->state);
        }
        break;
    }
    return strBuffer;
}

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

        strBuffer = wxT(" ") + ts.Format();
    }

    return strBuffer;
}

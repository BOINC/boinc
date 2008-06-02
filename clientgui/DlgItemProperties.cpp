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
	wxString html = wxEmptyString;	
	wxString htmltemplate = "<html><body><h3>%s %s</h3><table width=\"100%%\" border=\"1\"> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%0.2f</td></tr> \
							<tr><td><b>%s</b></td><td>%0.2f</td></tr> \
							<tr><td><b>%s</b></td><td>%0.2f</td></tr> \
							<tr><td><b>%s</b></td><td>%0.2f</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%0.2f</td></tr> \
							<tr><td><b>%s</b></td><td>%0.2f</td></tr> \
							<tr><td><b>%s</b></td><td>%0.4f</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							</table></body></html>";
	std::string name;
	//collecting infos
	project->get_name(name);
	html.Printf(htmltemplate,
			_("project infos for "),name.c_str(),
			_("master url"),project->master_url.c_str(),
			_("user name"),project->user_name.c_str(),
			_("team name"),project->team_name.c_str(),
			_("user total credit"),project->user_total_credit,
			_("user average credit"),project->user_expavg_credit,
			_("host total credit"),project->host_total_credit,
			_("host average credit"),project->host_expavg_credit,
			_("disk usage"),FormatDiskSpace(project->disk_usage).c_str(),
			_("short term debt"),project->short_term_debt,
			_("long term debt"),project->long_term_debt,
			_("duration correction factor"),project->duration_correction_factor,
			_("non cpu intensive"),project->non_cpu_intensive ? _("yes") : _("no"),
			_("suspended via gui"),project->suspended_via_gui ? _("yes") : _("no"),
			_("don't request more work"),project->dont_request_more_work ? _("yes") : _("no"),
			_("scheduler rpc in progress"),project->scheduler_rpc_in_progress ? _("yes") : _("no"),
			_("attached via account mgr"),project->attached_via_acct_mgr ? _("yes") : _("no"),
			_("detach when done"),project->detach_when_done ? _("yes") : _("no"),
			_("ended"),project->ended ? _("yes") : _("no")
			);
	this->m_html->SetPage(html);
}

void CDlgItemProperties::renderInfos(RESULT* result) {
	wxString html = wxEmptyString;	
	wxString htmltemplate = "<html><body><h3>%s %s</h3><table width=\"100%%\" border=\"1\"> \
							<tr><td><b>%s</b></td><td>%s</td></tr> \
							</table></body></html>";
	
	
	wxString appname = this->FormatApplicationName(result);
	html.Printf(htmltemplate,
			_("task infos for "),result->name.c_str(),
			_("application name"),appname.c_str()
			);
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


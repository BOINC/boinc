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
#pragma implementation "DlgAdvPreferences.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "SkinManager.h"
#include "Events.h"
#include "error_numbers.h"
#include "version.h"
#include "DlgAdvPreferences.h"

#include "res/usage.xpm"
#include "res/xfer.xpm"
#include "res/proj.xpm"
#include "res/warning.xpm"


using std::string;

IMPLEMENT_DYNAMIC_CLASS(CDlgAdvPreferences, wxDialog)

BEGIN_EVENT_TABLE(CDlgAdvPreferences, wxDialog)
	EVT_COMMAND_RANGE(20000,21000,wxEVT_COMMAND_CHECKBOX_CLICKED,CDlgAdvPreferences::OnHandleCommandEvent)
	EVT_COMMAND_RANGE(20000,21000,wxEVT_COMMAND_RADIOBUTTON_SELECTED,CDlgAdvPreferences::OnHandleCommandEvent)
	EVT_COMMAND_RANGE(20000,21000,wxEVT_COMMAND_TEXT_UPDATED,CDlgAdvPreferences::OnHandleCommandEvent)
    // list box
    EVT_COMMAND(ID_LISTBOX_EXCLAPPS,wxEVT_COMMAND_LISTBOX_SELECTED,CDlgAdvPreferences::OnExclusiveAppListEvent)
	//buttons
	EVT_BUTTON(ID_ADDEXCLUSIVEAPPBUTTON,CDlgAdvPreferences::OnAddExclusiveApp)
	EVT_BUTTON(ID_REMOVEEXCLUSIVEAPPBUTTON,CDlgAdvPreferences::OnRemoveExclusiveApp)
	EVT_BUTTON(wxID_OK,CDlgAdvPreferences::OnOK)
	EVT_BUTTON(ID_HELPBOINC,CDlgAdvPreferences::OnHelp)
	EVT_BUTTON(ID_BTN_CLEAR,CDlgAdvPreferences::OnClear)
END_EVENT_TABLE()

/* Constructor */
CDlgAdvPreferences::CDlgAdvPreferences(wxWindow* parent) : CDlgAdvPreferencesBase(parent,ID_ANYDIALOG) {
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

	m_bInInit=false;
	m_bPrefsDataChanged=false;
    m_bExclusiveAppsDataChanged=false;
	m_arrTabPageIds.Add(ID_TABPAGE_PROC);
	m_arrTabPageIds.Add(ID_TABPAGE_NET);
	m_arrTabPageIds.Add(ID_TABPAGE_DISK);
	m_arrTabPageIds.Add(ID_TABPAGE_EXCLAPPS);
	
	//setting tab page images (not handled by generated code)
    int iImageIndex = 0;
    wxImageList* pImageList = m_Notebook->GetImageList();
    if (!pImageList) {
        pImageList = new wxImageList(16, 16, true, 0);
        wxASSERT(pImageList != NULL);
        m_Notebook->SetImageList(pImageList);
    }
    iImageIndex = pImageList->Add(wxBitmap(proj_xpm));
	m_Notebook->SetPageImage(0,iImageIndex);

    iImageIndex = pImageList->Add(wxBitmap(xfer_xpm));
	m_Notebook->SetPageImage(1,iImageIndex);

    iImageIndex = pImageList->Add(wxBitmap(usage_xpm));
	m_Notebook->SetPageImage(2,iImageIndex);
    
    iImageIndex = pImageList->Add(*pSkinAdvanced->GetApplicationSnoozeIcon());
	m_Notebook->SetPageImage(3,iImageIndex);
        
	//setting warning bitmap
	m_bmpWarning->SetBitmap(wxBitmap(warning_xpm));
    
    m_removeExclusiveAppButton->Disable();

	// init special tooltips
	SetSpecialTooltips();
	//setting the validators for correct input handling
	SetValidators();
	//read in settings and initialisze controls
	ReadPreferenceSettings();
	//
	RestoreState();
}

/* destructor */
CDlgAdvPreferences::~CDlgAdvPreferences() {
	SaveState();
}

/* set validators for input filtering purposes only */
void CDlgAdvPreferences::SetValidators() {
	//proc page
	m_txtProcIdleFor->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtMaxLoad->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtProcSwitchEvery->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtProcUseProcessors->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtProcUseCPUTime->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	//net page
	m_txtNetConnectInterval->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtNetDownloadRate->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txt_daily_xfer_limit_mb->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txt_daily_xfer_period_days->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtNetUploadRate->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtNetAdditionalDays->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	//disk and memory page
	m_txtDiskMaxSpace->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtDiskLeastFree->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtDiskMaxOfTotal->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtDiskWriteToDisk->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtDiskMaxSwap->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtMemoryMaxInUse->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtMemoryMaxOnIdle->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	
}

/* some controls share the same tooltip, set them here */
void CDlgAdvPreferences::SetSpecialTooltips() {
	m_txtProcMonday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
	m_txtProcTuesday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
	m_txtProcWednesday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
	m_txtProcThursday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
	m_txtProcFriday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
	m_txtProcSaturday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
	m_txtProcSunday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
	//
	m_txtNetMonday->SetToolTip(TXT_NET_TIME_TOOLTIP);
	m_txtNetTuesday->SetToolTip(TXT_NET_TIME_TOOLTIP);
	m_txtNetWednesday->SetToolTip(TXT_NET_TIME_TOOLTIP);
	m_txtNetThursday->SetToolTip(TXT_NET_TIME_TOOLTIP);
	m_txtNetFriday->SetToolTip(TXT_NET_TIME_TOOLTIP);
	m_txtNetSaturday->SetToolTip(TXT_NET_TIME_TOOLTIP);
	m_txtNetSunday->SetToolTip(TXT_NET_TIME_TOOLTIP);
}

/* saves selected tab page */
bool CDlgAdvPreferences::SaveState() {
    wxString        strBaseConfigLocation = wxString(wxT("/DlgAdvPreferences/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);
	if (!pConfig) return false;

	pConfig->SetPath(strBaseConfigLocation);
	pConfig->Write(wxT("CurrentPage"),m_Notebook->GetSelection());
	return true;
}

/* restores former selected tab page */
bool CDlgAdvPreferences::RestoreState() {
    wxString        strBaseConfigLocation = wxString(wxT("/DlgAdvPreferences/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
	int				p;

	wxASSERT(pConfig);

    if (!pConfig) return false;

	pConfig->SetPath(strBaseConfigLocation);

	pConfig->Read(wxT("CurrentPage"), &p,0);
	m_Notebook->SetSelection(p);	

	return true;
}

// convert a Timestring HH:MM into a double
double CDlgAdvPreferences::TimeStringToDouble(wxString timeStr) {
	double hour;
	double minutes;
	timeStr.SubString(0,timeStr.First(':')).ToDouble(&hour);
	timeStr.SubString(timeStr.First(':')+1,timeStr.Length()).ToDouble(&minutes);
	minutes = minutes/60.0;
	return hour + minutes;
}

// convert a double into a timestring HH:MM
wxString CDlgAdvPreferences::DoubleToTimeString(double dt) {
	int hour = (int)dt;
	int minutes = (int)(60.0 * (dt - hour)+.5);
	return wxString::Format(wxT("%02d:%02d"),hour,minutes);
}

/* read preferences from core client and initialize control values */
void CDlgAdvPreferences::ReadPreferenceSettings() {
	m_bInInit=true;//prevent dialog handlers from doing anything 
    CMainDocument* pDoc = wxGetApp().GetDocument();    	
	wxString buffer;
	int retval;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	// Get current working preferences (including any overrides) from client
    retval = pDoc->rpc.get_global_prefs_working_struct(prefs, mask);
    if (retval == ERR_NOT_FOUND) {
        // Older clients don't support get_global_prefs_working_struct RPC
        prefs = pDoc->state.global_prefs;
        pDoc->rpc.get_global_prefs_override_struct(prefs, mask);
    }

	// ######### proc usage page
	// do work between
	*m_txtProcEveryDayStart << DoubleToTimeString(prefs.cpu_times.start_hour);	
	*m_txtProcEveryDayStop << DoubleToTimeString(prefs.cpu_times.end_hour);
	//special day times
	wxCheckBox* aChks[] = {m_chkProcSunday,m_chkProcMonday,m_chkProcTuesday,m_chkProcWednesday,m_chkProcThursday,m_chkProcFriday,m_chkProcSaturday};
	wxTextCtrl* aTxts[] = {m_txtProcSunday,m_txtProcMonday,m_txtProcTuesday,m_txtProcWednesday,m_txtProcThursday,m_txtProcFriday,m_txtProcSaturday};
	for(int i=0; i< 7;i++) {
        TIME_SPAN& cpu = prefs.cpu_times.week.days[i];
		if(cpu.present) {
			aChks[i]->SetValue(true);
			wxString timeStr = DoubleToTimeString(cpu.start_hour) +
								wxT("-") + DoubleToTimeString(cpu.end_hour);
			aTxts[i]->SetValue(timeStr);
		}
	}
	
	// on batteries
	m_chkProcOnBatteries->SetValue(prefs.run_on_batteries);
	// in use
	m_chkProcInUse->SetValue(prefs.run_if_user_active);
    m_chkGPUProcInUse->SetValue(prefs.run_gpu_if_user_active);
	// idle for X minutes
	buffer.Printf(wxT("%.2f"),prefs.idle_time_to_run);
	*m_txtProcIdleFor << buffer;

	buffer.Printf(wxT("%.0f"), prefs.suspend_cpu_usage);
	*m_txtMaxLoad << buffer;

	// switch every X minutes
	buffer.Printf(wxT("%.2f"),prefs.cpu_scheduling_period_minutes);
	*m_txtProcSwitchEvery << buffer;
	// max cpus
	buffer.Printf(wxT("%.2f"),prefs.max_ncpus_pct);
	*m_txtProcUseProcessors << buffer;
	//cpu limit
	buffer.Printf(wxT("%.2f"),prefs.cpu_usage_limit);
	*m_txtProcUseCPUTime << buffer;

	// ######### net usage page
	// use network between
	*m_txtNetEveryDayStart << DoubleToTimeString(prefs.net_times.start_hour);
	*m_txtNetEveryDayStop << DoubleToTimeString(prefs.net_times.end_hour);
	//special day times
	wxCheckBox* aChks2[] = {m_chkNetSunday,m_chkNetMonday,m_chkNetTuesday,m_chkNetWednesday,m_chkNetThursday,m_chkNetFriday,m_chkNetSaturday};
	wxTextCtrl* aTxts2[] = {m_txtNetSunday,m_txtNetMonday,m_txtNetTuesday,m_txtNetWednesday,m_txtNetThursday,m_txtNetFriday,m_txtNetSaturday};
	for(int i=0; i< 7;i++) {
        TIME_SPAN& net = prefs.net_times.week.days[i];
		if(net.present) {
			aChks2[i]->SetValue(true);
			wxString timeStr = DoubleToTimeString(net.start_hour) +
								wxT("-") + DoubleToTimeString(net.end_hour);
			aTxts2[i]->SetValue(timeStr);
		}
	}
	// connection interval
	buffer.Printf(wxT("%01.2f"),prefs.work_buf_min_days);
	*m_txtNetConnectInterval << buffer;
	//download rate
	buffer.Printf(wxT("%.2f"),prefs.max_bytes_sec_down / 1024);
	*m_txtNetDownloadRate << buffer;
	// upload rate
	buffer.Printf(wxT("%.2f"),prefs.max_bytes_sec_up / 1024);
	*m_txtNetUploadRate << buffer;

	buffer.Printf(wxT("%.2f"),prefs.daily_xfer_limit_mb);
	*m_txt_daily_xfer_limit_mb << buffer;
	buffer.Printf(wxT("%d"),prefs.daily_xfer_period_days );
	*m_txt_daily_xfer_period_days << buffer;

	//
	buffer.Printf(wxT("%.2f"),prefs.work_buf_additional_days);
	*m_txtNetAdditionalDays << buffer;
	// skip image verification
	m_chkNetSkipImageVerification->SetValue(prefs.dont_verify_images);
	// confirm before connect
	m_chkNetConfirmBeforeConnect->SetValue(prefs.confirm_before_connecting);
	// disconnect when done
	m_chkNetDisconnectWhenDone->SetValue(prefs.hangup_if_dialed);

	// ######### disk and memory usage page
	//max space used
	buffer.Printf(wxT("%.2f"),prefs.disk_max_used_gb);
	*m_txtDiskMaxSpace << buffer;
	// min free 
	buffer.Printf(wxT("%.2f"),prefs.disk_min_free_gb);
	*m_txtDiskLeastFree << buffer;
	// max used percentage
	buffer.Printf(wxT("%.2f"),prefs.disk_max_used_pct);
	*m_txtDiskMaxOfTotal << buffer;
	// write to disk every X seconds
	buffer.Printf(wxT("%.0f"),prefs.disk_interval);	
	*m_txtDiskWriteToDisk << buffer;
	// max swap space (virtual memory)
	buffer.Printf(wxT("%.2f"),prefs.vm_max_used_frac*100.0);
	*m_txtDiskMaxSwap << buffer;
	// max VM used
	buffer.Printf(wxT("%.2f"),prefs.ram_max_used_busy_frac*100.0);
	*m_txtMemoryMaxInUse << buffer;
	// max VM idle
	buffer.Printf(wxT("%.2f"),prefs.ram_max_used_idle_frac*100.0);
	*m_txtMemoryMaxOnIdle << buffer;
	// suspend to memory
	m_chkMemoryWhileSuspended->SetValue(prefs.leave_apps_in_memory);
    
    // Get cc_config.xml file flags
    log_flags.init();
    config.defaults();
    retval = pDoc->rpc.get_cc_config(config, log_flags);
    if (!retval) {
        for (unsigned int i=0; i<config.exclusive_apps.size(); ++i) {
            wxString appName = wxString(config.exclusive_apps[i].c_str(), wxConvUTF8);
            m_exclusiveApsListBox->Append(appName);
        }
    }

	m_bInInit=false;
	//update control states
	this->UpdateControlStates();
}

void clamp_pct(double& x) {
    if (x < 0) x = 0;
    if (x > 100) x = 100;
}

/* write overridden preferences to disk (global_prefs_override.xml) */
bool CDlgAdvPreferences::SavePreferencesSettings() {
	double td;

	mask.clear();
	//clear special times settings
    prefs.cpu_times.week.clear();
    prefs.net_times.week.clear();
	//proc page
	prefs.run_on_batteries=m_chkProcOnBatteries->GetValue();
	mask.run_on_batteries=true;
	//
	prefs.run_if_user_active=m_chkProcInUse->GetValue();
	mask.run_if_user_active=true;

	prefs.run_gpu_if_user_active=m_chkGPUProcInUse->GetValue();
	mask.run_gpu_if_user_active=true;
	//
	if(m_txtProcIdleFor->IsEnabled()) {
		m_txtProcIdleFor->GetValue().ToDouble(&td);
		prefs.idle_time_to_run=td;
		mask.idle_time_to_run=true;
	}

    m_txtMaxLoad->GetValue().ToDouble(&td);
    prefs.suspend_cpu_usage=td;
    mask.suspend_cpu_usage=true;

	//
	prefs.cpu_times.start_hour=TimeStringToDouble(m_txtProcEveryDayStart->GetValue());
	mask.start_hour = true;        
	//
	prefs.cpu_times.end_hour=TimeStringToDouble(m_txtProcEveryDayStop->GetValue());
	mask.end_hour = true;        
	//
	wxCheckBox* aChks[] = {m_chkProcSunday,m_chkProcMonday,m_chkProcTuesday,m_chkProcWednesday,m_chkProcThursday,m_chkProcFriday,m_chkProcSaturday};
	wxTextCtrl* aTxts[] = {m_txtProcSunday,m_txtProcMonday,m_txtProcTuesday,m_txtProcWednesday,m_txtProcThursday,m_txtProcFriday,m_txtProcSaturday};
	for(int i=0; i< 7;i++) {
		if(aChks[i]->GetValue()) {
			wxString timeStr = aTxts[i]->GetValue();
			wxString startStr = timeStr.SubString(0,timeStr.First('-'));
			wxString endStr = timeStr.SubString(timeStr.First('-')+1,timeStr.Length());
            prefs.cpu_times.week.set(i,
			    TimeStringToDouble(startStr),
			    TimeStringToDouble(endStr)
                );
		}
	}
	m_txtProcSwitchEvery->GetValue().ToDouble(&td);
	prefs.cpu_scheduling_period_minutes=td;
	mask.cpu_scheduling_period_minutes=true;
	//

	m_txtProcUseProcessors->GetValue().ToDouble(&td);
    clamp_pct(td);
	prefs.max_ncpus_pct=td;
	mask.max_ncpus_pct=true;

	//
	m_txtProcUseCPUTime->GetValue().ToDouble(&td);
	prefs.cpu_usage_limit=td;
	mask.cpu_usage_limit=true;
	// network page
	m_txtNetConnectInterval->GetValue().ToDouble(&td);
	prefs.work_buf_min_days=td;
	mask.work_buf_min_days=true;
	//
	m_txtNetDownloadRate->GetValue().ToDouble(&td);
	td = td * 1024;
	prefs.max_bytes_sec_down=td;
	mask.max_bytes_sec_down=true;
	//
	m_txtNetUploadRate->GetValue().ToDouble(&td);
	td = td * 1024;
	prefs.max_bytes_sec_up=td;
	mask.max_bytes_sec_up=true;

    m_txt_daily_xfer_limit_mb->GetValue().ToDouble(&td);
	prefs.daily_xfer_limit_mb=td;
	mask.daily_xfer_limit_mb=true;
    m_txt_daily_xfer_period_days->GetValue().ToDouble(&td);
	prefs.daily_xfer_period_days=(int)td;
	mask.daily_xfer_period_days=true;
	//
	prefs.dont_verify_images=m_chkNetSkipImageVerification->GetValue();
	mask.dont_verify_images=true;
	//
	prefs.confirm_before_connecting= m_chkNetConfirmBeforeConnect->GetValue();
	mask.confirm_before_connecting=true;
	//
	prefs.hangup_if_dialed= m_chkNetDisconnectWhenDone->GetValue();
	mask.hangup_if_dialed=true;
	//
	m_txtNetAdditionalDays->GetValue().ToDouble(&td);
	prefs.work_buf_additional_days = td;
	mask.work_buf_additional_days = true;
	//
	prefs.net_times.start_hour=TimeStringToDouble(m_txtNetEveryDayStart->GetValue());
	mask.net_start_hour = true;        
	//
	prefs.net_times.end_hour=TimeStringToDouble(m_txtNetEveryDayStop->GetValue());
	mask.net_end_hour = true;        
		
	wxCheckBox* aChks2[] = {m_chkNetSunday,m_chkNetMonday,m_chkNetTuesday,m_chkNetWednesday,m_chkNetThursday,m_chkNetFriday,m_chkNetSaturday};
	wxTextCtrl* aTxts2[] = {m_txtNetSunday,m_txtNetMonday,m_txtNetTuesday,m_txtNetWednesday,m_txtNetThursday,m_txtNetFriday,m_txtNetSaturday};
	for(int i=0; i< 7;i++) {
		if(aChks2[i]->GetValue()) {
			wxString timeStr = aTxts2[i]->GetValue();
			wxString startStr = timeStr.SubString(0,timeStr.First('-'));
			wxString endStr = timeStr.SubString(timeStr.First('-')+1,timeStr.Length());
            prefs.net_times.week.set(i,
			    TimeStringToDouble(startStr),
			    TimeStringToDouble(endStr)
                );
        }
	}
	//disk usage
	m_txtDiskMaxSpace->GetValue().ToDouble(&td);
	prefs.disk_max_used_gb=td;
	mask.disk_max_used_gb=true;
	//
	m_txtDiskLeastFree->GetValue().ToDouble(&td);
	prefs.disk_min_free_gb=td;
	mask.disk_min_free_gb=true;
	//
	m_txtDiskMaxOfTotal->GetValue().ToDouble(&td);
    clamp_pct(td);
	prefs.disk_max_used_pct=td;
	mask.disk_max_used_pct=true;
	//
	m_txtDiskWriteToDisk->GetValue().ToDouble(&td);
	prefs.disk_interval=td;
	mask.disk_interval=true;
	//
	m_txtDiskMaxSwap->GetValue().ToDouble(&td);
    clamp_pct(td);
	td = td / 100.0 ;
	prefs.vm_max_used_frac=td;
	mask.vm_max_used_frac=true;
	//Memory
	m_txtMemoryMaxInUse->GetValue().ToDouble(&td);
    clamp_pct(td);
	td = td / 100.0;
	prefs.ram_max_used_busy_frac=td;
	mask.ram_max_used_busy_frac=true;
	//
	m_txtMemoryMaxOnIdle->GetValue().ToDouble(&td);
    clamp_pct(td);
	td = td / 100.0;
	prefs.ram_max_used_idle_frac=td;
	mask.ram_max_used_idle_frac=true;
	//
	prefs.leave_apps_in_memory = m_chkMemoryWhileSuspended->GetValue();
	mask.leave_apps_in_memory=true;
    
    if (m_bExclusiveAppsDataChanged) {
        wxArrayString appNames = m_exclusiveApsListBox->GetStrings();

        config.exclusive_apps.clear();
        for (unsigned int i=0; i<appNames.size(); ++i) {
            std::string s = (const char*)appNames[i].mb_str();
            config.exclusive_apps.push_back(s);
        }
    }

	return true;
}

/* set state of control depending on other control's state */
void CDlgAdvPreferences::UpdateControlStates() {
	//proc usage page
	m_txtProcIdleFor->Enable(!m_chkProcInUse->IsChecked() || !m_chkGPUProcInUse->IsChecked());
	m_txtProcMonday->Enable(m_chkProcMonday->IsChecked());
	m_txtProcTuesday->Enable(m_chkProcTuesday->IsChecked());
	m_txtProcWednesday->Enable(m_chkProcWednesday->IsChecked());
	m_txtProcThursday->Enable(m_chkProcThursday->IsChecked());
	m_txtProcFriday->Enable(m_chkProcFriday->IsChecked());
	m_txtProcSaturday->Enable(m_chkProcSaturday->IsChecked());
	m_txtProcSunday->Enable(m_chkProcSunday->IsChecked());

	//net usage page
	m_txtNetMonday->Enable(m_chkNetMonday->IsChecked());
	m_txtNetTuesday->Enable(m_chkNetTuesday->IsChecked());
	m_txtNetWednesday->Enable(m_chkNetWednesday->IsChecked());
	m_txtNetThursday->Enable(m_chkNetThursday->IsChecked());
	m_txtNetFriday->Enable(m_chkNetFriday->IsChecked());
	m_txtNetSaturday->Enable(m_chkNetSaturday->IsChecked());
	m_txtNetSunday->Enable(m_chkNetSunday->IsChecked());
}

/* validates the entered informations */
bool CDlgAdvPreferences::ValidateInput() {
	wxString invMsgFloat = _("invalid float");
	wxString invMsgTime = _("invalid time, format is HH:MM");
	wxString invMsgInterval = _("invalid time interval, format is HH:MM-HH:MM");
	wxString buffer;
	//proc page
	if(m_txtProcIdleFor->IsEnabled()) {
		buffer = m_txtProcIdleFor->GetValue();
		if(!IsValidFloatValue(buffer)) {
			ShowErrorMessage(invMsgFloat,m_txtProcIdleFor);
			return false;
		}
	}
    buffer = m_txtMaxLoad->GetValue();
    if(!IsValidFloatValue(buffer)) {
        ShowErrorMessage(invMsgFloat, m_txtMaxLoad);
        return false;
    }
	
	buffer = m_txtProcEveryDayStart->GetValue();
	if(!IsValidTimeValue(buffer)) {
		ShowErrorMessage(invMsgTime,m_txtProcEveryDayStart);
		return false;
	}
	buffer = m_txtProcEveryDayStop->GetValue();
	if(!IsValidTimeValue(buffer)) {
		ShowErrorMessage(invMsgTime,m_txtProcEveryDayStop);
		return false;
	}
	//all text ctrls in proc special time panel
	wxWindowList children = m_panelProcSpecialTimes->GetChildren();
    wxWindowList::compatibility_iterator node = children.GetFirst();
	while(node) {
		if(node->GetData()->IsKindOf(CLASSINFO(wxTextCtrl))) {
			wxTextCtrl*  txt = wxDynamicCast(node->GetData(),wxTextCtrl);
			if(txt) {
				if(txt->IsEnabled()) {
					buffer = txt->GetValue();
					if(!IsValidTimeIntervalValue(buffer)) {
						ShowErrorMessage(invMsgInterval,txt);
						return false;
					}
				}
			}
		}
		node = node->GetNext();
	}		
	//net page
	buffer = m_txtNetEveryDayStart->GetValue();
	if(!IsValidTimeValue(buffer)) {
		ShowErrorMessage(invMsgTime,m_txtNetEveryDayStart);
		return false;
	}

	buffer = m_txtNetEveryDayStop->GetValue();
	if(!IsValidTimeValue(buffer)) {
		ShowErrorMessage(invMsgTime,m_txtNetEveryDayStop);
		return false;
	}

    //limit additional days from 0 to 10
	double td;
	m_txtNetConnectInterval->GetValue().ToDouble(&td);
	if(td>10.0 || td < 0.0) {
		ShowErrorMessage(invMsgFloat,m_txtNetConnectInterval);
		return false;
	}
	m_txtNetAdditionalDays->GetValue().ToDouble(&td);
	if(td>10.0 || td < 0.0) {
		ShowErrorMessage(invMsgFloat,m_txtNetAdditionalDays);
		return false;
	}

    //all text ctrls in net special time panel

	children = m_panelNetSpecialTimes->GetChildren();
	node = children.GetFirst();
	while(node) {
		if(node->GetData()->IsKindOf(CLASSINFO(wxTextCtrl))) {
			wxTextCtrl*  txt = wxDynamicCast(node->GetData(),wxTextCtrl);
			if(txt) {
				if(txt->IsEnabled()) {
					buffer = txt->GetValue();
					if(!IsValidTimeIntervalValue(buffer)) {
						ShowErrorMessage(invMsgInterval,txt);
						return false;
					}
				}//if(txt->IsEnabled())
			}//if(txt)
		}//if(node->GetData()
		node = node->GetNext();
	}		

	return true;
}

/* ensures that the page which contains txtCtrl is selected */
bool CDlgAdvPreferences::EnsureTabPageVisible(wxTextCtrl* txtCtrl) {	
	wxWindow* parent = txtCtrl->GetParent();
	wxASSERT(parent);
	int parentid = parent->GetId();
	int index = m_arrTabPageIds.Index(parentid);
	if(index == wxNOT_FOUND) {
		//some controls are containe din a additional panel, so look at its parent
		parent = parent->GetParent();
		wxASSERT(parent);
		parentid = parent->GetId();
		index = m_arrTabPageIds.Index(parentid);
		if(index == wxNOT_FOUND) {
			//this should never happen
			return false;
		}
	}
	m_Notebook->SetSelection(index);
	return true;
}

/* show an error message and set the focus to the control that caused the error */
void CDlgAdvPreferences::ShowErrorMessage(wxString& message,wxTextCtrl* errorCtrl) {
	wxASSERT(this->EnsureTabPageVisible(errorCtrl));
	errorCtrl->SetFocus();	
	//
	if(message.IsEmpty()){
		message = _("invalid input value detected");
	}
	wxGetApp().SafeMessageBox(message,_("Validation Error"),wxOK | wxCENTRE | wxICON_ERROR,this);
}

/* checks if ch is a valid character for float values */
bool CDlgAdvPreferences::IsValidFloatChar(const wxChar& ch) {
	//don't accept the e
	return wxIsdigit(ch) || ch=='.' || ch==',' || ch=='+' || ch=='-';
}

/* checks if ch is a valid character for time values */
bool CDlgAdvPreferences::IsValidTimeChar(const wxChar& ch) {	
	return wxIsdigit(ch) || ch==':';
}

/* checks if ch is a valid character for time interval values */
bool CDlgAdvPreferences::IsValidTimeIntervalChar(const wxChar& ch) {	
	return IsValidTimeChar(ch) || ch=='-';
}

/* checks if the value contains a valid float */
bool CDlgAdvPreferences::IsValidFloatValue(const wxString& value) {
	for(unsigned int i=0; i < value.Length();i++) {
		if(!IsValidFloatChar(value[i])) {
			return false;
		}
	}
	//all chars are valid, now what is with the value as a whole ?
	double td;
	if(!value.ToDouble(&td)) {
		return false;
	}
	return true;
}

/* checks if the value is a valid time */
bool CDlgAdvPreferences::IsValidTimeValue(const wxString& value) {
	for(unsigned int i=0; i < value.Length();i++) {
		if(!IsValidTimeChar(value[i])) {
			return false;
		}
	}
	//all chars are valid, now what is with the value as a whole ?
	wxDateTime dt;
	const wxChar* stopChar = dt.ParseFormat(value,wxT("%H:%M"));
	if(stopChar==NULL && value != wxT("24:00")) {
        // conversion failed
		return false;
	}
	return true;
}

/* checks if the value is a valid time interval, format HH:MM-HH:MM */
bool CDlgAdvPreferences::IsValidTimeIntervalValue(const wxString& value) {
	for(unsigned int i=0; i < value.Length();i++) {
		if(!IsValidTimeIntervalChar(value[i])) {
			return false;
		}
	}
	//all chars are valid, now what is with the value as a whole ?
	//check for -
	if(value.Find('-')<0) {
		return false;
	}
	//split up into start and stop
	wxString start = value.BeforeFirst('-');
	wxString stop = value.AfterFirst('-');	
	//validate start and stop parts
	if(!IsValidTimeValue(start) || !IsValidTimeValue(stop)) {
		return false;
	}
	//ensure that start is lower than stop
	wxDateTime dtStart,dtStop;
	dtStart.ParseFormat(start,wxT("%H:%M"));
	dtStop.ParseFormat(stop,wxT("%H:%M"));
	//
	/*if(dtStart>=dtStop) {
		return false;
	}*/
	return true;
}

// ------------ Event handlers starts here
// -------- generic command handler
// handles all control command events 
void CDlgAdvPreferences::OnHandleCommandEvent(wxCommandEvent& ev) {
	ev.Skip();
	if(!m_bInInit) {
		m_bPrefsDataChanged=true;
	}
	UpdateControlStates();
}

// ---- Exclusive Apps list box handler
void CDlgAdvPreferences::OnExclusiveAppListEvent(wxCommandEvent& ev) {
    wxArrayInt selections;
    int numSelected;
    
	if(!m_bInInit) {
        numSelected = m_exclusiveApsListBox->GetSelections(selections);
        m_removeExclusiveAppButton->Enable(numSelected > 0);
    }
	ev.Skip();
}

// ---- command buttons handlers
// handles Add button clicked
void CDlgAdvPreferences::OnAddExclusiveApp(wxCommandEvent&) {
    wxString strMachineName;
    int i, j, n;
    bool hostIsMac = false;
    bool hostIsWin = false;
    bool isDuplicate;
    wxArrayString appNames;
    wxChar *extension = wxT("");
    wxString errmsg;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (strstr(pDoc->state.host_info.os_name, "Darwin")) {
        hostIsMac = true;
        extension = wxT(".app");
    } else if (strstr(pDoc->state.host_info.os_name, "Microsoft")) {
        hostIsWin = true;
        extension = wxT(".exe");
    }
    
    pDoc->GetConnectedComputerName(strMachineName);
    if (pDoc->IsComputerNameLocal(strMachineName)) {
#ifdef __WXMAC__
        wxFileDialog picker(this, _("Applications to add"), 
                            wxT("/Applications"), wxT(""), wxT("*.app"), 
                            wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR|wxFD_MULTIPLE|wxFD_CHANGE_DIR);
#elif defined(__WXMSW__)
//TODO: fill in the default directory for MSW
        wxFileDialog picker(this, _("Applications to add"), 
                            wxT("C:/Program Files"), wxT(""), wxT("*.exe"), 
                            wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR|wxFD_MULTIPLE|wxFD_CHANGE_DIR);
#else
//TODO: fill in the default directory and wildcard for Linux
        wxFileDialog picker(this, _("Applications to add"), 
                            wxT("/"), wxT(""), wxT("*.*"), 
                            wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR|wxFD_MULTIPLE|wxFD_CHANGE_DIR);
#endif
        if (picker.ShowModal() != wxID_OK) return;
        picker.GetFilenames(appNames);

        for (i=appNames.Count()-1; i>=0; --i) {
#ifdef __WXMSW__
            // Under Windows, filename may include paths if a shortcut selected
            wxString appNameOnly = appNames[i].AfterLast('\\');
            appNames[i] = appNameOnly;
#endif
            wxString directory = picker.GetDirectory();
            wxFileName fn(directory, appNames[i]);
            if (!fn.IsOk() || !fn.IsFileExecutable()) {
                errmsg.Printf(_("'%s' is not an executable application."), appNames[i].c_str());
                wxGetApp().SafeMessageBox(errmsg, _("Add Exclusive App"),
                            wxOK | wxICON_EXCLAMATION, this);
                appNames.RemoveAt(i);
                continue;
            }
        }
    } else {
        // We can't use file picker if connected to a remote computer, 
        // so show a dialog with textedit field so user can type app name
        wxChar path_separator = wxT('/');
        
        wxTextEntryDialog dlg(this, _("Name of application to add?"), _("Add exclusive app"));
        if (hostIsMac) {
            dlg.SetValue(extension);
        } else if (hostIsWin) {
            dlg.SetValue(extension);
            path_separator = wxT('\\');
        }
        if (dlg.ShowModal() != wxID_OK) return;
        
        wxString theAppName = dlg.GetValue();
        // Strip off path if present
        appNames.Add(theAppName.AfterLast(path_separator));
        
    }
        
    for (i=0; i<(int)appNames.Count(); ++i) {
        // wxFileName::IsFileExecutable() doesn't seem to work on Windows, 
        // and we can only perform minimal validation on remote hosts, so 
        // check filename extension on Mac and Win
        if (hostIsMac || hostIsWin) {
            if (!appNames[0].EndsWith(extension)) {
                errmsg.Printf(_("Application names must end with '%s'"), extension);
                wxGetApp().SafeMessageBox(errmsg, _("Add Exclusive App"),
                            wxOK | wxICON_EXCLAMATION, this);
                return;
            }
        }

        if (hostIsMac) {
            int suffix = appNames[i].Find('.', true);
            if (suffix != wxNOT_FOUND) {
                appNames[i].Truncate(suffix);
            }
        }

        // Skip requests for duplicate entries
        isDuplicate = false;
        n = m_exclusiveApsListBox->GetCount();
        for (j=0; j<n; ++j) {
            if ((m_exclusiveApsListBox->GetString(j)).Cmp(appNames[i]) == 0) {
                isDuplicate = true;
                break;
            }
        }
        if (isDuplicate) {
            errmsg.Printf(_("'%s' is already in the list."), appNames[i].c_str());
            wxGetApp().SafeMessageBox(errmsg, _("Add Exclusive App"),
                        wxOK | wxICON_EXCLAMATION, this);
            continue;
        }
        
        m_exclusiveApsListBox->Append(appNames[i]);
        m_bExclusiveAppsDataChanged = true;
    }
}

static int myCompareInts(int *first, int *second) {
    return *first - *second;
}

typedef int (*sortcomparefunc)(int*, int*);

// handles Remove button clicked
void CDlgAdvPreferences::OnRemoveExclusiveApp(wxCommandEvent& ev) {
    wxArrayInt selections;
    int numSelected = m_exclusiveApsListBox->GetSelections(selections);
    
    // The selection indices are returned in random order.
    // We must sort them to ensure deleting the correct items.
    selections.Sort((sortcomparefunc)&myCompareInts);
    for (int i=numSelected-1; i>=0; --i) {
        m_exclusiveApsListBox->Delete(selections[i]);
        m_bExclusiveAppsDataChanged = true;
    }
	ev.Skip();
}

// handles OK button clicked
void CDlgAdvPreferences::OnOK(wxCommandEvent& ev) {
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	if(!ValidateInput()) {
		return;
	}
	if(SavePreferencesSettings()) {
		pDoc->rpc.set_global_prefs_override_struct(prefs,mask);		
		pDoc->rpc.read_global_prefs_override();
	}
    
    if (m_bExclusiveAppsDataChanged) {
        int retval = pDoc->rpc.set_cc_config(config, log_flags);
        if (!retval) {
            pDoc->rpc.read_cc_config();
        }
    }
	ev.Skip();
}

// handles Help button clicked
void CDlgAdvPreferences::OnHelp(wxCommandEvent& ev) {
    if (IsShown()) {

	    wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

		wxString wxurl;
		wxurl.Printf(
            wxT("%s?target=advanced_preferences&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            ev.GetId()
        );
        wxLaunchDefaultBrowser(wxurl);
    }
}

// handles Clear button clicked 
void CDlgAdvPreferences::OnClear(wxCommandEvent& ev) {
	if(this->ConfirmClear()) {
		CMainDocument*    pDoc = wxGetApp().GetDocument();

		wxASSERT(pDoc);
		wxASSERT(wxDynamicCast(pDoc, CMainDocument));

		mask.clear();
		pDoc->rpc.set_global_prefs_override_struct(prefs,mask);		
		pDoc->rpc.read_global_prefs_override();
		this->EndModal(wxID_CANCEL);
	}
	ev.Skip();
}

bool CDlgAdvPreferences::ConfirmClear() {
	int res = wxGetApp().SafeMessageBox(_(
	    "Do you really want to clear all local preferences?\n(This will not affect exclusive applications.)"),
		_("Confirmation"),wxCENTER | wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT,this);
	
	return res==wxYES;
}




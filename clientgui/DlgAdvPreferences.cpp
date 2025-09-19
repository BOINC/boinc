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

#include "res/warning.xpm"

using std::string;

IMPLEMENT_DYNAMIC_CLASS(CDlgAdvPreferences, wxDialog)

BEGIN_EVENT_TABLE(CDlgAdvPreferences, wxDialog)
    EVT_COMMAND_RANGE(ID_ADV_PREFS_START,ID_ADV_PREFS_LAST,wxEVT_COMMAND_CHECKBOX_CLICKED,CDlgAdvPreferences::OnHandleCommandEvent)
    //buttons
    EVT_BUTTON(wxID_OK,CDlgAdvPreferences::OnOK)
    EVT_BUTTON(ID_HELPBOINC,CDlgAdvPreferences::OnHelp)
    EVT_BUTTON(ID_BTN_CLEAR,CDlgAdvPreferences::OnClear)
END_EVENT_TABLE()

/* Constructor */
CDlgAdvPreferences::CDlgAdvPreferences(wxWindow* parent) : CDlgAdvPreferencesBase(parent,ID_ANYDIALOG) {
    m_arrTabPageIds.Add(ID_TABPAGE_PROC);
    m_arrTabPageIds.Add(ID_TABPAGE_NET);
    m_arrTabPageIds.Add(ID_TABPAGE_DISK);
    m_arrTabPageIds.Add(ID_TABPAGE_SCHED);

    //setting warning bitmap
    if (m_bmpWarning) {
        m_bmpWarning->SetBitmap(wxBitmap(warning_xpm));
    }

    wxCheckBox* proc_cb[] = {m_chkProcSunday,m_chkProcMonday,m_chkProcTuesday,m_chkProcWednesday,m_chkProcThursday,m_chkProcFriday,m_chkProcSaturday};
    wxTextCtrl* proc_tstarts[] = {m_txtProcSundayStart,m_txtProcMondayStart,m_txtProcTuesdayStart,m_txtProcWednesdayStart,m_txtProcThursdayStart,m_txtProcFridayStart,m_txtProcSaturdayStart};
    wxTextCtrl* proc_tstops[] = {m_txtProcSundayStop,m_txtProcMondayStop,m_txtProcTuesdayStop,m_txtProcWednesdayStop,m_txtProcThursdayStop,m_txtProcFridayStop,m_txtProcSaturdayStop};
    wxCheckBox* net_cb[] = {m_chkNetSunday,m_chkNetMonday,m_chkNetTuesday,m_chkNetWednesday,m_chkNetThursday,m_chkNetFriday,m_chkNetSaturday};
    wxTextCtrl* net_tstarts[] = {m_txtNetSundayStart,m_txtNetMondayStart,m_txtNetTuesdayStart,m_txtNetWednesdayStart,m_txtNetThursdayStart,m_txtNetFridayStart,m_txtNetSaturdayStart};
    wxTextCtrl* net_tstops[] = {m_txtNetSundayStop,m_txtNetMondayStop,m_txtNetTuesdayStop,m_txtNetWednesdayStop,m_txtNetThursdayStop,m_txtNetFridayStop,m_txtNetSaturdayStop};
    for (int i=0; i<7; ++i) {
        procDayChks[i] = proc_cb[i];
        procDayStartTxts[i] = proc_tstarts[i];
        procDayStopTxts[i] = proc_tstops[i];
        netDayChks[i] = net_cb[i];
        netDayStartTxts[i] = net_tstarts[i];
        netDayStopTxts[i] = net_tstops[i];
    }

    // init special tooltips
    SetSpecialTooltips();
    //setting the validators for correct input handling
    SetValidators();
    //read in settings and initialize controls
    ReadPreferenceSettings();

    lastErrorCtrl = NULL;
    stdTextBkgdColor = *wxWHITE;

    if (! m_bOKToShow) return;

    // Get default preference values
    defaultPrefs.enabled_defaults();
    //
    RestoreState();

#ifdef __WXMSW__
    int tabStart = 0, tabwidth = 0;
    RECT r;
    BOOL success = TabCtrl_GetItemRect(m_Notebook->GetHWND(), 0, &r);
    if (success) {
        tabStart = r.left;
    }

    success = TabCtrl_GetItemRect(m_Notebook->GetHWND(), m_Notebook->GetPageCount()-1, &r);
    if (success) {
        tabwidth = r.right - tabStart + 4;
    }
    wxSize sz = m_Notebook->GetBestSize();
    if (sz.x < tabwidth) {
        sz.x = tabwidth;
        m_Notebook->SetMinSize(sz);
    }
#endif
    Layout();
    Fit();
    Centre();
#if defined(__WXMSW__) || defined(__WXGTK__)
    SetDoubleBuffered(true);
#endif
}

/* destructor */
CDlgAdvPreferences::~CDlgAdvPreferences() {
    if (m_bOKToShow) {
        SaveState();
    }
    delete m_vTimeValidator;
}

// set validators for input filtering purposes only
// maximum length for variables storing doubles is set to 16.  Rounding errors
// occur when storing values >13 digits to the left of the decimal.  A maximum length
// of 16 will allow 13 digits, the decimal itself, and 2 digits to the right of the decimal.
//  This is only intended to prevent users from entering a very large string to process,
// the IsValidFloatBetween function will determine if the double is within range.
//
void CDlgAdvPreferences::SetValidators() {
    m_vTimeValidator = new wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
    m_vTimeValidator->SetCharIncludes(wxT("0123456789:"));

    // ######### proc usage page
    m_txtProcUseProcessors->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcUseProcessorsNotInUse->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcUseCPUTime->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcUseCPUTimeNotInUse->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

    m_txtProcIdleFor->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcIdleFor->SetMaxLength(16);
    m_txtNoRecentInput->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtNoRecentInput->SetMaxLength(16);
    m_txtMaxLoad->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

    m_txtNetConnectInterval->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtNetAdditionalDays->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcSwitchEvery->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcSwitchEvery->SetMaxLength(16);
    m_txtDiskWriteToDisk->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtDiskWriteToDisk->SetMaxLength(16);


    // ######### net usage page
    m_txtNetDownloadRate->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtNetDownloadRate->SetMaxLength(16);
    m_txt_daily_xfer_limit_mb->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txt_daily_xfer_limit_mb->SetMaxLength(16);
    m_txt_daily_xfer_period_days->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txt_daily_xfer_period_days->SetMaxLength(10);
    m_txtNetUploadRate->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtNetUploadRate->SetMaxLength(16);

    // ######### disk and memory page
    m_txtDiskMaxSpace->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtDiskMaxSpace->SetMaxLength(16);
    m_txtDiskLeastFree->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtDiskLeastFree->SetMaxLength(16);
    m_txtDiskMaxOfTotal->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtMemoryMaxInUse->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtMemoryMaxOnIdle->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtDiskMaxSwap->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

    // ######### daily schedules page
    m_txtProcEveryDayStart->SetValidator(*m_vTimeValidator);
    m_txtProcEveryDayStop->SetValidator(*m_vTimeValidator);

    m_txtNetEveryDayStart->SetValidator(*m_vTimeValidator);
    m_txtNetEveryDayStop->SetValidator(*m_vTimeValidator);

    for (int i=0; i<7; ++i) {
        procDayStartTxts[i]->SetValidator(*m_vTimeValidator);
        procDayStopTxts[i]->SetValidator(*m_vTimeValidator);
        netDayStartTxts[i]->SetValidator(*m_vTimeValidator);
        netDayStopTxts[i]->SetValidator(*m_vTimeValidator);
    }
}

/* some controls share the same tooltip, set them here */
void CDlgAdvPreferences::SetSpecialTooltips() {
    wxString procDaysTimeTT(PROC_DAY_OF_WEEK_TOOLTIP_TEXT);
    wxString netDaysTimeTT(NET_DAY_OF_WEEK_TOOLTIP_TEXT);
    for (int i=0; i<7; ++i) {
        procDayChks[i]->SetToolTip(procDaysTimeTT);
        procDayStartTxts[i]->SetToolTip(procDaysTimeTT);
        procDayStopTxts[i]->SetToolTip(procDaysTimeTT);
        netDayChks[i]->SetToolTip(netDaysTimeTT);
        netDayStartTxts[i]->SetToolTip(netDaysTimeTT);
        netDayStopTxts[i]->SetToolTip(netDaysTimeTT);
    }
}

/* saves selected tab page */
bool CDlgAdvPreferences::SaveState() {
    wxString        strBaseConfigLocation = wxString(wxT("/DlgAdvPreferences/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);
    if (!pConfig) return false;

    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("CurrentPage"),m_Notebook->GetSelection());

    pConfig->Flush();

    return true;
}

/* restores former selected tab page */
bool CDlgAdvPreferences::RestoreState() {
    wxString        strBaseConfigLocation = wxString(wxT("/DlgAdvPreferences/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    int                p;

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


// We only display 2 places past the decimal, so restrict the
// precision of saved values to .01.  This prevents unexpected
// behavior when, for example, a zero value means no restriction
// and the value is displayed as 0.00 but is actually 0.001.
//
double CDlgAdvPreferences::RoundToHundredths(double td) {
    int64_t i = (int64_t)((td + .005) * 100.);
    return ((double)(i) / 100.);
}

void CDlgAdvPreferences::DisplayValue(double value, wxTextCtrl* textCtrl, wxCheckBox* checkBox) {
    wxString buffer;

    wxASSERT(textCtrl);

    if (checkBox) {
        if (! checkBox->IsChecked()) {
            //textCtrl->Clear();
            textCtrl->Disable();
            return;
        }
    }
    buffer.Printf(wxT("%.2f"), value);
    textCtrl->ChangeValue(buffer);
    textCtrl->Enable();
}

void CDlgAdvPreferences::EnableDisableInUseItem(wxTextCtrl* textCtrl, bool doEnable) {
    if (doEnable) {
        if (! textCtrl->IsEnabled()) {
            textCtrl->Enable();
        }
    } else {
        textCtrl->Disable();
    }
}

void CDlgAdvPreferences::EnableDisableInUseItems() {
    bool doEnable = !(m_chkProcInUse->IsChecked());
    EnableDisableInUseItem(m_txtProcUseProcessors, doEnable);
    EnableDisableInUseItem(m_txtProcUseCPUTime, doEnable);
    m_chkMaxLoad->Enable(doEnable);
    EnableDisableInUseItem(m_txtMaxLoad, doEnable && m_chkMaxLoad->IsChecked());
    EnableDisableInUseItem(m_txtMemoryMaxInUse, doEnable);
}

// read preferences from core client and initialize control values
//
void CDlgAdvPreferences::ReadPreferenceSettings() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    int retval;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Get current working preferences (including any overrides) from client
    retval = pDoc->rpc.get_global_prefs_working_struct(prefs, mask);
    if (retval == ERR_NOT_FOUND) {
        // Older clients don't support get_global_prefs_working_struct RPC
        prefs = pDoc->state.global_prefs;
        retval = pDoc->rpc.get_global_prefs_override_struct(prefs, mask);
    }
    if (retval) {
        m_bOKToShow = false;
        return;
    }

    m_bOKToShow = true;

    // ######### proc usage page
    // max cpus
    // 0 means "no restriction" but we don't use a checkbox here
    if (prefs.max_ncpus_pct == 0.0) prefs.max_ncpus_pct = 100.0;
    DisplayValue(prefs.max_ncpus_pct, m_txtProcUseProcessors);
    if (prefs.niu_max_ncpus_pct == 0.0) prefs.niu_max_ncpus_pct = 100.0;
    DisplayValue(prefs.niu_max_ncpus_pct, m_txtProcUseProcessorsNotInUse);

    // cpu limit
    // 0 means "no restriction" but we don't use a checkbox here
    if (prefs.cpu_usage_limit == 0.0) prefs.cpu_usage_limit = 100.0;
    DisplayValue(prefs.cpu_usage_limit, m_txtProcUseCPUTime);
    if (prefs.niu_cpu_usage_limit == 0.0) prefs.niu_cpu_usage_limit = 100.0;
    DisplayValue(prefs.niu_cpu_usage_limit, m_txtProcUseCPUTimeNotInUse);

    // on batteries
    m_chkProcOnBatteries->SetValue(! prefs.run_on_batteries);

    // in use
    m_chkProcInUse->SetValue(! prefs.run_if_user_active);
    m_chkGPUProcInUse->SetValue(! prefs.run_gpu_if_user_active);

    //"Suspend while computer in use" implies "Suspend GPU while computer in use" so set
    // and disable "Suspend GPU ..." checkbox, overriding its saved value if necessary.
    if (m_chkProcInUse->IsChecked()) {
        m_chkGPUProcInUse->SetValue(true);
        m_chkGPUProcInUse->Disable();
    }

    // idle if no input in X minutes
    DisplayValue(prefs.idle_time_to_run, m_txtProcIdleFor);

    m_chkNoRecentInput->SetValue(prefs.suspend_if_no_recent_input > 0.0);
    DisplayValue(prefs.suspend_if_no_recent_input, m_txtNoRecentInput, m_chkNoRecentInput);

    m_chkMaxLoad->SetValue(prefs.suspend_cpu_usage > 0.0);
    DisplayValue(prefs.suspend_cpu_usage, m_txtMaxLoad, m_chkMaxLoad);
    m_chkMaxLoadNotInUse->SetValue(prefs.niu_suspend_cpu_usage > 0.0);
    DisplayValue(prefs.niu_suspend_cpu_usage, m_txtMaxLoadNotInUse, m_chkMaxLoadNotInUse);

    // connection interval
    DisplayValue(prefs.work_buf_min_days, m_txtNetConnectInterval);

    DisplayValue(prefs.work_buf_additional_days, m_txtNetAdditionalDays);

    // switch every X minutes
    DisplayValue(prefs.cpu_scheduling_period_minutes, m_txtProcSwitchEvery);

    // write to disk every X seconds
    DisplayValue(prefs.disk_interval, m_txtDiskWriteToDisk);

    // ######### net usage page

    //download rate
    m_chkNetDownloadRate->SetValue(prefs.max_bytes_sec_down > 0.0);
    DisplayValue((prefs.max_bytes_sec_down / 1024), m_txtNetDownloadRate, m_chkNetDownloadRate);


    // upload rate
    m_chkNetUploadRate->SetValue(prefs.max_bytes_sec_up > 0.0);
    DisplayValue((prefs.max_bytes_sec_up / 1024), m_txtNetUploadRate, m_chkNetUploadRate);

    m_chk_daily_xfer_limit->SetValue((prefs.daily_xfer_limit_mb > 0.0) && (prefs.daily_xfer_period_days > 0.0));
    DisplayValue(prefs.daily_xfer_limit_mb, m_txt_daily_xfer_limit_mb, m_chk_daily_xfer_limit);
    DisplayValue(prefs.daily_xfer_period_days, m_txt_daily_xfer_period_days, m_chk_daily_xfer_limit);

    //
    // skip image verification
    m_chkNetSkipImageVerification->SetValue(prefs.dont_verify_images);
    // confirm before connect
    m_chkNetConfirmBeforeConnect->SetValue(prefs.confirm_before_connecting);
    // disconnect when done
    m_chkNetDisconnectWhenDone->SetValue(prefs.hangup_if_dialed);

    // ######### disk and memory usage page
    //max space used
    m_chkDiskMaxSpace->SetValue(prefs.disk_max_used_gb > 0.0);
    DisplayValue(prefs.disk_max_used_gb, m_txtDiskMaxSpace, m_chkDiskMaxSpace);

    // min free
    m_chkDiskLeastFree->SetValue(prefs.disk_min_free_gb > 0.0);
    DisplayValue(prefs.disk_min_free_gb, m_txtDiskLeastFree, m_chkDiskLeastFree);

    // max used percentage
    m_chkDiskMaxOfTotal->SetValue(prefs.disk_max_used_pct < 100.0);
    DisplayValue(prefs.disk_max_used_pct, m_txtDiskMaxOfTotal, m_chkDiskMaxOfTotal);

    // max VM used
    DisplayValue((prefs.ram_max_used_busy_frac*100.0), m_txtMemoryMaxInUse);

    // max VM idle
    DisplayValue((prefs.ram_max_used_idle_frac*100.0), m_txtMemoryMaxOnIdle);

    // suspend to memory
    m_chkMemoryWhileSuspended->SetValue(prefs.leave_apps_in_memory);

    // max swap space (virtual memory)
    DisplayValue((prefs.vm_max_used_frac*100.0), m_txtDiskMaxSwap);

    // ######### daily schedules page
    // do work between
    m_chkProcEveryDay->SetValue(prefs.cpu_times.start_hour != prefs.cpu_times.end_hour);
    if (m_chkProcEveryDay->IsChecked()) {
        m_txtProcEveryDayStart->ChangeValue(DoubleToTimeString(prefs.cpu_times.start_hour));
        m_txtProcEveryDayStop->ChangeValue(DoubleToTimeString(prefs.cpu_times.end_hour));
    }

    //special day times
    for(int i=0; i< 7;i++) {
        TIME_SPAN& cpu = prefs.cpu_times.week.days[i];
        if(cpu.present && (cpu.start_hour != cpu.end_hour)) {
            procDayChks[i]->SetValue(true);
            procDayStartTxts[i]->ChangeValue(DoubleToTimeString(cpu.start_hour));
            procDayStopTxts[i]->ChangeValue(DoubleToTimeString(cpu.end_hour));
        }
    }

    // use network between
    m_chkNetEveryDay->SetValue(prefs.net_times.start_hour != prefs.net_times.end_hour);
    if (m_chkNetEveryDay->IsChecked()) {
        m_txtNetEveryDayStart->ChangeValue(DoubleToTimeString(prefs.net_times.start_hour));
        m_txtNetEveryDayStop->ChangeValue(DoubleToTimeString(prefs.net_times.end_hour));
    }

    //special net times
    for(int i=0; i< 7;i++) {
        TIME_SPAN& net = prefs.net_times.week.days[i];
        if(net.present && (net.start_hour != net.end_hour)) {
            netDayChks[i]->SetValue(true);
            netDayStartTxts[i]->ChangeValue(DoubleToTimeString(net.start_hour));
            netDayStopTxts[i]->ChangeValue(DoubleToTimeString(net.end_hour));
        }
    }

    //update control states
    this->UpdateControlStates();
}

// write overridden preferences to disk (global_prefs_override.xml)
// IMPORTANT: Any items added here must be checked in ValidateInput()!
//
bool CDlgAdvPreferences::SavePreferencesSettings() {
    double td;

    mask.clear();

    // proc usage page
    td = 0;
    m_txtProcUseProcessors->GetValue().ToDouble(&td);
    prefs.max_ncpus_pct = RoundToHundredths(td);
    mask.max_ncpus_pct=true;

    td = 0;
    m_txtProcUseProcessorsNotInUse->GetValue().ToDouble(&td);
    prefs.niu_max_ncpus_pct = RoundToHundredths(td);
    mask.niu_max_ncpus_pct=true;

    td = 0;
    m_txtProcUseCPUTime->GetValue().ToDouble(&td);
    prefs.cpu_usage_limit=RoundToHundredths(td);
    mask.cpu_usage_limit=true;

    td = 0;
    m_txtProcUseCPUTimeNotInUse->GetValue().ToDouble(&td);
    prefs.niu_cpu_usage_limit = RoundToHundredths(td);
    mask.niu_cpu_usage_limit = true;

    prefs.run_on_batteries = ! (m_chkProcOnBatteries->GetValue());
    mask.run_on_batteries=true;

    prefs.run_if_user_active = (! m_chkProcInUse->GetValue());
    mask.run_if_user_active=true;

    prefs.run_gpu_if_user_active = (! m_chkGPUProcInUse->GetValue());
    mask.run_gpu_if_user_active=true;

    if(m_txtProcIdleFor->IsEnabled()) {
        // ToDouble() returns an error (and doesn't change its arg)
        // if the string is empty.
        // So set td to zero first (here and below).
        td = 0;
        m_txtProcIdleFor->GetValue().ToDouble(&td);
        prefs.idle_time_to_run=RoundToHundredths(td);
        mask.idle_time_to_run=true;
    }

    if (m_chkNoRecentInput->IsChecked()) {
        td = 0;
        m_txtNoRecentInput->GetValue().ToDouble(&td);
        prefs.suspend_if_no_recent_input = RoundToHundredths(td);
    } else {
        prefs.suspend_if_no_recent_input = 0;
    }
    mask.suspend_if_no_recent_input = true;

    if (m_txtMaxLoad->IsEnabled() || !prefs.run_if_user_active) {
        td = 0;
        m_txtMaxLoad->GetValue().ToDouble(&td);
        prefs.suspend_cpu_usage=RoundToHundredths(td);
    } else {
        prefs.suspend_cpu_usage = 0.0;
    }
    mask.suspend_cpu_usage=true;

    if (m_chkMaxLoadNotInUse->IsChecked()) {
        td = 0;
        m_txtMaxLoadNotInUse->GetValue().ToDouble(&td);
        prefs.niu_suspend_cpu_usage=RoundToHundredths(td);
    } else {
        prefs.niu_suspend_cpu_usage = 0.0;
    }
    mask.niu_suspend_cpu_usage=true;

    td = 0;
    m_txtNetConnectInterval->GetValue().ToDouble(&td);
    prefs.work_buf_min_days=RoundToHundredths(td);
    mask.work_buf_min_days=true;

    td = 0;
    m_txtNetAdditionalDays->GetValue().ToDouble(&td);
    prefs.work_buf_additional_days = RoundToHundredths(td);
    mask.work_buf_additional_days = true;

    td = 00;
    m_txtProcSwitchEvery->GetValue().ToDouble(&td);
    prefs.cpu_scheduling_period_minutes=RoundToHundredths(td);
    mask.cpu_scheduling_period_minutes=true;

    td = 0;
    m_txtDiskWriteToDisk->GetValue().ToDouble(&td);
    prefs.disk_interval=RoundToHundredths(td);
    mask.disk_interval=true;

   // ######### net usage page
    //
    if (m_chkNetDownloadRate->IsChecked()) {
        td = 0;
        m_txtNetDownloadRate->GetValue().ToDouble(&td);
        td = RoundToHundredths(td);
        td = td * 1024;
        prefs.max_bytes_sec_down=td;
    } else {
        prefs.max_bytes_sec_down = 0.0;
    }
    mask.max_bytes_sec_down=true;

    if (m_chkNetUploadRate->IsChecked()) {
        td = 0;
        m_txtNetUploadRate->GetValue().ToDouble(&td);
        td = RoundToHundredths(td);
        td = td * 1024;
        prefs.max_bytes_sec_up=td;
    } else {
        prefs.max_bytes_sec_up = 0.0;
    }
    mask.max_bytes_sec_up=true;

    if (m_chk_daily_xfer_limit->IsChecked()) {
        td = 0;
        m_txt_daily_xfer_limit_mb->GetValue().ToDouble(&td);
        prefs.daily_xfer_limit_mb=RoundToHundredths(td);
        td = 0;
        m_txt_daily_xfer_period_days->GetValue().ToDouble(&td);
        prefs.daily_xfer_period_days=(int)td;
    } else {
        prefs.daily_xfer_limit_mb = 0.0;
        prefs.daily_xfer_period_days = 0.0;
    }
    mask.daily_xfer_limit_mb=true;
    mask.daily_xfer_period_days=true;

    prefs.dont_verify_images=m_chkNetSkipImageVerification->GetValue();
    mask.dont_verify_images=true;

    prefs.confirm_before_connecting= m_chkNetConfirmBeforeConnect->GetValue();
    mask.confirm_before_connecting=true;

    prefs.hangup_if_dialed= m_chkNetDisconnectWhenDone->GetValue();
    mask.hangup_if_dialed=true;

    // ######### disk and memory page

    if (m_chkDiskMaxSpace->IsChecked()) {
        td = 0;
        m_txtDiskMaxSpace->GetValue().ToDouble(&td);
        prefs.disk_max_used_gb=RoundToHundredths(td);
    } else {
        prefs.disk_max_used_gb = 0.0;
    }
    mask.disk_max_used_gb=true;

    if (m_chkDiskLeastFree->IsChecked()) {
        td = 0;
        m_txtDiskLeastFree->GetValue().ToDouble(&td);
        prefs.disk_min_free_gb=RoundToHundredths(td);
    } else {
        prefs.disk_min_free_gb = 0.0;
    }
    mask.disk_min_free_gb=true;

    if (m_chkDiskMaxOfTotal->IsChecked()) {
        td = 0;
        m_txtDiskMaxOfTotal->GetValue().ToDouble(&td);
        prefs.disk_max_used_pct = RoundToHundredths(td);
    } else {
        prefs.disk_max_used_pct = 100.0;
    }
    mask.disk_max_used_pct=true;

    // Memory

    td = 0;
    m_txtMemoryMaxInUse->GetValue().ToDouble(&td);
    td = RoundToHundredths(td);
    td = td / 100.0;
    prefs.ram_max_used_busy_frac=td;
    mask.ram_max_used_busy_frac=true;

    td = 0;
    m_txtMemoryMaxOnIdle->GetValue().ToDouble(&td);
    td = RoundToHundredths(td);
    td = td / 100.0;
    prefs.ram_max_used_idle_frac=td;
    mask.ram_max_used_idle_frac=true;

    prefs.leave_apps_in_memory = m_chkMemoryWhileSuspended->GetValue();
    mask.leave_apps_in_memory=true;

    td = 0;
    m_txtDiskMaxSwap->GetValue().ToDouble(&td);
    td = RoundToHundredths(td);
    td = td / 100.0 ;
    prefs.vm_max_used_frac=td;
    mask.vm_max_used_frac=true;

    // ######### daily schedules page

    if (m_chkProcEveryDay->IsChecked()) {
        prefs.cpu_times.start_hour = TimeStringToDouble(m_txtProcEveryDayStart->GetValue());
        prefs.cpu_times.end_hour = TimeStringToDouble(m_txtProcEveryDayStop->GetValue());
    } else {
        prefs.cpu_times.start_hour = prefs.cpu_times.end_hour = 0.0;
    }
    mask.start_hour = mask.end_hour = true;

    // clear special day times settings
    //
    prefs.cpu_times.week.clear();
    for(int i=0; i< 7;i++) {
        if(procDayChks[i]->GetValue()) {
            wxString startStr = procDayStartTxts[i]->GetValue();
            wxString endStr = procDayStopTxts[i]->GetValue();
            prefs.cpu_times.week.set(i,
                TimeStringToDouble(startStr),
                TimeStringToDouble(endStr)
                );
        }
    }

    if (m_chkNetEveryDay->IsChecked()) {
        prefs.net_times.start_hour = TimeStringToDouble(m_txtNetEveryDayStart->GetValue());
        prefs.net_times.end_hour = TimeStringToDouble(m_txtNetEveryDayStop->GetValue());
    } else {
        prefs.net_times.start_hour = prefs.net_times.end_hour = 0.0;
    }
    mask.net_start_hour = mask.net_end_hour = true;

    // clear special net times settings
    //
    prefs.net_times.week.clear();
    for(int i=0; i< 7;i++) {
        if(netDayChks[i]->GetValue()) {
            wxString startStr = netDayStartTxts[i]->GetValue();
            wxString endStr = netDayStopTxts[i]->GetValue();
            prefs.net_times.week.set(i,
                TimeStringToDouble(startStr),
                TimeStringToDouble(endStr)
                );
        }
    }

    return true;
}

// set state of control depending on other control's state
//
void CDlgAdvPreferences::UpdateControlStates() {
    // ######### proc usage page

    // If we suspend work when in use, disable and check "Use GPU when in use"
    m_chkGPUProcInUse->Enable(! m_chkProcInUse->IsChecked());
    if (m_chkProcInUse->IsChecked()) m_chkGPUProcInUse->SetValue(true);

    m_txtMaxLoadNotInUse->Enable(m_chkMaxLoadNotInUse->IsChecked());
    EnableDisableInUseItems();
    m_txtNoRecentInput->Enable(m_chkNoRecentInput->IsChecked());

    // ######### disk and memory usage page
    m_txtDiskMaxSpace->Enable(m_chkDiskMaxSpace->IsChecked());
    m_txtDiskLeastFree->Enable(m_chkDiskLeastFree->IsChecked());
    m_txtDiskMaxOfTotal->Enable(m_chkDiskMaxOfTotal->IsChecked());

    // ######### net usage page
    m_txtNetDownloadRate->Enable(m_chkNetDownloadRate->IsChecked());
    m_txtNetUploadRate->Enable(m_chkNetUploadRate->IsChecked());
    m_txt_daily_xfer_limit_mb->Enable(m_chk_daily_xfer_limit->IsChecked());
    m_txt_daily_xfer_period_days->Enable(m_chk_daily_xfer_limit->IsChecked());

    // ######### daily schedules page
    m_txtProcEveryDayStart->Enable(m_chkProcEveryDay->IsChecked());
    m_txtProcEveryDayStop->Enable(m_chkProcEveryDay->IsChecked());

    m_txtNetEveryDayStart->Enable(m_chkNetEveryDay->IsChecked());
    m_txtNetEveryDayStop->Enable(m_chkNetEveryDay->IsChecked());

    for (int i=0; i<7; ++i) {
        procDayStartTxts[i]->Enable(procDayChks[i]->IsChecked());
        procDayStopTxts[i]->Enable(procDayChks[i]->IsChecked());
        netDayStartTxts[i]->Enable(netDayChks[i]->IsChecked());
        netDayStopTxts[i]->Enable(netDayChks[i]->IsChecked());
    }
}

/* validates the entered informations */
bool CDlgAdvPreferences::ValidateInput() {
    wxString invMsgFloat = _("Invalid number");
    wxString invMsgTime = _("Invalid time, value must be between 0:00 and 24:00, format is HH:MM");
    wxString invMsgTimeSpan = _("Start time must be different from end time");
    wxString invMsgLimit10 = _("Number must be between 0 and 10");
    wxString invMsgLimit100 = _("Number must be between 0 and 100");
    wxString invMsgLimit1_100 = _("Number must be between 1 and 100");
    wxString invMsgIdle = _("Suspend when no mouse or keyboard input needs to be greater than 'in use' mouse or keyboard input.");
    wxString buffer;
    double startTime, endTime;

    // ######### proc usage page
    if (m_txtProcUseProcessors->IsEnabled()) {
        buffer = m_txtProcUseProcessors->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0.0, 100.0)) {
            ShowErrorMessage(invMsgLimit100, m_txtProcUseProcessors);
            return false;
        }
    }
    buffer = m_txtProcUseProcessorsNotInUse->GetValue();
    if(!IsValidFloatValueBetween(buffer, 0.0, 100.0)) {
        ShowErrorMessage(invMsgLimit100, m_txtProcUseProcessorsNotInUse);
        return false;
    }

    buffer = m_txtProcUseCPUTimeNotInUse->GetValue();
    if(!IsValidFloatValueBetween(buffer, 0.0, 100.0)) {
        ShowErrorMessage(invMsgLimit100, m_txtProcUseCPUTimeNotInUse);
        return false;
    }

    if(m_txtProcIdleFor->IsEnabled()) {
        buffer = m_txtProcIdleFor->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
            ShowErrorMessage(invMsgFloat,m_txtProcIdleFor);
            return false;
        }
    }

    if (m_chkNoRecentInput->IsChecked()) {
        buffer = m_txtNoRecentInput->GetValue();
        if (!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
            ShowErrorMessage(invMsgFloat, m_txtNoRecentInput);
            return false;
        }
    }

 // Checks for a condition where no computing could occur if suspended until idle and
 // suspend after being idle overlap.
 //
    if (m_txtProcIdleFor->IsEnabled() && m_chkNoRecentInput->IsChecked()) {
        wxString bufferNRI = m_txtNoRecentInput->GetValue();
        wxString bufferPIF = m_txtProcIdleFor->GetValue();
        double valueNRI;
        bufferNRI.ToDouble(&valueNRI);
        double valuePIF;
        bufferPIF.ToDouble(&valuePIF);
        if((valuePIF - valueNRI + 0.005) >=0) {  // 0.005 is included to factor in rounding to nearest hundredth
            ShowErrorMessage(invMsgIdle, m_txtNoRecentInput);
            return false;
        }
    }

    if (m_txtMaxLoad->IsEnabled()) {
        buffer = m_txtMaxLoad->GetValue();
        if(!IsValidFloatValueBetween(buffer, 1.0, 100.0)) {
            ShowErrorMessage(invMsgLimit1_100, m_txtMaxLoad);
            return false;
        }
    }

    if (m_chkMaxLoadNotInUse->IsChecked()) {
        buffer = m_txtMaxLoadNotInUse->GetValue();
        if(!IsValidFloatValueBetween(buffer, 1.0, 100.0)) {
            ShowErrorMessage(invMsgLimit1_100, m_txtMaxLoadNotInUse);
            return false;
        }
    }

    // limit additional days from 0 to 10
    buffer = m_txtNetConnectInterval->GetValue();
    if(!IsValidFloatValueBetween(buffer, 0.0, 10.0)) {
        ShowErrorMessage(invMsgLimit10,m_txtNetConnectInterval);
        return false;
    }

    buffer = m_txtNetAdditionalDays->GetValue();
    if(!IsValidFloatValueBetween(buffer, 0.0, 10.0)) {
        ShowErrorMessage(invMsgLimit10,m_txtNetAdditionalDays);
        return false;
    }

    buffer = m_txtProcSwitchEvery->GetValue();
    if(!IsValidFloatValueBetween(buffer, 1.0, 9999999999999.99)) {
        ShowErrorMessage(invMsgFloat, m_txtProcSwitchEvery);
        return false;
    }

    buffer = m_txtDiskWriteToDisk->GetValue();
    if(!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
        ShowErrorMessage(invMsgFloat, m_txtDiskWriteToDisk);
        return false;
    }

    // ######### net usage page

    if (m_chkNetDownloadRate->IsChecked()) {
        buffer = m_txtNetDownloadRate->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
            ShowErrorMessage(invMsgFloat, m_txtNetDownloadRate);
            return false;
        }
    }

    if (m_chkNetUploadRate->IsChecked()) {
        buffer = m_txtNetUploadRate->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
            ShowErrorMessage(invMsgFloat, m_txtNetUploadRate);
            return false;
        }
    }

    if (m_chk_daily_xfer_limit->IsChecked()) {
        buffer = m_txt_daily_xfer_limit_mb->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
            ShowErrorMessage(invMsgFloat, m_txt_daily_xfer_limit_mb);
            return false;
        }

        buffer = m_txt_daily_xfer_period_days->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0, 2147483647.0)) {
            ShowErrorMessage(invMsgFloat, m_txt_daily_xfer_period_days);
            return false;
        }
    }

    // ######### disk and memory page
    if (m_chkDiskMaxSpace->IsChecked()) {
        buffer = m_txtDiskMaxSpace->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
            ShowErrorMessage(invMsgFloat, m_txtDiskMaxSpace);
            return false;
        }
    }

    if (m_chkDiskLeastFree->IsChecked()) {
        buffer = m_txtDiskLeastFree->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
            ShowErrorMessage(invMsgFloat, m_txtDiskLeastFree);
            return false;
        }
    }

    if (m_chkDiskMaxOfTotal->IsChecked()) {
        buffer = m_txtDiskMaxOfTotal->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0.0, 100.0)) {
            ShowErrorMessage(invMsgLimit100, m_txtDiskMaxOfTotal);
            return false;
        }
    }

    if(m_txtMemoryMaxInUse->IsEnabled()) {
        buffer = m_txtMemoryMaxInUse->GetValue();
        if(!IsValidFloatValueBetween(buffer, 1.0, 100.0)) {
            ShowErrorMessage(invMsgLimit1_100, m_txtMemoryMaxInUse);
            return false;
        }
    }

    buffer = m_txtMemoryMaxOnIdle->GetValue();
    if(!IsValidFloatValueBetween(buffer, 1.0, 100.0)) {
        ShowErrorMessage(invMsgLimit1_100, m_txtMemoryMaxOnIdle);
        return false;
    }

    buffer = m_txtDiskMaxSwap->GetValue();
    if(!IsValidFloatValueBetween(buffer, 1.0, 100.0)) {
        ShowErrorMessage(invMsgLimit1_100, m_txtDiskMaxSwap);
        return false;
    }

    // ######### daily schedules page
    if (m_chkProcEveryDay->IsChecked()) {
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
        startTime = TimeStringToDouble(m_txtProcEveryDayStart->GetValue());
        endTime = TimeStringToDouble(m_txtProcEveryDayStop->GetValue());
        if (startTime == endTime) {
            ShowErrorMessage(invMsgTimeSpan,m_txtProcEveryDayStop);
            return false;
        }
    }

    //all text ctrls in proc special time textBox
    for(int i=0; i< 7;i++) {
        if(procDayChks[i]->GetValue()) {
           buffer = procDayStartTxts[i]->GetValue();
            if(!IsValidTimeValue(buffer)) {
                ShowErrorMessage(invMsgTime,procDayStartTxts[i]);
                return false;
            }
           buffer = procDayStopTxts[i]->GetValue();
            if(!IsValidTimeValue(buffer)) {
                ShowErrorMessage(invMsgTime,procDayStopTxts[i]);
                return false;
            }
            startTime = TimeStringToDouble(procDayStartTxts[i]->GetValue());
            endTime = TimeStringToDouble(procDayStopTxts[i]->GetValue());
            if (startTime == endTime) {
                ShowErrorMessage(invMsgTimeSpan,procDayStopTxts[i]);
                return false;
            }
        }
    }

    if (m_chkNetEveryDay->IsChecked()) {
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
        startTime = TimeStringToDouble(m_txtNetEveryDayStart->GetValue());
        endTime = TimeStringToDouble(m_txtNetEveryDayStop->GetValue());
        if (startTime == endTime) {
            ShowErrorMessage(invMsgTimeSpan,m_txtNetEveryDayStop);
            return false;
        }
    }

    //all text ctrls in net special time textBox
    for(int i=0; i< 7;i++) {
        if(netDayChks[i]->GetValue()) {
           buffer = netDayStartTxts[i]->GetValue();
            if(!IsValidTimeValue(buffer)) {
                ShowErrorMessage(invMsgTime,netDayStartTxts[i]);
                return false;
            }
            buffer = netDayStopTxts[i]->GetValue();
            if(!IsValidTimeValue(buffer)) {
                ShowErrorMessage(invMsgTime,netDayStopTxts[i]);
                return false;
            }
            startTime = TimeStringToDouble(netDayStartTxts[i]->GetValue());
            endTime = TimeStringToDouble(netDayStopTxts[i]->GetValue());
            if (startTime == endTime) {
                ShowErrorMessage(invMsgTimeSpan,netDayStopTxts[i]);
                return false;
            }
        }
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
        //some controls are contained in an additional panel,
        //so look at its parent and grandparent
        for (int i=0; i<2; ++i) {
            parent = parent->GetParent();
            wxASSERT(parent);
            parentid = parent->GetId();
            index = m_arrTabPageIds.Index(parentid);
            if(index != wxNOT_FOUND) break;
        }
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
#if wxDEBUG_LEVEL   // Prevent compiler warning (unused variable)
    bool visibleOK =
#endif
    this->EnsureTabPageVisible(errorCtrl);
    wxASSERT(visibleOK);
    //
    if(message.IsEmpty()){
        message = _("invalid input value detected");
    }
    if (lastErrorCtrl) {
        lastErrorCtrl->SetBackgroundColour(stdTextBkgdColor);
        lastErrorCtrl->Refresh();
    }
    if (lastErrorCtrl != errorCtrl) {
        stdTextBkgdColor = errorCtrl->GetBackgroundColour();
    }
    errorCtrl->SetBackgroundColour(wxColour(255, 192, 192));
    errorCtrl->Refresh();
    lastErrorCtrl = errorCtrl;
    wxGetApp().SafeMessageBox(message,_("Validation Error"),wxOK | wxCENTRE | wxICON_ERROR,this);
    errorCtrl->SetFocus();
}

/* checks if ch is a valid character for float values */
bool CDlgAdvPreferences::IsValidFloatChar(const wxChar& ch) {
    //don't accept the e
    return wxIsdigit(ch) || ch=='.' || ch==',' || ch=='+' || ch=='-';}

/* checks if ch is a valid character for time values */
bool CDlgAdvPreferences::IsValidTimeChar(const wxChar& ch) {
    return wxIsdigit(ch) || ch==':';
}

/* checks if the value contains a valid float */
bool CDlgAdvPreferences::IsValidFloatValue(const wxString& value, bool allowNegative) {
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
    if (!allowNegative) {
        if (td < 0.0) return false;
    }
    return true;
}

bool CDlgAdvPreferences::IsValidFloatValueBetween(const wxString& value, double minVal, double maxVal){
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
    if ((td < minVal) || (td > maxVal)) return false;
    return true;
}


/* checks if the value is a valid time */
bool CDlgAdvPreferences::IsValidTimeValue(const wxString& value) {
    for (unsigned int i = 0; i < value.Length(); i++) {
        if (!IsValidTimeChar(value[i])) {
            return false;
        }
    }
    //all chars are valid, now what is with the value as a whole ?
    if (value == wxT("24:00")) return true;
    wxDateTime dt;
    const wxChar* stopChar = dt.ParseFormat(value, wxT("%H:%M"));
    if (stopChar == NULL) return false;    // conversion failed
    if (*stopChar != '\0') return false;   // conversion failed
    return true;
}


// ------------ Event handlers starts here
// -------- generic command handler
// handles all control command events
void CDlgAdvPreferences::OnHandleCommandEvent(wxCommandEvent& ev) {
    ev.Skip();
    // If user has just set the checkbox, set textedit field to default value.
    // Note: use ChangeValue() here to avoid generating extra events.
    // m_txtProcIdleFor depends on 2 checkboxes, set it in UpdateControlStates().
    switch (ev.GetId()) {
        // processor usage page
    case ID_CHKMAXLOAD:
        if (!m_txtMaxLoad->GetValue()) {
            DisplayValue(defaultPrefs.suspend_cpu_usage, m_txtMaxLoad, m_chkMaxLoad);
        }
        break;
    case ID_CHKMAXLOADNOTINUSE:
        if (!m_txtMaxLoadNotInUse->GetValue()) {
            DisplayValue(defaultPrefs.niu_suspend_cpu_usage, m_txtMaxLoadNotInUse, m_chkMaxLoadNotInUse);
        }
        break;
    case ID_CHKNORECENTINPUT:
        if (!m_txtNoRecentInput->GetValue()) {
            DisplayValue(defaultPrefs.suspend_if_no_recent_input, m_txtNoRecentInput, m_chkNoRecentInput);
        }
        break;
    // network usage page
    case ID_CHKNETDOWNLOADRATE:
        DisplayValue((defaultPrefs.max_bytes_sec_down / 1024), m_txtNetDownloadRate, m_chkNetDownloadRate);
        break;
    case ID_CHKNETUPLOADRATE:
        DisplayValue((defaultPrefs.max_bytes_sec_up / 1024), m_txtNetUploadRate, m_chkNetUploadRate);
        break;
    case ID_CHKDAILYXFERLIMIT:
        DisplayValue(defaultPrefs.daily_xfer_limit_mb, m_txt_daily_xfer_limit_mb, m_chk_daily_xfer_limit);
        DisplayValue(defaultPrefs.daily_xfer_period_days, m_txt_daily_xfer_period_days, m_chk_daily_xfer_limit);
        break;

    // disk usage page
    case ID_CHKDISKMAXSPACE:
        DisplayValue(defaultPrefs.disk_max_used_gb, m_txtDiskMaxSpace, m_chkDiskMaxSpace);
        break;
    case ID_CHKDISKLEASTFREE:
        DisplayValue(defaultPrefs.disk_min_free_gb, m_txtDiskLeastFree, m_chkDiskLeastFree);
        break;
    case ID_CHKDISKMAXOFTOTAL:
        DisplayValue(defaultPrefs.disk_max_used_pct, m_txtDiskMaxOfTotal, m_chkDiskMaxOfTotal);
        break;
    case ID_CHKPROCEVERYDAY:
        if (ev.IsChecked()) {
            m_txtProcEveryDayStart->ChangeValue(DoubleToTimeString(defaultPrefs.cpu_times.start_hour));
            m_txtProcEveryDayStop->ChangeValue(DoubleToTimeString(defaultPrefs.cpu_times.end_hour));
        } else {
            m_txtProcEveryDayStart->Clear();
            m_txtProcEveryDayStop->Clear();
        }
        break;
    case ID_CHKPROCSUNDAY:
    case ID_CHKPROCMONDAY:
    case ID_CHKPROCTUESDAY:
    case ID_CHKPROCWEDNESDAY:
    case ID_CHKPROCTHURSDAY:
    case ID_CHKPROCFRIDAY:
    case ID_CHKPROCSATURDAY:
        if (ev.IsChecked()) {
            (procDayStartTxts[ev.GetId() - ID_CHKPROCSUNDAY])->ChangeValue(DoubleToTimeString(defaultPrefs.cpu_times.start_hour));
            (procDayStopTxts[ev.GetId() - ID_CHKPROCSUNDAY])->ChangeValue(DoubleToTimeString(defaultPrefs.cpu_times.end_hour));
        } else {
            (procDayStartTxts[ev.GetId() - ID_CHKPROCSUNDAY])->Clear();
            (procDayStopTxts[ev.GetId() - ID_CHKPROCSUNDAY])->Clear();
        }
        break;
    case ID_CHKNETEVERYDAY:
       if (ev.IsChecked()) {
            m_txtNetEveryDayStart->ChangeValue(DoubleToTimeString(defaultPrefs.net_times.start_hour));
            m_txtNetEveryDayStop->ChangeValue(DoubleToTimeString(defaultPrefs.net_times.end_hour));
        } else {
            m_txtNetEveryDayStart->Clear();
            m_txtNetEveryDayStop->Clear();
        }
        break;
    case ID_CHKNETSUNDAY:
    case ID_CHKNETMONDAY:
    case ID_CHKNETTUESDAY:
    case ID_CHKNETWEDNESDAY:
    case ID_CHKNETTHURSDAY:
    case ID_CHKNETFRIDAY:
    case ID_CHKNETSATURDAY:
       if (ev.IsChecked()) {
            (netDayStartTxts[ev.GetId() - ID_CHKNETSUNDAY])->ChangeValue(DoubleToTimeString(defaultPrefs.net_times.start_hour));
            (netDayStopTxts[ev.GetId() - ID_CHKNETSUNDAY])->ChangeValue(DoubleToTimeString(defaultPrefs.net_times.end_hour));
        } else {
            (netDayStartTxts[ev.GetId() - ID_CHKNETSUNDAY])->Clear();
            (netDayStopTxts[ev.GetId() - ID_CHKNETSUNDAY])->Clear();
        }
        break;

    default:
        break;
    }
//    }
    UpdateControlStates();
}

// ---- command buttons handlers
// handles OK button clicked
void CDlgAdvPreferences::OnOK(wxCommandEvent& ev) {
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if(!ValidateInput()) {
        return;
    }
    if (!m_bUsingLocalPrefs) {
        if(!this->ConfirmSetLocal()) {
            return;
        }
    }
    if(SavePreferencesSettings()) {
        pDoc->rpc.set_global_prefs_override_struct(prefs,mask);
        pDoc->rpc.read_global_prefs_override();
    }

    ev.Skip();
}

bool CDlgAdvPreferences::ConfirmSetLocal() {
    wxString strMessage     = wxEmptyString;
    strMessage.Printf(
            _("Changing to use the local preferences defined on this page. This will override your web-based preferences, even if you subsequently make changes there. Do you want to proceed?")
    );
    int res = wxGetApp().SafeMessageBox(
        strMessage,
        _("Confirmation"),wxCENTER | wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT,this);

    return res==wxYES;
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
        "Discard local preferences and use web-based preferences?"),
        _("Confirmation"),wxCENTER | wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT,this);

    return res==wxYES;
}

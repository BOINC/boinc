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
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgAdvPreferences.h"
#endif

#include "stdwx.h"
#include "DlgAdvPreferences.h"
#include "res/usage.xpm"
#include "res/xfer.xpm"
#include "res/proj.xpm"
#include "res/warning.xpm"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "SkinManager.h"
#include "hyperlink.h"
#include "Events.h"

IMPLEMENT_DYNAMIC_CLASS(CDlgAdvPreferences, wxDialog)

BEGIN_EVENT_TABLE(CDlgAdvPreferences, wxDialog)
	EVT_COMMAND_RANGE(20000,21000,wxEVT_COMMAND_CHECKBOX_CLICKED,CDlgAdvPreferences::OnHandleCommandEvent)
	EVT_COMMAND_RANGE(20000,21000,wxEVT_COMMAND_RADIOBUTTON_SELECTED,CDlgAdvPreferences::OnHandleCommandEvent)
	EVT_COMMAND_RANGE(20000,21000,wxEVT_COMMAND_TEXT_UPDATED,CDlgAdvPreferences::OnHandleCommandEvent)
	//buttons
	EVT_BUTTON(wxID_OK,CDlgAdvPreferences::OnOK)
	EVT_BUTTON(wxID_HELP,CDlgAdvPreferences::OnHelp)
	EVT_BUTTON(ID_BTN_CLEAR,CDlgAdvPreferences::OnClear)
END_EVENT_TABLE()

/* Constructor */
CDlgAdvPreferences::CDlgAdvPreferences(wxWindow* parent) : CDlgAdvPreferencesBase(parent,ID_ANYDIALOG) {
	m_bInInit=false;
	m_bDataChanged=false;
	m_arrTabPageIds.Add(ID_TABPAGE_PROC);
	m_arrTabPageIds.Add(ID_TABPAGE_NET);
	m_arrTabPageIds.Add(ID_TABPAGE_DISK);
	
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
	//setting warning bitmap
	m_bmpWarning->SetBitmap(wxBitmap(warning_xpm));
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
	m_txtProcSwitchEvery->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtProcUseProcessors->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtProcUseCPUTime->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	//net page
	m_txtNetConnectInterval->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtNetDownloadRate->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	m_txtNetUploadRate->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
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

/* saves selected tab page and dialog size*/
bool CDlgAdvPreferences::SaveState() {
    wxString        strBaseConfigLocation = wxString(wxT("/DlgAdvPreferences/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);
	if (!pConfig) return false;

	pConfig->SetPath(strBaseConfigLocation);
	pConfig->Write(wxT("CurrentPage"),m_Notebook->GetSelection());
	pConfig->Write(wxT("Width"),this->GetSize().GetWidth());
	pConfig->Write(wxT("Height"),this->GetSize().GetHeight());
	return true;
}

/* restores former selected tab page and dialog size*/
bool CDlgAdvPreferences::RestoreState() {
    wxString        strBaseConfigLocation = wxString(wxT("/DlgAdvPreferences/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
	int				iTemp,iTemp1;

	wxASSERT(pConfig);

    if (!pConfig) return false;

	pConfig->SetPath(strBaseConfigLocation);

	pConfig->Read(wxT("CurrentPage"), &iTemp,0);
	m_Notebook->SetSelection(iTemp);	
	pConfig->Read(wxT("Width"), &iTemp,-1);
	pConfig->Read(wxT("Height"), &iTemp1,-1);
	this->SetSize(iTemp,iTemp1);	

	return true;
}

/* read preferences from core client and initialize control values */
void CDlgAdvPreferences::ReadPreferenceSettings() {
	m_bInInit=true;//prevent dialog handlers from doing anything 
    CMainDocument* pDoc = wxGetApp().GetDocument();    	
	wxString buffer;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    //init prefs with defaults
	prefs.defaults();

	//override the global prefs with values in global_prefs_override.xml, if this file exists
	mask.clear();
	pDoc->rpc.get_global_prefs_override_struct(prefs, mask);

	// ######### proc usage page
	// do work between
	m_rbtProcEveryDay->SetValue(true);
	buffer.Printf(wxT("%02d:00"),prefs.start_hour);
	*m_txtProcEveryDayStart << buffer;
	buffer.Printf(wxT("%02d:00"),prefs.end_hour);
	*m_txtProcEveryDayStop << buffer;
	// on batteries
	m_chkProcOnBatteries->SetValue(prefs.run_on_batteries);
	// in use
	m_chkProcInUse->SetValue(prefs.run_if_user_active);
	// idle for X minutes
	buffer.Printf(wxT("%.2f"),prefs.idle_time_to_run);
	*m_txtProcIdleFor << buffer;
	// siwtch every X minutes
	buffer.Printf(wxT("%.2f"),prefs.cpu_scheduling_period_minutes);
	*m_txtProcSwitchEvery << buffer;
	// max cpus
	buffer.Printf(wxT("%d"),prefs.max_cpus);
	*m_txtProcUseProcessors << buffer;
	//cpu limit
	buffer.Printf(wxT("%.2f"),prefs.cpu_usage_limit);
	*m_txtProcUseCPUTime << buffer;

	// ######### net usage page
	// use network between
	m_rbtNetEveryDay->SetValue(true);
	buffer.Printf(wxT("%02d:00"),prefs.net_start_hour);
	*m_txtNetEveryDayStart << buffer;
	buffer.Printf(wxT("%02d:00"),prefs.net_end_hour);
	*m_txtNetEveryDayStop << buffer;
	// connection interval
	buffer.Printf(wxT("%01.4f"),prefs.work_buf_min_days);
	*m_txtNetConnectInterval << buffer;
	//download rate
	buffer.Printf(wxT("%.2f"),prefs.max_bytes_sec_down / 1024);
	*m_txtNetDownloadRate << buffer;
	// upload rate
	buffer.Printf(wxT("%.2f"),prefs.max_bytes_sec_up / 1024);
	*m_txtNetUploadRate << buffer;
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
	m_bInInit=false;
	//update control states
	this->UpdateControlStates();
}

/* write overridden preferences to disk (global_prefs_override.xml) */
bool CDlgAdvPreferences::SavePreferencesSettings() {
	double td;
	long tl;

	mask.clear();
	//proc page
	prefs.run_on_batteries=m_chkProcOnBatteries->GetValue();
	mask.run_on_batteries=true;
	//
	prefs.run_if_user_active=m_chkProcInUse->GetValue();
	mask.run_if_user_active=true;
	//
	if(m_txtProcIdleFor->IsEnabled()) {
		m_txtProcIdleFor->GetValue().ToLong(&tl);
		prefs.idle_time_to_run=tl;
		mask.idle_time_to_run=true;
	}
	//
	if(m_txtProcEveryDayStart->IsEnabled()) {
		m_txtProcEveryDayStart->GetValue().ToLong(&tl);
		prefs.start_hour=tl;
		mask.start_hour = true;        
	}
	//
	if(m_txtProcEveryDayStop->IsEnabled()) {
		m_txtProcEveryDayStop->GetValue().ToLong(&tl);
		prefs.end_hour=tl;
		mask.end_hour = true;        
	}
	//
	m_txtProcSwitchEvery->GetValue().ToDouble(&td);
	prefs.cpu_scheduling_period_minutes=td;
	mask.cpu_scheduling_period_minutes=true;
	//
	m_txtProcUseProcessors->GetValue().ToLong(&tl);
	prefs.max_cpus=tl;;
	mask.max_cpus=true;
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
	if(m_txtNetEveryDayStart->IsEnabled()) {
		m_txtNetEveryDayStart->GetValue().ToLong(&tl);
		prefs.net_start_hour=tl;
		mask.net_start_hour = true;        
	}
	//
	if(m_txtNetEveryDayStop->IsEnabled()) {
		m_txtNetEveryDayStop->GetValue().ToLong(&tl);
		prefs.net_end_hour=tl;
		mask.net_end_hour = true;        
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
	prefs.disk_max_used_pct=td;
	mask.disk_max_used_pct=true;
	//
	m_txtDiskWriteToDisk->GetValue().ToDouble(&td);
	prefs.disk_interval=td;
	mask.disk_interval=true;
	//
	m_txtDiskMaxSwap->GetValue().ToDouble(&td);
	td = td / 100.0 ;
	prefs.vm_max_used_frac=td;
	mask.vm_max_used_frac=true;
	//Memory
	m_txtMemoryMaxInUse->GetValue().ToDouble(&td);
	td = td / 100.0;
	prefs.ram_max_used_busy_frac=td;
	mask.ram_max_used_busy_frac=true;
	//
	m_txtMemoryMaxOnIdle->GetValue().ToDouble(&td);
	td = td / 100.0;
	prefs.ram_max_used_idle_frac=td;
	mask.ram_max_used_idle_frac=true;
	//
	prefs.leave_apps_in_memory = m_chkMemoryWhileSuspended->GetValue();
	mask.leave_apps_in_memory=true;
	return true;
}

/* set state of control depending on other control's state */
void CDlgAdvPreferences::UpdateControlStates() {
	//proc usage page
	if(m_rbtProcEveryDay->GetValue()) {
		m_panelProcSpecialTimes->Disable();
		m_txtProcEveryDayStart->Enable();
		m_txtProcEveryDayStop->Enable();
	}
	else {
		m_panelProcSpecialTimes->Enable();
		m_txtProcEveryDayStart->Disable();
		m_txtProcEveryDayStop->Disable();
	}
	m_txtProcIdleFor->Enable(!m_chkProcInUse->IsChecked());
	m_txtProcMonday->Enable(m_chkProcMonday->IsChecked());
	m_txtProcTuesday->Enable(m_chkProcTuesday->IsChecked());
	m_txtProcWednesday->Enable(m_chkProcWednesday->IsChecked());
	m_txtProcThursday->Enable(m_chkProcThursday->IsChecked());
	m_txtProcFriday->Enable(m_chkProcFriday->IsChecked());
	m_txtProcSaturday->Enable(m_chkProcSaturday->IsChecked());
	m_txtProcSunday->Enable(m_chkProcSunday->IsChecked());

	//net usage page
	if(m_rbtNetEveryDay->GetValue()) {
		m_panelNetSpecialTimes->Disable();
		m_txtNetEveryDayStart->Enable();
		m_txtNetEveryDayStop->Enable();
	}
	else {
		m_panelNetSpecialTimes->Enable();
		m_txtNetEveryDayStart->Disable();
		m_txtNetEveryDayStop->Disable();
	}
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
	wxString buffer;
	//proc page
	if(m_txtProcIdleFor->IsEnabled()) {
		buffer = m_txtProcIdleFor->GetValue();
		if(!IsValidFloatValue(buffer)) {
			ShowErrorMessage(m_txtProcIdleFor);
			return false;
		}
	}
	if(m_txtProcEveryDayStart->IsEnabled()) {
		buffer = m_txtProcEveryDayStart->GetValue();
		if(!IsValidTimeValue(buffer)) {
			ShowErrorMessage(m_txtProcEveryDayStart);
			return false;
		}
	}
	if(m_txtProcEveryDayStop->IsEnabled()) {
		buffer = m_txtProcEveryDayStop->GetValue();
		if(!IsValidTimeValue(buffer)) {
			ShowErrorMessage(m_txtProcEveryDayStop);
			return false;
		}
	}
	//all text ctrls in proc special time panel
	if(m_panelProcSpecialTimes->IsEnabled()) {
		wxWindowList children = m_panelProcSpecialTimes->GetChildren();
		wxWindowListNode* node = children.GetFirst();
		while(node) {
			if(node->GetData()->IsKindOf(CLASSINFO(wxTextCtrl))) {
				wxTextCtrl*  txt = wxDynamicCast(node->GetData(),wxTextCtrl);
				if(txt) {
					if(txt->IsEnabled()) {
						buffer = txt->GetValue();
						if(!IsValidTimeIntervalValue(buffer)) {
							ShowErrorMessage(txt);
							return false;
						}
					}
				}
			}
			node = node->GetNext();
		}		
	}
	//net page
	if(m_txtNetEveryDayStart->IsEnabled()) {
		buffer = m_txtNetEveryDayStart->GetValue();
		if(!IsValidTimeValue(buffer)) {
			ShowErrorMessage(m_txtNetEveryDayStart);
			return false;
		}
	}
	if(m_txtNetEveryDayStop->IsEnabled()) {
		buffer = m_txtNetEveryDayStop->GetValue();
		if(!IsValidTimeValue(buffer)) {
			ShowErrorMessage(m_txtNetEveryDayStop);
			return false;
		}
	}
	//all text ctrls in net special time panel
	if(m_panelNetSpecialTimes->IsEnabled()) {
		wxWindowList children = m_panelNetSpecialTimes->GetChildren();
		wxWindowListNode* node = children.GetFirst();
		while(node) {
			if(node->GetData()->IsKindOf(CLASSINFO(wxTextCtrl))) {
				wxTextCtrl*  txt = wxDynamicCast(node->GetData(),wxTextCtrl);
				if(txt) {
					if(txt->IsEnabled()) {
						buffer = txt->GetValue();
						if(!IsValidTimeIntervalValue(buffer)) {
							ShowErrorMessage(txt);
							return false;
						}
					}//if(txt->IsEnabled())
				}//if(txt)
			}//if(node->GetData()
			node = node->GetNext();
		}		
	}

	return true;
}

/* ensures that the page whioch contains txtCtrl is selected */
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
void CDlgAdvPreferences::ShowErrorMessage(wxTextCtrl* errorCtrl) {
	wxASSERT(this->EnsureTabPageVisible(errorCtrl));
	errorCtrl->SetFocus();	
	//
	wxMessageBox(_("invalid value detected"),_("Validation Error"),wxOK | wxCENTRE | wxICON_ERROR,this);
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
	if(stopChar==NULL) {//conversion failed
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
	if(dtStart>=dtStop) {
		return false;
	}
	return true;
}

// ------------ Event handlers starts here
// -------- generic command handler
// handles all control command events 
void CDlgAdvPreferences::OnHandleCommandEvent(wxCommandEvent& ev) {
	ev.Skip();
	if(!m_bInInit) {
		m_bDataChanged=true;
	}
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
	if(SavePreferencesSettings()) {
		pDoc->rpc.set_global_prefs_override_struct(prefs,mask);		
		pDoc->rpc.read_global_prefs_override();
	}
	ev.Skip();
}

// handles Help button clicked
void CDlgAdvPreferences::OnHelp(wxCommandEvent& ev) {
	wxString url = wxGetApp().GetSkinManager()->GetAdvanced()->GetCompanyWebsite();
	url += wxT("/prefs.php");//this seems not the right url, but which instead ?
	wxHyperLink::ExecuteLink(url);
	ev.Skip();
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
	int res = wxMessageBox(_("Do you really want to clear all local preferences ?"),
		_("Confirmation"),wxCENTER | wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT,this);
	
	return res==wxYES;
}
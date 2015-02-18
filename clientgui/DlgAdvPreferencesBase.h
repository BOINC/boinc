// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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


// This code was initially generated with wxFormBuilder (version Oct 13 2006)
// http://www.wxformbuilder.org/
//

#ifndef __DlgAdvPreferencesBase__
#define __DlgAdvPreferencesBase__

// Define WX_GCH in order to support precompiled headers with GCC compiler.
// You have to create the header "wx_pch.h" and include all files needed
// for compile your gui inside it.
// Then, compile it and place the file "wx_pch.h.gch" into the same
// directory that "wx_pch.h".
#ifdef WX_GCH
#include <wx_pch.h>
#else
#include <wx/wx.h>
#endif

#include <wx/button.h>
#include <wx/notebook.h> 
#include <wx/panel.h>
#include <wx/statbmp.h>

///////////////////////////////////////////////////////////////////////////

#define ID_DEFAULT wxID_ANY // Default
#define ID_ADV_PREFS_START 20000
enum {
    ID_BTN_CLEAR = ID_ADV_PREFS_START,
    // These 7 must be in sequence
    ID_CHKNETSUNDAY,
    ID_CHKNETMONDAY,
    ID_CHKNETTUESDAY,
    ID_CHKNETWEDNESDAY,
    ID_CHKNETTHURSDAY,
    ID_CHKNETFRIDAY,
    ID_CHKNETSATURDAY,

    // These 7 must be in sequence
    ID_CHKPROCSUNDAY,
    ID_CHKPROCMONDAY,
    ID_CHKPROCTUESDAY,
    ID_CHKPROCWEDNESDAY,
    ID_CHKPROCTHURSDAY,
    ID_CHKPROCFRIDAY,
    ID_CHKPROCSATURDAY,

    ID_CHKDAILYXFERLIMIT,
    ID_CHKDISKLEASTFREE,
    ID_CHKDISKMAXOFTOTAL,
    ID_CHKDISKMAXSPACE,
    ID_CHKMEMORYWHILESUSPENDED,
    ID_CHKNETCONFIRMBEFORECONNECT,
    ID_CHKNETDISCONNECTWHENDONE,
    ID_CHKNETDOWNLOADRATE,
    ID_CHKNETEVERYDAY,
    ID_CHKNETSKIPIMAGEVERIFICATION,
    ID_CHKNETUPLOADRATE,
    ID_CHKPROCEVERYDAY,
    ID_CHKPROCINUSE,
    ID_CHKMAXLOAD,
    ID_CHKPROCONBATTERIES,
    ID_TABPAGE_DISK,
    ID_TABPAGE_NET,
    ID_TABPAGE_PROC,
    ID_TXTDISKLEASTFREE,
    ID_TXTDISKMAXOFTOTAL,
    ID_TXTDISKMAXSPACE,
    ID_TXTDISKWRITETODISK,
    ID_TXTMEMORYMAXINUSE,
    ID_TXTMEMORYMAXONIDLE,
    ID_TXTNETADDITIONALDAYS,
    ID_TXTNETCONNECTINTERVAL,
    ID_TXTNETDOWNLOADRATE,
    ID_TXTNETEVERYDAYSTART,
    ID_TXTNETEVERYDAYSTOP,
    ID_TXTNETFRIDAY,
    ID_TXTNETMONDAY,
    ID_TXTNETSATURDAY,
    ID_TXTNETSUNDAY,
    ID_TXTNETTHURSDAY,
    ID_TXTNETTUESDAY,
    ID_TXTNETUPLOADRATE,
    ID_TXTNETWEDNESDAY,
    ID_TXTPOCUSECPUTIME,
    ID_TXTPROCEVERYDAYSTART,
    ID_TXTPROCEVERYDAYSTOP,
    ID_TXTPROCFRIDAY,
    ID_TXTPROCIDLEFOR,
    ID_TXTPROCMONDAY,
    ID_TXTPROCSATURDAY,
    ID_TXTPROCSUNDAY,
    ID_TXTPROCSWITCHEVERY,
    ID_TXTPROCTHURSDAY,
    ID_TXTPROCTUESDAY,
    ID_TXTPROCUSEPROCESSORS,
    ID_TXTPROCWEDNESDAY,
    ID_CHKGPUPROCINUSE,
    ID_TXTMAXLOAD,
    ID_DAILY_XFER_LIMIT_MB,
    ID_DAILY_XFER_PERIOD_DAYS,
    ID_ADV_PREFS_LAST
};


/**
 * Class CDlgAdvPreferencesBase
 */
class CDlgAdvPreferencesBase : public wxDialog 
{
protected:
    wxStaticBitmap* m_bmpWarning;
    wxButton* m_btnClear;
    wxPanel* m_panelControls;
    wxNotebook* m_Notebook;
    wxPanel* m_panelProcessor;
    wxTextCtrl* m_txtProcUseProcessors;
    wxTextCtrl* m_txtProcUseCPUTime;
    wxCheckBox* m_chkProcOnBatteries;
    wxCheckBox* m_chkProcInUse;
    wxCheckBox* m_chkGPUProcInUse;
    wxTextCtrl* m_txtProcIdleFor;
    wxCheckBox* m_chkMaxLoad;
    wxTextCtrl* m_txtMaxLoad;
    wxCheckBox* m_chkNetEveryDay;
    wxCheckBox* m_chkProcEveryDay;
    wxTextCtrl* m_txtProcEveryDayStart;
    wxTextCtrl* m_txtProcEveryDayStop;
    wxPanel* m_panelProcSpecialTimes;
    wxCheckBox* m_chkProcMonday;
    wxTextCtrl* m_txtProcMonday;
    wxCheckBox* m_chkProcTuesday;
    wxTextCtrl* m_txtProcTuesday;
    wxCheckBox* m_chkProcWednesday;
    wxTextCtrl* m_txtProcWednesday;
    wxCheckBox* m_chkProcThursday;
    wxTextCtrl* m_txtProcThursday;
    wxCheckBox* m_chkProcFriday;
    wxTextCtrl* m_txtProcFriday;
    wxCheckBox* m_chkProcSaturday;
    wxTextCtrl* m_txtProcSaturday;
    wxCheckBox* m_chkProcSunday;
    wxTextCtrl* m_txtProcSunday;
    wxTextCtrl* m_txtProcSwitchEvery;
    wxTextCtrl* m_txtDiskWriteToDisk;
    wxPanel* m_panelNetwork;
    wxCheckBox* m_chkNetDownloadRate;
    wxTextCtrl* m_txtNetDownloadRate;
    wxCheckBox* m_chkNetUploadRate;
    wxTextCtrl* m_txtNetUploadRate;

    wxCheckBox * m_chk_daily_xfer_limit;
    wxTextCtrl* m_txt_daily_xfer_limit_mb;
    wxTextCtrl* m_txt_daily_xfer_period_days;

    wxTextCtrl* m_txtNetConnectInterval;
    wxTextCtrl* m_txtNetAdditionalDays;
    wxCheckBox* m_chkNetSkipImageVerification;
    wxCheckBox* m_chkNetConfirmBeforeConnect;
    wxCheckBox* m_chkNetDisconnectWhenDone;
    wxTextCtrl* m_txtNetEveryDayStart;
    wxTextCtrl* m_txtNetEveryDayStop;
    wxPanel* m_panelNetSpecialTimes;
    wxCheckBox* m_chkNetMonday;
    wxTextCtrl* m_txtNetMonday;
    wxCheckBox* m_chkNetTuesday;
    wxTextCtrl* m_txtNetTuesday;
    wxCheckBox* m_chkNetWednesday;
    wxTextCtrl* m_txtNetWednesday;
    wxCheckBox* m_chkNetThursday;
    wxTextCtrl* m_txtNetThursday;
    wxCheckBox* m_chkNetFriday;
    wxTextCtrl* m_txtNetFriday;
    wxCheckBox* m_chkNetSaturday;
    wxTextCtrl* m_txtNetSaturday;
    wxCheckBox* m_chkNetSunday;
    wxTextCtrl* m_txtNetSunday;
    wxPanel* m_panelDiskAndMemory;
    wxCheckBox* m_chkDiskMaxSpace;
    wxTextCtrl* m_txtDiskMaxSpace;
    wxCheckBox* m_chkDiskLeastFree;
    wxTextCtrl* m_txtDiskLeastFree;
    wxCheckBox* m_chkDiskMaxOfTotal;
    wxTextCtrl* m_txtDiskMaxOfTotal;
    wxTextCtrl* m_txtDiskMaxSwap;
    wxTextCtrl* m_txtMemoryMaxInUse;
    wxTextCtrl* m_txtMemoryMaxOnIdle;
    wxCheckBox* m_chkMemoryWhileSuspended;
    wxPanel* m_panelDailySchedules;
    
    wxPanel* m_panelButtons;
    wxButton* m_btnOK;
    wxButton* m_btnCancel;
    wxButton* m_btnHelp;
    
    wxString *web_prefs_url;

public:
    CDlgAdvPreferencesBase( wxWindow* parent, int id = -1, wxString title = wxT(""), wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxDEFAULT_DIALOG_STYLE );

private:
    wxPanel* createProcessorTab(wxNotebook* notebook);
    wxPanel* createNetworkTab(wxNotebook* notebook);
    wxPanel* createDiskAndMemoryTab(wxNotebook* notebook);
    wxPanel* createDailySchedulesTab(wxNotebook* notebook);
    wxSize getTextCtrlSize(wxString maxText);
    bool doesLocalPrefsFileExist();
    void makeStaticBoxLabelItalic(wxStaticBox* staticBox);
};

#endif //__DlgAdvPreferencesBase__

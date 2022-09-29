// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

#ifndef BOINC_DLGADVPREFERENCESBASE_H
#define BOINC_DLGADVPREFERENCESBASE_H

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

#define PROC_DAY_OF_WEEK_TOOLTIP_TEXT _("On this day of the week, compute only during these hours.")
#define NET_DAY_OF_WEEK_TOOLTIP_TEXT _("On this day of the week, transfer files only during these hours.")

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
    ID_CHKMAXLOADNOTINUSE,
    ID_CHKPROCONBATTERIES,
    ID_TABPAGE_SCHED,
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
    ID_TXTNETFRIDAYSTART,
    ID_TXTNETFRIDAYSTOP,
    ID_TXTNETMONDAYSTART,
    ID_TXTNETMONDAYSTOP,
    ID_TXTNETSATURDAYSTART,
    ID_TXTNETSATURDAYSTOP,
    ID_TXTNETSUNDAYSTART,
    ID_TXTNETSUNDAYSTOP,
    ID_TXTNETTHURSDAYSTART,
    ID_TXTNETTHURSDAYSTOP,
    ID_TXTNETTUESDAYSTART,
    ID_TXTNETTUESDAYSTOP,
    ID_TXTNETUPLOADRATE,
    ID_TXTNETWEDNESDAYSTART,
    ID_TXTNETWEDNESDAYSTOP,
    ID_TXTPROCUSECPUTIME,
    ID_TXTPROCUSECPUTIMENOTINUSE,
    ID_TXTPROCEVERYDAYSTART,
    ID_TXTPROCEVERYDAYSTOP,
    ID_TXTPROCFRIDAYSTART,
    ID_TXTPROCFRIDAYSTOP,
    ID_TXTPROCIDLEFOR,
    ID_TXTNORECENTINPUT,
    ID_TXTPROCMONDAYSTART,
    ID_TXTPROCMONDAYSTOP,
    ID_TXTPROCSATURDAYSTART,
    ID_TXTPROCSATURDAYSTOP,
    ID_TXTPROCSUNDAYSTART,
    ID_TXTPROCSUNDAYSTOP,
    ID_TXTPROCSWITCHEVERY,
    ID_TXTPROCTHURSDAYSTART,
    ID_TXTPROCTHURSDAYSTOP,
    ID_TXTPROCTUESDAYSTART,
    ID_TXTPROCTUESDAYSTOP,
    ID_TXTPROCUSEPROCESSORS,
    ID_TXTPROCUSEPROCESSORSNOTINUSE,
    ID_TXTPROCWEDNESDAYSTART,
    ID_TXTPROCWEDNESDAYSTOP,
    ID_CHKGPUPROCINUSE,
    ID_CHKNORECENTINPUT,
    ID_TXTMAXLOAD,
    ID_TXTMAXLOADNOTINUSE,
    ID_DAILY_XFER_LIMIT_MB,
    ID_DAILY_XFER_PERIOD_DAYS,
    ID_ADV_PREFS_LAST
};

class CDlgAdvPreferencesBase : public wxDialog {
protected:
    wxStaticBitmap* m_bmpWarning;
    wxButton* m_btnClear;
    wxPanel* m_panelControls;
    wxNotebook* m_Notebook;

    // Computing panel
    //
    wxScrolledWindow* m_panelProcessor;
    // In-use items
    wxTextCtrl* m_txtProcIdleFor;
    wxCheckBox* m_chkProcInUse;
    wxCheckBox* m_chkGPUProcInUse;
    wxTextCtrl* m_txtProcUseProcessors;
    wxTextCtrl* m_txtProcUseCPUTime;
    wxCheckBox* m_chkMaxLoad;
    wxTextCtrl* m_txtMaxLoad;
    wxTextCtrl* m_txtMemoryMaxInUse;
    // Not in Use items
    //
    wxTextCtrl* m_txtProcUseProcessorsNotInUse;
    wxTextCtrl* m_txtProcUseCPUTimeNotInUse;
    wxCheckBox* m_chkMaxLoadNotInUse;
    wxTextCtrl* m_txtMaxLoadNotInUse;
    wxTextCtrl* m_txtMemoryMaxOnIdle;
    wxCheckBox* m_chkNoRecentInput;
    wxTextCtrl* m_txtNoRecentInput;
    // General items
    //
    wxCheckBox* m_chkProcOnBatteries;
    wxTextCtrl* m_txtProcSwitchEvery;
    wxTextCtrl* m_txtDiskWriteToDisk;
    wxCheckBox* m_chkMemoryWhileSuspended;
    wxTextCtrl* m_txtNetConnectInterval;
    wxTextCtrl* m_txtNetAdditionalDays;
    wxTextCtrl* m_txtDiskMaxSwap;

    // Network panel
    //
    wxPanel* m_panelNetwork;
    wxCheckBox* m_chkNetDownloadRate;
    wxTextCtrl* m_txtNetDownloadRate;
    wxCheckBox* m_chkNetUploadRate;
    wxTextCtrl* m_txtNetUploadRate;

    wxCheckBox* m_chk_daily_xfer_limit;
    wxTextCtrl* m_txt_daily_xfer_limit_mb;
    wxTextCtrl* m_txt_daily_xfer_period_days;

    wxCheckBox* m_chkNetSkipImageVerification;
    wxCheckBox* m_chkNetConfirmBeforeConnect;
    wxCheckBox* m_chkNetDisconnectWhenDone;

    // Disk panel
    //
    wxPanel* m_panelDisk;
    wxCheckBox* m_chkDiskMaxSpace;
    wxTextCtrl* m_txtDiskMaxSpace;
    wxCheckBox* m_chkDiskLeastFree;
    wxTextCtrl* m_txtDiskLeastFree;
    wxCheckBox* m_chkDiskMaxOfTotal;
    wxTextCtrl* m_txtDiskMaxOfTotal;

    // Daily schedules panel
    wxPanel* m_panelDailySchedules;
    wxCheckBox* m_chkNetEveryDay;
    wxCheckBox* m_chkProcEveryDay;
    wxTextCtrl* m_txtProcEveryDayStart;
    wxTextCtrl* m_txtProcEveryDayStop;
    wxCheckBox* m_chkProcMonday;
    wxTextCtrl* m_txtProcMondayStart;
    wxTextCtrl* m_txtProcMondayStop;
    wxCheckBox* m_chkProcTuesday;
    wxTextCtrl* m_txtProcTuesdayStart;
    wxTextCtrl* m_txtProcTuesdayStop;
    wxCheckBox* m_chkProcWednesday;
    wxTextCtrl* m_txtProcWednesdayStart;
    wxTextCtrl* m_txtProcWednesdayStop;
    wxCheckBox* m_chkProcThursday;
    wxTextCtrl* m_txtProcThursdayStart;
    wxTextCtrl* m_txtProcThursdayStop;
    wxCheckBox* m_chkProcFriday;
    wxTextCtrl* m_txtProcFridayStart;
    wxTextCtrl* m_txtProcFridayStop;
    wxCheckBox* m_chkProcSaturday;
    wxTextCtrl* m_txtProcSaturdayStart;
    wxTextCtrl* m_txtProcSaturdayStop;
    wxCheckBox* m_chkProcSunday;
    wxTextCtrl* m_txtProcSundayStart;
    wxTextCtrl* m_txtProcSundayStop;

    wxTextCtrl* m_txtNetEveryDayStart;
    wxTextCtrl* m_txtNetEveryDayStop;
    wxCheckBox* m_chkNetMonday;
    wxTextCtrl* m_txtNetMondayStart;
    wxTextCtrl* m_txtNetMondayStop;
    wxCheckBox* m_chkNetTuesday;
    wxTextCtrl* m_txtNetTuesdayStart;
    wxTextCtrl* m_txtNetTuesdayStop;
    wxCheckBox* m_chkNetWednesday;
    wxTextCtrl* m_txtNetWednesdayStart;
    wxTextCtrl* m_txtNetWednesdayStop;
    wxCheckBox* m_chkNetThursday;
    wxTextCtrl* m_txtNetThursdayStart;
    wxTextCtrl* m_txtNetThursdayStop;
    wxCheckBox* m_chkNetFriday;
    wxTextCtrl* m_txtNetFridayStart;
    wxTextCtrl* m_txtNetFridayStop;
    wxCheckBox* m_chkNetSaturday;
    wxTextCtrl* m_txtNetSaturdayStart;
    wxTextCtrl* m_txtNetSaturdayStop;
    wxCheckBox* m_chkNetSunday;
    wxTextCtrl* m_txtNetSundayStart;
    wxTextCtrl* m_txtNetSundayStop;

    wxPanel* m_panelButtons;
    wxButton* m_btnOK;
    wxButton* m_btnCancel;
    wxButton* m_btnHelp;

    wxString *web_prefs_url;
    bool m_bUsingLocalPrefs;

public:
    CDlgAdvPreferencesBase(
        wxWindow* parent, int id = -1, wxString title = wxT(""), wxPoint pos = wxDefaultPosition,
        wxSize size = wxDefaultSize, int style = wxDEFAULT_DIALOG_STYLE
    );

private:
    void addNewRowToSizer(wxSizer* toSizer, wxString& toolTipText,
        wxWindow* first, wxWindow* second, wxWindow* third,
        wxWindow* fourth=NULL, wxWindow* fifth=NULL
    );
    // variant with separate tooltip per item
    void add_row_to_sizer2(wxSizer* toSizer,
        wxWindow* item1, wxString& tt1,
        wxWindow* item2, wxString& tt2,
        wxWindow* item3, wxString& tt3,
        wxWindow* item4, wxString& tt4,
        wxWindow* item5, wxString& tt5
    );
    wxScrolledWindow* createProcessorTab(wxNotebook* notebook, bool bScrollable);
    wxPanel* createNetworkTab(wxNotebook* notebook);
    wxPanel* createDiskTab(wxNotebook* notebook);
    wxPanel* createDailySchedulesTab(wxNotebook* notebook);
    wxSize getTextCtrlSize(wxString maxText);
    bool doesLocalPrefsFileExist();
    void makeStaticBoxLabelItalic(wxStaticBox* staticBox);
};

#endif

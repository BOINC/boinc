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
#define ID_BTN_CLEAR 20000
#define ID_CHKMEMORYWHILESUSPENDED 20001
#define ID_CHKNETCONFIRMBEFORECONNECT 20002
#define ID_CHKNETDISCONNECTWHENDONE 20003
#define ID_CHKNETFRIDAY 20004
#define ID_CHKNETMONDAY 20005
#define ID_CHKNETSATURDAY 20006
#define ID_CHKNETSKIPIMAGEVERIFICATION 20007
#define ID_CHKNETSUNDAY 20008
#define ID_CHKNETTHURSDAY 20009
#define ID_CHKNETTUESDAY 20010
#define ID_CHKNETWEDNESDAY 20011
#define ID_CHKPROCFRIDAY 20012
#define ID_CHKPROCINUSE 20013
#define ID_CHKPROCMONDAY 20014
#define ID_CHKPROCONBATTERIES 20015
#define ID_CHKPROCSATURDAY 20016
#define ID_CHKPROCSUNDAY 20017
#define ID_CHKPROCTHURSDAY 20018
#define ID_CHKPROCTUESDAY 20019
#define ID_CHKPROCWEDNESDAY 20020
#define ID_TABPAGE_DISK 20021
#define ID_TABPAGE_NET 20022
#define ID_TABPAGE_PROC 20023
#define ID_TXTDISKLEASTFREE 20024
#define ID_TXTDISKMAXOFTOTAL 20025
#define ID_TXTDISKMAXSPACE 20026
#define ID_TXTDISKWRITETODISK 20027
#define ID_TXTMEMORYMAXINUSE 20028
#define ID_TXTMEMORYMAXONIDLE 20029
#define ID_TXTNETADDITIONALDAYS 20030
#define ID_TXTNETCONNECTINTERVAL 20031
#define ID_TXTNETDOWNLOADRATE 20032
#define ID_TXTNETEVERYDAYSTART 20033
#define ID_TXTNETEVERYDAYSTOP 20034
#define ID_TXTNETFRIDAY 20035
#define ID_TXTNETMONDAY 20036
#define ID_TXTNETSATURDAY 20037
#define ID_TXTNETSUNDAY 20038
#define ID_TXTNETTHURSDAY 20039
#define ID_TXTNETTUESDAY 20040
#define ID_TXTNETUPLOADRATE 20041
#define ID_TXTNETWEDNESDAY 20042
#define ID_TXTPOCUSECPUTIME 20043
#define ID_TXTPROCEVERYDAYSTART 20044
#define ID_TXTPROCEVERYDAYSTOP 20045
#define ID_TXTPROCFRIDAY 20046
#define ID_TXTPROCIDLEFOR 20047
#define ID_TXTPROCMONDAY 20048
#define ID_TXTPROCSATURDAY 20049
#define ID_TXTPROCSUNDAY 20050
#define ID_TXTPROCSWITCHEVERY 20051
#define ID_TXTPROCTHURSDAY 20052
#define ID_TXTPROCTUESDAY 20053
#define ID_TXTPROCUSEPROCESSORS 20054
#define ID_TXTPROCWEDNESDAY 20055
#define ID_CHKGPUPROCINUSE 20056
#define ID_TXTMAXLOAD 20057
#define ID_DAILY_XFER_LIMIT_MB  20058
#define ID_DAILY_XFER_PERIOD_DAYS  20059


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
    wxCheckBox* m_chkProcOnBatteries;
    wxCheckBox* m_chkProcInUse;
    wxCheckBox* m_chkGPUProcInUse;
    wxTextCtrl* m_txtProcIdleFor;
    wxTextCtrl* m_txtMaxLoad;
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
    wxTextCtrl* m_txtProcUseProcessors;
    wxTextCtrl* m_txtProcUseCPUTime;
    wxPanel* m_panelNetwork;
    wxTextCtrl* m_txtNetDownloadRate;
    wxTextCtrl* m_txtNetUploadRate;

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
    wxTextCtrl* m_txtDiskMaxSpace;
    wxTextCtrl* m_txtDiskLeastFree;
    wxTextCtrl* m_txtDiskMaxOfTotal;
    wxTextCtrl* m_txtDiskWriteToDisk;
    wxTextCtrl* m_txtDiskMaxSwap;
    wxTextCtrl* m_txtMemoryMaxInUse;
    wxTextCtrl* m_txtMemoryMaxOnIdle;
    wxCheckBox* m_chkMemoryWhileSuspended;
    
    wxPanel* m_panelButtons;
    wxButton* m_btnOK;
    wxButton* m_btnCancel;
    wxButton* m_btnHelp;

public:
    CDlgAdvPreferencesBase( wxWindow* parent, int id = -1, wxString title = wxT(""), wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxDEFAULT_DIALOG_STYLE );

private:
    wxPanel* createProcessorTab(wxNotebook* notebook);
    wxPanel* createNetworkTab(wxNotebook* notebook);
    wxPanel* createDiskAndMemoryTab(wxNotebook* notebook);
    wxSize getTextCtrlSize(wxString maxText);
};

#endif //__DlgAdvPreferencesBase__

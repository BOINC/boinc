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

#include "stdwx.h"
#include "Events.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "SkinManager.h"

#include "DlgAdvPreferencesBase.h"

///////////////////////////////////////////////////////////////////////////

// NOTE: On MS Windows with wxWidgets 3.0, controls inside a wxStaticBox 
// don't refresh properly unless they are children of the wxStaticBox!
//
CDlgAdvPreferencesBase::CDlgAdvPreferencesBase( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) :
    wxDialog( parent, id, title, pos, size, style )
{
    wxString strCaption = title;
    if (strCaption.IsEmpty()) {
        CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
        wxASSERT(pSkinAdvanced);
        wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        strCaption.Printf(_("%s - Computing preferences"), pSkinAdvanced->GetApplicationShortName().c_str());
    }

    this->SetExtraStyle( this->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY );
    this->Centre( wxBOTH );
    this->SetTitle(strCaption);

    wxBoxSizer* dialogSizer = new wxBoxSizer( wxVERTICAL );
    wxStaticBox* topControlsStaticBox = new wxStaticBox( this, -1, wxT("") );

    wxStaticBoxSizer* topControlsSizer = new wxStaticBoxSizer( topControlsStaticBox, wxHORIZONTAL );

    m_bmpWarning = new wxStaticBitmap( topControlsStaticBox, ID_DEFAULT, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
    m_bmpWarning->SetMinSize( wxSize( 48,48 ) );

    topControlsSizer->Add( m_bmpWarning, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    wxBoxSizer* legendSizer = new wxBoxSizer( wxVERTICAL );

    bool usingLocalPrefs = doesLocalPrefsFileExist();
    if (usingLocalPrefs) {
        legendSizer->Add(
            new wxStaticText( topControlsStaticBox, ID_DEFAULT,
                        _("Using local preferences.\n"
                        "Click \"Use web prefs\" to use web-based preferences from"
                        ), wxDefaultPosition, wxDefaultSize, 0 ),
            0, wxALL, 1
        );
    } else {
        legendSizer->Add(
            new wxStaticText( topControlsStaticBox, ID_DEFAULT,
                        _("Using web-based preferences from"),
                        wxDefaultPosition, wxDefaultSize, 0 ),
            0, wxALL, 1
        );
    }
    
     legendSizer->Add(
        new wxHyperlinkCtrl(
            topControlsStaticBox, wxID_ANY, *web_prefs_url, *web_prefs_url,
            wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE
        ),
        0, wxLEFT, 5
    );
    
    if (!usingLocalPrefs) {
        legendSizer->Add(
            new wxStaticText( topControlsStaticBox, ID_DEFAULT,
                 _("Set values and click OK to use local preferences instead."),
                 wxDefaultPosition, wxDefaultSize, 0 ),
            0, wxALL, 1
        );
    }
  
    topControlsSizer->Add( legendSizer, 1, wxALL, 1 );

#if 0
    wxStaticText* staticText321 = new wxStaticText( topControlsStaticBox, ID_DEFAULT, _("This dialog controls preferences for this computer only.\nClick OK to set preferences.\nClick Clear to restore web-based settings."), wxDefaultPosition, wxDefaultSize, 0 );
    topControlsSizer->Add( staticText321, 1, wxALL, 1 );

    m_btnClear = new wxButton( topControlsStaticBox, ID_BTN_CLEAR, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnClear->SetToolTip( _("clear all local preferences and close the dialog") );
#endif

    m_btnClear = new wxButton( topControlsStaticBox, ID_BTN_CLEAR, _("Use web prefs"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnClear->SetToolTip( _("restore web-based preferences and close the dialog") );
    if (!usingLocalPrefs) {
        m_btnClear->Hide();
    }
    
    topControlsSizer->Add( m_btnClear, 0, wxALIGN_BOTTOM|wxALL, 4 );

    dialogSizer->Add( topControlsSizer, 0, wxALL|wxEXPAND, 1 );

    m_panelControls = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_panelControls->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* notebookSizer = new wxBoxSizer( wxVERTICAL );

    m_Notebook = new wxNotebook( m_panelControls, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxNB_FLAT|wxNB_TOP );
    m_Notebook->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    m_panelProcessor = createProcessorTab(m_Notebook);
    m_Notebook->AddPage( m_panelProcessor, _("Computing"), false );

    m_panelNetwork = createNetworkTab(m_Notebook);
    m_Notebook->AddPage( m_panelNetwork, _("Network"), true );

    m_panelDiskAndMemory = createDiskAndMemoryTab(m_Notebook);
    m_Notebook->AddPage( m_panelDiskAndMemory, _("Disk and Memory"), false );

    m_panelDailySchedules = createDailySchedulesTab(m_Notebook);
    m_Notebook->AddPage( m_panelDailySchedules, _("Daily Schedules"), false );

    notebookSizer->Add( m_Notebook, 1, wxEXPAND | wxALL, 1 );

    m_panelControls->SetSizer( notebookSizer );
    m_panelControls->Layout();
    notebookSizer->Fit( m_panelControls );

    dialogSizer->Add( m_panelControls, 1, wxALL|wxEXPAND, 1 );

    m_panelButtons = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    m_btnOK = new wxButton( m_panelButtons, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnOK->SetToolTip( _("save all values and close the dialog") );
    m_btnOK->SetDefault();
    
    buttonSizer->Add( m_btnOK, 0, wxALL, 5 );

    m_btnCancel = new wxButton( m_panelButtons, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnCancel->SetToolTip( _("close the dialog without saving") );

    buttonSizer->Add( m_btnCancel, 0, wxALL, 5 );

    m_btnHelp = new wxButton( m_panelButtons, ID_HELPBOINC, _("Help"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnHelp->SetToolTip( _("shows the preferences web page") );

    buttonSizer->Add( m_btnHelp, 0, wxALL, 5 );

    m_panelButtons->SetSizer( buttonSizer );
    m_panelButtons->Layout();
    buttonSizer->Fit( m_panelButtons );
    dialogSizer->Add( m_panelButtons, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALL, 1 );

    dialogSizer->Fit( this );
    this->SetSizer( dialogSizer );
}

wxPanel* CDlgAdvPreferencesBase::createProcessorTab(wxNotebook* notebook)
{
    wxSize textCtrlSize = getTextCtrlSize(wxT("999.99"));
    
    wxPanel* processorTab = new wxPanel( notebook, ID_TABPAGE_PROC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    processorTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* processorTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* usageLimitsStaticBox = new wxStaticBox(processorTab, -1, _("Usage limits") );
    wxStaticBoxSizer* usageLimitsBoxSizer = new wxStaticBoxSizer(usageLimitsStaticBox, wxVERTICAL);
    
    wxFlexGridSizer* usageLimitsGridSizer = new wxFlexGridSizer( 2, 3, 0, 0 );
    usageLimitsGridSizer->AddGrowableCol( 2 );
    usageLimitsGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    usageLimitsGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    /*xgettext:no-c-format*/
    wxStaticText* m_staticText20 = new wxStaticText(
        usageLimitsStaticBox, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, 0 );
    usageLimitsGridSizer->Add( m_staticText20, 0, wxALL|wxEXPAND, 5 );

    m_txtProcUseProcessors = new wxTextCtrl( usageLimitsStaticBox, ID_TXTPROCUSEPROCESSORS, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    usageLimitsGridSizer->Add( m_txtProcUseProcessors, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText21 = new wxStaticText( usageLimitsStaticBox, ID_DEFAULT, _("% of the CPUs"), wxDefaultPosition, wxDefaultSize, 0 );
    usageLimitsGridSizer->Add( staticText21, 0, wxALL, 5 );

    wxStaticText* m_staticText22 = new wxStaticText(
        usageLimitsStaticBox, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, 0 );
    usageLimitsGridSizer->Add( m_staticText22, 0, wxALL|wxEXPAND, 5 );

    usageLimitsBoxSizer->Add( usageLimitsGridSizer, 0, wxEXPAND, 1 );

    m_txtProcUseCPUTime = new wxTextCtrl( usageLimitsStaticBox, ID_TXTPOCUSECPUTIME, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    usageLimitsGridSizer->Add( m_txtProcUseCPUTime, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText23 = new wxStaticText( usageLimitsStaticBox, ID_DEFAULT, _("% of CPU time"), wxDefaultPosition, wxDefaultSize, 0 );
    usageLimitsGridSizer->Add( staticText23, 0, wxALL, 5 );

    processorTabSizer->Add( usageLimitsBoxSizer, 0, wxEXPAND, 1 );
    
    wxStaticBox* suspendComputingStaticBox = new wxStaticBox(processorTab, -1, _("When to suspend") );
    wxStaticBoxSizer* suspendComputingBoxSizer = new wxStaticBoxSizer(suspendComputingStaticBox, wxVERTICAL);
    
    m_chkProcOnBatteries = new wxCheckBox(
        suspendComputingStaticBox, ID_CHKPROCONBATTERIES,
        _("Suspend when computer is on batteries"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_chkProcOnBatteries->SetToolTip(
        _("check this if you don't want this computer to do work while it runs on batteries")
    );
    suspendComputingBoxSizer->Add( m_chkProcOnBatteries, 0, wxALL, 5 );

    m_chkProcInUse = new wxCheckBox(
        suspendComputingStaticBox, ID_CHKPROCINUSE,
        _("Suspend when computer is in use"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_chkProcInUse->SetToolTip(
        _("check this if you don't want this computer to do work when you're using it")
    );
    suspendComputingBoxSizer->Add( m_chkProcInUse, 0, wxALL, 5 );

    m_chkGPUProcInUse = new wxCheckBox(
        suspendComputingStaticBox, ID_CHKGPUPROCINUSE,
        _("Suspend GPU when computer is in use"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_chkGPUProcInUse->SetToolTip(
        _("check this if you don't want your GPU to do work when you're using the computer")
    );
    suspendComputingBoxSizer->Add( m_chkGPUProcInUse, 0, wxALL, 5 );

    // min idle time
    wxFlexGridSizer* procIdleSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
    procIdleSizer->AddGrowableCol( 3 );
    procIdleSizer->SetFlexibleDirection( wxHORIZONTAL );
    procIdleSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    procIdleSizer->Add(
        new wxStaticText(
            suspendComputingStaticBox, ID_DEFAULT,
            _("'In use' means mouse/keyboard input in last"),
            wxDefaultPosition, wxDefaultSize, 0
        ),
        0, wxALL, 5
    );
    m_txtProcIdleFor = new wxTextCtrl(
        suspendComputingStaticBox, ID_TXTPROCIDLEFOR, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("999.99")), wxTE_RIGHT
    );
    m_txtProcIdleFor->SetToolTip(
        _("do work only after you haven't used the computer for this number of minutes")
    );
    procIdleSizer->Add( m_txtProcIdleFor, 0, wxALL, 1 );
    procIdleSizer->Add(
        new wxStaticText(
            suspendComputingStaticBox, ID_DEFAULT, _("minutes"),
            wxDefaultPosition, wxDefaultSize, 0
        ),
        0, wxALL, 5
    );
    procIdleSizer->Add(
        new wxStaticText( suspendComputingStaticBox, ID_DEFAULT, wxT(""), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );

    suspendComputingBoxSizer->Add( procIdleSizer, 0, wxEXPAND, 5);

    // max CPU load
    wxFlexGridSizer* maxLoadSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
    maxLoadSizer->AddGrowableCol( 3 );
    maxLoadSizer->SetFlexibleDirection( wxHORIZONTAL );
    maxLoadSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    m_chkMaxLoad = new wxCheckBox(
        suspendComputingStaticBox, ID_CHKMAXLOAD,
        _("Suspend when non-BOINC CPU usage is above"), wxDefaultPosition, wxDefaultSize, 0
    );
    maxLoadSizer->Add( m_chkMaxLoad, 0, wxALL, 5 );
    m_txtMaxLoad = new wxTextCtrl(
        suspendComputingStaticBox, ID_TXTMAXLOAD, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("100.00")), wxTE_RIGHT
    );
    m_txtMaxLoad->SetToolTip(
        _("suspend work if processor usage exceeds this level")
    );
    maxLoadSizer->Add( m_txtMaxLoad, 0, wxALL, 1 );
    maxLoadSizer->Add(
        new wxStaticText(
            suspendComputingStaticBox, ID_DEFAULT, _("percent"),
            wxDefaultPosition, wxDefaultSize, 0
        ),
        0, wxALL, 5
    );
    maxLoadSizer->Add(
        new wxStaticText( suspendComputingStaticBox, ID_DEFAULT, wxT(""), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );
    suspendComputingBoxSizer->Add( maxLoadSizer, 0, wxEXPAND, 5);

    processorTabSizer->Add( suspendComputingBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBox* miscProcStaticBox = new wxStaticBox( processorTab, -1, _("Other options") );
    wxStaticBoxSizer* miscProcBoxSizer = new wxStaticBoxSizer( miscProcStaticBox, wxVERTICAL );

    wxFlexGridSizer* miscProcGridSizer = new wxFlexGridSizer( 4, 3, 0, 0 );
    miscProcGridSizer->AddGrowableCol( 2 );
    miscProcGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    miscProcGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    // buffer sizes

    wxStaticText* staticText30 = new wxStaticText(
        miscProcStaticBox, ID_DEFAULT,
        _("Maintain at least"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT
    );
    miscProcGridSizer->Add( staticText30, 0, wxALL|wxEXPAND, 5 );

    m_txtNetConnectInterval = new wxTextCtrl(
        miscProcStaticBox, ID_TXTNETCONNECTINTERVAL, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );
    m_txtNetConnectInterval->SetToolTip(
        _("Try to maintain enough tasks to keep busy for this many days")
    );

    miscProcGridSizer->Add( m_txtNetConnectInterval, 0, wxALL, 1 );

    wxStaticText* staticText31 = new wxStaticText(
        miscProcStaticBox, ID_DEFAULT, _("days of work"), wxDefaultPosition, wxDefaultSize, 0
    );
    miscProcGridSizer->Add( staticText31, 0, wxALL, 5 );

    wxStaticText* staticText331 = new wxStaticText(
        miscProcStaticBox, ID_DEFAULT,
        _("Allow an additional"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT
    );
    miscProcGridSizer->Add( staticText331, 0, wxALL|wxEXPAND, 5 );

    m_txtNetAdditionalDays = new wxTextCtrl(
        miscProcStaticBox, ID_TXTNETADDITIONALDAYS, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );
    m_txtNetAdditionalDays->SetToolTip(
        _("In addition, maintain enough tasks for up to this many days")
    );
    miscProcGridSizer->Add( m_txtNetAdditionalDays, 0, wxALL, 1 );

    wxBoxSizer* workBufAdditonalDaysSizer = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* staticText341 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("days of work to be cached"), wxDefaultPosition, wxDefaultSize, 0 );
    workBufAdditonalDaysSizer->Add( staticText341, 0, 0, 0 );

    miscProcGridSizer->Add( workBufAdditonalDaysSizer, 0, wxALL, 5 );

    wxStaticText* staticText18 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("Switch between applications every"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    miscProcGridSizer->Add( staticText18, 0, wxALL|wxEXPAND, 5 );
    
    m_txtProcSwitchEvery = new wxTextCtrl( miscProcStaticBox, ID_TXTPROCSWITCHEVERY, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    miscProcGridSizer->Add( m_txtProcSwitchEvery, 0, wxALL, 1 );

    wxStaticText* staticText19 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
    miscProcGridSizer->Add( staticText19, 0, wxALL, 5 );

    wxStaticText* staticText46 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("Tasks checkpoint to disk at most every"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    miscProcGridSizer->Add( staticText46, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskWriteToDisk = new wxTextCtrl( miscProcStaticBox, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    miscProcGridSizer->Add( m_txtDiskWriteToDisk, 0, wxALL, 1 );

    wxStaticText* staticText47 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
    miscProcGridSizer->Add( staticText47, 0, wxALL, 5 );

    miscProcBoxSizer->Add( miscProcGridSizer, 0, wxEXPAND, 1 );
    miscProcBoxSizer->AddSpacer(1); // Ensure staticText22 is fully visible on Mac

    processorTabSizer->Add( miscProcBoxSizer, 0, wxEXPAND, 1 );

    processorTab->SetSizer( processorTabSizer );
    processorTab->Layout();
    processorTabSizer->Fit( processorTab );

    return processorTab;
}

wxPanel* CDlgAdvPreferencesBase::createNetworkTab(wxNotebook* notebook)
{
    wxSize textCtrlSize = getTextCtrlSize(wxT("9999.99"));

    wxPanel* networkTab = new wxPanel( notebook, ID_TABPAGE_NET, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    networkTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* networkTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* networkGeneralStaticBox = new wxStaticBox( networkTab, -1, _("General options") );
    wxStaticBoxSizer* networkGeneralBoxSizer = new wxStaticBoxSizer( networkGeneralStaticBox, wxVERTICAL );

    wxFlexGridSizer* networkGeneralGridSizer = new wxFlexGridSizer(3, 0, 0 );
    networkGeneralGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    networkGeneralGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    // upload/download rates

    m_chkNetDownloadRate = new wxCheckBox( networkGeneralStaticBox, ID_CHKNETDOWNLOADRATE, _("Limit download rate to"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_chkNetDownloadRate, 0, wxALL, 5 );

    m_txtNetDownloadRate = new wxTextCtrl( networkGeneralStaticBox, ID_TXTNETDOWNLOADRATE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    networkGeneralGridSizer->Add( m_txtNetDownloadRate, 0, wxALL, 1 );

    wxStaticText* staticText33 = new wxStaticText( networkGeneralStaticBox, ID_DEFAULT, _("KBytes/second"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( staticText33, 0, wxALL, 5 );

    m_chkNetUploadRate = new wxCheckBox( networkGeneralStaticBox, ID_CHKNETUPLOADRATE, _("Limit upload rate to"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_chkNetUploadRate, 0, wxALL, 5 );

    m_txtNetUploadRate = new wxTextCtrl( networkGeneralStaticBox, ID_TXTNETUPLOADRATE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    networkGeneralGridSizer->Add( m_txtNetUploadRate, 0, wxALL, 1 );

    wxStaticText* staticText35 = new wxStaticText( networkGeneralStaticBox, ID_DEFAULT, _("KBytes/second"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( staticText35, 0, wxALL, 5 );

    // long-term quota

    m_chk_daily_xfer_limit = new wxCheckBox( networkGeneralStaticBox, ID_CHKDAILYXFERLIMIT, _("Limit network usage to"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_chk_daily_xfer_limit, 0, wxALL, 5 );

    m_txt_daily_xfer_limit_mb = new wxTextCtrl( networkGeneralStaticBox, ID_TXTNETDOWNLOADRATE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    networkGeneralGridSizer->Add( m_txt_daily_xfer_limit_mb, 0, wxALL, 1 );

    wxBoxSizer* networkTransferLimitSizer = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* staticText_daily_xfer2 = new wxStaticText( networkGeneralStaticBox, ID_DEFAULT, _("MBytes every"), wxDefaultPosition, wxDefaultSize, 0 );

    networkTransferLimitSizer->Add( staticText_daily_xfer2, 0, wxALL, 5 );
    
    m_txt_daily_xfer_period_days = new wxTextCtrl( networkGeneralStaticBox, ID_TXTNETUPLOADRATE, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("999.99")), wxTE_RIGHT );
    networkTransferLimitSizer->Add( m_txt_daily_xfer_period_days, 0, wxALL, 1 );

    wxStaticText* staticText_daily_xfer4 = new wxStaticText( networkGeneralStaticBox, ID_DEFAULT, _("days"), wxDefaultPosition, wxDefaultSize, 0 );
    networkTransferLimitSizer->Add( staticText_daily_xfer4, 0, wxALL, 5 );

    networkGeneralGridSizer->Add( networkTransferLimitSizer, 0, wxALL, 0 );

    networkGeneralBoxSizer->Add( networkGeneralGridSizer, 0, wxEXPAND, 1 );

    m_chkNetSkipImageVerification = new wxCheckBox( networkGeneralStaticBox, ID_CHKNETSKIPIMAGEVERIFICATION, _("Skip image file verification"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkNetSkipImageVerification->SetToolTip( _("check this if your Internet provider modifies image files") );

    networkGeneralBoxSizer->Add( m_chkNetSkipImageVerification, 0, wxALL, 5 );

    networkTabSizer->Add( networkGeneralBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBox* connectOptionsStaticBox = new wxStaticBox( networkTab, -1, _("Connect options") );
    wxStaticBoxSizer* connectOptionsSizer = new wxStaticBoxSizer( connectOptionsStaticBox, wxVERTICAL );

    m_chkNetConfirmBeforeConnect = new wxCheckBox( connectOptionsStaticBox, ID_CHKNETCONFIRMBEFORECONNECT, _("Confirm before connecting to internet"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkNetConfirmBeforeConnect->SetToolTip( _("if checked, a confirmation dialog will be displayed before trying to connect to the Internet") );

    connectOptionsSizer->Add( m_chkNetConfirmBeforeConnect, 0, wxALL, 5 );

    m_chkNetDisconnectWhenDone = new wxCheckBox( connectOptionsStaticBox, ID_CHKNETDISCONNECTWHENDONE, _("Disconnect when done"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkNetDisconnectWhenDone->SetToolTip( _("if checked, BOINC hangs up when network usage is done\n(only relevant for dialup-connection)") );

    connectOptionsSizer->Add( m_chkNetDisconnectWhenDone, 0, wxALL, 5 );

    networkTabSizer->Add( connectOptionsSizer, 0, wxEXPAND, 1 );

    networkTab->SetSizer( networkTabSizer );
    networkTab->Layout();
    networkTabSizer->Fit( networkTab );

    return networkTab;
}

wxPanel* CDlgAdvPreferencesBase::createDiskAndMemoryTab(wxNotebook* notebook)
{
    wxSize textCtrlSize = getTextCtrlSize(wxT("9999.99"));
    
    wxPanel* diskMemoryTab = new wxPanel( notebook, ID_TABPAGE_DISK, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    diskMemoryTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* diskAndMemoryBoxSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* diskUsageStaticBox = new wxStaticBox( diskMemoryTab, -1, _("Disk usage") );
    wxStaticBoxSizer* diskUsageBoxSizer = new wxStaticBoxSizer( diskUsageStaticBox, wxVERTICAL );

    wxStaticBox* diskFreeSpaceStaticBox = new wxStaticBox( diskUsageStaticBox, -1, _("BOINC will use the most restrictive of these three settings") );
    wxStaticBoxSizer* diskFreeSpaceBoxSizer = new wxStaticBoxSizer( diskFreeSpaceStaticBox, wxVERTICAL );

    wxFlexGridSizer* diskUsageGridSizer = new wxFlexGridSizer( 3, 3, 0, 0 );
    diskUsageGridSizer->AddGrowableCol( 2 );
    diskUsageGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    diskUsageGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_chkDiskMaxSpace = new wxCheckBox (
        diskFreeSpaceStaticBox, ID_CHKDISKMAXSPACE, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    diskUsageGridSizer->Add( m_chkDiskMaxSpace, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskMaxSpace = new wxTextCtrl( diskFreeSpaceStaticBox, ID_TXTDISKMAXSPACE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtDiskMaxSpace->SetToolTip( _("the maximum disk space used by BOINC (in Gigabytes)") );

    diskUsageGridSizer->Add( m_txtDiskMaxSpace, 0, wxALL, 1 );

    wxStaticText* staticText41 = new wxStaticText( diskFreeSpaceStaticBox, ID_DEFAULT, _("Gigabytes disk space"), wxDefaultPosition, wxDefaultSize, 0 );
    diskUsageGridSizer->Add( staticText41, 0, wxALL, 5 );

    m_chkDiskLeastFree = new wxCheckBox (
        diskFreeSpaceStaticBox, ID_CHKDISKLEASTFREE, _("Leave at least"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    diskUsageGridSizer->Add( m_chkDiskLeastFree, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskLeastFree = new wxTextCtrl( diskFreeSpaceStaticBox, ID_TXTDISKLEASTFREE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtDiskLeastFree->SetToolTip( _("BOINC leaves at least this amount of disk space free (in Gigabytes)") );

    diskUsageGridSizer->Add( m_txtDiskLeastFree, 0, wxALL, 1 );

    wxStaticText* staticText43 = new wxStaticText( diskFreeSpaceStaticBox, ID_DEFAULT, _("Gigabytes disk space free"), wxDefaultPosition, wxDefaultSize, 0 );
    diskUsageGridSizer->Add( staticText43, 0, wxALL, 5 );

    m_chkDiskMaxOfTotal = new wxCheckBox (
        diskFreeSpaceStaticBox, ID_CHKDISKMAXOFTOTAL, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    diskUsageGridSizer->Add( m_chkDiskMaxOfTotal, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskMaxOfTotal = new wxTextCtrl( diskFreeSpaceStaticBox, ID_TXTDISKMAXOFTOTAL, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtDiskMaxOfTotal->SetToolTip( _("BOINC uses at most this percentage of total disk space") );

    diskUsageGridSizer->Add( m_txtDiskMaxOfTotal, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText45 = new wxStaticText( diskFreeSpaceStaticBox, ID_DEFAULT, _("% of total disk space"), wxDefaultPosition, wxDefaultSize, 0 );
    diskUsageGridSizer->Add( staticText45, 0, wxALL, 5 );

    diskFreeSpaceBoxSizer->Add(diskUsageGridSizer, 0, wxEXPAND, 1 );
    diskUsageBoxSizer->Add(diskFreeSpaceBoxSizer, 0, wxEXPAND, 1 );

    wxFlexGridSizer* swapCheckpointGridSizer = new wxFlexGridSizer( 2, 3, 0, 0 );
    swapCheckpointGridSizer->AddGrowableCol( 2 );
    swapCheckpointGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    swapCheckpointGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText* staticText48 = new wxStaticText( diskUsageStaticBox, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    swapCheckpointGridSizer->Add( staticText48, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskMaxSwap = new wxTextCtrl( diskUsageStaticBox, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    swapCheckpointGridSizer->Add( m_txtDiskMaxSwap, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText49 = new wxStaticText( diskUsageStaticBox, ID_DEFAULT, _("% of page file (swap space)"), wxDefaultPosition, wxDefaultSize, 0 );
    swapCheckpointGridSizer->Add( staticText49, 0, wxALL, 5 );

    diskUsageBoxSizer->Add( swapCheckpointGridSizer, 0, wxEXPAND, 1 );

    diskAndMemoryBoxSizer->Add( diskUsageBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBox* memoryUsageStaticBox = new wxStaticBox( diskMemoryTab, -1, _("Memory usage") );
    wxStaticBoxSizer* memoryUsageBoxSizer = new wxStaticBoxSizer( memoryUsageStaticBox, wxVERTICAL );

    wxFlexGridSizer* memoryUsageGridSizer = new wxFlexGridSizer( 3, 3, 0, 0 );
    memoryUsageGridSizer->AddGrowableCol( 2 );
    memoryUsageGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    memoryUsageGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText* staticText50 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    memoryUsageGridSizer->Add( staticText50, 0, wxALL|wxEXPAND, 5 );

    textCtrlSize = getTextCtrlSize(wxT("100.00"));
    m_txtMemoryMaxInUse = new wxTextCtrl( memoryUsageStaticBox, ID_TXTMEMORYMAXINUSE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    memoryUsageGridSizer->Add( m_txtMemoryMaxInUse, 0, wxALL, 1 );

    /*xgettext:no-c-format*/ 
    wxStaticText* staticText51 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("% when computer is in use"), wxDefaultPosition, wxDefaultSize, 0 );
    memoryUsageGridSizer->Add( staticText51, 0, wxALL, 5 );

    wxStaticText* staticText52 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    memoryUsageGridSizer->Add( staticText52, 0, wxALL|wxEXPAND, 5 );

    m_txtMemoryMaxOnIdle = new wxTextCtrl( memoryUsageStaticBox, ID_TXTMEMORYMAXONIDLE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    memoryUsageGridSizer->Add( m_txtMemoryMaxOnIdle, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText53 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("% when computer is idle"), wxDefaultPosition, wxDefaultSize, 0 );
    memoryUsageGridSizer->Add( staticText53, 0, wxALL, 5 );

    memoryUsageBoxSizer->Add( memoryUsageGridSizer, 0, wxEXPAND, 1 );

    m_chkMemoryWhileSuspended = new wxCheckBox( memoryUsageStaticBox, ID_CHKMEMORYWHILESUSPENDED, _("Leave applications in memory while suspended"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkMemoryWhileSuspended->SetToolTip( _("if checked, suspended work units are left in memory") );

    memoryUsageBoxSizer->Add( m_chkMemoryWhileSuspended, 0, wxALL, 5 );

    diskAndMemoryBoxSizer->Add( memoryUsageBoxSizer, 0, wxALL|wxEXPAND, 1 );

    diskMemoryTab->SetSizer( diskAndMemoryBoxSizer );
    diskMemoryTab->Layout();
    diskAndMemoryBoxSizer->Fit( diskMemoryTab );

    return diskMemoryTab;
}


wxPanel* CDlgAdvPreferencesBase::createDailySchedulesTab(wxNotebook* notebook)
{
    wxPanel* dailySchedulesTab = new wxPanel( notebook, ID_TABPAGE_DISK, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    dailySchedulesTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* dailySchedulesTabSizer = new wxBoxSizer( wxVERTICAL );

    // Computing schedule
    //
    wxStaticBox* computingTimesStaticBox = new wxStaticBox(dailySchedulesTab, -1, _("Computing allowed") );
    wxStaticBoxSizer* computingTimesStaticBoxBoxSizer = new wxStaticBoxSizer(computingTimesStaticBox, wxVERTICAL);
    
    wxBoxSizer* cpuTimesSizer = new wxBoxSizer( wxHORIZONTAL );

    m_chkProcEveryDay = new wxCheckBox(
        computingTimesStaticBox, ID_CHKPROCEVERYDAY,
        _("Every day between the hours of"), wxDefaultPosition, wxDefaultSize, 0 );
    cpuTimesSizer->Add( m_chkProcEveryDay, 0, wxALL, 5 );

    m_txtProcEveryDayStart = new wxTextCtrl( computingTimesStaticBox, ID_TXTPROCEVERYDAYSTART, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("23:59")), wxTE_RIGHT );
    m_txtProcEveryDayStart->SetToolTip( _("start work at this time") );

    cpuTimesSizer->Add( m_txtProcEveryDayStart, 0, wxALL, 1 );

    wxStaticText* staticText25 = new wxStaticText( computingTimesStaticBox, ID_DEFAULT, _("and"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    cpuTimesSizer->Add( staticText25, 0, wxALL|wxEXPAND, 5 );

    m_txtProcEveryDayStop = new wxTextCtrl( computingTimesStaticBox, ID_TXTPROCEVERYDAYSTOP, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("23:59")), wxTE_RIGHT );
    m_txtProcEveryDayStop->SetToolTip( _("stop work at this time") );

    cpuTimesSizer->Add( m_txtProcEveryDayStop, 0, wxALL, 1 );

    computingTimesStaticBoxBoxSizer->Add( cpuTimesSizer, 0, wxEXPAND, 1 );

    wxStaticText* staticText36 = new wxStaticText( computingTimesStaticBox, ID_DEFAULT, _("Day-of-week override:"), wxDefaultPosition, wxDefaultSize, 0 );
    computingTimesStaticBoxBoxSizer->Add( staticText36, 0, wxALL, 5 );

    m_panelProcSpecialTimes = new wxPanel( computingTimesStaticBox, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_panelProcSpecialTimes->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
    m_panelProcSpecialTimes->SetToolTip( _("check box to specify hours for this day of week") );

    wxFlexGridSizer* procDaysSizer = new wxFlexGridSizer( 4, 4, 0, 0 );
    procDaysSizer->SetFlexibleDirection( wxHORIZONTAL );
    procDaysSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_chkProcMonday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCMONDAY, _("Monday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcMonday, 0, wxALL, 5 );

    m_txtProcMonday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCMONDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    procDaysSizer->Add( m_txtProcMonday, 0, wxALL, 1 );

    m_chkProcTuesday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcTuesday, 0, wxALL, 5 );

    m_txtProcTuesday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCTUESDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    procDaysSizer->Add( m_txtProcTuesday, 0, wxALL, 1 );

    m_chkProcWednesday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcWednesday, 0, wxALL, 5 );

    m_txtProcWednesday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCWEDNESDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    procDaysSizer->Add( m_txtProcWednesday, 0, wxALL, 1 );

    m_chkProcThursday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcThursday, 0, wxALL, 5 );

    m_txtProcThursday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCTHURSDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    procDaysSizer->Add( m_txtProcThursday, 0, wxALL, 1 );

    m_chkProcFriday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCFRIDAY, _("Friday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcFriday, 0, wxALL, 5 );

    m_txtProcFriday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCFRIDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    procDaysSizer->Add( m_txtProcFriday, 0, wxALL, 1 );

    m_chkProcSaturday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcSaturday, 0, wxALL, 5 );

    m_txtProcSaturday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCSATURDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    procDaysSizer->Add( m_txtProcSaturday, 0, wxALL, 1 );

    m_chkProcSunday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcSunday, 0, wxALL, 5 );

    m_txtProcSunday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCSUNDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    procDaysSizer->Add( m_txtProcSunday, 0, wxALL, 1 );

    m_panelProcSpecialTimes->SetSizer( procDaysSizer );
    m_panelProcSpecialTimes->Layout();
    procDaysSizer->Fit( m_panelProcSpecialTimes );
    
    computingTimesStaticBoxBoxSizer->Add( m_panelProcSpecialTimes, 1, wxEXPAND | wxALL, 1 );
    dailySchedulesTabSizer->Add( computingTimesStaticBoxBoxSizer, 1, wxEXPAND | wxALL, 1 );
    
    // Network schedule
    //
    wxStaticBox* networkTimesStaticBox = new wxStaticBox( dailySchedulesTab, -1, _("Network usage allowed") );
    wxStaticBoxSizer* networkTimesBoxSizer = new wxStaticBoxSizer( networkTimesStaticBox, wxVERTICAL );

    wxBoxSizer* networkTimesSizer = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* staticText38 = new wxStaticText( networkTimesStaticBox, ID_DEFAULT, _("Every day between hours of"), wxDefaultPosition, wxDefaultSize, 0 );
    networkTimesSizer->Add( staticText38, 0, wxALL, 5 );

    m_txtNetEveryDayStart = new wxTextCtrl( networkTimesStaticBox, ID_TXTNETEVERYDAYSTART, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("23:59")), 0 );
    m_txtNetEveryDayStart->SetToolTip( _("network usage start hour") );

    networkTimesSizer->Add( m_txtNetEveryDayStart, 0, wxALL, 1 );

    wxStaticText* staticText37 = new wxStaticText( networkTimesStaticBox, ID_DEFAULT, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    networkTimesSizer->Add( staticText37, 0, wxALL, 5 );

    m_txtNetEveryDayStop = new wxTextCtrl( networkTimesStaticBox, ID_TXTNETEVERYDAYSTOP, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("23:59")), 0 );
    m_txtNetEveryDayStop->SetToolTip( _("network usage stop hour") );

    networkTimesSizer->Add( m_txtNetEveryDayStop, 0, wxALL, 1 );

    wxStaticText* staticText54 = new wxStaticText( networkTimesStaticBox, ID_DEFAULT, _("(no restriction if equal)"), wxDefaultPosition, wxDefaultSize, 0 );
    networkTimesSizer->Add( staticText54, 0, wxALL, 5 );

    networkTimesBoxSizer->Add( networkTimesSizer, 0, wxEXPAND, 1 );

    wxStaticText* staticText39 = new wxStaticText( networkTimesStaticBox, ID_DEFAULT, _("Day-of-week override:"), wxDefaultPosition, wxDefaultSize, 0 );
    networkTimesBoxSizer->Add( staticText39, 0, wxALL, 5 );

    m_panelNetSpecialTimes = new wxPanel( networkTimesStaticBox, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_panelNetSpecialTimes->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
    m_panelNetSpecialTimes->SetToolTip( _("check box to specify hours for this day of week") );

    wxFlexGridSizer* netDaysGridSizer = new wxFlexGridSizer( 4, 4, 0, 0 );
    netDaysGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    netDaysGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_chkNetMonday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETMONDAY, _("Monday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetMonday, 0, wxALL, 5 );

    m_txtNetMonday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETMONDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    netDaysGridSizer->Add( m_txtNetMonday, 0, wxALL, 1 );

    m_chkNetTuesday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetTuesday, 0, wxALL, 5 );

    m_txtNetTuesday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETTUESDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    netDaysGridSizer->Add( m_txtNetTuesday, 0, wxALL, 1 );

    m_chkNetWednesday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetWednesday, 0, wxALL, 5 );

    m_txtNetWednesday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETWEDNESDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    netDaysGridSizer->Add( m_txtNetWednesday, 0, wxALL, 1 );

    m_chkNetThursday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetThursday, 0, wxALL, 5 );

    m_txtNetThursday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETTHURSDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    netDaysGridSizer->Add( m_txtNetThursday, 0, wxALL, 1 );

    m_chkNetFriday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETFRIDAY, _("Friday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetFriday, 0, wxALL, 5 );

    m_txtNetFriday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETFRIDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    netDaysGridSizer->Add( m_txtNetFriday, 0, wxALL, 1 );

    m_chkNetSaturday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetSaturday, 0, wxALL, 5 );

    m_txtNetSaturday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETSATURDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    netDaysGridSizer->Add( m_txtNetSaturday, 0, wxALL, 1 );

    m_chkNetSunday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetSunday, 0, wxALL, 5 );

    m_txtNetSunday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETSUNDAY, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("00:00-23:59")), 0 );
    netDaysGridSizer->Add( m_txtNetSunday, 0, wxALL, 1 );

    m_panelNetSpecialTimes->SetSizer( netDaysGridSizer );
    m_panelNetSpecialTimes->Layout();
    netDaysGridSizer->Fit( m_panelNetSpecialTimes );
    
    networkTimesBoxSizer->Add( m_panelNetSpecialTimes, 0, wxEXPAND | wxALL, 1 );
    dailySchedulesTabSizer->Add( networkTimesBoxSizer, 1, wxEXPAND | wxALL, 1 );
   
    dailySchedulesTab->SetSizer( dailySchedulesTabSizer );
    dailySchedulesTab->Layout();
    dailySchedulesTabSizer->Fit( dailySchedulesTab );

    return dailySchedulesTab;
}


wxSize CDlgAdvPreferencesBase::getTextCtrlSize(wxString maxText) {
    int w, h, margin;
    wxSize sz;
    wxFont f = GetParent()->GetFont();
    GetTextExtent(maxText, &w, &h, NULL, NULL, &f);
    margin = w/3;
    if (margin < 9) margin = 9;
    sz.x = w + margin;
    sz.y = wxDefaultCoord;
    return sz;
}

bool CDlgAdvPreferencesBase::doesLocalPrefsFileExist() {
    std::string s;
    int retval;
    bool local_prefs_found = false;
    MIOFILE mf;
    bool found_venue;
    GLOBAL_PREFS web_prefs;
    GLOBAL_PREFS_MASK mask;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    retval = pDoc->rpc.get_global_prefs_override(s);
    local_prefs_found = (retval == BOINC_SUCCESS);
    
    s.clear();
    web_prefs.init();
    
    retval = pDoc->rpc.get_global_prefs_file(s);
    mf.init_buf_read(s.c_str());
    XML_PARSER xp(&mf);
    web_prefs.parse(xp, "", found_venue, mask);
    web_prefs_url = new wxString(web_prefs.source_project);
    
    return local_prefs_found;
}

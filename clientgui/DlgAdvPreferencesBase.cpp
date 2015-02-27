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
    m_btnClear->SetToolTip( _("Clear all local preferences and close the dialog.") );
#endif

    m_btnClear = new wxButton( topControlsStaticBox, ID_BTN_CLEAR, _("Use web prefs"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnClear->SetToolTip( _("Restore web-based preferences and close the dialog.") );
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

    // Note: we must set the third AddPage argument ("select") to
    // true for each page or ToolTips won't initialize properly.
    m_panelProcessor = createProcessorTab(m_Notebook);
    m_Notebook->AddPage( m_panelProcessor, _("Computing"), true );

    m_panelNetwork = createNetworkTab(m_Notebook);
    m_Notebook->AddPage( m_panelNetwork, _("Network"), true );

    m_panelDiskAndMemory = createDiskAndMemoryTab(m_Notebook);
    m_Notebook->AddPage( m_panelDiskAndMemory, _("Disk and Memory"), true );

    m_panelDailySchedules = createDailySchedulesTab(m_Notebook);
    m_Notebook->AddPage( m_panelDailySchedules, _("Daily Schedules"), true );

    notebookSizer->Add( m_Notebook, 1, wxEXPAND | wxALL, 1 );

    m_panelControls->SetSizer( notebookSizer );
    m_panelControls->Layout();
    notebookSizer->Fit( m_panelControls );

    dialogSizer->Add( m_panelControls, 1, wxALL|wxEXPAND, 1 );

    m_panelButtons = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    m_btnOK = new wxButton( m_panelButtons, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnOK->SetToolTip( _("Save all values and close the dialog.") );
    m_btnOK->SetDefault();
    
    buttonSizer->Add( m_btnOK, 0, wxALL, 5 );

    m_btnCancel = new wxButton( m_panelButtons, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnCancel->SetToolTip( _("Close the dialog without saving.") );

    buttonSizer->Add( m_btnCancel, 0, wxALL, 5 );

    m_btnHelp = new wxButton( m_panelButtons, ID_HELPBOINC, _("Help"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnHelp->SetToolTip( _("Shows the preferences web page.") );

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
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);

    wxSize textCtrlSize = getTextCtrlSize(wxT("999.99"));
    
    wxPanel* processorTab = new wxPanel( notebook, ID_TABPAGE_PROC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    processorTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* processorTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* usageLimitsStaticBox = new wxStaticBox(processorTab, -1, _("Usage limits") );
    wxStaticBoxSizer* usageLimitsBoxSizer = new wxStaticBoxSizer(usageLimitsStaticBox, wxVERTICAL);
    makeStaticBoxLabelItalic(usageLimitsStaticBox);
    
    wxFlexGridSizer* usageLimitsGridSizer = new wxFlexGridSizer( 2, 3, 0, 0 );
    usageLimitsGridSizer->AddGrowableCol( 2 );
    usageLimitsGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    usageLimitsGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    /*xgettext:no-c-format*/
    wxString MaxCPUPctTT(_("Keep some CPUs free for other applications. Example: 75% means use 6 cores on an 8-core CPU."));
    wxStaticText* staticText20 = new wxStaticText(
        usageLimitsStaticBox, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText20->SetToolTip(MaxCPUPctTT);
    usageLimitsGridSizer->Add( staticText20, 0, wxALL|wxEXPAND, 5 );
    
    m_txtProcUseProcessors = new wxTextCtrl( usageLimitsStaticBox, ID_TXTPROCUSEPROCESSORS, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtProcUseProcessors->SetToolTip(MaxCPUPctTT);
    usageLimitsGridSizer->Add( m_txtProcUseProcessors, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText21 = new wxStaticText( usageLimitsStaticBox, ID_DEFAULT, _("% of the CPUs"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText21->SetToolTip(MaxCPUPctTT);
    usageLimitsGridSizer->Add( staticText21, 0, wxALL, 5 );

    /*xgettext:no-c-format*/
    wxString MaxCPUTimeTT(_("Suspend/resume computing every few seconds to reduce CPU temperature and energy usage. Example: 75% means compute for 3 seconds, wait for 1 second, and repeat."));
    wxStaticText* staticText22 = new wxStaticText(
        usageLimitsStaticBox, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText22->SetToolTip(MaxCPUTimeTT);
    usageLimitsGridSizer->Add( staticText22, 0, wxALL|wxEXPAND, 5 );

    usageLimitsBoxSizer->Add( usageLimitsGridSizer, 0, wxEXPAND, 1 );

    m_txtProcUseCPUTime = new wxTextCtrl( usageLimitsStaticBox, ID_TXTPOCUSECPUTIME, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtProcUseCPUTime->SetToolTip(MaxCPUTimeTT);
    usageLimitsGridSizer->Add( m_txtProcUseCPUTime, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText23 = new wxStaticText( usageLimitsStaticBox, ID_DEFAULT, _("% of CPU time"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText23->SetToolTip(MaxCPUTimeTT);
    usageLimitsGridSizer->Add( staticText23, 0, wxALL, 5 );

    processorTabSizer->Add( usageLimitsBoxSizer, 0, wxEXPAND, 1 );
    
    wxStaticBox* suspendComputingStaticBox = new wxStaticBox(processorTab, -1, _("When to suspend") );
    wxStaticBoxSizer* suspendComputingBoxSizer = new wxStaticBoxSizer(suspendComputingStaticBox, wxVERTICAL);
    makeStaticBoxLabelItalic(suspendComputingStaticBox);
    
    m_chkProcOnBatteries = new wxCheckBox(
        suspendComputingStaticBox, ID_CHKPROCONBATTERIES,
        _("Suspend when computer is on battery"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_chkProcOnBatteries->SetToolTip(
        _("Check this to suspend computing on portables when running on battery power.")
    );
    suspendComputingBoxSizer->Add( m_chkProcOnBatteries, 0, wxALL, 5 );

    m_chkProcInUse = new wxCheckBox(
        suspendComputingStaticBox, ID_CHKPROCINUSE,
        _("Suspend when computer is in use"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_chkProcInUse->SetToolTip(
        _("Check this to suspend computing and file transfers when you're using the computer.")
    );
    suspendComputingBoxSizer->Add( m_chkProcInUse, 0, wxALL, 5 );

    m_chkGPUProcInUse = new wxCheckBox(
        suspendComputingStaticBox, ID_CHKGPUPROCINUSE,
        _("Suspend GPU computing when computer is in use"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_chkGPUProcInUse->SetToolTip(
        _("Check this to suspend GPU computing when you're using the computer.")
    );
    suspendComputingBoxSizer->Add( m_chkGPUProcInUse, 0, wxALL, 5 );

    // min idle time
    wxFlexGridSizer* procIdleSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
    procIdleSizer->AddGrowableCol( 3 );
    procIdleSizer->SetFlexibleDirection( wxHORIZONTAL );
    procIdleSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    wxString ProcIdleForTT(_("This determines when the computer is considered 'in use'."));

    wxStaticText* staticText24 = new wxStaticText(
            suspendComputingStaticBox, ID_DEFAULT,
            _("'In use' means mouse/keyboard input in last"),
            wxDefaultPosition, wxDefaultSize, 0
        );
    staticText24->SetToolTip(ProcIdleForTT);
    procIdleSizer->Add(staticText24, 0, wxALL, 5 );

    m_txtProcIdleFor = new wxTextCtrl(
        suspendComputingStaticBox, ID_TXTPROCIDLEFOR, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("999.99")), wxTE_RIGHT
    );
    m_txtProcIdleFor->SetToolTip(ProcIdleForTT);
    procIdleSizer->Add( m_txtProcIdleFor, 0, wxALL, 1 );
    wxStaticText* staticText25 = new wxStaticText(suspendComputingStaticBox, ID_DEFAULT, _("minutes"),
            wxDefaultPosition, wxDefaultSize, 0 );
    procIdleSizer->Add(staticText25, 0, wxALL, 5 );
    staticText25->SetToolTip(ProcIdleForTT);
    procIdleSizer->Add(
        new wxStaticText( suspendComputingStaticBox, ID_DEFAULT, wxT(""), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );

    suspendComputingBoxSizer->Add( procIdleSizer, 0, wxEXPAND, 5);

    // max CPU load
    wxFlexGridSizer* maxLoadSizer = new wxFlexGridSizer( 1, 3, 0, 0 );
    maxLoadSizer->SetFlexibleDirection( wxHORIZONTAL );
    maxLoadSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxString MaxLoadCheckBoxText = wxEmptyString;
    MaxLoadCheckBoxText.Printf(_("Suspend when non-%s CPU usage is above"), pSkinAdvanced->GetApplicationShortName().c_str());

    wxString MaxLoadTT(_("Suspend computing when your computer is busy running other programs."));
    m_chkMaxLoad = new wxCheckBox(
        suspendComputingStaticBox, ID_CHKMAXLOAD, MaxLoadCheckBoxText, wxDefaultPosition, wxDefaultSize, 0);
    m_chkMaxLoad->SetToolTip(MaxLoadTT);
    maxLoadSizer->Add( m_chkMaxLoad, 0, wxALL, 5 );
    m_txtMaxLoad = new wxTextCtrl(
        suspendComputingStaticBox, ID_TXTMAXLOAD, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("100.00")), wxTE_RIGHT
    );
    m_txtMaxLoad->SetToolTip(MaxLoadTT);
    maxLoadSizer->Add( m_txtMaxLoad, 0, wxALL, 1 );
    wxStaticText* staticText26 = new wxStaticText( suspendComputingStaticBox, ID_DEFAULT, _("percent"),
            wxDefaultPosition, wxDefaultSize, 0 );
    staticText26->SetToolTip(MaxLoadTT);
    maxLoadSizer->Add(staticText26, 0, wxALL, 5 );

    suspendComputingBoxSizer->Add( maxLoadSizer, 0, wxEXPAND, 5);

     suspendComputingBoxSizer->Add(
        new wxStaticText( suspendComputingStaticBox, ID_DEFAULT, wxT("To suspend by time of day, see the \"Daily schedules\" section."), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );

    processorTabSizer->Add( suspendComputingBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBox* miscProcStaticBox = new wxStaticBox( processorTab, -1, _("Other") );
    wxStaticBoxSizer* miscProcBoxSizer = new wxStaticBoxSizer( miscProcStaticBox, wxVERTICAL );
    makeStaticBoxLabelItalic(miscProcStaticBox);

    wxFlexGridSizer* miscProcGridSizer = new wxFlexGridSizer( 4, 3, 0, 0 );
    miscProcGridSizer->AddGrowableCol( 2 );
    miscProcGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    miscProcGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    // buffer sizes
    wxString NetConnectIntervalTT(_("Store at least enough tasks to keep the computer busy for this long."));
    wxStaticText* staticText30 = new wxStaticText(
        miscProcStaticBox, ID_DEFAULT,
        _("Store at least"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT
    );
    staticText30->SetToolTip(NetConnectIntervalTT);
    miscProcGridSizer->Add( staticText30, 0, wxALL|wxEXPAND, 5 );

    m_txtNetConnectInterval = new wxTextCtrl(
        miscProcStaticBox, ID_TXTNETCONNECTINTERVAL, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );
    m_txtNetConnectInterval->SetToolTip(NetConnectIntervalTT);
    miscProcGridSizer->Add( m_txtNetConnectInterval, 0, wxALL, 1 );

    wxStaticText* staticText31 = new wxStaticText(
        miscProcStaticBox, ID_DEFAULT, _("days of work"), wxDefaultPosition, wxDefaultSize, 0
    );
    staticText31->SetToolTip(NetConnectIntervalTT);
    miscProcGridSizer->Add( staticText31, 0, wxALL, 5 );

    wxString NetAdditionalDaysTT(_("Store additional tasks above the minimum level.  Determines how much work is requested when contacting a project."));
    wxStaticText* staticText331 = new wxStaticText(
        miscProcStaticBox, ID_DEFAULT,
        _("Store up to an additional"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT
    );
    staticText331->SetToolTip(NetAdditionalDaysTT);
    miscProcGridSizer->Add( staticText331, 0, wxALL|wxEXPAND, 5 );

    m_txtNetAdditionalDays = new wxTextCtrl(
        miscProcStaticBox, ID_TXTNETADDITIONALDAYS, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );
    m_txtNetAdditionalDays->SetToolTip(NetAdditionalDaysTT);
    miscProcGridSizer->Add( m_txtNetAdditionalDays, 0, wxALL, 1 );

    wxBoxSizer* workBufAdditonalDaysSizer = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* staticText341 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("days of work"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText341->SetToolTip(NetAdditionalDaysTT);
    workBufAdditonalDaysSizer->Add( staticText341, 0, 0, 0 );

    miscProcGridSizer->Add( workBufAdditonalDaysSizer, 0, wxALL, 5 );

    wxString ProcSwitchEveryTT = wxEmptyString;
    ProcSwitchEveryTT.Printf(_("If you run several projects, %s may switch between them this often."), pSkinAdvanced->GetApplicationShortName().c_str());
    
    wxStaticText* staticText18 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("Switch between tasks every"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    staticText18->SetToolTip(ProcSwitchEveryTT);
    miscProcGridSizer->Add( staticText18, 0, wxALL|wxEXPAND, 5 );
    
    m_txtProcSwitchEvery = new wxTextCtrl( miscProcStaticBox, ID_TXTPROCSWITCHEVERY, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtProcSwitchEvery->SetToolTip(ProcSwitchEveryTT);
    miscProcGridSizer->Add( m_txtProcSwitchEvery, 0, wxALL, 1 );

    wxStaticText* staticText19 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText19->SetToolTip(ProcSwitchEveryTT);
    miscProcGridSizer->Add( staticText19, 0, wxALL, 5 );

    wxString DiskWriteToDiskTT(_("This controls how often tasks save their state to disk, so that they can be restarted later."));
    wxStaticText* staticText46 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("Request tasks to checkpoint at most every"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    staticText46->SetToolTip(DiskWriteToDiskTT);
    miscProcGridSizer->Add( staticText46, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskWriteToDisk = new wxTextCtrl( miscProcStaticBox, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtDiskWriteToDisk->SetToolTip(DiskWriteToDiskTT);
    miscProcGridSizer->Add( m_txtDiskWriteToDisk, 0, wxALL, 1 );

    wxStaticText* staticText47 = new wxStaticText( miscProcStaticBox, ID_DEFAULT, _("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText47->SetToolTip(DiskWriteToDiskTT);
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
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);

    wxSize textCtrlSize = getTextCtrlSize(wxT("9999.99"));

    wxPanel* networkTab = new wxPanel( notebook, ID_TABPAGE_NET, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    networkTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* networkTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* networkUsageLimitsStaticBox = new wxStaticBox( networkTab, -1, _("Usage limits") );
    wxStaticBoxSizer* networkUsageLimitsBoxSizer = new wxStaticBoxSizer( networkUsageLimitsStaticBox, wxVERTICAL );
    makeStaticBoxLabelItalic(networkUsageLimitsStaticBox);

    wxFlexGridSizer* networkUsageLimitsGridSizer = new wxFlexGridSizer(3, 0, 0 );
    networkUsageLimitsGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    networkUsageLimitsGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    // upload/download rates

    wxString NetDownloadRateTT(_("Limit the download rate of file transfers."));
    m_chkNetDownloadRate = new wxCheckBox( networkUsageLimitsStaticBox, ID_CHKNETDOWNLOADRATE, _("Limit download rate to"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkNetDownloadRate-> SetToolTip(NetDownloadRateTT);
    networkUsageLimitsGridSizer->Add( m_chkNetDownloadRate, 0, wxALL, 5 );
    
    m_txtNetDownloadRate = new wxTextCtrl( networkUsageLimitsStaticBox, ID_TXTNETDOWNLOADRATE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtNetDownloadRate-> SetToolTip(NetDownloadRateTT);
    networkUsageLimitsGridSizer->Add( m_txtNetDownloadRate, 0, wxALL, 1 );

    wxStaticText* staticText33 = new wxStaticText( networkUsageLimitsStaticBox, ID_DEFAULT, _("Kbytes/second"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText33-> SetToolTip(NetDownloadRateTT);
    networkUsageLimitsGridSizer->Add( staticText33, 0, wxALL, 5 );

    wxString NetUploadRateTT(_("Limit the upload rate of file transfers."));
    m_chkNetUploadRate = new wxCheckBox( networkUsageLimitsStaticBox, ID_CHKNETUPLOADRATE, _("Limit upload rate to"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkNetUploadRate-> SetToolTip(NetUploadRateTT);
    networkUsageLimitsGridSizer->Add( m_chkNetUploadRate, 0, wxALL, 5 );

    m_txtNetUploadRate = new wxTextCtrl( networkUsageLimitsStaticBox, ID_TXTNETUPLOADRATE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtNetUploadRate-> SetToolTip(NetUploadRateTT);
    networkUsageLimitsGridSizer->Add( m_txtNetUploadRate, 0, wxALL, 1 );

    wxStaticText* staticText35 = new wxStaticText( networkUsageLimitsStaticBox, ID_DEFAULT, _("Kbytes/second"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText35-> SetToolTip(NetUploadRateTT);
    networkUsageLimitsGridSizer->Add( staticText35, 0, wxALL, 5 );

    // long-term quota

    wxString daily_xfer_limitTT = wxEmptyString;
    daily_xfer_limitTT.Printf(_("Example: %s should transfer at most 2000 MB of data every 30 days."), pSkinAdvanced->GetApplicationShortName().c_str());
    
    m_chk_daily_xfer_limit = new wxCheckBox( networkUsageLimitsStaticBox, ID_CHKDAILYXFERLIMIT, _("Limit usage to"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chk_daily_xfer_limit-> SetToolTip(daily_xfer_limitTT);
    networkUsageLimitsGridSizer->Add( m_chk_daily_xfer_limit, 0, wxALL, 5 );

    m_txt_daily_xfer_limit_mb = new wxTextCtrl( networkUsageLimitsStaticBox, ID_TXTNETDOWNLOADRATE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txt_daily_xfer_limit_mb-> SetToolTip(daily_xfer_limitTT);
    networkUsageLimitsGridSizer->Add( m_txt_daily_xfer_limit_mb, 0, wxALL, 1 );

    wxBoxSizer* networkTransferLimitSizer = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* staticText_daily_xfer2 = new wxStaticText( networkUsageLimitsStaticBox, ID_DEFAULT, _("Mbytes every"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText_daily_xfer2-> SetToolTip(daily_xfer_limitTT);
    networkTransferLimitSizer->Add( staticText_daily_xfer2, 0, wxALL, 5 );
    
    m_txt_daily_xfer_period_days = new wxTextCtrl( networkUsageLimitsStaticBox, ID_TXTNETUPLOADRATE, wxT(""), wxDefaultPosition, getTextCtrlSize(wxT("999.99")), wxTE_RIGHT );
    m_txt_daily_xfer_period_days-> SetToolTip(daily_xfer_limitTT);
    networkTransferLimitSizer->Add( m_txt_daily_xfer_period_days, 0, wxALL, 1 );

    wxStaticText* staticText_daily_xfer4 = new wxStaticText( networkUsageLimitsStaticBox, ID_DEFAULT, _("days"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText_daily_xfer4-> SetToolTip(daily_xfer_limitTT);
    networkTransferLimitSizer->Add( staticText_daily_xfer4, 0, wxALL, 5 );

    networkUsageLimitsGridSizer->Add( networkTransferLimitSizer, 0, wxALL, 0 );

    networkUsageLimitsBoxSizer->Add( networkUsageLimitsGridSizer, 0, wxEXPAND, 1 );

     networkUsageLimitsBoxSizer->Add(
        new wxStaticText( networkUsageLimitsStaticBox, ID_DEFAULT, wxT("See also \"Suspend when computer is in use\" in the \"Computing\" section."), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );
     networkUsageLimitsBoxSizer->Add(
        new wxStaticText( networkUsageLimitsStaticBox, ID_DEFAULT, wxT("To suspend by time of day, see the \"Daily schedules\" section."), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );

    networkTabSizer->Add( networkUsageLimitsBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBox* connectOptionsStaticBox = new wxStaticBox( networkTab, -1, _("Other") );
    wxStaticBoxSizer* connectOptionsSizer = new wxStaticBoxSizer( connectOptionsStaticBox, wxVERTICAL );
    makeStaticBoxLabelItalic(connectOptionsStaticBox);

    wxString NetSkipImageVerificationTT = wxEmptyString;
    NetSkipImageVerificationTT.Printf(_("Check this only if your Internet provider modifies image files. Skipping verification reduces the security of %s."), pSkinAdvanced->GetApplicationShortName().c_str());
    
    m_chkNetSkipImageVerification = new wxCheckBox( connectOptionsStaticBox, ID_CHKNETSKIPIMAGEVERIFICATION, _("Skip data verification for image files"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkNetSkipImageVerification->SetToolTip(NetSkipImageVerificationTT);
    connectOptionsSizer->Add( m_chkNetSkipImageVerification, 0, wxALL, 5 );

    m_chkNetConfirmBeforeConnect = new wxCheckBox( connectOptionsStaticBox, ID_CHKNETCONFIRMBEFORECONNECT, _("Confirm before connecting to Internet"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkNetConfirmBeforeConnect->SetToolTip( _("Useful only if you have a modem, ISDN or VPN connection.") );
    connectOptionsSizer->Add( m_chkNetConfirmBeforeConnect, 0, wxALL, 5 );

    m_chkNetDisconnectWhenDone = new wxCheckBox( connectOptionsStaticBox, ID_CHKNETDISCONNECTWHENDONE, _("Disconnect when done"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkNetDisconnectWhenDone->SetToolTip( _("Useful only if you have a modem, ISDN or VPN connection.") );
    connectOptionsSizer->Add( m_chkNetDisconnectWhenDone, 0, wxALL, 5 );

    networkTabSizer->Add( connectOptionsSizer, 0, wxEXPAND, 1 );

    networkTab->SetSizer( networkTabSizer );
    networkTab->Layout();
    networkTabSizer->Fit( networkTab );

    return networkTab;
}

wxPanel* CDlgAdvPreferencesBase::createDiskAndMemoryTab(wxNotebook* notebook)
{
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);

    wxSize textCtrlSize = getTextCtrlSize(wxT("9999.99"));
    
    wxPanel* diskMemoryTab = new wxPanel( notebook, ID_TABPAGE_DISK, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    diskMemoryTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* diskAndMemoryBoxSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* diskUsageStaticBox = new wxStaticBox( diskMemoryTab, -1, _("Disk") );
    wxStaticBoxSizer* diskUsageBoxSizer = new wxStaticBoxSizer( diskUsageStaticBox, wxVERTICAL );
    makeStaticBoxLabelItalic(diskUsageStaticBox);

    wxString MostRestrictiveText = wxEmptyString;
    MostRestrictiveText.Printf(_("%s will use the most restrictive of these three settings:"), pSkinAdvanced->GetApplicationShortName().c_str());
    diskUsageBoxSizer->Add(new wxStaticText( diskUsageStaticBox, -1, MostRestrictiveText, wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );

    wxFlexGridSizer* diskUsageGridSizer = new wxFlexGridSizer( 3, 3, 0, 0 );
    diskUsageGridSizer->AddGrowableCol( 2 );
    diskUsageGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    diskUsageGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxString DiskMaxSpaceTT = wxEmptyString;
    DiskMaxSpaceTT.Printf(_("Limit the total amount of disk space used by %s."), pSkinAdvanced->GetApplicationShortName().c_str());

    m_chkDiskMaxSpace = new wxCheckBox (
        diskUsageStaticBox, ID_CHKDISKMAXSPACE, _("Use no more than"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    m_chkDiskMaxSpace->SetToolTip(DiskMaxSpaceTT);
    diskUsageGridSizer->Add( m_chkDiskMaxSpace, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskMaxSpace = new wxTextCtrl( diskUsageStaticBox, ID_TXTDISKMAXSPACE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtDiskMaxSpace->SetToolTip(DiskMaxSpaceTT);
    diskUsageGridSizer->Add( m_txtDiskMaxSpace, 0, wxALL, 1 );

    wxStaticText* staticText41 = new wxStaticText( diskUsageStaticBox, ID_DEFAULT, _("Gigabytes"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText41->SetToolTip(DiskMaxSpaceTT);
    diskUsageGridSizer->Add( staticText41, 0, wxALL, 5 );

    wxString DiskLeastFreeTT = wxEmptyString;
    DiskLeastFreeTT.Printf(_("Limit disk usage to leave this much free space on the volume where %s stores data."), pSkinAdvanced->GetApplicationShortName().c_str());

    m_chkDiskLeastFree = new wxCheckBox (
        diskUsageStaticBox, ID_CHKDISKLEASTFREE, _("Leave at least"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    m_chkDiskLeastFree->SetToolTip(DiskLeastFreeTT);
    diskUsageGridSizer->Add( m_chkDiskLeastFree, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskLeastFree = new wxTextCtrl( diskUsageStaticBox, ID_TXTDISKLEASTFREE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtDiskLeastFree->SetToolTip(DiskLeastFreeTT);
    diskUsageGridSizer->Add( m_txtDiskLeastFree, 0, wxALL, 1 );

    wxStaticText* staticText43 = new wxStaticText( diskUsageStaticBox, ID_DEFAULT, _("Gigabytes free"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText43->SetToolTip(DiskLeastFreeTT);
    diskUsageGridSizer->Add( staticText43, 0, wxALL, 5 );

    wxString DiskMaxOfTotalTT = wxEmptyString;
    DiskMaxOfTotalTT.Printf(_("Limit the percentage of disk space used by %s on the volume where it stores data."), pSkinAdvanced->GetApplicationShortName().c_str());

    m_chkDiskMaxOfTotal = new wxCheckBox (
        diskUsageStaticBox, ID_CHKDISKMAXOFTOTAL, _("Use no more than"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    m_chkDiskMaxOfTotal->SetToolTip(DiskMaxOfTotalTT);
    diskUsageGridSizer->Add( m_chkDiskMaxOfTotal, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskMaxOfTotal = new wxTextCtrl( diskUsageStaticBox, ID_TXTDISKMAXOFTOTAL, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtDiskMaxOfTotal->SetToolTip(DiskMaxOfTotalTT);
    diskUsageGridSizer->Add( m_txtDiskMaxOfTotal, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText45 = new wxStaticText( diskUsageStaticBox, ID_DEFAULT, _("% of total"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText45->SetToolTip(DiskMaxOfTotalTT);
    diskUsageGridSizer->Add( staticText45, 0, wxALL, 5 );

    diskUsageBoxSizer->Add(diskUsageGridSizer, 0, wxEXPAND, 1 );
    diskAndMemoryBoxSizer->Add( diskUsageBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBox* memoryUsageStaticBox = new wxStaticBox( diskMemoryTab, -1, _("Memory") );
    wxStaticBoxSizer* memoryUsageBoxSizer = new wxStaticBoxSizer( memoryUsageStaticBox, wxVERTICAL );
    makeStaticBoxLabelItalic(memoryUsageStaticBox);

    wxFlexGridSizer* memoryUsageGridSizer = new wxFlexGridSizer( 3, 0, 0 );
    memoryUsageGridSizer->AddGrowableCol( 2 );
    memoryUsageGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    memoryUsageGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxString MemoryMaxInUseTT = wxEmptyString;
    MemoryMaxInUseTT.Printf(_("Limit the memory used by %s when you're using the computer."), pSkinAdvanced->GetApplicationShortName().c_str());

    wxStaticText* staticText50 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("When computer is in use, use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    staticText50->SetToolTip(MemoryMaxInUseTT);
    memoryUsageGridSizer->Add( staticText50, 0, wxALL|wxEXPAND, 5 );

    textCtrlSize = getTextCtrlSize(wxT("100.00"));
    m_txtMemoryMaxInUse = new wxTextCtrl( memoryUsageStaticBox, ID_TXTMEMORYMAXINUSE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtMemoryMaxInUse->SetToolTip(MemoryMaxInUseTT);
    memoryUsageGridSizer->Add( m_txtMemoryMaxInUse, 0, wxALL, 1 );

    /*xgettext:no-c-format*/ 
    wxStaticText* staticText51 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText51->SetToolTip(MemoryMaxInUseTT);
    memoryUsageGridSizer->Add( staticText51, 0, wxALL, 5 );

    wxString MemoryMaxOnIdleTT = wxEmptyString;
    MemoryMaxOnIdleTT.Printf(_("Limit the memory used by %s when you're not using the computer."), pSkinAdvanced->GetApplicationShortName().c_str());

    wxStaticText* staticText52 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("When computer is not in use, use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    staticText52->SetToolTip(MemoryMaxOnIdleTT);
    memoryUsageGridSizer->Add( staticText52, 0, wxALL|wxEXPAND, 5 );

    m_txtMemoryMaxOnIdle = new wxTextCtrl( memoryUsageStaticBox, ID_TXTMEMORYMAXONIDLE, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtMemoryMaxOnIdle->SetToolTip(MemoryMaxOnIdleTT);
    memoryUsageGridSizer->Add( m_txtMemoryMaxOnIdle, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText53 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText53->SetToolTip(MemoryMaxOnIdleTT);
    memoryUsageGridSizer->Add( staticText53, 0, wxALL, 5 );

    m_chkMemoryWhileSuspended = new wxCheckBox( memoryUsageStaticBox, ID_CHKMEMORYWHILESUSPENDED, _("Leave non-GPU tasks in memory while suspended"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkMemoryWhileSuspended->SetToolTip( _("If checked, suspended tasks stay in memory, and resume with no work lost. If unchecked, suspended tasks are removed from memory, and resume from their last checkpoint.") );

    memoryUsageGridSizer->Add( m_chkMemoryWhileSuspended, 0, wxALL, 5 );

    memoryUsageGridSizer->AddSpacer(1);
    memoryUsageGridSizer->AddSpacer(1);

    wxString DiskMaxSwapTT = wxEmptyString;
    DiskMaxSwapTT.Printf(_("Limit the swap space (page file) used by %s."), pSkinAdvanced->GetApplicationShortName().c_str());

    wxStaticText* staticText48 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("Page/swap file: use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    staticText48->SetToolTip(DiskMaxSwapTT);
    memoryUsageGridSizer->Add( staticText48, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskMaxSwap = new wxTextCtrl( memoryUsageStaticBox, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtDiskMaxSwap->SetToolTip(DiskMaxSwapTT);
    memoryUsageGridSizer->Add( m_txtDiskMaxSwap, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    wxStaticText* staticText49 = new wxStaticText( memoryUsageStaticBox, ID_DEFAULT, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText49->SetToolTip(DiskMaxSwapTT);
    memoryUsageGridSizer->Add( staticText49, 0, wxALL, 5 );

    memoryUsageBoxSizer->Add( memoryUsageGridSizer, 0, wxEXPAND, 1 );

    diskAndMemoryBoxSizer->Add( memoryUsageBoxSizer, 0, wxALL|wxEXPAND, 1 );

    diskMemoryTab->SetSizer( diskAndMemoryBoxSizer );
    diskMemoryTab->Layout();
    diskAndMemoryBoxSizer->Fit( diskMemoryTab );

    return diskMemoryTab;
}


wxPanel* CDlgAdvPreferencesBase::createDailySchedulesTab(wxNotebook* notebook)
{
    wxSize textCtrlSize = getTextCtrlSize(wxT("23:59"));
    wxString toString(_("to"));
    
    wxPanel* dailySchedulesTab = new wxPanel( notebook, ID_TABPAGE_SCHED, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    dailySchedulesTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* dailySchedulesTabSizer = new wxBoxSizer( wxVERTICAL );

    // Computing schedule
    //
    wxStaticBox* computingTimesStaticBox = new wxStaticBox(dailySchedulesTab, -1, _("Schedule computing") );
    wxStaticBoxSizer* computingTimesStaticBoxSizer = new wxStaticBoxSizer(computingTimesStaticBox, wxVERTICAL);
    makeStaticBoxLabelItalic(computingTimesStaticBox);

    wxBoxSizer* cpuTimesEveryDaySizer = new wxBoxSizer( wxHORIZONTAL );

    wxString ProcEveryDayTT(_("Compute only during a particular range of hours each day."));
    m_chkProcEveryDay = new wxCheckBox(
        computingTimesStaticBox, ID_CHKPROCEVERYDAY,
        _("Compute only between"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkProcEveryDay->SetToolTip(ProcEveryDayTT);
//    m_chkProcEveryDay->SetToolTip(_("Compute only during a particular range of hours each day."));
    cpuTimesEveryDaySizer->Add( m_chkProcEveryDay, 0, wxLEFT|wxRIGHT, 5 );

    m_txtProcEveryDayStart = new wxTextCtrl( computingTimesStaticBox, ID_TXTPROCEVERYDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtProcEveryDayStart->SetToolTip(ProcEveryDayTT);
    cpuTimesEveryDaySizer->Add( m_txtProcEveryDayStart, 0, wxLEFT|wxRIGHT, 1 );

    wxStaticText* staticText25 = new wxStaticText( computingTimesStaticBox, ID_DEFAULT, _("and"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    staticText25->SetToolTip(ProcEveryDayTT);
    cpuTimesEveryDaySizer->Add( staticText25, 0, wxLEFT|wxRIGHT, 5 );

    m_txtProcEveryDayStop = new wxTextCtrl( computingTimesStaticBox, ID_TXTPROCEVERYDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    m_txtProcEveryDayStop->SetToolTip(ProcEveryDayTT);
    cpuTimesEveryDaySizer->Add( m_txtProcEveryDayStop, 0, wxLEFT|wxRIGHT, 1 );

    computingTimesStaticBoxSizer->Add( cpuTimesEveryDaySizer, 0, wxLEFT|wxRIGHT, 1 );

    wxStaticBox* procSpecialTimesStaticBox = new wxStaticBox(computingTimesStaticBox, -1, _("Day-of-week override") );
    wxStaticBoxSizer* procSpecialTimesStaticBoxSizer = new wxStaticBoxSizer(procSpecialTimesStaticBox, wxVERTICAL);
    makeStaticBoxLabelItalic(procSpecialTimesStaticBox);

    wxStaticText* staticText36 = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, _("Override the \"Every day\" times above on the days selected below:"), wxDefaultPosition, wxDefaultSize, 0 );
    procSpecialTimesStaticBoxSizer->Add( staticText36, 0, wxALL, 0 );
 
    procSpecialTimesStaticBoxSizer->AddSpacer(3);

//    procSpecialTimesStaticBox->SetToolTip(_("On each selected \"override\" day, ignore the \"Every day\" times above and suspend if the time is outside the range shown for that day"));
   
    wxFlexGridSizer* procDaysSizer = new wxFlexGridSizer( 4, 9, 0, 0 );
    procDaysSizer->SetFlexibleDirection( wxHORIZONTAL );
    procDaysSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    // Tooltips for Day-of-Week override wxCheckBoxes and wxTextCtrls are set in CDlgAdvPreferences::SetSpecialTooltips()
    wxString procDaysTimeTT(PROC_DAY_OF_WEEK_TOOLTIP_TEXT);

    m_chkProcMonday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCMONDAY, _("Monday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcMonday, 0, wxLEFT, 5 );

    m_txtProcMondayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCMONDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcMondayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcMonday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcMonday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcMonday , 0, wxTOP, 5 );

    m_txtProcMondayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCMONDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcMondayStop, 0, wxALL, 1 );

    procDaysSizer->AddSpacer(15);

    m_chkProcTuesday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcTuesday, 0, wxLEFT, 5 );

    m_txtProcTuesdayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCTUESDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcTuesdayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcTuesday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcTuesday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcTuesday , 0, wxTOP, 5 );

    m_txtProcTuesdayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCTUESDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcTuesdayStop, 0, wxALL, 1 );

    m_chkProcWednesday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcWednesday, 0, wxLEFT, 5 );

    m_txtProcWednesdayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCWEDNESDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcWednesdayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcWednesday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcWednesday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcWednesday , 0, wxTOP, 5 );

    m_txtProcWednesdayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCWEDNESDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcWednesdayStop, 0, wxALL, 1 );

    procDaysSizer->AddSpacer(15);

    m_chkProcThursday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcThursday, 0, wxLEFT, 5 );

    m_txtProcThursdayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCTHURSDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcThursdayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcThursday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcThursday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcThursday , 0, wxTOP, 5 );

    m_txtProcThursdayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCTHURSDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcThursdayStop, 0, wxALL, 1 );

    m_chkProcFriday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCFRIDAY, _("Friday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcFriday, 0, wxLEFT, 5 );

    m_txtProcFridayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCFRIDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcFridayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcFriday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcFriday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcFriday , 0, wxTOP, 5 );

    m_txtProcFridayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCFRIDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcFridayStop, 0, wxALL, 1 );

    procDaysSizer->AddSpacer(15);

    m_chkProcSaturday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcSaturday, 0, wxLEFT, 5 );

    m_txtProcSaturdayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCSATURDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcSaturdayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcSaturday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcSaturday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcSaturday , 0, wxTOP, 5 );

    m_txtProcSaturdayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCSATURDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcSaturdayStop, 0, wxALL, 1 );

    m_chkProcSunday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcSunday, 0, wxLEFT, 5 );

    m_txtProcSundayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCSUNDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcSundayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcSunday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcSunday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcSunday , 0, wxTOP, 5 );

    m_txtProcSundayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCSUNDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcSundayStop, 0, wxALL, 1 );
    
    procSpecialTimesStaticBoxSizer->Add( procDaysSizer, 0, wxALL, 0 );
    computingTimesStaticBoxSizer->Add( procSpecialTimesStaticBoxSizer, 1, wxEXPAND | wxALL, 1 );
    dailySchedulesTabSizer->Add( computingTimesStaticBoxSizer, 1, wxEXPAND | wxALL, 1 );
    
    // Network schedule
    //
    wxStaticBox* networkTimesStaticBox = new wxStaticBox( dailySchedulesTab, -1, _("Schedule network usage") );
    wxStaticBoxSizer* networkTimesBoxSizer = new wxStaticBoxSizer( networkTimesStaticBox, wxVERTICAL );
    makeStaticBoxLabelItalic(networkTimesStaticBox);

    wxBoxSizer* networkTimesEveryDaySizer = new wxBoxSizer( wxHORIZONTAL );

    wxString NetEveryDayTT(_("Transfer files only during a particular range of hours each day."));
    m_chkNetEveryDay = new wxCheckBox(
        networkTimesStaticBox, ID_CHKNETEVERYDAY, _("Tranfer files only between"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkNetEveryDay->SetToolTip(NetEveryDayTT);
    networkTimesEveryDaySizer->Add( m_chkNetEveryDay, 0, wxLEFT|wxRIGHT, 5 );

    m_txtNetEveryDayStart = new wxTextCtrl( networkTimesStaticBox, ID_TXTNETEVERYDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    m_txtNetEveryDayStart->SetToolTip(NetEveryDayTT);
    networkTimesEveryDaySizer->Add( m_txtNetEveryDayStart, 0, wxLEFT|wxRIGHT, 1 );

    wxStaticText* staticText37 = new wxStaticText( networkTimesStaticBox, ID_DEFAULT, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    staticText37->SetToolTip(NetEveryDayTT);
    networkTimesEveryDaySizer->Add( staticText37, 0, wxLEFT|wxRIGHT, 5 );

    m_txtNetEveryDayStop = new wxTextCtrl( networkTimesStaticBox, ID_TXTNETEVERYDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    m_txtNetEveryDayStop->SetToolTip(NetEveryDayTT);
    networkTimesEveryDaySizer->Add( m_txtNetEveryDayStop, 0, wxLEFT|wxRIGHT, 1 );

    networkTimesBoxSizer->Add( networkTimesEveryDaySizer, 0, wxLEFT|wxRIGHT, 1 );

    wxStaticBox* netSpecialTimesStaticBox = new wxStaticBox(networkTimesStaticBox, -1, _("Day-of-week override") );
    wxStaticBoxSizer* netSpecialTimesStaticBoxSizer = new wxStaticBoxSizer(netSpecialTimesStaticBox, wxVERTICAL);
    makeStaticBoxLabelItalic(netSpecialTimesStaticBox);
    
    wxStaticText* staticText39 = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, _("Override the \"Every day\" times above on the days selected below:"), wxDefaultPosition, wxDefaultSize, 0 );
    netSpecialTimesStaticBoxSizer->Add( staticText39, 0, wxALL, 0 );

    netSpecialTimesStaticBoxSizer->AddSpacer(3);
    
//    netSpecialTimesStaticBox->SetToolTip(_("On each selected \"override\" day, ignore the \"Every day\" times above and suspend if the time is outside the range shown for that day"));
   
    // Tooltips for Day-of-Week overrides are set in CDlgAdvPreferences::SetSpecialTooltips()
    wxString netDaysTimeTT(NET_DAY_OF_WEEK_TOOLTIP_TEXT);

    wxFlexGridSizer* netDaysGridSizer = new wxFlexGridSizer( 4, 9, 0, 0 );
    netDaysGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    netDaysGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_chkNetMonday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETMONDAY, _("Monday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetMonday, 0, wxLEFT, 5 );

    m_txtNetMondayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETMONDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetMondayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetMonday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetMonday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetMonday , 0, wxTOP, 5 );

    m_txtNetMondayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETMONDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetMondayStop, 0, wxALL, 1 );

    netDaysGridSizer->AddSpacer(15);
    
    m_chkNetTuesday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetTuesday, 0, wxLEFT, 5 );

    m_txtNetTuesdayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETTUESDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetTuesdayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetTuesay = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetTuesay->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetTuesay , 0, wxTOP, 5 );

    m_txtNetTuesdayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETTUESDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetTuesdayStop, 0, wxALL, 1 );

    m_chkNetWednesday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetWednesday, 0, wxLEFT, 5 );

    m_txtNetWednesdayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETWEDNESDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetWednesdayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetWednesday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetWednesday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetWednesday , 0, wxTOP, 5 );

    m_txtNetWednesdayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETWEDNESDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetWednesdayStop, 0, wxALL, 1 );

    netDaysGridSizer->AddSpacer(15);
    
    m_chkNetThursday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetThursday, 0, wxLEFT, 5 );

    m_txtNetThursdayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETTHURSDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetThursdayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetThursday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetThursday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetThursday , 0, wxTOP, 5 );

    m_txtNetThursdayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETTHURSDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetThursdayStop, 0, wxALL, 1 );

    m_chkNetFriday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETFRIDAY, _("Friday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetFriday, 0, wxLEFT, 5 );

    m_txtNetFridayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETFRIDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetFridayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetFriday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetFriday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetFriday , 0, wxTOP, 5 );

    m_txtNetFridayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETFRIDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetFridayStop, 0, wxALL, 1 );

    netDaysGridSizer->AddSpacer(15);
    
    m_chkNetSaturday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetSaturday, 0, wxLEFT, 5 );

    m_txtNetSaturdayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETSATURDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetSaturdayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetSaturday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetSaturday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetSaturday , 0, wxTOP, 5 );

    m_txtNetSaturdayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETSATURDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetSaturdayStop, 0, wxALL, 1 );

    m_chkNetSunday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetSunday, 0, wxLEFT, 5 );

    m_txtNetSundayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETSUNDAYSTART, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetSundayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetSunday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetSunday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetSunday , 0, wxTOP, 5 );

    m_txtNetSundayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETSUNDAYSTOP, wxT(""), wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetSundayStop, 0, wxALL, 1 );

    netSpecialTimesStaticBoxSizer->Add( netDaysGridSizer, 1, wxEXPAND | wxALL, 1 );
    networkTimesBoxSizer->Add(netSpecialTimesStaticBoxSizer, 1, wxEXPAND | wxALL, 1 );
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

void CDlgAdvPreferencesBase::makeStaticBoxLabelItalic(wxStaticBox* staticBox) {
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxFont myFont = staticBox->GetFont();
    myFont.MakeItalic();
    myFont.MakeBold();
    staticBox->SetOwnFont(myFont);
#endif
}


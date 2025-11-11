// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
#include "BOINCBaseFrame.h"

#include "DlgAdvPreferencesBase.h"

#include "res/usage.xpm"
#include "res/xfer.xpm"
#include "res/proj.xpm"
#include "res/clock.xpm"

#define STATICBOXBORDERSIZE 8
#define STATICBOXVERTICALSPACER 10
#define DAYOFWEEKBORDERSIZE 10

///////////////////////////////////////////////////////////////////////////

// NOTE: On MS Windows with wxWidgets 3.0, controls inside a wxStaticBox
// don't refresh properly unless they are children of the wxStaticBox!
//
CDlgAdvPreferencesBase::CDlgAdvPreferencesBase( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) :
    wxDialog( parent, id, title, pos, size, style )
{
    int iImageIndex = 0;
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

    // Get the current display space for the current window
    int iDisplay = wxNOT_FOUND;
    if ( wxGetApp().GetFrame() != NULL )
        iDisplay = wxDisplay::GetFromWindow(wxGetApp().GetFrame());
    if ( iDisplay == wxNOT_FOUND )
        iDisplay = 0;
    wxDisplay *display = new wxDisplay(iDisplay);
    wxRect rDisplay = display->GetClientArea();
    const bool bNeedScrollableWindow = rDisplay.GetHeight() < 768;
    this->SetMaxSize(wxSize(-1, rDisplay.GetHeight()));

    wxBoxSizer* dialogSizer = new wxBoxSizer( wxVERTICAL );

    m_bUsingLocalPrefs = doesLocalPrefsFileExist();
    if (web_prefs_url->IsEmpty()) {
        m_bmpWarning = NULL;
    } else {
        wxStaticBox* topControlsStaticBox = new wxStaticBox( this, -1, wxEmptyString );

        wxStaticBoxSizer* topControlsSizer = new wxStaticBoxSizer( topControlsStaticBox, wxHORIZONTAL );

        m_bmpWarning = new wxStaticBitmap( topControlsStaticBox, ID_DEFAULT, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
        m_bmpWarning->SetMinSize( wxSize( 48,48 ) );

        topControlsSizer->Add( m_bmpWarning, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

        wxBoxSizer* legendSizer = new wxBoxSizer( wxVERTICAL );

        if (m_bUsingLocalPrefs) {
            legendSizer->Add(
                new wxStaticText( topControlsStaticBox, ID_DEFAULT,
                    _("Using local prefs.")
                    + "  "
                    +_("Click to use web prefs from"),
                    wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALL, 1
            );
        } else {
            legendSizer->Add(
                new wxStaticText( topControlsStaticBox, ID_DEFAULT,
                            _("Using web prefs from"),
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

        if (!m_bUsingLocalPrefs) {
            legendSizer->Add(
                new wxStaticText( topControlsStaticBox, ID_DEFAULT,
                     _("Set values and click Save to use local prefs instead."),
                     wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALL, 1
            );
        }

        topControlsSizer->Add( legendSizer, 1, wxALL, 1 );

        m_btnClear = new wxButton( topControlsStaticBox, ID_BTN_CLEAR, _("Use web prefs"), wxDefaultPosition, wxDefaultSize, 0 );
        m_btnClear->SetToolTip( _("Restore web-based preferences and close the dialog.") );
        if (!m_bUsingLocalPrefs) {
            m_btnClear->Hide();
        }

        topControlsSizer->Add( m_btnClear, 0, wxALIGN_BOTTOM|wxALL, 4 );

#ifdef __WXMAC__
        dialogSizer->Add( topControlsSizer, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 10 );
#else
        dialogSizer->Add( topControlsSizer, 0, wxALL|wxEXPAND, 5 );
#endif
    }
    m_panelControls = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_panelControls->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* notebookSizer = new wxBoxSizer( wxVERTICAL );

    m_Notebook = new wxNotebook( m_panelControls, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxNB_TOP );
    m_Notebook->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxImageList* pImageList = new wxImageList(16, 16, true, 0);
    wxASSERT(pImageList != NULL);
    m_Notebook->SetImageList(pImageList);

    // Note: we must set the third AddPage argument ("select") to
    // true for each page or ToolTips won't initialize properly.
    m_panelProcessor = createProcessorTab(m_Notebook, bNeedScrollableWindow);
    iImageIndex = pImageList->Add(wxBitmap(proj_xpm));
    m_Notebook->AddPage( m_panelProcessor, _("Computing"), true, iImageIndex );

    m_panelNetwork = createNetworkTab(m_Notebook);
    iImageIndex = pImageList->Add(wxBitmap(xfer_xpm));
    m_Notebook->AddPage( m_panelNetwork, _("Network"), true, iImageIndex );

    m_panelDisk = createDiskTab(m_Notebook);
    iImageIndex = pImageList->Add(wxBitmap(usage_xpm));
    m_Notebook->AddPage( m_panelDisk, _("Disk"), true, iImageIndex );

    m_panelDailySchedules = createDailySchedulesTab(m_Notebook);
    iImageIndex = pImageList->Add(wxBitmap(clock_xpm));
    m_Notebook->AddPage( m_panelDailySchedules, _("Daily schedules"), true, iImageIndex );

    notebookSizer->Add( m_Notebook, 1, wxEXPAND | wxALL, 1 );

    m_panelControls->SetSizer( notebookSizer );
    m_panelControls->Layout();
    notebookSizer->Fit( m_panelControls );

    dialogSizer->Add( m_panelControls, 1, wxALL|wxEXPAND, 5 );

    m_panelButtons = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    m_btnOK = new wxButton( m_panelButtons, wxID_OK, _("&Save"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnOK->SetToolTip( _("Save all values and close the dialog") );
    if (m_bUsingLocalPrefs) {
        m_btnOK->SetDefault();
    }
    buttonSizer->Add( m_btnOK, 0, wxALL, 5 );

    m_btnCancel = new wxButton( m_panelButtons, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnCancel->SetToolTip( _("Close the dialog without saving") );
    if (!m_bUsingLocalPrefs) {
        m_btnCancel->SetDefault();
    }
    buttonSizer->Add( m_btnCancel, 0, wxALL, 5 );

    m_btnHelp = new wxButton( m_panelButtons, ID_HELPBOINC, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnHelp->SetToolTip( _("Shows the preferences web page.") );

    buttonSizer->Add( m_btnHelp, 0, wxALL, 5 );

    m_panelButtons->SetSizer( buttonSizer );
    m_panelButtons->Layout();
    buttonSizer->Fit( m_panelButtons );
    dialogSizer->Add( m_panelButtons, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1 );

    dialogSizer->Fit( this );
    this->SetSizer( dialogSizer );
    if (bNeedScrollableWindow) {
        m_panelProcessor->SetVirtualSize(notebookSizer->GetMinSize());
    }
}

#define PAD0    1
#define PAD1    3

// this version lets you attach different tooltips to different items
//
void CDlgAdvPreferencesBase::add_row_to_sizer2(wxSizer* toSizer,
    wxWindow* item1, wxString& tt1,
    wxWindow* item2, wxString& tt2,
    wxWindow* item3, wxString& tt3,
    wxWindow* item4, wxString& tt4,
    wxWindow* item5, wxString& tt5
) {
    wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

    rowSizer->Add(item1, 0, wxALL, PAD1);
    item1->SetToolTip(tt1);
    rowSizer->Add(item2, 0, wxALL, PAD0);
    item2->SetToolTip(tt2);
    rowSizer->Add(item3, 0, wxALL, PAD1);
    item3->SetToolTip(tt3);
    rowSizer->Add(item4, 0, wxALL, PAD0);
    item4->SetToolTip(tt4);
    rowSizer->Add(item5, 0, wxALL, PAD1);
    item5->SetToolTip(tt5);

    toSizer->Add(rowSizer, 0, 0, 1);
}

void CDlgAdvPreferencesBase::addNewRowToSizer(
    wxSizer* toSizer, wxString& toolTipText,
    wxWindow* first, wxWindow* second, wxWindow* third,
    wxWindow* fourth, wxWindow* fifth
) {
    wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

#ifdef __WXMSW__
    // MSW adds space to the right of checkbox label
    if (first->IsKindOf(CLASSINFO(wxCheckBox))) {
        rowSizer->Add(first, 0, wxTOP | wxBOTTOM |wxLEFT, PAD1 );
    } else
#endif
        rowSizer->Add(first, 0, wxALL, PAD1 );

    // the last arg is padding.  Less for text fields, to make things line up

    first->SetToolTip(toolTipText);

    rowSizer->Add(second, 0, wxALL, PAD0 );
    second->SetToolTip(toolTipText);

    rowSizer->Add(third, 0, wxALL, PAD1);
    third->SetToolTip(toolTipText);

    if (fourth) {
        rowSizer->Add(fourth, 0, wxALL, PAD0);
        fourth->SetToolTip(toolTipText);
    }

    if (fifth) {
        rowSizer->Add(fifth, 0, wxALL, PAD1);
        fifth->SetToolTip(toolTipText);
    }

    toSizer->Add( rowSizer, 0, 0, 1 );
}

wxScrolledWindow* CDlgAdvPreferencesBase::createProcessorTab(wxNotebook* notebook, bool bScrollable) {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);

    wxSize textCtrlSize = getTextCtrlSize(wxT("999.99"));

    wxScrolledWindow* processorTab = new wxScrolledWindow( notebook, ID_TABPAGE_PROC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxVSCROLL );
    if (bScrollable) {
        processorTab->SetScrollRate(0, 5);
    }

    wxBoxSizer* processorTabSizer = new wxBoxSizer( wxVERTICAL );

    // ------------ In-use box --------------
    //
    wxStaticBox* box = new wxStaticBox(processorTab, -1, _("When computer is in use") );
    wxStaticBoxSizer* box_sizer = new wxStaticBoxSizer(box, wxVERTICAL);
    makeStaticBoxLabelItalic(box);

    // in-use if input in last X min
    //
    wxString ProcIdleForTT(_("This determines when the computer is considered 'in use'."));
    wxStaticText* staticText24 = new wxStaticText(
        box, ID_DEFAULT,
        // context: 'In use' means mouse or keyboard input in last ___ minutes
        _("'In use' means mouse or keyboard input in last"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtProcIdleFor = new wxTextCtrl(
        box, ID_TXTPROCIDLEFOR, wxEmptyString, wxDefaultPosition, getTextCtrlSize(wxT("999.99")), wxTE_RIGHT
    );
    wxStaticText* staticText25 = new wxStaticText(
        box, ID_DEFAULT,
        // context: 'In use' means mouse or keyboard input in last ___ minutes
        _("minutes"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    addNewRowToSizer(box_sizer, ProcIdleForTT, staticText24, m_txtProcIdleFor, staticText25);

    // suspend while in use
    //
    m_chkProcInUse = new wxCheckBox(
        box, ID_CHKPROCINUSE,
        _("Suspend when computer is in use"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_chkProcInUse->SetToolTip(
        _("Check this to suspend computing and file transfers when you're using the computer.")
    );
    box_sizer->Add( m_chkProcInUse, 0, wxALL, 5 );

    // suspend GPU while in use
    //
    m_chkGPUProcInUse = new wxCheckBox(
        box, ID_CHKGPUPROCINUSE,
        _("Suspend GPU computing when computer is in use"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_chkGPUProcInUse->SetToolTip(
        _("Check this to suspend GPU computing when you're using the computer.")
    );
    box_sizer->Add( m_chkGPUProcInUse, 0, wxALL, 5 );

    // max # CPUs and throttling
    //
    wxStaticText* staticText20 = new wxStaticText(
        box, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtProcUseProcessors = new wxTextCtrl(box, ID_TXTPROCUSEPROCESSORS, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    /*xgettext:no-c-format*/
    wxStaticText* staticText22 = new wxStaticText(
        box, ID_DEFAULT, _("% of the CPUs and at most"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtProcUseCPUTime = new wxTextCtrl(box, ID_TXTPROCUSECPUTIME, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    /*xgettext:no-c-format*/
    wxStaticText* staticText23 = new wxStaticText(box, ID_DEFAULT, _("% of CPU time"), wxDefaultPosition, wxDefaultSize, 0 );
    /*xgettext:no-c-format*/
    wxString tt_ncpus(_("Keep some CPUs free for other applications. Example: 75% means use 6 cores on an 8-core CPU."));
    /*xgettext:no-c-format*/
    wxString tt_throttle(_("Suspend/resume computing every few seconds to reduce CPU temperature and energy usage. Example: 75% means compute for 3 seconds, wait for 1 second, and repeat."));
    add_row_to_sizer2(box_sizer,
        staticText20, tt_ncpus,
        m_txtProcUseProcessors, tt_ncpus,
        staticText22, tt_ncpus,
        m_txtProcUseCPUTime, tt_throttle,
        staticText23, tt_throttle
    );

    // max CPU load
    //
    m_chkMaxLoad = new wxCheckBox(
        box, ID_CHKMAXLOAD,
        wxString(_("Suspend when non-BOINC CPU usage is above")),
        wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtMaxLoad = new wxTextCtrl(
        box, ID_TXTMAXLOAD, wxEmptyString, wxDefaultPosition, getTextCtrlSize(wxT("100.00")), wxTE_RIGHT
    );
    wxString load_tt(_("Suspend computing when your computer is busy running other programs."));
    addNewRowToSizer(box_sizer,
        load_tt,
        m_chkMaxLoad, m_txtMaxLoad,
        new wxStaticText(box, ID_DEFAULT, wxT("%"), wxDefaultPosition, wxDefaultSize, 0)
    );

    // max memory while in use
    //
    wxString MemoryMaxInUseTT = wxEmptyString;
    MemoryMaxInUseTT.Printf(_("Limit the memory used by %s when you're using the computer."), pSkinAdvanced->GetApplicationShortName().c_str());
    wxStaticText* staticText50 = new wxStaticText(box, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, 0 );
    textCtrlSize = getTextCtrlSize(wxT("100.00"));
    m_txtMemoryMaxInUse = new wxTextCtrl( box, ID_TXTMEMORYMAXINUSE, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    /*xgettext:no-c-format*/
    wxStaticText* staticText51 = new wxStaticText( box, ID_DEFAULT, _("% of memory"), wxDefaultPosition, wxDefaultSize, 0 );
    addNewRowToSizer(box_sizer, MemoryMaxInUseTT, staticText50, m_txtMemoryMaxInUse, staticText51);

    processorTabSizer->AddSpacer( STATICBOXVERTICALSPACER );
    processorTabSizer->Add(box_sizer, 0, wxLEFT | wxRIGHT | wxEXPAND, STATICBOXBORDERSIZE );

    // ------------ Not-in-use box --------------
    //

    box = new wxStaticBox(processorTab, -1, _("When computer is not in use") );
    box_sizer = new wxStaticBoxSizer(box, wxVERTICAL);
    makeStaticBoxLabelItalic(box);

    // max # CPUs and throttling
    //
    wxStaticText* staticText20niu = new wxStaticText(
        box, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtProcUseProcessorsNotInUse = new wxTextCtrl(box, ID_TXTPROCUSEPROCESSORSNOTINUSE, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT);
    wxStaticText* staticText22niu = new wxStaticText(
        /*xgettext:no-c-format*/
        box, ID_DEFAULT, _("% of the CPUs and at most"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtProcUseCPUTimeNotInUse = new wxTextCtrl(box, ID_TXTPROCUSECPUTIMENOTINUSE, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT);
    /*xgettext:no-c-format*/
    wxStaticText* staticText23niu = new wxStaticText(box, ID_DEFAULT, _("% of CPU time"), wxDefaultPosition, wxDefaultSize, 0);
    add_row_to_sizer2(box_sizer,
        staticText20niu, tt_ncpus,
        m_txtProcUseProcessorsNotInUse, tt_ncpus,
        staticText22niu, tt_ncpus,
        m_txtProcUseCPUTimeNotInUse, tt_throttle,
        staticText23niu, tt_throttle
    );

    // max CPU load
    //
    wxString MaxLoadCheckBoxTextniu = wxEmptyString;
    MaxLoadCheckBoxTextniu.Printf(_("Suspend when non-BOINC CPU usage is above"));
    m_chkMaxLoadNotInUse = new wxCheckBox(
        box, ID_CHKMAXLOADNOTINUSE, MaxLoadCheckBoxTextniu, wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtMaxLoadNotInUse = new wxTextCtrl(
        box, ID_TXTMAXLOADNOTINUSE, wxEmptyString, wxDefaultPosition, getTextCtrlSize(wxT("100.00")), wxTE_RIGHT
    );
    wxStaticText* staticText26niu = new wxStaticText(box, ID_DEFAULT, wxT("%"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    wxString MaxLoadTTniu(_("Suspend computing when your computer is busy running other programs."));
    addNewRowToSizer(box_sizer, MaxLoadTTniu, m_chkMaxLoadNotInUse, m_txtMaxLoadNotInUse, staticText26niu);

    // max memory when not in use
    //
    wxString MemoryMaxOnIdleTT = wxEmptyString;
    MemoryMaxOnIdleTT.Printf(_("Limit the memory used by %s when you're not using the computer."), pSkinAdvanced->GetApplicationShortName().c_str());
    wxStaticText* staticText52 = new wxStaticText( box, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, 0 );
    m_txtMemoryMaxOnIdle = new wxTextCtrl( box, ID_TXTMEMORYMAXONIDLE, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    /*xgettext:no-c-format*/
    wxStaticText* staticText53 = new wxStaticText( box, ID_DEFAULT, _("% of memory"), wxDefaultPosition, wxDefaultSize, 0 );
    addNewRowToSizer(box_sizer, MemoryMaxOnIdleTT, staticText52, m_txtMemoryMaxOnIdle, staticText53);

    // suspend after idle time
    //
    wxString str0 = wxEmptyString;
    str0.Printf(_("Suspend when no mouse or keyboard input in last"));
    m_chkNoRecentInput = new wxCheckBox(
        box, ID_CHKNORECENTINPUT, str0, wxDefaultPosition, wxDefaultSize, 0
    );
    wxStaticText* staticText28 = new wxStaticText(
        box, ID_DEFAULT,
        _("minutes"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtNoRecentInput = new wxTextCtrl(
        box, ID_TXTNORECENTINPUT, wxEmptyString, wxDefaultPosition, getTextCtrlSize(wxT("999.99")), wxTE_RIGHT
    );
    wxString NoRecentInputTT(_("This allows some computers to enter low-power mode when not in use."));
    addNewRowToSizer(box_sizer, NoRecentInputTT, m_chkNoRecentInput, m_txtNoRecentInput, staticText28);

    processorTabSizer->AddSpacer( STATICBOXVERTICALSPACER );
    processorTabSizer->Add(box_sizer, 0, wxLEFT | wxRIGHT | wxEXPAND, STATICBOXBORDERSIZE);

    // ------------ General box --------------
    //

    box = new wxStaticBox(processorTab, -1, _("General"));
    box_sizer = new wxStaticBoxSizer(box, wxVERTICAL);
    makeStaticBoxLabelItalic(box);

    // suspend while on battery
    //
    m_chkProcOnBatteries = new wxCheckBox(
        box, ID_CHKPROCONBATTERIES,
        _("Suspend when computer is on battery"), wxDefaultPosition, wxDefaultSize, 0
    );
    m_chkProcOnBatteries->SetToolTip(
        _("Check this to suspend computing on portables when running on battery power.")
    );
    box_sizer->Add( m_chkProcOnBatteries, 0, wxALL, 5 );

    // switch every X
    //
    wxString ProcSwitchEveryTT = wxEmptyString;
    ProcSwitchEveryTT.Printf(_("If you run several projects, %s may switch between them this often."), pSkinAdvanced->GetApplicationShortName().c_str());
    wxStaticText* staticText18 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Switch between tasks every ___ minutes
        _("Switch between tasks every"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtProcSwitchEvery = new wxTextCtrl( box, ID_TXTPROCSWITCHEVERY, wxEmptyString, wxDefaultPosition, getTextCtrlSize(wxT("9999.99")), wxTE_RIGHT );
    wxStaticText* staticText19 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Switch between tasks every ___ minutes
        _("minutes"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    addNewRowToSizer(box_sizer, ProcSwitchEveryTT, staticText18, m_txtProcSwitchEvery, staticText19);

    // checkpoint every X
    //
    wxString DiskWriteToDiskTT(_("This controls how often tasks save their state to disk, so that they later can be continued from that point."));
    wxStaticText* staticText46 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Request tasks to checkpoint at most every ___ seconds
        _("Request tasks to checkpoint at most every"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtDiskWriteToDisk = new wxTextCtrl( box, ID_TXTDISKWRITETODISK, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    wxStaticText* staticText47 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Request tasks to checkpoint at most every ___ seconds
        _("seconds"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    addNewRowToSizer(box_sizer, DiskWriteToDiskTT, staticText46, m_txtDiskWriteToDisk, staticText47);

    // leave non-GPU tasks in memory while suspended
//
    m_chkMemoryWhileSuspended = new wxCheckBox(box, ID_CHKMEMORYWHILESUSPENDED, _("Leave non-GPU tasks in memory while suspended"), wxDefaultPosition, wxDefaultSize, 0);
    m_chkMemoryWhileSuspended->SetToolTip(_("If checked, suspended tasks stay in memory, and resume with no work lost. If unchecked, suspended tasks are removed from memory, and resume from their last checkpoint."));
    box_sizer->Add(m_chkMemoryWhileSuspended, 0, wxALL, 5);

#if 0
    // work buffer min
    //
    wxString NetConnectIntervalTT(_("Store at least enough tasks to keep the computer busy for this long."));
    wxStaticText* staticText30 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Store at least ___ days of work
        _("Store at least"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtNetConnectInterval = new wxTextCtrl(
        box, ID_TXTNETCONNECTINTERVAL, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );
    wxStaticText* staticText31 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Store at least ___ days of work
        _("days of work"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    addNewRowToSizer(box_sizer, NetConnectIntervalTT, staticText30, m_txtNetConnectInterval, staticText31);

    // work buffer additional
    //
    wxString NetAdditionalDaysTT(_("Store additional tasks above the minimum level.  Determines how much work is requested when contacting a project."));
    wxStaticText* staticText331 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Store up to an additional ___ days of work
        _("Store up to an additional"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    staticText331->SetToolTip(NetAdditionalDaysTT);
    m_txtNetAdditionalDays = new wxTextCtrl(
        box, ID_TXTNETADDITIONALDAYS, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );
    wxStaticText* staticText341 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Store up to an additional ___ days of work
        _("days of work"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    addNewRowToSizer(box_sizer, NetAdditionalDaysTT, staticText331, m_txtNetAdditionalDays, staticText341);
#else
    // work buffer
    //
    wxString tt_min(_("Store at least enough tasks to keep the computer busy for this long."));
    wxStaticText* staticText30 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Store at least ___ days of work
        _("Store at least"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    m_txtNetConnectInterval = new wxTextCtrl(
        box, ID_TXTNETCONNECTINTERVAL, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );
    wxStaticText* staticText31 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Store at least ___ days of work
        _("days and up to an additional"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    wxString tt_extra(_("Store additional tasks above the minimum level.  Determines how much work is requested when contacting a project."));
    m_txtNetAdditionalDays = new wxTextCtrl(
        box, ID_TXTNETADDITIONALDAYS, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );
    wxStaticText* staticText341 = new wxStaticText(
        box, ID_DEFAULT,
        // context: Store up to an additional ___ days of work
        _("days of work"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    add_row_to_sizer2(box_sizer,
        staticText30, tt_min,
        m_txtNetConnectInterval, tt_min,
        staticText31, tt_min,
        m_txtNetAdditionalDays, tt_extra,
        staticText341, tt_extra
    );
#endif

#if 0
    box_sizer->Add(
        new wxStaticText(box, ID_DEFAULT, _("To suspend by time of day, see the \"Daily Schedules\" section."), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );
#endif
    box_sizer->AddSpacer(1); // Ensure staticText22 is fully visible on Mac

    //processorTabSizer->AddSpacer( STATICBOXVERTICALSPACER );
    processorTabSizer->Add( box_sizer, 0, wxLEFT | wxRIGHT | wxEXPAND, STATICBOXBORDERSIZE );
    //processorTabSizer->AddSpacer( STATICBOXVERTICALSPACER );

    processorTab->SetSizer( processorTabSizer );
    processorTab->Layout();
    if (!bScrollable) {
        processorTabSizer->Fit( processorTab );
    }

    return processorTab;
}

wxPanel* CDlgAdvPreferencesBase::createNetworkTab(wxNotebook* notebook) {
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);

    wxSize textCtrlSize = getTextCtrlSize(wxT("9999.99"));

    wxPanel* networkTab = new wxPanel( notebook, ID_TABPAGE_NET, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    networkTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* networkTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* box = new wxStaticBox( networkTab, -1, _("Network usage") );
    wxStaticBoxSizer* box_sizer = new wxStaticBoxSizer(box, wxVERTICAL);
    makeStaticBoxLabelItalic(box);

    // download rate
    //
    wxString NetDownloadRateTT(_("Limit the download rate of file transfers."));
    m_chkNetDownloadRate = new wxCheckBox( box, ID_CHKNETDOWNLOADRATE, _("Limit download rate to"), wxDefaultPosition, wxDefaultSize, 0 );
    m_txtNetDownloadRate = new wxTextCtrl( box, ID_TXTNETDOWNLOADRATE, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    wxStaticText* staticText33 = new wxStaticText( box, ID_DEFAULT, _("KB/second"), wxDefaultPosition, wxDefaultSize, 0 );
    addNewRowToSizer(box_sizer, NetDownloadRateTT, m_chkNetDownloadRate, m_txtNetDownloadRate, staticText33);

    // upload rate
    //
    wxString NetUploadRateTT(_("Limit the upload rate of file transfers."));
    m_chkNetUploadRate = new wxCheckBox( box, ID_CHKNETUPLOADRATE, _("Limit upload rate to"), wxDefaultPosition, wxDefaultSize, 0 );
    m_txtNetUploadRate = new wxTextCtrl( box, ID_TXTNETUPLOADRATE, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    wxStaticText* staticText35 = new wxStaticText( box, ID_DEFAULT, _("KB/second"), wxDefaultPosition, wxDefaultSize, 0 );
    addNewRowToSizer(box_sizer, NetUploadRateTT, m_chkNetUploadRate, m_txtNetUploadRate, staticText35);

    // long-term quota
    //
    wxString daily_xfer_limitTT = wxEmptyString;
    daily_xfer_limitTT.Printf(_("Example: %s should transfer at most 2000 MB of data every 30 days."), pSkinAdvanced->GetApplicationShortName().c_str());
    m_chk_daily_xfer_limit = new wxCheckBox( box, ID_CHKDAILYXFERLIMIT, _("Limit usage to"), wxDefaultPosition, wxDefaultSize, 0 );
    m_txt_daily_xfer_limit_mb = new wxTextCtrl( box, ID_TXTNETDOWNLOADRATE, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    wxStaticText* staticText_daily_xfer2 = new wxStaticText( box, ID_DEFAULT, _("MB every"), wxDefaultPosition, wxDefaultSize, 0 );
    m_txt_daily_xfer_period_days = new wxTextCtrl( box, ID_TXTNETUPLOADRATE, wxEmptyString, wxDefaultPosition, getTextCtrlSize(wxT("999.99")), wxTE_RIGHT );
    wxStaticText* staticText_daily_xfer4 = new wxStaticText( box, ID_DEFAULT, _("days"), wxDefaultPosition, wxDefaultSize, 0 );
    addNewRowToSizer(box_sizer, daily_xfer_limitTT, m_chk_daily_xfer_limit, m_txt_daily_xfer_limit_mb, staticText_daily_xfer2, m_txt_daily_xfer_period_days, staticText_daily_xfer4);
    box_sizer->Add(
        new wxStaticText( box, ID_DEFAULT, _("To limit transfers by time of day, see the \"Daily Schedules\" section."), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );

    networkTabSizer->AddSpacer( STATICBOXVERTICALSPACER );
    networkTabSizer->Add( box_sizer, 0, wxLEFT | wxRIGHT | wxEXPAND, STATICBOXBORDERSIZE );

        // Context: heading for a group of miscellaneous preferences
    wxStaticBox* connectOptionsStaticBox = new wxStaticBox(
        networkTab, -1,
        // Context: heading for a group of miscellaneous preferences
        _("Other")
    );
    wxStaticBoxSizer* connectOptionsSizer = new wxStaticBoxSizer( connectOptionsStaticBox, wxVERTICAL );
    makeStaticBoxLabelItalic(connectOptionsStaticBox);

    // skip image verification
    //
    wxString NetSkipImageVerificationTT = wxEmptyString;
    NetSkipImageVerificationTT.Printf(_("Check this only if your Internet provider modifies image files. Skipping verification reduces the security of %s."), pSkinAdvanced->GetApplicationShortName().c_str());
    m_chkNetSkipImageVerification = new wxCheckBox( connectOptionsStaticBox, ID_CHKNETSKIPIMAGEVERIFICATION, _("Skip data verification for image files"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkNetSkipImageVerification->SetToolTip(NetSkipImageVerificationTT);
    connectOptionsSizer->Add( m_chkNetSkipImageVerification, 0, wxALL, 5 );

    // confirm before connecting
    //
    m_chkNetConfirmBeforeConnect = new wxCheckBox( connectOptionsStaticBox, ID_CHKNETCONFIRMBEFORECONNECT, _("Confirm before connecting to Internet"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkNetConfirmBeforeConnect->SetToolTip( _("Useful only if you have a modem, ISDN or VPN connection.") );
    connectOptionsSizer->Add( m_chkNetConfirmBeforeConnect, 0, wxALL, 5 );

    // disconnect when done
    m_chkNetDisconnectWhenDone = new wxCheckBox( connectOptionsStaticBox, ID_CHKNETDISCONNECTWHENDONE, _("Disconnect when done"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chkNetDisconnectWhenDone->SetToolTip( _("Useful only if you have a modem, ISDN or VPN connection.") );
    connectOptionsSizer->Add( m_chkNetDisconnectWhenDone, 0, wxALL, 5 );

    networkTabSizer->AddSpacer( STATICBOXVERTICALSPACER );
    networkTabSizer->Add( connectOptionsSizer, 0, wxLEFT | wxRIGHT | wxEXPAND, STATICBOXBORDERSIZE );
    networkTabSizer->AddSpacer( STATICBOXVERTICALSPACER );

    networkTab->SetSizer( networkTabSizer );
    networkTab->Layout();
    networkTabSizer->Fit( networkTab );

    return networkTab;
}

wxPanel* CDlgAdvPreferencesBase::createDiskTab(wxNotebook* notebook) {
    CSkinAdvanced*      pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);

    wxSize textCtrlSize = getTextCtrlSize(wxT("9999.99"));

    wxPanel* diskMemoryTab = new wxPanel( notebook, ID_TABPAGE_DISK, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    diskMemoryTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* diskTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox* diskUsageStaticBox = new wxStaticBox( diskMemoryTab, -1, _("Disk usage") );
    wxStaticBoxSizer* diskUsageBoxSizer = new wxStaticBoxSizer( diskUsageStaticBox, wxVERTICAL );
    makeStaticBoxLabelItalic(diskUsageStaticBox);

    // total disk usage
    //
    wxString DiskMaxSpaceTT = wxEmptyString;
    DiskMaxSpaceTT.Printf(_("Limit the total amount of disk space used by %s."), pSkinAdvanced->GetApplicationShortName().c_str());
    m_chkDiskMaxSpace = new wxCheckBox (
        diskUsageStaticBox, ID_CHKDISKMAXSPACE, _("Use no more than"), wxDefaultPosition, wxDefaultSize, 0 );
    m_txtDiskMaxSpace = new wxTextCtrl( diskUsageStaticBox, ID_TXTDISKMAXSPACE,wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    wxStaticText* staticText41 = new wxStaticText( diskUsageStaticBox, ID_DEFAULT, _("GB"), wxDefaultPosition, wxDefaultSize, 0 );
    addNewRowToSizer(diskUsageBoxSizer, DiskMaxSpaceTT, m_chkDiskMaxSpace, m_txtDiskMaxSpace, staticText41);

    // leave at least X free
    //
    wxString DiskLeastFreeTT = wxEmptyString;
    DiskLeastFreeTT.Printf(_("Limit disk usage to leave this much free space on the volume where %s stores data."), pSkinAdvanced->GetApplicationShortName().c_str());
    m_chkDiskLeastFree = new wxCheckBox (
        diskUsageStaticBox, ID_CHKDISKLEASTFREE, _("Leave at least"), wxDefaultPosition, wxDefaultSize, 0 );
    m_txtDiskLeastFree = new wxTextCtrl( diskUsageStaticBox, ID_TXTDISKLEASTFREE, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    wxStaticText* staticText43 = new wxStaticText( diskUsageStaticBox, ID_DEFAULT, _("GB free"), wxDefaultPosition, wxDefaultSize, 0 );
    addNewRowToSizer(diskUsageBoxSizer, DiskLeastFreeTT, m_chkDiskLeastFree, m_txtDiskLeastFree, staticText43);

    // use at most X%
    //
    wxString DiskMaxOfTotalTT = wxEmptyString;
    DiskMaxOfTotalTT.Printf(_("Limit the percentage of disk space used by %s on the volume where it stores data."), pSkinAdvanced->GetApplicationShortName().c_str());
    m_chkDiskMaxOfTotal = new wxCheckBox (
        diskUsageStaticBox, ID_CHKDISKMAXOFTOTAL, _("Use no more than"), wxDefaultPosition, wxDefaultSize, 0 );
    m_txtDiskMaxOfTotal = new wxTextCtrl( diskUsageStaticBox, ID_TXTDISKMAXOFTOTAL, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    /*xgettext:no-c-format*/
    wxStaticText* staticText45 = new wxStaticText( diskUsageStaticBox, ID_DEFAULT, _("% of total"), wxDefaultPosition, wxDefaultSize, 0 );
    addNewRowToSizer(diskUsageBoxSizer, DiskMaxOfTotalTT, m_chkDiskMaxOfTotal, m_txtDiskMaxOfTotal, staticText45);

    wxString MostRestrictiveText = wxEmptyString;
    MostRestrictiveText.Printf(_("%s will use the most restrictive of the above settings"), pSkinAdvanced->GetApplicationShortName().c_str());
    diskUsageBoxSizer->Add(new wxStaticText(diskUsageStaticBox, -1, MostRestrictiveText, wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );

    // swap space limit
    //
    wxString DiskMaxSwapTT = wxEmptyString;
    DiskMaxSwapTT.Printf(_("Limit the swap space (page file) used by %s."), pSkinAdvanced->GetApplicationShortName().c_str());
    wxStaticText* staticText48 = new wxStaticText(diskUsageStaticBox, ID_DEFAULT, _("Page/swap file: use at most"), wxDefaultPosition, wxDefaultSize, 0 );
    m_txtDiskMaxSwap = new wxTextCtrl(diskUsageStaticBox, ID_TXTDISKWRITETODISK, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );
    /*xgettext:no-c-format*/
    wxStaticText* staticText49 = new wxStaticText(diskUsageStaticBox, ID_DEFAULT, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
    addNewRowToSizer(diskUsageBoxSizer, DiskMaxSwapTT, staticText48, m_txtDiskMaxSwap, staticText49);

    diskTabSizer->AddSpacer(STATICBOXVERTICALSPACER);
    diskTabSizer->Add(diskUsageBoxSizer, 0, wxLEFT | wxRIGHT | wxEXPAND, STATICBOXBORDERSIZE);

    diskMemoryTab->SetSizer( diskTabSizer );
    diskMemoryTab->Layout();
    diskTabSizer->Fit( diskMemoryTab );

    return diskMemoryTab;
}


wxPanel* CDlgAdvPreferencesBase::createDailySchedulesTab(wxNotebook* notebook)
{
    wxSize textCtrlSize = getTextCtrlSize(wxT("23:59 "));

    wxString andString(_("and"));
    wxString toString(wxT(" ")+_("to")+wxT(" "));

    wxPanel* dailySchedulesTab = new wxPanel(
        notebook, ID_TABPAGE_SCHED, wxDefaultPosition,
        wxDefaultSize, wxTAB_TRAVERSAL
    );
    dailySchedulesTab->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* dailySchedulesTabSizer = new wxBoxSizer( wxVERTICAL );

    // Computing schedule
    //
    wxStaticBox* computingTimesStaticBox = new wxStaticBox(
        dailySchedulesTab, -1, _("Computing")
    );
    wxStaticBoxSizer* computingTimesStaticBoxSizer = new wxStaticBoxSizer(
        computingTimesStaticBox, wxVERTICAL
    );
    makeStaticBoxLabelItalic(computingTimesStaticBox);

    wxString ProcEveryDayTT(_("Compute only during a particular period each day."));
    m_chkProcEveryDay = new wxCheckBox(
        computingTimesStaticBox, ID_CHKPROCEVERYDAY,
        _("Compute only between"), wxDefaultPosition, wxDefaultSize, 0
    );

    m_txtProcEveryDayStart = new wxTextCtrl(
        computingTimesStaticBox, ID_TXTPROCEVERYDAYSTART, wxEmptyString,
        wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );

    wxStaticText* staticText25 = new wxStaticText(
        computingTimesStaticBox, ID_DEFAULT, andString, wxDefaultPosition,
        wxDefaultSize, wxALIGN_CENTRE
    );

    m_txtProcEveryDayStop = new wxTextCtrl(
        computingTimesStaticBox, ID_TXTPROCEVERYDAYSTOP, wxEmptyString,
        wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );

    addNewRowToSizer(
        computingTimesStaticBoxSizer, ProcEveryDayTT, m_chkProcEveryDay,
        m_txtProcEveryDayStart, staticText25, m_txtProcEveryDayStop
    );

    wxStaticBox* procSpecialTimesStaticBox = new wxStaticBox(
        computingTimesStaticBox, -1, _("Day-of-week override")
    );
    wxStaticBoxSizer* procSpecialTimesStaticBoxSizer = new wxStaticBoxSizer(
        procSpecialTimesStaticBox, wxVERTICAL
    );
    makeStaticBoxLabelItalic(procSpecialTimesStaticBox);

    wxStaticText* staticText36 = new wxStaticText(
        procSpecialTimesStaticBox, ID_DEFAULT,
        _("Override the times above on the selected days:"),
        wxDefaultPosition, wxDefaultSize, 0
    );
#ifdef __WXMAC__
    procSpecialTimesStaticBoxSizer->Add( staticText36, 0, wxLEFT, 5 );
#else
    procSpecialTimesStaticBoxSizer->Add( staticText36, 0, wxLEFT, DAYOFWEEKBORDERSIZE );
#endif

    procSpecialTimesStaticBoxSizer->AddSpacer(3);

//    procSpecialTimesStaticBox->SetToolTip(_("On each selected \"override\" day, ignore the \"Every day\" times above and suspend if the time is outside the range shown for that day"));

    wxFlexGridSizer* procDaysSizer = new wxFlexGridSizer( 4, 9, 0, 0 );
    procDaysSizer->SetFlexibleDirection( wxHORIZONTAL );
    procDaysSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    // Tooltips for Day-of-Week override wxCheckBoxes and wxTextCtrls
    // are set in CDlgAdvPreferences::SetSpecialTooltips()
    //
    wxString procDaysTimeTT(PROC_DAY_OF_WEEK_TOOLTIP_TEXT);

    m_chkProcMonday = new wxCheckBox(
        procSpecialTimesStaticBox, ID_CHKPROCMONDAY, _("Monday"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    procDaysSizer->Add( m_chkProcMonday, 0, wxTOP, 5 );

    m_txtProcMondayStart = new wxTextCtrl(
        procSpecialTimesStaticBox, ID_TXTPROCMONDAYSTART, wxEmptyString,
        wxDefaultPosition, textCtrlSize, 0
    );
    procDaysSizer->Add( m_txtProcMondayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcMonday = new wxStaticText(
        procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition,
        wxDefaultSize, 0
    );
    toStringProcMonday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcMonday , 0, wxTOP, 5 );

    m_txtProcMondayStop = new wxTextCtrl(
        procSpecialTimesStaticBox, ID_TXTPROCMONDAYSTOP, wxEmptyString,
        wxDefaultPosition, textCtrlSize, 0
    );
    procDaysSizer->Add( m_txtProcMondayStop, 0, wxALL, 1 );

    procDaysSizer->AddSpacer(15);

    m_chkProcFriday = new wxCheckBox(
        procSpecialTimesStaticBox, ID_CHKPROCFRIDAY, _("Friday"),
        wxDefaultPosition, wxDefaultSize, 0
    );
    procDaysSizer->Add( m_chkProcFriday, 0, wxTOP, 5 );

    m_txtProcFridayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCFRIDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcFridayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcFriday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcFriday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcFriday , 0, wxTOP, 5 );

    m_txtProcFridayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCFRIDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcFridayStop, 0, wxALL, 1 );

    m_chkProcTuesday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcTuesday, 0, wxTOP, 5 );

    m_txtProcTuesdayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCTUESDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcTuesdayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcTuesday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcTuesday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcTuesday , 0, wxTOP, 5 );

    m_txtProcTuesdayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCTUESDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcTuesdayStop, 0, wxALL, 1 );

    procDaysSizer->AddSpacer(15);

    m_chkProcSaturday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcSaturday, 0, wxTOP, 5 );

    m_txtProcSaturdayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCSATURDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcSaturdayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcSaturday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcSaturday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcSaturday , 0, wxTOP, 5 );

    m_txtProcSaturdayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCSATURDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcSaturdayStop, 0, wxALL, 1 );

    m_chkProcWednesday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcWednesday, 0, wxTOP, 5 );

    m_txtProcWednesdayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCWEDNESDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcWednesdayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcWednesday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcWednesday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcWednesday , 0, wxTOP, 5 );

    m_txtProcWednesdayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCWEDNESDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcWednesdayStop, 0, wxALL, 1 );

    procDaysSizer->AddSpacer(15);

    m_chkProcSunday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcSunday, 0, wxTOP, 5 );

    m_txtProcSundayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCSUNDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcSundayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcSunday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcSunday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcSunday , 0, wxTOP, 5 );

    m_txtProcSundayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCSUNDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcSundayStop, 0, wxALL, 1 );

    m_chkProcThursday = new wxCheckBox( procSpecialTimesStaticBox, ID_CHKPROCTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_chkProcThursday, 0, wxTOP, 5 );

    m_txtProcThursdayStart = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCTHURSDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcThursdayStart, 0, wxALL, 1 );

    wxStaticText* toStringProcThursday = new wxStaticText( procSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringProcThursday->SetToolTip(procDaysTimeTT);
    procDaysSizer->Add(toStringProcThursday , 0, wxTOP, 5 );

    m_txtProcThursdayStop = new wxTextCtrl( procSpecialTimesStaticBox, ID_TXTPROCTHURSDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    procDaysSizer->Add( m_txtProcThursdayStop, 0, wxALL, 1 );

#ifdef __WXMAC__
    procSpecialTimesStaticBoxSizer->Add(
        procDaysSizer, 0, wxRIGHT | wxBOTTOM, DAYOFWEEKBORDERSIZE
    );
    computingTimesStaticBoxSizer->Add(
        procSpecialTimesStaticBoxSizer, 0, wxRIGHT | wxBOTTOM,
        STATICBOXBORDERSIZE + 3
    );
#else
    procSpecialTimesStaticBoxSizer->Add(
        procDaysSizer, 1, wxRIGHT | wxLEFT | wxBOTTOM, DAYOFWEEKBORDERSIZE
    );
    computingTimesStaticBoxSizer->Add(
        procSpecialTimesStaticBoxSizer, 1, wxRIGHT | wxLEFT | wxBOTTOM,
        STATICBOXBORDERSIZE
    );
#endif
    dailySchedulesTabSizer->AddSpacer( STATICBOXVERTICALSPACER );
    dailySchedulesTabSizer->Add(
        computingTimesStaticBoxSizer, 1, wxRIGHT | wxLEFT | wxEXPAND,
        STATICBOXBORDERSIZE
    );

    // Network schedule
    //
    wxStaticBox* networkTimesStaticBox = new wxStaticBox( dailySchedulesTab, -1, _("Network") );
    wxStaticBoxSizer* networkTimesBoxSizer = new wxStaticBoxSizer( networkTimesStaticBox, wxVERTICAL );
    makeStaticBoxLabelItalic(networkTimesStaticBox);

    wxString NetEveryDayTT(_("Transfer files only during a particular period each day."));
    m_chkNetEveryDay = new wxCheckBox(
        networkTimesStaticBox, ID_CHKNETEVERYDAY, _("Transfer files only between"), wxDefaultPosition, wxDefaultSize, 0 );

    m_txtNetEveryDayStart = new wxTextCtrl( networkTimesStaticBox, ID_TXTNETEVERYDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );

    wxStaticText* staticText37 = new wxStaticText( networkTimesStaticBox, ID_DEFAULT, andString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );

    m_txtNetEveryDayStop = new wxTextCtrl( networkTimesStaticBox, ID_TXTNETEVERYDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );

    addNewRowToSizer(networkTimesBoxSizer, NetEveryDayTT, m_chkNetEveryDay, m_txtNetEveryDayStart, staticText37, m_txtNetEveryDayStop);

    wxStaticBox* netSpecialTimesStaticBox = new wxStaticBox(networkTimesStaticBox, -1, _("Day-of-week override") );
    wxStaticBoxSizer* netSpecialTimesStaticBoxSizer = new wxStaticBoxSizer(netSpecialTimesStaticBox, wxVERTICAL);
    makeStaticBoxLabelItalic(netSpecialTimesStaticBox);

    wxStaticText* staticText39 = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, _("Override the times above on the selected days:"), wxDefaultPosition, wxDefaultSize, 0 );
#ifdef __WXMAC__
    netSpecialTimesStaticBoxSizer->Add( staticText39, 0, wxLEFT, 5 );
#else
    netSpecialTimesStaticBoxSizer->Add( staticText39, 0, wxLEFT, DAYOFWEEKBORDERSIZE );
#endif
    netSpecialTimesStaticBoxSizer->AddSpacer(3);

//    netSpecialTimesStaticBox->SetToolTip(_("On each selected \"override\" day, ignore the \"Every day\" times above and suspend if the time is outside the range shown for that day"));

    // Tooltips for Day-of-Week overrides are set in CDlgAdvPreferences::SetSpecialTooltips()
    wxString netDaysTimeTT(NET_DAY_OF_WEEK_TOOLTIP_TEXT);

    wxFlexGridSizer* netDaysGridSizer = new wxFlexGridSizer( 4, 9, 0, 0 );
    netDaysGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    netDaysGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_chkNetMonday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETMONDAY, _("Monday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetMonday, 0, wxTOP, 5 );

    m_txtNetMondayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETMONDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetMondayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetMonday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetMonday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetMonday , 0, wxTOP, 5 );

    m_txtNetMondayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETMONDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetMondayStop, 0, wxALL, 1 );

    netDaysGridSizer->AddSpacer(15);

    m_chkNetFriday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETFRIDAY, _("Friday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetFriday, 0, wxTOP, 5 );

    m_txtNetFridayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETFRIDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetFridayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetFriday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetFriday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetFriday , 0, wxTOP, 5 );

    m_txtNetFridayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETFRIDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetFridayStop, 0, wxALL, 1 );

    m_chkNetTuesday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetTuesday, 0, wxTOP, 5 );

    m_txtNetTuesdayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETTUESDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetTuesdayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetTuesay = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetTuesay->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetTuesay , 0, wxTOP, 5 );

    m_txtNetTuesdayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETTUESDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetTuesdayStop, 0, wxALL, 1 );

    netDaysGridSizer->AddSpacer(15);

    m_chkNetSaturday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetSaturday, 0, wxTOP, 5 );

    m_txtNetSaturdayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETSATURDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetSaturdayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetSaturday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetSaturday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetSaturday , 0, wxTOP, 5 );

    m_txtNetSaturdayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETSATURDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetSaturdayStop, 0, wxALL, 1 );

    m_chkNetWednesday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetWednesday, 0, wxTOP, 5 );

    m_txtNetWednesdayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETWEDNESDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetWednesdayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetWednesday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetWednesday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetWednesday , 0, wxTOP, 5 );

    m_txtNetWednesdayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETWEDNESDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetWednesdayStop, 0, wxALL, 1 );

    netDaysGridSizer->AddSpacer(15);

    m_chkNetSunday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetSunday, 0, wxTOP, 5 );

    m_txtNetSundayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETSUNDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetSundayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetSunday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetSunday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetSunday , 0, wxTOP, 5 );

    m_txtNetSundayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETSUNDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetSundayStop, 0, wxALL, 1 );

    m_chkNetThursday = new wxCheckBox( netSpecialTimesStaticBox, ID_CHKNETTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_chkNetThursday, 0, wxTOP, 5 );

    m_txtNetThursdayStart = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETTHURSDAYSTART, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetThursdayStart, 0, wxALL, 1 );

    wxStaticText* toStringNetThursday = new wxStaticText( netSpecialTimesStaticBox, ID_DEFAULT, toString, wxDefaultPosition, wxDefaultSize, 0 );
    toStringNetThursday->SetToolTip(netDaysTimeTT);
    netDaysGridSizer->Add(toStringNetThursday , 0, wxTOP, 5 );

    m_txtNetThursdayStop = new wxTextCtrl( netSpecialTimesStaticBox, ID_TXTNETTHURSDAYSTOP, wxEmptyString, wxDefaultPosition, textCtrlSize, 0 );
    netDaysGridSizer->Add( m_txtNetThursdayStop, 0, wxALL, 1 );

#ifdef __WXMAC__
    netSpecialTimesStaticBoxSizer->Add( netDaysGridSizer, 1, wxRIGHT | wxBOTTOM, DAYOFWEEKBORDERSIZE );
    networkTimesBoxSizer->Add(netSpecialTimesStaticBoxSizer, 1, wxRIGHT | wxBOTTOM, STATICBOXBORDERSIZE +3 );
#else
    netSpecialTimesStaticBoxSizer->Add( netDaysGridSizer, 1, wxRIGHT | wxLEFT | wxBOTTOM, DAYOFWEEKBORDERSIZE );
    networkTimesBoxSizer->Add(netSpecialTimesStaticBoxSizer, 1, wxRIGHT | wxLEFT | wxBOTTOM, STATICBOXBORDERSIZE );
#endif
    dailySchedulesTabSizer->AddSpacer( STATICBOXVERTICALSPACER );
    dailySchedulesTabSizer->Add( networkTimesBoxSizer, 1, wxRIGHT | wxLEFT | wxEXPAND, STATICBOXBORDERSIZE );
    dailySchedulesTabSizer->AddSpacer( STATICBOXVERTICALSPACER );

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
    if (retval) {
        web_prefs_url = new wxString(wxEmptyString);
    } else {
        mf.init_buf_read(s.c_str());
        XML_PARSER xp(&mf);
        web_prefs.parse(xp, "", found_venue, mask);
        web_prefs_url = new wxString(web_prefs.source_project);
    }

    return local_prefs_found;
}

void CDlgAdvPreferencesBase::makeStaticBoxLabelItalic(wxStaticBox*
#if defined(__WXMSW__)
        staticBox
#endif
        ) {
#if defined(__WXMSW__)
    wxFont myFont = staticBox->GetFont();
    myFont.MakeItalic();
    myFont.MakeBold();
    staticBox->SetOwnFont(myFont);
#endif
}

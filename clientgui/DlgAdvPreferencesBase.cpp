///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 13 2006)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"

#include "DlgAdvPreferencesBase.h"

///////////////////////////////////////////////////////////////////////////

CDlgAdvPreferencesBase::CDlgAdvPreferencesBase( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) :
    wxDialog( parent, id, title, pos, size, style )
{
    wxString strCaption = title;
    if (strCaption.IsEmpty()) {
        CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
        wxASSERT(pSkinAdvanced);
        wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        strCaption.Printf(_("%s - Preferences"), pSkinAdvanced->GetApplicationName().c_str());
    }

    this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	this->Centre( wxBOTH );
    this->SetTitle(strCaption);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer92;
	sbSizer92 = new wxStaticBoxSizer( new wxStaticBox( this, -1, wxT("") ), wxHORIZONTAL );

	m_bmpWarning = new wxStaticBitmap( this, ID_DEFAULT, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	m_bmpWarning->SetMinSize( wxSize( 48,48 ) );

	sbSizer92->Add( m_bmpWarning, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_staticText321 = new wxStaticText( this, ID_DEFAULT, _("This dialog controls preferences for this computer only.\nClick OK to set preferences.\nClick Clear to restore web-based settings."), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer92->Add( m_staticText321, 1, wxALL, 1 );

	m_btnClear = new wxButton( this, ID_BTN_CLEAR, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btnClear->SetToolTip( _("clear all local preferences and close the dialog") );

	sbSizer92->Add( m_btnClear, 0, wxALIGN_BOTTOM|wxALL, 1 );

	bSizer1->Add( sbSizer92, 0, wxALL|wxEXPAND, 1 );

	m_panelControls = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panelControls->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	m_Notebook = new wxNotebook( m_panelControls, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxNB_FLAT|wxNB_TOP );
	m_Notebook->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

	m_panelProcessor = new wxPanel( m_Notebook, ID_TABPAGE_PROC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panelProcessor->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer(
        new wxStaticBox(m_panelProcessor, -1, _("Computing allowed") ), wxVERTICAL
    );
	m_chkProcOnBatteries = new wxCheckBox(
        m_panelProcessor, ID_CHKPROCONBATTERIES,
        _(" While computer is on batteries"), wxDefaultPosition, wxDefaultSize, 0
    );
	m_chkProcOnBatteries->SetToolTip(
        _("check this if you want this computer to do work while it runs on batteries")
    );
	sbSizer4->Add( m_chkProcOnBatteries, 0, wxALL, 5 );

	m_chkProcInUse = new wxCheckBox(
        m_panelProcessor, ID_CHKPROCINUSE,
        _(" While computer is in use"), wxDefaultPosition, wxDefaultSize, 0
    );
	m_chkProcInUse->SetToolTip(
        _("check this if you want this computer to do work even when you're using it")
    );
	sbSizer4->Add( m_chkProcInUse, 0, wxALL, 5 );

    m_chkGPUProcInUse = new wxCheckBox(
        m_panelProcessor, ID_CHKGPUPROCINUSE,
        _(" Use GPU while computer is in use"), wxDefaultPosition, wxDefaultSize, 0
    );
	m_chkGPUProcInUse->SetToolTip(
        _("check this if you want your GPU to do work even when you're using the computer")
    );
	sbSizer4->Add( m_chkGPUProcInUse, 0, wxALL, 5 );

    // min idle time
	wxFlexGridSizer* fgSizer5 = new wxFlexGridSizer( 2, 4, 0, 0 );
	fgSizer5->AddGrowableCol( 3 );
	fgSizer5->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer5->Add(
        new wxStaticText(
            m_panelProcessor, ID_DEFAULT,
            _("Only after computer has been idle for"),
            wxDefaultPosition, wxDefaultSize, 0
        ),
        0, wxALL, 5
    );
	m_txtProcIdleFor = new wxTextCtrl(
        m_panelProcessor, ID_TXTPROCIDLEFOR, wxT(""), wxDefaultPosition,
        wxSize( 50,-1 ), wxTE_RIGHT
    );
	m_txtProcIdleFor->SetToolTip(
        _("do work only after you haven't used the computer for this number of minutes")
    );
	fgSizer5->Add( m_txtProcIdleFor, 0, wxALL, 1 );
	fgSizer5->Add(
        new wxStaticText(
            m_panelProcessor, ID_DEFAULT, _("minutes"),
            wxDefaultPosition, wxDefaultSize, 0
        ),
        0, wxALL, 5
    );
	fgSizer5->Add(
        new wxStaticText( m_panelProcessor, ID_DEFAULT, wxT(""), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );
	sbSizer4->Add( fgSizer5, 0, wxEXPAND, 5);

    // max CPU load
	wxFlexGridSizer* fgSizer13 = new wxFlexGridSizer( 2, 4, 0, 0 );
	fgSizer13->AddGrowableCol( 3 );
	fgSizer13->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer13->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer13->Add(
        new wxStaticText(
            m_panelProcessor, ID_DEFAULT,
            _("While processor usage is less than"),
            wxDefaultPosition, wxDefaultSize, 0
        ),
        0, wxALL, 5
    );
	m_txtMaxLoad = new wxTextCtrl(
        m_panelProcessor, ID_TXTMAXLOAD, wxT(""), wxDefaultPosition,
        wxSize( 30,-1 ), wxTE_RIGHT
    );
	m_txtMaxLoad->SetToolTip(
        _("suspend work if processor usage exceeds this level")
    );
	fgSizer13->Add( m_txtMaxLoad, 0, wxALL, 1 );
	fgSizer13->Add(
        new wxStaticText(
            m_panelProcessor, ID_DEFAULT, _("percent (0 means no restriction)"),
            wxDefaultPosition, wxDefaultSize, 0
        ),
        0, wxALL, 5
    );
	fgSizer13->Add(
        new wxStaticText( m_panelProcessor, ID_DEFAULT, wxT(""), wxDefaultPosition, wxDefaultSize, 0),
        0, wxALL, 5
    );
	sbSizer4->Add( fgSizer13, 0, wxEXPAND, 5);


	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText351 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("Every day between hours of"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer111->Add( m_staticText351, 0, wxALL, 5 );

	m_txtProcEveryDayStart = new wxTextCtrl( m_panelProcessor, ID_TXTPROCEVERYDAYSTART, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	m_txtProcEveryDayStart->SetToolTip( _("start work at this time") );

	bSizer111->Add( m_txtProcEveryDayStart, 0, wxALL, 1 );

	m_staticText25 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("and"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	bSizer111->Add( m_staticText25, 0, wxALL|wxEXPAND, 5 );

	m_txtProcEveryDayStop = new wxTextCtrl( m_panelProcessor, ID_TXTPROCEVERYDAYSTOP, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	m_txtProcEveryDayStop->SetToolTip( _("stop work at this time") );

	bSizer111->Add( m_txtProcEveryDayStop, 0, wxALL, 1 );

	m_staticText55 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("(no restriction if equal)"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	bSizer111->Add( m_staticText55, 0, wxALL|wxEXPAND, 5 );

	sbSizer4->Add( bSizer111, 0, wxEXPAND, 1 );

	m_staticText36 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("Day-of-week override:"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer4->Add( m_staticText36, 0, wxALL, 5 );

	m_panelProcSpecialTimes = new wxPanel( m_panelProcessor, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	m_panelProcSpecialTimes->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	m_panelProcSpecialTimes->SetToolTip( _("check box to specify hours for this day of week") );

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 4, 4, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_chkProcMonday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCMONDAY, _("Monday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer6->Add( m_chkProcMonday, 0, wxALL, 5 );

	m_txtProcMonday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCMONDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_txtProcMonday, 0, wxALL, 1 );

	m_chkProcTuesday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer6->Add( m_chkProcTuesday, 0, wxALL, 5 );

	m_txtProcTuesday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCTUESDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_txtProcTuesday, 0, wxALL, 1 );

	m_chkProcWednesday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer6->Add( m_chkProcWednesday, 0, wxALL, 5 );

	m_txtProcWednesday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCWEDNESDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_txtProcWednesday, 0, wxALL, 1 );

	m_chkProcThursday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer6->Add( m_chkProcThursday, 0, wxALL, 5 );

	m_txtProcThursday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCTHURSDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_txtProcThursday, 0, wxALL, 1 );

	m_chkProcFriday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCFRIDAY, _("Friday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer6->Add( m_chkProcFriday, 0, wxALL, 5 );

	m_txtProcFriday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCFRIDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_txtProcFriday, 0, wxALL, 1 );

	m_chkProcSaturday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer6->Add( m_chkProcSaturday, 0, wxALL, 5 );

	m_txtProcSaturday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCSATURDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_txtProcSaturday, 0, wxALL, 1 );

	m_chkProcSunday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer6->Add( m_chkProcSunday, 0, wxALL, 5 );

	m_txtProcSunday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCSUNDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_txtProcSunday, 0, wxALL, 1 );

	bSizer11->Add( fgSizer6, 1, wxEXPAND, 1 );

	m_panelProcSpecialTimes->SetSizer( bSizer11 );
	m_panelProcSpecialTimes->Layout();
	bSizer11->Fit( m_panelProcSpecialTimes );
	sbSizer4->Add( m_panelProcSpecialTimes, 1, wxEXPAND | wxALL, 1 );

	bSizer7->Add( sbSizer4, 0, wxEXPAND, 1 );

	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( m_panelProcessor, -1, _("Other options") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 3, 3, 0, 0 );
	fgSizer3->AddGrowableCol( 2 );
	fgSizer3->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText18 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("Switch between applications every"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizer3->Add( m_staticText18, 0, wxALL|wxEXPAND, 5 );

	m_txtProcSwitchEvery = new wxTextCtrl( m_panelProcessor, ID_TXTPROCSWITCHEVERY, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizer3->Add( m_txtProcSwitchEvery, 0, wxALL, 1 );

	m_staticText19 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_staticText19, 0, wxALL, 5 );

	m_staticText20 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("On multiprocessor systems, use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizer3->Add( m_staticText20, 0, wxALL|wxEXPAND, 5 );

	m_txtProcUseProcessors = new wxTextCtrl( m_panelProcessor, ID_TXTPROCUSEPROCESSORS, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizer3->Add( m_txtProcUseProcessors, 0, wxALL, 1 );

    /*xgettext:no-c-format*/ 
    m_staticText21 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("% of the processors"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_staticText21, 0, wxALL, 5 );

	m_staticText22 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizer3->Add( m_staticText22, 0, wxALL|wxEXPAND, 5 );

	m_txtProcUseCPUTime = new wxTextCtrl( m_panelProcessor, ID_TXTPOCUSECPUTIME, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizer3->Add( m_txtProcUseCPUTime, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    m_staticText23 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("% CPU time"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_staticText23, 0, wxALL, 5 );

	sbSizer3->Add( fgSizer3, 0, wxEXPAND, 1 );

	bSizer7->Add( sbSizer3, 0, wxEXPAND, 1 );

	m_panelProcessor->SetSizer( bSizer7 );
	m_panelProcessor->Layout();
	bSizer7->Fit( m_panelProcessor );
	m_Notebook->AddPage( m_panelProcessor, _("processor usage"), false );
	m_panelNetwork = new wxPanel( m_Notebook, ID_TABPAGE_NET, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panelNetwork->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( m_panelNetwork, -1, _("General options") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 3, 6, 0, 0 );
	fgSizer7->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    // upload/download rates

	m_staticText32 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Maximum download rate"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText32, 0, wxALL, 5 );

	m_txtNetDownloadRate = new wxTextCtrl( m_panelNetwork, ID_TXTNETDOWNLOADRATE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizer7->Add( m_txtNetDownloadRate, 0, wxALL, 1 );

	m_staticText33 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("KBytes/sec."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText33, 0, wxALL, 5 );

	m_staticText34 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Maximum upload rate"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText34, 0, wxALIGN_RIGHT|wxALL, 5 );

	m_txtNetUploadRate = new wxTextCtrl( m_panelNetwork, ID_TXTNETUPLOADRATE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizer7->Add( m_txtNetUploadRate, 0, wxALL, 1 );

	m_staticText35 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("KBytes/sec."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText35, 0, wxALL, 5 );

    // long-term quota

	m_staticText_daily_xfer1 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Transfer at most"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText_daily_xfer1, 0, wxALL, 5 );

	m_txt_daily_xfer_limit_mb = new wxTextCtrl( m_panelNetwork, ID_TXTNETDOWNLOADRATE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizer7->Add( m_txt_daily_xfer_limit_mb, 0, wxALL, 1 );

	m_staticText_daily_xfer2 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Mbytes"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText_daily_xfer2, 0, wxALL, 5 );

    m_staticText_daily_xfer3 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("every"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText_daily_xfer3, 0, wxALIGN_RIGHT|wxALL, 5 );

	m_txt_daily_xfer_period_days = new wxTextCtrl( m_panelNetwork, ID_TXTNETUPLOADRATE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizer7->Add( m_txt_daily_xfer_period_days, 0, wxALL, 1 );

	m_staticText_daily_xfer4 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("days"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText_daily_xfer4, 0, wxALL, 5 );


    // buffer sizes

	m_staticText30 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Connect about every"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText30, 0, wxALL, 5 );

	m_txtNetConnectInterval = new wxTextCtrl( m_panelNetwork, ID_TXTNETCONNECTINTERVAL, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	m_txtNetConnectInterval->SetToolTip( _("this computer is connected to the Internet about every X days\n(0 if it's always connected)") );

	fgSizer7->Add( m_txtNetConnectInterval, 0, wxALL, 1 );

	m_staticText31 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("days"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText31, 0, wxALL, 5 );

	m_staticText331 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Additional work buffer"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText331, 0, wxALIGN_RIGHT|wxALL, 5 );

	m_txtNetAdditionalDays = new wxTextCtrl( m_panelNetwork, ID_TXTNETADDITIONALDAYS, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizer7->Add( m_txtNetAdditionalDays, 0, wxALL, 1 );

	m_staticText341 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("days (max. 10)"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_staticText341, 0, wxALL, 5 );

	m_chkNetSkipImageVerification = new wxCheckBox( m_panelNetwork, ID_CHKNETSKIPIMAGEVERIFICATION, _(" Skip image file verification"), wxDefaultPosition, wxDefaultSize, 0 );

	m_chkNetSkipImageVerification->SetToolTip( _("check this if your Internet provider modifies image files") );

	fgSizer7->Add( m_chkNetSkipImageVerification, 0, wxALL, 5 );

	sbSizer8->Add( fgSizer7, 0, wxEXPAND, 1 );

	bSizer12->Add( sbSizer8, 0, wxEXPAND, 1 );

	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer( new wxStaticBox( m_panelNetwork, -1, _("Connect options") ), wxVERTICAL );

	m_chkNetConfirmBeforeConnect = new wxCheckBox( m_panelNetwork, ID_CHKNETCONFIRMBEFORECONNECT, _("Confirm before connecting to internet"), wxDefaultPosition, wxDefaultSize, 0 );

	m_chkNetConfirmBeforeConnect->SetToolTip( _("if checked, a confirmation dialog will be displayed before trying to connect to the Internet") );

	sbSizer7->Add( m_chkNetConfirmBeforeConnect, 0, wxALL, 5 );

	m_chkNetDisconnectWhenDone = new wxCheckBox( m_panelNetwork, ID_CHKNETDISCONNECTWHENDONE, _("Disconnect when done"), wxDefaultPosition, wxDefaultSize, 0 );

	m_chkNetDisconnectWhenDone->SetToolTip( _("if checked, BOINC hangs up when network usage is done\n(only relevant for dialup-connection)") );

	sbSizer7->Add( m_chkNetDisconnectWhenDone, 0, wxALL, 5 );

	bSizer12->Add( sbSizer7, 0, wxEXPAND, 1 );

	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( m_panelNetwork, -1, _("Network usage allowed") ), wxVERTICAL );

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText38 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Every day between hours of"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_staticText38, 0, wxALL, 5 );

	m_txtNetEveryDayStart = new wxTextCtrl( m_panelNetwork, ID_TXTNETEVERYDAYSTART, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_txtNetEveryDayStart->SetToolTip( _("network usage start hour") );

	bSizer14->Add( m_txtNetEveryDayStart, 0, wxALL, 1 );

	m_staticText37 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_staticText37, 0, wxALL, 5 );

	m_txtNetEveryDayStop = new wxTextCtrl( m_panelNetwork, ID_TXTNETEVERYDAYSTOP, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_txtNetEveryDayStop->SetToolTip( _("network usage stop hour") );

	bSizer14->Add( m_txtNetEveryDayStop, 0, wxALL, 1 );

	m_staticText54 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("(no restriction if equal)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_staticText54, 0, wxALL, 5 );

	sbSizer9->Add( bSizer14, 0, wxEXPAND, 1 );

	m_staticText39 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Day-of-week override:"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer9->Add( m_staticText39, 0, wxALL, 5 );

	m_panelNetSpecialTimes = new wxPanel( m_panelNetwork, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	m_panelNetSpecialTimes->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	m_panelNetSpecialTimes->SetToolTip( _("check box to specify hours for this day of week") );

	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 4, 4, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_chkNetMonday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETMONDAY, _("Monday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer8->Add( m_chkNetMonday, 0, wxALL, 5 );

	m_txtNetMonday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETMONDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer8->Add( m_txtNetMonday, 0, wxALL, 1 );

	m_chkNetTuesday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer8->Add( m_chkNetTuesday, 0, wxALL, 5 );

	m_txtNetTuesday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETTUESDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer8->Add( m_txtNetTuesday, 0, wxALL, 1 );

	m_chkNetWednesday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer8->Add( m_chkNetWednesday, 0, wxALL, 5 );

	m_txtNetWednesday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETWEDNESDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer8->Add( m_txtNetWednesday, 0, wxALL, 1 );

	m_chkNetThursday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer8->Add( m_chkNetThursday, 0, wxALL, 5 );

	m_txtNetThursday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETTHURSDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer8->Add( m_txtNetThursday, 0, wxALL, 1 );

	m_chkNetFriday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETFRIDAY, _("Friday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer8->Add( m_chkNetFriday, 0, wxALL, 5 );

	m_txtNetFriday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETFRIDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer8->Add( m_txtNetFriday, 0, wxALL, 1 );

	m_chkNetSaturday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer8->Add( m_chkNetSaturday, 0, wxALL, 5 );

	m_txtNetSaturday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETSATURDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer8->Add( m_txtNetSaturday, 0, wxALL, 1 );

	m_chkNetSunday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer8->Add( m_chkNetSunday, 0, wxALL, 5 );

	m_txtNetSunday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETSUNDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer8->Add( m_txtNetSunday, 0, wxALL, 1 );

	bSizer15->Add( fgSizer8, 0, wxEXPAND, 1 );

	m_panelNetSpecialTimes->SetSizer( bSizer15 );
	m_panelNetSpecialTimes->Layout();
	bSizer15->Fit( m_panelNetSpecialTimes );
	sbSizer9->Add( m_panelNetSpecialTimes, 0, wxEXPAND | wxALL, 1 );

	bSizer12->Add( sbSizer9, 0, wxEXPAND, 1 );

	m_panelNetwork->SetSizer( bSizer12 );
	m_panelNetwork->Layout();
	bSizer12->Fit( m_panelNetwork );
	m_Notebook->AddPage( m_panelNetwork, _("network usage"), true );
	m_panelDiskAndMemory = new wxPanel( m_Notebook, ID_TABPAGE_DISK, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panelDiskAndMemory->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizerDiskUsage;
	sbSizerDiskUsage = new wxStaticBoxSizer( new wxStaticBox( m_panelDiskAndMemory, -1, _("Disk usage") ), wxVERTICAL );

	wxFlexGridSizer* fgSizerDiskUsage;
	fgSizerDiskUsage = new wxFlexGridSizer( 5, 3, 0, 0 );
	fgSizerDiskUsage->AddGrowableCol( 2 );
	fgSizerDiskUsage->SetFlexibleDirection( wxHORIZONTAL );
	fgSizerDiskUsage->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText40 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizerDiskUsage->Add( m_staticText40, 0, wxALL|wxEXPAND, 5 );

	m_txtDiskMaxSpace = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKMAXSPACE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	m_txtDiskMaxSpace->SetToolTip( _("the maximum disk space used by BOINC (in Gigabytes)") );

	fgSizerDiskUsage->Add( m_txtDiskMaxSpace, 0, wxALL, 1 );

	m_staticText41 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Gigabytes disk space"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerDiskUsage->Add( m_staticText41, 0, wxALL, 5 );

	m_staticText42 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Leave at least"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizerDiskUsage->Add( m_staticText42, 0, wxALL|wxEXPAND, 5 );

	m_txtDiskLeastFree = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKLEASTFREE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	m_txtDiskLeastFree->SetToolTip( _("BOINC leaves at least this amount of disk space free (in Gigabytes)") );

	fgSizerDiskUsage->Add( m_txtDiskLeastFree, 0, wxALL, 1 );

	m_staticText43 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Gigabytes disk space free"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerDiskUsage->Add( m_staticText43, 0, wxALL, 5 );

	m_staticText44 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizerDiskUsage->Add( m_staticText44, 0, wxALL|wxEXPAND, 5 );

	m_txtDiskMaxOfTotal = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKMAXOFTOTAL, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	m_txtDiskMaxOfTotal->SetToolTip( _("BOINC uses at most this percentage of total disk space") );

	fgSizerDiskUsage->Add( m_txtDiskMaxOfTotal, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
	m_staticText45 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("% of total disk space"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerDiskUsage->Add( m_staticText45, 0, wxALL, 5 );

	m_staticText46 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Tasks checkpoint to disk at most every"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizerDiskUsage->Add( m_staticText46, 0, wxALL|wxEXPAND, 5 );

	m_txtDiskWriteToDisk = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizerDiskUsage->Add( m_txtDiskWriteToDisk, 0, wxALL, 1 );

	m_staticText47 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerDiskUsage->Add( m_staticText47, 0, wxALL, 5 );

	m_staticText48 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizerDiskUsage->Add( m_staticText48, 0, wxALL|wxEXPAND, 5 );

	m_txtDiskMaxSwap = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizerDiskUsage->Add( m_txtDiskMaxSwap, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
	m_staticText49 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("% of page file (swap space)"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerDiskUsage->Add( m_staticText49, 0, wxALL, 5 );

	sbSizerDiskUsage->Add( fgSizerDiskUsage, 0, wxEXPAND, 1 );

	bSizer25->Add( sbSizerDiskUsage, 0, wxEXPAND, 1 );

	wxStaticBoxSizer* sbSizerMemoryUsage;
	sbSizerMemoryUsage = new wxStaticBoxSizer( new wxStaticBox( m_panelDiskAndMemory, -1, _("Memory usage") ), wxVERTICAL );

	wxFlexGridSizer* fgSizerMemoryUsage;
	fgSizerMemoryUsage = new wxFlexGridSizer( 3, 3, 0, 0 );
	fgSizerMemoryUsage->AddGrowableCol( 2 );
	fgSizerMemoryUsage->SetFlexibleDirection( wxHORIZONTAL );
	fgSizerMemoryUsage->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText50 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizerMemoryUsage->Add( m_staticText50, 0, wxALL|wxEXPAND, 5 );

	m_txtMemoryMaxInUse = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTMEMORYMAXINUSE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizerMemoryUsage->Add( m_txtMemoryMaxInUse, 0, wxALL, 1 );

    /*xgettext:no-c-format*/ 
    m_staticText51 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("% when computer is in use"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerMemoryUsage->Add( m_staticText51, 0, wxALL, 5 );

	m_staticText52 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	fgSizerMemoryUsage->Add( m_staticText52, 0, wxALL|wxEXPAND, 5 );

	m_txtMemoryMaxOnIdle = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTMEMORYMAXONIDLE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
	fgSizerMemoryUsage->Add( m_txtMemoryMaxOnIdle, 0, wxALL, 1 );

    /*xgettext:no-c-format*/ 
	m_staticText53 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("% when computer is idle"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerMemoryUsage->Add( m_staticText53, 0, wxALL, 5 );

	sbSizerMemoryUsage->Add( fgSizerMemoryUsage, 0, wxEXPAND, 1 );

	m_chkMemoryWhileSuspended = new wxCheckBox( m_panelDiskAndMemory, ID_CHKMEMORYWHILESUSPENDED, _(" Leave applications in memory while suspended"), wxDefaultPosition, wxDefaultSize, 0 );

	m_chkMemoryWhileSuspended->SetToolTip( _("if checked, suspended work units are left in memory") );

	sbSizerMemoryUsage->Add( m_chkMemoryWhileSuspended, 0, wxALL, 5 );

	bSizer25->Add( sbSizerMemoryUsage, 0, wxALL|wxEXPAND, 1 );

	m_panelDiskAndMemory->SetSizer( bSizer25 );
	m_panelDiskAndMemory->Layout();
	bSizer25->Fit( m_panelDiskAndMemory );
	m_Notebook->AddPage( m_panelDiskAndMemory, _("disk and memory usage"), false );

	bSizer3->Add( m_Notebook, 1, wxEXPAND | wxALL, 1 );

	m_panelControls->SetSizer( bSizer3 );
	m_panelControls->Layout();
	bSizer3->Fit( m_panelControls );
	bSizer1->Add( m_panelControls, 1, wxALL|wxEXPAND, 1 );

	m_panelButtons = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );

	m_btnOK = new wxButton( m_panelButtons, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btnOK->SetToolTip( _("save all values and close the dialog") );

	bSizer5->Add( m_btnOK, 0, wxALL, 5 );

	m_btnCancel = new wxButton( m_panelButtons, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btnCancel->SetToolTip( _("close the dialog without saving") );

	bSizer5->Add( m_btnCancel, 0, wxALL, 5 );

	m_btnHelp = new wxButton( m_panelButtons, wxID_HELP, _("Help"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btnHelp->SetToolTip( _("shows the preferences web page") );

	bSizer5->Add( m_btnHelp, 0, wxALL, 5 );

	m_panelButtons->SetSizer( bSizer5 );
	m_panelButtons->Layout();
	bSizer5->Fit( m_panelButtons );
	bSizer1->Add( m_panelButtons, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALL, 1 );

    bSizer1->Fit( this );
	this->SetSizer( bSizer1 );
	this->Layout();
}

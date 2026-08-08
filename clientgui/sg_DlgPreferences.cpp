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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "sg_DlgPreferences.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "str_util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "version.h"

#include "sg_CustomControls.h"
#include "sg_DlgPreferences.h"

#include "res/warning.xpm"

#define TEST_BACKGROUND_WITH_MAGENTA_FILL 0

using std::string;


/*!
 * CPanelPreferences type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPanelPreferences, wxPanel )

/*!
 * CPanelPreferences event table definition
 */

BEGIN_EVENT_TABLE( CPanelPreferences, wxPanel )
////@begin CPanelPreferences event table entries
    EVT_COMMAND_RANGE(ID_SG_PREFS_START,ID_SG_PREFS_LAST,
                        wxEVT_COMMAND_CHECKBOX_CLICKED,
                        CPanelPreferences::OnHandleCheckboxEvent
                    )
    EVT_ERASE_BACKGROUND( CPanelPreferences::OnEraseBackground )
    EVT_BUTTON( ID_SIMPLE_HELP, CPanelPreferences::OnButtonHelp )
////@end CPanelPreferences event table entries
END_EVENT_TABLE()

/*!
 * CPanelPreferences constructors
 */

CPanelPreferences::CPanelPreferences( )
{
}


CPanelPreferences::CPanelPreferences( wxWindow* parent ) :
    wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
    Create();
}


CPanelPreferences::~CPanelPreferences( )
{
    if (m_backgroundBitmap) {
        delete m_backgroundBitmap;
        m_backgroundBitmap = NULL;
    }
}


/*!
 * CPanelPreferences creator
 */

bool CPanelPreferences::Create()
{
    m_backgroundBitmap = NULL;
    lastErrorCtrl = NULL;

    stdTextBkgdColor = wxGetApp().GetIsDarkMode() ? *wxBLACK : *wxWHITE;

    CreateControls();

    defaultPrefs.enabled_defaults();

    ReadPreferenceSettings();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    return true;
}


/*!
 * Control creation for CPanelPreferences
 */

void CPanelPreferences::CreateControls()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();
    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    wxSize textCtrlSize = getTextCtrlSize(wxT("999.99"));
    wxSize timeCtrlSize = getTextCtrlSize(wxT("23:59 "));

    CPanelPreferences* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_bUsingLocalPrefs = doesLocalPrefsFileExist();
    if (! web_prefs_url->IsEmpty()) {
        wxStaticBox* topSectionStaticBox = new wxStaticBox();
        topSectionStaticBox->SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
        topSectionStaticBox->Create(this, -1, wxEmptyString);

        wxStaticBoxSizer* topSectionSizer = new wxStaticBoxSizer( topSectionStaticBox, wxVERTICAL );

        wxBoxSizer* topControlsSizer = new wxBoxSizer( wxHORIZONTAL );
        topSectionSizer->Add(topControlsSizer);

        wxBitmap warningBmp = wxBitmap(warning_xpm);
        CTransparentStaticBitmap* bmpWarning = new CTransparentStaticBitmap(
                                    topSectionStaticBox, wxID_ANY,
                                    warningBmp,
                                    wxDefaultPosition, wxDefaultSize, 0
                                    );
        bmpWarning->SetMinSize( warningBmp.GetSize() );

        topControlsSizer->Add( bmpWarning, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

        wxBoxSizer* legendSizer = new wxBoxSizer( wxVERTICAL );

        if (m_bUsingLocalPrefs) {
            legendSizer->Add(
                new CTransparentStaticText( topSectionStaticBox, wxID_ANY,
                            _("Using local preferences.\n"
                            "Click \"Use web prefs\" to use web-based preferences from"
                            ), wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALL, 1
            );
        } else {
            legendSizer->Add(
                new CTransparentStaticText( topSectionStaticBox, wxID_ANY,
                            _("Using web-based preferences from"),
                            wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALL, 1
            );
        }

         legendSizer->Add(
            new CTransparentHyperlinkCtrl(
                topSectionStaticBox, wxID_ANY, *web_prefs_url, *web_prefs_url,
                wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE,
                wxHyperlinkCtrlNameStr, &m_backgroundBitmap
            ),
            0, wxLEFT, 5
        );

        if (!m_bUsingLocalPrefs) {
            legendSizer->Add(
                new CTransparentStaticText( topSectionStaticBox, wxID_ANY,
                     _("Set values and click Save to use local preferences instead."),
                     wxDefaultPosition, wxDefaultSize, 0 ),
                0, wxALL, 1
            );
        }

#if 1
        topSectionSizer->AddSpacer( 10 );

        CTransparentStaticLine* itemStaticLine8 = new CTransparentStaticLine( topSectionStaticBox, wxID_ANY,
                                                                            wxDefaultPosition,
                                                                            wxSize(300, 1),
                                                                             wxLI_HORIZONTAL|wxNO_BORDER
                                                                             );
        itemStaticLine8->SetLineColor(pSkinSimple->GetStaticLineColor());
        topSectionSizer->Add(itemStaticLine8, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 20);

        topSectionSizer->AddSpacer( 10 );

        CTransparentStaticText* itemStaticText7 = new CTransparentStaticText( topSectionStaticBox, wxID_ANY, _("For additional settings, select Computing Preferences in the Advanced View."), wxDefaultPosition, wxDefaultSize, 0 );

        topSectionSizer->Add(itemStaticText7, 0, wxALL, 0);

        topSectionSizer->AddSpacer( 10 );
#endif

        topControlsSizer->Add( legendSizer, 1, wxALL, 1 );

        m_btnClear = new wxButton( topSectionStaticBox, ID_SGPREFERENCESCLEAR, _("Use web prefs"), wxDefaultPosition, wxDefaultSize, 0 );
        m_btnClear->SetToolTip( _("Restore web-based preferences and close the dialog.") );
        if (!m_bUsingLocalPrefs) {
            m_btnClear->Hide();
        }

        topControlsSizer->Add( m_btnClear, 0, wxALIGN_BOTTOM|wxALL, 4 );

#ifdef __WXMAC__
        itemBoxSizer2->Add( topSectionSizer, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 10 );
#else
        itemBoxSizer2->Add( topSectionSizer, 0, wxALL|wxEXPAND, 5 );
#endif
    }

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxLEFT, 20);

    wxString ProcOnBatteriesTT(_("Check this to suspend computing on portables when running on battery power."));

    m_chkProcOnBatteries = new CTransparentCheckBox(
        itemDialog1, ID_CHKPROCONBATTERIES,
        _("Suspend when computer is on battery"),
        wxDefaultPosition, wxDefaultSize, 0,
        wxDefaultValidator,  wxCheckBoxNameStr,
        &m_backgroundBitmap
    );

    m_chkProcOnBatteries->SetToolTip(ProcOnBatteriesTT);

    itemBoxSizer11->Add(m_chkProcOnBatteries, 0, wxALL, 5 );

    wxString ProcInUseTT(_("Check this to suspend computing and file transfers when you're using the computer."));

    m_chkProcInUse = new CTransparentCheckBox(
        itemDialog1, ID_CHKPROCINUSE,
        _("Suspend when computer is in use"),
        wxDefaultPosition, wxDefaultSize, 0,
        wxDefaultValidator,  wxCheckBoxNameStr,
        &m_backgroundBitmap
    );

    m_chkProcInUse->SetToolTip(ProcInUseTT);

    itemBoxSizer11->Add(m_chkProcInUse, 0, wxALL, 5 );

    // min idle time
    wxString ProcIdleForTT(_("This determines when the computer is considered 'in use'."));

    CTransparentStaticText* staticText24 = new CTransparentStaticText(
            itemDialog1, wxID_ANY,
            _("'In use' means mouse/keyboard input in last"),
            wxDefaultPosition, wxDefaultSize, 0
        );

    m_txtProcIdleFor = new wxTextCtrl(
        itemDialog1, ID_TXTPROCIDLEFOR, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT
    );

    CTransparentStaticText* staticText25 = new CTransparentStaticText(itemDialog1, wxID_ANY, _("minutes"),
            wxDefaultPosition, wxDefaultSize, 0 );

    addNewRowToSizer(itemBoxSizer11, ProcIdleForTT, staticText24, m_txtProcIdleFor, staticText25);



    /*xgettext:no-c-format*/
    wxString MaxCPUTimeTT(_("Suspend/resume computing every few seconds to reduce CPU temperature and energy usage. Example: 75% means compute for 3 seconds, wait for 1 second, and repeat."));
    CTransparentStaticText* staticText22 = new CTransparentStaticText(
        itemDialog1, wxID_ANY, _("Use at most"), wxDefaultPosition, wxDefaultSize, 0 );

    m_txtProcUseCPUTime = new wxTextCtrl( itemDialog1, ID_TXTPROCUSECPUTIME, wxEmptyString, wxDefaultPosition, textCtrlSize, wxTE_RIGHT );

    /*xgettext:no-c-format*/
    CTransparentStaticText* staticText23 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("% of CPU time"), wxDefaultPosition, wxDefaultSize, 0 );

    addNewRowToSizer(itemBoxSizer11, MaxCPUTimeTT, staticText22, m_txtProcUseCPUTime, staticText23);

    wxString andString(_("and"));
    wxString ProcEveryDayTT(_("Compute only during a particular period each day."));
    m_chkProcEveryDay = new CTransparentCheckBox(
        itemDialog1, ID_CHKPROCEVERYDAY,
        _("Compute only between"),
        wxDefaultPosition, wxDefaultSize, 0,
        wxDefaultValidator,  wxCheckBoxNameStr,
        &m_backgroundBitmap
    );
    m_txtProcEveryDayStart = new wxTextCtrl(
        itemDialog1, ID_TXTPROCEVERYDAYSTART, wxEmptyString, wxDefaultPosition, timeCtrlSize, wxTE_RIGHT
    );
    CTransparentStaticText* staticText26 = new CTransparentStaticText(
        itemDialog1, wxID_ANY, andString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE
    );
    m_txtProcEveryDayStop = new wxTextCtrl(
        itemDialog1, ID_TXTPROCEVERYDAYSTOP, wxEmptyString, wxDefaultPosition, timeCtrlSize, wxTE_RIGHT
    );
    addNewRowToSizer(
        itemBoxSizer11, ProcEveryDayTT, m_chkProcEveryDay, m_txtProcEveryDayStart, staticText26, m_txtProcEveryDayStop
    );

    wxString NetEveryDayTT(_("Transfer files only during a particular period each day."));
    m_chkNetEveryDay = new CTransparentCheckBox(
        itemDialog1, ID_CHKNETEVERYDAY, _("Transfer files only between"),
        wxDefaultPosition, wxDefaultSize, 0,
        wxDefaultValidator,  wxCheckBoxNameStr,
        &m_backgroundBitmap
    );

    m_txtNetEveryDayStart = new wxTextCtrl( itemDialog1, ID_TXTNETEVERYDAYSTART, wxEmptyString, wxDefaultPosition, timeCtrlSize, 0 );

    CTransparentStaticText* staticText37 = new CTransparentStaticText( itemDialog1, wxID_ANY, andString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );

    m_txtNetEveryDayStop = new wxTextCtrl( itemDialog1, ID_TXTNETEVERYDAYSTOP, wxEmptyString, wxDefaultPosition, timeCtrlSize, 0 );

    addNewRowToSizer(itemBoxSizer11, NetEveryDayTT, m_chkNetEveryDay, m_txtNetEveryDayStart, staticText37, m_txtNetEveryDayStop);


    wxString DiskMaxSpaceTT = wxEmptyString;
    DiskMaxSpaceTT.Printf(_("Limit the total amount of disk space used by %s."), pSkinAdvanced->GetApplicationShortName().c_str());

    m_chkDiskMaxSpace = new CTransparentCheckBox (
        itemDialog1, ID_CHKDISKMAXSPACE, _("Use no more than"),
        wxDefaultPosition, wxDefaultSize, 0,
        wxDefaultValidator,  wxCheckBoxNameStr,
        &m_backgroundBitmap
    );

    m_txtDiskMaxSpace = new wxTextCtrl( itemDialog1, ID_TXTDISKMAXSPACE,wxEmptyString, wxDefaultPosition, getTextCtrlSize(wxT("9999.99")), wxTE_RIGHT );

    CTransparentStaticText* staticText41 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("GB of disk space"), wxDefaultPosition, wxDefaultSize, 0 );

    addNewRowToSizer(itemBoxSizer11, DiskMaxSpaceTT, m_chkDiskMaxSpace, m_txtDiskMaxSpace, staticText41);

    wxBoxSizer* itemBoxSizer44 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer44, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton44 = new wxButton( itemDialog1, wxID_OK, _("&Save"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton44->SetToolTip( _("Save all values and close the dialog") );
    if (m_bUsingLocalPrefs) {
        itemButton44->SetDefault();
    }
    itemBoxSizer44->Add(itemButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton45 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton45->SetToolTip( _("Close the dialog without saving") );
    if (!m_bUsingLocalPrefs) {
        itemButton45->SetDefault();
    }
    itemBoxSizer44->Add(itemButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


#ifndef __WXMSW__
#ifdef __WXMAC__
    wxButton* itemButton46 = new wxButton( this, ID_SIMPLE_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
#ifdef wxUSE_TOOLTIPS
    wxString helpTip;
    helpTip.Printf(_("Get help with %s"), pSkinAdvanced->GetApplicationShortName().c_str());
	itemButton46->SetToolTip(helpTip);
#endif
#else
    wxContextHelpButton* itemButton46 = new wxContextHelpButton(this);
#endif
    itemBoxSizer44->Add(itemButton46, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif

    // Set validators
    m_vTimeValidator = new wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
    m_vTimeValidator->SetCharIncludes(wxT("0123456789:"));

    m_txtProcIdleFor->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcIdleFor->SetMaxLength(16);
    m_txtProcEveryDayStart->SetValidator(*m_vTimeValidator);
    m_txtProcEveryDayStop->SetValidator(*m_vTimeValidator);
    m_txtProcUseCPUTime->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtNetEveryDayStart->SetValidator(*m_vTimeValidator);
    m_txtNetEveryDayStop->SetValidator(*m_vTimeValidator);
    m_txtDiskMaxSpace->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtDiskMaxSpace->SetMaxLength(16);
////@end CPanelPreferences content construction
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SIMPLE_HELP
 */

void CPanelPreferences::OnButtonHelp( wxCommandEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CPanelPreferences::OnHelp - Function Begin"));

    if (IsShown()) {
        wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

        wxString wxurl;
		wxurl.Printf(
            wxT("%s?target=simple_preferences&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            event.GetId()
        );
        wxLaunchDefaultBrowser(wxurl);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CPanelPreferences::OnHelp - Function End"));
}


void CPanelPreferences::MakeBackgroundBitmap() {
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    if (m_backgroundBitmap) {
        delete m_backgroundBitmap;
        m_backgroundBitmap = NULL;
    }

    wxMemoryDC memDC;
    wxCoord w, h, x, y;

    // Get the desired background bitmap
    wxBitmap bmp(*pSkinSimple->GetDialogBackgroundImage()->GetBitmap());

    // Dialog dimensions
    wxSize sz = GetClientSize();

    m_backgroundBitmap = new wxBitmap(sz);
    wxMemoryDC dc(*m_backgroundBitmap);

    // bitmap dimensions
    w = bmp.GetWidth();
    h = bmp.GetHeight();

    // Is the bitmap smaller than the window?
    if ( (w < sz.x) || (h < sz.y) ) {
        // Check to see if they need to be rescaled to fit in the window
        wxImage img = bmp.ConvertToImage();

        if (wxGetApp().GetIsDarkMode()) {
            // Darken the image
            unsigned char *bgImagePixels = img.GetData(); // RGBRGBRGB...
            for (int i=0; i<w; ++i) {
                for (int j=0; j<h; ++j) {
                    for (int k=0; k<3; ++k) {
                        *bgImagePixels /= 4;
                        ++bgImagePixels;
                    }
                }
            }
        }
        img.Rescale((int) sz.x, (int) sz.y);

        // Draw our cool background (enlarged and centered)
        dc.DrawBitmap(wxBitmap(img), 0, 0);
    } else {
        switch(pSkinSimple->GetDialogBackgroundImage()->GetHorizontalAnchor()) {
        case BKGD_ANCHOR_HORIZ_LEFT:
        default:
            x = 0;
            break;
        case BKGD_ANCHOR_HORIZ_CENTER:
            x = (w - sz.x) / 2;
            break;
        case BKGD_ANCHOR_HORIZ_RIGHT:
            x = w - sz.x;
            break;
        }

        switch(pSkinSimple->GetDialogBackgroundImage()->GetVerticalAnchor()) {
        case BKGD_ANCHOR_VERT_TOP:
        default:
            y = 0;
            break;
        case BKGD_ANCHOR_VERT_CENTER:
            y = (h - sz.y) /2;
            break;
        case BKGD_ANCHOR_VERT_BOTTOM:
            y = h - sz.y;
            break;
        }

        // Select the desired bitmap (or its darkened version) into
        //   the memory DC so we can take the desired chunk of it.
        if (wxGetApp().GetIsDarkMode()) {
            // Darken the bitmap
            wxImage bgImage = bmp.ConvertToImage();
            unsigned char *bgImagePixels;
            bgImagePixels = bgImage.GetData(); // RGBRGBRGB...
           for (int i=0; i<w; ++i) {
                for (int j=0; j<h; ++j) {
                    for (int k=0; k<3; ++k) {
                        *bgImagePixels /= 4;
                        ++bgImagePixels;
                    }
                }
            }
            wxBitmap darkened(bgImage);
            memDC.SelectObject(darkened);
        } else {
            memDC.SelectObject(bmp);
        }

        // Draw the desired chunk on the window
        dc.Blit(0, 0, sz.x, sz.y, &memDC, x, y, wxCOPY);

        // Drop the bitmap
        memDC.SelectObject(wxNullBitmap);
    }
}


/*!
 * wxEVT_ERASE_BACKGROUND event handler for ID_DLGPREFERENCES
 */

void CPanelPreferences::OnEraseBackground( wxEraseEvent& event ) {
    if (!m_backgroundBitmap) {
        MakeBackgroundBitmap();
    }
    // Create a buffered device context to reduce flicker
#ifndef __WXGTK__
    wxBufferedDC dc(event.GetDC(), GetClientSize(), wxBUFFER_CLIENT_AREA);
#else
    wxDC &dc = *event.GetDC();
#endif

#if TEST_BACKGROUND_WITH_MAGENTA_FILL
    // Fill the dialog with a magenta color so people can detect when something
    //   is wrong
    wxSize sz = GetClientSize();
    dc.SetBrush(wxBrush(wxColour(255,0,255)));
    dc.SetPen(wxPen(wxColour(255,0,255)));
    dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
#else
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxColour bgColor(*pSkinSimple->GetDialogBackgroundImage()->GetBackgroundColor());
    SetBackgroundColour(bgColor);
#endif

    if (m_backgroundBitmap->IsOk()) {
       dc.DrawBitmap(*m_backgroundBitmap, 0, 0);
    }
}


void CPanelPreferences::OnButtonClear() {
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

#if 1
    // Delete all local prefs (delete global_prefs_override.xml file)
    global_preferences_override_mask.clear();
#else
    // Delete only those settings controlled by this dialog
    ClearPreferenceSettings();
#endif
    pDoc->rpc.set_global_prefs_override_struct(global_preferences_working, global_preferences_override_mask);
    pDoc->rpc.read_global_prefs_override();
}


bool CPanelPreferences::OnOK() {
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if(!ValidateInput()) {
        return false;
    }
    if (!m_bUsingLocalPrefs) {
        if(!ConfirmSetLocal()) {
            return false;
        }
    }
    SavePreferenceSettings();

	pDoc->rpc.set_global_prefs_override_struct(global_preferences_working, global_preferences_override_mask);
	pDoc->rpc.read_global_prefs_override();

    return true;
}


bool CPanelPreferences::UpdateControlStates() {
    m_txtProcIdleFor->Enable(m_chkProcInUse->IsChecked());

    m_txtProcEveryDayStart->Enable(m_chkProcEveryDay->IsChecked());
    m_txtProcEveryDayStop->Enable(m_chkProcEveryDay->IsChecked());

    m_txtNetEveryDayStart->Enable(m_chkNetEveryDay->IsChecked());
    m_txtNetEveryDayStop->Enable(m_chkNetEveryDay->IsChecked());

    m_txtDiskMaxSpace->Enable(m_chkDiskMaxSpace->IsChecked());
    return true;
}


bool CPanelPreferences::ClearPreferenceSettings() {
    global_preferences_override_mask.start_hour = false;
    global_preferences_override_mask.end_hour = false;
    global_preferences_override_mask.net_start_hour = false;
    global_preferences_override_mask.net_end_hour = false;
    global_preferences_override_mask.disk_max_used_gb = false;
    global_preferences_override_mask.cpu_usage_limit = false;
    global_preferences_override_mask.run_if_user_active = false;
    global_preferences_override_mask.run_on_batteries = false;
    global_preferences_override_mask.idle_time_to_run = false;
    return true;
}


// convert a Timestring HH:MM into a double
double CPanelPreferences::TimeStringToDouble(wxString timeStr) {
    double hour;
    double minutes;
    timeStr.SubString(0,timeStr.First(':')).ToDouble(&hour);
    timeStr.SubString(timeStr.First(':')+1,timeStr.Length()).ToDouble(&minutes);
    minutes = minutes/60.0;
    return hour + minutes;
}


// convert a double into a timestring HH:MM
wxString CPanelPreferences::DoubleToTimeString(double dt) {
    int hour = (int)dt;
    int minutes = (int)(60.0 * (dt - hour)+.5);
    return wxString::Format(wxT("%02d:%02d"),hour,minutes);
}


// We only display 2 places past the decimal, so restrict the
// precision of saved values to .01.  This prevents unexpected
// behavior when, for example, a zero value means no restriction
// and the value is displayed as 0.00 but is actually 0.001.
//
double CPanelPreferences::RoundToHundredths(double td) {
    int64_t i = (int64_t)((td + .005) * 100.);
    return ((double)(i) / 100.);
}


void CPanelPreferences::DisplayValue(double value, wxTextCtrl* textCtrl, wxCheckBox* checkBox) {
    wxString buffer;

    wxASSERT(textCtrl);

    if (checkBox) {
        if (! checkBox->IsChecked()) {
            textCtrl->Clear();
            textCtrl->Disable();
            return;
        }
    }
    buffer.Printf(wxT("%.2f"), value);
    textCtrl->ChangeValue(buffer);
    textCtrl->Enable();
}


/* read preferences from core client and initialize control values */
bool CPanelPreferences::ReadPreferenceSettings() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    int            retval;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    // Populate values and arrays from preferences
    // Get override mask from client
    retval = pDoc->rpc.get_global_prefs_override_struct(global_preferences_working, global_preferences_override_mask);
    // Get current working preferences (including any overrides) from client
    retval = pDoc->rpc.get_global_prefs_working_struct(global_preferences_working, global_preferences_mask);
    if (retval == ERR_NOT_FOUND) {
        // Older clients don't support get_global_prefs_working_struct RPC
        global_preferences_working = pDoc->state.global_prefs;
        retval = pDoc->rpc.get_global_prefs_override_struct(global_preferences_working, global_preferences_mask);
    }
    if (retval) {
        m_bOKToShow = false;
        return true;
    }

    m_bOKToShow = true;

#if 0   // We might use this to tell user whether local prefs exist
    if (!retval && global_preferences_override_mask.are_simple_prefs_set()) {
        m_bCustomizedPreferences = true;
    } else {
        m_bCustomizedPreferences = false;
    }
#endif

    // on batteries
    m_chkProcOnBatteries->SetValue(! global_preferences_working.run_on_batteries);

    // in use
    m_chkProcInUse->SetValue(! global_preferences_working.run_if_user_active);

    if (m_chkProcInUse->IsChecked()) {
        m_txtProcIdleFor->Enable();
        DisplayValue(global_preferences_working.idle_time_to_run, m_txtProcIdleFor);
    } else {
        m_txtProcIdleFor->Clear();
        m_txtProcIdleFor->Disable();
    }

    // do work between
    m_chkProcEveryDay->SetValue(global_preferences_working.cpu_times.start_hour != global_preferences_working.cpu_times.end_hour);
    if (m_chkProcEveryDay->IsChecked()) {
        m_txtProcEveryDayStart->ChangeValue(DoubleToTimeString(global_preferences_working.cpu_times.start_hour));
        m_txtProcEveryDayStop->ChangeValue(DoubleToTimeString(global_preferences_working.cpu_times.end_hour));
    }

    //cpu limit
    // 0 means "no restriction" but we don't use a checkbox here
    if (global_preferences_working.cpu_usage_limit == 0.0) global_preferences_working.cpu_usage_limit = 100.0;
    DisplayValue(global_preferences_working.cpu_usage_limit, m_txtProcUseCPUTime);

    // use network between
    m_chkNetEveryDay->SetValue(global_preferences_working.net_times.start_hour != global_preferences_working.net_times.end_hour);
    if (m_chkNetEveryDay->IsChecked()) {
        m_txtNetEveryDayStart->ChangeValue(DoubleToTimeString(global_preferences_working.net_times.start_hour));
        m_txtNetEveryDayStop->ChangeValue(DoubleToTimeString(global_preferences_working.net_times.end_hour));
    }

    //max disk space used
    m_chkDiskMaxSpace->SetValue(global_preferences_working.disk_max_used_gb > 0.0);
    DisplayValue(global_preferences_working.disk_max_used_gb, m_txtDiskMaxSpace, m_chkDiskMaxSpace);

    // Now make sure the UI is in sync with the settings
    UpdateControlStates();

    return true;
}


/* write overridden preferences to disk (global_prefs_override.xml) */
/* IMPORTANT: Any items added here must be checked in ValidateInput()! */
bool CPanelPreferences::SavePreferenceSettings() {
    double td;

    // on batteries
    global_preferences_working.run_on_batteries = ! (m_chkProcOnBatteries->GetValue());
    global_preferences_override_mask.run_on_batteries=true;

    // in use
    global_preferences_working.run_if_user_active = (! m_chkProcInUse->GetValue());
    global_preferences_override_mask.run_if_user_active=true;

    if(m_txtProcIdleFor->IsEnabled()) {
        m_txtProcIdleFor->GetValue().ToDouble(&td);
        global_preferences_working.idle_time_to_run=RoundToHundredths(td);
        global_preferences_override_mask.idle_time_to_run=true;
    }
    // else leave idle_time_to_run value and mask unchanged (in case run_gpu_if_user_active is false)

    // do work between
    if (m_chkProcEveryDay->IsChecked()) {
        global_preferences_working.cpu_times.start_hour = TimeStringToDouble(m_txtProcEveryDayStart->GetValue());
        global_preferences_working.cpu_times.end_hour = TimeStringToDouble(m_txtProcEveryDayStop->GetValue());
    } else {
        global_preferences_working.cpu_times.start_hour = global_preferences_working.cpu_times.end_hour = 0.0;
    }
    global_preferences_override_mask.start_hour = global_preferences_override_mask.end_hour = true;


    //cpu limit
    m_txtProcUseCPUTime->GetValue().ToDouble(&td);
    global_preferences_working.cpu_usage_limit=RoundToHundredths(td);
    global_preferences_override_mask.cpu_usage_limit=true;

    if (m_chkDiskMaxSpace->IsChecked()) {
        m_txtDiskMaxSpace->GetValue().ToDouble(&td);
        global_preferences_working.disk_max_used_gb=RoundToHundredths(td);
    } else {
        global_preferences_working.disk_max_used_gb = 0.0;
    }
    global_preferences_override_mask.disk_max_used_gb=true;

    // use network between
    if (m_chkNetEveryDay->IsChecked()) {
        global_preferences_working.net_times.start_hour = TimeStringToDouble(m_txtNetEveryDayStart->GetValue());
        global_preferences_working.net_times.end_hour = TimeStringToDouble(m_txtNetEveryDayStop->GetValue());
    } else {
        global_preferences_working.net_times.start_hour = global_preferences_working.net_times.end_hour = 0.0;
    }
    global_preferences_override_mask.net_start_hour = global_preferences_override_mask.net_end_hour = true;

    //max disk space used
    // BOINC uses the most restrictive of these 3 settings:
    // disk_max_used_gb, disk_min_free_gb, disk_max_used_pct.
    // Since the Simple Prefs dialog allows the user to set only
    // disk_max_used_gb, we set the other two to "no restriction"
    global_preferences_working.disk_min_free_gb = 0.0;
    global_preferences_override_mask.disk_min_free_gb=true;
    global_preferences_working.disk_max_used_pct = 100.0;
    global_preferences_override_mask.disk_max_used_pct=true;

    return true;
}


/* validates the entered informations */
bool CPanelPreferences::ValidateInput() {
    wxString invMsgFloat = _("Invalid number");
    wxString invMsgTime = _("Invalid time, value must be between 0:00 and 24:00, format is HH:MM");
    wxString invMsgTimeSpan = _("Start time must be different from end time");
    wxString invMsgLimit100 = _("Number must be between 0 and 100");
    double startTime, endTime;
    wxString buffer;

    if(m_txtProcIdleFor->IsEnabled()) {
        buffer = m_txtProcIdleFor->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
            ShowErrorMessage(invMsgFloat,m_txtProcIdleFor);
            return false;
        }
    }

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

    buffer = m_txtProcUseCPUTime->GetValue();
    if(!IsValidFloatValueBetween(buffer, 0.0, 100.0)) {
        ShowErrorMessage(invMsgLimit100, m_txtProcUseCPUTime);
        return false;
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

    if (m_chkDiskMaxSpace->IsChecked()) {
        buffer = m_txtDiskMaxSpace->GetValue();
        if(!IsValidFloatValueBetween(buffer, 0, 9999999999999.99)) {
            ShowErrorMessage(invMsgFloat, m_txtDiskMaxSpace);
            return false;
        }
    }

    return true;
}


/* show an error message and set the focus to the control that caused the error */
void CPanelPreferences::ShowErrorMessage(wxString& message,wxTextCtrl* errorCtrl) {
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
bool CPanelPreferences::IsValidFloatChar(const wxChar& ch) {
    //don't accept the e
    return wxIsdigit(ch) || ch=='.' || ch==',' || ch=='+' || ch=='-';}

/* checks if ch is a valid character for time values */
bool CPanelPreferences::IsValidTimeChar(const wxChar& ch) {
    return wxIsdigit(ch) || ch==':';
}


/* checks if the value contains a valid float */
bool CPanelPreferences::IsValidFloatValue(const wxString& value, bool allowNegative) {
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


bool CPanelPreferences::IsValidFloatValueBetween(const wxString& value, double minVal, double maxVal){
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
bool CPanelPreferences::IsValidTimeValue(const wxString& value) {
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


void CPanelPreferences::OnHandleCheckboxEvent(wxCommandEvent& ev) {
    ev.Skip();
    // If user has just set the checkbox, set textedit field to default value.
    // Note: use ChangeValue() here to avoid generating extra events.
    // m_txtProcIdleFor depends on 2 checkboxes, set it in UpdateControlStates().
    switch (ev.GetId()) {
    case ID_CHKPROCINUSE:
        DisplayValue(defaultPrefs.idle_time_to_run, m_txtProcIdleFor, m_chkProcInUse);
        break;
    case ID_CHKDISKMAXSPACE:
        DisplayValue(defaultPrefs.disk_max_used_gb, m_txtDiskMaxSpace, m_chkDiskMaxSpace);
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
    case ID_CHKNETEVERYDAY:
       if (ev.IsChecked()) {
            m_txtNetEveryDayStart->ChangeValue(DoubleToTimeString(defaultPrefs.net_times.start_hour));
            m_txtNetEveryDayStop->ChangeValue(DoubleToTimeString(defaultPrefs.net_times.end_hour));
        } else {
            m_txtNetEveryDayStart->Clear();
            m_txtNetEveryDayStop->Clear();
        }
        break;




    default:
        break;
    }
    UpdateControlStates();
}


void CPanelPreferences::addNewRowToSizer(
                wxSizer* toSizer, wxString& toolTipText,
                wxWindow* first, wxWindow* second, wxWindow* third,
                wxWindow* fourth, wxWindow* fifth)
{
    wxBoxSizer* rowSizer = new wxBoxSizer( wxHORIZONTAL );

#ifdef __WXMSW__
    // MSW adds space to the right of checkbox label
    if (first->IsKindOf(CLASSINFO(CTransparentCheckBox))) {
        rowSizer->Add(first, 0, wxTOP | wxBOTTOM |wxLEFT, 5 );
    } else
#endif
        rowSizer->Add(first, 0, wxALL, 5 );

    first->SetToolTip(toolTipText);

    rowSizer->Add(second, 0, wxALL, 2 );
    second->SetToolTip(toolTipText);

    rowSizer->Add(third, 0, wxALL, 5 );
    third->SetToolTip(toolTipText);

    if (fourth) {
        rowSizer->Add(fourth, 0, wxALL, 2 );
        fourth->SetToolTip(toolTipText);
    }

    if (fifth) {
        rowSizer->Add(fifth, 0, wxALL, 5 );
        fifth->SetToolTip(toolTipText);
    }

    toSizer->Add( rowSizer, 0, 0, 1 );
}


wxSize CPanelPreferences::getTextCtrlSize(wxString maxText) {
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


bool CPanelPreferences::doesLocalPrefsFileExist() {
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


/*!
 * CDlgPreferences type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgPreferences, wxDialog )

/*!
 * CDlgPreferences event table definition
 */

BEGIN_EVENT_TABLE( CDlgPreferences, wxDialog )
////@begin CDlgPreferences event table entries
    EVT_HELP(wxID_ANY, CDlgPreferences::OnHelp)
    EVT_BUTTON( ID_SGPREFERENCESCLEAR, CDlgPreferences::OnButtonClear )
    EVT_BUTTON( wxID_OK, CDlgPreferences::OnOK )
////@end CDlgPreferences event table entries
END_EVENT_TABLE()

/*!
 * CDlgPreferences constructors
 */

CDlgPreferences::CDlgPreferences( )
{
}


CDlgPreferences::CDlgPreferences( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CDlgPreferences creator
 */

bool CDlgPreferences::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    SetExtraStyle(GetExtraStyle()|wxDIALOG_EX_CONTEXTHELP|wxWS_EX_BLOCK_EVENTS);

    wxDialog::Create( parent, id, caption, pos, size, style );

    // Initialize Application Title
    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        strCaption.Printf(_("%s - Computing Preferences"), pSkinAdvanced->GetApplicationShortName().c_str());
    }
    SetTitle(strCaption);

    // Initialize Application Icon
    SetIcons(*pSkinAdvanced->GetApplicationIcon());

    Freeze();

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    SetForegroundColour(*wxBLACK);
#if TEST_BACKGROUND_WITH_MAGENTA_FILL
    SetBackgroundColour(wxColour(255, 0, 255));
#endif

    wxBoxSizer* dialogSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer(dialogSizer);
    m_pBackgroundPanel = new CPanelPreferences(this);
    dialogSizer->Add(m_pBackgroundPanel, 0, wxGROW, 0);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    SetEscapeId(wxID_CANCEL);

    Thaw();

    return true;
}


/*!
 * wxEVT_HELP event handler for ID_DLGPREFERENCES
 */

void CDlgPreferences::OnHelp(wxHelpEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CDlgPreferences::OnHelp - Function Begin"));

    if (IsShown()) {
    	wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

		wxString wxurl;
		wxurl.Printf(
            wxT("%s?target=simple_preferences&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            event.GetId()
        );
        wxLaunchDefaultBrowser(wxurl);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CDlgPreferences::OnHelp - Function End"));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SGPREFERENCESCLEAR
 */

void CDlgPreferences::OnButtonClear( wxCommandEvent& WXUNUSED(event) ) {
	if(ConfirmClear()) {
        m_pBackgroundPanel->OnButtonClear();
        EndModal(wxID_OK);
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CDlgPreferences::OnOK( wxCommandEvent& WXUNUSED(event) ) {
    if (m_pBackgroundPanel->OnOK()) {
        EndModal(wxID_OK);
    }
}


bool CDlgPreferences::ConfirmClear() {
	int res = wxGetApp().SafeMessageBox(_(
        "Discard all local preferences and use web-based preferences?"),
		_("Confirmation"),wxCENTER | wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT, this);

	return res==wxYES;
}

bool CPanelPreferences::ConfirmSetLocal() {
    wxString strMessage     = wxEmptyString;
    strMessage.Printf(
            _("Changing to use the local preferences defined on this page. This will override your web-based preferences, even if you subsequently make changes there. Do you want to proceed?")
    );
    int res = wxGetApp().SafeMessageBox(
        strMessage,
        _("Confirmation"),wxCENTER | wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT,this);

    return res==wxYES;
}

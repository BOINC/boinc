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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "sg_DlgPreferences.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "sg_CustomControls.h"
#include "sg_DlgPreferences.h"

#ifdef __WXMAC__
#define TINY_FONT 12
#define SMALL_FONT 12
#define MEDIUM_FONT 14
#define LARGE_FONT 16
#else
#define TINY_FONT 8
#define SMALL_FONT 9
#define MEDIUM_FONT 12
#define LARGE_FONT 16
#endif


////@begin includes
////@end includes

////@begin XPM images
////@end XPM images

// Useful arrays used as templates for arrays created at runtime.
//
int iTimeOfDayArraySize = 24;
wxString astrTimeOfDayStrings[] = {
    wxT("0:00"),
    wxT("1:00"),
    wxT("2:00"),
    wxT("3:00"),
    wxT("4:00"),
    wxT("5:00"),
    wxT("6:00"),
    wxT("7:00"),
    wxT("8:00"),
    wxT("9:00"),
    wxT("10:00"),
    wxT("11:00"),
    wxT("12:00"),
    wxT("13:00"),
    wxT("14:00"),
    wxT("15:00"),
    wxT("16:00"),
    wxT("17:00"),
    wxT("18:00"),
    wxT("19:00"),
    wxT("20:00"),
    wxT("21:00"),
    wxT("22:00"),
    wxT("23:00"),
    wxT("24:00")
};


int iDiskUsageArraySize = 10;
wxString astrDiskUsageStrings[] = {
    _("100 MB"),
    _("200 MB"),
    _("500 MB"),
    _("1 GB"),
    _("2 GB"),
    _("5 GB"),
    _("10 GB"),
    _("20 GB"),
    _("50 GB"),
    _("100 GB")
};

// Used for sorting disk usage values
static int CompareDiskUsage(const wxString& strFirst, const wxString& strSecond) {
    long lFirstValue;
    long lSecondValue;

    // Is first measured in GB and second measured in MB?
    if ((strFirst.Find(wxT("GB")) != -1) && (strSecond.Find(wxT("MB")) != -1)) return 1;

    // Is first measured in MB and second measured in GB?
    if ((strFirst.Find(wxT("MB")) != -1) && (strSecond.Find(wxT("GB")) != -1)) return -1;

    // Convert to numbers
    strFirst.ToLong(&lFirstValue);
    strSecond.ToLong(&lSecondValue);

    // Is lFirstValue larger than lSecondValue?
    if (lFirstValue > lSecondValue) return 1;

    // Is lFirstValue less than lSecondValue?
    if (lFirstValue < lSecondValue) return -1;

    // they must be equal
    return 0;
}


int iCPUUsageArraySize = 10;
wxString astrCPUUsageStrings[] = {
    _("10%"),
    _("20%"),
    _("30%"),
    _("40%"),
    _("50%"),
    _("60%"),
    _("70%"),
    _("80%"),
    _("90%"),
    _("100%")
};

// Used for sorting cpu usage values
static int CompareCPUUsage(const wxString& strFirst, const wxString& strSecond) {
    long lFirstValue;
    long lSecondValue;

    // Convert to numbers
    strFirst.ToLong(&lFirstValue);
    strSecond.ToLong(&lSecondValue);

    // Is lFirstValue larger than lSecondValue?
    if (lFirstValue > lSecondValue) return 1;

    // Is lFirstValue less than lSecondValue?
    if (lFirstValue < lSecondValue) return -1;

    // they must be equal
    return 0;
}


int iWorkWhenIdleArraySize = 7;
wxString astrWorkWhenIdleStrings[] = {
    _("1"),
    _("3"),
    _("5"),
    _("10"),
    _("15"),
    _("30"),
    _("60")
};

// Used for sorting work when idle values
static int CompareWorkWhenIdle(const wxString& strFirst, const wxString& strSecond) {
    long lFirstValue;
    long lSecondValue;

    // Convert to numbers
    strFirst.ToLong(&lFirstValue);
    strSecond.ToLong(&lSecondValue);

    // Is lFirstValue larger than lSecondValue?
    if (lFirstValue > lSecondValue) return 1;

    // Is lFirstValue less than lSecondValue?
    if (lFirstValue < lSecondValue) return -1;

    // they must be equal
    return 0;
}


/*!
 * CPanelPreferences type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPanelPreferences, wxPanel )

/*!
 * CPanelPreferences event table definition
 */

BEGIN_EVENT_TABLE( CPanelPreferences, wxPanel )
////@begin CPanelPreferences event table entries
    EVT_ERASE_BACKGROUND( CPanelPreferences::OnEraseBackground )
    EVT_CHECKBOX( ID_CUSTOMIZEPREFERENCES, CPanelPreferences::OnCustomizePreferencesClick )
    EVT_COMBOBOX( ID_WORKBETWEENBEGIN, CPanelPreferences::OnWorkBetweenBeginSelected )
    EVT_COMBOBOX( ID_CONNECTBETWEENBEGIN, CPanelPreferences::OnConnectBetweenBeginSelected )
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


/*!
 * CPanelPreferences creator
 */

bool CPanelPreferences::Create()
{
////@begin CPanelPreferences member initialisation
    m_SkinSelectorCtrl = NULL;
    m_CustomizePreferencesCtrl = NULL;
    m_WorkBetweenBeginCtrl = NULL;
    m_WorkBetweenEndCtrl = NULL;
    m_ConnectBetweenBeginCtrl = NULL;
    m_ConnectBetweenEndCtrl = NULL;
    m_MaxDiskUsageCtrl = NULL;
    m_MaxCPUUsageCtrl = NULL;
    m_WorkWhileInUseCtrl = NULL;
    m_WorkWhileOnBatteryCtrl = NULL;
    m_WorkWhenIdleCtrl = NULL;
////@end CPanelPreferences member initialisation

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    ReadPreferenceSettings();
    ReadSkinSettings();

    return true;
}


/*!
 * Control creation for CPanelPreferences
 */

void CPanelPreferences::CreateControls()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    CPanelPreferences* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(1, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxGROW|wxALL, 5);

    CTransparentStaticText* itemStaticText4 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Skin"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText4->SetFont(wxFont(LARGE_FONT, wxSWISS, wxNORMAL, wxBOLD, false, _T("Arial")));
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_LEFT|wxLEFT|wxBOTTOM, 20);

    CTransparentStaticText* itemStaticText6 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Skin:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText6->SetFont(wxFont(TINY_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxString* m_SkinSelectorCtrlStrings = NULL;
    m_SkinSelectorCtrl = new wxComboBox( itemDialog1, ID_SKINSELECTOR, _T(""), wxDefaultPosition, wxSize(175, -1), 0, m_SkinSelectorCtrlStrings, wxCB_READONLY );
    itemBoxSizer5->Add(m_SkinSelectorCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    CTransparentStaticLine* itemStaticLine8 = new CTransparentStaticLine( itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(300, 1), wxLI_HORIZONTAL|wxNO_BORDER );
    itemStaticLine8->SetLineColor(pSkinSimple->GetStaticLineColor());
    itemBoxSizer2->Add(itemStaticLine8, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 20);

    wxFlexGridSizer* itemFlexGridSizer9 = new wxFlexGridSizer(1, 1, 0, 0);
    itemFlexGridSizer9->AddGrowableCol(0);
    itemBoxSizer2->Add(itemFlexGridSizer9, 0, wxGROW|wxALL, 5);

    CTransparentStaticText* itemStaticText10 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    itemStaticText10->SetFont(wxFont(LARGE_FONT, wxSWISS, wxNORMAL, wxBOLD, false, _T("Arial")));
    itemFlexGridSizer9->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT, 20);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer11->Add(itemBoxSizer12, 0, wxALIGN_LEFT|wxALL, 0);

    m_CustomizePreferencesCtrl = new wxCheckBox( itemDialog1, ID_CUSTOMIZEPREFERENCES, _(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_CustomizePreferencesCtrl->SetValue(false);
    itemBoxSizer12->Add(m_CustomizePreferencesCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    CTransparentStaticTextAssociate* itemStaticText14 = new CTransparentStaticTextAssociate( 
        itemDialog1, 
        wxID_ANY, 
        _("I want to customize my preferences for this computer only."), 
        wxDefaultPosition, 
        wxDefaultSize, 
        0
    );
    itemStaticText14->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText14->AssociateWindow(m_CustomizePreferencesCtrl);
    itemBoxSizer12->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText15 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Customized Preferences"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText15->SetFont(wxFont(MEDIUM_FONT, wxSWISS, wxNORMAL, wxBOLD, false, _T("Arial")));
    itemBoxSizer11->Add(itemStaticText15, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    wxFlexGridSizer* itemFlexGridSizer15 = new wxFlexGridSizer(7, 2, 0, 0);
    itemFlexGridSizer15->AddGrowableRow(0);
    itemFlexGridSizer15->AddGrowableRow(1);
    itemFlexGridSizer15->AddGrowableRow(2);
    itemFlexGridSizer15->AddGrowableRow(3);
    itemFlexGridSizer15->AddGrowableRow(4);
    itemFlexGridSizer15->AddGrowableRow(5);
    itemFlexGridSizer15->AddGrowableRow(6);
    itemFlexGridSizer15->AddGrowableCol(0);
    itemFlexGridSizer15->AddGrowableCol(1);
    itemBoxSizer11->Add(itemFlexGridSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    CTransparentStaticText* itemStaticText16 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Do work only between:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText16->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText16->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_WorkBetweenBeginCtrlStrings = NULL;
    m_WorkBetweenBeginCtrl = new wxComboBox( itemDialog1, ID_WORKBETWEENBEGIN, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_WorkBetweenBeginCtrlStrings, wxCB_READONLY );
    m_WorkBetweenBeginCtrl->Enable(false);
    itemBoxSizer17->Add(m_WorkBetweenBeginCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText19 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText19->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer17->Add(itemStaticText19, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString* m_WorkBetweenEndCtrlStrings = NULL;
    m_WorkBetweenEndCtrl = new wxComboBox( itemDialog1, ID_WORKBETWEENEND, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_WorkBetweenEndCtrlStrings, wxCB_READONLY );
    m_WorkBetweenEndCtrl->Enable(false);
    itemBoxSizer17->Add(m_WorkBetweenEndCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText21 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Connect to internet only between:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText21->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText21->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText21, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer22, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_ConnectBetweenBeginCtrlStrings = NULL;
    m_ConnectBetweenBeginCtrl = new wxComboBox( itemDialog1, ID_CONNECTBETWEENBEGIN, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_ConnectBetweenBeginCtrlStrings, wxCB_READONLY );
    m_ConnectBetweenBeginCtrl->Enable(false);
    itemBoxSizer22->Add(m_ConnectBetweenBeginCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText24 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText24->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer22->Add(itemStaticText24, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString* m_ConnectBetweenEndCtrlStrings = NULL;
    m_ConnectBetweenEndCtrl = new wxComboBox( itemDialog1, ID_CONNECTBETWEENEND, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_ConnectBetweenEndCtrlStrings, wxCB_READONLY );
    m_ConnectBetweenEndCtrl->Enable(false);
    itemBoxSizer22->Add(m_ConnectBetweenEndCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText26 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Use no more than:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText26->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText26->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_MaxDiskUsageCtrlStrings = NULL;
    m_MaxDiskUsageCtrl = new wxComboBox( itemDialog1, ID_MAXDISKUSAGE, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_MaxDiskUsageCtrlStrings, wxCB_READONLY );
    m_MaxDiskUsageCtrl->Enable(false);
    itemBoxSizer27->Add(m_MaxDiskUsageCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText29 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("of disk space"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText29->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer27->Add(itemStaticText29, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    CTransparentStaticText* itemStaticText30 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Use no more than:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText30->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText30->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText30, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer31 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer31, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_MaxCPUUsageCtrlStrings = NULL;
    m_MaxCPUUsageCtrl = new wxComboBox( itemDialog1, ID_MAXCPUUSAGE, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_MaxCPUUsageCtrlStrings, wxCB_READONLY );
    m_MaxCPUUsageCtrl->Enable(false);
    itemBoxSizer31->Add(m_MaxCPUUsageCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText33 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("of the processor"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText33->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer31->Add(itemStaticText33, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    CTransparentStaticText* itemStaticText34 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Do work while in use?"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText34->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText34->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText34, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer35 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer35, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_WorkWhileInUseCtrl = new wxCheckBox( itemDialog1, ID_WORKWHILEINUSE, _T(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_WorkWhileInUseCtrl->SetValue(false);
    m_WorkWhileInUseCtrl->Enable(false);
    itemBoxSizer35->Add(m_WorkWhileInUseCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText37 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Do work while on battery?"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText37->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText37->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText37, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer38 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer38, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_WorkWhileOnBatteryCtrl = new wxCheckBox( itemDialog1, ID_WORKWHILEONBATTERY, _T(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_WorkWhileOnBatteryCtrl->SetValue(false);
    m_WorkWhileOnBatteryCtrl->Enable(false);
    itemBoxSizer38->Add(m_WorkWhileOnBatteryCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText40 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Do work after idle for:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText40->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText40->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText40, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer41, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_WorkWhenIdleCtrlStrings = NULL;
    m_WorkWhenIdleCtrl = new wxComboBox( itemDialog1, ID_WORKWHENIDLE, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_WorkWhenIdleCtrlStrings, wxCB_READONLY );
    m_WorkWhenIdleCtrl->Enable(false);
    itemBoxSizer41->Add(m_WorkWhenIdleCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText43 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText43->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer41->Add(itemStaticText43, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer44 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer44, 0, wxALIGN_RIGHT|wxALL, 5);

    wxBitmapButton* itemBitmapButton44 = new wxBitmapButton( itemDialog1, wxID_OK, *pSkinSimple->GetSaveButton()->GetBitmap(), wxPoint(120,340),wxSize(57,16), wxBU_AUTODRAW );
	if ( pSkinSimple->GetSaveButton()->GetBitmapClicked() != NULL ) {
		itemBitmapButton44->SetBitmapSelected(*pSkinSimple->GetSaveButton()->GetBitmapClicked());
	}
    itemBoxSizer44->Add(itemBitmapButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton45 = new wxBitmapButton( itemDialog1, wxID_CANCEL, *pSkinSimple->GetCancelButton()->GetBitmap(), wxPoint(187,340),wxSize(57,16), wxBU_AUTODRAW );
	if ( pSkinSimple->GetCancelButton()->GetBitmapClicked() != NULL ) {
		itemBitmapButton45->SetBitmapSelected(*pSkinSimple->GetCancelButton()->GetBitmapClicked());
	}
    itemBoxSizer44->Add(itemBitmapButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    // Set validators
    m_SkinSelectorCtrl->SetValidator( wxGenericValidator(& m_strSkinSelector) );
    m_CustomizePreferencesCtrl->SetValidator( wxGenericValidator(& m_bCustomizedPreferences) );
    m_WorkBetweenBeginCtrl->SetValidator( wxGenericValidator(& m_strWorkBetweenBegin) );
    m_WorkBetweenEndCtrl->SetValidator( wxGenericValidator(& m_strWorkBetweenEnd) );
    m_ConnectBetweenBeginCtrl->SetValidator( wxGenericValidator(& m_strConnectBetweenBegin) );
    m_ConnectBetweenEndCtrl->SetValidator( wxGenericValidator(& m_strConnectBetweenEnd) );
    m_MaxDiskUsageCtrl->SetValidator( wxGenericValidator(& m_strMaxDiskUsage) );
    m_MaxCPUUsageCtrl->SetValidator( wxGenericValidator(& m_strMaxCPUUsage) );
    m_WorkWhileInUseCtrl->SetValidator( wxGenericValidator(& m_bWorkWhileInUse) );
    m_WorkWhileOnBatteryCtrl->SetValidator( wxGenericValidator(& m_bWorkWhileOnBattery) );
    m_WorkWhenIdleCtrl->SetValidator( wxGenericValidator(& m_strWorkWhenIdle) );
////@end CPanelPreferences content construction
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CUSTOMIZEPREFERENCES
 */

void CPanelPreferences::OnCustomizePreferencesClick( wxCommandEvent& /*event*/ ) {
    UpdateControlStates();
}


/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_WORKBETWEENBEGIN
 */

void CPanelPreferences::OnWorkBetweenBeginSelected( wxCommandEvent& /*event*/ ) {
    UpdateControlStates();
}


/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_CONNECTBETWEENBEGIN
 */

void CPanelPreferences::OnConnectBetweenBeginSelected( wxCommandEvent& /*event*/ ) {
    UpdateControlStates();
}


/*!
 * wxEVT_ERASE_BACKGROUND event handler for ID_DLGPREFERENCES
 */

void CPanelPreferences::OnEraseBackground( wxEraseEvent& event ) {
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();
    
    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxMemoryDC memDC;
    wxCoord w, h, x, y;

    // Get the desired background bitmap
    wxBitmap bmp(*pSkinSimple->GetDialogBackgroundImage()->GetBitmap());

    // Dialog dimensions
    wxSize sz = GetClientSize();

    // Create a buffered device context to reduce flicker
    wxBufferedDC dc(event.GetDC(), sz, wxBUFFER_CLIENT_AREA);

    // bitmap dimensions
    w = bmp.GetWidth();
    h = bmp.GetHeight();

    // Fill the dialog with a magenta color so people can detect when something
    //   is wrong
    dc.SetBrush(wxBrush(wxColour(255,0,255)));
    dc.SetPen(wxPen(wxColour(255,0,255)));
    dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());

    // Is the bitmap smaller than the window?
    if ( (w < sz.x) || (h < sz.y) ) {
        // Check to see if they need to be rescaled to fit in the window
        wxImage img = bmp.ConvertToImage();
        img.Rescale((int) sz.x, (int) sz.y);

        // Draw our cool background (centered)
        dc.DrawBitmap(wxBitmap(img), 0, 0);
    } else {
        // Snag the center of the bitmap and use it
        //   for the background image
        x = wxMax(0, (w - sz.x)/2);
        y = wxMax(0, (h - sz.y)/2);

        // Select the desired bitmap into the memory DC so we can take
        //   the center chunk of it.
        memDC.SelectObject(bmp);

        // Draw the center chunk on the window
        dc.Blit(0, 0, w, h, &memDC, x, y, wxCOPY);

        // Drop the bitmap
        memDC.SelectObject(wxNullBitmap);
    }
}


void CPanelPreferences::OnOK() {
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    TransferDataFromWindow();

    SaveSkinSettings();


    if (m_bCustomizedPreferences) {
        SavePreferenceSettings();
    } else {
        ClearPreferenceSettings();
    }


	pDoc->rpc.set_global_prefs_override_struct(global_preferences_override, global_preferences_mask);
	pDoc->rpc.read_global_prefs_override();
}


bool CPanelPreferences::UpdateControlStates() {
    if (m_CustomizePreferencesCtrl->IsChecked()) {
        m_WorkBetweenBeginCtrl->Enable();
        m_WorkBetweenEndCtrl->Enable();
        m_ConnectBetweenBeginCtrl->Enable();
        m_ConnectBetweenEndCtrl->Enable();
        m_MaxDiskUsageCtrl->Enable();
        m_MaxCPUUsageCtrl->Enable();
        m_WorkWhileInUseCtrl->Enable();
        m_WorkWhileOnBatteryCtrl->Enable();
        m_WorkWhenIdleCtrl->Enable();

        if (m_WorkBetweenBeginCtrl->GetValue() == _("Anytime")) {
            m_WorkBetweenEndCtrl->Disable();
        }
        if (m_ConnectBetweenBeginCtrl->GetValue() == _("Anytime")) {
            m_ConnectBetweenEndCtrl->Disable();
        }

    } else {
        m_WorkBetweenBeginCtrl->Disable();
        m_WorkBetweenEndCtrl->Disable();
        m_ConnectBetweenBeginCtrl->Disable();
        m_ConnectBetweenEndCtrl->Disable();
        m_MaxDiskUsageCtrl->Disable();
        m_MaxCPUUsageCtrl->Disable();
        m_WorkWhileInUseCtrl->Disable();
        m_WorkWhileOnBatteryCtrl->Disable();
        m_WorkWhenIdleCtrl->Disable();
    }
    return true;
}


bool CPanelPreferences::ClearPreferenceSettings() {
    global_preferences_mask.start_hour = false;
    global_preferences_mask.end_hour = false;
    global_preferences_mask.net_start_hour = false;
    global_preferences_mask.net_end_hour = false;
    global_preferences_mask.disk_max_used_gb = false;
    global_preferences_mask.cpu_usage_limit = false;
    global_preferences_mask.run_if_user_active = false;
    global_preferences_mask.run_on_batteries = false;
    global_preferences_mask.idle_time_to_run = false;
    return true;
}


bool CPanelPreferences::ReadPreferenceSettings() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    GLOBAL_PREFS   display_global_preferences;
    double         dTempValue1 = 0.0;
    double         dTempValue2 = 0.0;

    unsigned int i;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    // Populate values and arrays from preferences
    pDoc->rpc.get_global_prefs_override_struct(global_preferences_override, global_preferences_mask);
    if (are_prefs_set()) {
        m_bCustomizedPreferences = true;
        display_global_preferences = global_preferences_override;
    } else {
        m_bCustomizedPreferences = false;
        display_global_preferences = pDoc->state.global_prefs;
    }


    // Do work only between:
    //   Start:
    wxArrayString aWorkBetweenBegin = wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings);
    aWorkBetweenBegin.Insert(_("Anytime"), 0);

    m_WorkBetweenBeginCtrl->Append(aWorkBetweenBegin);
    if (display_global_preferences.start_hour == display_global_preferences.end_hour) {
        m_strWorkBetweenBegin = _("Anytime");
    } else {
        m_strWorkBetweenBegin = astrTimeOfDayStrings[display_global_preferences.start_hour];
    }

    //   End:
    m_WorkBetweenEndCtrl->Append(wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings));
    m_strWorkBetweenEnd = astrTimeOfDayStrings[display_global_preferences.end_hour];

    // Connect to internet only between:
    //   Start:
    wxArrayString aConnectBetweenBegin = wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings);
    aConnectBetweenBegin.Insert(_("Anytime"), 0);

    m_ConnectBetweenBeginCtrl->Append(aConnectBetweenBegin);
    if (display_global_preferences.net_start_hour == display_global_preferences.net_end_hour) {
        m_strConnectBetweenBegin = _("Anytime");
    } else {
        m_strConnectBetweenBegin = astrTimeOfDayStrings[display_global_preferences.net_start_hour];
    }

    //   End:
    m_ConnectBetweenEndCtrl->Append(wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings));
    m_strConnectBetweenEnd = astrTimeOfDayStrings[display_global_preferences.net_end_hour];

    // Use no more than %s of disk space
    wxArrayString aDiskUsage = wxArrayString(iDiskUsageArraySize, astrDiskUsageStrings);
    wxString strDiskUsage = wxEmptyString;
    int iDiskUsageIndex = iDiskUsageArraySize;

    if (display_global_preferences.disk_max_used_gb > 0) {
        if (display_global_preferences.disk_max_used_gb < 1) {
            strDiskUsage.Printf(_("%d MB"), (int)(display_global_preferences.disk_max_used_gb * 1000));  
	    } else {
            strDiskUsage.Printf(_("%4.2f GB"), display_global_preferences.disk_max_used_gb); 
	    }

        // Null out strDiskUsage if it is a duplicate
        for (i = 0; i < aDiskUsage.Count(); i++) {
            strDiskUsage.ToDouble(&dTempValue1);
            if (strDiskUsage.Find(wxT("MB")) != -1) {
                dTempValue1 /= 1000;
            }
            aDiskUsage[i].ToDouble(&dTempValue2);
            if (aDiskUsage[i].Find(wxT("MB")) != -1) {
                dTempValue2 /= 1000;
            }
            if ((strDiskUsage == aDiskUsage[i]) || (dTempValue1 == dTempValue2)) {
                strDiskUsage = wxEmptyString;
                // Store this value so we know what to set the selection too in the
                //   combo box.
                iDiskUsageIndex = i;
                break;
            }
        }
    }

    if (!strDiskUsage.IsEmpty()) {
        aDiskUsage.Add(strDiskUsage);
        aDiskUsage.Sort(CompareDiskUsage);
    }

    m_MaxDiskUsageCtrl->Append(aDiskUsage);
    if (!strDiskUsage.IsEmpty()) {
        m_strMaxDiskUsage = strDiskUsage;
    } else {
        m_strMaxDiskUsage = astrDiskUsageStrings[iDiskUsageIndex];
    }

    // Use no more than %s of the processor
    wxArrayString aCPUUsage = wxArrayString(iCPUUsageArraySize, astrCPUUsageStrings);
    wxString strCPUUsage = wxEmptyString;
    int iCPUUsageIndex = iCPUUsageArraySize - 4;

    if (display_global_preferences.cpu_usage_limit > 0) {
        strCPUUsage.Printf(_("%d%%"), (int)display_global_preferences.cpu_usage_limit); 

        // Null out strCPUUsage if it is a duplicate
        for (i=0; i < aCPUUsage.Count(); i++) {
            strCPUUsage.ToDouble(&dTempValue1);
            aCPUUsage[i].ToDouble(&dTempValue2);
            if ((strCPUUsage == aCPUUsage[i]) || (dTempValue1 == dTempValue2)) {
                strCPUUsage = wxEmptyString;
                // Store this value so we know what to set the selection too in the
                //   combo box.
                iCPUUsageIndex = i;
                break;
            }
        }
    }

    if (!strCPUUsage.IsEmpty()) {
        aCPUUsage.Add(strCPUUsage);
        aCPUUsage.Sort(CompareCPUUsage);
    }

    m_MaxCPUUsageCtrl->Append(aCPUUsage);
    if (!strCPUUsage.IsEmpty()) {
        m_strMaxCPUUsage = strCPUUsage;
    } else {
        m_strMaxCPUUsage = astrCPUUsageStrings[iCPUUsageIndex];
    }

    // Do work while computer is in use?
    m_bWorkWhileInUse = display_global_preferences.run_if_user_active;

    // Do work while computer is on battery?
    m_bWorkWhileOnBattery = display_global_preferences.run_on_batteries;

    // Do work after computer is idle for:
    wxArrayString aWorkWhenIdle = wxArrayString(iWorkWhenIdleArraySize, astrWorkWhenIdleStrings);
    wxString strWorkWhenIdle = wxEmptyString;
    int iWorkWhenIdleIndex = 2;

    if (display_global_preferences.idle_time_to_run > 0) {
        strWorkWhenIdle.Printf(_("%d"), (int)display_global_preferences.idle_time_to_run); 

        // Null out strWorkWhenIdle if it is a duplicate
        for (i=0; i < aWorkWhenIdle.Count(); i++) {
            strWorkWhenIdle.ToDouble(&dTempValue1);
            aWorkWhenIdle[i].ToDouble(&dTempValue2);
            if ((strWorkWhenIdle == aWorkWhenIdle[i]) || (dTempValue1 == dTempValue2)) {
                strWorkWhenIdle = wxEmptyString;
                // Store this value so we know what to set the selection too in the
                //   combo box.
                iWorkWhenIdleIndex = i;
                break;
            }
        }
    }

    if (!strWorkWhenIdle.IsEmpty()) {
        aWorkWhenIdle.Add(strWorkWhenIdle);
        aWorkWhenIdle.Sort(CompareWorkWhenIdle);
    }

    m_WorkWhenIdleCtrl->Append(aWorkWhenIdle);
    if (!strWorkWhenIdle.IsEmpty()) {
        m_strWorkWhenIdle = strWorkWhenIdle;
    } else {
        m_strWorkWhenIdle = aWorkWhenIdle[iWorkWhenIdleIndex];
    }

    // Now make sure the UI is in sync with the settings
    TransferDataToWindow();
    UpdateControlStates();

    return true;
}


// Like GLOBAL_PREFS_MASK::are_prefs_set() but checks only the items on dialog
bool CPanelPreferences::are_prefs_set() {
    if (global_preferences_mask.start_hour) return true;         // 0..23; no restriction if start==end
    if (global_preferences_mask.end_hour) return true;
    if (global_preferences_mask.net_start_hour) return true;     // 0..23; no restriction if start==end
    if (global_preferences_mask.net_end_hour) return true;
    if (global_preferences_mask.disk_max_used_gb) return true;
    if (global_preferences_mask.cpu_usage_limit) return true;
    if (global_preferences_mask.run_on_batteries) return true;
    if (global_preferences_mask.run_if_user_active) return true;
    if (global_preferences_mask.idle_time_to_run) return true;
    return false;
}



bool CPanelPreferences::ReadSkinSettings() {
    CSkinManager* pSkinManager = wxGetApp().GetSkinManager();

    wxASSERT(pSkinManager);
    wxASSERT(wxDynamicCast(pSkinManager, CSkinManager));


    // Setup the values for all the skins, and then set the default.
    m_SkinSelectorCtrl->Append(pSkinManager->GetCurrentSkins());
    m_SkinSelectorCtrl->SetValue(pSkinManager->GetSelectedSkin());

    return true;
}


bool CPanelPreferences::SavePreferenceSettings() {
    // Do work only between:
    if (_("Anytime") == m_strWorkBetweenBegin) {
        global_preferences_override.start_hour = 0;
        global_preferences_override.end_hour = 0;
    } else {
        m_strWorkBetweenBegin.ToLong((long*)&global_preferences_override.start_hour);
        m_strWorkBetweenEnd.ToLong((long*)&global_preferences_override.end_hour);
    }
    global_preferences_mask.start_hour = true;        
    global_preferences_mask.end_hour = true;

    // Connect to internet only between:
    if (_("Anytime") == m_strConnectBetweenBegin) {
        global_preferences_override.net_start_hour = 0;
        global_preferences_override.net_end_hour = 0;
    } else {
        m_strConnectBetweenBegin.ToLong((long*)&global_preferences_override.net_start_hour);
        m_strConnectBetweenEnd.ToLong((long*)&global_preferences_override.net_end_hour);
    }
    global_preferences_mask.net_start_hour = true;        
    global_preferences_mask.net_end_hour = true;        

    // Use no more than %s of disk space
    m_strMaxDiskUsage.ToDouble((double*)&global_preferences_override.disk_max_used_gb);
    if (m_strMaxDiskUsage.Find(wxT("MB")) != -1) {
        global_preferences_override.disk_max_used_gb /= 1000;
    }
    global_preferences_mask.disk_max_used_gb = true;        

    // Use no more than %s of the processor
    m_strMaxCPUUsage.ToDouble((double*)&global_preferences_override.cpu_usage_limit);
    global_preferences_mask.cpu_usage_limit = true;        

    // Do work while computer is in use?
    global_preferences_override.run_if_user_active = m_bWorkWhileInUse;
    global_preferences_mask.run_if_user_active = true;        

    // Do work while computer is on battery?
    global_preferences_override.run_on_batteries = m_bWorkWhileOnBattery;
    global_preferences_mask.run_on_batteries = true;        

    // Do work after computer is idle for:
    m_strWorkWhenIdle.ToDouble((double*)&global_preferences_override.idle_time_to_run);
    global_preferences_mask.idle_time_to_run = true;        

    return true;
}


bool CPanelPreferences::SaveSkinSettings() {
    CSkinManager* pSkinManager = wxGetApp().GetSkinManager();
    wxLocale* pLocale = wxGetApp().GetLocale();

    wxASSERT(pSkinManager);
    wxASSERT(pLocale);
    wxASSERT(wxDynamicCast(pSkinManager, CSkinManager));

    pSkinManager->ReloadSkin(pLocale, m_strSkinSelector);

    return true;
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
    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        strCaption = _("BOINC Manager - Preferences");
    }
    wxDialog::Create( parent, id, strCaption, pos, size, style );


#ifdef __WXDEBUG__
    SetBackgroundColour(wxColour(255, 0, 255));
#endif
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    SetForegroundColour(*wxBLACK);
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);


    Freeze();

    m_pBackgroundPanel = new CPanelPreferences(this);
    wxBoxSizer* itemBoxSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer);
    itemBoxSizer->Add(m_pBackgroundPanel, 0, wxGROW, 0);
    
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    Thaw();

    return true;
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CDlgPreferences::OnOK( wxCommandEvent& event ) {
    wxDialog::OnOK(event);
    m_pBackgroundPanel->OnOK();
    EndModal(wxID_OK);
}


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


int iDiskUsageArraySize = 6;
wxString astrDiskUsageStrings[] = {
    _("100 MB"),
    _("200 MB"),
    _("500 MB"),
    _("1 GB"),
    _("2 GB"),
    _("5 GB")
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


int iWorkWhenIdleArraySize = 6;
wxString astrWorkWhenIdleStrings[] = {
    _("1"),
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
 * CDlgPreferences type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgPreferences, wxDialog )

/*!
 * CDlgPreferences event table definition
 */

BEGIN_EVENT_TABLE( CDlgPreferences, wxDialog )
////@begin CDlgPreferences event table entries
    EVT_ERASE_BACKGROUND( CDlgPreferences::OnEraseBackground )
    EVT_CHECKBOX( ID_CUSTOMIZEPREFERENCES, CDlgPreferences::OnCustomizePreferencesClick )
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
////@begin CDlgPreferences member initialisation
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
////@end CDlgPreferences member initialisation

    wxDialog::Create( parent, id, caption, pos, size, style );
#ifdef __WXDEBUG__
    SetBackgroundColour(wxColour(255, 0, 255));
#endif
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    SetForegroundColour(*wxBLACK);
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    Freeze();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    ReadPreferenceSettings();
    ReadSkinSettings();

    TransferDataToWindow();

    Thaw();

    return true;
}


/*!
 * Control creation for CDlgPreferences
 */

void CDlgPreferences::CreateControls()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    CDlgPreferences* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(1, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxGROW|wxALL, 5);

    CTransparentStaticText* itemStaticText4 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Skin"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText4->SetFont(wxFont(16, wxSWISS, wxNORMAL, wxBOLD, false, _T("Arial")));
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_LEFT|wxLEFT|wxBOTTOM, 20);

    CTransparentStaticText* itemStaticText6 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Skin:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText6->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
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
    itemStaticText10->SetFont(wxFont(16, wxSWISS, wxNORMAL, wxBOLD, false, _T("Arial")));
    itemFlexGridSizer9->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT, 20);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer11->Add(itemBoxSizer12, 0, wxALIGN_LEFT|wxALL, 0);

    m_CustomizePreferencesCtrl = new wxCheckBox( itemDialog1, ID_CUSTOMIZEPREFERENCES, _(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_CustomizePreferencesCtrl->SetValue(false);
    itemBoxSizer12->Add(m_CustomizePreferencesCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    CTransparentStaticTextAssociate* itemStaticText14 = new CTransparentStaticTextAssociate( itemDialog1, wxID_ANY, _("I want to customize my preferences for this computer."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText14->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText14->AssociateWindow(m_CustomizePreferencesCtrl);
    itemBoxSizer12->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText15 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Customized Preferences"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText15->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, false, _T("Arial")));
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
    itemStaticText16->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText16->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_WorkBetweenBeginCtrlStrings = NULL;
    m_WorkBetweenBeginCtrl = new wxComboBox( itemDialog1, ID_WORKBETWEENBEGIN, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_WorkBetweenBeginCtrlStrings, wxCB_READONLY|wxCB_SORT );
    m_WorkBetweenBeginCtrl->Enable(false);
    itemBoxSizer17->Add(m_WorkBetweenBeginCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText19 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText19->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer17->Add(itemStaticText19, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString* m_WorkBetweenEndCtrlStrings = NULL;
    m_WorkBetweenEndCtrl = new wxComboBox( itemDialog1, ID_WORKBETWEENEND, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_WorkBetweenEndCtrlStrings, wxCB_READONLY );
    m_WorkBetweenEndCtrl->Enable(false);
    itemBoxSizer17->Add(m_WorkBetweenEndCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText21 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Connect to internet only between:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText21->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText21->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText21, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer22, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_ConnectBetweenBeginCtrlStrings = NULL;
    m_ConnectBetweenBeginCtrl = new wxComboBox( itemDialog1, ID_CONNECTBETWEENBEGIN, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_ConnectBetweenBeginCtrlStrings, wxCB_READONLY );
    m_ConnectBetweenBeginCtrl->Enable(false);
    itemBoxSizer22->Add(m_ConnectBetweenBeginCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText24 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText24->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer22->Add(itemStaticText24, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString* m_ConnectBetweenEndCtrlStrings = NULL;
    m_ConnectBetweenEndCtrl = new wxComboBox( itemDialog1, ID_CONNECTBETWEENEND, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_ConnectBetweenEndCtrlStrings, wxCB_READONLY );
    m_ConnectBetweenEndCtrl->Enable(false);
    itemBoxSizer22->Add(m_ConnectBetweenEndCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText26 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Use no more than:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText26->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText26->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_MaxDiskUsageCtrlStrings = NULL;
    m_MaxDiskUsageCtrl = new wxComboBox( itemDialog1, ID_MAXDISKUSAGE, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_MaxDiskUsageCtrlStrings, wxCB_READONLY );
    m_MaxDiskUsageCtrl->Enable(false);
    itemBoxSizer27->Add(m_MaxDiskUsageCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText29 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("of disk space"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText29->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer27->Add(itemStaticText29, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    CTransparentStaticText* itemStaticText30 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Use no more than:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText30->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText30->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText30, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer31 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer31, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_MaxCPUUsageCtrlStrings = NULL;
    m_MaxCPUUsageCtrl = new wxComboBox( itemDialog1, ID_MAXCPUUSAGE, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_MaxCPUUsageCtrlStrings, wxCB_READONLY );
    m_MaxCPUUsageCtrl->Enable(false);
    itemBoxSizer31->Add(m_MaxCPUUsageCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText33 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("of the processor"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText33->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer31->Add(itemStaticText33, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    CTransparentStaticText* itemStaticText34 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Do work while in use?"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText34->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText34->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText34, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer35 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer35, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_WorkWhileInUseCtrl = new wxCheckBox( itemDialog1, ID_WORKWHILEINUSE, _T(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_WorkWhileInUseCtrl->SetValue(false);
    m_WorkWhileInUseCtrl->Enable(false);
    itemBoxSizer35->Add(m_WorkWhileInUseCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText37 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Do work while on battery?"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText37->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText37->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText37, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer38 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer38, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_WorkWhileOnBatteryCtrl = new wxCheckBox( itemDialog1, ID_WORKWHILEONBATTERY, _T(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_WorkWhileOnBatteryCtrl->SetValue(false);
    m_WorkWhileOnBatteryCtrl->Enable(false);
    itemBoxSizer38->Add(m_WorkWhileOnBatteryCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText40 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Do work after idle for:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText40->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText40->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText40, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer41 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer41, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_WorkWhenIdleCtrlStrings = NULL;
    m_WorkWhenIdleCtrl = new wxComboBox( itemDialog1, ID_WORKWHENIDLE, _T(""), wxDefaultPosition, wxSize(100, -1), 0, m_WorkWhenIdleCtrlStrings, wxCB_READONLY );
    m_WorkWhenIdleCtrl->Enable(false);
    itemBoxSizer41->Add(m_WorkWhenIdleCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText43 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText43->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer41->Add(itemStaticText43, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer44 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer44, 0, wxALIGN_RIGHT|wxALL, 5);

    wxBitmapButton* itemBitmapButton44 = new wxBitmapButton( itemDialog1, wxID_OK, *pSkinSimple->GetSaveButton()->GetBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_NOAUTODRAW );
    itemBitmapButton44->SetBitmapSelected(*pSkinSimple->GetSaveButton()->GetBitmapClicked());
    itemBoxSizer44->Add(itemBitmapButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton45 = new wxBitmapButton( itemDialog1, wxID_CANCEL, *pSkinSimple->GetCancelButton()->GetBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_NOAUTODRAW );
    itemBitmapButton45->SetBitmapSelected(*pSkinSimple->GetCancelButton()->GetBitmapClicked());
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
////@end CDlgPreferences content construction
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CUSTOMIZEPREFERENCES
 */

void CDlgPreferences::OnCustomizePreferencesClick( wxCommandEvent& event ) {
    UpdateControlStates(event.IsChecked());
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CDlgPreferences::OnOK( wxCommandEvent& event ) {
    wxDialog::OnOK(event);
    if (m_bCustomizedPreferences) {
        SavePreferenceSettings();
    } else {
        ClearPreferenceSettings();
    }
    SaveSkinSettings();
    EndModal(wxID_OK);
}


/*!
 * wxEVT_ERASE_BACKGROUND event handler for ID_DLGPREFERENCES
 */

void CDlgPreferences::OnEraseBackground( wxEraseEvent& event ) {
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();
    
    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxDC* dc = event.GetDC();
    wxSize sz = GetClientSize();
    wxBitmap bmp(*pSkinSimple->GetPreferencesDialogBackgroundImage()->GetBitmap());
    wxMemoryDC memDC;
    wxCoord w, h, x, y;

#ifdef __WXDEBUG__
    // Fill the dialog with a magenta color so people can detect when something
    //   is wrong
    dc->SetBrush(wxBrush(wxColour(255,0,255)));
    dc->SetPen(wxPen(wxColour(255,0,255)));
    dc->DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
#endif

    // Our bitmap dimensions
    w = bmp.GetWidth();
    h = bmp.GetHeight();

    // Is the bitmap smaller than the window?
    if ( (w < sz.x) || (h < sz.y) ) {
        // Center the bitmap on the window, but never
        //   draw at a negative position.
        x = wxMax(0, (sz.x - w)/2);
        y = wxMax(0, (sz.y - h)/2);

        // Draw our cool background (centered)
        dc->DrawBitmap(bmp, x, y);
    } else {
        // Snag the center of the bitmap and use it
        //   for the background image
        x = wxMax(0, (w - sz.x)/2);
        y = wxMax(0, (h - sz.y)/2);

        // Select the desired bitmap into the memory DC so we can take
        //   the center chunk of it.
        memDC.SelectObject(bmp);

        // Draw the center chunk on the window
        dc->Blit(0, 0, w, h, &memDC, x, y, wxCOPY);

        // Drop the bitmap
        memDC.SelectObject(wxNullBitmap);
    }
}


bool CDlgPreferences::UpdateControlStates(bool bChecked) {
    if (bChecked) {
        m_WorkBetweenBeginCtrl->Enable();
        m_WorkBetweenEndCtrl->Enable();
        m_ConnectBetweenBeginCtrl->Enable();
        m_ConnectBetweenEndCtrl->Enable();
        m_MaxDiskUsageCtrl->Enable();
        m_MaxCPUUsageCtrl->Enable();
        m_WorkWhileInUseCtrl->Enable();
        m_WorkWhileOnBatteryCtrl->Enable();
        m_WorkWhenIdleCtrl->Enable();
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


bool CDlgPreferences::ClearPreferenceSettings() {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    std::string str;
    str.clear();

	pDoc->rpc.set_global_prefs_override(str);

    return true;
}


bool CDlgPreferences::ReadPreferenceSettings() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    unsigned int i;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    // Populate values and arrays from preferences
    if (pDoc->rpc.get_global_prefs_override_struct(m_prefs) == 0) {
        m_bCustomizedPreferences = true;
    } else {
        m_bCustomizedPreferences = false;
        m_prefs = pDoc->state.global_prefs;
    }


    // Do work only between:
    //   Start:
    m_WorkBetweenBeginCtrl->Append(wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings));
    m_strWorkBetweenBegin = astrTimeOfDayStrings[m_prefs.start_hour];
    //   End:
    m_WorkBetweenEndCtrl->Append(wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings));
    m_strWorkBetweenEnd = astrTimeOfDayStrings[m_prefs.end_hour];

    // Connect to internet only between:
    //   Start:
    m_ConnectBetweenBeginCtrl->Append(wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings));
    m_strConnectBetweenBegin = astrTimeOfDayStrings[m_prefs.net_start_hour];
    //   End:
    m_ConnectBetweenEndCtrl->Append(wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings));
    m_strConnectBetweenEnd = astrTimeOfDayStrings[m_prefs.net_end_hour];

    // Use no more than %s of disk space
    wxArrayString aDiskUsage = wxArrayString(iDiskUsageArraySize, astrDiskUsageStrings);
    wxString strDiskUsage = wxEmptyString;
    int iDiskUsageIndex = iDiskUsageArraySize;

    if (m_prefs.disk_max_used_gb > 0) {
        if (m_prefs.disk_max_used_gb < 1) {
            strDiskUsage.Printf(_("%d MB"), (int)(m_prefs.disk_max_used_gb * 1000));  
	    } else {
            strDiskUsage.Printf(_("%4.2f GB"), m_prefs.disk_max_used_gb); 
	    }

        // Null out strDiskUsage if it is a duplicate
        for (i = 0; i < aDiskUsage.Count(); i++) {
            if (strDiskUsage == aDiskUsage[i]) {
                strDiskUsage = wxEmptyString;
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

    if (m_prefs.cpu_usage_limit > 0) {
        strCPUUsage.Printf(_("%d%%"), (int)m_prefs.cpu_usage_limit); 

        // Null out strCPUUsage if it is a duplicate
        for (i=0; i < aCPUUsage.Count(); i++) {
            if (strCPUUsage == aCPUUsage[i]) {
                strCPUUsage = wxEmptyString;
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
    m_bWorkWhileInUse = m_prefs.run_if_user_active;

    // Do work while computer is on battery?
    m_bWorkWhileOnBattery = m_prefs.run_on_batteries;

    // Do work after computer is idle for:
    wxArrayString aWorkWhenIdle = wxArrayString(iWorkWhenIdleArraySize, astrWorkWhenIdleStrings);
    wxString strWorkWhenIdle = wxEmptyString;
    int iWorkWhenIdleIndex = 2;

    if (m_prefs.idle_time_to_run > 0) {
        strWorkWhenIdle.Printf(_("%d"), (int)m_prefs.idle_time_to_run); 

        // Null out strWorkWhenIdle if it is a duplicate
        for (i=0; i < aWorkWhenIdle.Count(); i++) {
            if (strWorkWhenIdle == aWorkWhenIdle[i]) {
                strWorkWhenIdle = wxEmptyString;
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
    UpdateControlStates(m_bCustomizedPreferences);

    return true;
}


bool CDlgPreferences::ReadSkinSettings() {
    CSkinManager* pSkinManager = wxGetApp().GetSkinManager();

    wxASSERT(pSkinManager);
    wxASSERT(wxDynamicCast(pSkinManager, CSkinManager));


    // Setup the values for all the skins, and then set the default.
    m_SkinSelectorCtrl->Append(pSkinManager->GetCurrentSkins());
    m_SkinSelectorCtrl->SetValue(pSkinManager->GetSelectedSkin());

    return true;
}


bool CDlgPreferences::SavePreferenceSettings() {
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    // Copy all the defaults from global_prefs
    m_prefs = pDoc->state.global_prefs;

    // Do work only between:
    m_strWorkBetweenBegin.ToLong((long*)&m_prefs.start_hour);
    m_strWorkBetweenEnd.ToLong((long*)&m_prefs.end_hour);

    // Connect to internet only between:
    m_strConnectBetweenBegin.ToLong((long*)&m_prefs.net_start_hour);
    m_strConnectBetweenEnd.ToLong((long*)&m_prefs.net_end_hour);

    // Use no more than %s of disk space
    m_strMaxDiskUsage.ToDouble((double*)&m_prefs.disk_max_used_gb);
    if (m_strMaxDiskUsage.Find(wxT("GB")) != -1) {
        m_prefs.disk_max_used_gb /= 1000;
    }

    // Use no more than %s of the processor
    m_strMaxCPUUsage.ToDouble((double*)&m_prefs.cpu_usage_limit);

    // Do work while computer is in use?
    m_prefs.run_if_user_active = m_bWorkWhileInUse;

    // Do work while computer is on battery?
    m_prefs.run_on_batteries = m_bWorkWhileOnBattery;

    // Do work after computer is idle for:
    m_strWorkWhenIdle.ToDouble((double*)&m_prefs.idle_time_to_run);

	pDoc->rpc.set_global_prefs_override_struct(m_prefs);
	pDoc->rpc.read_global_prefs_override();
    return true;
}


bool CDlgPreferences::SaveSkinSettings() {
    CSkinManager* pSkinManager = wxGetApp().GetSkinManager();
    wxLocale* pLocale = wxGetApp().GetLocale();

    wxASSERT(pSkinManager);
    wxASSERT(pLocale);
    wxASSERT(wxDynamicCast(pSkinManager, CSkinManager));

    pSkinManager->ReloadSkin(pLocale, m_strSkinSelector);

    return true;
}


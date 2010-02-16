// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

using std::string;

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
int iTimeOfDayArraySize = 25;
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
    m_WorkWhileOnBatteryCtrl = NULL;
    m_WorkWhenIdleCtrl = NULL;
////@end CPanelPreferences member initialisation

    CreateControls();

    ReadPreferenceSettings();
    ReadSkinSettings();

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
    m_SkinSelectorCtrl = new wxComboBox( itemDialog1, ID_SKINSELECTOR, _T(""), wxDefaultPosition, wxSize(-1, -1), 0, m_SkinSelectorCtrlStrings, wxCB_READONLY );
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

    m_CustomizePreferencesCtrl = new wxCheckBox( itemDialog1, ID_CUSTOMIZEPREFERENCES, wxT(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
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
    m_WorkBetweenBeginCtrl = new wxComboBox( itemDialog1, ID_WORKBETWEENBEGIN, _T(""), wxDefaultPosition, wxDefaultSize, 0, m_WorkBetweenBeginCtrlStrings, wxCB_READONLY );
    m_WorkBetweenBeginCtrl->Enable(false);
    itemBoxSizer17->Add(m_WorkBetweenBeginCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText19 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText19->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer17->Add(itemStaticText19, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString* m_WorkBetweenEndCtrlStrings = NULL;
    m_WorkBetweenEndCtrl = new wxComboBox( itemDialog1, ID_WORKBETWEENEND, _T(""), wxDefaultPosition, wxDefaultSize, 0, m_WorkBetweenEndCtrlStrings, wxCB_READONLY );
    m_WorkBetweenEndCtrl->Enable(false);
    itemBoxSizer17->Add(m_WorkBetweenEndCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText21 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Connect to internet only between:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText21->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText21->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText21, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer22, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_ConnectBetweenBeginCtrlStrings = NULL;
    m_ConnectBetweenBeginCtrl = new wxComboBox( itemDialog1, ID_CONNECTBETWEENBEGIN, _T(""), wxDefaultPosition, wxDefaultSize, 0, m_ConnectBetweenBeginCtrlStrings, wxCB_READONLY );
    m_ConnectBetweenBeginCtrl->Enable(false);
    itemBoxSizer22->Add(m_ConnectBetweenBeginCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText24 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText24->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer22->Add(itemStaticText24, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxString* m_ConnectBetweenEndCtrlStrings = NULL;
    m_ConnectBetweenEndCtrl = new wxComboBox( itemDialog1, ID_CONNECTBETWEENEND, _T(""), wxDefaultPosition, wxDefaultSize, 0, m_ConnectBetweenEndCtrlStrings, wxCB_READONLY );
    m_ConnectBetweenEndCtrl->Enable(false);
    itemBoxSizer22->Add(m_ConnectBetweenEndCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText26 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("Use no more than:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText26->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemStaticText26->Wrap(250);
    itemFlexGridSizer15->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer15->Add(itemBoxSizer27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxString* m_MaxDiskUsageCtrlStrings = NULL;
    m_MaxDiskUsageCtrl = new wxComboBox( itemDialog1, ID_MAXDISKUSAGE, _T(""), wxDefaultPosition, wxSize(-1, -1), 0, m_MaxDiskUsageCtrlStrings, wxCB_READONLY );
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
    m_MaxCPUUsageCtrl = new wxComboBox( itemDialog1, ID_MAXCPUUSAGE, _T(""), wxDefaultPosition, wxDefaultSize, 0, m_MaxCPUUsageCtrlStrings, wxCB_READONLY );
    m_MaxCPUUsageCtrl->Enable(false);
    itemBoxSizer31->Add(m_MaxCPUUsageCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText33 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("of the processor"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText33->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer31->Add(itemStaticText33, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

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
    m_WorkWhenIdleCtrl = new wxComboBox( itemDialog1, ID_WORKWHENIDLE, _T(""), wxDefaultPosition, wxSize(-1, -1), 0, m_WorkWhenIdleCtrlStrings, wxCB_READONLY );
    m_WorkWhenIdleCtrl->Enable(false);
    itemBoxSizer41->Add(m_WorkWhenIdleCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText43 = new CTransparentStaticText( itemDialog1, wxID_ANY, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText43->SetFont(wxFont(SMALL_FONT, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer41->Add(itemStaticText43, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer44 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer44, 0, wxALIGN_RIGHT|wxALL, 5);

    wxBitmapButton* itemBitmapButton44 = new wxBitmapButton(
        itemDialog1,
        wxID_OK,
        *pSkinSimple->GetSaveButton()->GetBitmap(),
        wxDefaultPosition,
        wxSize(
            (*pSkinSimple->GetSaveButton()->GetBitmap()).GetWidth(),
            (*pSkinSimple->GetSaveButton()->GetBitmap()).GetHeight()
        ),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetSaveButton()->GetBitmapClicked() != NULL ) {
		itemBitmapButton44->SetBitmapSelected(*pSkinSimple->GetSaveButton()->GetBitmapClicked());
	}
    itemBoxSizer44->Add(itemBitmapButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton45 = new wxBitmapButton(
        itemDialog1,
        wxID_CANCEL,
        *pSkinSimple->GetCancelButton()->GetBitmap(),
        wxDefaultPosition,
        wxSize(
            (*pSkinSimple->GetCancelButton()->GetBitmap()).GetWidth(),
            (*pSkinSimple->GetCancelButton()->GetBitmap()).GetHeight()
        ),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetCancelButton()->GetBitmapClicked() != NULL ) {
		itemBitmapButton45->SetBitmapSelected(*pSkinSimple->GetCancelButton()->GetBitmapClicked());
	}
    itemBoxSizer44->Add(itemBitmapButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifndef __WXMSW__
#ifdef __WXMAC__
	wxBitmapButton* itemButton46 = new wxBitmapButton(
        this,
        ID_SIMPLE_HELP,
        *pSkinSimple->GetHelpButton()->GetBitmap(),
        wxDefaultPosition,
        wxSize(
            (*pSkinSimple->GetHelpButton()->GetBitmap()).GetWidth(),
            (*pSkinSimple->GetHelpButton()->GetBitmap()).GetHeight()
        ),
        wxBU_AUTODRAW
    );
	if ( pSkinSimple->GetHelpButton()->GetBitmapClicked() != NULL ) {
		itemButton46->SetBitmapSelected(*pSkinSimple->GetHelpButton()->GetBitmapClicked());
	}
#ifdef wxUSE_TOOLTIPS
	itemButton46->SetToolTip(new wxToolTip(_("Get help with BOINC")));
#endif
    itemBoxSizer44->Add(itemButton46, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#else
    wxContextHelpButton* itemButton45 = new wxContextHelpButton(this);
    itemBoxSizer44->Add(itemButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#endif
#endif

    // Set validators
    m_SkinSelectorCtrl->SetValidator( wxGenericValidator(& m_strSkinSelector) );
    m_CustomizePreferencesCtrl->SetValidator( wxGenericValidator(& m_bCustomizedPreferences) );
    m_WorkBetweenBeginCtrl->SetValidator( wxGenericValidator(& m_strWorkBetweenBegin) );
    m_WorkBetweenEndCtrl->SetValidator( wxGenericValidator(& m_strWorkBetweenEnd) );
    m_ConnectBetweenBeginCtrl->SetValidator( wxGenericValidator(& m_strConnectBetweenBegin) );
    m_ConnectBetweenEndCtrl->SetValidator( wxGenericValidator(& m_strConnectBetweenEnd) );
    m_MaxDiskUsageCtrl->SetValidator( wxGenericValidator(& m_strMaxDiskUsage) );
    m_MaxCPUUsageCtrl->SetValidator( wxGenericValidator(& m_strMaxCPUUsage) );
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


	pDoc->rpc.set_global_prefs_override_struct(global_preferences_working, global_preferences_override_mask);
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
        m_WorkWhileOnBatteryCtrl->Disable();
        m_WorkWhenIdleCtrl->Disable();
    }
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


bool CPanelPreferences::ReadPreferenceSettings() {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    GLOBAL_PREFS   display_global_preferences;
    double         dTempValue1 = 0.0;
    double         dTempValue2 = 0.0;
    int            retval;
    unsigned int   i;

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

    if (!retval && global_preferences_override_mask.are_simple_prefs_set()) {
        m_bCustomizedPreferences = true;
    } else {
        m_bCustomizedPreferences = false;
    }
	
	// Allow this structure to be modified for display purposes
    display_global_preferences = global_preferences_working;


    // Do work only between:
    //   Start:
    wxArrayString aWorkBetweenBegin = wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings);
    aWorkBetweenBegin.Insert(_("Anytime"), 0);

    m_WorkBetweenBeginCtrl->Append(aWorkBetweenBegin);
    if (display_global_preferences.cpu_times.start_hour == display_global_preferences.cpu_times.end_hour) {
        m_strWorkBetweenBegin = _("Anytime");
    } else {
        m_strWorkBetweenBegin = astrTimeOfDayStrings[(int)display_global_preferences.cpu_times.start_hour];
    }

    //   End:
    m_WorkBetweenEndCtrl->Append(wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings));
    m_strWorkBetweenEnd = astrTimeOfDayStrings[(int)display_global_preferences.cpu_times.end_hour];

    // Connect to internet only between:
    //   Start:
    wxArrayString aConnectBetweenBegin = wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings);
    aConnectBetweenBegin.Insert(_("Anytime"), 0);

    m_ConnectBetweenBeginCtrl->Append(aConnectBetweenBegin);
    if (display_global_preferences.net_times.start_hour == display_global_preferences.net_times.end_hour) {
        m_strConnectBetweenBegin = _("Anytime");
    } else {
        m_strConnectBetweenBegin = astrTimeOfDayStrings[(int)display_global_preferences.net_times.start_hour];
    }

    //   End:
    m_ConnectBetweenEndCtrl->Append(wxArrayString(iTimeOfDayArraySize, astrTimeOfDayStrings));
    m_strConnectBetweenEnd = astrTimeOfDayStrings[(int)display_global_preferences.net_times.end_hour];

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

    // Do work while computer is on battery?
    m_bWorkWhileOnBattery = display_global_preferences.run_on_batteries;

    // Do work after computer is idle for:
    wxArrayString aWorkWhenIdle = wxArrayString(iWorkWhenIdleArraySize, astrWorkWhenIdleStrings);
    wxString strWorkWhenIdle = wxEmptyString;
    int iWorkWhenIdleIndex = 2;

    aWorkWhenIdle.Insert(_("0 (Run Always)"), 0);

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

	if (display_global_preferences.run_if_user_active) {
		// run_if_user_active and idle_time_to_run were merged into a single combo
		//   box. 0 = run if active.
		m_strWorkWhenIdle = aWorkWhenIdle[0];
	} else {	
		if (strWorkWhenIdle.IsEmpty()) {
			m_strWorkWhenIdle = aWorkWhenIdle[iWorkWhenIdleIndex];
		} else {
			m_strWorkWhenIdle = strWorkWhenIdle;
		}
	}

    // Now make sure the UI is in sync with the settings
    TransferDataToWindow();
    UpdateControlStates();

    return true;
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
        global_preferences_working.cpu_times.start_hour = 0;
        global_preferences_working.cpu_times.end_hour = 0;
    } else {
        m_strWorkBetweenBegin.ToDouble(&global_preferences_working.cpu_times.start_hour);
        m_strWorkBetweenEnd.ToDouble(&global_preferences_working.cpu_times.end_hour);
    }
    global_preferences_override_mask.start_hour = true;        
    global_preferences_override_mask.end_hour = true;

    // Connect to internet only between:
    if (_("Anytime") == m_strConnectBetweenBegin) {
        global_preferences_working.net_times.start_hour = 0;
        global_preferences_working.net_times.end_hour = 0;
    } else {
        m_strConnectBetweenBegin.ToDouble(&global_preferences_working.net_times.start_hour);
        m_strConnectBetweenEnd.ToDouble(&global_preferences_working.net_times.end_hour);
    }
    global_preferences_override_mask.net_start_hour = true;        
    global_preferences_override_mask.net_end_hour = true;        

    // Use no more than %s of disk space
    m_strMaxDiskUsage.ToDouble((double*)&global_preferences_working.disk_max_used_gb);
    if (m_strMaxDiskUsage.Find(wxT("MB")) != -1) {
        global_preferences_working.disk_max_used_gb /= 1000;
    }
    global_preferences_override_mask.disk_max_used_gb = true;        

    // Use no more than %s of the processor
    m_strMaxCPUUsage.ToDouble((double*)&global_preferences_working.cpu_usage_limit);
    global_preferences_override_mask.cpu_usage_limit = true;        

    // Do work while computer is on battery?
    global_preferences_working.run_on_batteries = m_bWorkWhileOnBattery;
    global_preferences_override_mask.run_on_batteries = true;        

    // Do work after computer is idle for:
    m_strWorkWhenIdle.ToDouble((double*)&global_preferences_working.idle_time_to_run);
    if (0 == global_preferences_working.idle_time_to_run) {
        global_preferences_working.run_if_user_active = true;
        global_preferences_override_mask.idle_time_to_run = false;
    } else {
        global_preferences_working.run_if_user_active = false;
        global_preferences_override_mask.idle_time_to_run = true;
    }
    global_preferences_override_mask.run_if_user_active = true;        

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
    EVT_HELP(wxID_ANY, CDlgPreferences::OnHelp)
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
        strCaption.Printf(_("%s - Preferences"), pSkinAdvanced->GetApplicationName().c_str());
    }
    SetTitle(strCaption);

    // Initialize Application Icon
    wxIconBundle icons;
    icons.AddIcon(*pSkinAdvanced->GetApplicationIcon());
    icons.AddIcon(*pSkinAdvanced->GetApplicationIcon32());
    SetIcons(icons);

    Freeze();

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    SetForegroundColour(*wxBLACK);
#ifdef __WXDEBUG__
    SetBackgroundColour(wxColour(255, 0, 255));
#endif

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
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CDlgPreferences::OnOK( wxCommandEvent& WXUNUSED(event) ) {
    m_pBackgroundPanel->OnOK();
    EndModal(wxID_OK);
}




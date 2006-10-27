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
#include "sg_StaticLine.h"
#include "sg_StaticText.h"
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
    EVT_BUTTON( wxID_OK, CDlgPreferences::OnOK )
    EVT_BUTTON( ID_CHANGEBUTTON, CDlgPreferences::OnChange )
    EVT_BUTTON( ID_CLEARBUTTON, CDlgPreferences::OnClear )
    EVT_ERASE_BACKGROUND( CDlgPreferences::OnEraseBackground )
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
    m_PreferenceIndicatorCtrl = NULL;
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

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

	if (wxDefaultSize == size) {
		SetSize(416, 404);
	}

    ReadPreferenceSettings();
    ReadSkinSettings();

    TransferDataToWindow();

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

////@begin CDlgPreferences content construction
    CDlgPreferences* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(1, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxGROW|wxALL, 5);

    CTransparentStaticText* itemStaticText4 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Skin"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT|wxTRANSPARENT_WINDOW );
    itemStaticText4->SetFont(wxFont(16, wxSWISS, wxNORMAL, wxBOLD, false, _T("Arial")));
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    CTransparentStaticText* itemStaticText6 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Skin:"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    itemStaticText6->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    m_SkinSelectorCtrl = new wxComboBox( itemDialog1, ID_SKINSELECTOR, _T(""), wxDefaultPosition, wxSize(175, -1), 0, NULL, wxCB_READONLY );
    itemBoxSizer5->Add(m_SkinSelectorCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxBitmapButton* itemBitmapButton8 = new wxBitmapButton( itemDialog1, ID_CHANGEBUTTON, *pSkinSimple->GetChangeButton()->GetBitmap(), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBitmapButton8->SetBitmapSelected(*pSkinSimple->GetChangeButton()->GetBitmapClicked());
    itemBoxSizer5->Add(itemBitmapButton8, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    CStaticLine* itemStaticLine9 = new CStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxSize(380, 1), 0 );
    itemStaticLine9->SetLineColor(pSkinSimple->GetStaticLineColor());
    itemBoxSizer6->Add(itemStaticLine9, 0, wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer10 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer10->AddGrowableCol(0);
    itemFlexGridSizer10->AddGrowableCol(1);
    itemFlexGridSizer10->AddGrowableCol(2);
    itemBoxSizer2->Add(itemFlexGridSizer10, 0, wxGROW|wxALL, 5);

    CTransparentStaticText* itemStaticText11 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Preferences"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    itemStaticText11->SetFont(wxFont(16, wxSWISS, wxNORMAL, wxBOLD, false, _T("Arial")));
    itemFlexGridSizer10->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    itemFlexGridSizer10->Add(0, 5, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PreferenceIndicatorCtrl = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Using global preferences"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    m_PreferenceIndicatorCtrl->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemFlexGridSizer10->Add(m_PreferenceIndicatorCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxFlexGridSizer* itemFlexGridSizer14 = new wxFlexGridSizer(7, 2, 0, 0);
    itemFlexGridSizer14->AddGrowableRow(0);
    itemFlexGridSizer14->AddGrowableRow(1);
    itemFlexGridSizer14->AddGrowableRow(2);
    itemFlexGridSizer14->AddGrowableRow(3);
    itemFlexGridSizer14->AddGrowableRow(4);
    itemFlexGridSizer14->AddGrowableRow(5);
    itemFlexGridSizer14->AddGrowableRow(6);
    itemFlexGridSizer14->AddGrowableCol(0);
    itemFlexGridSizer14->AddGrowableCol(1);
    itemBoxSizer2->Add(itemFlexGridSizer14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    CTransparentStaticText* itemStaticText15 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Do work only between:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText15->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemFlexGridSizer14->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer14->Add(itemBoxSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_WorkBetweenBeginCtrl = new wxComboBox( itemDialog1, ID_WORKBETWEENBEGIN, _("0:00"), wxDefaultPosition, wxSize(75, -1), 0, NULL, wxCB_READONLY );
    itemBoxSizer16->Add(m_WorkBetweenBeginCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText18 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText18->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer16->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_WorkBetweenEndCtrl = new wxComboBox( itemDialog1, ID_WORKBETWEENEND, _("0:00"), wxDefaultPosition, wxSize(75, -1), 0, NULL, wxCB_READONLY );
    itemBoxSizer16->Add(m_WorkBetweenEndCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText20 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Connect to internet only between:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText20->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemFlexGridSizer14->Add(itemStaticText20, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer14->Add(itemBoxSizer21, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ConnectBetweenBeginCtrl = new wxComboBox( itemDialog1, ID_CONNECTBETWEENBEGIN, _("0:00"), wxDefaultPosition, wxSize(75, -1), 0, NULL, wxCB_READONLY );
    itemBoxSizer21->Add(m_ConnectBetweenBeginCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText23 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText23->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer21->Add(itemStaticText23, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    m_ConnectBetweenEndCtrl = new wxComboBox( itemDialog1, ID_CONNECTBETWEENEND, _("0:00"), wxDefaultPosition, wxSize(75, -1), 0, NULL, wxCB_READONLY );
    itemBoxSizer21->Add(m_ConnectBetweenEndCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText25 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Use no more than:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText25->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemFlexGridSizer14->Add(itemStaticText25, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer14->Add(itemBoxSizer26, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_MaxDiskUsageCtrl = new wxComboBox( itemDialog1, ID_MAXDISKUSAGE, _("5 GB"), wxDefaultPosition, wxSize(75, -1), 0, NULL, wxCB_READONLY );
    itemBoxSizer26->Add(m_MaxDiskUsageCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText28 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("of disk space"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText28->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer26->Add(itemStaticText28, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    CTransparentStaticText* itemStaticText29 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Use no more than:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText29->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemFlexGridSizer14->Add(itemStaticText29, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer14->Add(itemBoxSizer30, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_MaxCPUUsageCtrl = new wxComboBox( itemDialog1, ID_MAXCPUUSAGE, _("70%"), wxDefaultPosition, wxSize(55, -1), 0, NULL, wxCB_READONLY );
    itemBoxSizer30->Add(m_MaxCPUUsageCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText32 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("of the processor"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText32->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer30->Add(itemStaticText32, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    CTransparentStaticText* itemStaticText33 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Do work while computer is in use?"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText33->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemFlexGridSizer14->Add(itemStaticText33, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer14->Add(itemBoxSizer34, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_WorkWhileInUseCtrl = new wxCheckBox( itemDialog1, ID_WORKWHILEINUSE, _T(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemBoxSizer34->Add(m_WorkWhileInUseCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText36 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Do work while computer is on battery?"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText36->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemFlexGridSizer14->Add(itemStaticText36, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer14->Add(itemBoxSizer37, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_WorkWhileOnBatteryCtrl = new wxCheckBox( itemDialog1, ID_WORKWHILEONBATTERY, _T(""), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemBoxSizer37->Add(m_WorkWhileOnBatteryCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText39 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("Do work after computer is idle for:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemStaticText39->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemFlexGridSizer14->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer40 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer14->Add(itemBoxSizer40, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_WorkWhenIdleCtrl = new wxComboBox( itemDialog1, ID_WORKWHENIDLE, _("5"), wxDefaultPosition, wxSize(55, -1), 0, NULL, wxCB_READONLY );
    itemBoxSizer40->Add(m_WorkWhenIdleCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    CTransparentStaticText* itemStaticText42 = new CTransparentStaticText( itemDialog1, wxID_STATIC, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText42->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxNORMAL, false, _T("Arial")));
    itemBoxSizer40->Add(itemStaticText42, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer43 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer43, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton44 = new wxBitmapButton( itemDialog1, wxID_OK, *pSkinSimple->GetSaveButton()->GetBitmap(), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBitmapButton44->SetBitmapSelected(*pSkinSimple->GetSaveButton()->GetBitmapClicked());
    itemBoxSizer43->Add(itemBitmapButton44, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton45 = new wxBitmapButton( itemDialog1, wxID_CANCEL, *pSkinSimple->GetCancelButton()->GetBitmap(), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBitmapButton45->SetBitmapSelected(*pSkinSimple->GetCancelButton()->GetBitmapClicked());
    itemBoxSizer43->Add(itemBitmapButton45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton46 = new wxBitmapButton( itemDialog1, ID_CLEARBUTTON, *pSkinSimple->GetClearButton()->GetBitmap(), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBitmapButton46->SetBitmapSelected(*pSkinSimple->GetClearButton()->GetBitmapClicked());
    itemBoxSizer43->Add(itemBitmapButton46, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    // Set validators
    m_SkinSelectorCtrl->SetValidator( wxGenericValidator(&m_strSkinSelector) );
    m_PreferenceIndicatorCtrl->SetValidator( wxGenericValidator(&m_strPreferenceIndicator) );
    m_WorkBetweenBeginCtrl->SetValidator( wxGenericValidator(&m_strWorkBetweenBegin) );
    m_WorkBetweenEndCtrl->SetValidator( wxGenericValidator(&m_strWorkBetweenEnd) );
    m_ConnectBetweenBeginCtrl->SetValidator( wxGenericValidator(&m_strConnectBetweenBegin) );
    m_ConnectBetweenEndCtrl->SetValidator( wxGenericValidator(&m_strConnectBetweenEnd) );
    m_MaxDiskUsageCtrl->SetValidator( wxGenericValidator(&m_strMaxDiskUsage) );
    m_MaxCPUUsageCtrl->SetValidator( wxGenericValidator(&m_strMaxCPUUsage) );
    m_WorkWhileInUseCtrl->SetValidator( wxGenericValidator(&m_bWorkWhileInUse) );
    m_WorkWhileOnBatteryCtrl->SetValidator( wxGenericValidator(&m_bWorkWhileOnBattery) );
    m_WorkWhenIdleCtrl->SetValidator( wxGenericValidator(&m_strWorkWhenIdle) );
////@end CDlgPreferences content construction
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CHANGEBUTTON
 */

void CDlgPreferences::OnChange( wxCommandEvent& event ) {
    wxDialog::OnOK(event);
    SaveSkinSettings();
    EndModal(wxID_OK);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CDlgPreferences::OnOK( wxCommandEvent& event ) {
    wxDialog::OnOK(event);
    SavePreferenceSettings();
    EndModal(wxID_OK);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEARBUTTON
 */

void CDlgPreferences::OnClear( wxCommandEvent& /*event*/ ) {
    ClearPreferenceSettings();
	EndModal(wxID_CANCEL);
}


/*!
 * wxEVT_ERASE_BACKGROUND event handler for ID_DLGPREFERENCES
 */

void CDlgPreferences::OnEraseBackground( wxEraseEvent& event ) {
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();
    wxDC* dc = event.GetDC();
    
    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    // Draw our cool background
    dc->DrawBitmap(*pSkinSimple->GetPreferencesDialogBackgroundImage()->GetBitmap(), 0, 0);
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
        m_strPreferenceIndicator = _("Using local preferences");
    } else {
        m_strPreferenceIndicator = _("Using global preferences");
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


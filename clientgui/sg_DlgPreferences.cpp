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
#include "sg_DlgPreferences.h"


/*!
 * CDlgPreferences type definition
 */

IMPLEMENT_DYNAMIC_CLASS(CDlgPreferences, wxDialog)

/*!
 * CDlgPreferences event table definition
 */

BEGIN_EVENT_TABLE(CDlgPreferences, wxDialog)

////@begin CDlgOptions event table entries
  EVT_BUTTON(-1,CDlgPreferences::OnChange)
  EVT_PAINT(CDlgPreferences::OnPaint)
  EVT_BUTTON(-1,CDlgPreferences::OnBtnClick)
  EVT_ERASE_BACKGROUND(CDlgPreferences::OnEraseBackground)
////@end CDlgOptions event table entries

END_EVENT_TABLE()

/*!
 * CDlgPreferences constructors
 */

CDlgPreferences::CDlgPreferences()
{
}

CDlgPreferences::CDlgPreferences(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) 
{
	Create(parent,id,title,pos,size,style);
}

/*!
 * CDlgPreferences creator
 */

bool CDlgPreferences::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
////@begin CDlgOptions member initialisation
////@end CDlgOptions member initialisation

////@begin CDlgOptions creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();

////@end CDlgOptions creation

	if(wxDefaultSize == size) {
		SetSize(416,403);
	}

    CSkinManager* pSkinManager = wxGetApp().GetSkinManager();

    wxASSERT(pSkinManager);
    wxASSERT(wxDynamicCast(pSkinManager, CSkinManager));


    // Setup the values for all the skins, and then set the default.
    cmbSkinPicker->Append(pSkinManager->GetCurrentSkins());
    cmbSkinPicker->SetValue(pSkinManager->GetSelectedSkin());



    Centre();

    return TRUE;
}





void CDlgPreferences::CreateControls()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();
	CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	// populate values from prefs
	if(pDoc->rpc.get_global_prefs_override_struct(m_prefs) == 0){
		m_PrefIndicator = _("Using local preferences");
	}else{
        m_PrefIndicator = _("Using global preferences");
		m_prefs = pDoc->state.global_prefs;
	}
	
	//Set Background color
    SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());

	cmbSkinPicker = new wxComboBox(this, ID_SKINPICKERCMBBOX, wxT(""), wxPoint(206,37), wxSize(140,21), NULL, wxNO_BORDER | wxCB_READONLY);

	wxToolTip *ttSaveSkin = new wxToolTip(_("Change skin"));
    btnSaveSkin=new wxBitmapButton(this,ID_CHANGEBUTTON,*(pSkinSimple->GetChangeButton()->GetBitmap()),wxPoint(184,82),wxSize(62,16),wxNO_BORDER);
    btnSaveSkin->SetBitmapSelected(*(pSkinSimple->GetChangeButton()->GetBitmapClicked()));
	btnSaveSkin->SetToolTip(ttSaveSkin);

	lnMyTop = new CStaticLine(this,wxPoint(16,113),wxSize(378,1));
    lnMyTop->SetLineColor(pSkinSimple->GetStaticLineColor());

	wxString itmsHourIntervals[]={wxT("0:00"),wxT("1:00"),wxT("2:00"),wxT("3:00"),wxT("4:00"),wxT("5:00"),wxT("6:00"),wxT("7:00"),wxT("8:00"),wxT("9:00"),wxT("10:00"),wxT("11:00"),wxT("12:00"),
		wxT("13:00"),wxT("14:00"),wxT("15:00"),wxT("16:00"),wxT("17:00"),wxT("18:00"),wxT("19:00"),wxT("20:00"),wxT("21:00"),wxT("22:00"),wxT("23:00")};
	
    cmbDWBtwnBgn=new wxComboBox(this,ID_DOWORKONLYBGNCMBBOX,wxT(""),wxPoint(206,155),wxSize(75,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	//
	cmbDWBtwnEnd=new wxComboBox(this,-1,wxT(""),wxPoint(307,155),wxSize(75,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	//
	cmbCTIBtwnBgn=new wxComboBox(this,ID_DOCONNECTONLYBGNCMBBOX,wxT(""),wxPoint(206,190),wxSize(75,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	//
	cmbCTIBtwnEnd=new wxComboBox(this,-1,wxT(""),wxPoint(307,190),wxSize(75,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	//
	wxString userValMaxUsed;
	if(m_prefs.disk_max_used_gb < 1){
        userValMaxUsed.Printf(_("%d MB"),(int)(0.1*1000));  
	}else{
        userValMaxUsed.Printf(_("%4.2f GB"),m_prefs.disk_max_used_gb); 
	}
	wxString itmsUseNoMoreGB[]={wxT("100MB"),wxT("200MB"),wxT("500MB"),wxT("1GB"),wxT("2GB"),wxT("5GB")};
	int itmUseNoMore = 6;
	if(!this->CheckIfInArray(itmsUseNoMoreGB,userValMaxUsed,itmUseNoMore)){
		wxString itmsUseNoMoreGBU[]={wxT("100MB"),wxT("200MB"),wxT("500MB"),wxT("1GB"),wxT("2GB"),wxT("5GB"),userValMaxUsed};
	    cmbUseNoMoreGB=new wxComboBox(this,-1,wxT(""),wxPoint(206,225),wxSize(75,21),itmUseNoMore+1,itmsUseNoMoreGBU,wxNO_BORDER | wxCB_READONLY);
	}else{
        cmbUseNoMoreGB=new wxComboBox(this,-1,wxT(""),wxPoint(206,225),wxSize(75,21),itmUseNoMore,itmsUseNoMoreGB,wxNO_BORDER | wxCB_READONLY);
	}
	cmbUseNoMoreGB->SetValue(userValMaxUsed);
	//
	wxString itmsDWWCInUse[]={wxT("Yes"),wxT("No")};
	cmbDWWCInUse=new wxComboBox(this,-1,wxT(""),wxPoint(206,260),wxSize(45,21),2,itmsDWWCInUse,wxNO_BORDER | wxCB_READONLY);
	if(m_prefs.run_if_user_active){
        cmbDWWCInUse->SetValue(wxT("Yes"));
	}else{
        cmbDWWCInUse->SetValue(wxT("No"));
	}
	//
	wxString userValIdleFor;
	userValIdleFor.Printf(_("%d"),(int)m_prefs.idle_time_to_run); 
	wxString itmsDWACIdleFor[]={wxString("1"),wxString("5"),wxString("10"),wxString("15"),wxString("30"),wxString("60")};
	int itmIdlCnt = 6;
	if(!this->CheckIfInArray(itmsDWACIdleFor,userValIdleFor,itmIdlCnt)){
		wxString itmsDWACIdleForU[]={wxString("1"),wxString("5"),wxString("10"),wxString("15"),wxString("30"),wxString("60"),userValIdleFor};
        cmbDWACIdleFor=new wxComboBox(this,-1,wxT(""),wxPoint(206,295),wxSize(75,21),itmIdlCnt+1,itmsDWACIdleForU,wxNO_BORDER | wxCB_READONLY);
	}else{
        cmbDWACIdleFor=new wxComboBox(this,-1,wxT(""),wxPoint(206,295),wxSize(75,21),itmIdlCnt,itmsDWACIdleFor,wxNO_BORDER | wxCB_READONLY);
	}
	cmbDWACIdleFor->SetValue(userValIdleFor);
	// Btn Save and Cancel
	wxToolTip *ttSave = new wxToolTip(_("Save preferences locally and close window"));
	btnSave=new wxBitmapButton(this,ID_SAVEBUTTON,*(pSkinSimple->GetSaveButton()->GetBitmap()),wxPoint(120,340),wxSize(57,16),wxNO_BORDER);
	btnSave->SetBitmapSelected(*(pSkinSimple->GetSaveButton()->GetBitmapClicked()));
	btnSave->SetToolTip(ttSave);

	wxToolTip *ttCancel = new wxToolTip(_("Cancel changes and close window"));
	btnCancel=new wxBitmapButton(this,ID_CANCELBUTTON,*(pSkinSimple->GetCancelButton()->GetBitmap()),wxPoint(187,340),wxSize(57,16),wxNO_BORDER);
	btnCancel->SetBitmapSelected(*(pSkinSimple->GetCancelButton()->GetBitmapClicked()));
	btnCancel->SetToolTip(ttCancel);

	wxToolTip *ttClear = new wxToolTip(_("Clear local preferences and use preferences from the web"));
	btnClear=new wxBitmapButton(this,ID_CLEARBUTTON,*(pSkinSimple->GetClearButton()->GetBitmap()),wxPoint(254,340),wxSize(57,16),wxNO_BORDER);
	btnClear->SetBitmapSelected(*(pSkinSimple->GetClearButton()->GetBitmapClicked()));
	btnClear->SetToolTip(ttClear);

	//
	ReadSettings(m_prefs);
	
	Refresh();
}


void CDlgPreferences::VwXDrawBackImg(wxEraseEvent& event, wxWindow *win, wxBitmap* bitMap, int opz) {
    wxRect rec;
    wxDC* dc;
    dc=event.GetDC();
    dc->SetBackground(wxBrush(win->GetBackgroundColour(),wxSOLID));
    dc->Clear();
    switch (opz) {
        case 0:
            dc->DrawBitmap(*bitMap, 0, 0);
            break;
        case 1:
            rec=win->GetClientRect();
            rec.SetLeft((rec.GetWidth()-bitMap->GetWidth())   / 2);
            rec.SetTop ((rec.GetHeight()-bitMap->GetHeight()) / 2);
            dc->DrawBitmap(*bitMap,rec.GetLeft(),rec.GetTop(),0);
            break;
        case 2:
            rec=win->GetClientRect();
            for(int y=0;y < rec.GetHeight();y+=bitMap->GetHeight()){
                for(int x=0;x < rec.GetWidth();x+=bitMap->GetWidth()){
                    dc->DrawBitmap(*bitMap,x,y,0);
                }
            }
            break;
    }
}


void CDlgPreferences::OnEraseBackground(wxEraseEvent& event){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    VwXDrawBackImg (
        event,
        this,
        pSkinSimple->GetPreferencesDialogBackgroundImage()->GetBitmap(),
        0
    );
}


void CDlgPreferences::OnChange(wxCommandEvent& /*event*/){
    CSkinManager* pSkinManager = wxGetApp().GetSkinManager();
    wxLocale* pLocale = wxGetApp().GetLocale();

    wxASSERT(pSkinManager);
    wxASSERT(pLocale);
    wxASSERT(wxDynamicCast(pSkinManager, CSkinManager));

    pSkinManager->ReloadSkin(pLocale, cmbSkinPicker->GetValue());
    
    EndModal(wxID_OK);
}


void CDlgPreferences::OnBtnClick(wxCommandEvent& event){ //init function
	int btnID =  event.GetId();
	if(btnID==ID_SAVEBUTTON){
        WriteSettings();
		EndModal(wxID_OK);
	} else if(btnID==ID_CHANGEBUTTON){
	    EndModal(wxID_OK);
	} else if(btnID==ID_CLEARBUTTON){
		CMainDocument* pDoc = wxGetApp().GetDocument();
        wxASSERT(pDoc);
		std::string emptyS = "";
		pDoc->rpc.set_global_prefs_override(emptyS);
		// possible re read of global prefs to client
		EndModal(wxID_CANCEL);
	}else{
		//wxMessageBox("OnBtnClick - btnCancel");
		EndModal(wxID_CANCEL);
	}
} //end function

bool CDlgPreferences::CheckIfInArray(wxString valArray[],wxString value,int size){
	
	for(int x=0; x<size; x++){
		wxString val = valArray[x];
		if(valArray[x] == value){
			return true;
			break;
		}
	}
	return false;
}

void CDlgPreferences::WriteSettings(){

	CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    m_prefs = pDoc->state.global_prefs;
    
	int dwBtwnIntB = ConvertToNumber(cmbDWBtwnBgn->GetValue());
    int dwBtwnIntE = ConvertToNumber(cmbDWBtwnEnd->GetValue());
	int ctiBtwnIntB = ConvertToNumber(cmbCTIBtwnBgn->GetValue());
	int ctiBtwnIntE = ConvertToNumber(cmbCTIBtwnEnd->GetValue());
	
	m_prefs.start_hour = dwBtwnIntB;
	m_prefs.end_hour = dwBtwnIntE;
	m_prefs.net_start_hour = ctiBtwnIntB;
	m_prefs.net_end_hour = ctiBtwnIntE;
    // Use No More Than GB
	wxString useNoMoreGB = cmbUseNoMoreGB->GetValue();
    wxString useNoMoreGBNum = useNoMoreGB.Mid(0,useNoMoreGB.Length()-2);
    wxString valGBMB = useNoMoreGB.Mid(useNoMoreGB.Length()-2);
	double dblUseNoMoreGB = 0;
	if(valGBMB == wxString("GB")){
		if(useNoMoreGBNum.ToDouble(&dblUseNoMoreGB)){
			m_prefs.disk_max_used_gb = dblUseNoMoreGB;
		}
	}else{
		if(useNoMoreGBNum.ToDouble(&dblUseNoMoreGB)){
			m_prefs.disk_max_used_gb = dblUseNoMoreGB/1000;
		}
	}
	//Do Work While Comp In Use
	wxString doWorkWhileInUse = cmbDWWCInUse->GetValue();
	if(doWorkWhileInUse == wxString("Yes")){
		m_prefs.run_if_user_active = true;
	}else{
		m_prefs.run_if_user_active = false;
	}  
    // Do work after connection idle
	wxString doWorkConnIdleFor = cmbDWACIdleFor->GetValue();
	double dblDoWorkConnIdleFor = 0;
    if(doWorkConnIdleFor.ToDouble(&dblDoWorkConnIdleFor)){
		m_prefs.idle_time_to_run = dblDoWorkConnIdleFor;
	}
	//
	pDoc->rpc.set_global_prefs_override_struct(m_prefs);
	pDoc->rpc.read_global_prefs_override();

}

void CDlgPreferences::ReadSettings(GLOBAL_PREFS prefs){
	wxString hoursStrVal[]={wxT("0:00"),wxT("1:00"),wxT("2:00"),wxT("3:00"),wxT("4:00"),wxT("5:00"),wxT("6:00"),wxT("7:00"),wxT("8:00"),wxT("9:00"),wxT("10:00"),wxT("11:00"),wxT("12:00"),
		wxT("13:00"),wxT("14:00"),wxT("15:00"),wxT("16:00"),wxT("17:00"),wxT("18:00"),wxT("19:00"),wxT("20:00"),wxT("21:00"),wxT("22:00"),wxT("23:00")};

	cmbDWBtwnBgn->SetValue(hoursStrVal[prefs.start_hour]);
	//
	cmbDWBtwnEnd->SetValue(hoursStrVal[prefs.end_hour]);
	//
	cmbCTIBtwnBgn->SetValue(hoursStrVal[prefs.net_start_hour]);
	//
	cmbCTIBtwnEnd->SetValue(hoursStrVal[prefs.net_end_hour]);
	//

}

int CDlgPreferences::ConvertToNumber(wxString num){
	wxString hoursStrVal[]={wxT("0:00"),wxT("1:00"),wxT("2:00"),wxT("3:00"),wxT("4:00"),wxT("5:00"),wxT("6:00"),wxT("7:00"),wxT("8:00"),wxT("9:00"),wxT("10:00"),wxT("11:00"),wxT("12:00"),
		wxT("13:00"),wxT("14:00"),wxT("15:00"),wxT("16:00"),wxT("17:00"),wxT("18:00"),wxT("19:00"),wxT("20:00"),wxT("21:00"),wxT("22:00"),wxT("23:00")};
	
	for(int ind = 0; ind < 24; ind++){
		if(hoursStrVal[ind] == num){
			return ind;
		}
	}
	return -1;
}

void CDlgPreferences::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
 	dc.SetFont(wxFont(16,74,90,90,0,wxT("Arial"))); 
	dc.DrawText(_("Skin"), wxPoint(16,10)); 
	dc.SetFont(wxFont(9,74,90,90,0,wxT("Arial")));
	dc.DrawText(_("Skin XML file:"), wxPoint(133,40));
 	dc.SetFont(wxFont(16,74,90,90,0,wxT("Arial"))); 
	dc.DrawText(_("Preferences"), wxPoint(16,122)); 
	dc.SetFont(wxFont(8,74,90,90,0,wxT("Arial")));
	dc.DrawText(m_PrefIndicator, wxPoint(272,128));
	dc.SetFont(wxFont(9,74,90,90,0,wxT("Arial")));
	dc.DrawText(_("Do work only between:"), wxPoint(82,158));
    dc.DrawText(_("and"), wxPoint(284,158));
	dc.DrawText(_("Connect to internet only between:"), wxPoint(24,193));
	dc.DrawText(_("and"), wxPoint(284,193));
	dc.DrawText(_("Use no more than:"), wxPoint(103,228));
	dc.DrawText(_("of disk space"), wxPoint(284,228));
    dc.DrawText(_("Do work while computer is in use?"), wxPoint(16,263));
    dc.DrawText(_("Do work after computer is idle for:"), wxPoint(22,298));
    dc.DrawText(_("minutes"), wxPoint(284,298));
}


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
#include "BOINCGUIApp.h"
#include "sg_DlgPreferences.h"
#include "sg_SkinClass.h"
//#include <wx/dir.h> 

enum 
{ 
    ID_OPENBUTTON = 10001, 
    ID_SAVEBUTTON = 10002, 
    ID_CANCELBUTTON = 10003,
	ID_CLEARBUTTON = 10007,
	ID_SKINPICKERCMBBOX = 10004, 
	ID_DOWORKONLYBGNCMBBOX = 10005,
	ID_DOCONNECTONLYBGNCMBBOX = 10006,
}; 

BEGIN_EVENT_TABLE( CDlgPreferences,wxDialog)
  EVT_PAINT(CDlgPreferences::OnPaint)
  EVT_COMBOBOX(-1,CDlgPreferences::OnCmbSelected)
  EVT_BUTTON(-1,CDlgPreferences::OnBtnClick)
  EVT_ERASE_BACKGROUND(CDlgPreferences::OnEraseBackground)
END_EVENT_TABLE()
// end events

CDlgPreferences::CDlgPreferences(wxWindow* parent, wxString dirPref,wxWindowID id,const wxString& title,const wxPoint& pos,const wxSize& size,long style,const wxString& name)
{
	m_SkinDirPrefix = dirPref;
	m_skinNames.Add(wxT("default"));
	Create(parent,id,title,pos,size,style,name);

	if((pos==wxDefaultPosition)&&(size==wxDefaultSize)){
		SetSize(0,0,416,403);
	}

	if((pos!=wxDefaultPosition)&&(size==wxDefaultSize)){
		SetSize(416,403);
	}
	//
	m_PrefIndicator = wxT("");
	m_globalPrefUsed = true;
	initBefore();
	// load images from skin file
	LoadSkinImages();
	//Create dialog
	CreateDialog();
	initAfter();
}
CDlgPreferences::~CDlgPreferences()
{

}


void CDlgPreferences::CreateDialog()
{
	CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);

	m_prefs = pDoc->state.global_prefs;

	SetBackgroundColour(appSkin->GetAppBgCol());
	wxString itmsHourIntervals[]={wxT("0:00"),wxT("1:00"),wxT("2:00"),wxT("3:00"),wxT("4:00"),wxT("5:00"),wxT("6:00"),wxT("7:00"),wxT("8:00"),wxT("9:00"),wxT("10:00"),wxT("11:00"),wxT("12:00"),
		wxT("13:00"),wxT("14:00"),wxT("15:00"),wxT("16:00"),wxT("17:00"),wxT("18:00"),wxT("19:00"),wxT("20:00"),wxT("21:00"),wxT("22:00"),wxT("23:00")};
	
    cmbDWBtwnBgn=new wxComboBox(this,ID_DOWORKONLYBGNCMBBOX,wxT(""),wxPoint(206,90),wxSize(75,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	//
	cmbDWBtwnEnd=new wxComboBox(this,-1,wxT(""),wxPoint(307,90),wxSize(75,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	//
	cmbCTIBtwnBgn=new wxComboBox(this,ID_DOCONNECTONLYBGNCMBBOX,wxT(""),wxPoint(206,125),wxSize(75,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	//
	cmbCTIBtwnEnd=new wxComboBox(this,-1,wxT(""),wxPoint(307,125),wxSize(75,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	//
	wxString userValMaxUsed;
	if(m_prefs.disk_max_used_gb < 1){
        userValMaxUsed.Printf(_("%d MB"),(int)(0.1*1000));  
	}else{
        userValMaxUsed.Printf(_("%4.2f GB"),m_prefs.disk_max_used_gb); 
	}
	wxString itmsUseNoMoreGB[]={wxT("100MB"),wxT("200MB"),wxT("500MB"),wxT("1GB"),wxT("2GB"),wxT("5GB"),userValMaxUsed};
	cmbUseNoMoreGB=new wxComboBox(this,-1,wxT(""),wxPoint(206,160),wxSize(75,21),7,itmsUseNoMoreGB,wxNO_BORDER | wxCB_READONLY);
	cmbUseNoMoreGB->SetValue(userValMaxUsed);
	//
	wxString itmsDWWCInUse[]={wxT("Yes"),wxT("No")};
	cmbDWWCInUse=new wxComboBox(this,-1,wxT(""),wxPoint(206,195),wxSize(45,21),2,itmsDWWCInUse,wxNO_BORDER | wxCB_READONLY);
	if(m_prefs.run_if_user_active){
        cmbDWWCInUse->SetValue(wxT("Yes"));
	}else{
        cmbDWWCInUse->SetValue(wxT("No"));
	}
	//
	wxString userValIdleFor;
	userValIdleFor.Printf(_("%d"),(int)m_prefs.idle_time_to_run); 
	wxString itmsDWACIdleFor[]={wxString("1"),wxString("5"),wxString("10"),wxString("15"),wxString("30"),wxString("60")};
	int lngth = itmsDWACIdleFor->Len();
	bool contains = itmsDWACIdleFor->Contains(wxString("5"));
	if(!this->CheckIfInArray(itmsDWACIdleFor,userValIdleFor,6)){
		wxString itmsDWACIdleFor[]={wxT("1"),wxT("5"),wxT("10"),wxT("15"),wxT("30"),wxT("60"),userValIdleFor};
	}
	cmbDWACIdleFor=new wxComboBox(this,-1,wxT(""),wxPoint(206,230),wxSize(75,21),7,itmsDWACIdleFor,wxNO_BORDER | wxCB_READONLY);
	cmbDWACIdleFor->SetValue(userValIdleFor);
    //
	cmbSkinPicker=new wxComboBox(this,ID_SKINPICKERCMBBOX,wxT(""),wxPoint(206,265),wxSize(140,21),m_skinNames,wxNO_BORDER | wxCB_READONLY);
	cmbSkinPicker->SetValue(m_SkinName);
	// Btn Save and Cancel
	wxToolTip *ttSave = new wxToolTip(wxT("Save preferences"));
	btnSave=new wxBitmapButton(this,ID_SAVEBUTTON,btmpSave,wxPoint(120,325),wxSize(57,16),wxNO_BORDER);
	btnSave->SetBitmapSelected(btmpSaveClick);
	btnSave->SetToolTip(ttSave);

	wxToolTip *ttCancel = new wxToolTip(wxT("Cancel changes"));
	btnCancel=new wxBitmapButton(this,ID_CANCELBUTTON,btmpCancel,wxPoint(187,325),wxSize(57,16),wxNO_BORDER);
	btnCancel->SetBitmapSelected(btmpCancelClick);
	btnCancel->SetToolTip(ttCancel);

	wxToolTip *ttClear = new wxToolTip(wxT("Clear saved preferences"));
	btnClear=new wxBitmapButton(this,ID_CLEARBUTTON,btmpClear,wxPoint(254,325),wxSize(57,16),wxNO_BORDER);
	btnClear->SetBitmapSelected(btmpClearClick);
	btnClear->SetToolTip(ttClear);

	// populate values from prefs
	/// 
	if(pDoc->rpc.get_global_prefs_override_struct(m_prefs) == 0){
		m_PrefIndicator = wxT("Using local preferences");
		m_globalPrefUsed = false;
	}else{
        m_PrefIndicator = wxT("Using global preferences");
		m_globalPrefUsed = true;
	}
	//
	ReadSettings(m_prefs);
	
	Refresh();
}
void CDlgPreferences::LoadSkinImages(){
    
	fileImgBuf[0].LoadFile(m_SkinDirPrefix + appSkin->GetDlgPrefBg(),wxBITMAP_TYPE_PNG);
	
	// save
	g_save = new wxImage(m_SkinDirPrefix + appSkin->GetBtnSave(), wxBITMAP_TYPE_PNG);
	g_saveClick = new wxImage(m_SkinDirPrefix + appSkin->GetBtnSaveClick(), wxBITMAP_TYPE_PNG);
    // cancel
	g_cancel = new wxImage(m_SkinDirPrefix + appSkin->GetBtnCancel(), wxBITMAP_TYPE_PNG);
	g_cancelClick = new wxImage(m_SkinDirPrefix + appSkin->GetBtnCancelClick(), wxBITMAP_TYPE_PNG);
    
	// clear
	g_clear = new wxImage(m_SkinDirPrefix + appSkin->GetBtnClear(), wxBITMAP_TYPE_PNG);
	g_clearClick = new wxImage(m_SkinDirPrefix + appSkin->GetBtnClearClick(), wxBITMAP_TYPE_PNG);
   
	dlgBack=&fileImgBuf[0];
	btmpSave= wxBitmap(g_save); 
    btmpSaveClick= wxBitmap(g_saveClick); 
	btmpCancel= wxBitmap(g_cancel); 
	btmpClearClick= wxBitmap(g_saveClick); 
	btmpClear= wxBitmap(g_clear); 
    btmpClearClick= wxBitmap(g_clearClick); 
}
void CDlgPreferences::VwXDrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz){
 event.Skip(false);wxDC *dc;
 dc=event.GetDC();
 dc->SetBackground(wxBrush(win->GetBackgroundColour(),wxSOLID));
 dc->Clear();
 switch (opz) {
  case 0:{
         dc->DrawBitmap(bitMap, 0, 0);
         break;}
  case 1:{
         wxRect rec=win->GetClientRect();
         rec.SetLeft((rec.GetWidth()-bitMap.GetWidth())   / 2);
         rec.SetTop ((rec.GetHeight()-bitMap.GetHeight()) / 2);
         dc->DrawBitmap(bitMap,rec.GetLeft(),rec.GetTop(),0);
         break;}
  case 2:{
         wxRect rec=win->GetClientRect();
         for(int y=0;y < rec.GetHeight();y+=bitMap.GetHeight()){
           for(int x=0;x < rec.GetWidth();x+=bitMap.GetWidth()){
             dc->DrawBitmap(bitMap,x,y,0);
           }
         }
         break;}
 }
}
void CDlgPreferences::OnEraseBackground(wxEraseEvent& event){
 wxObject *m_wxWin = event.GetEventObject();
 if(m_wxWin==this){event.Skip(true);VwXDrawBackImg(event,this,*dlgBack,0);VwXEvOnEraseBackground(event) ;return;}
 event.Skip(true);
}

void CDlgPreferences::OnBtnClick(wxCommandEvent& event){ //init function
    wxObject *m_wxDlgBtnObj = event.GetEventObject();
	int btnID =  event.GetId();
	if(btnID==ID_SAVEBUTTON){
		//wxMessageBox("OnBtnClick - btnSave");
        CheckSettings();
		EndModal(wxID_OK);
	}
	else if(btnID==ID_CLEARBUTTON){
		btnClear->Refresh();
	}else{
		//wxMessageBox("OnBtnClick - btnCancel");
		EndModal(wxID_CANCEL);
	}
} //end function

void CDlgPreferences::OnCmbSelected(wxCommandEvent& event){ //init function
    wxObject *m_wxDlgCmbObj = event.GetEventObject();
	int cmbID =  event.GetId();
	if(cmbID==ID_SKINPICKERCMBBOX){
		m_SkinName = event.GetString();
	}
} //end function

void CDlgPreferences::VwXEvOnEraseBackground(wxEraseEvent& WXUNUSED(event)){ //init function
 //[29b]Code event VwX...Don't modify[29b]//
 //add your code here

} //end function

void CDlgPreferences::initBefore(){
	//get skin class
	appSkin = SkinClass::Instance();
	#ifdef __WXMSW__ 
        wxString separator = '\\';
    #else 
        wxString separator = '/'; 
    #endif 
	wxString currentDir = wxGetCwd();
    wxString currentSkinsDir = currentDir + separator +appSkin->GetSkinsFolder();
	if(wxDir::Exists(currentSkinsDir)) { 
	  // get the names of all directories in skins dir
      DirTraverserSkins skinTraverser(m_skinNames);
	  wxDir skinsDir(currentSkinsDir);
	  skinsDir.Traverse(skinTraverser);
    }
	//finally set the name of the current skin
	m_SkinName = appSkin->GetSkinName();
}

bool CDlgPreferences::CheckIfInArray(wxString valArray[],wxString value,int size){
	
	for(int x=0; x<size; x++){
		//if(valArray->Contains
	}

	return true;

}
void CDlgPreferences::CheckSettings(){

	CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    bool valuesChanged = false;
    m_prefs = pDoc->state.global_prefs;
    
	if(cmbDWBtwnBgn->GetValue() == wxString("Always")){
		if((pDoc->state.global_prefs.start_hour !=0) && (pDoc->state.global_prefs.end_hour !=0)){

		}
	}else if(cmbDWBtwnBgn->GetValue()== cmbDWBtwnEnd->GetValue()){
		//if
	}
	
	//pDoc->rpc.get_global_prefs_override_struct(m_prefs) == 0)
	int rf = 9;


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
	
	for(int ind = 0; ind < hoursStrVal->Length(); ind++){
		if(hoursStrVal[ind] == num){
			return ind;
			break;
		}
	}
	return -1;
}
void CDlgPreferences::initAfter(){
 //add your code here
    Centre();
}

void CDlgPreferences::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
    //Project Name
	dc.SetFont(wxFont(16,74,90,90,0,wxT("Arial"))); 
	dc.DrawText(wxT("Preferences"), wxPoint(16,10)); 
	dc.SetFont(wxFont(8,74,90,90,0,wxT("Arial")));
	dc.DrawText(m_PrefIndicator, wxPoint(272,16));
	//static: APPLICATION,MY PROGRESS,ELAPSED TIME,TIME REMAINING
	dc.SetFont(wxFont(9,74,90,90,0,wxT("Arial")));
	dc.DrawText(wxT("Modify settings for this computer:"), wxPoint(26,60));
	dc.DrawText(wxT("Do work only between:"), wxPoint(82,93));
    dc.DrawText(wxT("and"), wxPoint(284,93));
	dc.DrawText(wxT("Connect to internet only between:"), wxPoint(24,128));
	dc.DrawText(wxT("and"), wxPoint(284,128));
	dc.DrawText(wxT("Use no more than:"), wxPoint(103,163));
    dc.DrawText(wxT("Do work while computer is in use?"), wxPoint(16,198));
    dc.DrawText(wxT("Do work after computer is idle for:"), wxPoint(22,233));
    dc.DrawText(wxT("minutes"), wxPoint(284,233));
	dc.DrawText(wxT("Skin XML file:"), wxPoint(133,268));
	
	

}
//[evtFunc]end your code
wxDirTraverseResult DirTraverserSkins::OnFile(const wxString& filename)
{
    return wxDIR_CONTINUE;
}
wxDirTraverseResult DirTraverserSkins::OnDir(const wxString& dirname)
{
	#ifdef __WXMSW__ 
        char separator = '\\';
    #else 
        char separator = '/'; 
    #endif 
		
	wxString skinName = dirname.AfterLast(separator);
	if(skinName != wxT("default")){
		m_skins.Add(skinName);
	}
	
    return wxDIR_IGNORE;
}



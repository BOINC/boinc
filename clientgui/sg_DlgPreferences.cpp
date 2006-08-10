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
		SetSize(0,0,400,400);
	}

	if((pos!=wxDefaultPosition)&&(size==wxDefaultSize)){
		SetSize(400,400);
	}
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
	SetBackgroundColour(appSkin->GetAppBgCol());
	wxString itmsHourIntervals[]={wxT("Always"),wxT("12:00 AM"),wxT("1:00 AM"),wxT("2:00 AM"),wxT("3:00 AM"),wxT("4:00 AM"),wxT("5:00 AM"),wxT("6:00 AM"),wxT("7:00 AM"),wxT("8:00 AM"),wxT("9:00 AM"),wxT("10:00 AM"),wxT("11:00 AM"),wxT("12:00 PM"),
		wxT("13:00 PM"),wxT("14:00 PM"),wxT("15:00 PM"),wxT("16:00 PM"),wxT("17:00 PM"),wxT("18:00 PM"),wxT("19:00 PM"),wxT("20:00 PM"),wxT("21:00 PM"),wxT("22:00 PM"),wxT("23:00 PM")};
	wxString itmsHourIntervalsNoAlways[]={wxT("12:00 AM"),wxT("1:00 AM"),wxT("2:00 AM"),wxT("3:00 AM"),wxT("4:00 AM"),wxT("5:00 AM"),wxT("6:00 AM"),wxT("7:00 AM"),wxT("8:00 AM"),wxT("9:00 AM"),wxT("10:00 AM"),wxT("11:00 AM"),wxT("12:00 PM"),
		wxT("13:00 PM"),wxT("14:00 PM"),wxT("15:00 PM"),wxT("16:00 PM"),wxT("17:00 PM"),wxT("18:00 PM"),wxT("19:00 PM"),wxT("20:00 PM"),wxT("21:00 PM"),wxT("22:00 PM"),wxT("23:00 PM")};
	

	//lblPref=new wxStaticText(this,-1,wxT(""),wxPoint(10,10),wxSize(84,18),wxST_NO_AUTORESIZE);
	//lblPref->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblPref->SetLabel(wxT("Preferences"));
	//lblPref->SetFont(wxFont(12,74,90,90,0,wxT("Tahoma")));
	// Modify settings for this computer
	//lblModifySett=new wxStaticText(this,-1,wxT(""),wxPoint(15,60),wxSize(164,13),wxST_NO_AUTORESIZE);
	//lblModifySett->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblModifySett->SetLabel(wxT("Modify settings for this computer:"));
	// Do work only between
	//lblDoWorkBtwn=new wxStaticText(this,-1,wxT(""),wxPoint(65,95),wxSize(114,13),wxST_NO_AUTORESIZE);
	//lblDoWorkBtwn->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblDoWorkBtwn->SetLabel(wxT("Do work only between:"));
    cmbDWBtwnBgn=new wxComboBox(this,ID_DOWORKONLYBGNCMBBOX,wxT(""),wxPoint(185,90),wxSize(85,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	cmbDWBtwnBgn->SetValue(wxT("Always"));
	//lblAnd1=new wxStaticText(this,-1,wxT(""),wxPoint(270,95),wxSize(19,13),wxST_NO_AUTORESIZE);
	//lblAnd1->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblAnd1->SetLabel(wxT("and"));
	//lblAnd1->Show(false);
	cmbDWBtwnEnd=new wxComboBox(this,-1,wxT(""),wxPoint(300,90),wxSize(90,21),24,itmsHourIntervalsNoAlways,wxNO_BORDER | wxCB_READONLY);
	cmbDWBtwnEnd->SetValue(wxT("12:00 AM"));
	///Connect to internet only between
	//lblConnToIntBtwn=new wxStaticText(this,-1,wxT(""),wxPoint(10,130),wxSize(169,13),wxST_NO_AUTORESIZE);
	//lblConnToIntBtwn->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblConnToIntBtwn->SetLabel(wxT("Connect to internet only between:"));
	cmbCTIBtwnBgn=new wxComboBox(this,ID_DOCONNECTONLYBGNCMBBOX,wxT(""),wxPoint(185,125),wxSize(85,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	cmbCTIBtwnBgn->SetValue(wxT("Always"));
	//lblAnd2=new wxStaticText(this,-1,wxT(""),wxPoint(270,130),wxSize(19,13),wxST_NO_AUTORESIZE);
	//lblAnd2->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblAnd2->SetLabel(wxT("and"));
	cmbCTIBtwnEnd=new wxComboBox(this,-1,wxT(""),wxPoint(300,125),wxSize(90,21),24,itmsHourIntervalsNoAlways,wxNO_BORDER | wxCB_READONLY);
	cmbCTIBtwnEnd->SetValue(wxT("12:00 AM"));
	///Use no more than
	//lblUseNoMoreGB=new wxStaticText(this,-1,wxT(""),wxPoint(85,165),wxSize(94,13),wxST_NO_AUTORESIZE);
	//lblUseNoMoreGB->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblUseNoMoreGB->SetLabel(wxT("Use no more than:"));
	wxString itmsUseNoMoreGB[]={wxT("100MB"),wxT("200MB"),wxT("500MB"),wxT("1GB"),wxT("2GB"),wxT("5GB")};
	cmbUseNoMoreGB=new wxComboBox(this,-1,wxT(""),wxPoint(185,160),wxSize(85,21),5,itmsUseNoMoreGB,wxNO_BORDER | wxCB_READONLY);
	cmbUseNoMoreGB->SetValue(wxT("500MB"));
	//lblGB=new wxStaticText(this,-1,wxT(""),wxPoint(270,165),wxSize(14,13),wxST_NO_AUTORESIZE);
	//lblGB->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblGB->SetLabel(wxT("GB"));
	/// Do work while computer is in use
	//lblDWWCInUse=new wxStaticText(this,-1,wxT(""),wxPoint(15,200),wxSize(164,13),wxST_NO_AUTORESIZE);
	//lblDWWCInUse->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblDWWCInUse->SetLabel(wxT("Do work while computer is in use?"));
	wxString itmsDWWCInUse[]={wxT("Yes"),wxT("No")};
	cmbDWWCInUse=new wxComboBox(this,-1,wxT(""),wxPoint(185,195),wxSize(45,21),2,itmsDWWCInUse,wxNO_BORDER | wxCB_READONLY);
	cmbDWWCInUse->SetValue(wxT("Yes"));
	///Do work after computer is idle for
	//lblDWACIdleFor=new wxStaticText(this,-1,wxT(""),wxPoint(15,235),wxSize(164,13),wxST_NO_AUTORESIZE);
	//lblDWACIdleFor->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblDWACIdleFor->SetLabel(wxT("Do work after computer is idle for:"));
	wxString itmsDWACIdleFor[]={wxT("1"),wxT("5"),wxT("10"),wxT("15"),wxT("30"),wxT("60")};
	cmbDWACIdleFor=new wxComboBox(this,-1,wxT(""),wxPoint(185,230),wxSize(85,21),5,itmsDWACIdleFor,wxNO_BORDER | wxCB_READONLY);
	cmbDWACIdleFor->SetValue(wxT("5"));
	//lblMinutes=new wxStaticText(this,-1,wxT(""),wxPoint(270,235),wxSize(39,13),wxST_NO_AUTORESIZE);
	//lblMinutes->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblMinutes->SetLabel(wxT("minutes"));
	////Skin XML file
	//lblSkinXML=new wxStaticText(this,-1,wxT(""),wxPoint(115,270),wxSize(64,13),wxST_NO_AUTORESIZE);
	//lblSkinXML->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	//lblSkinXML->SetLabel(wxT("Skin XML file:"));
	//skin picker control
	cmbSkinPicker=new wxComboBox(this,ID_SKINPICKERCMBBOX,wxT(""),wxPoint(185,265),wxSize(140,21),m_skinNames,wxNO_BORDER | wxCB_READONLY);
	cmbSkinPicker->SetValue(m_SkinName);
	// Btn Save and Cancel
	wxToolTip *ttSave = new wxToolTip(wxT("Save preferences"));
	btnSave=new wxBitmapButton(this,ID_SAVEBUTTON,btmpSave,wxPoint(115,325),wxSize(57,16),wxNO_BORDER);
	btnSave->SetBitmapSelected(btmpSaveClick);
	btnSave->SetToolTip(ttSave);

	wxToolTip *ttCancel = new wxToolTip(wxT("Cancel changes"));
	btnCancel=new wxBitmapButton(this,ID_CANCELBUTTON,btmpCancel,wxPoint(182,325),wxSize(57,16),wxNO_BORDER);
	btnCancel->SetBitmapSelected(btmpCancelClick);
	btnCancel->SetToolTip(ttCancel);

	wxToolTip *ttClear = new wxToolTip(wxT("Clear saved preferences"));
	btnClear=new wxBitmapButton(this,ID_CLEARBUTTON,btmpClear,wxPoint(249,325),wxSize(57,16),wxNO_BORDER);
	btnClear->SetBitmapSelected(btmpClearClick);
	btnClear->SetToolTip(ttClear);
	
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
	}else if(cmbID==ID_DOWORKONLYBGNCMBBOX){
		if(event.GetString() == wxT("Always")){
			cmbDWBtwnEnd->Show(false);
		}else{
			cmbDWBtwnEnd->Show(true);
		}
	}else if(cmbID==ID_DOCONNECTONLYBGNCMBBOX){
		if(event.GetString() == wxT("Always")){
			cmbCTIBtwnEnd->Show(false);
		}else{
			cmbCTIBtwnEnd->Show(true);
		}
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

void CDlgPreferences::initAfter(){
 //add your code here
    Centre();
}

void CDlgPreferences::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
    //Project Name
	dc.SetFont(wxFont(16,74,90,90,0,wxT("Arial"))); 
	dc.DrawText(wxT("Preferences"), wxPoint(10,10)); 
	//static: APPLICATION,MY PROGRESS,ELAPSED TIME,TIME REMAINING
	dc.SetFont(wxFont(9,74,90,90,0,wxT("Arial")));
	dc.DrawText(wxT("Modify settings for this computer:"), wxPoint(10,60));
	dc.DrawText(wxT("Do work only between:"), wxPoint(60,93));
    dc.DrawText(wxT("and"), wxPoint(275,93));
	dc.DrawText(wxT("Connect to internet only between:"), wxPoint(5,128));
	dc.DrawText(wxT("and"), wxPoint(275,130));
	dc.DrawText(wxT("Use no more than:"), wxPoint(85,163));
    dc.DrawText(wxT("GB"), wxPoint(275,165));
    dc.DrawText(wxT("Do work while computer is in use?"), wxPoint(5,198));
    dc.DrawText(wxT("Do work after computer is idle for:"), wxPoint(5,233));
    dc.DrawText(wxT("minutes"), wxPoint(275,233));
	dc.DrawText(wxT("Skin XML file:"), wxPoint(115,268));
	
	

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



#include "stdwx.h"

// Don't modify comment 
#include "sg_DlgPreferences.h"
#include "sg_SkinClass.h"
//[inc]add your include files here

//[inc]end your include
enum 
{ 
    ID_OPENBUTTON = 10001, 
    ID_SAVEBUTTON = 10002, 
    ID_CANCELBUTTON = 10003,
	ID_SKINPICKERCMBBOX = 10004, 
}; 

BEGIN_EVENT_TABLE( CDlgPreferences,wxDialog)
  EVT_COMBOBOX(-1,CDlgPreferences::OnCmbSelected)
  EVT_BUTTON(-1,CDlgPreferences::OnBtnClick)
  EVT_ERASE_BACKGROUND(CDlgPreferences::OnEraseBackground)
END_EVENT_TABLE()
// end events

CDlgPreferences::CDlgPreferences(wxWindow* parent, wxString dirPref,wxWindowID id,const wxString& title,const wxPoint& pos,const wxSize& size,long style,const wxString& name)
{
 m_SkinDirPrefix = dirPref;
 OnPreCreate();
 Create(parent,id,title,pos,size,style,name);

 if((pos==wxDefaultPosition)&&(size==wxDefaultSize)){
     SetSize(0,0,400,450);
 }

 if((pos!=wxDefaultPosition)&&(size==wxDefaultSize)){
     SetSize(400,450);
 }
 initBefore();
 // load images from skin file
 LoadSkinImages();
 //Create dialog
 InitDialog();
 initAfter();
}
CDlgPreferences::~CDlgPreferences()
{

}
wxPoint& CDlgPreferences::SetwxPoint(long x,long y){
  m_tmppoint.x=x;
  m_tmppoint.y=y;
  return m_tmppoint;
}

wxSize& CDlgPreferences::SetwxSize(long w,long h){
  m_tmpsize.SetWidth(w);
  m_tmpsize.SetHeight(h);
  return m_tmpsize;
}

void CDlgPreferences::InitDialog()
{
	SetBackgroundColour(appSkin->GetAppBgCol());

	lblPref=new wxStaticText(this,-1,wxT(""),SetwxPoint(10,10),SetwxSize(84,18),wxST_NO_AUTORESIZE);
	lblPref->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblPref->SetLabel(wxT("Preferences"));
	lblPref->SetFont(wxFont(12,74,90,90,0,wxT("Tahoma")));
	// Modify settings for this computer
	lblModifySett=new wxStaticText(this,-1,wxT(""),SetwxPoint(15,60),SetwxSize(164,13),wxST_NO_AUTORESIZE);
	lblModifySett->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblModifySett->SetLabel(wxT("Modify settings for this computer:"));
	// Do work only between
	lblDoWorkBtwn=new wxStaticText(this,-1,wxT(""),SetwxPoint(65,95),SetwxSize(114,13),wxST_NO_AUTORESIZE);
	lblDoWorkBtwn->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblDoWorkBtwn->SetLabel(wxT("Do work only between:"));
    wxString itmsHourIntervals[]={wxT("Always"),wxT("12:00 AM"),wxT("1:00 AM"),wxT("2:00 AM"),wxT("3:00 AM"),wxT("4:00 AM"),wxT("5:00 AM"),wxT("6:00 AM"),wxT("7:00 AM"),wxT("8:00 AM"),wxT("9:00 AM"),wxT("10:00 AM"),wxT("11:00 AM"),wxT("12:00 PM"),
		wxT("13:00 PM"),wxT("14:00 PM"),wxT("15:00 PM"),wxT("16:00 PM"),wxT("17:00 PM"),wxT("18:00 PM"),wxT("19:00 PM"),wxT("20:00 PM"),wxT("21:00 PM"),wxT("22:00 PM"),wxT("23:00 PM")};
	cmbDWBtwnBgn=new wxComboBox(this,-1,wxT(""),SetwxPoint(180,90),SetwxSize(85,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	cmbDWBtwnBgn->SetValue(wxT("Always"));
	lblAnd1=new wxStaticText(this,-1,wxT(""),SetwxPoint(270,95),SetwxSize(19,13),wxST_NO_AUTORESIZE);
	lblAnd1->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblAnd1->SetLabel(wxT("and"));
	lblAnd1->Show(false);
	cmbDWBtwnEnd=new wxComboBox(this,-1,wxT(""),SetwxPoint(295,90),SetwxSize(85,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	cmbDWBtwnEnd->SetValue(wxT("Always"));
	cmbDWBtwnEnd->Show(false);
	///Connect to internet only between
	lblConnToIntBtwn=new wxStaticText(this,-1,wxT(""),SetwxPoint(10,130),SetwxSize(169,13),wxST_NO_AUTORESIZE);
	lblConnToIntBtwn->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblConnToIntBtwn->SetLabel(wxT("Connect to internet only between:"));
	cmbCTIBtwnBgn=new wxComboBox(this,-1,wxT(""),SetwxPoint(180,125),SetwxSize(85,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	cmbCTIBtwnBgn->SetValue(wxT("Always"));
	lblAnd2=new wxStaticText(this,-1,wxT(""),SetwxPoint(270,130),SetwxSize(19,13),wxST_NO_AUTORESIZE);
	lblAnd2->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblAnd2->SetLabel(wxT("and"));
	lblAnd2->Show(false);
	cmbCTIBtwnEnd=new wxComboBox(this,-1,wxT(""),SetwxPoint(295,125),SetwxSize(85,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	cmbCTIBtwnEnd->SetValue(wxT("Always"));
	cmbCTIBtwnEnd->Show(false);
	///Use no more than
	lblUseNoMoreGB=new wxStaticText(this,-1,wxT(""),SetwxPoint(85,165),SetwxSize(94,13),wxST_NO_AUTORESIZE);
	lblUseNoMoreGB->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblUseNoMoreGB->SetLabel(wxT("Use no more than:"));
	wxString itmsUseNoMoreGB[]={wxT("100MB"),wxT("200MB"),wxT("500MB"),wxT("1GB"),wxT("2GB"),wxT("5GB")};
	cmbUseNoMoreGB=new wxComboBox(this,-1,wxT(""),SetwxPoint(180,160),SetwxSize(85,21),5,itmsUseNoMoreGB,wxNO_BORDER | wxCB_READONLY);
	cmbUseNoMoreGB->SetValue(wxT("500MB"));
	lblGB=new wxStaticText(this,-1,wxT(""),SetwxPoint(270,165),SetwxSize(14,13),wxST_NO_AUTORESIZE);
	lblGB->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblGB->SetLabel(wxT("GB"));
	/// Do work while computer is in use
	lblDWWCInUse=new wxStaticText(this,-1,wxT(""),SetwxPoint(15,200),SetwxSize(164,13),wxST_NO_AUTORESIZE);
	lblDWWCInUse->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblDWWCInUse->SetLabel(wxT("Do work while computer is in use?"));
	wxString itmsDWWCInUse[]={wxT("Yes"),wxT("No")};
	cmbDWWCInUse=new wxComboBox(this,-1,wxT(""),SetwxPoint(180,195),SetwxSize(45,21),2,itmsDWWCInUse,wxNO_BORDER | wxCB_READONLY);
	cmbDWWCInUse->SetValue(wxT("Yes"));
	///Do work after computer is idle for
	lblDWACIdleFor=new wxStaticText(this,-1,wxT(""),SetwxPoint(15,235),SetwxSize(164,13),wxST_NO_AUTORESIZE);
	lblDWACIdleFor->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblDWACIdleFor->SetLabel(wxT("Do work after computer is idle for:"));
	wxString itmsDWACIdleFor[]={wxT("1"),wxT("5"),wxT("10"),wxT("15"),wxT("30"),wxT("60")};
	cmbDWACIdleFor=new wxComboBox(this,-1,wxT(""),SetwxPoint(180,230),SetwxSize(85,21),5,itmsDWACIdleFor,wxNO_BORDER | wxCB_READONLY);
	cmbDWACIdleFor->SetValue(wxT("5"));
	lblMinutes=new wxStaticText(this,-1,wxT(""),SetwxPoint(270,235),SetwxSize(39,13),wxST_NO_AUTORESIZE);
	lblMinutes->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblMinutes->SetLabel(wxT("minutes"));
	////Skin XML file
	lblSkinXML=new wxStaticText(this,-1,wxT(""),SetwxPoint(115,270),SetwxSize(64,13),wxST_NO_AUTORESIZE);
	lblSkinXML->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblSkinXML->SetLabel(wxT("Skin XML file:"));
	//skin picker control
    wxString itmsSkinPicker[]={wxT("Default"),wxT("WorldCommunityGrid")};
	cmbSkinPicker=new wxComboBox(this,ID_SKINPICKERCMBBOX,wxT(""),SetwxPoint(180,265),SetwxSize(140,21),2,itmsSkinPicker,wxNO_BORDER | wxCB_READONLY);
	cmbSkinPicker->SetValue(wxT("Default"));
	//tx30c=new wxTextCtrl(this,-1,wxT(""),SetwxPoint(180,265),SetwxSize(148,21),wxSIMPLE_BORDER);
	//btnOpen=new wxBitmapButton(this,ID_OPENBUTTON,*btmpBtnAttProjL,SetwxPoint(331,265),SetwxSize(59,20));
	// Btn Save and Cancel
	btnSave=new wxBitmapButton(this,ID_SAVEBUTTON,*bti26cImg1,SetwxPoint(115,325),SetwxSize(59,20));
	btnCancel=new wxBitmapButton(this,ID_CANCELBUTTON,*bti27cImg1,SetwxPoint(190,325),SetwxSize(59,20));
	 
	Refresh();
}
void CDlgPreferences::LoadSkinImages(){
	//get skin class
	appSkin = SkinClass::Instance();
    wxString str1 = m_SkinDirPrefix + appSkin->GetDlgPrefBg();
	fileImgBuf[0].LoadFile(m_SkinDirPrefix + appSkin->GetDlgPrefBg(),wxBITMAP_TYPE_BMP);
	fileImgBuf[1].LoadFile(m_SkinDirPrefix + appSkin->GetBtnSave(),wxBITMAP_TYPE_BMP);
	fileImgBuf[2].LoadFile(m_SkinDirPrefix + appSkin->GetBtnCancel(),wxBITMAP_TYPE_BMP);
	fileImgBuf[3].LoadFile(m_SkinDirPrefix + appSkin->GetBtnOpen(),wxBITMAP_TYPE_BMP);
	dlg10484fImg0=&fileImgBuf[0];
	bti26cImg1=&fileImgBuf[1];
	bti27cImg1=&fileImgBuf[2];
	btmpBtnAttProjL=&fileImgBuf[3];
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
 if(m_wxWin==this){event.Skip(true);VwXDrawBackImg(event,this,*dlg10484fImg0,0);VwXEvOnEraseBackground(event) ;return;}
 event.Skip(true);
}

void CDlgPreferences::OnBtnClick(wxCommandEvent& event){ //init function
    wxObject *m_wxDlgBtnObj = event.GetEventObject();
	int btnID =  event.GetId();
	if(btnID==ID_SAVEBUTTON){
		//wxMessageBox("OnBtnClick - btnSave");
		EndModal(wxID_CANCEL);
	}
	else if(btnID==ID_OPENBUTTON){
		wxString fileName = wxFileSelector(_("Choose a file to open"), _(""), _(""), _("*.xml*"), _("*.xml*"), wxOPEN); 
		if(!fileName.IsEmpty()){
			//tx30c->SetLabel(fileName);
			//tx30c->Disable();
			//tx30c->SetEditable(false);
			//set the path value
			//this->SetSkinPath(fileName);  REPLACED WITH DROPDOWN
		}
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

void CDlgPreferences::VwXEvOnEraseBackground(wxEraseEvent& event){ //init function
 //[29b]Code event VwX...Don't modify[29b]//
 //add your code here

} //end function

void CDlgPreferences::OnPreCreate(){
 //add your code here

}

void CDlgPreferences::initBefore(){
 //add your code here

}

void CDlgPreferences::initAfter(){
 //add your code here
    Centre();
}

//[evtFunc]end your code

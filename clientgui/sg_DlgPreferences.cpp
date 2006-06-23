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
}; 

BEGIN_EVENT_TABLE( CDlgPreferences,wxDialog)
  EVT_BUTTON(-1,CDlgPreferences::OnBtnClick)
  EVT_ERASE_BACKGROUND(CDlgPreferences::OnEraseBackground)
END_EVENT_TABLE()
// end events

CDlgPreferences::CDlgPreferences(wxWindow* parent,wxWindowID id,const wxString& title,const wxPoint& pos,const wxSize& size,long style,const wxString& name)
{
 m_SkinPath = "";

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

	st3c=new wxStaticText(this,-1,wxT(""),SetwxPoint(10,10),SetwxSize(84,18),wxST_NO_AUTORESIZE);
	st3c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st3c->SetLabel(wxT("Preferences"));
	st3c->SetFont(wxFont(12,74,90,90,0,wxT("Tahoma")));
	st4c=new wxStaticText(this,-1,wxT(""),SetwxPoint(15,60),SetwxSize(164,13),wxST_NO_AUTORESIZE);
	st4c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st4c->SetLabel(wxT("Modify settings for this computer:"));
	st5c=new wxStaticText(this,-1,wxT(""),SetwxPoint(65,95),SetwxSize(114,13),wxST_NO_AUTORESIZE);
	st5c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st5c->SetLabel(wxT("Do work only between:"));
	st6c=new wxStaticText(this,-1,wxT(""),SetwxPoint(270,95),SetwxSize(19,13),wxST_NO_AUTORESIZE);
	st6c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st6c->SetLabel(wxT("and"));
	st7c=new wxStaticText(this,-1,wxT(""),SetwxPoint(10,130),SetwxSize(169,13),wxST_NO_AUTORESIZE);
	st7c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st7c->SetLabel(wxT("Connect to internet only between:"));
	cmb8c=new wxComboBox(this,-1,wxT(""),SetwxPoint(180,90),SetwxSize(85,21),0,NULL);
	cmb8c->SetLabel(wxT("control"));
	cmb11c=new wxComboBox(this,-1,wxT(""),SetwxPoint(295,90),SetwxSize(85,21),0,NULL);
	cmb11c->SetLabel(wxT("control"));
	cmb12c=new wxComboBox(this,-1,wxT(""),SetwxPoint(180,125),SetwxSize(85,21),0,NULL);
	cmb12c->SetLabel(wxT("control"));
	cmb13c=new wxComboBox(this,-1,wxT(""),SetwxPoint(295,125),SetwxSize(85,21),0,NULL);
	cmb13c->SetLabel(wxT("control"));
	st14c=new wxStaticText(this,-1,wxT(""),SetwxPoint(270,130),SetwxSize(19,13),wxST_NO_AUTORESIZE);
	st14c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st14c->SetLabel(wxT("and"));
	st15c=new wxStaticText(this,-1,wxT(""),SetwxPoint(85,165),SetwxSize(94,13),wxST_NO_AUTORESIZE);
	st15c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st15c->SetLabel(wxT("Use no more than:"));
	tx17c=new wxTextCtrl(this,-1,wxT(""),SetwxPoint(180,160),SetwxSize(85,21),wxSIMPLE_BORDER);
	st18c=new wxStaticText(this,-1,wxT(""),SetwxPoint(270,165),SetwxSize(14,13),wxST_NO_AUTORESIZE);
	st18c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st18c->SetLabel(wxT("GB"));
	st19c=new wxStaticText(this,-1,wxT(""),SetwxPoint(15,200),SetwxSize(164,13),wxST_NO_AUTORESIZE);
	st19c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st19c->SetLabel(wxT("Do work while computer is in use?"));
	st22c=new wxStaticText(this,-1,wxT(""),SetwxPoint(90,235),SetwxSize(89,13),wxST_NO_AUTORESIZE);
	st22c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st22c->SetLabel(wxT("Use no more than:"));
	tx23c=new wxTextCtrl(this,-1,wxT(""),SetwxPoint(180,230),SetwxSize(85,21),wxSIMPLE_BORDER);
	st24c=new wxStaticText(this,-1,wxT(""),SetwxPoint(270,235),SetwxSize(39,13),wxST_NO_AUTORESIZE);
	st24c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st24c->SetLabel(wxT("minutes"));
	// yes or no combo box
	wxString cmb25cItem[]={wxT("Yes"),wxT("No")};
	cmb25c=new wxComboBox(this,-1,wxT(""),SetwxPoint(180,195),SetwxSize(45,21),2,cmb25cItem,wxNO_BORDER | wxCB_READONLY);
	st28c=new wxStaticText(this,-1,wxT(""),SetwxPoint(115,270),SetwxSize(64,13),wxST_NO_AUTORESIZE);
	st28c->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	st28c->SetLabel(wxT("Skin XML file:"));
	//file picker control
	tx30c=new wxTextCtrl(this,-1,wxT(""),SetwxPoint(180,265),SetwxSize(148,21),wxSIMPLE_BORDER);
	btnOpen=new wxBitmapButton(this,ID_OPENBUTTON,*btmpBtnAttProjL,SetwxPoint(331,265),SetwxSize(59,20));
	// Btn Save and Cancel
	btnSave=new wxBitmapButton(this,ID_SAVEBUTTON,*bti26cImg1,SetwxPoint(115,325),SetwxSize(59,20));
	btnCancel=new wxBitmapButton(this,ID_CANCELBUTTON,*bti27cImg1,SetwxPoint(190,325),SetwxSize(59,20));
	 
	Refresh();
}
void CDlgPreferences::LoadSkinImages(){
	//get skin class
	appSkin = SkinClass::Instance();

	fileImgBuf[0].LoadFile(appSkin->GetDlgPrefBg(),wxBITMAP_TYPE_BMP);
	fileImgBuf[1].LoadFile(appSkin->GetBtnSave(),wxBITMAP_TYPE_BMP);
	fileImgBuf[2].LoadFile(appSkin->GetBtnCancel(),wxBITMAP_TYPE_BMP);
	fileImgBuf[3].LoadFile(appSkin->GetBtnOpen(),wxBITMAP_TYPE_BMP);
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
			tx30c->SetLabel(fileName);
			//tx30c->Disable();
			tx30c->SetEditable(false);
			//set the path value
			this->SetSkinPath(fileName);
		}
	}else{
		//wxMessageBox("OnBtnClick - btnCancel");
		EndModal(wxID_CANCEL);
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

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


#ifndef _DLG_PREFERENCES_H_ 
#define _DLG_PREFERENCES_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_DlgPreferences.cpp"
#endif

class SkinClass;

class CDlgPreferences:public wxDialog
{
public:
 CDlgPreferences(wxWindow* parent, wxString dirPref,wxWindowID id = -1, const wxString& title = wxT(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("dialogBox"));
 //Skin Class
 SkinClass *appSkin;
 // Pointer control
 wxStaticText *lblPref;
 wxStaticText *lblModifySett;
 wxStaticText *lblDoWorkBtwn;
 wxStaticText *lblAnd1;
 wxStaticText *lblConnToIntBtwn;
 wxComboBox *cmbDWBtwnBgn;
 wxComboBox *cmbDWBtwnEnd;
 wxComboBox *cmbCTIBtwnBgn;
 wxComboBox *cmbCTIBtwnEnd;
 wxStaticText *lblAnd2;
 wxStaticText *lblUseNoMoreGB;
 wxComboBox *cmbUseNoMoreGB;
 wxStaticText *lblGB;
 wxStaticText *lblDWWCInUse;
 wxStaticText *lblDWACIdleFor;
 wxComboBox *cmbDWACIdleFor;
 wxStaticText *lblMinutes;
 wxComboBox *cmbDWWCInUse;
 wxBitmapButton *btnSave;
 wxBitmapButton *btnCancel;
 wxStaticText *lblSkinXML;
 wxComboBox *cmbSkinPicker;
 //wxTextCtrl *tx30c;
 wxBitmapButton *btnOpen;
 wxBitmap *dlg10484fImg0;
 wxBitmap *bti26cImg1;
 wxBitmap *bti27cImg1;
 wxBitmap *btmpBtnAttProjL;
 wxBitmap fileImgBuf[4];
 virtual ~CDlgPreferences();
 void initBefore();
 void OnPreCreate();
 void InitDialog();
 void LoadSkinImages();
 wxString GetSkinDirPrefix() const { return m_SkinDirPrefix; }
 wxString GetSkinName() const { return m_SkinName; }
 void SetSkinDirPrefix(const wxString& pref) { m_SkinDirPrefix = pref; }
 void SetSkinName(const wxString& skinName) { m_SkinName = skinName; }
 
 void initAfter();

 DECLARE_EVENT_TABLE()

protected:
  wxString m_SkinName;
  wxString m_SkinDirPrefix;
 void OnEraseBackground(wxEraseEvent& event);
 void OnBtnClick(wxCommandEvent& event);
 void OnCmbSelected(wxCommandEvent& event);
 void VwXEvOnEraseBackground(wxEraseEvent& event);
 void VwXDrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz);

//[win]end your code 
};

#endif  // end CDlgPreferences

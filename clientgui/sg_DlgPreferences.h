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

#include <wx/dir.h> 

class SkinClass;

class DirTraverserSkins : public wxDirTraverser
{
public:
    DirTraverserSkins(wxArrayString& skins) : m_skins(skins) { }

    virtual wxDirTraverseResult OnFile(const wxString& filename);
    virtual wxDirTraverseResult OnDir(const wxString& WXUNUSED(dirname));

private:
    wxArrayString& m_skins;
};

class CDlgPreferences:public wxDialog
{
public:
 CDlgPreferences(wxWindow* parent, wxString dirPref,wxWindowID id = -1, const wxString& title = wxT(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("dialogBox"));
	//Skin Class
	SkinClass *appSkin;
	//btns
	wxImage *g_save;
	wxImage *g_saveClick;
	wxImage *g_cancel;
	wxImage *g_cancelClick;
	wxImage *g_clear;
	wxImage *g_clearClick;
	wxBitmap btmpSave; 
	wxBitmap btmpSaveClick; 
	wxBitmap btmpCancel; 
	wxBitmap btmpCancelClick; 
	wxBitmap btmpClear; 
	wxBitmap btmpClearClick; 
	wxBitmapButton *btnSave;
	wxBitmapButton *btnCancel;
	wxBitmapButton *btnClear;
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
	
	wxStaticText *lblSkinXML;
	wxComboBox *cmbSkinPicker;
	//wxTextCtrl *tx30c;

	wxBitmap *dlgBack;
	wxBitmap fileImgBuf[1];
	virtual ~CDlgPreferences();
	void initBefore();
	void CheckSettings();
	bool CheckIfInArray(wxString valArray[],wxString value,int size);
	void ReadSettings(GLOBAL_PREFS prefs);
	void CreateDialog();
	void LoadSkinImages();
	int ConvertToNumber(wxString num);
	wxString GetSkinName() const { return m_SkinName; }
	void SetSkinName(const wxString& skinName) { m_SkinName = skinName; }
	void OnPaint(wxPaintEvent& event); 
	void initAfter();

	DECLARE_EVENT_TABLE()

protected:
	wxString m_SkinName;
	wxString m_PrefIndicator;
	bool m_globalPrefUsed;
	wxArrayString m_skinNames;
	wxString m_SkinDirPrefix;
	GLOBAL_PREFS m_prefs;
	void OnEraseBackground(wxEraseEvent& event);
	void OnBtnClick(wxCommandEvent& event);
	void OnCmbSelected(wxCommandEvent& event);
	void VwXEvOnEraseBackground(wxEraseEvent& event);
	void VwXDrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz);

//[win]end your code 
};

#endif  // end CDlgPreferences

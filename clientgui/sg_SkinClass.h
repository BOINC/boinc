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

#ifndef _SKIN_CLASS
#define _SKIN_CLASS

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_SkinClass.cpp"
#endif

#include "common/wxFlatNotebook.h"

class SkinClass
{ 
public: 
	    static SkinClass* Instance();
		bool change_skin(const wxString& new_skin_name);
		bool init_skin(const wxString& skin_name);

        // Getters
		wxBitmap* GetAppBg() { return skinImages[wxString(_T("background"))]; }
		wxColour GetAppBgCol() { return GetColorFromStr(m_appBgCol); }
		wxBitmap* GetProjCompBg() { return skinImages[wxString(_T("project_component_background"))]; }
        wxBitmap* GetSpacerImage() { return skinImages[wxString(_T("spacer_image"))]; }
		wxBitmap* GetWorkunitBg() { return skinImages[wxString(_T("workunit_background"))]; }
		wxBitmap* GetBtnPrefer() { return skinImages[wxString(_T("preferences_button"))]; }
		wxBitmap* GetBtnAddProj() { return skinImages[wxString(_T("add_project_button"))]; }
		wxBitmap* GetBtnAddProjClick() { return skinImages[wxString(_T("add_project_clicked_button"))]; }
		wxBitmap* GetBtnAdvView() { return skinImages[wxString(_T("advanced_view_button"))]; }
        wxBitmap* GetBtnResume() { return skinImages[wxString(_T("resume_button"))]; }
        wxBitmap* GetBtnPause() { return skinImages[wxString(_T("pause_button"))]; }
		wxBitmap* GetBtnMessages() { return skinImages[wxString(_T("messages_button"))]; }
		wxBitmap* GetBtnAlertMessages() { return skinImages[wxString(_T("alert_messages_button"))]; }
		wxBitmap* GetBtnSave() { return skinImages[wxString(_T("save_button"))]; }
		wxBitmap* GetBtnSaveClick() { return skinImages[wxString(_T("save_clicked_button"))]; }
		wxBitmap* GetBtnCancel() { return skinImages[wxString(_T("cancel_button"))]; }
		wxBitmap* GetBtnCancelClick() { return skinImages[wxString(_T("cancel_clicked_button"))]; }
		wxBitmap* GetBtnClear() { return skinImages[wxString(_T("clear_button"))]; }
		wxBitmap* GetBtnClearClick() { return skinImages[wxString(_T("clear_clicked_button"))]; }
		wxBitmap* GetBtnClose() { return skinImages[wxString(_T("close_button"))]; }
		wxBitmap* GetBtnCloseClick() { return skinImages[wxString(_T("close_clicked_button"))]; }
		wxBitmap* GetBtnLeftArr() { return skinImages[wxString(_T("left_arrow_button"))]; }
		wxBitmap* GetBtnRightArr() { return skinImages[wxString(_T("right_arrow_button"))]; }
		wxBitmap* GetBtnLeftArrClick() { return skinImages[wxString(_T("left_arrow_clicked_button"))]; }
		wxBitmap* GetBtnRightArrClick() { return skinImages[wxString(_T("right_arrow_clicked_button"))]; }
        
		wxBitmap* GetDlgPrefBg() { return skinImages[wxString(_T("preferences_dialogue"))]; }
		wxBitmap* GetDlgMessBg() { return skinImages[wxString(_T("messages_dialogue"))]; }

		wxColour GetStaticLineCol() { return GetColorFromStr(m_staticLineCol); }

		wxBitmap* GetGaugeBg() { return skinImages[wxString(_T("gauge_background"))]; }
		wxBitmap* GetGaugeProgressInd() { return skinImages[wxString(_T("gauge_progress"))]; }

		wxBitmap* GetStateIndBg() { return skinImages[wxString(_T("state_indicator_background"))]; }
	    wxBitmap* GetConnInd() { return skinImages[wxString(_T("connecting_indicator"))]; }
	    wxBitmap* GetErrorInd() { return skinImages[wxString(_T("error_indicator"))]; }
	
        wxBitmap* GetTabAreaBg() { return skinImages[wxString(_T("tab_area_background"))]; }
        wxColour GetTabFromColAc() { return GetColorFromStr(m_tabFromColAc); }
		wxColour GetTabToColAc() { return GetColorFromStr(m_tabToColAc); }
		wxColour GetTabBrdColAc() { return GetColorFromStr(m_tabBrdColAc); }
		wxColour GetTabFromColIn() { return GetColorFromStr(m_tabFromColIn); }
		wxColour GetTabToColIn() { return GetColorFromStr(m_tabToColIn); }
		wxColour GetTabBrdColIn() { return GetColorFromStr(m_tabBrdColIn); }
        
		wxBitmap* GetIcnWorkingWkUnit() { return skinImages[wxString(_T("workingWkUnit"))]; }
        wxBitmap* GetDefaultStatIcn() { return skinImages[wxString(_T("defaultStatIcon"))]; }
        wxBitmap* GetAnimationBg() { return skinImages[wxString(_T("animation_background"))]; }
        wxString GetAnimationFile() const { return m_animFile; }
        
		wxString GetSkinName() const { return m_skinName; }
		wxString GetSkinsFolder() const { return m_skinsFolder; }

		wxBitmap &getFrameBG();

        // Setters
		void SetAppBgCol(const wxString& clr) { m_appBgCol = clr; }
    
		void SetStaticLineCol(const wxString& clr) { m_staticLineCol = clr; }

		void SetTabFromColAc(const wxString& clr) { m_tabFromColAc = clr; }
		void SetTabToColAc(const wxString& clr) { m_tabToColAc = clr; }
		void SetTabBrdColAc(const wxString& clr) { m_tabBrdColAc = clr; }
		void SetTabFromColIn(const wxString& clr) { m_tabFromColIn = clr; }
		void SetTabToColIn(const wxString& clr) { m_tabToColIn = clr; }
		void SetTabBrdColIn(const wxString& clr) { m_tabBrdColIn = clr; }

		void SetAnimationFile(const wxString& imgsrc) { m_animFile = imgsrc; }
		
		void SetSkinName(const wxString& name) { m_skinName = name; }
		void SetSkinsFolder(const wxString& fldr) { m_skinsFolder = fldr; }
		wxString ComputeSkinDir();

private: 
	    /// Constructors
	    SkinClass();
		wxColour GetColorFromStr(wxString col);
		wxString compute_skin_path();

		bool GetImageName(char* buf, const char* field);

		bool CheckSkin();
		int LoadSkinXML();
		bool LoadImages();

		// Bg
		wxString m_appBgCol;
		// Bg
		wxString m_staticLineCol;
		//gauge 
        wxString m_tabFromColAc; 
		wxString m_tabToColAc; 
		wxString m_tabBrdColAc; 
		//inactive tab
		wxString m_tabFromColIn; 
		wxString m_tabToColIn; 
		wxString m_tabBrdColIn; 
		// annimiation
		wxString m_animFile;
		//skin info
		wxString m_skinName;
		wxString m_skinsFolder;

		WX_DECLARE_STRING_HASH_MAP( wxString, wxStringHashMap );
		wxStringHashMap skinImageNames;
		WX_DECLARE_STRING_HASH_MAP( wxBitmap*, wxBitmapHashMap );
		wxBitmapHashMap skinImages;

		wxFlatNotebookImageList m_ImageList;

};

#endif  /* _SKIN_CLASS */

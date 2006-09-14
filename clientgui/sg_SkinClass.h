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
		wxString GetAppBg() { return skinImageNames[wxString(_T("background"))]; }
		wxColour GetAppBgCol() { return GetColorFromStr(m_appBgCol); }
		wxString GetProjCompBg() { return skinImageNames[wxString(_T("project_component_background"))]; }
        wxString GetSpacerImage() { return skinImageNames[wxString(_T("spacer_image"))]; }
		wxString GetWorkunitBg() { return skinImageNames[wxString(_T("workunit_background"))]; }
		wxString GetBtnPrefer() { return skinImageNames[wxString(_T("preferences_button"))]; }
		wxString GetBtnAddProj() { return skinImageNames[wxString(_T("add_project_button"))]; }
		wxString GetBtnAddProjClick() { return skinImageNames[wxString(_T("add_project_clicked_button"))]; }
		wxString GetBtnAdvView() { return skinImageNames[wxString(_T("advanced_view_button"))]; }
        wxString GetBtnResume() { return skinImageNames[wxString(_T("resume_button"))]; }
        wxString GetBtnPause() { return skinImageNames[wxString(_T("pause_button"))]; }
		wxString GetBtnMessages() { return skinImageNames[wxString(_T("messages_button"))]; }
		wxString GetBtnAlertMessages() { return skinImageNames[wxString(_T("alert_messages_button"))]; }
		wxString GetBtnSave() { return skinImageNames[wxString(_T("save_button"))]; }
		wxString GetBtnSaveClick() { return skinImageNames[wxString(_T("save_clicked_button"))]; }
		wxString GetBtnCancel() { return skinImageNames[wxString(_T("cancel_button"))]; }
		wxString GetBtnCancelClick() { return skinImageNames[wxString(_T("cancel_clicked_button"))]; }
		wxString GetBtnClear() { return skinImageNames[wxString(_T("clear_button"))]; }
		wxString GetBtnClearClick() { return skinImageNames[wxString(_T("clear_clicked_button"))]; }
		wxString GetBtnClose() { return skinImageNames[wxString(_T("close_button"))]; }
		wxString GetBtnCloseClick() { return skinImageNames[wxString(_T("close_clicked_button"))]; }
		wxString GetBtnLeftArr() { return skinImageNames[wxString(_T("left_arrow_button"))]; }
		wxString GetBtnRightArr() { return skinImageNames[wxString(_T("right_arrow_button"))]; }
		wxString GetBtnLeftArrClick() { return skinImageNames[wxString(_T("left_arrow_clicked_button"))]; }
		wxString GetBtnRightArrClick() { return skinImageNames[wxString(_T("right_arrow_clicked_button"))]; }
        
		wxString GetDlgPrefBg() { return skinImageNames[wxString(_T("preferences_dialogue"))]; }
		wxString GetDlgMessBg() { return skinImageNames[wxString(_T("messages_dialogue"))]; }

		wxColour GetStaticLineCol() { return GetColorFromStr(m_staticLineCol); }

		wxString GetGaugeBg() { return skinImageNames[wxString(_T("gauge_background"))]; }
		wxString GetGaugeProgressInd() { return skinImageNames[wxString(_T("gauge_progress"))]; }

		wxString GetStateIndBg() { return skinImageNames[wxString(_T("state_indicator_background"))]; }
	    wxString GetConnInd() { return skinImageNames[wxString(_T("connecting_indicator"))]; }
	    wxString GetErrorInd() { return skinImageNames[wxString(_T("error_indicator"))]; }
	
        wxString GetTabAreaBg() { return skinImageNames[wxString(_T("tab_area_background"))]; }
        wxColour GetTabFromColAc() { return GetColorFromStr(m_tabFromColAc); }
		wxColour GetTabToColAc() { return GetColorFromStr(m_tabToColAc); }
		wxColour GetTabBrdColAc() { return GetColorFromStr(m_tabBrdColAc); }
		wxColour GetTabFromColIn() { return GetColorFromStr(m_tabFromColIn); }
		wxColour GetTabToColIn() { return GetColorFromStr(m_tabToColIn); }
		wxColour GetTabBrdColIn() { return GetColorFromStr(m_tabBrdColIn); }
        
		wxString GetIcnWorkingWkUnit() { return skinImageNames[wxString(_T("workingWkUnit"))]; }
        wxString GetDefaultStatIcn() { return skinImageNames[wxString(_T("defaultStatIcon"))]; }
        wxString GetAnimationBg() { return skinImageNames[wxString(_T("animation_background"))]; }
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

private: 
	    /// Constructors
	    SkinClass();
		wxColour GetColorFromStr(wxString col);
		wxString compute_skin_path();
		wxString compute_skin_dir();

		bool GetImageName(char* buf, const char* field);

		bool CheckSkin();
		int LoadSkinXML();

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
		WX_DECLARE_STRING_HASH_MAP( wxBitmap, wxBitmapHashMap );
		wxBitmapHashMap skinImages;

		wxFlatNotebookImageList m_ImageList;

};

#endif  /* _SKIN_CLASS */

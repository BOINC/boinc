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

class SkinClass
{ 
public: 
	    static SkinClass* Instance();

        // Getters
		wxString GetAppBg() { return m_appBg; }
		wxColour GetAppBgCol() { return GetColorFromStr(m_appBgCol); }
		wxString GetProjCompBg() { return m_projCompBg; }
        wxString GetSpacerImage() { return m_spacerImage; }
		wxString GetWorkunitBg() { return m_workunitBg; }
		wxString GetBtnPrefer() const { return m_btnPrefer; }
		wxString GetBtnAddProj() const { return m_btnAddProj; }
		wxString GetBtnAddProjClick() const { return m_btnAddProjClick; }
		wxString GetBtnAdvView() const { return m_btnAdvView; }
        wxString GetBtnResume() const { return m_btnResume; }
        wxString GetBtnPause() const { return m_btnPause; }
		wxString GetBtnMessages() const { return m_btnMessages; }
		wxString GetBtnSave() const { return m_btnSave; }
		wxString GetBtnSaveClick() const { return m_btnSaveClick; }
		wxString GetBtnCancel() const { return m_btnCancel; }
		wxString GetBtnCancelClick() const { return m_btnCancelClick; }
		wxString GetBtnClear() const { return m_btnClear; }
		wxString GetBtnClearClick() const { return m_btnClearClick; }
		wxString GetBtnClose() const { return m_btnClose; }
		wxString GetBtnCloseClick() const { return m_btnCloseClick; }
		wxString GetBtnLeftArr() const { return m_btnLeftArr; }
		wxString GetBtnRightArr() const { return m_btnRightArr; }
		wxString GetBtnLeftArrClick() const { return m_btnLeftArrClick; }
		wxString GetBtnRightArrClick() const { return m_btnRightArrClick; }
        
		wxString GetDlgPrefBg() { return m_dlgPrefBg; }
		wxString GetDlgMessBg() { return m_dlgMessBg; }

		wxColour GetStaticLineCol() { return GetColorFromStr(m_staticLineCol); }

		wxString GetGaugeBg() { return m_gaugeBg; }
		wxString GetGaugeProgressInd() { return m_gaugeProgressInd; }
	
        wxString GetTabAreaBg() const { return m_tabAreaBg; }
        wxColour GetTabFromColAc() { return GetColorFromStr(m_tabFromColAc); }
		wxColour GetTabToColAc() { return GetColorFromStr(m_tabToColAc); }
		wxColour GetTabBrdColAc() { return GetColorFromStr(m_tabBrdColAc); }
		wxColour GetTabFromColIn() { return GetColorFromStr(m_tabFromColIn); }
		wxColour GetTabToColIn() { return GetColorFromStr(m_tabToColIn); }
		wxColour GetTabBrdColIn() { return GetColorFromStr(m_tabBrdColIn); }
        
		wxString GetIcnWorkingWkUnit() const { return m_icnWorkingWkUnit; }
        wxString GetDefaultStatIcn() const { return m_defaultStatIcn; }
        wxString GetAnimationBg() const { return m_animBg; }
        wxString GetAnimationFile() const { return m_animFile; }
        
		wxString GetSkinName() const { return m_skinName; }
		wxString GetSkinsFolder() const { return m_skinsFolder; }

        // Setters
		void SetAppBg(const wxString imgsrc) { m_appBg = imgsrc; }
		void SetAppBgCol(const wxString& clr) { m_appBgCol = clr; }
		void SetProjCompBg(const wxString& imgsrc) { m_projCompBg = imgsrc; }
		void SetSpacerImage(const wxString& imgsrc) { m_spacerImage = imgsrc; }
		void SetWorkunitBg(const wxString& imgsrc) { m_workunitBg = imgsrc; }
		void SetBtnPrefer(const wxString& imgsrc) { m_btnPrefer = imgsrc; }
		void SetBtnAddProj(const wxString& imgsrc) { m_btnAddProj = imgsrc; }
		void SetBtnAddProjClick(const wxString& imgsrc) { m_btnAddProjClick = imgsrc; }
		void SetBtnAdvView(const wxString& imgsrc) { m_btnAdvView = imgsrc; }
		void SetBtnResume(const wxString& imgsrc) { m_btnResume = imgsrc; }
		void SetBtnPause(const wxString& imgsrc) { m_btnPause = imgsrc; }
		void SetBtnMessages(const wxString& imgsrc) { m_btnMessages = imgsrc; }
		void SetBtnOpen(const wxString& imgsrc) { m_btnOpen = imgsrc; }
		void SetBtnSave(const wxString& imgsrc) { m_btnSave = imgsrc; }
		void SetBtnSaveClick(const wxString& imgsrc) { m_btnSaveClick = imgsrc; }
		void SetBtnCancel(const wxString& imgsrc) { m_btnCancel = imgsrc; }
		void SetBtnCancelClick(const wxString& imgsrc) { m_btnCancelClick = imgsrc; }
		void SetBtnClear(const wxString& imgsrc) { m_btnClear = imgsrc; }
		void SetBtnClearClick(const wxString& imgsrc) { m_btnClearClick = imgsrc; }
		void SetBtnClose(const wxString& imgsrc) { m_btnClose = imgsrc; }
		void SetBtnCloseClick(const wxString& imgsrc) { m_btnCloseClick = imgsrc; }
		void SetBtnLeftArr(const wxString& imgsrc) { m_btnLeftArr = imgsrc; }
		void SetBtnRightArr(const wxString& imgsrc) { m_btnRightArr = imgsrc; }
		void SetBtnLeftArrClick(const wxString& imgsrc) { m_btnLeftArrClick = imgsrc; }
		void SetBtnRightArrClick(const wxString& imgsrc) { m_btnRightArrClick = imgsrc; }
		
		void SetDlgPrefBg(const wxString& imgsrc) { m_dlgPrefBg = imgsrc; }
		void SetDlgMessBg(const wxString& imgsrc) { m_dlgMessBg = imgsrc; }
    
		void SetStaticLineCol(const wxString& clr) { m_staticLineCol = clr; }

		void SetGaugeBg(const wxString& clr) { m_gaugeBg = clr; }
		void SetGaugeProgressInd(const wxString& clr) { m_gaugeProgressInd = clr; }

		void SetTabAreaBg(const wxString& imgsrc) { m_tabAreaBg = imgsrc; }
		void SetTabFromColAc(const wxString& clr) { m_tabFromColAc = clr; }
		void SetTabToColAc(const wxString& clr) { m_tabToColAc = clr; }
		void SetTabBrdColAc(const wxString& clr) { m_tabBrdColAc = clr; }
		void SetTabFromColIn(const wxString& clr) { m_tabFromColIn = clr; }
		void SetTabToColIn(const wxString& clr) { m_tabToColIn = clr; }
		void SetTabBrdColIn(const wxString& clr) { m_tabBrdColIn = clr; }

		void SetIcnWorkingWkUnit(const wxString& imgsrc) { m_icnWorkingWkUnit = imgsrc; }
		void SetDefaultStatIcn(const wxString& imgsrc) { m_defaultStatIcn = imgsrc; }
		void SetAnimationBg(const wxString& imgsrc) { m_animBg = imgsrc; }
		void SetAnimationFile(const wxString& imgsrc) { m_animFile = imgsrc; }
		
		void SetSkinName(const wxString& name) { m_skinName = name; }
		void SetSkinsFolder(const wxString& fldr) { m_skinsFolder = fldr; }

private: 
	    /// Constructors
	    SkinClass();
		wxColour GetColorFromStr(wxString col);
	    // Bg
	    wxString m_appBg; 
		wxString m_appBgCol;
		wxString m_projCompBg;
		wxString m_spacerImage;
		wxString m_workunitBg;
        //Dialogs
		wxString m_dlgPrefBg;
		wxString m_dlgMessBg;
		// Bg
		wxString m_staticLineCol;
		//gauge 
        wxString m_gaugeBg; 
		wxString m_gaugeProgressInd;
		//notebook colors
		//active tab
		wxString m_tabAreaBg;
        wxString m_tabFromColAc; 
		wxString m_tabToColAc; 
		wxString m_tabBrdColAc; 
		//inactive tab
		wxString m_tabFromColIn; 
		wxString m_tabToColIn; 
		wxString m_tabBrdColIn; 
		// Btns
		wxString m_btnPrefer;
		wxString m_btnAddProj;
		wxString m_btnAddProjClick;
		wxString m_btnAdvView;
		wxString m_btnResume;
        wxString m_btnPause;
        wxString m_btnMessages;
		wxString m_btnOpen;
        wxString m_btnSave;
		wxString m_btnSaveClick;
		wxString m_btnCancel;
		wxString m_btnCancelClick;
		wxString m_btnClear;
		wxString m_btnClearClick;
		wxString m_btnClose;
		wxString m_btnCloseClick;
		wxString m_btnLeftArr;
		wxString m_btnRightArr;
		wxString m_btnLeftArrClick;
		wxString m_btnRightArrClick;
		wxString m_btnExpand;
		wxString m_btnExpandClick;
		// Icons
		wxString m_icnWorkingWkUnit;
		wxString m_icnSleepingWkUnit;
		wxString m_defaultStatIcn;// default stat icon
		// animation
		wxString m_animBg;
		wxString m_animFile;
		//skin info
		wxString m_skinName;
		wxString m_skinsFolder;
};
#endif  /* _SKIN_CLASS */

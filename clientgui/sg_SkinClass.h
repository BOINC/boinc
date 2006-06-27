#ifndef _SKIN_CLASS
#define _SKIN_CLASS

#include <string>

class SkinClass
{ 
public: 
	    static SkinClass* Instance();

        // Getters
		wxString GetAppBg() { return m_appBg; }
		wxColour GetAppBgCol() { return GetColorFromStr(m_appBgCol); }
		wxString GetBtnPrefer() const { return m_btnPrefer; }
		wxString GetBtnAttProj() const { return m_btnAttProj; }
		wxString GetBtnAdvView() const { return m_btnAdvView; }
        wxString GetBtnPlay() const { return m_btnPlay; }
        wxString GetBtnPause() const { return m_btnPause; }
		wxString GetBtnMessages() const { return m_btnMessages; }
		wxString GetBtnOpen() const { return m_btnOpen; }
        wxString GetBtnSave() const { return m_btnSave; }
		wxString GetBtnCancel() const { return m_btnCancel; }
		wxString GetBtnLeftArr() const { return m_btnLeftArr; }
		wxString GetBtnRightArr() const { return m_btnRightArr; }
		wxString GetBtnLeftArrClick() const { return m_btnLeftArrClick; }
		wxString GetBtnRightArrClick() const { return m_btnRightArrClick; }
        wxString GetBtnExpand() const { return m_btnExpand; }
		wxString GetBtnCollapse() const { return m_btnCollapse; }
		wxString GetBtnExpandClick() const { return m_btnExpandClick; }
		wxString GetBtnCollapseClick() const { return m_btnCollapseClick; }
        
		wxString GetDlgPrefBg() { return m_dlgPrefBg; }

		wxColour GetGaugeFgCol() { return GetColorFromStr(m_gaugeFgCol); }
		wxColour GetGaugeBgCol() { return GetColorFromStr(m_gaugeBgCol); }
	

		wxColour GetTabFromColAc() { return GetColorFromStr(m_tabFromColAc); }
		wxColour GetTabToColAc() { return GetColorFromStr(m_tabToColAc); }
		wxColour GetTabBrdColAc() { return GetColorFromStr(m_tabBrdColAc); }
		wxColour GetTabFromColIn() { return GetColorFromStr(m_tabFromColIn); }
		wxColour GetTabToColIn() { return GetColorFromStr(m_tabToColIn); }
		wxColour GetTabBrdColIn() { return GetColorFromStr(m_tabBrdColIn); }
        
		wxString GetIcnWorking() const { return m_icnWorking; }
        wxString GetIcnSleeping() const { return m_icnSleeping; }
        wxString GetIcnWorkingWkUnit() const { return m_icnWorkingWkUnit; }
        wxString GetIcnSleepingWkUnit() const { return m_icnSleepingWkUnit; }
        wxString GetIcnPrjWCG() const { return m_icnPrjWCG; }
        wxString GetIcnPrjPRED() const { return m_icnPrjPRED; }
        wxString GetAnimationBG() const { return m_animBg; }
        wxString GetAnimationFile() const { return m_animFile; }
        
        // Setters
		void SetAppBg(const wxString imgsrc) { m_appBg = imgsrc; }
		void SetAppBgCol(const wxString& clr) { m_appBgCol = clr; }
		void SetBtnPrefer(const wxString& imgsrc) { m_btnPrefer = imgsrc; }
		void SetBtnAttProj(const wxString& imgsrc) { m_btnAttProj = imgsrc; }
		void SetBtnAdvView(const wxString& imgsrc) { m_btnAdvView = imgsrc; }
		void SetBtnPlay(const wxString& imgsrc) { m_btnPlay = imgsrc; }
		void SetBtnPause(const wxString& imgsrc) { m_btnPause = imgsrc; }
		void SetBtnMessages(const wxString& imgsrc) { m_btnMessages = imgsrc; }
		void SetBtnOpen(const wxString& imgsrc) { m_btnOpen = imgsrc; }
		void SetBtnSave(const wxString& imgsrc) { m_btnSave = imgsrc; }
		void SetBtnCancel(const wxString& imgsrc) { m_btnCancel = imgsrc; }
		void SetBtnLeftArr(const wxString& imgsrc) { m_btnLeftArr = imgsrc; }
		void SetBtnRightArr(const wxString& imgsrc) { m_btnRightArr = imgsrc; }
		void SetBtnLeftArrClick(const wxString& imgsrc) { m_btnLeftArrClick = imgsrc; }
		void SetBtnRightArrClick(const wxString& imgsrc) { m_btnRightArrClick = imgsrc; }
		void SetBtnExpand(const wxString& imgsrc) { m_btnExpand = imgsrc; }
		void SetBtnCollapse(const wxString& imgsrc)  { m_btnCollapse = imgsrc; }
		void SetBtnExpandClick(const wxString& imgsrc)  { m_btnExpandClick = imgsrc; }
		void SetBtnCollapseClick(const wxString& imgsrc)  { m_btnCollapseClick = imgsrc; }
        
		void SetDlgPrefBg(const wxString& imgsrc) { m_dlgPrefBg = imgsrc; }

		void SetGaugeFgCol(const wxString& clr) { m_gaugeFgCol = clr; }
		void SetGaugeBgCol(const wxString& clr) { m_gaugeBgCol = clr; }

		void SetTabFromColAc(const wxString& clr) { m_tabFromColAc = clr; }
		void SetTabToColAc(const wxString& clr) { m_tabToColAc = clr; }
		void SetTabBrdColAc(const wxString& clr) { m_tabBrdColAc = clr; }
		void SetTabFromColIn(const wxString& clr) { m_tabFromColIn = clr; }
		void SetTabToColIn(const wxString& clr) { m_tabToColIn = clr; }
		void SetTabBrdColIn(const wxString& clr) { m_tabBrdColIn = clr; }

		void SetIcnWorking(const wxString& imgsrc) { m_icnWorking = imgsrc; }
		void SetIcnSleeping(const wxString& imgsrc) { m_icnSleeping = imgsrc; }
		void SetIcnWorkingWkUnit(const wxString& imgsrc) { m_icnWorkingWkUnit = imgsrc; }
		void SetIcnSleepingWkUnit(const wxString& imgsrc) { m_icnSleepingWkUnit = imgsrc; }
		void SetIcnPrjWCG(const wxString& imgsrc) { m_icnPrjWCG = imgsrc; }
		void SetIcnPrjPRED(const wxString& imgsrc) { m_icnPrjPRED = imgsrc; }
		void SetAnimationBg(const wxString& imgsrc) { m_animBg = imgsrc; }
		void SetAnimationFile(const wxString& imgsrc) { m_animFile = imgsrc; }

private: 
	    /// Constructors
	    SkinClass();
		wxColour GetColorFromStr(wxString col);
	    // Bg
	    wxString m_appBg; 
		wxString m_appBgCol;
        //Dialogs
		wxString m_dlgPrefBg;
		//gauge colors
        wxString m_gaugeFgCol; 
		wxString m_gaugeBgCol;
		//notebook colors
		//active tab
        wxString m_tabFromColAc; 
		wxString m_tabToColAc; 
		wxString m_tabBrdColAc; 
		//inactive tab
		wxString m_tabFromColIn; 
		wxString m_tabToColIn; 
		wxString m_tabBrdColIn; 
		// Btns
		wxString m_btnPrefer;
		wxString m_btnAttProj;
		wxString m_btnAdvView;
		wxString m_btnPlay;
        wxString m_btnPause;
        wxString m_btnMessages;
		wxString m_btnOpen;
        wxString m_btnSave;
		wxString m_btnCancel;
		wxString m_btnLeftArr;
		wxString m_btnRightArr;
		wxString m_btnLeftArrClick;
		wxString m_btnRightArrClick;
		wxString m_btnExpand;
		wxString m_btnCollapse;
		wxString m_btnExpandClick;
		wxString m_btnCollapseClick;
		// Icons
		wxString m_icnWorking;
		wxString m_icnSleeping;	
		wxString m_icnWorkingWkUnit;
		wxString m_icnSleepingWkUnit;
		wxString m_icnPrjWCG;	
		wxString m_icnPrjPRED;
		// animation
		wxString m_animBg;
		wxString m_animFile;
};
#endif  /* _SKIN_CLASS */

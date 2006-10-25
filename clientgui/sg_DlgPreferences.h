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

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_CDLGPREFERENCES_STYLE wxDEFAULT_DIALOG_STYLE
#define SYMBOL_CDLGPREFERENCES_TITLE _("BOINC Manager - Preferences")
#define SYMBOL_CDLGPREFERENCES_IDNAME ID_DIALOG
#define SYMBOL_CDLGPREFERENCES_SIZE wxDefaultSize
#define SYMBOL_CDLGPREFERENCES_POSITION wxDefaultPosition
#define ID_CANCELBUTTON 10001
#define ID_CHANGEBUTTON 10002
#define ID_CLEARBUTTON 10003
#define ID_SAVEBUTTON 10004 
#define ID_SKINPICKERCMBBOX 10005 
#define ID_DOWORKONLYBGNCMBBOX 10006
#define ID_DOCONNECTONLYBGNCMBBOX 10007
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * CDlgPreferences class declaration
 */

class CDlgPreferences : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CDlgPreferences )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgPreferences();
    CDlgPreferences( wxWindow* parent, wxWindowID id = SYMBOL_CDLGPREFERENCES_IDNAME, const wxString& caption = SYMBOL_CDLGPREFERENCES_TITLE, const wxPoint& pos = SYMBOL_CDLGPREFERENCES_POSITION, const wxSize& size = SYMBOL_CDLGPREFERENCES_SIZE, long style = SYMBOL_CDLGPREFERENCES_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGPREFERENCES_IDNAME, const wxString& caption = SYMBOL_CDLGPREFERENCES_TITLE, const wxPoint& pos = SYMBOL_CDLGPREFERENCES_POSITION, const wxSize& size = SYMBOL_CDLGPREFERENCES_SIZE, long style = SYMBOL_CDLGPREFERENCES_STYLE );

    /// Creates the controls and sizers
    void CreateControls();


	wxBitmapButton *btnSave;
	wxBitmapButton *btnSaveSkin;
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

	void WriteSettings();
	bool CheckIfInArray(wxString valArray[],wxString value,int size);
	void ReadSettings(GLOBAL_PREFS prefs);
	int ConvertToNumber(wxString num);
	wxString GetSkinName() const { return m_SkinName; }
	void SetSkinName(const wxString& skinName) { m_SkinName = skinName; }
	void OnPaint(wxPaintEvent& event); 

protected:
	wxString m_SkinName;
	wxString m_PrefIndicator;
	wxArrayString m_skinNames;
	GLOBAL_PREFS m_prefs;
	void OnEraseBackground(wxEraseEvent& event);

    void OnChange(wxCommandEvent& event);

	void OnBtnClick(wxCommandEvent& event);
	void VwXDrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap* bitMap,int opz);

private: 
	CStaticLine *lnMyTop;

};

#endif  // end CDlgPreferences

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

#ifndef _VIEWTABPAGE_H_
#define _VIEWTABPAGE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_ViewTabPage.cpp"
#endif

class MyCanvas : public wxScrolledWindow
{
public:
    MyCanvas(wxWindow *parent, const wxPoint& pos, const wxSize& size);
    void OnPaint(wxPaintEvent& event);

private:

    DECLARE_EVENT_TABLE()
};

class CViewTabPage : public wxPanel {
    DECLARE_DYNAMIC_CLASS( CViewTabPage )

public:
	
	//Skin Class
    SkinClass *appSkin;
	// project icons
	wxImage *g_prjIcn;
	ImageLoader *i_prjIcnPI;
	wxWindow *w_iconPI;
	// btns ////////////
	// Collapse button
	wxImage *g_collapse;
	wxImage *g_collapseClick;
	wxBitmap btmpCol; 
    wxBitmap btmpColClick; 
	wxBitmapButton *btnCollapse;
	//ShowGraphic button
    wxImage *g_showGraphic;
	wxImage *g_showGraphicClick;
	wxBitmap btmpShwGrph; 
    wxBitmap btmpShwGrphClick; 
	wxBitmapButton *btnShowGraphic;
	////////////////////
	// animation
	wxBitmap *btmpBgAnim;
	wxBitmap fileImgBuf[1];
	wxStaticBitmap *imgBgAnim;
	wxWindow *wAnimWk1;
    //lbls
	wxStaticText *lblProjectName;
	wxStaticLine *lnProjName;
	wxStaticText *lblMyProgress;
	wxGauge *gaugeWUMain;
	wxStaticText *lblWrkUnitName;
	wxStaticText *lblElapsedTime;
	wxStaticText *lblElapsedTimeValue;
	wxStaticText *lblTimeRemaining;
	wxStaticText *lblTimeRemainingValue;

    CViewTabPage();
    CViewTabPage(
        wxFlatNotebook* parent, int index, std::string name
    );
    ~CViewTabPage();

	void LoadSkinImages();
    void CreatePage();
	void UpdateInterface();
    void OnBtnClick(wxCommandEvent& event);
    
	// Setters
	void SetTabName(const std::string nme) { m_name = nme; }
    // Getters
	std::string GetTabName() { return  m_name; }
		
	// Animation
    MyCanvas* GetCanvas() const { return m_canvas; }
    wxGIFAnimationCtrl* GetAnimationCtrl() const { return m_animationCtrl; }
	
	DECLARE_EVENT_TABLE()

protected:
	int m_tabIndex;
    //tab identifier
	std::string m_name;

	wxInt32 FormatCPUTime( RESULT* rslt, wxString& strBuffer ) const;
    wxInt32 FormatTimeToCompletion( RESULT* rslt, wxString& strBuffer ) const;
	void SGUITimeFormat( float fBuffer, wxString& strBuffer) const;
    
	MyCanvas*           m_canvas;
    wxGIFAnimationCtrl* m_animationCtrl;
#if 0
    wxAnimationPlayer   m_player;
    wxGIFAnimation      m_animation;
#endif

};


#endif


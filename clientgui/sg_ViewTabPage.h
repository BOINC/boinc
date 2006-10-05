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

#include <vector>
#include "common/wxFlatNotebook.h"
#include "BOINCGUIApp.h"
#include "sg_ImageLoader.h"


class MyCanvas : public wxScrolledWindow
{
public:
	MyCanvas(wxWindow *parent, const wxPoint& pos, const wxSize& size, std::vector<wxBitmap> images);
	~MyCanvas();
    void OnPaint(wxPaintEvent& event);

private:
	void OnChangeSlide(wxTimerEvent& WXUNUSED(event));
	wxTimer* changeSlideTimer;
	std::vector<ImageLoader*> vSlideShow;
	int currentImageIndex;
    DECLARE_EVENT_TABLE()
};

class CProgressBar;
class CStaticLine;
class CStaticLine;
class CImageButton;

class CViewTabPage : public wxPanel {
    DECLARE_DYNAMIC_CLASS( CViewTabPage )

public:
	
	bool isAlive;
	//Skin Class
    SkinClass *appSkin;
	// btns ////////////
	////////////////////
	// animation
	wxBitmap* btmpBgAnim;
	CImageButton * btnAminBg;
    //line
	CStaticLine *lnProjName;
	//strings
	wxString projName;
	wxString projectFrName;
	wxString wrkUnitName;
	wxString gaugePercent;
	wxString elapsedTimeValue;
	wxString timeRemainingValue;
	//wxGauge *gaugeWUMain;
	CProgressBar *gaugeWUMain;
	wxStaticText *lblWrkUnitName;
	wxStaticText *lblProjectFrName;
	wxString percStr;
	wxFloat64 percNum;
	// bg
	RESULT* resultWU;

    CViewTabPage();
    CViewTabPage(
		wxFlatNotebook* parent, RESULT* result, std::string name,std::string url
    );
    ~CViewTabPage();

    void CreatePage();
	void UpdateInterface();
	void ReskinInterface();
	void OnWorkShowGraphics();
	void OnPaint(wxPaintEvent& event); 
	void OnLeftUp(wxMouseEvent& event);
	void DrawText();
	void OnImageButton();
    
	// Setters
	void SetTabName(const std::string nme) { m_name = nme; }
    // Getters
	std::string GetTabName() { return  m_name; }
		
	// Animation
    MyCanvas* GetCanvas() const { return m_canvas; }
	
	DECLARE_EVENT_TABLE()

protected:

    //tab identifier
	std::string m_name;
	std::string m_prjUrl;
	bool m_hasGraphic;

	wxInt32 FormatCPUTime( RESULT* rslt, wxString& strBuffer ) const;
    wxInt32 FormatTimeToCompletion( RESULT* rslt, wxString& strBuffer ) const;
	void SGUITimeFormat( float fBuffer, wxString& strBuffer) const;

	void OnEraseBackground(wxEraseEvent& event);
    void DrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap bitMap,int opz);
    

private:

	void CreateSlideShowWindow();
	void LoadSlideShow(std::vector<wxBitmap> *vSlideShow);
	std::vector<wxBitmap> GetSlideShow();
	wxWindow* wSlideShow;
	MyCanvas* m_canvas;

};


#endif


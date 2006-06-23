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


#ifndef _SIMPLEFRAME_H_
#define _SIMPLEFRAME_H_


class MyCanvas : public wxScrolledWindow
{
public:
    MyCanvas(wxWindow *parent, const wxPoint& pos, const wxSize& size);
    void OnPaint(wxPaintEvent& event);

private:

    DECLARE_EVENT_TABLE()
};

// Define a new frame
class CSimpleFrame : public CBOINCBaseFrame
{
    DECLARE_DYNAMIC_CLASS(CSimpleFrame)

public:
    CSimpleFrame();
    CSimpleFrame(wxString title, wxIcon* icon);

   ~CSimpleFrame();

   


	// Images
	wxImage *g_prjIcnWCG;
	wxImage *g_prjIcnPDRC;
	wxImage *g_icoSleepWU;
	wxImage *g_icoWorkWU;
	//
	ImageLoader *i_prjIcnPT1;
	wxWindow *w_iconPT1;
	//
	ImageLoader *i_prjIcnP1;
	wxWindow *w_iconP1;
	///
	ImageLoader *i_prjIcnP2;
	wxWindow *w_iconP2;
	// Flat Neotebook
	wxWindow *wTab1;
    wxWindow *wTab2;
	wxFlatNotebookImageList m_ImageList;
	wxFlatNotebook *wrkUnitNB;
	//wxBitmap const sleepWUico;
    wxBitmap const workWUico;
    ////// Skin variables //////
	//XML doc
	wxXmlDocument *skinXML;
	//Skin Class
    SkinClass *appSkin;
	wxString skinPath;
	// My projects area
	wxImage *g_statWCG;
	StatImageLoader *i_statWCG;
	wxWindow *w_statWCG;

	//
	wxImage *g_statSeti;
	StatImageLoader *i_statSeti;
	wxWindow *w_statSeti;
	//
	wxImage *g_statPred;
	StatImageLoader *i_statPred;
	wxWindow *w_statPred;
	// arrows
    wxImage *g_arwLeft;
	wxImage *g_arwRight;
	wxImage *g_arwLeftClick;
	wxImage *g_arwRightClick;
	wxBitmap btmpArwL; 
    wxBitmap btmpArwR; 
    wxBitmap btmpArwLC; 
    wxBitmap btmpArwRC; 
    wxBitmapButton *btnArwLeft;
	wxBitmapButton *btnArwRight;
	//Collapse buttons
	bool midAppCollapsed;
	bool btmAppCollapsed;
	wxImage *g_collapse;
	wxImage *g_collapseClick;
	wxBitmap btmpCol; 
    wxBitmap btmpColClick; 
	wxBitmapButton *btnCollapseMid;
	//wxBitmapButton *btnCollapseBtm;
	//Expand buttons
    wxImage *g_expand;
	wxImage *g_expandClick;
	wxBitmap btmpExp; 
    wxBitmap btmpExpClick; 
	wxBitmapButton *btnExpandMid;
	////////////////////////////
	wxStaticText *st9c;
	wxWindow *w11c;
	wxStaticText *st12c;
	wxStaticLine *lno14c;
	wxStaticText *st15c;
	wxStaticText *st16c;
	wxGauge *gaugeWuP1;
	wxStaticText *st18c;
	wxStaticText *st19c;
	wxStaticText *st20c;
	wxStaticText *st21c;
	wxStaticText *st22c;
	wxStaticText *st23c;
	wxStaticText *stMyProj;
	wxStaticText *st27c;
	wxStaticLine *lnMyProjTop;
	wxStaticBitmap *bm29c;
	wxGauge *gaugeProjP1;
	wxBitmapButton *btnPreferences;
	wxBitmapButton *btnAttProj;
	wxStaticBitmap *icnProjWork;
	wxStaticBitmap *bm39c;
	wxGauge *gaugeProjP2;
	wxStaticText *st41c;
	wxStaticBitmap *icnProjSleep;
	wxBitmapButton *btnPause;
	wxBitmapButton *btnPlay;
	wxBitmapButton *btnMessages;
	wxBitmapButton *btnAdvancedView;
	wxStaticBitmap *imgBgAnim;
	wxStaticLine *lnMyProjBtm;

    void InitSimpleClient();
	void LoadSkinXML();
	void LoadSkinImages();
	void ReskinAppGUI();
	void initAfter();
	void MoveControlsUp();
	void MoveControlsDown();
	//////////
	wxFlexGridSizer *mainSizer;
	wxSize wxNotebookSize;
	wxWindow* CreateNotebookPage();
	//////////
	wxBitmap *CSimpleFrameImg0;
	wxBitmap *bm13cImg0;
	wxBitmap *btmpBtnPrefL;
	wxBitmap *btmpBtnAttProjL;
	wxBitmap *btmpIcnWorking;
	wxBitmap *bm39cImg0;
	wxBitmap *btmpBtnPauseL;
	wxBitmap *btmpBtnPlayL;
	wxBitmap *btmpMessagesBtnL;
	wxBitmap *btmpBtnAdvViewL;
	wxBitmap *btmpBgAnim;
	wxBitmap *btmpIcnSleeping;
	wxBitmap fileImgBuf[11];
	wxWindow *wAnimWk1;
    // Animation
    MyCanvas* GetCanvas() const { return m_canvas; }
    wxGIFAnimationCtrl* GetAnimationCtrl() const { return m_animationCtrl; }

#if 0
    wxAnimationPlayer& GetPlayer() { return m_player; }
    wxAnimationBase& GetAnimation() { return m_animation; }
#endif

    DECLARE_EVENT_TABLE()

protected:
	void OnEraseBackground(wxEraseEvent& event);
	void OnBtnClick(wxCommandEvent& event);
	void DrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz);
    wxPoint m_tmppoint;
    wxSize  m_tmpsize;
    wxPoint& SetwxPoint(long x,long y);
    wxSize& SetwxSize(long w,long h);
    /////////////////////////////////////////////////////
    MyCanvas*           m_canvas;
    wxGIFAnimationCtrl* m_animationCtrl;
#if 0
    wxAnimationPlayer   m_player;
    wxGIFAnimation      m_animation;
#endif
};

#endif


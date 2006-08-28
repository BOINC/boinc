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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_BoincSimpleGUI.cpp"
#endif

class CViewTabPage;
class StatImageLoader;
class SkinClass;
class ImageLoader;
class CProjectsComponent;
class ClientStateIndicator;

#include "common/wxAnimate.h"
#include "common/wxFlatNotebook.h"

// Define a new frame
class CSimpleFrame : public CBOINCBaseFrame
{
    DECLARE_DYNAMIC_CLASS(CSimpleFrame)

public:
    CSimpleFrame();
    CSimpleFrame(wxString title, wxIcon* icon);

   ~CSimpleFrame();

	// Images
	wxImage *g_icoWorkWU;
	//
	// Flat Neotebook
	std::vector<CViewTabPage*> m_windows; // vector of all window tabs created for notebook
	wxFlatNotebookImageList m_ImageList;
	wxFlatNotebook *wrkUnitNB;
    wxBitmap const workWUico;
    ////// Skin variables //////
	//XML doc
	wxXmlDocument *skinXML;
	//Skin Class
    SkinClass *appSkin;
	wxString skinName;
	// My projects component
	CProjectsComponent *projComponent;
	// Client State Indicator
	ClientStateIndicator *clientState;
	//Collapse button
	bool midAppCollapsed;
	bool btmAppCollapsed;
	////////////////////////////;
	bool projectViewInitialized;
	bool resultViewInitialized;
	bool emptyViewInitialized;
	bool notebookViewInitialized;

	void InitEmptyView();
	void UpdateEmptyView();
	void DestroyEmptyView();
    void InitResultView();
	void UpdateResultView();
	void InitProjectView();
	void UpdateProjectView();
	void LoadSkinImages();
	void ReskinAppGUI();
	void InitNotebook();
	void DestroyNotebook();
	void OnProjectsAttachToProject();
	//////////
	wxFlexGridSizer *mainSizer;
	wxSize wxNotebookSize;
	//////////
	wxBitmap *frameBg;
	wxBitmap *bm13cImg0;
	wxBitmap *btmpIcnWorking;
	wxBitmap *bm39cImg0;

	wxBitmap *btmpIcnSleeping;
	wxBitmap fileImgBuf[1];
	
#if 0
    wxAnimationPlayer& GetPlayer() { return m_player; }
    wxAnimationBase& GetAnimation() { return m_animation; }
#endif

    wxTimer*        m_pFrameRenderTimer;

    DECLARE_EVENT_TABLE()

protected:
	void OnEraseBackground(wxEraseEvent& event);
	void OnBtnClick(wxCommandEvent& event);
	void OnConnect(CFrameEvent& event );
    void OnFrameRender(wxTimerEvent& event );
	void OnPageChanged(wxFlatNotebookEvent& event);
    void DrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz);
};

#endif


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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


#ifndef _SIMPLEFRAME_H_
#define _SIMPLEFRAME_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_BoincSimpleGUI.cpp"
#endif

class CViewTabPage;
class StatImageLoader;
class ImageLoader;
class CProjectsComponent;
class ClientStateIndicator;
class WorkunitNotebook;

class CSimplePanel : public wxPanel
{
    DECLARE_DYNAMIC_CLASS(CSimplePanel)

public:
    CSimplePanel();
    CSimplePanel(wxWindow* parent);

   ~CSimplePanel();
	//
	// Flat Neotebook
	WorkunitNotebook *wrkUnitNB;
    wxBitmap const workWUico;
	// My projects component
	CProjectsComponent *projComponent;
	// Client State Indicator
	ClientStateIndicator *clientState;
	//Collapse button
	bool midAppCollapsed;
	bool btmAppCollapsed;
	////////////////////////////;
	bool projectViewInitialized;
	bool emptyViewInitialized;
	bool notebookViewInitialized;

    void ReskinInterface();
	void InitEmptyView();
	void UpdateEmptyView();
	void DestroyEmptyView();
	void InitResultView();
	void InitProjectView();
	void UpdateProjectView();
	void InitNotebook();
	void DestroyNotebook();
	void OnProjectsAttachToProject();
	void SetDlgOpen(bool newDlgState) { dlgOpen = newDlgState; }
	bool GetDlgOpen() { return dlgOpen; }
	//////////
	wxFlexGridSizer *mainSizer;
	wxSize wxNotebookSize;
	//////////
	wxBitmap *frameBg;
	wxBitmap *bm13cImg0;
	wxBitmap *btmpIcnWorking;
	wxBitmap *bm39cImg0;

	wxBitmap *btmpIcnSleeping;
	wxTimer* m_pFrameRenderTimer;

    DECLARE_EVENT_TABLE()

protected:
    void OnFrameRender(wxTimerEvent& event );
    void OnEraseBackground(wxEraseEvent& event);

private:
    bool dlgOpen;
};


// Define a new frame
class CSimpleFrame : public CBOINCBaseFrame
{
    DECLARE_DYNAMIC_CLASS(CSimpleFrame)

public:
    CSimpleFrame();
    CSimpleFrame(wxString title, wxIcon* icon, wxIcon* icon32);

   ~CSimpleFrame();

    void OnHelp( wxHelpEvent& event );
    void OnHelpBOINC( wxCommandEvent& event );

	void OnConnect(CFrameEvent& event );
    void OnProjectsAttachToProject();
    void OnReloadSkin( CFrameEvent& event );

private:
    bool SaveState();
    bool RestoreState();

protected:

#ifdef __WXMAC__
	wxMenuBar* m_pMenubar;
#endif

	wxAcceleratorEntry  m_Shortcuts[1];
    wxAcceleratorTable* m_pAccelTable;

	CSimplePanel* m_pBackgroundPanel;

    DECLARE_EVENT_TABLE()
};

#endif



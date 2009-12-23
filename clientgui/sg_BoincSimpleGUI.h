// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


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
class CDlgMessages;

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
    void OnFrameRender();
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

    DECLARE_EVENT_TABLE()

protected:
    void OnEraseBackground(wxEraseEvent& event);
#ifdef __WXMAC__
    void SetupMacAccessibilitySupport();
    void RemoveMacAccessibilitySupport();
    
    int oldSimpleGUIWorkCount;
    EventHandlerRef m_pSGAccessibilityEventHandlerRef;

#endif

private:
    bool dlgOpen;
};


// Define a new frame
class CSimpleFrame : public CBOINCBaseFrame
{
    DECLARE_DYNAMIC_CLASS(CSimpleFrame)

public:
    CSimpleFrame();
    CSimpleFrame(wxString title, wxIcon* icon, wxIcon* icon32, wxPoint position, wxSize size);

   ~CSimpleFrame();

    void OnChangeGUI( wxCommandEvent& event );
    void OnHelp( wxHelpEvent& event );
    void OnHelpBOINC( wxCommandEvent& event );

    void OnProjectsAttachToProject();

	void OnConnect(CFrameEvent& event );
    void OnReloadSkin( CFrameEvent& event );
    void OnRefreshView( CFrameEvent& event );

	void SetMsgsDlgOpen(CDlgMessages* newDlgPtr) { dlgMsgsPtr = newDlgPtr; }
    bool isMessagesDlgOpen() { return (dlgMsgsPtr != NULL); }

    bool SaveState();

protected:
    virtual int     _GetCurrentViewPage();

#ifdef __WXMAC__
	wxMenuBar*      m_pMenubar;
#endif
    wxAcceleratorEntry  m_Shortcuts[1];
    wxAcceleratorTable* m_pAccelTable;

	CSimplePanel* m_pBackgroundPanel;


private:
    CDlgMessages* dlgMsgsPtr;

    DECLARE_EVENT_TABLE()
};

#endif



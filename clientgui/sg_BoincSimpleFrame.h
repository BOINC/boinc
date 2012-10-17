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


#ifndef _BOINCSIMPLEFRAME_H_
#define _BOINCSIMPLEFRAME_H_

#include "BOINCBaseFrame.h"

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_BoincSimpleFrame.cpp"
#endif

class CSimpleTaskPanel;
class CSimpleProjectPanel;
class CSimpleTaskPanel;
class CDlgMessages;

class CSimpleGUIPanel : public wxPanel
{
    DECLARE_DYNAMIC_CLASS(CSimpleGUIPanel)

public:
    CSimpleGUIPanel();
    CSimpleGUIPanel(wxWindow* parent);

   ~CSimpleGUIPanel();
	//
    // My tasks panel (shown when there are active tasks)
    CSimpleTaskPanel *m_taskPanel;
	// My projects panel
	CSimpleProjectPanel *m_projPanel;
	////////////////////////////;

    void SetBackgroundBitmap();
    void ReskinInterface();
	void UpdateProjectView();
    void OnFrameRender();
	void OnProjectsAttachToProject();
    void OnShowNotices(wxCommandEvent& event);
    void OnSuspendResume(wxCommandEvent& event);
    void OnHelp( wxCommandEvent& event );
	void SetDlgOpen(bool newDlgState) { dlgOpen = newDlgState; }
	bool GetDlgOpen() { return dlgOpen; }
    wxBitmap *GetBackgroundBitMap() { return &m_bmpBg; }
    void OnCheckForNewNotices(wxTimerEvent& WXUNUSED(event));
    void NoticesViewed();

	//////////
	wxBoxSizer *mainSizer;
	//////////
    bool        m_bNewNoticeAlert;
    bool        m_bNoticesButtonIsRed;
    DECLARE_EVENT_TABLE()

protected:
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
#ifdef __WXMAC__
//    void SetupMacAccessibilitySupport();
//    void RemoveMacAccessibilitySupport();
    
//    EventHandlerRef m_pSGAccessibilityEventHandlerRef;

    int         m_iRedRingRadius;
#endif
    wxBitmap    m_bmpBg;
    wxButton    *m_NoticesButton;
    wxButton    *m_SuspendResumeButton;
    wxButton    *m_HelpButton;
    wxString    m_sSuspendString;
    wxString    m_sResumeString;
    int         m_oldWorkCount;
    bool        m_bIsSuspended;

private:
    int         m_irefreshCount;
    bool        dlgOpen;
	wxTimer*    checkForNewNoticesTimer;
    wxString    m_sSuspendButtonToolTip;
    wxString    m_sResumeButtonToolTip;
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
    void BuildSkinSubmenu( wxMenu *submenu );
    void OnSelectDefaultSkin( wxCommandEvent& event );
    void OnSelectSkin( wxCommandEvent& event );
    void OnPreferences( wxCommandEvent& event );
    void OnOptions( wxCommandEvent& event );
    void OnOldSG( wxCommandEvent& event );
    void OnHelp( wxHelpEvent& event );
    void OnHelpBOINC( wxCommandEvent& event );
    void OnHelpAbout( wxCommandEvent& event );

    void OnProjectsAttachToProject();

	void OnConnect(CFrameEvent& event );
    void OnReloadSkin( CFrameEvent& event );
    void OnRefreshView( CFrameEvent& event );
    void OnNotification( CFrameEvent& event );
    void OnEventLog(wxCommandEvent& event);
    
	void SetMsgsDlgOpen(CDlgMessages* newDlgPtr) { dlgMsgsPtr = newDlgPtr; }
    bool isMessagesDlgOpen() { return (dlgMsgsPtr != NULL); }

    bool SaveState();
    bool RestoreState();

protected:
    virtual int     _GetCurrentViewPage();

	wxMenuBar*          m_pMenubar;
    wxMenu*             m_pSubmenuSkins;
    wxAcceleratorEntry  m_Shortcuts[2];
    wxAcceleratorTable* m_pAccelTable;

	CSimpleGUIPanel* m_pBackgroundPanel;


private:
    CDlgMessages* dlgMsgsPtr;

    DECLARE_EVENT_TABLE()
};

#endif  // _BOINCSIMPLEFRAME_H_



// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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


#ifndef BOINC_SG_BOINCSIMPLEFRAME_H
#define BOINC_SG_BOINCSIMPLEFRAME_H

#include "BOINCBaseFrame.h"

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_BoincSimpleFrame.cpp"
#endif

class CSimpleTaskPanel;
class CSimpleProjectPanel;
class CSimpleTaskPanel;
class CDlgMessages;
class CDlgPreferences;

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
    void OnProjectsAttachToProject(wxCommandEvent& event);
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


// Simple view frame
//
class CSimpleFrame : public CBOINCBaseFrame
{
    DECLARE_DYNAMIC_CLASS(CSimpleFrame)

public:
    CSimpleFrame();
    CSimpleFrame(wxString title, wxIconBundle* icons, wxPoint position, wxSize size);

   ~CSimpleFrame();

    void OnMenuOpening( wxMenuEvent &event);
    void OnChangeGUI( wxCommandEvent& event );
    void BuildSkinSubmenu( wxMenu *submenu );
    void OnSelectDefaultSkin( wxCommandEvent& event );
    void OnSelectSkin( wxCommandEvent& event );
    void OnPreferences( wxCommandEvent& event );
    void OnOptions( wxCommandEvent& event );
    void OnDiagnosticLogFlags( wxCommandEvent& event );
    void OnHelp( wxHelpEvent& event );
    void OnHelpBOINC( wxCommandEvent& event );
    void OnHelpAbout( wxCommandEvent& event );
    void OnCheckVersion( wxCommandEvent& event );
    void OnReportBug( wxCommandEvent& event );

    void OnConnect(CFrameEvent& event );
    void OnReloadSkin( CFrameEvent& event );
    void OnRefreshView( CFrameEvent& event );
    void OnNotification( CFrameEvent& event );
    void OnEventLog(wxCommandEvent& event);
    void OnDarkModeChanged( wxSysColourChangedEvent& event );

    void SetMsgsDlgOpen(CDlgMessages* newDlgPtr) { dlgMsgsPtr = newDlgPtr; }
    bool isMessagesDlgOpen() { return (dlgMsgsPtr != NULL); }

    bool SaveWindowPosition();
    bool SaveState();
    bool RestoreState();
    void OnMove(wxMoveEvent& event);

    void StartTimers() {
        StartTimersBase();
    }
    void StopTimers() {
        StopTimersBase();
    }
    bool CreateMenus();

protected:
    virtual int     _GetCurrentViewPage();

    wxMenuBar*          m_pMenubar;
    wxMenu*             m_pSubmenuSkins;
    wxAcceleratorEntry  m_Shortcuts[3];
    wxAcceleratorTable* m_pAccelTable;

    CSimpleGUIPanel* m_pBackgroundPanel;


private:
    CDlgMessages* dlgMsgsPtr;
    CDlgPreferences* dlgPrefsPtr;

    wxBoxSizer* mainSizer;

    DECLARE_EVENT_TABLE()
};

#endif

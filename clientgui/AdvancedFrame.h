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


#ifndef _ADVANCEDFRAME_H_
#define _ADVANCEDFRAME_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "AdvancedFrame.cpp"
#endif


class CBOINCBaseView;
class CDlgEventLog;

class CStatusBar : public wxStatusBar
{
    DECLARE_DYNAMIC_CLASS(CStatusBar)

public:
    CStatusBar();
    CStatusBar(wxWindow *parent);
    ~CStatusBar();

    void OnSize(wxSizeEvent& event);

    wxStaticBitmap* m_pbmpConnected;
    wxStaticText*   m_ptxtConnected;
    wxStaticBitmap* m_pbmpDisconnect;
    wxStaticText*   m_ptxtDisconnect;

private:
    DECLARE_EVENT_TABLE()
};


class CAdvancedFrame : public CBOINCBaseFrame
{
    DECLARE_DYNAMIC_CLASS( CAdvancedFrame )

public:
    CAdvancedFrame();
    CAdvancedFrame( wxString title, wxIcon* icon, wxIcon* icon32, wxPoint position, wxSize size );

    ~CAdvancedFrame(void);

    void OnChangeView( wxCommandEvent& event );
    void OnChangeGUI( wxCommandEvent& event );

    void OnWizardAttach( wxCommandEvent& event );
    void OnWizardUpdate( wxCommandEvent& event );
    void OnWizardDetach( wxCommandEvent& event );

    void OnActivitySelection( wxCommandEvent& event );
    void OnGPUSelection( wxCommandEvent& event );
    void OnNetworkSelection( wxCommandEvent& event );

    void OnOptions( wxCommandEvent& event );
	void OnPreferences( wxCommandEvent& event );
    void OnSelectComputer( wxCommandEvent& event );
    void OnClientShutdown( wxCommandEvent& event );
    void OnRunBenchmarks( wxCommandEvent& event );
    void OnRetryCommunications( wxCommandEvent& event );
    void OnReadPreferences( wxCommandEvent& event );
    void OnReadConfig( wxCommandEvent& event );
    void OnEventLog( wxCommandEvent& event );

    void OnHelp( wxHelpEvent& event );
    void OnHelpBOINC( wxCommandEvent& event );
    void OnHelpAbout( wxCommandEvent& event );

    void OnRefreshState( wxTimerEvent& event );
    void OnFrameRender( wxTimerEvent& event );

    void OnNotebookSelectionChanged( wxNotebookEvent& event );

    void OnRefreshView( CFrameEvent& event );
    void OnConnect( CFrameEvent& event );
    void OnNotification( CFrameEvent& event );

    void OnUpdateStatus( CFrameEvent& event );

    void ResetReminderTimers();

    bool RestoreState();
    bool SaveState();

    wxTimer*        m_pRefreshStateTimer;
    wxTimer*        m_pFrameRenderTimer;


protected:
    virtual int     _GetCurrentViewPage();

    wxAcceleratorEntry  m_Shortcuts[1];     // For HELP keyboard shortcut
    wxAcceleratorTable* m_pAccelTable;

private:

    wxMenuBar*      m_pMenubar;
    wxNotebook*     m_pNotebook;
    CStatusBar*     m_pStatusbar;

    wxString        m_strBaseTitle;

    bool            CreateMenu();
    bool            DeleteMenu();

    bool            CreateNotebook();
    bool            RepopulateNotebook();
    bool            CreateNotebookPage( CBOINCBaseView* pwndNewNotebookPage );
    bool            DeleteNotebook();
    void            UpdateNoticesTabText();
    bool            CreateStatusbar();
    bool            DeleteStatusbar();

    void            SaveWindowDimensions();

    void            UpdateActivityModeControls( CC_STATUS& status );
    void            UpdateGPUModeControls( CC_STATUS& status );
    void            UpdateNetworkModeControls( CC_STATUS& status );
    void            UpdateRefreshTimerInterval( wxInt32 iCurrentNotebookPage );
    void            UpdateRefreshTimerInterval();

    void            StartTimers();
    void            StopTimers();

    DECLARE_EVENT_TABLE()
};


#endif



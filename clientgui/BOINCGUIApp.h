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
//

///
/// @defgroup BOINCMgr BOINC Manager
/// The BOINC Manager
/// @{

#ifndef _BOINCGUIAPP_H_
#define _BOINCGUIAPP_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCGUIApp.cpp"
#endif

#ifdef __WXMAC__
#include "mac/MacSysMenu.h"     // Must be included before MainDocument.h
#endif

///
/// Which view is on display
///
#define BOINC_ADVANCEDGUI                   1
#define BOINC_SIMPLEGUI                     2


class wxLogBOINC;
class CBOINCBaseFrame;
class CMainDocument;
class CTaskBarIcon;
class CSkinManager;
class CDlgEventLog;
class CRPCFinishedEvent;


class CBOINCGUIApp : public wxApp {

    DECLARE_DYNAMIC_CLASS(CBOINCGUIApp)

protected:
    int                 OnExit();

    void                OnInitCmdLine(wxCmdLineParser &parser);
    bool                OnCmdLineParsed(wxCmdLineParser &parser);

    bool                DetectDuplicateInstance();
    void                DetectExecutableName();
    void                DetectRootDirectory();
    void                DetectDataDirectory();

    void                InitSupportedLanguages();

    int                 IdleTrackerAttach();
    int                 IdleTrackerDetach();

    wxConfig*           m_pConfig;
    wxLocale*           m_pLocale;
    wxLogBOINC*         m_pLog;

    CSkinManager*       m_pSkinManager;
    CBOINCBaseFrame*    m_pFrame;
    CMainDocument*      m_pDocument;
    CTaskBarIcon*       m_pTaskBarIcon;
    CDlgEventLog*       m_pEventLog;
#ifdef __WXMAC__
    CMacSystemMenu*     m_pMacSystemMenu;
#endif
    wxFileSystemHandler* m_pInternetFSHandler;
    
    wxString            m_strBOINCMGRExecutableName;
    wxString            m_strBOINCMGRRootDirectory;
    wxString            m_strBOINCMGRDataDirectory;
    wxString            m_strHostNameArg;
    wxString            m_strPasswordArg;
    wxString            m_strBOINCArguments;
    int                 m_iRPCPortArg;

    bool                m_bBOINCMGRAutoStarted;
    int                 m_iBOINCMGRDisableAutoStart;
    int                 m_iShutdownCoreClient;
    int                 m_iDisplayExitDialog;

    bool                m_bGUIVisible;
    
    int                 m_iGUISelected;
    bool                m_bDebugSkins;
    bool                m_bMultipleInstancesOK;
    bool                m_bFilterEvents;

#ifdef __WXMSW__
    HINSTANCE           m_hClientLibraryDll;
#endif
#ifdef __WXMAC__
    ProcessSerialNumber m_psnCurrentProcess;
#endif


    // The last value defined in the wxLanguage enum is wxLANGUAGE_USER_DEFINED.
    // defined in: wx/intl.h
    wxArrayString       m_astrLanguages;
    
    int                 m_bSafeMessageBoxDisplayed;

public:

    bool                OnInit();

    wxLocale*           GetLocale()                 { return m_pLocale; }
    CSkinManager*       GetSkinManager()            { return m_pSkinManager; }
    CBOINCBaseFrame*    GetFrame()                  { return m_pFrame; }
    CMainDocument*      GetDocument()               { return m_pDocument; }
    wxString            GetExecutableName()         { return m_strBOINCMGRExecutableName; }
    wxString            GetRootDirectory()          { return m_strBOINCMGRRootDirectory; }
    wxString            GetDataDirectory()          { return m_strBOINCMGRDataDirectory; }
    wxString            GetClientHostNameArg()      { return m_strHostNameArg; }    
    wxString            GetClientPasswordArg()      { return m_strPasswordArg; }    
    wxString            GetArguments()              { return m_strBOINCArguments; }
    int                 GetClientRPCPortArg()       { return m_iRPCPortArg; }
    CDlgEventLog*       GetEventLog()               { return m_pEventLog; }
    CTaskBarIcon*       GetTaskBarIcon()            { return m_pTaskBarIcon; }
    void                DeleteTaskBarIcon();

    bool                IsMgrMultipleInstance()     { return m_bMultipleInstancesOK; }

#ifdef __WXMAC__
    CMacSystemMenu*     GetMacSystemMenu()          { return m_pMacSystemMenu; }
    void                DeleteMacSystemMenu();
    int                 ShouldShutdownCoreClient()  { return true; }
#else
    int                 ShouldShutdownCoreClient()  { return m_iShutdownCoreClient; }
#endif

    int                 GetBOINCMGRDisableAutoStart()
                                                    { return m_iBOINCMGRDisableAutoStart; }
    void                SetBOINCMGRDisableAutoStart(int iDisableAutoStart)
                                                    { m_iBOINCMGRDisableAutoStart = iDisableAutoStart; }

    int                 GetBOINCMGRDisplayExitMessage()
                                                    { return m_iDisplayExitDialog; }
    void                SetBOINCMGRDisplayExitMessage(int iDisplayExitMessage)
                                                    { m_iDisplayExitDialog = iDisplayExitMessage; }


    wxArrayString&      GetSupportedLanguages()     { return m_astrLanguages; }
    
    wxFileSystemHandler*   GetInternetFSHandler()  { return m_pInternetFSHandler; }

    void                DisplayEventLog(bool bShowWindow = true);
    void                OnEventLogClose();

    void                FireReloadSkin();
    void                FrameClosed()               { m_pFrame = NULL; }

    int                 StartBOINCScreensaverTest();
    int                 StartBOINCDefaultScreensaverTest();

    bool                SetActiveGUI(int iGUISelection, bool bShowWindow = true);
    
    void                OnActivateApp( wxActivateEvent& event );
    void                OnRPCFinished( CRPCFinishedEvent& event );
    
    int                 ConfirmExit();

    int                 SafeMessageBox(
                            const wxString& message,
                            const wxString& caption = wxMessageBoxCaptionStr,
                            long style = wxOK | wxCENTRE,
                            wxWindow *parent = NULL,
                            int x = wxDefaultCoord,
                            int y = wxDefaultCoord
                        );

    int                 IsAnotherInstanceRunning();
    bool                IsApplicationVisible();
    void                ShowApplication(bool bShow);
    bool                ShowInterface();
    bool                ShowNotifications();

    bool                IsModalDialogDisplayed();
    bool                IsSafeMesageBoxDisplayed() { return (m_bSafeMessageBoxDisplayed != 0); };

    int                 FilterEvent(wxEvent &event);


    int                 UpdateSystemIdleDetection();
    
    void                SetEventFiltering(bool set) { m_bFilterEvents = set; }

DECLARE_EVENT_TABLE()
};


DECLARE_APP(CBOINCGUIApp)


#endif

/// @}

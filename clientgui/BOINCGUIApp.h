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

#ifndef _BOINCGUIAPP_H_
#define _BOINCGUIAPP_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCGUIApp.cpp"
#endif

#ifdef __WXMAC__
#include "mac/MacSysMenu.h"     // Must be included before MainDocument.h
#endif

#define BOINC_ADVANCEDGUI                   1
#define BOINC_SIMPLEGUI                     2


class wxLogBOINC;
class CBOINCBaseFrame;
class CMainDocument;
class CTaskBarIcon;
class CSkinManager;


class CBOINCGUIApp : public wxApp {
    DECLARE_DYNAMIC_CLASS(CBOINCGUIApp)

protected:
    int                 OnExit();

    void                OnInitCmdLine(wxCmdLineParser &parser);
    bool                OnCmdLineParsed(wxCmdLineParser &parser);

    void                DetectDisplayInfo();

    void                InitSupportedLanguages();

    bool                IsBOINCCoreRunning();
    void                StartupBOINCCore();
    void                ShutdownBOINCCore();
#ifdef __WXMAC__
    bool                ProcessExists(pid_t thePID);
    static OSErr        QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon );
#endif

    int                 ClientLibraryStartup();
    int                 ClientLibraryShutdown();

    wxConfig*           m_pConfig;
    wxLocale*           m_pLocale;
    wxLogBOINC*         m_pLog;

    CSkinManager*       m_pSkinManager;
    CBOINCBaseFrame*    m_pFrame;
    CMainDocument*      m_pDocument;
    CTaskBarIcon*       m_pTaskBarIcon;
#ifdef __WXMAC__
    CMacSystemMenu*     m_pMacSystemMenu;
#endif

    bool                m_bBOINCStartedByManager;
    wxString            m_strBOINCArguments;

    int                 m_iDisplayExitWarning;

    bool                m_bGUIVisible;
    int                 m_iGUISelected;

    int                 m_lBOINCCoreProcessId;

#ifdef __WXMSW__
    HANDLE              m_hBOINCCoreProcess;
    HINSTANCE           m_hClientLibraryDll;
#endif

    // The last value defined in the wxLanguage enum is wxLANGUAGE_USER_DEFINED.
    // defined in: wx/intl.h
    wxArrayString       m_astrLanguages;

public:

    wxString            m_strDefaultWindowStation;
    wxString            m_strDefaultDesktop;
    wxString            m_strDefaultDisplay;

    bool                OnInit();

    bool                AutoRestartBOINC();

    int                 IsNetworkAlive(long* lpdwFlags);
    int                 IsNetworkAlwaysOnline();
    int                 UpdateSystemIdleDetection();

    int                 StartBOINCScreensaverTest();

    wxLocale*           GetLocale()                 { return m_pLocale; }
    CSkinManager*       GetSkinManager()            { return m_pSkinManager; }
    CBOINCBaseFrame*    GetFrame()                  { return m_pFrame; }
    CMainDocument*      GetDocument()               { return m_pDocument; }
#if defined(__WXMSW__) || defined(__WXMAC__)
    CTaskBarIcon*       GetTaskBarIcon()            { return m_pTaskBarIcon; }
#endif
#ifdef __WXMAC__
    CMacSystemMenu*     GetMacSystemMenu()          { return m_pMacSystemMenu; }
    int                 GetCurrentGUISelection()    { return m_iGUISelected; }
#endif

    wxArrayString&      GetSupportedLanguages()     { return m_astrLanguages; }

    int                 GetDisplayExitWarning() { return m_iDisplayExitWarning; }
    void                SetDisplayExitWarning(int display) { m_iDisplayExitWarning = display; }

    void                FireReloadSkin();

    bool                SetActiveGUI(int iGUISelection, bool bShowWindow = true);
    
    int                 ConfirmExit();
};


DECLARE_APP(CBOINCGUIApp)


#endif


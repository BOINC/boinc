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
class CRPCFinishedEvent;


class CBOINCGUIApp : public wxApp {

    DECLARE_DYNAMIC_CLASS(CBOINCGUIApp)

protected:
    int                 OnExit();

    void                OnInitCmdLine(wxCmdLineParser &parser);
    bool                OnCmdLineParsed(wxCmdLineParser &parser);

    void                DetectDisplayInfo();
    void                DetectRootDirectory();
    void                DetectDataDirectory();

    void                InitSupportedLanguages();

    int                 ClientLibraryStartup();
    int                 IdleTrackerAttach();
    int                 IdleTrackerDetach();
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

    wxString            m_strBOINCMGRRootDirectory;
    wxString            m_strBOINCMGRDataDirectory;
    wxString            m_strBOINCArguments;

    int                 m_iDisplayExitWarning;

    bool                m_bGUIVisible;
    int                 m_iGUISelected;

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

    wxString            m_strDefaultWindowStation;
    wxString            m_strDefaultDesktop;
    wxString            m_strDefaultDisplay;

    bool                OnInit();

    wxLocale*           GetLocale()                 { return m_pLocale; }
    CSkinManager*       GetSkinManager()            { return m_pSkinManager; }
    CBOINCBaseFrame*    GetFrame()                  { return m_pFrame; }
    CMainDocument*      GetDocument()               { return m_pDocument; }
    wxString            GetArguments()              { return m_strBOINCArguments; }
    wxString            GetRootDirectory()          { return m_strBOINCMGRRootDirectory; }
    wxString            GetDataDirectory()          { return m_strBOINCMGRDataDirectory; }
#if defined(__WXMSW__) || defined(__WXMAC__)
    CTaskBarIcon*       GetTaskBarIcon()            { return m_pTaskBarIcon; }
#endif
#ifdef __WXMAC__
    CMacSystemMenu*     GetMacSystemMenu()          { return m_pMacSystemMenu; }
#endif

    wxArrayString&      GetSupportedLanguages()     { return m_astrLanguages; }

    int                 GetDisplayExitWarning()     { return m_iDisplayExitWarning; }
    void                SetDisplayExitWarning(int display) { m_iDisplayExitWarning = display; }

    void                FireReloadSkin();
    void                FrameClosed()               { m_pFrame = NULL; }

    int                 StartBOINCScreensaverTest();

    bool                SetActiveGUI(int iGUISelection, bool bShowWindow = true);

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

    bool                IsApplicationVisible();
    void                ShowApplication(bool bShow);

    bool                IsModalDialogDisplayed();

    int                 FilterEvent(wxEvent &event);

    int                 UpdateSystemIdleDetection();
};


DECLARE_APP(CBOINCGUIApp)


#endif

/// @}

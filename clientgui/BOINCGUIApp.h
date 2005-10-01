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

#include "LogBOINC.h"
#include "MainFrame.h"

#include "BOINCTaskBar.h"   // Must be included before MainDocument.h
#ifdef __WXMAC__
#include "mac/MacSysMenu.h"     // Must be included before MainDocument.h
#endif

#include "MainDocument.h"


class CBOINCGUIApp : public wxApp {
    DECLARE_DYNAMIC_CLASS(CBOINCGUIApp)

protected:
    int             OnExit();

    void            OnInitCmdLine(wxCmdLineParser &parser);
    bool            OnCmdLineParsed(wxCmdLineParser &parser);

    void            DetectDisplayInfo();

    void            InitSupportedLanguages();

    bool            IsBOINCCoreRunning();
    void            StartupBOINCCore();
    void            ShutdownBOINCCore();
#ifdef __WXMAC__
    bool            ProcessExists(pid_t thePID);
#endif

    int             StartupSystemIdleDetection();
    int             ShutdownSystemIdleDetection();

    wxConfig*       m_pConfig;
    wxLocale*       m_pLocale;
    wxLogBOINC*     m_pLog;

    CMainFrame*     m_pFrame;
    CMainDocument*  m_pDocument;
#if defined(__WXMSW__) || defined(__WXMAC__)
    CTaskBarIcon*   m_pTaskBarIcon;
#endif
#ifdef __WXMAC__
    CMacSystemMenu* m_pMacSystemMenu;
#endif

    bool            m_bBOINCStartedByManager;
    bool            m_bFrameVisible;

    int             m_lBOINCCoreProcessId;

#ifdef __WXMSW__
    HANDLE          m_hBOINCCoreProcess;
    HINSTANCE       m_hIdleDetectionDll;
#endif

    // The last value defined in the wxLanguage enum is wxLANGUAGE_USER_DEFINED.
    // defined in: wx/intl.h
    wxArrayString  m_astrLanguages;

public:

    std::string     m_strDefaultWindowStation;
    std::string     m_strDefaultDesktop;
    std::string     m_strDefaultDisplay;

    bool            OnInit();

    int         UpdateSystemIdleDetection();

    CMainFrame*     GetFrame()                   { return m_pFrame; }
    CMainDocument*  GetDocument()                { return m_pDocument; }
#if defined(__WXMSW__) || defined(__WXMAC__)
    CTaskBarIcon*   GetTaskBarIcon()             { return m_pTaskBarIcon; }
#endif
#ifdef __WXMAC__
    CMacSystemMenu* GetMacSystemMenu()           { return m_pMacSystemMenu; }
#endif

    wxArrayString&  GetSupportedLanguages()      { return m_astrLanguages; }
};


DECLARE_APP(CBOINCGUIApp)


#endif


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


#include "MainFrame.h"
#include "MainDocument.h"
#ifndef NOTASKBAR
#include "BOINCTaskBar.h"
#endif


class CBOINCGUIApp : public wxApp
{
    DECLARE_DYNAMIC_CLASS(CBOINCGUIApp)

protected:
    int             OnExit();

    void            OnInitCmdLine(wxCmdLineParser &parser);
    bool            OnCmdLineParsed(wxCmdLineParser &parser);

    void            DetectDefaultWindowStation();
    void            DetectDefaultDesktop();

    bool            IsBOINCCoreRunning();
    void            StartupBOINCCore();
    void            ShutdownBOINCCore();

    wxInt32         StartupSystemIdleDetection();
    wxInt32         ShutdownSystemIdleDetection();

    wxLocale*       m_pLocale;
    wxConfig*       m_pConfig;

    CMainFrame*     m_pFrame;
    CMainDocument*  m_pDocument;
#ifndef NOTASKBAR
    CTaskBarIcon*   m_pTaskBarIcon;
#endif

    bool            m_bBOINCStartedByManager;
    bool            m_bFrameVisible;

    wxInt32         m_lBOINCCoreProcessId;

#ifdef __WXMSW__
    HANDLE          m_hBOINCCoreProcess;
    HINSTANCE       m_hIdleDetectionDll;
#endif

    wxString        m_strDefaultWindowStation;
    wxString        m_strDefaultDesktop;

public:

    bool            OnInit();

    wxInt32         UpdateSystemIdleDetection();

    CMainFrame*     GetFrame()                { return m_pFrame; };
    CMainDocument*  GetDocument()             { return m_pDocument; };
#ifndef NOTASKBAR
    CTaskBarIcon*   GetTaskBarIcon()          { return m_pTaskBarIcon; };
#endif

    wxString        GetDefaultWindowStation() { return m_strDefaultWindowStation; };
    wxString        GetDefaultDesktop()       { return m_strDefaultDesktop; };

};


DECLARE_APP(CBOINCGUIApp)


#endif


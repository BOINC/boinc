static volatile const char *BOINCrcsid="$Id$";
// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCGUIApp.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainFrame.h"
#include "MainDocument.h"


IMPLEMENT_APP(CBOINCGUIApp)
IMPLEMENT_DYNAMIC_CLASS(CBOINCGUIApp, wxApp)


bool CBOINCGUIApp::OnInit()
{
    // Setup application and company information
    SetVendorName(wxT("Space Sciences Laboratory, U.C. Berkeley"));
    SetAppName(wxT("BOINC Manager"));

    // Setup variables with default values
    m_bBOINCStartedByManager = false;
    m_bFrameVisible = true;
    m_lBOINCCoreProccessId = 0;

    // Enable Trace Masks
    wxLog::AddTraceMask( wxT("Function Start/End") );


    // Enable the in memory virtual file system for
    //   storing images
    wxFileSystem::AddHandler(new wxMemoryFSHandler);

    // Enable known image types
    wxImage::AddHandler(new wxXPMHandler);

    // Commandline parsing is done in wxApp::OnInit()
    if (!wxApp::OnInit())
        return false;

    // Initialize the internationalization module
    m_pLocale = new wxLocale();
    wxASSERT(NULL != m_pLocale);

    m_pLocale->Init();
#ifdef __WXMSW__
    // In Windows local information is stored relative to the executable,
    //   in the Unix world it may be different.
    m_pLocale->AddCatalogLookupPathPrefix(wxT("locale"));
#endif
    m_pLocale->AddCatalog(GetAppName());

    // Initialize the configuration storage module
    m_pConfig = new wxConfig(GetAppName());
    wxConfigBase::Set(m_pConfig);
    wxASSERT(NULL != m_pConfig);

    // Initialize the main document
    m_pDocument = new CMainDocument();
    wxASSERT(NULL != m_pDocument);

    m_pDocument->OnInit();

    // Initialize the main gui window
    m_pFrame = new CMainFrame(GetAppName());
    wxASSERT(NULL != m_pFrame);

#ifndef NOTASKBAR
    // Initialize the task bar icon
    m_pTaskBarIcon = new CTaskBarIcon();
    wxASSERT(NULL != m_pTaskBarIcon);
#endif

    // Detect the default Window Station and Desktop and store the
    //   information for later use.
    DetectDefaultWindowStation();
    DetectDefaultDesktop();

    // Detect if we need to start the BOINC Core Client due to configuration
    StartupBOINCCore();

    // Show the UI
    SetTopWindow(m_pFrame);
    if (m_bFrameVisible)
        m_pFrame->Show();

    return true;
}


int CBOINCGUIApp::OnExit()
{
    // Detect if we need to stop the BOINC Core Client due to configuration
    ShutdownBOINCCore();

#ifndef NOTASKBAR
    if (m_pTaskBarIcon)
        delete m_pTaskBarIcon;
#endif

    if (m_pDocument)
    {
        m_pDocument->OnExit();
        delete m_pDocument;
    }

    if (m_pLocale)
        delete m_pLocale;

    return wxApp::OnExit();
}


void CBOINCGUIApp::OnInitCmdLine(wxCmdLineParser &parser)
{
    wxApp::OnInitCmdLine(parser);
    static const wxCmdLineEntryDesc cmdLineDesc[] = {
        { wxCMD_LINE_SWITCH, wxT("s"), wxT("systray"), _("Startup BOINC so only the system tray icon is visible")},
        { wxCMD_LINE_NONE}  //DON'T forget this line!!
    };
    parser.SetDesc(cmdLineDesc);
}


bool CBOINCGUIApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
    // Give default processing (-?, --help and --verbose) the chance to do something.
    wxApp::OnCmdLineParsed(parser);
    if (parser.Found(wxT("systray")))
    {
        m_bFrameVisible = false;
    }
    return true;
}


void CBOINCGUIApp::DetectDefaultWindowStation()
{
    wxChar szWindowStation[256];
    memset(szWindowStation, 0, sizeof(szWindowStation)/sizeof(wxChar));

#ifdef __WXMSW__

    if ( wxWIN95 != wxGetOsVersion( NULL, NULL ) )
    {
        // Retrieve the current window station and desktop names
        GetUserObjectInformation( 
            GetProcessWindowStation(), 
            UOI_NAME, 
            szWindowStation,
            (sizeof(szWindowStation) / sizeof(wxChar)),
            NULL
        );
    }

#endif

    m_strDefaultWindowStation = szWindowStation;
}


void CBOINCGUIApp::DetectDefaultDesktop()
{
    wxChar szDesktop[256];
    memset(szDesktop, 0, sizeof(szDesktop)/sizeof(wxChar));

#ifdef __WXMSW__

    if ( wxWIN95 != wxGetOsVersion( NULL, NULL ) )
    {
        GetUserObjectInformation( 
            GetThreadDesktop(GetCurrentThreadId()), 
            UOI_NAME, 
            szDesktop,
            (sizeof(szDesktop) / sizeof(wxChar)),
            NULL
        );
    }

#endif

    m_strDefaultDesktop = szDesktop;
}


bool CBOINCGUIApp::IsBOINCCoreRunning()
{
    wxInt32 iMode = -1;
    return ( 0 == m_pDocument->GetActivityRunMode(iMode));
}


void CBOINCGUIApp::StartupBOINCCore()
{
    if ( !IsBOINCCoreRunning() )
    {
        wxString strExecute = wxEmptyString;
        wxChar   szExecutableDirectory[4096];

        memset(szExecutableDirectory, 0, sizeof(szExecutableDirectory));

#ifdef __WXMSW__

        // On the surface it would seem that GetCurrentDirectory would be a better choice
        //   for determing which directory we should prepend to the execution string before
        //   starting BOINC, except that we cannot depend on any shortcuts being configured
        //   to startup in the correct directory, since the user may have created the
        //   shortcut themselves.  So determine where boinc.exe is based off of our
        //   current execution location and then execute it.
        GetModuleFileName( 
            NULL, 
            szExecutableDirectory,
            (sizeof(szExecutableDirectory) / sizeof(wxChar) )
        );

#endif

        // We are only interested in the path component of the fully qualified path.
        wxFileName::SplitPath( szExecutableDirectory, &strExecute, NULL, NULL );

        // Set the current directory ahead of the application launch so the core
        //   client can find its files
        ::wxSetWorkingDirectory( strExecute );

#ifdef __WXMSW__

        // Append boinc.exe to the end of the strExecute string and get ready to rock
        strExecute += wxT("\\boinc.exe");

        PROCESS_INFORMATION pi;
        STARTUPINFO         si;
        BOOL                bProcessStarted;

        memset(&pi, 0, sizeof(pi));
        memset(&si, 0, sizeof(si));
 
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        bProcessStarted = CreateProcess(
            NULL,
            (LPTSTR)strExecute.c_str(),
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi
        );
        if (bProcessStarted)
        {
            m_lBOINCCoreProccessId = pi.dwProcessId;
        }

#else

        // Append boinc.exe to the end of the strExecute string and get ready to rock
        strExecute += wxT("/boinc");
        m_lBOINCCoreProccessId = ::wxExecute( strExecute );

#endif

        if ( 0 != m_lBOINCCoreProccessId )
            m_bBOINCStartedByManager = true;
    }
}


void CBOINCGUIApp::ShutdownBOINCCore()
{
    wxInt32 iCount = 0;
    bool    bClientQuit = false;

    if ( m_bBOINCStartedByManager )
    {
        if ( wxProcess::Exists( m_lBOINCCoreProccessId ) )
        {
            m_pDocument->CoreClientQuit();
/*
            for ( iCount = 0; iCount <= 10; iCount++ )
            {
                if ( !bClientQuit && !wxProcess::Exists( m_lBOINCCoreProccessId ) )
                {
                    bClientQuit = true;
                    continue;
                }
                ::wxSleep(1);
            }

            if ( !bClientQuit )
                ::wxKill( m_lBOINCCoreProccessId );
*/
        }
    }
}


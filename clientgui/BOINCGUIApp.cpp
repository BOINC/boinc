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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCGUIApp.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "diagnostics.h"
#include "MainFrame.h"
#include "MainDocument.h"


#ifdef __WXMSW__
typedef BOOL (CALLBACK* IdleTrackerInit)();
typedef void (CALLBACK* IdleTrackerTerm)();
typedef DWORD (CALLBACK* IdleTrackerGetIdleTickCount)();
#endif


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
    m_lBOINCCoreProcessId = 0;
#ifdef __WXMSW__
    m_hBOINCCoreProcess = NULL;
    m_hIdleDetectionDll = NULL;
#endif

    // Initialize the BOINC Diagnostics Framework
    int dwDiagnosticsFlags =
        BOINC_DIAG_DUMPCALLSTACKENABLED | 
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_MEMORYLEAKCHECKENABLED |
        BOINC_DIAG_REDIRECTSTDERR |
        BOINC_DIAG_REDIRECTSTDOUT |
        BOINC_DIAG_TRACETOSTDOUT;

    diagnostics_init(
        dwDiagnosticsFlags,
        "stdoutgui",
        "stderrgui"
    );

    // Initialize the configuration storage module
    m_pConfig = new wxConfig(GetAppName());
    wxConfigBase::Set(m_pConfig);
    wxASSERT(NULL != m_pConfig);

    // Enable Logging and Trace Masks
    m_pLog = new wxLogBOINC();
    wxLog::SetActiveTarget(m_pLog);

    m_pLog->AddTraceMask( wxT("Function Start/End") );

    // Enable the in memory virtual file system for
    //   storing images
    wxFileSystem::AddHandler(new wxMemoryFSHandler);

    // Enable known image types
    wxImage::AddHandler(new wxXPMHandler);

    // Initialize the internationalization module
    m_pLocale = new wxLocale();
    wxASSERT(NULL != m_pLocale);

    // Locale information is stored relative to the executable,
    m_pLocale->Init();
    m_pLocale->AddCatalogLookupPathPrefix(wxT("locale"));
    m_pLocale->AddCatalog(GetAppName());

    // Commandline parsing is done in wxApp::OnInit()
    if (!wxApp::OnInit())
        return false;

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

    // Startup the System Idle Detection code
    StartupSystemIdleDetection();

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

    // Shutdown the System Idle Detection code
    ShutdownSystemIdleDetection();

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
        strExecute += wxT("\\boinc.exe -redirectio");

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
            m_lBOINCCoreProcessId = pi.dwProcessId;
            m_hBOINCCoreProcess = pi.hProcess;
        }

#else

        // Append boinc.exe to the end of the strExecute string and get ready to rock
        strExecute += wxT("/boinc");
        m_lBOINCCoreProcessId = ::wxExecute( strExecute );

#endif

        if ( 0 != m_lBOINCCoreProcessId )
            m_bBOINCStartedByManager = true;
    }
}


#ifdef __WXMSW__

void CBOINCGUIApp::ShutdownBOINCCore()
{
    wxInt32 iCount = 0;
    bool    bClientQuit = false;

    if ( m_bBOINCStartedByManager )
    {
        DWORD dwExitCode;
        if ( GetExitCodeProcess( m_hBOINCCoreProcess, &dwExitCode ) )
        {
            if ( STILL_ACTIVE == dwExitCode )
            {
                m_pDocument->CoreClientQuit();
                for ( iCount = 0; iCount <= 10; iCount++ )
                {
                    if ( !bClientQuit && GetExitCodeProcess( m_hBOINCCoreProcess, &dwExitCode ) )
                    {
                        if ( STILL_ACTIVE != dwExitCode )
                        {
                            bClientQuit = true;
                            continue;
                        }
                    }
                    ::wxSleep(1);
                }
            }
        }

        if ( !bClientQuit )
            ::wxKill( m_lBOINCCoreProcessId );
    }
}

#else

void CBOINCGUIApp::ShutdownBOINCCore()
{
    wxInt32 iCount = 0;
    bool    bClientQuit = false;

    if ( m_bBOINCStartedByManager )
    {
        if ( wxProcess::Exists( m_lBOINCCoreProcessId ) )
        {
            m_pDocument->CoreClientQuit();
            for ( iCount = 0; iCount <= 10; iCount++ )
            {
                if ( !bClientQuit && !wxProcess::Exists( m_lBOINCCoreProcessId ) )
                {
                    bClientQuit = true;
                    continue;
                }
                ::wxSleep(1);
            }
        }

        if ( !bClientQuit )
            ::wxKill( m_lBOINCCoreProcessId );
    }
}

#endif


wxInt32 CBOINCGUIApp::StartupSystemIdleDetection()
{
#ifdef __WXMSW__
    // load dll and start idle detection
    m_hIdleDetectionDll = LoadLibrary("boinc.dll");
    if(m_hIdleDetectionDll)
    {
        IdleTrackerInit fn;
        fn = (IdleTrackerInit)GetProcAddress(m_hIdleDetectionDll, wxT("IdleTrackerInit"));
        if(!fn)
        {
            FreeLibrary(m_hIdleDetectionDll);
            m_hIdleDetectionDll = NULL;
            return -1;
        }
        else 
        {
            if(!fn())
            {
                FreeLibrary(m_hIdleDetectionDll);
                m_hIdleDetectionDll = NULL;
                return -1;
            }
        }
    }
#endif
    return 0;
}


wxInt32 CBOINCGUIApp::ShutdownSystemIdleDetection()
{
#ifdef __WXMSW__
    if(m_hIdleDetectionDll) {
        IdleTrackerTerm fn;
        fn = (IdleTrackerTerm)GetProcAddress(m_hIdleDetectionDll, wxT("IdleTrackerTerm"));
        if(fn)
        {
            fn();
        }
        else
        {
            return -1;
        }
        FreeLibrary(m_hIdleDetectionDll);
        m_hIdleDetectionDll = NULL;
    }
#endif
    return 0;
}


wxInt32 CBOINCGUIApp::UpdateSystemIdleDetection()
{
#ifdef __WXMSW__
    if (m_hIdleDetectionDll)
    {
        IdleTrackerGetIdleTickCount fn;
        fn = (IdleTrackerGetIdleTickCount)GetProcAddress(m_hIdleDetectionDll, wxT("IdleTrackerGetIdleTickCount"));
        if(fn)
        {
            fn();
        }
        else
        {
            return -1;
        }
    }
#endif
    return 0;
}


const char *BOINC_RCSID_487cbf3018 = "$Id$";

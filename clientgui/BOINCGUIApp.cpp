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

    SetVendorName("Space Sciences Laboratory, U.C. Berkeley");
    SetAppName("BOINC Manager");

    // Commandline parsing is done in wxApp::OnInit()
    if (!Inherited::OnInit())
        return false;

    // Enable the in memory virtual file system for
    //   storing images
    wxFileSystem::AddHandler(new wxMemoryFSHandler);

    // Enable known image types
    wxImage::AddHandler(new wxXPMHandler);

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
    m_pConfig = new wxConfig(wxTheApp->GetAppName());
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

    // Show the UI
    SetTopWindow(m_pFrame);
    m_pFrame->Show();

    return true;
}


int CBOINCGUIApp::OnExit()
{
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

    return Inherited::OnExit();
}


void CBOINCGUIApp::OnInitCmdLine(wxCmdLineParser &parser)
{
    Inherited::OnInitCmdLine(parser);
    static const wxCmdLineEntryDesc cmdLineDesc[] = {
        { wxCMD_LINE_SWITCH, "e", "example", "example command line option"},
        { wxCMD_LINE_NONE}  //DON'T forget this line!!
    };
    parser.SetDesc(cmdLineDesc);
}


bool CBOINCGUIApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
    // Give default processing (-?, --help and --verbose) the chance to do something.
    Inherited::OnCmdLineParsed(parser);
    if (parser.Found("example")) {

        wxMessageDialog* pDlg = new wxMessageDialog(
            NULL, 
            _("You have specified -e on the commmand line. The application will exit now."),
            _("Example command line option"),
            wxOK | wxICON_INFORMATION,
            wxDefaultPosition
            );

        if (pDlg)
            pDlg->ShowModal();

        return false;
    } else {
        // Log a message, but only if --verbose has been set on the command line.
        wxLogVerbose("Commandline has been parsed. -e was not found.");
    }
    return true;
}


void CBOINCGUIApp::DetectDefaultWindowStation()
{
    wxChar szWindowStation[256];
    memset(szWindowStation, 0, sizeof(szWindowStation));

#ifdef __WXMSW__

    if ( wxWIN95 != wxGetOsVersion( NULL, NULL) )
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
    memset(szDesktop, 0, sizeof(szDesktop));

#ifdef __WXMSW__

    if ( wxWIN95 != wxGetOsVersion( NULL, NULL) )
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


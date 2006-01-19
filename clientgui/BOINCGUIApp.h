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


class CBrandingScheme : public wxObject {
private:
    bool        m_bIsBranded;
    wxString    m_strApplicationName;
    wxIcon      m_iconApplicationIcon;
    wxBitmap    m_bitmapApplicationLogo;
    wxString    m_strCompanyName;
    wxString    m_strCompanyWebsite;
    wxString    m_strProjectName;
    bool        m_bDefaultTabSpecified;
    int         m_iDefaultTab;
    wxString    m_strAPWizardTitle;
    wxBitmap    m_bitmapAPWizardLogo;
    wxString    m_strAPWizardAccountInfoText;
    wxString    m_strAPWizardCompletionTitle;
    wxString    m_strAPWizardCompletionBrandedMessage;
    wxString    m_strAPWizardCompletionMessage;
    wxString    m_strAMWizardTitle;
    wxBitmap    m_bitmapAMWizardLogo;
    wxString    m_strAMWizardAttachMessage;

public:
    bool        IsBranded() { return m_bIsBranded; }
    wxString    GetApplicationName() { return m_strApplicationName; }
    wxIcon*     GetApplicationIcon() { return &m_iconApplicationIcon; }
    wxBitmap*   GetApplicationLogo() { return &m_bitmapApplicationLogo; }
    wxString    GetCompanyName() { return m_strCompanyName; }
    wxString    GetCompanyWebsite() { return m_strCompanyWebsite; }
    wxString    GetProjectName() { return m_strProjectName; }
    bool        IsDefaultTabSpecified() { return m_bDefaultTabSpecified; }
    int         GetDefaultTab() { return m_iDefaultTab; }
    wxString    GetAPWizardTitle() { return m_strAPWizardTitle; }
    wxBitmap*   GetAPWizardLogo() { return &m_bitmapAPWizardLogo; }
    wxString    GetAPWizardAccountInfoText() { return m_strAPWizardAccountInfoText; }
    wxString    GetAPWizardCompletionTitle() { return m_strAPWizardCompletionTitle; }
    wxString    GetAPWizardCompletionBrandedMessage() { return m_strAPWizardCompletionBrandedMessage; }
    wxString    GetAPWizardCompletionMessage() { return m_strAPWizardCompletionMessage; }
    wxString    GetAMWizardTitle() { return m_strAMWizardTitle; }
    wxBitmap*   GetAMWizardLogo() { return &m_bitmapAMWizardLogo; }
    wxString    GetAMWizardSuccessMessage() { return m_strAMWizardAttachMessage; }

    bool        OnInit( wxConfigBase* pConfig );
};


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

    wxConfig*        m_pConfig;
    wxLocale*        m_pLocale;
    wxLogBOINC*      m_pLog;

    CBrandingScheme* m_pBranding;
    CMainFrame*      m_pFrame;
    CMainDocument*   m_pDocument;
#if defined(__WXMSW__) || defined(__WXMAC__)
    CTaskBarIcon*    m_pTaskBarIcon;
#endif
#ifdef __WXMAC__
    CMacSystemMenu*  m_pMacSystemMenu;
#endif

    bool             m_bBOINCStartedByManager;
    bool             m_bFrameVisible;

    int              m_lBOINCCoreProcessId;

#ifdef __WXMSW__
    HANDLE           m_hBOINCCoreProcess;
    HINSTANCE        m_hIdleDetectionDll;
#endif

    // The last value defined in the wxLanguage enum is wxLANGUAGE_USER_DEFINED.
    // defined in: wx/intl.h
    wxArrayString   m_astrLanguages;

public:

    std::string     m_strDefaultWindowStation;
    std::string     m_strDefaultDesktop;
    std::string     m_strDefaultDisplay;

    bool            OnInit();

    int             UpdateSystemIdleDetection();

    CBrandingScheme* GetBrand()                  { return m_pBranding; }
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


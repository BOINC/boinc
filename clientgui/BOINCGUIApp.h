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

    typedef wxApp Inherited;

protected:
    int             OnExit();

    void            OnInitCmdLine(wxCmdLineParser &parser);
    bool            OnCmdLineParsed(wxCmdLineParser &parser);

    void            DetectDefaultWindowStation();
    void            DetectDefaultDesktop();

    wxLocale*       m_pLocale;
    wxConfig*       m_pConfig;

    CMainFrame*     m_pFrame;
    CMainDocument*  m_pDocument;
#ifndef NOTASKBAR
    CTaskBarIcon*   m_pTaskBarIcon;
#endif

    wxString        m_strDefaultWindowStation;
    wxString        m_strDefaultDesktop;

public:

    bool            OnInit();

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


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
// $Log$
// Revision 1.2  2004/07/12 08:46:26  rwalton
// Document parsing of the <get_state/> message
//
// Revision 1.1  2004/06/25 22:50:57  rwalton
// Client spamming server hotfix
//
//

#ifndef _WORKUNIT_H_
#define _WORKUNIT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "Workunit.cpp"
#endif

#include "XMLParser.h"
#include "Project.h"
#include "App.h"
#include "AppVersion.h"


class CWorkunit : public CXMLParser
{
    DECLARE_DYNAMIC_CLASS(CWorkunit)

private:
    wxString        name;
    wxString        app_name;
    wxInt32         version_num;
    double          rsc_fpops_est;
    double          rsc_fpops_bound;
    double          rsc_memory_bound;
    double          rsc_disk_bound;
    CProject*       project;
    CApp*           app;
    CAppVersion*    avp;

public:
    CWorkunit();
    ~CWorkunit();

    wxInt32         Parse(wxTextInputStream* input);

    wxString&       GetName()
                    { return name; }

    wxString&       GetApplicationName()
                    { return app_name; }

    wxInt32         GetApplicationVersion()
                    { return version_num; }

    double          GetFPOPSEstimated()
                    { return rsc_fpops_est; }

    double          GetFPOPSBound()
                    { return rsc_fpops_bound; }

    double          GetMemoryBound()
                    { return rsc_memory_bound; }

    double          GetDiskBound()
                    { return rsc_disk_bound; }

    CProject*       GetProject()
                    { return project; }

    CApp*           GetApp()
                    { return app; }

    CAppVersion*    GetAppVersion()
                    { return avp; }

    void            SetProject(CProject* pProject)
                    { project = pProject; }

    void            SetApp(CApp* pApp)
                    { app = pApp; }

    void            SetAppVersion(CAppVersion* pAppVersion)
                    { avp = pAppVersion; }

};


WX_DECLARE_OBJARRAY(CWorkunit, CArrayWorkunit);

#endif


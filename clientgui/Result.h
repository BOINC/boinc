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

#ifndef _RESULT_H_
#define _RESULT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "Result.cpp"
#endif

#include "XMLParser.h"
#include "App.h"
#include "Workunit.h"
#include "Project.h"


class CResult : public CXMLParser
{
    DECLARE_DYNAMIC_CLASS(CResult)

private:
    wxString        name;
    wxString        wu_name;
    wxInt32         report_deadline;
    bool            ready_to_report;
    bool            got_server_ack;
    double          final_cpu_time;
    wxInt32         state;
    wxInt32         exit_status;
    wxInt32         signal;
    wxInt32         active_task_state;
    wxString        stderr_out;
    CApp*           app;
    CWorkunit*      wup;
    CProject*       project;

public:
    CResult();
    ~CResult();

    wxInt32         Parse(wxTextInputStream* input);

    wxString&       GetName()
                    { return name; }

    wxString&       GetWorkunitName()
                    { return wu_name; }

    wxInt32         GetReportDeadline()
                    { return report_deadline; }

    bool            IsReadyToReport()
                    { return ready_to_report; }

    bool            IsAcknowledged()
                    { return got_server_ack; }

    double          GetFinalCPUTime()
                    { return final_cpu_time; }

    wxInt32         GetState()
                    { return state; }

    wxInt32         GetExitStatus()
                    { return exit_status; }

    wxInt32         GetSignal()
                    { return signal; }

    wxInt32         GetActiveTaskState()
                    { return active_task_state; }

    wxString&       GetSTDERROut()
                    { return stderr_out; }

    CApp*           GetApp()
                    { return app; }

    CWorkunit*      GetWorkunit()
                    { return wup; }

    CProject*       GetProject()
                    { return project; }

    void            SetApp(CApp* pApp)
                    { app = pApp; }

    void            SetWorkunit(CWorkunit* pWorkunit)
                    { wup = pWorkunit; }

    void            SetProject(CProject* pProject)
                    { project = pProject; }

};


WX_DECLARE_OBJARRAY(CResult, CArrayResult);

#endif


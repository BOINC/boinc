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
// Revision 1.1  2004/06/25 22:50:57  rwalton
// Client spamming server hotfix
//
//
//

#ifndef _PROJECT_H_
#define _PROJECT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "Project.cpp"
#endif

#include "XMLParser.h"


class CProject : public CXMLParser
{
    DECLARE_DYNAMIC_CLASS(CProject)

private:
    wxString        master_url;
    double          resource_share;
    wxString        project_name;
    wxString        user_name;
    wxString        team_name;
    double          user_total_credit;
    double          user_expavg_credit;
    double          host_total_credit;
    double          host_expavg_credit;
    wxInt32         nrpc_failures;
    wxInt32         master_fetch_failures;
    wxInt32         min_rpc_time;
    bool            master_url_fetch_pending;
    bool            sched_rpc_pending;
    bool            tentative;

public:
    CProject();
    ~CProject();

    wxInt32         Parse(wxTextInputStream* input);

    wxString        GetMasterURL()                  { return master_url; }
    double          GetResourceShare()              { return resource_share; }
    wxString        GetProjectName()                { return project_name; }
    wxString        GetUserName()                   { return user_name; }
    wxString        GetTeamName()                   { return team_name; }
    double          GetUserTotalCredit()            { return user_total_credit; }
    double          GetUserExpAvgCredit()           { return user_expavg_credit; }
    double          GetHostTotalCredit()            { return host_total_credit; }
    double          GetHostExpAvgCredit()           { return host_expavg_credit; }
    wxInt32         GetRPCFailureCount()            { return nrpc_failures; }
    wxInt32         GetMasterFetchFailureCount()    { return master_fetch_failures; }
    wxInt32         GetNextRPCTime()                { return min_rpc_time; }
    bool            IsMasterURLFetchPending()       { return master_url_fetch_pending; }
    bool            IsSchedRPCPending()             { return sched_rpc_pending; }
    bool            IsTentative()                   { return tentative; }

};


#endif


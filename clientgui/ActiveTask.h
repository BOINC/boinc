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
// Revision 1.1  2004/06/25 22:50:56  rwalton
// Client spamming server hotfix
//
//
//

#ifndef _ACTIVETASK_H_
#define _ACTIVETASK_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ActiveTask.cpp"
#endif

#include "XMLParser.h"
#include "Project.h"
#include "Result.h"


class CActiveTask : public CXMLParser
{
    DECLARE_DYNAMIC_CLASS(CActiveTask)

private:
    wxString        result_name;
    wxInt32         app_version_num;
    double          checkpoint_cpu_time;
    double          current_cpu_time;
    double          fraction_done;
    CProject*       project;
    CResult*        result;

public:
    CActiveTask();
    ~CActiveTask();

    wxInt32         Parse(wxTextInputStream* input);

    wxString        GetName()                       { return result_name; }
    wxInt32         GetAppVersionNumber()           { return app_version_num; }
    double          GetCheckpointCPUTime()          { return checkpoint_cpu_time; }
    double          GetCurrentCPUTime()             { return current_cpu_time; }
    double          GetFractionDone()               { return fraction_done; }
    CProject*       GetProject()                    { return project; }
    CResult*        GetResult()                     { return result; }

};


#endif


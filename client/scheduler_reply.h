// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include "types.h"
#include "prefs.h"

struct SCHEDULER_REPLY {
    int hostid;
    int request_delay;
    char message[1024];
    char message_priority[256];
    PREFS prefs;
    vector<APP> apps;
    vector<FILE_INFO> file_infos;
    vector<APP_VERSION> app_versions;
    vector<WORKUNIT> workunits;
    vector<RESULT> results;
    vector<RESULT> result_acks;

    int parse(FILE*);
};

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

#ifndef _SERVER_TYPES_
#define _SERVER_TYPES_

#include <stdio.h>
#include <vector>

#include "db.h"

//The following are the states that the client is in according to the result. 
#define CLIENT_UNINITIALIZED    0
#define CLIENT_DOWNLOADING      1
#define CLIENT_COMPUTING        2
#define CLIENT_UPLOADING        3
#define CLIENT_DONE             4

struct SCHEDULER_REQUEST {
    char authenticator[256];
    char platform_name[256];
    int hostid;                 // zero if first RPC
    int core_client_version;
    int rpc_seqno;
    int work_req_seconds;
    unsigned int prefs_mod_time;
    char* prefs_xml;

    HOST host;
    vector<RESULT> results;

    SCHEDULER_REQUEST();
    ~SCHEDULER_REQUEST();
    int parse(FILE*);
};

struct SCHEDULER_REPLY {
    int request_delay;          // don't request again until this time elapses
    char message[1024];
    char message_priority[256];
    int hostid;                 // send this only if nonzero.
                                // this tells client to reset rpc_seqno
    bool send_prefs;            // whether to send preferences
    USER user;
    vector<APP> apps;
    vector<APP_VERSION> app_versions;
    vector<WORKUNIT>wus;
    vector<RESULT>results;
    vector<RESULT>result_acks;

    SCHEDULER_REPLY();
    ~SCHEDULER_REPLY();
    int write(FILE*);
    void insert_app_unique(APP&);
    void insert_app_version_unique(APP_VERSION&);
    void insert_workunit_unique(WORKUNIT&);
    void insert_result(RESULT&);
};

#endif

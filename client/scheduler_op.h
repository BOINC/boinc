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

#ifndef _SCHEDULER_OP_
#define _SCHEDULER_OP_

// Logic for communicating with a scheduling server.
// If we don't yet have the addresses a scheduling server,
// we have to get the file from the project's master URL
// and parse if for <scheduler> elements

// TODO: try alternate scheduling servers;
// implement backoff and give-up policies

#include "client_types.h"
#include "http.h"
#include "prefs.h"

#define SCHEDULER_OP_STATE_IDLE         0
#define SCHEDULER_OP_STATE_GET_MASTER   1
#define SCHEDULER_OP_STATE_RPC          2
#define SCHEDULER_OP_STATE_DONE         3

struct SCHEDULER_OP {
    int state;
    int scheduler_op_retval;
    HTTP_OP http_op;
    HTTP_OP_SET* http_ops;
    PROJECT* project;
    char scheduler_url[256];

    SCHEDULER_OP(HTTP_OP_SET*);
    int poll();
    int start_op(PROJECT*);
    int start_rpc();
    int parse_master_file();
};

struct SCHEDULER_REPLY {
    int hostid;
    int request_delay;
    char message[1024];
    char message_priority[256];
    int prefs_mod_time;
    char* prefs_xml;
    vector<APP> apps;
    vector<FILE_INFO> file_infos;
    vector<APP_VERSION> app_versions;
    vector<WORKUNIT> workunits;
    vector<RESULT> results;
    vector<RESULT> result_acks;
    char* code_sign_key;
    char* code_sign_key_signature;

    SCHEDULER_REPLY();
    ~SCHEDULER_REPLY();
    int parse(FILE*);
};

#endif

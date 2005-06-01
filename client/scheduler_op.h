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

#ifndef _SCHEDULER_OP_
#define _SCHEDULER_OP_

// SCHEDULER_OP encapsulates the policy and mechanism
// for communicating with scheduling servers.
// It is implemented as a finite-state machine.
// It is active in one of two modes:
//    get_work: the client wants to get work, and possibly to
//       return results as a side-effect
//    return_results: the client wants to return results, and possibly
//       to get work as a side-effect
//

#include <vector>

#include "client_types.h"
#include "http.h"
#include "prefs.h"

// default constants related to scheduler RPC policy

#define MASTER_FETCH_PERIOD     10
    // fetch and parse master URL if nrpc_failures is a multiple of this
#define RETRY_BASE_PERIOD       1
    // after failure, back off 2^nrpc_failures times this times random
#define RETRY_CAP               10
    // cap on nrpc_failures in the above formula
#define MASTER_FETCH_RETRY_CAP 3
    // cap on how many times we will contact master_url
    // before moving into a state in which we will not
    // exponentially backoff anymore but rather contact the master URL
    // at the frequency below
#define MASTER_FETCH_INTERVAL (60*60*24*7*2)    // 2 weeks
    // This is the Max on the time to wait after we've contacted the Master URL MASTER_FETCH_RETRY_CAP times.

//The next two constants are used to bound RPC exponential waiting.
#define SCHED_RETRY_DELAY_MIN    60                // 1 minute
#define SCHED_RETRY_DELAY_MAX    (60*60*4)         // 4 hours

#define SCHEDULER_OP_STATE_IDLE         0
    // invariant: in this state, our HTTP_OP is not in the HTTP_OP_SET
#define SCHEDULER_OP_STATE_GET_MASTER   1
#define SCHEDULER_OP_STATE_RPC          2

struct SCHEDULER_OP {
    int state;
    int scheduler_op_retval;
    HTTP_OP http_op;
    HTTP_OP_SET* http_ops;
    PROJECT* project;               // project we're currently contacting
    char scheduler_url[256];
    bool must_get_work;             // true iff in get_work mode
    int url_index;                  // index within project's URL list
    double url_random;              // used to randomize order

    SCHEDULER_OP(HTTP_OP_SET*);
    bool poll();
    int init_get_work(int urgency);
    int init_return_results(PROJECT*);
    int init_op_project();
    int init_master_fetch();
    int set_min_rpc_time(PROJECT*);
    bool update_urls(std::vector<std::string> &urls);
    int start_op(PROJECT*);
    bool check_master_fetch_start();
    void backoff(PROJECT* p, const char *error_msg);
    int start_rpc();
    int parse_master_file(std::vector<std::string>&);
};

struct USER_MESSAGE {
    std::string message;
    std::string priority;
    USER_MESSAGE(char*, char*);
};

struct SCHEDULER_REPLY {
    int hostid;
    double request_delay;
    std::vector<USER_MESSAGE> messages;
    char* global_prefs_xml;
        // not including <global_preferences> tags;
        // may include <venue> elements
    char* project_prefs_xml;
        // not including <project_preferences> tags
        // may include <venue> elements
    char host_venue[256];
    unsigned int user_create_time;
    std::vector<APP> apps;
    std::vector<FILE_INFO> file_infos;
    std::vector<std::string> file_deletes;
    std::vector<APP_VERSION> app_versions;
    std::vector<WORKUNIT> workunits;
    std::vector<RESULT> results;
    std::vector<RESULT> result_acks;
    char* code_sign_key;
    char* code_sign_key_signature;
    bool message_ack;
    bool project_is_down;
    bool send_file_list;      

    SCHEDULER_REPLY();
    ~SCHEDULER_REPLY();
    int parse(FILE*, PROJECT*);
};

#endif

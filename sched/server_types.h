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

#include <cstdio>
#include <vector>

#include "boinc_db.h"
#include "result_state.h"
#include "md5_file.h"

// summary of a client's request for work
//
struct WORK_REQ {
    bool infeasible_only;
    double seconds_to_fill;
    double disk_available;
    int nresults;
    int core_client_version;

    // the following flags are set whenever a result is infeasible;
    // used to construct explanatory message to user
    //
    bool insufficient_disk;
    bool insufficient_mem;
    bool insufficient_speed;
    bool no_app_version;
    bool homogeneous_redundancy_reject;
    bool outdated_core;
    bool daily_result_quota_exceeded;

    bool work_needed(struct SCHEDULER_REPLY&);
    void update_for_result(double seconds_filled);
};

// a description of a sticky file on host.
//
struct FILE_INFO {
    char name[256];

    int parse(FILE*);
};

struct MSG_FROM_HOST_DESC {
    char variety[256];
    std::string msg_text;
    int parse(FILE*);
};

// an app version from an anonymous-platform client
//
struct CLIENT_APP_VERSION {
    char app_name[256];
    int version_num;

    int parse(FILE*);
};

// subset of global prefs used by scheduler
//
struct GLOBAL_PREFS {
    double disk_max_used_gb;
    double disk_max_used_pct;
    double disk_min_free_gb;

    void parse(char* buf, char* venue);
};

struct GUI_URLS {
    char* text;
    void init();
    void get_gui_urls(USER& user, HOST& host, TEAM& team, char*);
};

struct SCHEDULER_REQUEST {
    char authenticator[256];
    char platform_name[256];
    char cross_project_id[256];
    int hostid;                 // zero if first RPC
    char host_venue[256];
    int core_client_major_version;
    int core_client_minor_version;
    int rpc_seqno;
    int work_req_seconds;
    double resource_share_fraction;
    double estimated_delay;
    char global_prefs_xml[LARGE_BLOB_SIZE];
    char projects_xml[LARGE_BLOB_SIZE];
    char code_sign_key[4096];

// ROMW: Added these back in since we have 3.x clients who still want
//       want to send us the older style for determining disk usage.
// TODO: Remove the two lines below when the 4.x way of doing things
//       is completely implemented.
    double total_disk_usage;
    double project_disk_usage;

#if 0
    double project_disk_free;
    double potentially_free_offender;
    double potentially_free_self;
#endif

    bool anonymous_platform;
    std::vector<CLIENT_APP_VERSION> client_app_versions;
    GLOBAL_PREFS global_prefs;
    char global_prefs_source_email_hash[MD5_LEN];

    HOST host;      // request message is parsed into here.
                    // does NOT contain the full host record.
    std::vector<RESULT> results;
    std::vector<MSG_FROM_HOST_DESC> msgs_from_host;
    std::vector<FILE_INFO> file_infos;   // sticky files reported by host

    SCHEDULER_REQUEST();
    ~SCHEDULER_REQUEST();
    int parse(FILE*);
    bool has_version(APP& app);
};

// NOTE: if any field requires initialization,
// you must do it in the constructor.  Nothing is zeroed by default.
//
struct SCHEDULER_REPLY {
    int request_delay;          // don't request again until this time elapses
    char message[1024];
    char message_priority[256];
    int hostid;                 // send this only if nonzero.
                                // this tells client to reset rpc_seqno
    bool send_global_prefs;     // whether to send global preferences
    bool nucleus_only;          // send only message
    bool probable_user_browser;
    USER user;
    char email_hash[MD5_LEN];
    HOST host;                  // after validation, contains full host rec
    TEAM team;
    std::vector<APP> apps;
    std::vector<APP_VERSION> app_versions;
    std::vector<WORKUNIT>wus;
    std::vector<RESULT>results;
    std::vector<RESULT>result_acks;
    std::vector<MSG_TO_HOST>msgs_to_host;
    std::vector<FILE_INFO>file_deletes;
    char code_sign_key[4096];
    char code_sign_key_signature[4096];
    bool send_msg_ack;
    bool update_user_record;
    bool deletion_policy_priority;
    bool deletion_policy_expire;

    SCHEDULER_REPLY();
    ~SCHEDULER_REPLY();
    int write(FILE*);
    void insert_app_unique(APP&);
    void insert_app_version_unique(APP_VERSION&);
    void insert_workunit_unique(WORKUNIT&);
    void insert_result(RESULT&);
};

#endif

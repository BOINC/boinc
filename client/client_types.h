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

// If you change anything, make sure you also change:
// types.C         (to write and parse it)
// client_state.C  (to cross-link objects)
//

#ifndef _CLIENT_TYPES_
#define _CLIENT_TYPES_

#ifdef _WIN32
#include "windows_cpp.h"
#endif

#include <vector>
#include <stdio.h>

#include "hostinfo.h"

#define MAX_BLOB_LEN 4096
#define DEFAULT_MAX_PROCESSING  1e10
#define DEFAULT_MAX_DISK        1e10

class PERS_FILE_XFER;
struct RESULT;

struct STRING256 {
    char text[256];
};

class PROJECT {
public:
    // the following items come from prefs.xml
    // They are a function only of the user and the project
    //
    char master_url[256];       // url of site that contains scheduler tags
                                // for this project
    char authenticator[256];    // user's authenticator on this project
    char project_specific_prefs[MAX_BLOB_LEN];   // without enclosing tags
    double resource_share;      // project's resource share
                                // relative to other projects.  Arbitrary scale.

    // the following items come from client_state.xml
    // They may depend on the host as well as user and project
    //
    vector<STRING256> scheduler_urls;       // where to find scheduling servers
    char project_name[256];             // descriptive.  not unique
    char user_name[256];
    double user_total_credit;    // as reported by server
    double user_expavg_credit;    // as reported by server
    unsigned int user_create_time;   // as reported by server
    int rpc_seqno;
    int hostid;
    double host_total_credit;      // as reported by server
    double host_expavg_credit;     // as reported by server
    unsigned int host_create_time; // as reported by server 
    double exp_avg_cpu;         // exponentially weighted CPU time
    int exp_avg_mod_time;       // last time average was changed
    char code_sign_key[MAX_BLOB_LEN];
    int nrpc_failures;          // # of consecutive times we've failed to
                                // contact all scheduling servers
    int min_rpc_time;           // earliest time to contact any server
                                // of this project (or zero)
    int master_fetch_failures;
    // the following items are transient; not saved in state file
    double resource_debt;       // How much CPU time we owe this project
                                // (arbitrary scale)
    int debt_order;             // 0 == largest debt
    bool master_url_fetch_pending;
                                // need to fetch and parse the master URL

    PROJECT();
    ~PROJECT();
    void copy_state_fields(PROJECT&);
    int parse_account(FILE*);
    int parse_state(FILE*);
    int write_state(FILE*);
};

struct APP {
    char name[256];
    PROJECT* project;

    int parse(FILE*);
    int write(FILE*);
};

// If the status is neither of these two, it will be
// an error code defined in err_numbers.h
#define FILE_NOT_PRESENT    0
#define FILE_PRESENT        1

class FILE_INFO {
public:
    char name[256];
    char md5_cksum[33];
    double max_nbytes;
    double nbytes;
    double upload_offset;
    bool generated_locally; // file is produced by app
    int status;
    bool executable;        // change file protections to make executable
    bool uploaded;          // file has been uploaded
    bool upload_when_present;
    bool sticky;            // don't delete unless instructed to do so
    bool signature_required;    // true iff associated with app version
    PERS_FILE_XFER* pers_file_xfer;   // nonzero if in the process of being up/downloaded
    RESULT* result;         // for upload files (to authenticate)
    PROJECT* project;
    int ref_cnt;
    vector<STRING256> urls;
    int start_url;
    int current_url;
    char signed_xml[MAX_BLOB_LEN];
    char xml_signature[MAX_BLOB_LEN];
    char file_signature[MAX_BLOB_LEN];

    FILE_INFO();
    ~FILE_INFO();
    int set_permissions();
    int parse(FILE*, bool from_server);
    int write(FILE*, bool to_server);
    int delete_file();      // attempt to delete the underlying file
    char* get_url();
    bool had_failure(int& failnum);
};

// Describes a connection between a file and a workunit, result, or application.
// In the first two cases,
// the app will either use open() or fopen() to access the file
// (in which case "open_name" is the name it will use)
// or the app will be connected by the given fd (in which case fd is nonzero)
//
struct FILE_REF {
    char file_name[256];
    char open_name[256];
    int fd;
    bool main_program;
    FILE_INFO* file_info;

    int parse(FILE*);
    int write(FILE*);
};

struct APP_VERSION {
    char app_name[256];
    int version_num;
    APP* app;
    PROJECT* project;
    vector<FILE_REF> app_files;

    int parse(FILE*);
    int write(FILE*);
};

struct WORKUNIT {
    char name[256];
    char app_name[256];
    int version_num;
        // This isn't sent from the server.
        // Instead, the client picks the latest app version
    char command_line[256];
    char env_vars[256];         // environment vars in URL format
    vector<FILE_REF> input_files;
    PROJECT* project;
    APP* app;
    APP_VERSION* avp;
    int ref_cnt;
    double seconds_to_complete;
        // estimated CPU time, based on server-supplied resource estimates
        // together with host info.
    double max_processing;  // abort if use this many cobblestones
    double max_disk;        // abort if use this much disk

    int parse(FILE*);
    int write(FILE*);
    bool had_failure(int& failnum);
};

#define RESULT_NEW              0
    // New result, files may still need to be downloaded
#define RESULT_FILES_DOWNLOADED 1
    // Files are downloaded, result can be computed
#define RESULT_COMPUTE_DONE     2
    // Computation is done, if no error then files need to be uploaded
#define RESULT_FILES_UPLOADED   3
    // Files are uploaded, notify scheduling server

struct RESULT {
    char name[256];
    char wu_name[256];
    int report_deadline;
    vector<FILE_REF> output_files;
    bool is_active;         // an app is currently running for this
    bool ready_to_ack;      // all the files have been uploaded or there
                            // was an error and we are ready to report this to 
                            // the server
    bool server_ack;        // received the ack for the report of 
                            // the status of the result from server
    double final_cpu_time;
    int state;              // state of this result, see above
    int exit_status;        // return value from the application
    int signal;             // the signal caught by the active_task,
                // defined only if active_task_state is PROCESS_SIGNALED
    int active_task_state; // the state of the active task corresponding to this result
    char stderr_out[MAX_BLOB_LEN];
    
    APP* app;
    WORKUNIT* wup;
    PROJECT* project;

    void clear();
    int parse_server(FILE*);
    int parse_state(FILE*);
    int parse_ack(FILE*);
    int write(FILE*, bool to_server);
    bool is_upload_done();    // files uploaded?
};

int verify_downloaded_file(char* pathname, FILE_INFO& file_info);

#endif

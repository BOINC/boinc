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

#include "windows_cpp.h"

#ifndef _CLIENT_TYPES_
#define _CLIENT_TYPES_

#include <vector>
#include <stdio.h>

#include "hostinfo.h"

#define STDERR_MAX_LEN 4096

class FILE_XFER;
class RESULT;

struct STRING256 {
    char text[256];
};

class PROJECT {
public:
    // the following items come from prefs.xml
    // They are a function only of the user and the project
    //
    char master_url[256];
    char authenticator[256];    // user's authenticator on this project
    char* project_specific_prefs;
    double resource_share;      // project's resource share
                                // relative to other projects.  Arbitrary scale.

    // the following items come from client_state.xml
    // They may depend on the host as well as user and project
    //
    vector<STRING256> scheduler_urls;       // where to find scheduling servers
    char project_name[256];             // descriptive.  not unique
    char user_name[256];
    int rpc_seqno;
    int hostid;
    int next_request_time;      // don't contact server until this time
    double exp_avg_cpu;         // exponentially weighted cpu time
    int exp_avg_mod_time;       // last time average was changed
    char* code_sign_key;

    PROJECT();
    ~PROJECT();
    void copy_state_fields(PROJECT&);
    void copy_prefs_fields(PROJECT&);
    int parse_prefs(FILE*);
    int parse_state(FILE*);
    int write_state(FILE*);
};

struct APP {
    char name[256];
    int version_num;
        // use this version number for new results
    PROJECT* project;

    int parse(FILE*);
    int write(FILE*);
};

class FILE_INFO {
public:
    char name[256];
    char md5_cksum[33];
    double max_nbytes;
    double nbytes;
    bool generated_locally; // file is produced by app
    bool file_present;
    bool executable;        // change file protections to make executable
    bool uploaded;          // file has been uploaded
    bool upload_when_present;
    bool sticky;            // don't delete unless instructed to do so
    bool signature_required;    // true iff associated with app version
    FILE_XFER* file_xfer;   // nonzero if in the process of being up/downloaded
    RESULT* result;         // for upload files (to authenticate)
    PROJECT* project;
    int ref_cnt;
    vector<STRING256> urls;
    char* signed_xml;
    char* xml_signature;
    char* file_signature;

    FILE_INFO();
    ~FILE_INFO();
    int parse(FILE*, bool from_server);
    int write(FILE*, bool to_server);
    int delete_file();      // attempt to delete the underlying file
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
        // Instead, the client picks an app version
        // (TODO: use alpha/beta/prod scheme)
    char command_line[256];
    char env_vars[256];         // environment vars in URL format
    vector<FILE_REF> input_files;
    PROJECT* project;
    APP* app;
    APP_VERSION* avp;
    int ref_cnt;
    double seconds_to_complete; //needs to be initialized

    int parse(FILE*);
    int write(FILE*);
};

struct RESULT {
    char name[256];
    char wu_name[256];
    vector<FILE_REF> output_files;
    bool is_active;         // an app is currently running for this
    bool is_compute_done;   // computation finished
    bool is_server_ack;     // ack received from scheduling server
    double cpu_time;
    int exit_status;
    char stderr_out[STDERR_MAX_LEN];
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

#endif

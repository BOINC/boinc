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

#ifndef _BOINC_DB_
#define _BOINC_DB_

// Structures corresponding to database records.
// Some of these types have counterparts in client/types.h,
// but don't be deceived - client and server have different variants.

// The parse and write functions are for use in scheduler RPC.
// They don't necessarily serialize the entire records.

#include <cstdio>
#include <vector>

#include "db_base.h"

extern DB_CONN boinc_db;

// Sizes of text buffers in memory, corresponding to database BLOBs.
// Large is for fields with user-supplied text, and preferences

//#define MEDIUM_BLOB_SIZE   4096
#define LARGE_BLOB_SIZE   65536

// Dummy name for file xfers
#define FILE_MOVER "move_file"


// A compilation target, i.e. a architecture/OS combination.
// The core client will be given only applications with the same platform
//
struct PLATFORM {
    int id;
    int create_time;
    char name[256];                 // i.e. "sparc-sun-solaris"
    char user_friendly_name[256];   // i.e. "SPARC Solaris 2.8"
    int deprecated;
    void clear();
};

// A version of the core client
//
struct CORE_VERSION {
    int id;
    int create_time;
    int version_num;
    int platformid;
    char xml_doc[LARGE_BLOB_SIZE];      // a <file_info> for the download file
    char message[256];      // if we get a request from this version,
                            // send this message
    bool deprecated;        // if we get a request from this version,
                            // don't send it any work.

    void clear();
};

// An application.
//
struct APP {
    int id;
    int create_time;
    char name[256];         // application name, preferably short
    int min_version;        // don't use app versions before this
    bool deprecated;
    char user_friendly_name[256];
    bool homogeneous_redundancy;

    int write(FILE*);
    void clear();
};

// A version of an application.
//
struct APP_VERSION {
    int id;
    int create_time;
    int appid;
    int version_num;
    int platformid;
    char xml_doc[LARGE_BLOB_SIZE];
    // describes app files. format:
    // <file_info>...</file_info>
    // ...
    // <app_version>
    //     <app_name>...</app_name>
    //     <version_num>x</version_num>
    //     <file_ref>
    //        ...
    //        [<main_program/>]
    //        [<copy_file/>]
    //     </file_ref>
    // </app_version>
    //

    // the following let you handle backwards-incompatible changes to
    // the core client / app interface
    //
    int min_core_version;   // min core version this app will run with
    int max_core_version;   // if <>0, max core version this will run with
    bool deprecated;

    int write(FILE*, APP&);
    void clear();
};

struct USER {
    int id;
    int create_time;
    char email_addr[256];
    char name[256];
    char authenticator[256];
    char country[256];
    char postal_code[256];
    double total_credit;
    double expavg_credit;           // credit per second, recent average
    double expavg_time;             // when the above was computed
    char global_prefs[LARGE_BLOB_SIZE];
        // global preferences, within <global_preferences> tag
    char project_prefs[LARGE_BLOB_SIZE];
        // project preferences; format:
        // <project_preferences>
        //    <resource_share>X</resource_share>
        //    <project_specific>
        //        ...
        //    </project_specific>
        //    <venue name="x">
        //       <resource_share>x</resource_share>
        //       <project_specific>
        //           ...
        //       </project_specific>
        //    </venue>
        //    ...
        // </project_preferences>
    int teamid;                     // team ID if any
    char venue[256];                // home/work/school (default)
    char url[256];                  // user's web page if any
    bool send_email;
    bool show_hosts;
    int posts;                      // number of messages posted (redundant)

    // The following are specific to SETI@home;
    // they record info about the user's involvement in a prior project
    int seti_id;                    // ID in old DB
    int seti_nresults;              // number of WUs completed
    int seti_last_result_time;      // time of last result (UNIX)
    double seti_total_cpu;          // number of CPU seconds
    char signature[256];
    bool has_profile;
    char cross_project_id[256];
    void clear();
};

#define TEAM_TYPE_CLUB                  1
#define TEAM_TYPE_COMPANY               2
#define TEAM_TYPE_PRIMARY               3
#define TEAM_TYPE_SECONDARY             4
#define TEAM_TYPE_JUNIOR_COLLEGE        5
#define TEAM_TYPE_UNIVERSITY            6
#define TEAM_TYPE_GOVERNMENT            7

// invariants of teams:
// a team has > 0 members

struct TEAM {
    int id;
    int create_time;
    int userid;             // User ID of team founder
    char name[256];
    char name_lc[256];      // Team name in lowercase (used for searching)
    char url[256];
    int type;               // Team type (see above)
    char name_html[256];
    char description[LARGE_BLOB_SIZE];
    int nusers;             // UNDEFINED BY DEFAULT
    char country[256];
    double total_credit;
    double expavg_credit;
    double expavg_time;

    // The following is specific to SETI@home
    int seti_id;            // ID in old DB
    void clear();
};

struct HOST {
    int id;
    int create_time;
    int userid;             // ID of user running this host
    int rpc_seqno;          // last seqno received from client
    int rpc_time;           // time of last scheduler RPC
    double total_credit;
    double expavg_credit;   // credit per second, recent average
    double expavg_time;     // last time the above was updated

    // all remaining items are assigned by the client
    int timezone;           // hours difference from GMT
    char domain_name[256];
    char serialnum[256];
    char last_ip_addr[256]; // IP address as of last RPC
    int nsame_ip_addr;      // # of RPCs with same IP address

    double on_frac;         // Fraction of time (0-1) that BOINC is running
    double connected_frac;  // Fraction of time that host is connected to net
    double active_frac;     // Fraction of time that host is enabled to work

    int p_ncpus;            // Number of CPUs on host
    char p_vendor[256];     // Vendor name of CPU
    char p_model[256];      // Model of CPU
    double p_fpops;         // measured floating point ops/sec of CPU
    double p_iops;          // measured integer ops/sec of CPU
    double p_membw;         // measured memory bandwidth (bytes/sec) of CPU
                            // The above are per CPU, not total
    double p_calculated;    // when the above were calculated

    char os_name[256];      // Name of operating system
    char os_version[256];   // Version of operating system

    double m_nbytes;        // Size of memory in bytes
    double m_cache;         // Size of CPU cache in bytes (L1 or L2?)
    double m_swap;          // Size of swap space in bytes

    double d_total;         // Total disk space on host
                            // - may include all volumes,
                            // even if BOINC can use only one of them
                            // - may include network (shared) storage
    double d_free;          // Of the total disk space, how much is free
    double d_boinc_used_total;
                            // amount being used for all projects
    double d_boinc_used_project;
                            // amount being used for this project
    double d_boinc_max;     // max amount that BOINC is allowed to use
                            // This reflects both user preferences
                            // and the fact that BOINC can use only 1 volume
    double n_bwup;          // Average upload bandwidth, bytes/sec
    double n_bwdown;        // Average download bandwidth, bytes/sec
                            // The above are derived from actual
                            // file upload/download times, and may reflect
                            // factors other than network bandwidth

    // The following is derived (by server) from other fields
    double credit_per_cpu_sec;

    char venue[256];        // home/work/school
    int nresults_today;     // results sent since midnight
    double avg_turnaround;  // recent average result turnaround time
    char host_cpid[256];    // host cross-project ID

    int parse(FILE*);
    int parse_time_stats(FILE*);
    int parse_net_stats(FILE*);
    void fix_nans();
    void clear();
};

// values for file_delete state
#define FILE_DELETE_INIT        0
#define FILE_DELETE_READY       1
    // set to this value only when we believe all files are uploaded
#define FILE_DELETE_DONE        2
    // means the file uploader ATTEMPTED to delete files.
    // May have failed.  TODO: retry delete later

// values for assimilate_state
#define ASSIMILATE_INIT         0
#define ASSIMILATE_READY        1
#define ASSIMILATE_DONE         2

// NOTE: there is no overall state for a WU (like done/not done)
// There's just a bunch of independent substates
// (file delete, assimilate, and states of results, error flags)

// bit fields of error_mask
#define WU_ERROR_COULDNT_SEND_RESULT            1
#define WU_ERROR_TOO_MANY_ERROR_RESULTS         2
#define WU_ERROR_TOO_MANY_SUCCESS_RESULTS       4
#define WU_ERROR_TOO_MANY_TOTAL_RESULTS         8
#define WU_ERROR_CANCELLED                      16

struct WORKUNIT {
    int id;
    int create_time;
    int appid;                  // associated app
    char name[256];
    char xml_doc[LARGE_BLOB_SIZE];
    int batch;
    double rsc_fpops_est;       // estimated # of FP operations
        // used to estimate how long a result will take on a host
    double rsc_fpops_bound;     // upper bound on # of FP ops
        // used to calculate an upper bound on the CPU time for a result
        // before it is aborted.
    double rsc_memory_bound;    // upper bound on RAM working set (bytes)
        // currently used only by scheduler to screen hosts
        // At some point, could use as runtime limit
    double rsc_disk_bound;      // upper bound on amount of disk needed (bytes)
        // (including input, output and temp files, but NOT the app)
        // used for 2 purposes:
        // 1) for scheduling (don't send this WU to a host w/ insuff. disk)
        // 2) abort task if it uses more than this disk
    bool need_validate;         // this WU has at least 1 result in
                                // validate state = NEED_CHECK
    int canonical_resultid;     // ID of canonical result, or zero
    double canonical_credit;    // credit that all correct results get
    int transition_time;        // when should transition_handler
                                // next check this WU?
                                // MAXINT if no need to check
    int delay_bound;            // determines result deadline,
                                // timeout check time
    int error_mask;             // bitmask of errors (see above)
    int file_delete_state;
    int assimilate_state;
    int hr_class;               // homogeneous redundancy class
        // used to send redundant copies only to "similar" hosts
        // (in terms of numerics, performance, or both)
    double opaque;              // project-specific; usually external ID
    int min_quorum;             // minimum quorum size
    int target_nresults;        // try to get this many successful results
                                // may be > min_quorum to get consensus
                                // quicker or reflect loss rate
    int max_error_results;      // WU error if < #error results
    int max_total_results;      // WU error if < #total results
        // (need this in case results are never returned)
    int max_success_results;    // WU error if < #success results
        // without consensus (i.e. WU is nondeterministic)
    char result_template_file[64];
    int priority;
    char mod_time[16];

    // the following not used in the DB
    char app_name[256];
    void clear();
};

// WARNING: be Very careful about changing any values,
// especially for a project already running -
// the database will become inconsistent

#define RESULT_SERVER_STATE_INACTIVE       1
#define RESULT_SERVER_STATE_UNSENT         2
#define RESULT_SERVER_STATE_UNSENT_SEQ     3
    // unsent, part of a work sequence
#define RESULT_SERVER_STATE_IN_PROGRESS    4
#define RESULT_SERVER_STATE_OVER           5
    // we received a reply, timed out, or decided not to send.
    // Note: we could get a reply even after timing out.

#define RESULT_OUTCOME_INIT             0
#define RESULT_OUTCOME_SUCCESS          1
#define RESULT_OUTCOME_COULDNT_SEND     2
#define RESULT_OUTCOME_CLIENT_ERROR     3
    // an error happened on the client
#define RESULT_OUTCOME_NO_REPLY         4
#define RESULT_OUTCOME_DIDNT_NEED       5
    // we created the result but didn't need to send it because we already
    // got a quorum
#define RESULT_OUTCOME_VALIDATE_ERROR   6
    // The outcome was initially SUCCESS, but the validator
    // had a permanent error reading a result file,
    // or the result file had a syntax error

#define VALIDATE_STATE_INIT         0
#define VALIDATE_STATE_VALID        1
#define VALIDATE_STATE_INVALID      2
#define VALIDATE_STATE_NO_CHECK     3
    // WU had error, so we'll never get around to validating its results
    // This lets us avoid showing the claimed credit as "pending"
#define VALIDATE_STATE_INCONCLUSIVE 4
    // the validator looked this result (as part of a check_set() call)
    // but didn't find a canonical result.

struct RESULT {
    int id;
    int create_time;
    int workunitid;
    int server_state;               // see above
    int outcome;                    // see above; defined if server state OVER
    int client_state;               // phase that client contacted us in.
                                    // if UPLOADED then outcome is success.
                                    // error details are in stderr_out.
                                    // The values for this field are defined
                                    // in lib/result_state.h
    int hostid;                     // host processing this result
    int userid;                     // user processing this result
    int report_deadline;            // deadline for receiving result
    int sent_time;                  // when result was sent to host
    int received_time;              // when result was received from host
    char name[256];
    double cpu_time;                // CPU time used to complete result
    char xml_doc_in[LARGE_BLOB_SIZE];     // descriptions of output files
    char xml_doc_out[LARGE_BLOB_SIZE];    // MD5s of output files
    char stderr_out[LARGE_BLOB_SIZE];     // stderr output, if any
    int batch;
    int file_delete_state;	    // see above; values for file_delete_state
    int validate_state;
    double claimed_credit;          // CPU time times host credit/sec
    double granted_credit;          // == canonical credit of WU
    double opaque;                  // project-specific; usually external ID
    int random;                     // determines send order
    int app_version_num;            // version# of app (not core client)
    int appid;                      // copy of WU's appid
    int exit_status;                // application exit status, if any
    int teamid;
    int priority;
    char mod_time[16];

    // the following not used in the DB
    char wu_name[256];
    int parse_from_client(FILE*);
    void clear();
};

struct MSG_FROM_HOST {
    int id;
    int create_time;
    int hostid;
    char variety[256];              // project-defined; what kind of msg
    bool handled;                   // message handler has processed this
    char xml[LARGE_BLOB_SIZE];
    void clear();
};

struct MSG_TO_HOST {
    int id;
    int create_time;
    int hostid;
    char variety[256];              // project-defined; what kind of msg
    bool handled;                   // scheduler has sent this
    char xml[LARGE_BLOB_SIZE];      // text to include in sched reply
    void clear();
};

struct TRANSITIONER_ITEM {
    int id;
    char name[256];
    int appid;
    int min_quorum;
    bool need_validate;
    int canonical_resultid;
    int transition_time;
    int delay_bound;
    int error_mask;
    int max_error_results;
    int max_total_results;
    int file_delete_state;
    int assimilate_state;
    int target_nresults;
    char result_template_file[64];
    int priority;
    int res_id;
    char res_name[256];
    int res_report_deadline;
    int res_server_state;
    int res_outcome;
    int res_validate_state;
    int res_file_delete_state;
    int res_sent_time;
    int res_hostid;

    void clear();
    void parse(MYSQL_ROW&);
};

struct VALIDATOR_ITEM {
    WORKUNIT wu;
    RESULT res;
 
    void clear();
    void parse(MYSQL_ROW&);
};

class DB_PLATFORM : public DB_BASE, public PLATFORM {
public:
    DB_PLATFORM(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_CORE_VERSION : public DB_BASE, public CORE_VERSION {
public:
    DB_CORE_VERSION(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_APP : public DB_BASE, public APP {
public:
    DB_APP(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_APP_VERSION : public DB_BASE, public APP_VERSION {
public:
    DB_APP_VERSION(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_USER : public DB_BASE, public USER {
public:
    DB_USER(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(USER& r) {USER::operator=(r);}
};

class DB_TEAM : public DB_BASE, public TEAM {
public:
    DB_TEAM(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_HOST : public DB_BASE, public HOST {
public:
    DB_HOST(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(HOST& r) {HOST::operator=(r);}
};

class DB_RESULT : public DB_BASE, public RESULT {
public:
    DB_RESULT(DB_CONN* p=0);
    int get_id();
    int update_subset();
    void db_print(char*);
    void db_print_values(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(RESULT& r) {RESULT::operator=(r);}
};

class DB_WORKUNIT : public DB_BASE, public WORKUNIT {
public:
    DB_WORKUNIT(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(WORKUNIT& w) {WORKUNIT::operator=(w);}
};

class DB_MSG_FROM_HOST : public DB_BASE, public MSG_FROM_HOST {
public:
    DB_MSG_FROM_HOST(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_MSG_TO_HOST : public DB_BASE, public MSG_TO_HOST {
public:
    DB_MSG_TO_HOST(DB_CONN* p=0);
    int get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

// The transitioner uses this to get (WU, result) pairs efficiently.
// Each call to enumerate() returns a list of the pairs for a single WU
//
class DB_TRANSITIONER_ITEM_SET : public DB_BASE_SPECIAL {
public:
    DB_TRANSITIONER_ITEM_SET(DB_CONN* p=0);
    TRANSITIONER_ITEM last_item;
    int nitems_this_query;

    int enumerate(
        int transition_time,
        int nresult_limit,
        std::vector<TRANSITIONER_ITEM>& items
    );
    int update_result(TRANSITIONER_ITEM&);
    int update_workunit(TRANSITIONER_ITEM&);
};

// The validator uses this to get (WU, result) pairs efficiently.
// Each call to enumerate() returns a list of the pairs for a single WU
//
class DB_VALIDATOR_ITEM_SET : public DB_BASE_SPECIAL {
public:
    DB_VALIDATOR_ITEM_SET(DB_CONN* p=0);
    VALIDATOR_ITEM last_item;
    int nitems_this_query;

    int enumerate(
        int appid,
        int nresult_limit,
        std::vector<VALIDATOR_ITEM>& items
    );
    int update_result(RESULT&);
    int update_workunit(WORKUNIT&);
};

// used by the feeder and scheduler for outgoing work
//
struct WORK_ITEM {
    int res_id;
    WORKUNIT wu;
    void parse(MYSQL_ROW& row);
};

class DB_WORK_ITEM : public WORK_ITEM, public DB_BASE_SPECIAL {
public:
    DB_WORK_ITEM(DB_CONN* p=0);
    // CURSOR cursor;
    int enumerate(int limit, char* order_clause);
        // used by feeder
    int read_result();
        // used by scheduler to read result server state
    int update();
        // used by scheduler to update WU transition time
        // and various result fields
};

// Used by the scheduler to handle results reported by clients
// The read and the update of these results are combined
// into single SQL queries.

struct SCHED_RESULT_ITEM {
    char queried_name[256];     // name as reported by client
    int id;
    char name[256];
    int workunitid;
    int server_state;
    int client_state;
    int validate_state;
    int outcome;
    int hostid;
    int userid;
    int teamid;
    int received_time;
    double cpu_time;
    double claimed_credit;
    char xml_doc_out[LARGE_BLOB_SIZE];
    char stderr_out[LARGE_BLOB_SIZE];
    int app_version_num;
    int exit_status;

    void clear();
    void parse(MYSQL_ROW& row);
};

class DB_SCHED_RESULT_ITEM_SET : public DB_BASE_SPECIAL {
public:
    DB_SCHED_RESULT_ITEM_SET(DB_CONN* p=0);
    std::vector<SCHED_RESULT_ITEM> results;

    int add_result(char* result_name);

    int enumerate();
        // using a single SQL query, look up all the reported results,
        // (based on queried_name)
        // and fill in the rest of the entries in the results vector

    int lookup_result(char* result_name, SCHED_RESULT_ITEM** result);

    int update_result(SCHED_RESULT_ITEM& result);
    int update_workunits();
};

#endif

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

#ifndef _DB_
#define _DB_

// Structures corresponding to database records.
// Some of these types have counterparts in client/types.h,
// but don't be deceived - client and server have different variants.

// The parse and write functions are for use in scheduler RPC.
// They don't necessarily serialize the entire records.

#include <stdio.h>

// Maximum allowed size for SQL based blobs (Binary Large Object)
#define MAX_BLOB_SIZE   4096

struct PROJECT {
    int id;
    char name[256];
};

// A compilation target, i.e. a architecture/OS combination.
// Currently the core client will be given only applications
// that match its platform exactly.
// This could be generatlized to allow, say, an app compiled
// for AMD to be run on a "generic Intel x86" core client.
// In this case we'd need a 1 to N mapping from APP_VERSION to PLATFORM
//
struct PLATFORM {
    int id;
    unsigned int create_time;
    char name[256];             // i.e. "sparc-sun-solaris2.7"
};

// An application.
// The core client is viewed as an application; its name is "core_client".
//
struct APP {
    int id;
    unsigned int create_time;
    char name[256];     // application name, preferably short
    int min_version;    // don't use app versions before this
    int write(FILE*);
};

// A version of an application.
//
struct APP_VERSION {
    int id;
    unsigned int create_time;
    int appid;
    int version_num;
    int platformid;

    char xml_doc[MAX_BLOB_SIZE];
	// describes app files. format:
	// <file_info>...</file_info>
	// ...
	// <file_assocs>
	//    <io_file_desc>...</io_file_desc>
	//    ...
	// </file_assocs>
	//

    // The following defined for apps other than core client.
    // They let you handle backwards-incompatible changes to
    // the core client / app interface
    //
    int min_core_version;   // min core version this will run with
    int max_core_version;   // if <>0, max core version this will run with

    // the following defined for core client
    //
    char message[256];      // if we get a request from this version,
                            // send this message
    bool deprecated;        // if we get a request from this version,
                            // don't send it any work.

    int write(FILE*, APP&);
};

struct USER {
    int id;
    unsigned int create_time;
    char email_addr[256];
    char name[256];
    char web_password[256];         // optional
    char authenticator[256];
    char country[256];
    char postal_code[256];
    double total_credit;
    double expavg_credit;           // credit per second, recent average
    double expavg_time;             // when the above was computed
    char global_prefs[MAX_BLOB_SIZE];  // global preferences
        // within <global_preferences> tag
    char project_prefs[MAX_BLOB_SIZE];
        // within <project_preferences> tag
    int teamid;                     // if user is part of a team
};

#define TEAM_TYPE_COMPANY_SMALL         1
#define TEAM_TYPE_COMPANY_MEDIUM        2
#define TEAM_TYPE_COMPANY_LARGE         3
#define TEAM_TYPE_CLUB                  4
#define TEAM_TYPE_PRIMARY               5
#define TEAM_TYPE_SECONDARY             6
#define TEAM_TYPE_UNIVERSITY            7
#define TEAM_TYPE_JUNIOR_COLLEGE        8
#define TEAM_TYPE_GOVERNMENT            9

struct TEAM {
    int id;
    int userid;             // User ID of team founder
    char name[256];
    char name_lc[256];      // Team name in lowercase (used for searching)
    char url[256];
    int type;               // Team type (see above)
    char name_html[256];
    char description[256];
    int nusers;
};

struct HOST {
    int id;
    unsigned int create_time;
    int userid;             // ID of user running this host
    int rpc_seqno;          // last seqno received from client
    unsigned int rpc_time;  // time of last scheduler RPC
    double total_credit;
    double expavg_credit;   // credit per second, recent average
    double expavg_time;     // last time the above was updated

    // all remaining items are assigned by the client
    int timezone;
    char domain_name[256];
    char serialnum[256];
    char last_ip_addr[256];
    int nsame_ip_addr;

    double on_frac;         // Fraction of the time (0-1) that the host is on
    double connected_frac;  // Fraction of time that host is connected
    double active_frac;     // Fraction of time that host is enabled to work

    int p_ncpus;            // Number of CPUs on host
    char p_vendor[256];     // Vendor name of CPU
    char p_model[256];      // Model of CPU
    double p_fpops;         // measured floating point ops/sec of CPU
    double p_iops;          // measured integer ops/sec of CPU
    double p_membw;         // measured memory bandwidth (bytes/sec) of CPU
    double p_calculated;    // when the above were calculated

    char os_name[256];      // Name of operating system
    char os_version[256];   // Version of operating system

    double m_nbytes;        // Size of memory in bytes
    double m_cache;         // Size of CPU cache in bytes (L1 or L2?)
    double m_swap;          // Size of swap space in bytes

    double d_total;         // Total disk space
    double d_free;

    double n_bwup;          // Average upload bandwidth
    double n_bwdown;        // Average download bandwidth

    // The following is derived (by server) from other fields
    double credit_per_cpu_sec;

    int parse(FILE*);
    int parse_time_stats(FILE*);
    int parse_net_stats(FILE*);
};

#define WU_STATE_SEND_FAIL          1
    // failed to send results for this WU
#define WU_STATE_TOO_MANY_ERRORS    2
    // too many errors; may have bug
#define WU_STATE_TOO_MANY_DONE      3
    // too many results without consensus; may be nondeterministic

struct WORKUNIT {
    int id;
    unsigned int create_time;
    int appid;                  // associated app
    char name[256];
    char xml_doc[MAX_BLOB_SIZE];
    int batch;
    double rsc_fpops;           // estimated # of FP operations
    double rsc_iops;            // estimated # of integer operations
    double rsc_memory;          // estimated size of RAM working set (bytes)
    double rsc_disk;            // estimated amount of disk needed (bytes)
    bool need_validate;         // this WU has at least 1 result in
                                // VALIDATE_STATE_NEED_CHECK state
    int canonical_resultid;     // ID of canonical result, or zero
    double canonical_credit;    // credit that all correct results get
    double retry_check_time;    // when to check for result retry
    int delay_bound;            // determines result deadline, retry check time
    int state;                  // see values above
    int workseq_next;           // if part of a sequence, the next WU

    // the following not used in the DB
    char app_name[256];
};

#define RESULT_STATE_INACTIVE       1
#define RESULT_STATE_UNSENT         2
#define RESULT_STATE_IN_PROGRESS    3
#define RESULT_STATE_DONE           4
#define RESULT_STATE_TIMEOUT        5
#define RESULT_STATE_ERROR          6
#define RESULT_STATE_UNSENT_SEQ     7
    // unsent, part of a work sequence

#define VALIDATE_STATE_INITIAL      0
#define VALIDATE_STATE_NEED_CHECK   1
#define VALIDATE_STATE_VALID        2
#define VALIDATE_STATE_INVALID      3

struct RESULT {
    int id;
    unsigned int create_time;
    int workunitid;
    int state;                      // state (see above)
    int hostid;                     // host processing this result
    unsigned int report_deadline;   // deadline for receiving result
    unsigned int sent_time;         // when result was sent to host
    unsigned int received_time;     // when result was received from host
    char name[256];
    int exit_status;
    double cpu_time;                // CPU time used to complete result
    char xml_doc_in[MAX_BLOB_SIZE];     // descriptions of output files
    char xml_doc_out[MAX_BLOB_SIZE];    // MD5s of output files
    char stderr_out[MAX_BLOB_SIZE];     // stderr output, if any
    int batch;
    int project_state;
    int validate_state;
    double claimed_credit;      // CPU time times host credit/sec
    double granted_credit;      // == canonical credit of WU

    // the following not used in the DB
    char wu_name[256];
    int parse_from_client(FILE*);
};

#define WORKSEQ_STATE_UNASSIGNED    0
#define WORKSEQ_STATE_ASSIGNED      1
#define WORKSEQ_STATE_DONE          2

struct WORKSEQ {
    int id;
    unsigned int create_time;
    int state;
    int hostid;                     // host this seq is assigned to
    int wuid_last_done;             // last validated WU or zero
    int wuid_last_sent;             // last sent WU or zero
    int workseqid_master;           // if part of a redundant group, master ID
};

extern int db_open(char* dbname, char* passwd);
extern int db_close();
extern void db_print_error(char*);
extern int db_insert_id();

extern int db_project_new(PROJECT& p);
extern int db_project_enum(PROJECT& p);

extern int db_platform_new(PLATFORM& p);
extern int db_platform_enum(PLATFORM& p);
extern int db_platform_lookup_name(PLATFORM&);

extern int db_app_new(APP&);
extern int db_app(int, APP&);
extern int db_app_enum(APP&);
extern int db_app_update(APP&);
extern int db_app_lookup_name(APP&);

extern int db_app_version_new(APP_VERSION&);
extern int db_app_version_lookup(
    int appid, int platformid, int version, APP_VERSION&
);
extern int db_app_version_enum(APP_VERSION&);

extern int db_user_new(USER&);
extern int db_user(int, USER&);
extern int db_user_update(USER&);
extern int db_user_lookup_auth(USER&);
extern int db_user_lookup_email_addr(USER&);

extern int db_team(int, TEAM&);
extern int db_team_new(TEAM&);
extern int db_team_update(TEAM&);
extern int db_team_lookup_name(TEAM&);
extern int db_team_lookup_name_lc(TEAM&);
extern int db_team_enum(TEAM&);

extern int db_host_new(HOST& p);
extern int db_host(int, HOST&);
extern int db_host_update(HOST&);

extern int db_workunit_new(WORKUNIT& p);
extern int db_workunit(int id, WORKUNIT&);
extern int db_workunit_update(WORKUNIT& p);
extern int db_workunit_lookup_name(WORKUNIT&);
extern int db_workunit_enum_app_need_validate(WORKUNIT&);
extern int db_workunit_enum_retry_check_time(WORKUNIT&);

extern int db_result_new(RESULT& p);
extern int db_result(int id, RESULT&);
extern int db_result_update(RESULT& p);
extern int db_result_lookup_name(RESULT& p);
extern int db_result_enum_state(RESULT&, int);
extern int db_result_enum_wuid(RESULT&);
extern int db_result_count_state(int state, int&);

extern int db_workseq_new(WORKSEQ& p);
#endif

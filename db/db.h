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

// Some of these types are similar to those in client/types.h
// But we choose to keep them separate, since client and server have
// different variants.

// The parse and write functions are for use in scheduler RPC.
// They don't necessarily serialize the entire records.

#include <stdio.h>

#define MAX_BLOB_SIZE   4096

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
    char name[256];
};

// An application.
// The core client is viewed as an application; its name is "core_client".
//
struct APP {
    int id;
    unsigned int create_time;
    char name[256];     // preferably short
    int alpha_vers;
    int beta_vers;
    int prod_vers;
    char result_xml_template[MAX_BLOB_SIZE];
        // if any workunits have dynamic results,
        // XML template for results comes from here
        // The template is processed by the function
        // process_result_template() from backend_lib.C

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

    // describes app files. format:
    // <file_info>...</file_info>
    // ...
    // <file_assocs>
    //    <io_file_desc>...</io_file_desc>
    //    ...
    // </file_assocs>
    //
    char xml_doc[MAX_BLOB_SIZE];

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
    char web_password[256];
    char authenticator[256];
    char country[256];
    char postal_code[256];
    double total_credit;
    double expavg_credit;   // exponenially averaged credit
    int expavg_time;        // last time the above was computed
    char prefs[MAX_BLOB_SIZE];
    unsigned int prefs_mod_time;
    int teamid;			//if the user is part of a team
};

#define TEAM_TYPE_COMPANY_SMALL		1
#define TEAM_TYPE_COMPANY_MEDIUM	2
#define TEAM_TYPE_COMPANY_LARGE		3
#define TEAM_TYPE_CLUB			4
#define TEAM_TYPE_PRIMARY		5
#define TEAM_TYPE_SECONDARY		6
#define TEAM_TYPE_UNIVERSITY		7
#define TEAM_TYPE_JUNIOR_COLLEGE	8
#define TEAM_TYPE_GOVERNMENT		9

struct TEAM {
    int id;
    int userid;
    char name[256];
    char name_lc[256];
    char url[256];
    int type;
    char name_html[256];
    char description[256];
    int nusers;
};

struct HOST {
    int id;
    unsigned int create_time;
    int userid;
    int rpc_seqno;          // last seqno received from client
    unsigned int rpc_time;

    // all remaining items are assigned by the client
    int timezone;
    char domain_name[256];
    char serialnum[256];
    char last_ip_addr[256];
    int nsame_ip_addr;

    double on_frac;
    double connected_frac;
    double active_frac;

    int p_ncpus;
    char p_vendor[256];
    char p_model[256];
    double p_fpops;
    double p_iops;
    double p_membw;
    double p_calculated;

    char os_name[256];
    char os_version[256];

    double m_nbytes;
    double m_cache;
    double m_swap;

    double d_total;
    double d_free;

    double n_bwup;
    double n_bwdown;

    int parse(FILE*);
    int parse_time_stats(FILE*);
    int parse_net_stats(FILE*);
};

struct WORKUNIT {
    int id;
    unsigned int create_time;
    int appid;
    int previous_wuid;
    bool has_successor;
    char name[256];
    char xml_doc[MAX_BLOB_SIZE];
    int batch;
    double rsc_fpops;       // estimated # of FP operations
    double rsc_iops;        // estimated # of integer operations
    double rsc_memory;      // estimated size of RAM working set (bytes)
    double rsc_disk;        // estimated amount of disk needed (bytes)
    bool dynamic_results;
        // whether to create new results on demand
    int max_results;        // 0 if no limit
    int nresults;
    int nresults_unsent;
    int nresults_done;
    int nresults_fail;

    // the following not used in the DB
    char app_name[256];
};

#define RESULT_STATE_INACTIVE       1
#define RESULT_STATE_UNSENT         2
#define RESULT_STATE_IN_PROGRESS    3
#define RESULT_STATE_DONE           4
#define RESULT_STATE_TIMEOUT        5
#define RESULT_STATE_ERROR          6

struct RESULT {
    int id;
    unsigned int create_time;
    int workunitid;
    int state;
    int hostid;
    unsigned int report_deadline;
    unsigned int sent_time;
    unsigned int received_time;
    char name[256];
    int exit_status;
    double cpu_time;
    char xml_doc_in[MAX_BLOB_SIZE];     // descriptions of output files
    char xml_doc_out[MAX_BLOB_SIZE];    // MD5s of output files
    char stderr_out[MAX_BLOB_SIZE];     // stderr output, if any
    int batch;
    int project_state;
    bool validated;

    // the following not used in the DB
    char wu_name[256];
    int parse_from_client(FILE*);
};

extern int db_open(char*);
extern int db_close();
extern void db_print_error(char*);
extern int db_insert_id();

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
extern int db_workunit_enum_dynamic_to_send(WORKUNIT&, int);

extern int db_result_new(RESULT& p);
extern int db_result_update(RESULT& p);
extern int db_result_lookup_name(RESULT& p);
extern int db_result_enum_to_send(RESULT&, int);

#endif

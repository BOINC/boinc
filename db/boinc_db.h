// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _BOINC_DB_
#define _BOINC_DB_

// Structures passed to and from DB queries.
//
// Mostly these correspond to DB tables, and inherit structs
// defined in boinc_db_types.h
// But some of them - TRANSITIONER_ITEM, STATE_COUNTS, SCHED_RESULT_ITEM, etc. -
// combine the info from multiple tables (from joins)
// or have subsets of table data.
//
// Some of these types have counterparts in client/types.h,
// but don't be deceived - client and server have different variants.

// The parse and write functions are for use in scheduler RPC.
// They don't necessarily serialize the entire records.

#include <cstdio>
#include <vector>
#include <string.h>

#include "db_base.h"
#include "boinc_db_types.h"

extern DB_CONN boinc_db;

struct TRANSITIONER_ITEM {
    // the following read from workunit table

    DB_ID_TYPE id; // This is the WU ID
    char name[256];
    DB_ID_TYPE appid;
    int min_quorum;
    bool need_validate;
    DB_ID_TYPE canonical_resultid;
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
    int hr_class;
    int batch;
    DB_ID_TYPE app_version_id;
    int transitioner_flags;
    int size_class;

    // the following read from result table

    DB_ID_TYPE res_id; // This is the RESULT ID
    char res_name[256];
    int res_report_deadline;
    int res_server_state;
    int res_outcome;
    int res_validate_state;
    int res_file_delete_state;
    int res_sent_time;
    DB_ID_TYPE res_hostid;
    int res_received_time;
    DB_ID_TYPE res_app_version_id;
    int res_exit_status;

    void clear();
    void parse(MYSQL_ROW&);
};

struct DB_HOST_APP_VERSION : public DB_BASE, public HOST_APP_VERSION {
    DB_HOST_APP_VERSION(DB_CONN* p=0);
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    int update_scheduler(DB_HOST_APP_VERSION&);
    int update_validator(DB_HOST_APP_VERSION&);
};

struct DB_USER_SUBMIT : public DB_BASE, public USER_SUBMIT {
    DB_USER_SUBMIT(DB_CONN* p=0);
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

struct STATE_COUNTS {
    DB_ID_TYPE appid;
    int last_update_time;
    int result_server_state_2;
    int result_server_state_4;
    int result_file_delete_state_1;
    int result_file_delete_state_2;
    int result_server_state_5_and_file_delete_state_0;
    int workunit_need_validate_1;
    int workunit_assimilate_state_1;
    int workunit_file_delete_state_1;
    int workunit_file_delete_state_2;

    void clear();
};

struct DB_STATE_COUNTS : public DB_BASE, public STATE_COUNTS {
    DB_STATE_COUNTS(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char *);
    void db_parse(MYSQL_ROW &row);
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
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_APP : public DB_BASE, public APP {
public:
    DB_APP(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_APP_VERSION : public DB_BASE, public APP_VERSION {
public:
    DB_APP_VERSION(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(APP_VERSION& w) {APP_VERSION::operator=(w);}
};

class DB_USER : public DB_BASE, public USER {
public:
    DB_USER(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(USER& r) {USER::operator=(r);}
};

class DB_USER_DELETED : public DB_BASE, public USER_DELETED {
public:
    DB_USER_DELETED(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(USER_DELETED& r) {USER_DELETED::operator=(r);}
};

class DB_TEAM : public DB_BASE, public TEAM {
public:
    DB_TEAM(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_HOST : public DB_BASE, public HOST {
public:
    DB_HOST(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    int update_diff_sched(HOST&);
    int update_diff_validator(HOST&);
    int fpops_percentile(double percentile, double& fpops);
        // return the given percentile of p_fpops
    int fpops_mean(double& mean);
    int fpops_stddev(double& stddev);
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(HOST& r) {HOST::operator=(r);}
};

class DB_HOST_DELETED : public DB_BASE, public HOST_DELETED {
public:
    DB_HOST_DELETED(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(HOST_DELETED& r) {HOST_DELETED::operator=(r);}
};

class DB_RESULT : public DB_BASE, public RESULT {
public:
    DB_RESULT(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    int mark_as_sent(int old_server_state, int report_grace_period);
    void db_print(char*);
    void db_print_values(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(RESULT& r) {RESULT::operator=(r);}
    int get_unsent_counts(APP&, int* unsent, int count_max);
    int make_unsent(
        APP&, int size_class, int n, const char* order_clause, int& nchanged
    );
};

class DB_WORKUNIT : public DB_BASE, public WORKUNIT {
public:
    DB_WORKUNIT(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_print_values(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(WORKUNIT& w) {WORKUNIT::operator=(w);}
};

class DB_BATCH : public DB_BASE, public BATCH {
public:
    DB_BATCH(DB_CONN* p=0);
    DB_ID_TYPE get_id();
};


class DB_CREDITED_JOB : public DB_BASE, public CREDITED_JOB {
public:
    DB_CREDITED_JOB(DB_CONN* p=0);
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(CREDITED_JOB& wh) {CREDITED_JOB::operator=(wh);}
};

class DB_MSG_FROM_HOST : public DB_BASE, public MSG_FROM_HOST {
public:
    DB_MSG_FROM_HOST(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_MSG_TO_HOST : public DB_BASE, public MSG_TO_HOST {
public:
    DB_MSG_TO_HOST(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

class DB_ASSIGNMENT : public DB_BASE, public ASSIGNMENT {
public:
    DB_ASSIGNMENT(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW& row);
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
        int wu_id_modulus,
        int wu_id_remainder,
        std::vector<TRANSITIONER_ITEM>& items
    );
    int update_result(TRANSITIONER_ITEM&);
    int update_workunit(TRANSITIONER_ITEM&, TRANSITIONER_ITEM&);
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
        DB_ID_TYPE appid,
        int nresult_limit,
        int wu_id_modulus,
        int wu_id_remainder,
        DB_ID_TYPE wu_id_min,
        DB_ID_TYPE wu_id_max,
        std::vector<VALIDATOR_ITEM>& items
    );
    int update_result(RESULT&);
    int update_workunit(WORKUNIT&);
};


// used by the feeder and scheduler for outgoing work
//
struct WORK_ITEM {
    DB_ID_TYPE res_id;
    int res_priority;
    int res_server_state;
    double res_report_deadline;
    WORKUNIT wu;
    void parse(MYSQL_ROW& row, bool batch_accel=false);
};

class DB_WORK_ITEM : public WORK_ITEM, public DB_BASE_SPECIAL {
    DB_ID_TYPE start_id;
        // when enumerate_all is used, keeps track of which ID to start from
public:
    DB_WORK_ITEM(DB_CONN* p=0);
    int enumerate(
        int limit, const char* select_clause, const char* order_clause,
        bool batch_accel
    );
        // used by feeder
    int enumerate_all(
        int limit, const char* select_clause
    );
        // used by feeder when HR is used.
        // Successive calls cycle through all results.
    int read_result();
        // used by scheduler to read result server state
    int update();
        // used by scheduler to update WU transition time
        // and various result fields
};

// Used by the scheduler to send <result_abort> or <result_abort_if_not_started>
// messages if the result is no longer needed.
//
struct IN_PROGRESS_RESULT {
	char result_name[256];
	int assimilate_state;
	int error_mask;
	int server_state;
	int outcome;
    void parse(MYSQL_ROW& row);
};

class DB_IN_PROGRESS_RESULT : public IN_PROGRESS_RESULT, public DB_BASE_SPECIAL {
public:
    DB_IN_PROGRESS_RESULT(DB_CONN* p=0);
    int enumerate(DB_ID_TYPE hostid, const char* result_names);
};

// Used by the scheduler to handle results reported by clients
// The read and the update of these results are combined
// into single SQL queries.

struct SCHED_RESULT_ITEM {
    char queried_name[256];     // name as reported by client

    // the following items are read from the DB.
    // They are used to sanity-check the reported result
    // (e.g. it may have been reported already)
    // or to write to the job log
    // TODO: do we need all of them?

    DB_ID_TYPE id;
    char name[256];
    DB_ID_TYPE workunitid;
    DB_ID_TYPE appid;
    int server_state;
    int client_state;
    int validate_state;
    int outcome;
    DB_ID_TYPE hostid;
    DB_ID_TYPE userid;
    int sent_time;
    int received_time;
    int file_delete_state;
    DB_ID_TYPE app_version_id;

    // the following are populated, then written to the DB

    DB_ID_TYPE teamid;
    double cpu_time;
    char xml_doc_out[BLOB_SIZE];
    char stderr_out[BLOB_SIZE];
    int app_version_num;
    int exit_status;
    double elapsed_time;
    double peak_working_set_size;
    double peak_swap_size;
    double peak_disk_usage;

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

struct FILE_ITEM {
    DB_ID_TYPE id;
    char name[254];
    char md5sum[34];
    double size;

    void clear();
};

class DB_FILE : public DB_BASE, public FILE_ITEM {
public:
    DB_FILE(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(FILE_ITEM& f) {FILE_ITEM::operator=(f);}
};

struct FILESET_ITEM {
    DB_ID_TYPE id;
    char name[254];

    void clear();
};

class DB_FILESET : public DB_BASE, public FILESET_ITEM {
public:
    DB_FILESET(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(FILESET_ITEM& f) {FILESET_ITEM::operator=(f);}

    // retrieve fileset instance (populate object)
    int select_by_name(const char* name);
};

struct FILESET_FILE_ITEM {
    DB_ID_TYPE fileset_id;
    DB_ID_TYPE file_id;

    void clear();
};

class DB_FILESET_FILE : public DB_BASE, public FILESET_FILE_ITEM {
public:
    DB_FILESET_FILE(DB_CONN* p=0);
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(FILESET_FILE_ITEM& tf) {FILESET_FILE_ITEM::operator=(tf);}
};

struct SCHED_TRIGGER_ITEM {
    DB_ID_TYPE id;
    DB_ID_TYPE fileset_id;
    bool need_work;
    bool work_available;
    bool no_work_available;
    bool working_set_removal;

    void clear();
};

class DB_SCHED_TRIGGER : public DB_BASE, public SCHED_TRIGGER_ITEM {
public:
    DB_SCHED_TRIGGER(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
    void operator=(SCHED_TRIGGER_ITEM& t) {SCHED_TRIGGER_ITEM::operator=(t);}

    typedef enum {
        none                         = 0,
        state_need_work              = 1,
        state_work_available         = 2,
        state_no_work_available      = 3,
        state_working_set_removal    = 4
    } STATE;

    // retrieve trigger instance (populate object)
    int select_unique_by_fileset_name(const char* fileset_name);
    // set single trigger state
    int update_single_state(const DB_SCHED_TRIGGER::STATE state, const bool value);
};

struct FILESET_SCHED_TRIGGER_ITEM {
    FILESET_ITEM fileset;
    SCHED_TRIGGER_ITEM trigger;

    void clear();
};

class DB_FILESET_SCHED_TRIGGER_ITEM : public DB_BASE_SPECIAL, public FILESET_SCHED_TRIGGER_ITEM {
public:
    DB_FILESET_SCHED_TRIGGER_ITEM(DB_CONN* p=0);
    void db_parse(MYSQL_ROW &row);
    void operator=(FILESET_SCHED_TRIGGER_ITEM& fst) {FILESET_SCHED_TRIGGER_ITEM::operator=(fst);}
};

class DB_FILESET_SCHED_TRIGGER_ITEM_SET : public DB_BASE_SPECIAL {
public:
    DB_FILESET_SCHED_TRIGGER_ITEM_SET(DB_CONN* p=0);

    // select available triggers based on name and/or state
    // -> name filter optional (set string, default NULL)
    // -> pattern search optional (set use_regexp to true, default false))
    // -> state filter optional (set state, default none)
    // -> state_value (default true)
    int select_by_name_state(
            const char* fileset_name,
            const bool use_regexp,
            const DB_SCHED_TRIGGER::STATE state,
            const bool state_value);

    // check if given trigger (fileset name) is part of set and return position (1-indexed)
    int contains_trigger(const char* fileset_name);

    // storage vector
    std::vector<DB_FILESET_SCHED_TRIGGER_ITEM> items;
};

struct DB_VDA_FILE : public DB_BASE, public VDA_FILE {
    DB_VDA_FILE(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

struct DB_VDA_CHUNK_HOST : public DB_BASE, public VDA_CHUNK_HOST {
    DB_VDA_CHUNK_HOST(DB_CONN* p=0);
    void db_print(char*);
    void db_parse(MYSQL_ROW &row);
};

struct DB_BADGE : public DB_BASE, public BADGE {
    DB_BADGE(DB_CONN* p=0);
    DB_ID_TYPE get_id() {return id;};
    void db_print(char*){};
    void db_parse(MYSQL_ROW&);
};

struct DB_BADGE_USER : public DB_BASE, public BADGE_USER {
    DB_BADGE_USER(DB_CONN* p=0);
    void db_print(char*){};
    void db_parse(MYSQL_ROW&);
};

struct DB_BADGE_TEAM : public DB_BASE, public BADGE_TEAM {
    DB_BADGE_TEAM(DB_CONN* p=0);
    void db_print(char*){};
    void db_parse(MYSQL_ROW&);
};

struct DB_CREDIT_USER : public DB_BASE, public CREDIT_USER {
    DB_CREDIT_USER(DB_CONN* p=0);
    void db_print(char*);
    void db_parse(MYSQL_ROW&);
};

struct DB_CREDIT_TEAM : public DB_BASE, public CREDIT_TEAM {
    DB_CREDIT_TEAM(DB_CONN* p=0);
    void db_print(char*);
    void db_parse(MYSQL_ROW&);
};

struct DB_CONSENT_TYPE : public DB_BASE, public CONSENT_TYPE {
    DB_CONSENT_TYPE(DB_CONN* p=0);
    DB_ID_TYPE get_id();
    void db_print(char *);
    void db_parse(MYSQL_ROW &row);
};


#endif

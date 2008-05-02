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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _SERVER_TYPES_
#define _SERVER_TYPES_

#include <cstdio>
#include <vector>

#include "boinc_db.h"
#include "common_defs.h"
#include "md5_file.h"
#include "coproc.h"

#include "edf_sim.h"

// for projects that support work filtering by app,
// this records an app for which the user will accept work
//
struct APP_INFO {
	int appid;
	int work_available;
};

// Details concerning a host
//
struct HOST_INFO {
	int allow_non_preferred_apps;
	int allow_beta_work;
	bool reliable;
	std::vector<APP_INFO> preferred_apps;
};

// represents a resource (disk etc.) that the client may not have enough of
//
struct RESOURCE {
    bool insufficient;
    double needed;      // the min extra amount needed

    inline void set_insufficient(double x) {
        insufficient = true;
        if (needed) {
            if (x < needed) needed = x;
        } else {
            needed = x;
        }
    }
};

// message intended for human eyes
//
struct USER_MESSAGE {
    std::string message;
    std::string priority;
    USER_MESSAGE(const char* m, const char*p);
};

struct HOST_USAGE {
    COPROCS coprocs;
    double avg_ncpus;
    double max_ncpus;
    double flops;
    char cmdline[256];

    void init_seq(double x) {
        coprocs.coprocs.clear();
        avg_ncpus = 1;
        max_ncpus = 1;
        flops = x;
        strcpy(cmdline, "");
    }
    ~HOST_USAGE(){}
};

// keep track of the best app_version for each app for this host
//
struct BEST_APP_VERSION {
    int appid;
    APP_VERSION* avp;       // NULL if none exists
    HOST_USAGE host_usage;
};

// summary of a client's request for work, and our response to it
//
struct WORK_REQ {
    bool infeasible_only;
    bool reliable_only;
    bool user_apps_only;
    bool beta_only;
    HOST_INFO host_info;
    double seconds_to_fill;
		// in "normalized CPU seconds"; see
        // http://boinc.berkeley.edu/trac/wiki/ClientSched#NormalizedCPUTime
    double disk_available;
    int nresults;
    int core_client_version;
    double running_frac;

    RESOURCE disk;
    RESOURCE mem;
    RESOURCE speed;
    RESOURCE bandwidth;

    std::vector<USER_MESSAGE> no_work_messages;
    std::vector<BEST_APP_VERSION*> best_app_versions;

    bool no_allowed_apps_available;
    bool excessive_work_buf;
    bool no_app_version;
    bool hr_reject_temp;
    bool hr_reject_perm;
    bool outdated_core;
    bool daily_result_quota_exceeded;
    int  daily_result_quota; // for this machine: number of cpus * daily_quota/cpu
    bool cache_size_exceeded;
    int nresults_on_host;
        // How many results from this project are (or should be) on the host.
        // Initially this is the number of "other_results"
        // reported in the request message.
        // If the resend_lost_results option is used,
        // it's set to the number of outstanding results taken from the DB
        // (those that were lost are resent).
        // As new results are sent, it's incremented.
    void update_for_result(double seconds_filled);
    void insert_no_work_message(USER_MESSAGE&);
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
    double mod_time;
    double disk_max_used_gb;
    double disk_max_used_pct;
    double disk_min_free_gb;
    double work_buf_min_days;
    double ram_max_used_busy_frac;
    double ram_max_used_idle_frac;
    double max_ncpus_pct;

    void parse(const char* buf, const char* venue);
    void defaults();
    inline double work_buf_min() {return work_buf_min_days*86400;}
};

struct GUI_URLS {
    char* text;
    void init();
    void get_gui_urls(USER& user, HOST& host, TEAM& team, char*);
};

struct PROJECT_FILES {
    char* text;
    void init();
};

// Represents a result from this project that the client has.
// The request message has a list of these.
// The reply message may include a list of those to be aborted
// or aborted if not started
//
struct OTHER_RESULT {
    std::string name;
    bool abort;
    bool abort_if_not_started;
    int reason;     // see codes below

    int parse(FILE*);
};

#define ABORT_REASON_NOT_FOUND      1
#define ABORT_REASON_WU_CANCELLED   2
#define ABORT_REASON_ASSIMILATED    3
#define ABORT_REASON_TIMED_OUT      4

struct CLIENT_PLATFORM {
    char name[256];
    int parse(FILE*);
};

struct PLATFORM_LIST {
    std::vector<PLATFORM*> list;
};

struct SCHEDULER_REQUEST {
    char authenticator[256];
    CLIENT_PLATFORM platform;
    std::vector<CLIENT_PLATFORM> alt_platforms;
    PLATFORM_LIST platforms;
    char cross_project_id[256];
    int hostid;                 // zero if first RPC
    int core_client_major_version;
    int core_client_minor_version;
    int core_client_release;
    int rpc_seqno;
    double work_req_seconds;
		// in "normalized CPU seconds" (see work_req.php)
    double resource_share_fraction;
        // this project's fraction of total resource share
    double rrs_fraction;
        // ... of runnable resource share
    double prrs_fraction;
        // ... of potentially runnable resource share
    double estimated_delay;
        // how many wall-clock seconds will elapse before
        // host will begin any new work for this project
    double duration_correction_factor;
    char global_prefs_xml[BLOB_SIZE];
    char working_global_prefs_xml[BLOB_SIZE];
    char code_sign_key[4096];

    bool anonymous_platform;
    std::vector<CLIENT_APP_VERSION> client_app_versions;
    GLOBAL_PREFS global_prefs;
    char global_prefs_source_email_hash[MD5_LEN];

    HOST host;      // request message is parsed into here.
                    // does NOT contain the full host record.
    COPROCS coprocs;
    std::vector<RESULT> results;
        // completed results being reported
    std::vector<MSG_FROM_HOST_DESC> msgs_from_host;
    std::vector<FILE_INFO> file_infos;
        // sticky files reported by host for locality scheduling
    std::vector<FILE_INFO> file_delete_candidates;
        // sticky files reported by host, deletion candidates
    std::vector<FILE_INFO> files_not_needed;
        // sticky files reported by host, no longer needed
    std::vector<OTHER_RESULT> other_results;
        // in-progress results from this project
    std::vector<IP_RESULT> ip_results;
        // in-progress results from all projects
    bool have_other_results_list;
    bool have_ip_results_list;
    bool have_time_stats_log;
    bool client_cap_plan_class;

    SCHEDULER_REQUEST();
    ~SCHEDULER_REQUEST();
    int parse(FILE*);
    bool has_version(APP& app);
    int write(FILE*); // write request info to file: not complete
};

// keep track of bottleneck disk preference
//
struct DISK_LIMITS {
    double max_used;
    double max_frac;
    double min_free;
};

// NOTE: if any field requires initialization,
// you must do it in the constructor.  Nothing is zeroed by default.
//
struct SCHEDULER_REPLY {
    WORK_REQ wreq;
    DISK_LIMITS disk_limits;
    double request_delay;       // don't request again until this time elapses
    std::vector<USER_MESSAGE> messages;
    int hostid;
        // nonzero only if a new host record was created.
        // this tells client to reset rpc_seqno
    int lockfile_fd; // file descriptor of lockfile, or -1 if no lock.
    bool send_global_prefs;
    bool nucleus_only;          // send only message
    USER user;
    char email_hash[MD5_LEN];
    HOST host;                  // after validation, contains full host rec
    TEAM team;
    std::vector<APP> apps;
    std::vector<APP_VERSION> app_versions;
    std::vector<WORKUNIT>wus;
    std::vector<RESULT>results;
    std::vector<std::string>result_acks;
    std::vector<std::string>result_aborts;
    std::vector<std::string>result_abort_if_not_starteds;
    std::vector<MSG_TO_HOST>msgs_to_host;
    std::vector<FILE_INFO>file_deletes;
    char code_sign_key[4096];
    char code_sign_key_signature[4096];
    bool send_msg_ack;
    bool project_is_down;

    SCHEDULER_REPLY();
    ~SCHEDULER_REPLY();
    int write(FILE*);
    void insert_app_unique(APP&);
    void insert_app_version_unique(APP_VERSION&);
    void insert_workunit_unique(WORKUNIT&);
    void insert_result(RESULT&);
    void insert_message(USER_MESSAGE&);
    bool work_needed(bool locality_sched=false);
    void set_delay(double);
    void got_good_result();     // adjust max_results_day
    void got_bad_result();      // adjust max_results_day
};

#endif

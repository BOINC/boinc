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
    double ncudas;
    double natis;
    double gpu_ram;
    double avg_ncpus;
    double max_ncpus;
    double projected_flops;
        // the scheduler's best estimate of wu.rsc_fpops_est/elapsed_time.
        // Taken from host_app_version elapsed time statistics if available,
        // else on estimate provided by app_plan()
    double peak_flops;
        // stored in result.estimated_flops, and used for credit calculations
    char cmdline[256];

    HOST_USAGE() {
        ncudas = 0;
        natis = 0;
        gpu_ram = 0;
        avg_ncpus = 1;
        max_ncpus = 1;
        projected_flops = 0;
        peak_flops = 0;
        strcpy(cmdline, "");
    }
    void sequential_app(double flops) {
        ncudas = 0;
        natis = 0;
        gpu_ram = 0;
        avg_ncpus = 1;
        max_ncpus = 1;
        if (flops <= 0) flops = 1e9;
        projected_flops = flops;
        peak_flops = flops;
        strcpy(cmdline, "");
    }
    inline bool is_sequential_app() {
         if (ncudas) return false;
         if (natis) return false;
         if (avg_ncpus != 1) return false;
         return true;
    }
    inline int resource_type() {
        if (ncudas) {
            return ANON_PLATFORM_NVIDIA;
        } else if (natis) {
            return ANON_PLATFORM_ATI;
        }
        return ANON_PLATFORM_CPU;
    }
    inline const char* resource_name() {
        if (ncudas) {
            return "nvidia GPU";
        } else if (natis) {
            return "ATI GPU";
        }
        return "CPU";
    }
    inline bool uses_gpu() {
        if (ncudas) return true;
        if (natis) return true;
        return false;
    }
};

// a description of a sticky file on host.
//
struct FILE_INFO {
    char name[256];

    int parse(XML_PARSER&);
};

struct MSG_FROM_HOST_DESC {
    char variety[256];
    std::string msg_text;
    int parse(XML_PARSER&);
};

// an app version from an anonymous-platform client
// (starting with 6.11, ALL clients send these)
//
struct CLIENT_APP_VERSION {
    char app_name[256];
    char platform[256];
    int version_num;
    char plan_class[256];
    HOST_USAGE host_usage;
    double rsc_fpops_scale;
        // multiply wu.rsc_fpops_est and rsc_fpops_limit
        // by this amount when send to client,
        // to reflect the discrepancy between how fast the client
        // thinks the app is versus how fast we think it is
    APP* app;
        // if NULL, this record is a place-holder,
        // used to preserve array indices

    int parse(XML_PARSER&);
};

// keep track of the best app_version for each app for this host
//
struct BEST_APP_VERSION {
    int appid;
    bool for_64b_jobs;
        // maintain this separately for jobs that need > 2GB RAM,
        // in which case we can't use 32-bit apps

    bool present;
        // false means there's no usable version for this app

    CLIENT_APP_VERSION* cavp;
        // populated if anonymous platform

    APP_VERSION* avp;
        // populated otherwise

    HOST_USAGE host_usage;
        // populated in either case

    bool reliable;
    bool trusted;

    DB_HOST_APP_VERSION* host_app_version();
        // get the HOST_APP_VERSION, if any

    BEST_APP_VERSION() {
        present = false;
        cavp = NULL;
        avp = NULL;
    }
};

struct SCHED_DB_RESULT : DB_RESULT {
    // the following used by the scheduler, but not stored in the DB
    //
    char wu_name[256];
    int units;      // used for granting credit by # of units processed
    int parse_from_client(XML_PARSER&);
    char platform_name[256];
    BEST_APP_VERSION bav;

    int write_to_client(FILE*);
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
    char name[256];
    int app_version;    // index into CLIENT_APP_VERSION array
    char plan_class[64];
    bool have_plan_class;
    bool abort;
    bool abort_if_not_started;
    int reason;     // see codes below

    int parse(XML_PARSER&);
};

#define ABORT_REASON_NOT_FOUND      1
#define ABORT_REASON_WU_CANCELLED   2
#define ABORT_REASON_ASSIMILATED    3
#define ABORT_REASON_TIMED_OUT      4

struct CLIENT_PLATFORM {
    char name[256];
    int parse(XML_PARSER&);
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
    int core_client_version;    // 10000*major + 100*minor + release
    int rpc_seqno;
    double work_req_seconds;
        // in "normalized CPU seconds" (see work_req.php)
    double cpu_req_secs;
    double cpu_req_instances;
    double resource_share_fraction;
        // this project's fraction of total resource share
    double rrs_fraction;
        // ... of runnable resource share
    double prrs_fraction;
        // ... of potentially runnable resource share
    double cpu_estimated_delay;
        // currently queued jobs saturate the CPU for this long;
        // used for crude deadline check
    double duration_correction_factor;
    char global_prefs_xml[BLOB_SIZE];
    char working_global_prefs_xml[BLOB_SIZE];
    char code_sign_key[4096];

    std::vector<CLIENT_APP_VERSION> client_app_versions;

    GLOBAL_PREFS global_prefs;
    char global_prefs_source_email_hash[MD5_LEN];

    HOST host;      // request message is parsed into here.
                    // does NOT contain the full host record.
    COPROCS coprocs;
    std::vector<SCHED_DB_RESULT> results;
        // completed results being reported
    std::vector<RESULT> file_xfer_results;
    std::vector<MSG_FROM_HOST_DESC> msgs_from_host;
    std::vector<FILE_INFO> file_infos;
        // sticky files reported by host

    // temps used by locality scheduling:
    std::vector<FILE_INFO> file_delete_candidates;
        // deletion candidates
    std::vector<FILE_INFO> files_not_needed;
        // files no longer needed

    std::vector<OTHER_RESULT> other_results;
        // in-progress results from this project
    std::vector<IP_RESULT> ip_results;
        // in-progress results from all projects
    bool have_other_results_list;
    bool have_ip_results_list;
    bool have_time_stats_log;
    bool client_cap_plan_class;
    int sandbox;
        // whether client uses account-based sandbox.  -1 = don't know
    int allow_multiple_clients;
        // whether client allows multiple clients per host, -1 don't know
    bool using_weak_auth;
        // Request uses weak authenticator.
        // Don't modify user prefs or CPID
    int last_rpc_dayofyear;
    int current_rpc_dayofyear;
    std::string client_opaque;

    SCHEDULER_REQUEST(){};
    ~SCHEDULER_REQUEST(){};
    const char* parse(XML_PARSER&);
    int write(FILE*); // write request info to file: not complete
};

// keep track of bottleneck disk preference
//
struct DISK_LIMITS {
    double max_used;
    double max_frac;
    double min_free;
};

// summary of a client's request for work, and our response to it
// Note: this is zeroed out in SCHEDULER_REPLY constructor
//
struct WORK_REQ {
    bool anonymous_platform;

    // the following defined if anonymous platform
    //
    bool have_cpu_apps;
    bool have_cuda_apps;
    bool have_ati_apps;

    // Flags used by old-style scheduling,
    // while making multiple passes through the work array
    bool infeasible_only;
    bool reliable_only;
    bool user_apps_only;
    bool beta_only;

    bool resend_lost_results;
        // this is set if the request is reporting a result
        // that was previously reported.
        // This is evidence that the earlier reply was not received
        // by the client.  It may have contained results,
        // so check and resend just in case.

    // user preferences
    bool no_cuda;
    bool no_ati;
    bool no_cpu;
    bool allow_non_preferred_apps;
    bool allow_beta_work;
    std::vector<APP_INFO> preferred_apps;

    bool has_reliable_version;
        // whether the host has a reliable app version

    int effective_ncpus;
    int effective_ngpus;

    // 6.7+ clients send separate requests for different resource types:
    //
    double cpu_req_secs;        // instance-seconds requested
    double cpu_req_instances;   // number of idle instances, use if possible
    double cuda_req_secs;
    double cuda_req_instances;
    double ati_req_secs;
    double ati_req_instances;
    inline bool need_cpu() {
        return (cpu_req_secs>0) || (cpu_req_instances>0);
    }
    inline bool need_cuda() {
        return (cuda_req_secs>0) || (cuda_req_instances>0);
    }
    inline bool need_ati() {
        return (ati_req_secs>0) || (ati_req_instances>0);
    }
    inline void clear_cpu_req() {
        cpu_req_secs = 0;
        cpu_req_instances = 0;
    }
    inline void clear_gpu_req() {
        cuda_req_secs = 0;
        cuda_req_instances = 0;
        ati_req_secs = 0;
        ati_req_instances = 0;
    }

    // older clients send send a single number, the requested duration of jobs
    //
    double seconds_to_fill;

    // true if new-type request, which has resource-specific requests
    //
    bool rsc_spec_request;

    double disk_available;
    double ram, usable_ram;
    double running_frac;
    int njobs_sent;

    // The following keep track of the "easiest" job that was rejected
    // by EDF simulation.
    // Any jobs harder than this can be rejected without doing the simulation.
    //
    double edf_reject_min_cpu;
    int edf_reject_max_delay_bound;
    bool have_edf_reject;
    void edf_reject(double cpu, int delay_bound) {
        if (have_edf_reject) {
            if (cpu < edf_reject_min_cpu) edf_reject_min_cpu = cpu;
            if (delay_bound> edf_reject_max_delay_bound) edf_reject_max_delay_bound = delay_bound;
        } else {
            edf_reject_min_cpu = cpu;
            edf_reject_max_delay_bound = delay_bound;
            have_edf_reject = true;
        }
    }
    bool edf_reject_test(double cpu, int delay_bound) {
        if (!have_edf_reject) return false;
        if (cpu < edf_reject_min_cpu) return false;
        if (delay_bound > edf_reject_max_delay_bound) return false;
        return true;
    }

    RESOURCE disk;
    RESOURCE mem;
    RESOURCE speed;
    RESOURCE bandwidth;

    std::vector<USER_MESSAGE> no_work_messages;
    std::vector<BEST_APP_VERSION*> best_app_versions;
    std::vector<DB_HOST_APP_VERSION> host_app_versions;
    std::vector<DB_HOST_APP_VERSION> host_app_versions_orig;

    // various reasons for not sending jobs (used to explain why)
    //
    bool no_allowed_apps_available;
    bool hr_reject_temp;
    bool hr_reject_perm;
    bool outdated_client;
    bool no_cuda_prefs;
    bool no_ati_prefs;
    bool no_cpu_prefs;
    bool max_jobs_on_host_exceeded;
    bool max_jobs_on_host_cpu_exceeded;
    bool max_jobs_on_host_gpu_exceeded;
    bool no_jobs_available;     // project has no work right now
    int max_jobs_per_rpc;
    void update_for_result(double seconds_filled);
    void add_no_work_message(const char*);
    void get_job_limits();

    ~WORK_REQ() {}
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
    std::vector<SCHED_DB_RESULT>results;
    std::vector<std::string>result_acks;
    std::vector<std::string>result_aborts;
    std::vector<std::string>result_abort_if_not_starteds;
    std::vector<MSG_TO_HOST>msgs_to_host;
    std::vector<FILE_INFO>file_deletes;
    std::vector<std::string> file_transfer_requests;
    char code_sign_key[4096];
    char code_sign_key_signature[4096];
    bool send_msg_ack;
    bool project_is_down;
    std::vector<APP_VERSION>old_app_versions;
        // superceded app versions that we consider using because of
        // homogeneous app version.

    SCHEDULER_REPLY();
    ~SCHEDULER_REPLY(){};
    int write(FILE*, SCHEDULER_REQUEST&);
    void insert_app_unique(APP&);
    void insert_app_version_unique(APP_VERSION&);
    void insert_workunit_unique(WORKUNIT&);
    void insert_result(SCHED_DB_RESULT&);
    void insert_message(const char* msg, const char* prio);
    void insert_message(USER_MESSAGE&);
    void set_delay(double);
};

extern SCHEDULER_REQUEST* g_request;
extern SCHEDULER_REPLY* g_reply;
extern WORK_REQ* g_wreq;
extern double capped_host_fpops();

static inline void add_no_work_message(const char* m) {
    g_wreq->add_no_work_message(m);
}

extern void get_weak_auth(USER&, char*);
extern void get_rss_auth(USER&, char*);
extern void read_host_app_versions();
extern DB_HOST_APP_VERSION* get_host_app_version(int gavid);
extern void write_host_app_versions();

extern DB_HOST_APP_VERSION* gavid_to_havp(int gavid);
extern DB_HOST_APP_VERSION* quota_exceeded_version();

inline bool is_64b_platform(const char* name) {
    return (strstr(name, "64") != NULL);
}

#endif

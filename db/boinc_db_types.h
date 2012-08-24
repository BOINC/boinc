// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

#ifndef _BOINC_DB_TYPES_
#define _BOINC_DB_TYPES_

#include <vector>

#include "average.h"
#include "parse.h"

// Sizes of text buffers in memory, corresponding to database BLOBs.
// The following is for regular blobs, 64KB

#define BLOB_SIZE   65536

// The following are for "medium blobs",
// which are 16MB in the DB
//
#define APP_VERSION_XML_BLOB_SIZE   262144
#define MSG_FROM_HOST_BLOB_SIZE     262144
#define MSG_TO_HOST_BLOB_SIZE       262144

struct BEST_APP_VERSION;

// A compilation target, i.e. a architecture/OS combination.
// Client will be sent applications only for platforms they support.
//
struct PLATFORM {
    int id;
    int create_time;
    char name[256];                 // i.e. "sparc-sun-solaris"
    char user_friendly_name[256];   // i.e. "SPARC Solaris 2.8"
    int deprecated;
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
    int homogeneous_redundancy;
    double weight;          // tells the feeder what fraction of results
                            // should come from this app
    bool beta;
    int target_nresults;
    double min_avg_pfc;
        // the weighted average of app_version.pfc.avg
        // over GPU or CPU versions, whichever is less.
        // Approximates (actual FLOPS)/wu.rsc_fpops_est
    bool host_scale_check;
        // use host scaling cautiously, to thwart cherry picking
    bool homogeneous_app_version;
        // do all instances of each job using the same app version

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
    char xml_doc[APP_VERSION_XML_BLOB_SIZE];
    // describes app files. format:
    // <file_info>...</file_info>
    // ...
    // <app_version>
    //     <app_name>...</app_name>
    //     <version_num>x</version_num>
    //     <api_version>n.n.n</api_version>
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
    char plan_class[256];
    AVERAGE pfc;
        // the stats of (claimed PFC)/wu.rsc_fpops_est
        // If wu.rsc_fpops_est is accurate,
        // this is the reciprocal of efficiency
    double pfc_scale;
        // PFC scaling factor for this app (or 0 if not enough data)
        // The reciprocal of this version's efficiency, averaged over all jobs,
        // relative to that of the most efficient version
    double expavg_credit;
    double expavg_time;

    // the following used by scheduler, not in DB
    //
    BEST_APP_VERSION* bavp;

    // used by validator, not in DB
    //
    std::vector<double>pfc_samples;
    std::vector<double>credit_samples;
    std::vector<double>credit_times;

    int write(FILE*);
    void clear();

    inline bool is_multithread() {
        return (strstr(plan_class, "mt") != NULL);
    }
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
    char global_prefs[BLOB_SIZE];
        // global preferences, within <global_preferences> tag
    char project_prefs[BLOB_SIZE];
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
        // deprecated as of 9/2004 - forum_preferences.posts is used instead
        // now used as salt for weak auth

    // The following are specific to SETI@home;
    // they record info about the user's involvement in a prior project
    int seti_id;                    // ID in old DB
    int seti_nresults;              // number of WUs completed
    int seti_last_result_time;      // time of last result (UNIX)
    double seti_total_cpu;          // number of CPU seconds
    char signature[256];
        // deprecated as of 9/2004 - may be used as temp
    bool has_profile;
    char cross_project_id[256];
    char passwd_hash[256];
    bool email_validated;           // deprecated
    int donated;
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
    char description[BLOB_SIZE];
    int nusers;             // UNDEFINED BY DEFAULT
    char country[256];
    double total_credit;
    double expavg_credit;
    double expavg_time;

    int seti_id;            // ID in another DB
        // this is used to identify BOINC-wide teams
    int ping_user;          // user who asked to become founder
    int ping_time;          // when they asked.
        // see html/inc/team.inc for more details

    void clear();
};

struct HOST {
    int id;
    int create_time;
    int userid;             // ID of user running this host
        // If the host is "zombied" during merging of duplicate hosts,
        // this field is set to zero and rpc_seqno is used to
        // store the ID of the new host (kludge, but what the heck)
    int rpc_seqno;          // last seqno received from client
        // also used as a "forwarding ID" for zombied hosts (see above)
    int rpc_time;           // time of last scheduler RPC
    double total_credit;
    double expavg_credit;   // credit per second, recent average
    double expavg_time;     // last time the above was updated

    // all remaining items are assigned by the client
    int timezone;           // local STANDARD time at host - UTC time
                            // (in seconds) 
    char domain_name[256];
    char serialnum[256];    // textual description of coprocessors
    char last_ip_addr[256]; // internal IP address as of last RPC
    int nsame_ip_addr;      // # of RPCs with same IP address

    double on_frac;         // see client/time_stats.h
    double connected_frac;
    double active_frac;
    double cpu_efficiency;  // deprecated as of 6.4 client
                            // reused as a flag for volunteer data archival:
                            // nonzero means this host has been declared dead
    double duration_correction_factor;

    int p_ncpus;            // Number of CPUs on host
    char p_vendor[256];     // Vendor name of CPU
    char p_model[256];      // Model of CPU
    double p_fpops;         // measured floating point ops/sec of CPU
    double p_iops;          // measured integer ops/sec of CPU
    double p_membw;         // measured memory bandwidth (bytes/sec) of CPU
                            // The above are per CPU, not total

    char os_name[256];      // Name of operating system
    char os_version[256];   // Version of operating system

    double m_nbytes;        // Size of memory in bytes
    double m_cache;         // Size of CPU cache in bytes (L1 or L2?)
    double m_swap;          // Size of swap space in bytes

    double d_total;         // Total disk space on volume containing
                            // the BOINC client directory.
    double d_free;          // how much is free on that volume

    // the following 2 items are reported in scheduler RPCs
    // from clients w/ source Oct 4 2005 and later.
    // NOTE: these items plus d_total and d_free are sufficient
    // to avoid exceeding BOINC's limit on total disk space.
    // But they are NOT sufficient to do resource-share-based
    // disk space allocation.
    // This needs to thought about.
    //
    double d_boinc_used_total;
                            // disk space being used in BOINC client dir,
                            // including all projects and BOINC itself
    double d_boinc_used_project;
                            // amount being used for this project

    double d_boinc_max;
        // This field has been repurposed.
        // it's now used to store the project's share of available disk space
        // (reported by recent clients as <d_project_share>
    double n_bwup;          // Average upload bandwidth, bytes/sec
    double n_bwdown;        // Average download bandwidth, bytes/sec
                            // The above are derived from actual
                            // file upload/download times, and may reflect
                            // factors other than network bandwidth

    double credit_per_cpu_sec;
        // deprecated

    char venue[256];        // home/work/school
    int nresults_today;     // results sent since midnight
    double avg_turnaround;  // recent average result turnaround time
    char host_cpid[256];    // host cross-project ID
    char external_ip_addr[256]; // IP address seen by scheduler
    int _max_results_day;
        // MRD is dynamically adjusted to limit work sent to bad hosts.
        // The maximum # of results sent per day is
        // max_results_day * (NCPUS + NCUDA * cuda_multiplier).
        // 0 means uninitialized; set to config.daily_result_quota
        // -1 means this host is blacklisted - don't return results
        // or accept results or trickles; just send it an error message
        // Otherwise it lies in the range 0 .. config.daily_result_quota
        // DEPRECATED: only use is -1 means host is blacklisted
    double _error_rate;
        // dynamic estimate of fraction of results
        // that fail validation
        // DEPRECATED

    // the following not in DB
    char p_features[1024];
    char virtualbox_version[256];
    bool p_vm_extensions_disabled;

    int parse(XML_PARSER&);
    int parse_time_stats(XML_PARSER&);
    int parse_net_stats(XML_PARSER&);
    int parse_disk_usage(XML_PARSER&);

    void fix_nans();
    void clear();
};

// values for file_delete state
#define FILE_DELETE_INIT        0
#define FILE_DELETE_READY       1
    // set to this value only when we believe all files are uploaded
#define FILE_DELETE_DONE        2
    // means the files were successfully deleted
#define FILE_DELETE_ERROR       3
    // Any error was returned while attempting to delete the file

// values for assimilate_state
#define ASSIMILATE_INIT         0
#define ASSIMILATE_READY        1
#define ASSIMILATE_DONE         2

// NOTE: there is no overall state for a WU (like done/not done)
// There's just a bunch of independent substates
// (file delete, assimilate, and states of results, error flags)

// bit fields of error_mask
//
#define WU_ERROR_COULDNT_SEND_RESULT            1
#define WU_ERROR_TOO_MANY_ERROR_RESULTS         2
#define WU_ERROR_TOO_MANY_SUCCESS_RESULTS       4
#define WU_ERROR_TOO_MANY_TOTAL_RESULTS         8
#define WU_ERROR_CANCELLED                      16
#define WU_ERROR_NO_CANONICAL_RESULT            32

// bit fields of transition_flags; used for assigned jobs
//
#define TRANSITION_NONE             1
    // don't transition; used for broadcast jobs
#define TRANSITION_NO_NEW_RESULTS   2
    // transition, but don't create results; used for targeted jobs

struct WORKUNIT {
    int id;
    int create_time;
    int appid;                  // associated app
    char name[256];
    char xml_doc[BLOB_SIZE];
    int batch;
        // projects can use this for any of several purposes:
        // - group together related jobs so you can use a DB query
        //   to see if they're all done
        // - defer deleting output files (see file_deleter.cpp)
        // - GPUGRID: store the min # of processors needed for the job
        //   (see sched_customize.cpp)
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
    bool need_validate;         // this WU has at least 1 successful result in
                                // validate state = INIT
    int canonical_resultid;     // ID of canonical result, or zero
    double canonical_credit;    // credit that all correct results get
        // TODO: deprecate and remove code
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
    int target_nresults;
        // try to get this many successful results
        // may be > min_quorum to get consensus quicker or reflect loss rate
    int max_error_results;      // WU error if < #error results
    int max_total_results;      // WU error if < #total results
        // (need this in case results are never returned)
    int max_success_results;    // WU error if < #success results
        // without consensus (i.e. WU is nondeterministic)
    char result_template_file[64];
    int priority;
    char mod_time[16];
    double rsc_bandwidth_bound;
        // send only to hosts with at least this much download bandwidth
    int fileset_id;
    int app_version_id;
        // if app uses homogeneous_app_version,
        // which version this job is committed to (0 if none)
    int transitioner_flags;
        // bitmask; see values above

    // the following not used in the DB
    char app_name[256];
    void clear();
};

struct CREDITED_JOB {
    int userid;
    double workunitid;

    // the following not used in the DB
    void clear();
};

// WARNING: be very careful about changing any values,
// especially for a project already running -
// the database will become inconsistent

// values of result.server_state
//
//#define RESULT_SERVER_STATE_INACTIVE       1
#define RESULT_SERVER_STATE_UNSENT         2
#define RESULT_SERVER_STATE_IN_PROGRESS    4
#define RESULT_SERVER_STATE_OVER           5
    // we received a reply, timed out, or decided not to send.
    // Note: we could get a reply even after timing out.

// values of result.outcome
//
#define RESULT_OUTCOME_INIT             0
#define RESULT_OUTCOME_SUCCESS          1
#define RESULT_OUTCOME_COULDNT_SEND     2
#define RESULT_OUTCOME_CLIENT_ERROR     3
    // an error happened on the client
#define RESULT_OUTCOME_NO_REPLY         4
#define RESULT_OUTCOME_DIDNT_NEED       5
    // we created the result but didn't need to send it because
    // 1) we already got a canonical result for the WU, or
    // 2) the WU had an error
#define RESULT_OUTCOME_VALIDATE_ERROR   6
    // The outcome was initially SUCCESS,
    // but the validator had a permanent error reading a result file,
    // or the result file had a syntax error
#define RESULT_OUTCOME_CLIENT_DETACHED  7
    // we believe that the client detached

// values of result.validate_state
//
#define VALIDATE_STATE_INIT         0
#define VALIDATE_STATE_VALID        1
#define VALIDATE_STATE_INVALID      2
#define VALIDATE_STATE_NO_CHECK     3
    // WU had error, so we'll never get around to validating its results
    // This lets us avoid showing the claimed credit as "pending"
#define VALIDATE_STATE_INCONCLUSIVE 4
    // the validator looked this result (as part of a check_set() call)
    // but didn't find a canonical result.
    // This needs to be distinct from INIT for the transitioner to decide
    // whether to trigger the validator
#define VALIDATE_STATE_TOO_LATE     5
    // The result arrived after the canonical result's files were deleted,
    // so we can't determine if it's valid

// values for ASSIGNMENT.target_type
#define ASSIGN_NONE     0
#define ASSIGN_HOST     1
#define ASSIGN_USER     2
#define ASSIGN_TEAM     3

// values for RESULT.app_version_id for anonymous platform
#define ANON_PLATFORM_UNKNOWN -1    // relic of old scheduler
#define ANON_PLATFORM_CPU     -2
#define ANON_PLATFORM_NVIDIA  -3
#define ANON_PLATFORM_ATI     -4
#define ANON_PLATFORM_INTEL   -5

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
    char xml_doc_in[BLOB_SIZE];     // descriptions of output files
    char xml_doc_out[BLOB_SIZE];    // MD5s of output files
    char stderr_out[BLOB_SIZE];     // stderr output, if any
    int batch;
    int file_delete_state;          // see above; values for file_delete_state
    int validate_state;
    double claimed_credit;          // deprecated
    double granted_credit;          // == canonical credit of WU
    double opaque;                  // project-specific; usually external ID
    int random;                     // determines send order
    int app_version_num;            // version# of app (not core client)
        // DEPRECATED - THIS DOESN'T DETERMINE VERSION ANY MORE
    int appid;                      // copy of WU's appid
    int exit_status;                // application exit status, if any
    int teamid;
    int priority;
    char mod_time[16];
    double elapsed_time;
        // AKA runtime; returned by 6.10+ clients
    double flops_estimate;
        // misnomer: actually the peak device FLOPS, returned by app_plan().
    int app_version_id;
        // ID of app version used to compute this
        // 0 if unknown (relic of old scheduler)
        // -1 anon platform, unknown resource type (relic)
        // -2/-3/-4 anonymous platform (see variants above)
    bool runtime_outlier;
        // the validator tagged this as having an unusual elapsed time;
        // don't include it in PFC or elapsed time statistics.

    void clear();
};

struct BATCH {
    int id;
    int user_id;
        // submitter
    int create_time;
    double logical_start_time;
    double logical_end_time;
    double est_completion_time;
        // current estimate of completion time
    int njobs;
        // # of workunits
    double fraction_done;
        // based on workunits completed
    int nerror_jobs;
        // # of workunits with error
    int state;
        // see below
    double completion_time;
        // when state became >= COMPLETE
    double credit_estimate;
        // initial estimate of required credit, counting replicas
    double credit_canonical;
        // the sum of credits of canonical results
    double credit_total;
        // the sum of credits of all results
    char name[256];
        // user-assigned name; need not be unique
    int app_id;
    int project_state;
        // project-assigned
    char description[256];
        // project-assigned
};

// values of batch.state
//
#define BATCH_STATE_INIT            0
#define BATCH_STATE_IN_PROGRESS     1
#define BATCH_STATE_COMPLETE        2
    // "complete" means all workunits have either
    // a canonical result or an error
#define BATCH_STATE_ABORTED         3
#define BATCH_STATE_RETIRED         4
    // input/output files can be deleted,
    // result and workunit records can be purged.

struct MSG_FROM_HOST {
    int id;
    int create_time;
    int hostid;
    char variety[256];              // project-defined; what kind of msg
    bool handled;                   // message handler has processed this
    char xml[MSG_FROM_HOST_BLOB_SIZE];
    void clear();
};

struct MSG_TO_HOST {
    int id;
    int create_time;
    int hostid;
    char variety[256];              // project-defined; what kind of msg
    bool handled;                   // scheduler has sent this
    char xml[MSG_TO_HOST_BLOB_SIZE];      // text to include in sched reply
    void clear();
};

struct ASSIGNMENT {
    int id;
    int create_time;
    int target_id;              // ID of target host, user, or team
    int target_type;            // none/host/user/team
    int multi;                  // 0 = single host, 1 = all hosts in set
    int workunitid;
    int _resultid;              // if not multi, the result ID
        // deprecated
    void clear();
};

struct HOST_APP_VERSION {
    int host_id;
    int app_version_id;
        // or for anon platform:
        // 1000000*appid + 2 (CPU)
        // 1000000*appid + 3 (NVIDIA)
        // 1000000*appid + 4 (ATI)
    AVERAGE pfc;
        // the statistics of (claimed peak FLOPS)/wu.rsc_fpops_est
        // If wu.rsc_fpops_est is accurate,
        // this is roughly the reciprocal of efficiency
    AVERAGE_VAR et;
        // the statistics of (elapsed time)/wu.rsc_fpops_est
        //
        // for old clients (which don't report elapsed time)
        // we use this for CPU time stats
    int max_jobs_per_day;
        // the actual limit is:
        // for GPU versions:
        //   this times config.gpu_multiplier * #GPUs of this type
        // for CPU versions:
        //   this times #CPUs
    int n_jobs_today;
    AVERAGE_VAR turnaround;
        // the stats of turnaround time (received - sent)
        // (NOT normalized by wu.rsc_fpops_est)
    int consecutive_valid;
        // number of consecutive validated relicated results.
        // reset to zero on timeouts, errors, invalid

    void clear();

    // not stored in the DB
    bool reliable;
    bool trusted;
    bool daily_quota_exceeded;
};

struct VDA_FILE {
    int id;
    double create_time;
    char dir[256];
    char file_name[256];
    double size;
    double chunk_size;
    bool need_update;
    bool initialized;
    bool retrieving;
    bool retrieved;
    void clear();
};

struct VDA_CHUNK_HOST {
    double create_time;
    int vda_file_id;
    int host_id;
    char physical_file_name[256];     // e.g. vda_467_0_file.ext
    bool present_on_host;
    bool transfer_in_progress;
    bool transfer_wait;
    double transfer_request_time;
        // when vdad decided to do the transfer
    double transfer_send_time;
        // when transfer request was sent to host

    // the following not in DB
    //
    bool found;

    void clear();
    inline bool download_in_progress() {
        return (transfer_in_progress && !present_on_host);
    }
    void print_status(int level);
};

#endif

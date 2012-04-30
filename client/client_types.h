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

// If you change anything, make sure you also change:
// client_types.C         (to write and parse it)
// client_state.C  (to cross-link objects)
//

#ifndef _CLIENT_TYPES_
#define _CLIENT_TYPES_

#include "cpp.h"

#if !defined(_WIN32) || defined(__CYGWIN32__)
#include <cstdio>
#include <sys/time.h>
#endif

#include "md5_file.h"
#include "cert_sig.h"
#include "hostinfo.h"
#include "coproc.h"
#include "miofile.h"
#include "common_defs.h"
#include "cc_config.h"

#include "rr_sim.h"
#include "work_fetch.h"
#include "cs_notice.h"
#include "cs_trickle.h"

#ifdef SIM
#include "sim.h"
#endif

#define MAX_FILE_INFO_LEN   4096
#define MAX_SIGNATURE_LEN   4096
#define MAX_KEY_LEN         4096

#define MAX_COPROCS_PER_JOB 8
    // max # of instances of a GPU that a job can use

extern int rsc_index(const char*);
extern const char* rsc_name(int);
extern COPROCS coprocs;

struct FILE_INFO;
struct ASYNC_VERIFY;

// represents a list of URLs (e.g. to download a file)
// and a current position in that list
//
struct URL_LIST {
    std::vector<std::string> urls;
    int start_index;
    int current_index;

    URL_LIST(){};

    void clear() {
        urls.clear();
        start_index = -1;
        current_index = -1;
    }
    bool empty() {return urls.empty();}
    const char* get_init_url();
    const char* get_next_url();
    const char* get_current_url(FILE_INFO&);
    inline void add(std::string url) {
        urls.push_back(url);
    }
    void replace(URL_LIST& ul) {
        clear();
        for (unsigned int i=0; i<ul.urls.size(); i++) {
            add(ul.urls[i]);
        }
    }
};

// Values of FILE_INFO::status.
// If the status is neither of these two,
// it's an error code indicating an unrecoverable error
// in the transfer of the file,
// or that the file was too big and was deleted.
//
#define FILE_NOT_PRESENT    0
#define FILE_PRESENT        1
#define FILE_VERIFY_PENDING	2

struct FILE_INFO {
    char name[256];
    char md5_cksum[33];
    double max_nbytes;
    double nbytes;
    double gzipped_nbytes;  // defined if download_gzipped is true
    double upload_offset;
    int status;             // see above
    bool executable;        // change file protections to make executable
    bool uploaded;          // file has been uploaded
    bool sticky;            // don't delete unless instructed to do so
    bool signature_required;    // true iff associated with app version
    bool is_user_file;
    bool is_project_file;
	bool is_auto_update_file;
    bool anonymous_platform_file;
    bool gzip_when_done;
        // for output files: gzip file when done, and append .gz to its name
    class PERS_FILE_XFER* pers_file_xfer;
        // nonzero if in the process of being up/downloaded
    RESULT* result;
        // for upload files (to authenticate)
    PROJECT* project;
    int ref_cnt;
    URL_LIST download_urls;
    URL_LIST upload_urls;
    bool download_gzipped;
        // if set, download NAME.gz and gunzip it to NAME
    char xml_signature[MAX_SIGNATURE_LEN];
        // the upload signature
    char file_signature[MAX_SIGNATURE_LEN];
        // if the file itself is signed (for executable files)
        // this is the signature
    std::string error_msg;
        // if permanent error occurs during file xfer, it's recorded here
    CERT_SIGS* cert_sigs;
    ASYNC_VERIFY* async_verify;

    FILE_INFO();
    ~FILE_INFO();
    void reset();
    int set_permissions(const char* path=0);
    int parse(XML_PARSER&);
    int write(MIOFILE&, bool to_server);
    int write_gui(MIOFILE&);
    int delete_file();
        // attempt to delete the underlying file
    bool had_failure(int& failnum);
    void failure_message(std::string&);
    int merge_info(FILE_INFO&);
    int verify_file(bool, bool, bool);
    bool verify_file_certs();
    int gzip();
        // gzip file and add .gz to name
    int gunzip(char*);
        // unzip file and remove .gz from filename.
        // optionally compute MD5 also
    inline bool uploadable() {
        return !upload_urls.empty();
    }
    inline bool downloadable() {
        return !download_urls.empty();
    }
    inline URL_LIST& get_url_list(bool is_upload) {
        return is_upload?upload_urls:download_urls;
    }
};

// Describes a connection between a file and a workunit, result, or app version
//
struct FILE_REF {
    char file_name[256];
        // physical name
    char open_name[256];
        // logical name
    bool main_program;
    FILE_INFO* file_info;
    bool copy_file;
        // if true, core client will copy the file instead of linking
	bool optional;
		// for output files: app may not generate file;
		// don't treat as error if file is missing.
    int parse(XML_PARSER&);
    int write(MIOFILE&);
};

// file xfer backoff state for a project and direction (up/down)
// if file_xfer_failures exceeds FILE_XFER_FAILURE_LIMIT,
// we switch from a per-file to a project-wide backoff policy
// (separately for the up/down directions)
// NOTE: this refers to transient failures, not permanent.
//
#define FILE_XFER_FAILURE_LIMIT 3
struct FILE_XFER_BACKOFF {
    int file_xfer_failures;
        // count of consecutive failures
    double next_xfer_time;
        // when to start trying again
    bool ok_to_transfer();
    void file_xfer_failed(PROJECT*);
    void file_xfer_succeeded();

    FILE_XFER_BACKOFF() {
        file_xfer_failures = 0;
        next_xfer_time = 0;
    }

    // clear backoff but maintain failure count;
    // called when network becomes available
    //
    void clear_temporary() {
        next_xfer_time = 0;
    }
};

// statistics at a specific day

struct DAILY_STATS {
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    double day;

    void clear();
    DAILY_STATS() {clear();}
    int parse(FILE*);
};
bool operator < (const DAILY_STATS&, const DAILY_STATS&);

// base class for PROJECT and ACCT_MGR_INFO
//
struct PROJ_AM {
    char master_url[256];
    char project_name[256];
        // descriptive.  not unique
    std::vector<RSS_FEED> proj_feeds;
    inline char *get_project_name() {
        if (strlen(project_name)) {
            return project_name;
        } else {
            return master_url;
        }
    }
};

struct PROJECT : PROJ_AM {
    // the following items come from the account file
    // They are a function only of the user and the project
    //
    char authenticator[256];
        // user's authenticator on this project
    std::string project_prefs;
        // without the enclosing <project_preferences> tags.
        // May include <venue> elements
        // This field is used only briefly: between handling a
        // scheduler RPC reply and writing the account file
    std::string project_specific_prefs;
        // without enclosing <project_specific> tags
        // Does not include <venue> elements
    std::string gui_urls;
        // GUI URLs, with enclosing <gui_urls> tags
    double resource_share;
        // project's resource share relative to other projects.
    double resource_share_frac;
        // fraction of RS of non-suspended, compute-intensive projects

    // the following are from the user's project prefs
    //
    bool no_rsc_pref[MAX_RSC];

    // derived from GPU exclusions in cc_config.xml;
    // disable work fetch if all instances excluded
    //
    bool no_rsc_config[MAX_RSC];

    // the following are from the project itself
    // (or derived from app version list if anonymous platform)
    //
    bool no_rsc_apps[MAX_RSC];

    // the following are from the account manager, if any
    //
    bool no_rsc_ams[MAX_RSC];

    // the following set dynamically
    //
    bool rsc_defer_sched[MAX_RSC];
        // This project has a GPU job for which there's insuff. video RAM.
        // Don't fetch more jobs of this type; they might have same problem

    char host_venue[256];
        // logically, this belongs in the client state file
        // rather than the account file.
        // But we need it in the latter in order to parse prefs.
    bool using_venue_specific_prefs;

    // the following items come from client_state.xml
    // They may depend on the host as well as user and project
    // NOTE: if you add anything, add it to copy_state_fields() also!!!
    //
    std::vector<std::string> scheduler_urls;
        // where to find scheduling servers
    char symstore[256];
        // URL of symbol server (Windows)
    char user_name[256];
    char team_name[256];
    char email_hash[MD5_LEN];
    char cross_project_id[MD5_LEN];
    double cpid_time;
    double user_total_credit;
    double user_expavg_credit;
    double user_create_time;
    int userid;
    int teamid;
    int hostid;
    double host_total_credit;
    double host_expavg_credit;
    double host_create_time;
    double ams_resource_share;
        // resource share according to AMS; overrides project
        // -1 means not specified by AMS

    // stuff related to scheduler RPCs and master fetch
    //
    int rpc_seqno;
    int nrpc_failures;
        // # of consecutive times we've failed to contact all scheduling servers
    int master_fetch_failures;
    double min_rpc_time;
        // earliest time to contact any server of this project (or zero)
    void set_min_rpc_time(double future_time, const char* reason);
    double next_rpc_time;
        // if nonzero, specifies a time when another scheduler RPC
        // should be done (as requested by server).
        // An RPC could be done sooner than this.
    bool waiting_until_min_rpc_time();
        // returns true if min_rpc_time > now
    bool master_url_fetch_pending;
        // need to fetch and parse the master URL
    int sched_rpc_pending;
        // we need to do a scheduler RPC, for various possible reasons:
        // user request, propagate host CPID, time-based, etc.
		// Reasons are enumerated in lib/common_defs.h
	bool possibly_backed_off;
        // we need to call request_work_fetch() when a project
        // transitions from being backed off to not.
        // This (slightly misnamed) keeps track of whether this
        // may still need to be done for given project
    bool trickle_up_pending;
        // have trickle up to send
    double last_rpc_time;
        // when last RPC finished
        // not maintained across client sessions
        // used by Manager (simple view)

    // Other stuff

    bool anonymous_platform;
        // app_versions.xml file found in project dir;
        // use those apps rather then getting from server
    bool non_cpu_intensive;
        // All this project's apps are non-CPU-intensive.
        // Apps can also be individually marked as NCI
    bool verify_files_on_app_start;
        // Check app version and input files on app startup,
        // to make sure they haven't been tampered with.
        // This provides only the illusion of security.
    bool use_symlinks;
    double disk_usage;
        // computed by get_disk_usages()
    double disk_share;
        // computed by get_disk_shares();

    // items send in scheduler replies, requesting that
    // various things be sent in the next request
    //
    int send_time_stats_log;
        // if nonzero, send time stats log from that point on
    int send_job_log;
        // if nonzero, send this project's job log from that point on
    bool send_full_workload;
    bool dont_use_dcf;

    bool suspended_via_gui;
    bool dont_request_more_work; 
        // Return work, but don't request more
        // Used for a clean exit to a project,
        // or if a user wants to pause doing work for the project
    bool attached_via_acct_mgr;
    bool detach_when_done;
        // when no results for this project, detach it.
    bool ended;
        // project has ended; advise user to detach
    char code_sign_key[MAX_KEY_LEN];
    std::vector<FILE_REF> user_files;
    std::vector<FILE_REF> project_files;
        // files not specific to apps or work - e.g. icons
    int parse_preferences_for_user_files();
    int parse_project_files(XML_PARSER&, bool delete_existing_symlinks);
    void write_project_files(MIOFILE&);
    void link_project_files(bool recreate_symlink_files);
    int write_symlink_for_project_file(FILE_INFO*);
    double project_files_downloaded_time;
        // when last project file download finished
    void update_project_files_downloaded_time();
        // called when a project file download finishes.
        // If it's the last one, set project_files_downloaded_time to now

    double duration_correction_factor;
        // Multiply by this when estimating the CPU time of a result
        // (based on FLOPs estimated and benchmarks).
        // This is dynamically updated in a way that maintains an upper bound.
        // it goes down slowly but if a new estimate X is larger,
        // the factor is set to X.
        //
        // Deprecated - current server logic handles this,
        // and this should go to 1.
        // But we need to keep it around for older projects
    void update_duration_correction_factor(ACTIVE_TASK*);
    
    // fields used by CPU scheduler and work fetch
    // everything from here on applies only to CPU intensive projects

    bool can_request_work();
        // not suspended and not deferred and not no more work
    bool runnable(int rsc_type);
        // has a runnable result using the given resource type
    bool downloading();
        // has a result in downloading state
    bool potentially_runnable();
        // runnable or contactable or downloading
    bool nearly_runnable();
        // runnable or downloading
    bool overworked();
        // the project has used too much CPU time recently
    bool some_download_stalled();
        // a download is backed off
    bool some_result_suspended();
    double last_upload_start;
        // the last time an upload was started.
        // Used for "work fetch deferral" mechanism:
        // don't request work from a project if an upload started
        // in last X minutes and is still active
    bool uploading();
    bool has_results();

    struct RESULT *next_runnable_result;
        // the next result to run for this project
    int nuploading_results;
        // number of results in UPLOADING state
        // Don't start new results if these exceeds 2*ncpus.
    bool too_many_uploading_results;

    // scheduling (work fetch and job scheduling)
    //
    double sched_priority;
    void compute_sched_priority();

    // stuff for RR sim
    //
    double rr_sim_cpu_share;
    bool rr_sim_active;
    int ncoprocs_excluded[MAX_RSC];
        // number of excluded instances per processor type
    bool operator<(const PROJECT& p) {
        return sched_priority > p.sched_priority;
    }

    // stuff related to work fetch
    //
    RSC_PROJECT_WORK_FETCH rsc_pwf[MAX_RSC];
    PROJECT_WORK_FETCH pwf;
    inline void reset() {
        for (int i=0; i<coprocs.n_rsc; i++) {
            rsc_pwf[i].reset();
        }
    }
    inline int deadlines_missed(int rsc_type) {
        return rsc_pwf[rsc_type].deadlines_missed;
    }
    void get_task_durs(double& not_started_dur, double& in_progress_dur);

    int nresults_returned;
        // # of results being returned in current scheduler op
    const char* get_scheduler_url(int index, double r);
        // get scheduler URL with random offset r
    bool checked;
        // temporary used when scanning projects

    FILE_XFER_BACKOFF download_backoff;
    FILE_XFER_BACKOFF upload_backoff;
    inline FILE_XFER_BACKOFF& file_xfer_backoff(bool is_upload) {
        return is_upload?upload_backoff:download_backoff;
    }

    // support for replicated trickle-ups
    //
    std::vector<TRICKLE_UP_OP*> trickle_up_ops;

    PROJECT();
    ~PROJECT(){}
    void init();
    void copy_state_fields(PROJECT&);
    int write_account_file();
    int parse_account(FILE*);
    int parse_account_file_venue();
    int parse_account_file();
    int parse_state(XML_PARSER&);
    int write_state(MIOFILE&, bool gui_rpc=false);

    // statistic of the last x days
    std::vector<DAILY_STATS> statistics;
    int parse_statistics(MIOFILE&);
    int parse_statistics(FILE*);
    int write_statistics(MIOFILE&, bool gui_rpc=false);
    int write_statistics_file();

    void suspend();
    void resume();
    void abort_not_started();
        // abort unstarted jobs

    // clear AMS-related fields
    inline void detach_ams() {
        attached_via_acct_mgr = false;
        ams_resource_share = -1;
        for (int i=0; i<MAX_RSC; i++) {
            no_rsc_ams[i] = false;
        }
    }

#ifdef SIM
    RANDOM_PROCESS available;
    int index;
    int result_index;
    double idle_time;
    double idle_time_sumsq;
    bool idle;
    int max_infeasible_count;
    bool no_apps;
    // for DCF variants:
    int completed_task_count;
    double completions_ratio_mean;
    double completions_ratio_s;
    double completions_ratio_stdev;
    double completions_required_stdevs;
    PROJECT_RESULTS project_results;
    void print_results(FILE*, SIM_RESULTS&);
    void backoff();
    void update_dcf_stats(RESULT*);
#endif
};

struct APP {
    char name[256];
    char user_friendly_name[256];
    bool non_cpu_intensive;
    PROJECT* project;
#ifdef SIM
    double latency_bound;
    double fpops_est;
    NORMAL_DIST fpops;
    NORMAL_DIST checkpoint_period;
    double working_set;
    double weight;
    bool ignore;
    APP() {memset(this, 0, sizeof(APP));}
#endif

    int parse(XML_PARSER&);
    int write(MIOFILE&);
};

struct GPU_USAGE {
    int rsc_type;
    double usage;
};

struct APP_VERSION {
    char app_name[256];
    int version_num;
    char platform[256];
    char plan_class[64];
    char api_version[16];
    double avg_ncpus;
    double max_ncpus;
    GPU_USAGE gpu_usage;    // can only use 1 GPUtype
    double gpu_ram;
    double flops;
    char cmdline[256];
        // additional cmdline args
    char file_prefix[256];
        // prepend this to input/output file logical names
        // (e.g. "share" for VM apps)
    bool needs_network;

    APP* app;
    PROJECT* project;
    std::vector<FILE_REF> app_files;
    int ref_cnt;
    char graphics_exec_path[512];
    char graphics_exec_file[256];
    double max_working_set_size;
        // max working set of tasks using this app version.
        // unstarted jobs using this app version are assumed
        // to use this much RAM,
        // so that we don't run a long sequence of jobs,
        // each of which turns out not to fit in available RAM
    bool missing_coproc;
    double missing_coproc_usage;
    char missing_coproc_name[256];
    bool dont_throttle;

    int index;  // temp var for make_scheduler_request()
#ifdef SIM
    bool dont_use;
#endif

    APP_VERSION(){}
    ~APP_VERSION(){}
    int parse(XML_PARSER&);
    int write(MIOFILE&, bool write_file_info = true);
    bool had_download_failure(int& failnum);
    void get_file_errors(std::string&);
    void clear_errors();
    int api_major_version();
    inline bool uses_coproc(int rt) {
        return (gpu_usage.rsc_type == rt);
    }
    inline int rsc_type() {
        return gpu_usage.rsc_type;
    }
};

struct WORKUNIT {
    char name[256];
    char app_name[256];
    int version_num;
        // Deprecated, but need to keep around to let people revert
        // to versions before multi-platform support
    std::string command_line;
    std::vector<FILE_REF> input_files;
    PROJECT* project;
    APP* app;
    int ref_cnt;
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;

    WORKUNIT(){}
    ~WORKUNIT(){}
    int parse(XML_PARSER&);
    int write(MIOFILE&);
    bool had_download_failure(int& failnum);
    void get_file_errors(std::string&);
    void clear_errors();
};

struct RESULT {
    char name[256];
    char wu_name[256];
    double received_time;   // when we got this from server
    double report_deadline;
    int version_num;        // identifies the app used
    char plan_class[64];
    char platform[256];
    APP_VERSION* avp;
    std::vector<FILE_REF> output_files;
    bool ready_to_report;
        // we're ready to report this result to the server;
        // either computation is done and all the files have been uploaded
        // or there was an error
    double completed_time;
        // time when ready_to_report was set
    bool got_server_ack;
        // we've received the ack for this result from the server
    double final_cpu_time;
    double final_elapsed_time;
#ifdef SIM
    double peak_flop_count;
    double sim_flops_left;
#endif

    // the following are nonzero if reported by app
    double fpops_per_cpu_sec;
    double fpops_cumulative;
    double intops_per_cpu_sec;
    double intops_cumulative;

    int _state;
        // state of this result: see lib/result_state.h
    inline int state() { return _state; }
    inline void set_ready_to_report() {
        ready_to_report = true;
    }
    void set_state(int, const char*);
    int exit_status;
        // return value from the application
    std::string stderr_out;
        // the concatenation of:
        //
        // - if report_result_error() is called for this result:
        //   <message>x</message>
        //   <exit_status>x</exit_status>
        //   <signal>x</signal>
        //   - if called in FILES_DOWNLOADED state:
        //     <couldnt_start>x</couldnt_start>
        //   - if called in NEW state:
        //     <download_error>x</download_error> for each failed download
        //   - if called in COMPUTE_DONE state:
        //     <upload_error>x</upload_error> for each failed upload
        //
        // - <stderr_txt>X</stderr_txt>, where X is the app's stderr output
    bool suspended_via_gui;
    bool coproc_missing;
        // a coproc needed by this job is missing
        // (e.g. because user removed their GPU board).
    bool report_immediately;
    bool not_started;   // temp for CPU sched

    std::string name_md5;   // see sort_results();
    int index;              // index in results vector

    APP* app;
    WORKUNIT* wup;
    PROJECT* project;

    RESULT(){}
    ~RESULT(){}
    void clear();
    int parse_server(XML_PARSER&);
    int parse_state(XML_PARSER&);
    int parse_name(XML_PARSER&, const char* end_tag);
    int write(MIOFILE&, bool to_server);
    int write_gui(MIOFILE&);
    bool is_upload_done();    // files uploaded?
    void clear_uploaded_flags();
    FILE_REF* lookup_file(FILE_INFO*);
    FILE_INFO* lookup_file_logical(const char*);
    void abort_inactive(int);
        // abort the result if it hasn't started computing yet
        // Called only for results with no active task
        // (otherwise you need to abort the active task)
    void append_log_record();

    // stuff related to CPU scheduling

    bool is_not_started();
    double estimated_duration();
    double estimated_duration_uncorrected();
    double estimated_time_remaining();
    inline double estimated_flops_remaining() {
#ifdef SIM
        return sim_flops_left;
#else
        return estimated_time_remaining()*avp->flops;
#endif
    }

    inline bool computing_done() {
        if (state() >= RESULT_COMPUTE_ERROR) return true; 
        if (ready_to_report) return true;
        return false;
    }
    bool runnable();
        // downloaded, not finished, not suspended, project not suspended
    bool nearly_runnable();
        // downloading or downloaded,
        // not finished, suspended, project not suspended
    bool downloading();
        // downloading, not downloaded, not suspended, project not suspended
    bool some_download_stalled();
        // some input or app file is downloading, and backed off
        // i.e. it may be a long time before we can run this result
    inline bool uses_coprocs() {
        return (avp->gpu_usage.rsc_type != 0);
    }
    inline int resource_type() {
        return avp->gpu_usage.rsc_type;
    }
    inline bool non_cpu_intensive() {
        if (project->non_cpu_intensive) return true;
        if (app->non_cpu_intensive) return true;
        return false;
    }
    inline bool dont_throttle() {
        if (non_cpu_intensive()) return true;
        if (avp->dont_throttle) return true;
        return false;
    }

    // temporaries used in CLIENT_STATE::rr_simulation():
    double rrsim_flops_left;
    double rrsim_finish_delay;
    double rrsim_flops;
    bool rrsim_done;

    bool already_selected;
        // used to keep cpu scheduler from scheduling a result twice
        // transient; used only within schedule_cpus()
    double computation_deadline();
        // report deadline - prefs.work_buf_min - time slice
    bool rr_sim_misses_deadline;

    // temporaries used in enforce_schedule():
    bool unfinished_time_slice;
    int seqno;

    bool edf_scheduled;
        // temporary used to tell GUI that this result is deadline-scheduled

    int coproc_indices[MAX_COPROCS_PER_JOB];
        // keep track of coprocessor reservations
    char resources[256];
        // textual description of resources used
    double schedule_backoff;
        // don't try to schedule until this time
        // (wait for free GPU RAM)
    char schedule_backoff_reason[256];
};

// represents an always/auto/never value, possibly temporarily overridden

struct RUN_MODE {
    int perm_mode;
    int temp_mode;
    int prev_mode;
    double temp_timeout;
    RUN_MODE();
    void set(int mode, double duration);
    void set_prev(int mode);
    int get_perm();
    int get_prev();
    int get_current();
	double delay();
};

// a platform supported by the client.

struct PLATFORM {
    std::string name;
};

#endif

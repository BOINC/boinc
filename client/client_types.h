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

#include "rr_sim.h"
#include "work_fetch.h"
#include "cs_notice.h"

#define MAX_FILE_INFO_LEN   4096
#define MAX_SIGNATURE_LEN   4096
#define MAX_KEY_LEN         4096

#define MAX_COPROCS_PER_JOB 8

// If the status is neither of these two,
// it will be an error code defined in error_numbers.h,
// indicating an unrecoverable error in the upload or download of the file,
// or that the file was too big and was deleted
//
#define FILE_NOT_PRESENT    0
#define FILE_PRESENT        1

struct FILE_INFO {
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
    bool report_on_rpc;     // include this in each scheduler request
    bool marked_for_delete;     // server requested delete;
        // if not in use, delete even if sticky is true
        // don't report to server even if report_on_rpc is true
    bool signature_required;    // true iff associated with app version
    bool is_user_file;
    bool is_project_file;
	bool is_auto_update_file;
    bool gzip_when_done;
        // for output files: gzip file when done, and append .gz to its name
    class PERS_FILE_XFER* pers_file_xfer;
        // nonzero if in the process of being up/downloaded
    RESULT* result;
        // for upload files (to authenticate)
    PROJECT* project;
    int ref_cnt;
    std::vector<std::string> urls;
    int start_url;
    int current_url;
    char signed_xml[MAX_FILE_INFO_LEN];
        // if the file_info is signed (for uploadable files)
        // this is the text that is signed
        // Otherwise it is the FILE_INFO's XML descriptor
        // (without enclosing <file_info> tags)
    char xml_signature[MAX_SIGNATURE_LEN];
        // ... and this is the signature
    char file_signature[MAX_SIGNATURE_LEN];
        // if the file itself is signed (for executable files)
        // this is the signature
    std::string error_msg;
        // if permanent error occurs during file xfer, it's recorded here
    CERT_SIGS* cert_sigs;

    FILE_INFO();
    ~FILE_INFO();
    void reset();
    int set_permissions();
    int parse(MIOFILE&, bool from_server);
    int write(MIOFILE&, bool to_server);
    int write_gui(MIOFILE&);
    int delete_file();
        // attempt to delete the underlying file
    const char* get_init_url();
    const char* get_next_url();
    const char* get_current_url();
    bool had_failure(int& failnum);
    void failure_message(std::string&);
    int merge_info(FILE_INFO&);
    int verify_file(bool, bool);
    bool verify_file_certs();
    int gzip();
        // gzip file and add .gz to name
};

// Describes a connection between a file and a workunit, result, or application

// In the first two cases,
// the app will either use open() or fopen() to access the file
// (in which case "open_name" is the name it will use)
// or the app will be connected by the given fd (in which case fd is nonzero)

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
    int parse(MIOFILE&);
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

struct PROJECT {
    // the following items come from the account file
    // They are a function only of the user and the project
    //
    char master_url[256];
        // url of site that contains scheduler tags for this project
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

        // the following are the user's project prefs
    bool no_cpu_pref;
    bool no_cuda_pref;
    bool no_ati_pref;

        // the following are from the project itself
    bool no_cpu_apps;
    bool no_cuda_apps;
    bool no_ati_apps;

        // the following set dynamically
    bool cuda_defer_sched;
        // This project has a CUDA job for which there's insuff. video RAM.
        // Don't fetch more CUDA jobs; they might have same problem
    bool ati_defer_sched;
        // same, ATI

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
    char project_name[256];
        // descriptive.  not unique
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
		// Reasons are enumerated in scheduler_op.h
	bool possibly_backed_off;
        // we need to call request_work_fetch() when a project
        // transitions from being backed off to not.
        // This (slightly misnamed) keeps track of whether this
        // may still need to be done for given project
    bool trickle_up_pending;
        // have trickle up to send
    double last_rpc_time;
        // when last RPC finished

    // Other stuff

    bool anonymous_platform;
        // app_versions.xml file found in project dir;
        // use those apps rather then getting from server
    bool non_cpu_intensive;
    bool verify_files_on_app_start;
    bool use_symlinks;

    // items send in scheduler replies, requesting that
    // various things be sent in the next request
    //
    bool send_file_list;
        // send the list of permanent files associated with the project
        // in the next scheduler reply
    int send_time_stats_log;
        // if nonzero, send time stats log from that point on
    int send_job_log;
        // if nonzero, send this project's job log from that point on
    bool send_full_workload;

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
    int parse_project_files(MIOFILE&, bool delete_existing_symlinks);
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

    RR_SIM_PROJECT_STATUS rr_sim_status;
        // temps used in CLIENT_STATE::rr_simulation();

    struct RESULT *next_runnable_result;
        // the next result to run for this project
    int nuploading_results;
        // number of results in UPLOADING state
        // Don't start new results if these exceeds 2*ncpus.
    bool too_many_uploading_results;

    // stuff related to work fetch
    //
    RSC_PROJECT_WORK_FETCH cpu_pwf;
    RSC_PROJECT_WORK_FETCH cuda_pwf;
    RSC_PROJECT_WORK_FETCH ati_pwf;
    PROJECT_WORK_FETCH pwf;
    inline void reset() {
        cpu_pwf.reset();
        cuda_pwf.reset();
        ati_pwf.reset();
    }
    inline int deadlines_missed(int rsc_type) {
        switch(rsc_type) {
        case RSC_TYPE_CUDA: return cuda_pwf.deadlines_missed;
        case RSC_TYPE_ATI: return ati_pwf.deadlines_missed;
        }
        return cpu_pwf.deadlines_missed;
    }

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

    PROJECT();
    ~PROJECT(){}
    void init();
    void copy_state_fields(PROJECT&);
    char *get_project_name();
    int write_account_file();
    int parse_account(FILE*);
    int parse_account_file_venue();
    int parse_account_file();
    int parse_state(MIOFILE&);
    int write_state(MIOFILE&, bool gui_rpc=false);

    // statistic of the last x days
    std::vector<DAILY_STATS> statistics;
    int parse_statistics(MIOFILE&);
    int parse_statistics(FILE*);
    int write_statistics(MIOFILE&, bool gui_rpc=false);
    int write_statistics_file();

    // feed-related
    std::vector<RSS_FEED> proj_feeds;
};

struct APP {
    char name[256];
    char user_friendly_name[256];
    PROJECT* project;

    int parse(MIOFILE&);
    int write(MIOFILE&);
};

struct APP_VERSION {
    char app_name[256];
    int version_num;
    char platform[256];
    char plan_class[64];
    char api_version[16];
    double avg_ncpus;
    double max_ncpus;
    double ncudas;
    double natis;
    double gpu_ram;
    double flops;
    char cmdline[256];
        // additional cmdline args

    APP* app;
    PROJECT* project;
    std::vector<FILE_REF> app_files;
    int ref_cnt;
    char graphics_exec_path[512];
    char graphics_exec_file[256];
    double max_working_set_size;
        // max working set of tasks using this app version.

    int index;  // temp var for make_scheduler_request()

    APP_VERSION(){}
    ~APP_VERSION(){}
    int parse(MIOFILE&);
    int write(MIOFILE&, bool write_file_info = true);
    bool had_download_failure(int& failnum);
    void get_file_errors(std::string&);
    void clear_errors();
    int api_major_version();
    bool missing_coproc();
    inline bool uses_coproc(int rsc_type) {
        switch (rsc_type) {
        case RSC_TYPE_CUDA: return (ncudas>0);
        case RSC_TYPE_ATI: return (natis>0);
        }
        return false;
    }
    inline int rsc_type() {
        if (ncudas>0) return RSC_TYPE_CUDA;
        if (natis>0) return RSC_TYPE_ATI;
        return RSC_TYPE_CPU;
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
    int parse(MIOFILE&);
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

    // the following are nonzero if reported by app
    double fpops_per_cpu_sec;
    double fpops_cumulative;
    double intops_per_cpu_sec;
    double intops_cumulative;

    int _state;
        // state of this result: see lib/result_state.h
    inline int state() { return _state; }
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

    APP* app;
    WORKUNIT* wup;
        // this may be NULL after result is finished
    PROJECT* project;

    RESULT(){}
    ~RESULT(){}
    void clear();
    int parse_server(MIOFILE&);
    int parse_state(MIOFILE&);
    int parse_name(FILE*, const char* end_tag);
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

    double estimated_duration(bool for_work_fetch);
    double estimated_duration_uncorrected();
    double estimated_time_remaining(bool for_work_fetch);
    inline double estimated_flops_remaining() {
        return estimated_time_remaining(false)*avp->flops;
    }

    inline bool computing_done() {
        if (state() >= RESULT_COMPUTE_ERROR) return true; 
        if (ready_to_report) return true;
        return false;
    }
    bool not_started();
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
    inline bool uses_cuda() {
        return (avp->ncudas > 0);
    }
    inline bool uses_ati() {
        return (avp->natis > 0);
    }
    inline bool uses_coprocs() {
        if (avp->ncudas > 0) return true;
        if (avp->natis > 0) return true;
        return false;
    }

    // temporaries used in CLIENT_STATE::rr_simulation():
    double rrsim_flops_left;
    double rrsim_finish_delay;
    double rrsim_flops;

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
};

// represents an always/auto/never value, possibly temporarily overridden

class MODE {
private:
    int perm_mode;
    int temp_mode;
    double temp_timeout;
public:
    MODE();
    void set(int mode, double duration);
    int get_perm();
    int get_current();
	double delay();
};

// a platform supported by the client.

class PLATFORM {
public:
    std::string name;
};

#endif

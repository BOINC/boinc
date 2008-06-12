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

// If you change anything, make sure you also change:
// client_types.C         (to write and parse it)
// client_state.C  (to cross-link objects)
//

#ifndef _CLIENT_TYPES_
#define _CLIENT_TYPES_

#include "cpp.h"

#if !defined(_WIN32) || defined(__CYGWIN32__)
#include <stdio.h>
#include <sys/time.h>
#endif

#include "md5_file.h"
#include "hostinfo.h"
#include "coproc.h"
#include "miofile.h"

#define P_LOW 1
#define P_MEDIUM 3
#define P_HIGH 5

#define MAX_FILE_INFO_LEN   4096
#define MAX_SIGNATURE_LEN   4096
#define MAX_KEY_LEN         4096

// If the status is neither of these two,
// it will be an error code defined in error_numbers.h,
// indicating an unrecoverable error in the upload or download of the file,
// or that the file was too big and was deleted
//
#define FILE_NOT_PRESENT    0
#define FILE_PRESENT        1

class FILE_INFO {
public:
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
    struct RESULT* result;         // for upload files (to authenticate)
    class PROJECT* project;
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
    std::string error_msg;       // if permanent error occurs during file xfer,
                            // it's recorded here

    FILE_INFO();
    ~FILE_INFO();
    void reset();
    int set_permissions();
    int parse(MIOFILE&, bool from_server);
    int write(MIOFILE&, bool to_server);
    int write_gui(MIOFILE&);
    int delete_file();      // attempt to delete the underlying file
    const char* get_init_url(bool);
    const char* get_next_url(bool);
    const char* get_current_url(bool);
    bool is_correct_url_type(bool, std::string&);
    bool had_failure(int& failnum);
    void failure_message(std::string&);
    int merge_info(FILE_INFO&);
    int verify_file(bool, bool);
    int gzip();     // gzip file and add .gz to name
};

// Describes a connection between a file and a workunit, result, or application.
// In the first two cases,
// the app will either use open() or fopen() to access the file
// (in which case "open_name" is the name it will use)
// or the app will be connected by the given fd (in which case fd is nonzero)
//
struct FILE_REF {
    char file_name[256];    // physical name
    char open_name[256];    // logical name
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

// statistics at a specific day
//
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

struct RR_SIM_PROJECT_STATUS {
    std::vector<RESULT*>active;
        // jobs currently running (in simulation)
    std::vector<RESULT*>pending;
        // jobs runnable but not running yet
    int deadlines_missed;
    double proc_rate;
        // fraction of each CPU this project will get
        // set in CLIENT_STATE::rr_misses_deadline();
    double cpu_shortfall;

    inline void clear() {
        active.clear();
        pending.clear();
        deadlines_missed = 0;
        proc_rate = 0;
        cpu_shortfall = 0;
    }
    inline void activate(RESULT* rp) {
        active.push_back(rp);
    }
    inline void add_pending(RESULT* rp) {
        pending.push_back(rp);
    }
    inline bool none_active() {
        return !active.size();
    }
    inline bool can_run(RESULT*, int ncpus) {
        return (int)active.size() < ncpus;
    }
    inline void remove_active(RESULT* r) {
        std::vector<RESULT*>::iterator it = active.begin();
        while (it != active.end()) {
            if (*it == r) {
                it = active.erase(it);
            } else {
                it++;
            }
        }
    }
    inline RESULT* get_pending() {
        if (!pending.size()) return NULL;
        RESULT* rp = pending[0];
        pending.erase(pending.begin());
        return rp;
    }
    inline int cpus_used() {
        return (int) active.size();
    }
};

class PROJECT {
public:
    // the following items come from the account file
    // They are a function only of the user and the project
    //
    char master_url[256];       // url of site that contains scheduler tags
                                // for this project
    char authenticator[256];    // user's authenticator on this project
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
    char project_name[256];             // descriptive.  not unique
    char symstore[256];             // URL of symbol server (Windows)
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

    // stuff related to scheduler RPCs and master fetch
    //
    int rpc_seqno;
    int nrpc_failures;          // # of consecutive times we've failed to
                                // contact all scheduling servers
    int master_fetch_failures;
    double min_rpc_time;           // earliest time to contact any server
                                   // of this project (or zero)
    void set_min_rpc_time(double future_time, const char* reason);
    bool waiting_until_min_rpc_time();
        // returns true if min_rpc_time > now
    bool master_url_fetch_pending;
                                // need to fetch and parse the master URL
    int sched_rpc_pending;
        // we need to do a scheduler RPC, for various possible reasons:
        // user request, propagate host CPID, time-based, etc.
		// Reasons are enumerated in scheduler_op.h
    double next_rpc_time;
        // if nonzero, specifies a time when another scheduler RPC
        // should be done (as requested by server)
	bool possibly_backed_off;
        // we need to call request_work_fetch() when a project
        // transitions from being backed off to not.
        // This (slightly misnamed) keeps track of whether this
        // may still need to be done for given project
    bool trickle_up_pending;    // have trickle up to send
    double last_rpc_time;          // when last RPC finished

    // Other stuff

    bool anonymous_platform;    // app_versions.xml file found in project dir;
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
        //
    void update_duration_correction_factor(RESULT*);
    
    // fields used by CPU scheduler and work fetch
    // everything from here on applies only to CPU intensive projects

    bool contactable();
        // not suspended and not deferred and not no more work
    bool runnable();
        // has a runnable result
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
    void set_rrsim_proc_rate(double rrs);

    int deadlines_missed;   // used as scratch by scheduler, enforcer

    // "debt" is how much CPU time we owe this project relative to others

    double short_term_debt;
        // computed over runnable projects
        // used for CPU scheduling
	double long_term_debt;
        // Computed over potentially runnable projects
        // (defined for all projects, but doesn't change if
        // not potentially runnable).
        // Normalized so mean over all projects is zero

    double anticipated_debt;
        // expected debt by the end of the preemption period
    double wall_cpu_time_this_debt_interval;
        // how much "wall CPU time" has been devoted to this
        // project in the current debt interval
    struct RESULT *next_runnable_result;
        // the next result to run for this project
    int nuploading_results;
        // number of results in UPLOADING state
        // Don't start new results if these exceeds 2*ncpus.

    double work_request;
        // the unit is "project-normalized CPU seconds",
        // i.e. the work should take 1 CPU on this host
        // X seconds of wall-clock time to complete,
        // taking into account
        // 1) this project's fractional resource share
        // 2) on_frac, active_frac, and cpu_effiency
        // see doc/sched.php
    int work_request_urgency;

    int nresults_returned;
        // # of results being returned in current scheduler op
    const char* get_scheduler_url(int index, double r);
        // get scheduler URL with random offset r
    bool checked;
        // temporary used when scanning projects

    // vars related to file-transfer backoff
    // file_xfer_failures_up: count of consecutive upload failures
    // next_file_xfer_up: when to start trying uploads again
    //
    // if file_xfer_failures_up exceeds FILE_XFER_FAILURE_LIMIT,
    // we switch from a per-file to a project-wide backoff policy
    // (separately for the up/down directions)
    //
    // NOTE: all this refers to transient failures, not permanent.
    // Also, none of this is used right now (commented out)
    //
#define FILE_XFER_FAILURE_LIMIT 3
    int file_xfer_failures_up;
    int file_xfer_failures_down;
    double next_file_xfer_up;
    double next_file_xfer_down;

    double next_file_xfer_time(const bool);
    void file_xfer_failed(const bool);
    void file_xfer_succeeded(const bool);

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
    double flops;
    char cmdline[256];      // additional cmdline args
    COPROCS coprocs;

    APP* app;
    PROJECT* project;
    std::vector<FILE_REF> app_files;
    int ref_cnt;
    char graphics_exec_path[512];

    APP_VERSION(){}
    ~APP_VERSION(){}
    int parse(MIOFILE&);
    int write(MIOFILE&);
    bool had_download_failure(int& failnum);
    void get_file_errors(std::string&);
    void clear_errors();
    int api_major_version();
};

struct WORKUNIT {
    char name[256];
    char app_name[256];
    int version_num;
        // Deprecated, but need to keep around to let people revert
        // to versions before multi-platform support
    std::string command_line;
    //char env_vars[256];         // environment vars in URL format
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
        // we're received the ack for this result from the server
    double final_cpu_time;
    double fpops_per_cpu_sec;   // nonzero if reported by app
    double fpops_cumulative;    // nonzero if reported by app
    double intops_per_cpu_sec;   // nonzero if reported by app
    double intops_cumulative;    // nonzero if reported by app
    int _state;                  // state of this result: see lib/result_state.h
    inline int state() { return _state; }
    void set_state(int, const char*);
    int exit_status;            // return value from the application
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

    double estimated_cpu_time(bool for_work_fetch);
    double estimated_cpu_time_uncorrected();
    double estimated_cpu_time_remaining(bool for_work_fetch);
    bool computing_done();
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

    // temporaries used in CLIENT_STATE::rr_simulation():
    double rrsim_cpu_left;
    double rrsim_finish_delay;
    bool already_selected;
        // used to keep cpu scheduler from scheduling a result twice
        // transient; used only within schedule_cpus()
    double computation_deadline();
        // report deadline - prefs.work_buf_min - time slice
    bool rr_sim_misses_deadline;
    bool last_rr_sim_missed_deadline;

    // temporary used to tell GUI that this result is deadline-scheduled
    bool edf_scheduled;
};

// represents an always/auto/never value, possibly temporarily overridden
//
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
//
class PLATFORM {
public:
    std::string name;
};

#endif

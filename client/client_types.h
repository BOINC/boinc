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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// If you change anything, make sure you also change:
// client_types.C         (to write and parse it)
// client_state.C  (to cross-link objects)
//

#ifndef _CLIENT_TYPES_
#define _CLIENT_TYPES_

#include "cpp.h"

#ifndef _WIN32
#include <stdio.h>
#include <sys/time.h>
#endif

#include "md5_file.h"
#include "hostinfo.h"
#include "miofile.h"
#include "result_state.h"

#define MAX_BLOB_LEN 4096
#define P_LOW 1
#define P_MEDIUM 3
#define P_HIGH 5

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
    class PERS_FILE_XFER* pers_file_xfer;   // nonzero if in the process of being up/downloaded
    struct RESULT* result;         // for upload files (to authenticate)
    class PROJECT* project;
    int ref_cnt;
    std::vector<std::string> urls;
    int start_url;
    int current_url;
    char signed_xml[MAX_BLOB_LEN];
        // if the file_info is signed (for uploadable files)
        // this is the text that is signed
        // Otherwise it is the FILE_INFO's XML descriptor
        // (without enclosing <file_info> tags)
    char xml_signature[MAX_BLOB_LEN];
        // ... and this is the signature
    char file_signature[MAX_BLOB_LEN];
        // if the file itself is signed (for executable files)
        // this is the signature
#if 0
    int priority;
    double time_last_used;         // time of last use of FILE_INFO, update during parsing, writing, or application usage
    double exp_date;
#endif
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
    bool had_failure(int& failnum, char* buf=0);
    int merge_info(FILE_INFO&);
    int verify_file(bool);
    int update_time();       // updates time last used to the current time
};

// Describes a connection between a file and a workunit, result, or application.
// In the first two cases,
// the app will either use open() or fopen() to access the file
// (in which case "open_name" is the name it will use)
// or the app will be connected by the given fd (in which case fd is nonzero)
//
struct FILE_REF {
    char file_name[256];
    char open_name[256];
    int fd;
    bool main_program;
    FILE_INFO* file_info;
    bool copy_file;  // if true, core client will copy the file instead of linking

    int parse(MIOFILE&);
    int write(MIOFILE&);
};

// statistics at a specific day
//
struct STATISTIC {
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    double day;
};

// reasons for attach failure
#define ATTACH_FAIL_INIT       1
#define ATTACH_FAIL_DOWNLOAD   2
#define ATTACH_FAIL_PARSE      3
#define ATTACH_FAIL_BAD_KEY    4
#define ATTACH_FAIL_FILE_WRITE 5

class PROJECT {
public:
    // the following items come from the account file
    // They are a function only of the user and the project
    //
    char master_url[256];       // url of site that contains scheduler tags
                                // for this project
    char authenticator[256];    // user's authenticator on this project
#if 0
                                        // deletion policy, least recently used
    bool deletion_policy_priority;       // deletion policy, priority of files
    bool deletion_policy_expire;         // deletion policy, delete expired files first
    double share_size;          // size allocated by the resource share
                                // used for enforcement of boundaries but isn't one itself
    double size;                // the total size of all the files in all subfolder
                                // of the project
#endif
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
    char user_name[256];
    char team_name[256];
    char email_hash[MD5_LEN];
    char cross_project_id[MD5_LEN];
    double user_total_credit;      // as reported by server
    double user_expavg_credit;     // as reported by server
    double user_create_time;       // as reported by server
    int rpc_seqno;
    int hostid;
    double host_total_credit;      // as reported by server
    double host_expavg_credit;     // as reported by server
    double host_create_time;       // as reported by server
    double exp_avg_cpu;            // exponentially weighted CPU time
    double exp_avg_mod_time;       // last time average was changed
    int nrpc_failures;          // # of consecutive times we've failed to
                                // contact all scheduling servers
    int master_fetch_failures;
    double min_rpc_time;           // earliest time to contact any server
                                  // of this project (or zero)
    double min_report_min_rpc_time; // when to next report on min_rpc_time
                                    // (or zero)
    bool master_url_fetch_pending;
                                // need to fetch and parse the master URL
    bool sched_rpc_pending;     // contact scheduling server for preferences
    bool tentative;             // master URL and account ID not confirmed
    bool anonymous_platform;    // app_versions.xml file found in project dir;
                                // use those apps rather then getting from server
    bool non_cpu_intensive;
    bool send_file_list;
        // send the list of permanent files associated/with the project
        // in the next scheduler reply
    bool suspended_via_gui;
    bool dont_request_more_work; 
        // set the project to only return work and not request more
        // for a clean exit to a project, or if a user wants to 
        // pause doing work for the project
    char code_sign_key[MAX_BLOB_LEN];
    std::vector<FILE_REF> user_files;
    int parse_preferences_for_user_files();
    
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
    double wall_cpu_time_this_period;
        // how much "wall CPU time" has been devoted to this
        // project in the current scheduling period (secs)
    struct RESULT *next_runnable_result;
        // the next result to run for this project

    double work_request;
        // the unit is "normalized CPU seconds",
        // i.e. the work should take 1 CPU on this host
        // X seconds of wall-clock time to complete,
        // taking into account
        // 1) other projects and resource share;
        // 2) on_frac and active_frac
        // see doc/work_req.php
    int work_request_urgency;

    int nresults_returned;
        // # of results being returned in current scheduler op
    const char* get_scheduler_url(int index, double r);
        // get scheduler URL with random offset r

#if 0
    // used in disk-space management (temp)
    bool checked;
#endif

    PROJECT();
    ~PROJECT();
    void init();
    void copy_state_fields(PROJECT&);
    char *get_project_name();
    int write_account_file();
    int parse_account(FILE*);
    int parse_account_file();
    int parse_state(MIOFILE&);
    int write_state(MIOFILE&, bool gui_rpc=false);
    void attach_failed(int reason);
#if 0
    bool associate_file(FILE_INFO*);
#endif

    // set min_rpc_time and have_reported_min_rpc_time
    void set_min_rpc_time(double future_time);
    // returns true if min_rpc_time > now; may print a message
    bool waiting_until_min_rpc_time();

    // statistic of the last x days
    std::vector<STATISTIC> statistics;
    int parse_statistics(MIOFILE&);
    int parse_statistics(FILE*);
    int write_statistics(MIOFILE&, bool gui_rpc=false);
    int write_statistics_file();
};

struct APP {
    char name[256];
    PROJECT* project;

    int parse(MIOFILE&);
    int write(MIOFILE&);
};

struct APP_VERSION {
    char app_name[256];
    int version_num;
    APP* app;
    PROJECT* project;
    std::vector<FILE_REF> app_files;
    int ref_cnt;

    int parse(MIOFILE&);
    int write(MIOFILE&);
    bool had_download_failure(int& failnum);
    void get_file_errors(std::string&);
    void clear_errors();
};

struct WORKUNIT {
    char name[256];
    char app_name[256];
    int version_num;
        // This isn't sent from the server.
        // Instead, the client picks the latest app version
    std::string command_line;
    //char env_vars[256];         // environment vars in URL format
    std::vector<FILE_REF> input_files;
    PROJECT* project;
    APP* app;
    APP_VERSION* avp;
    int ref_cnt;
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;

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
    int state;              // state of this result: see lib/result_state.h
    int exit_status;        // return value from the application
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
    bool aborted_via_gui;

    APP* app;
    WORKUNIT* wup;
        // this may be NULL after result is finished
    PROJECT* project;

    bool already_selected;
        // used to keep cpu scheduler from scheduling a result twice
        // transient; used only within schedule_cpus()
    void clear();
    int parse_server(MIOFILE&);
    int parse_state(MIOFILE&);
    int parse_ack(FILE*);
    int write(MIOFILE&, bool to_server);
    int write_gui(MIOFILE&);
    bool is_upload_done();    // files uploaded?
    void get_app_version_string(std::string&);
    void reset_files();
    FILE_REF* lookup_file(FILE_INFO*);
    FILE_INFO* lookup_file_logical(const char*);
    double estimated_cpu_time();
    double estimated_cpu_time_remaining();
    bool computing_done();
    bool runnable();
        // downloaded, not finished, not suspended, project not suspended
    bool runnable_soon();
        // downloading or downloaded,
        // not finished, suspended, project not suspended
};

#endif

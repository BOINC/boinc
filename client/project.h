// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

#ifndef BOINC_PROJECT_H
#define BOINC_PROJECT_H

#include "app_config.h"
#include "client_types.h"

// describes a project to which this client is attached
//
struct PROJECT : PROJ_AM {
    char _project_dir[MAXPATHLEN];
    char _project_dir_absolute[MAXPATHLEN];

    // the following items come from the account file
    // They are a function of the user and the project (not host)
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
        // temp; fraction of RS of non-suspended, compute-intensive projects
    double disk_resource_share;
        // temp in get_disk_shares()
    double desired_disk_usage;
        // reported by project
    double ddu;
        // temp in get_disk_shares()
    double disk_quota;
        // temp in get_disk_shares()

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

    char host_venue[256];
        // logically, this belongs in the client state file
        // rather than the account file.
        // But we need it in the latter in order to parse prefs.
    bool using_venue_specific_prefs;

    ///////  START OF ITEMS STORED IN client_state.xml
    //
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
        // the "internal" user CPID
    char external_cpid[MD5_LEN];
        // the "external" user CPID (as exported to stats sites)
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
        // -1 means not specified by AMS, or not using an AMS
    double last_rpc_time;
        // when last RPC finished; used by Manager
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

    // accounting info; estimated credit and time for CPU and GPU
    //
    double cpu_ec;
    double cpu_time;
    double gpu_ec;
    double gpu_time;

    // stuff related to scheduler RPCs and master fetch
    //
    int rpc_seqno;
    int nrpc_failures;
        // # of consecutive times we've failed to contact all scheduling servers
    int master_fetch_failures;
    double min_rpc_time;
        // earliest time to contact any server of this project (or zero)
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
    bool trickle_up_pending;
        // have trickle up to send
    double disk_usage;
        // computed by get_disk_usages()
    double disk_share;
        // computed by get_disk_shares();

    ///////  END OF ITEMS STORED IN client_state.xml

    // Other stuff
    //
    bool possibly_backed_off;
        // we need to call request_work_fetch() when a project
        // transitions from being backed off to not.
        // This (slightly misnamed) keeps track of whether this
        // may still need to be done for given project
    bool anonymous_platform;
        // app_versions.xml file found in project dir;
        // use those apps rather then getting from server
    bool non_cpu_intensive;
        // The project has asserted (in sched reply) that
        // all its apps are non-CPU-intensive.
    bool strict_memory_bound;
        // assume that jobs from this project will have a WSS
        // of wu.rsc_memory_bound,
        // even if it's currently less.
        // For example, CPDN jobs start small and get big later.
        // If we run a lot of them (based on the small WSS)
        // the system will run out of RAM and swap when they get big
    bool use_symlinks;
    bool report_results_immediately;
    bool sched_req_no_work[MAX_RSC];
        // the last sched request asked for work for resource i
        // and didn't get any

    // items sent in scheduler replies,
    // requesting that various things be sent subsequent requests
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
        // if using AM, do AM RPC before detaching
    bool ended;
        // project has ended; advise user to detach
    char code_sign_key[MAX_KEY_LEN];
    std::vector<FILE_REF> user_files;
    std::vector<FILE_REF> project_files;
        // files not specific to apps or work - e.g. icons
    bool app_test;
        // this is the project created by app_test_init();
        // use slots/app_test for its jobs

    ///////////////// member functions /////////////////

    void set_min_rpc_time(double future_time, const char* reason);
    int parse_preferences_for_user_files();
    void write_project_files(MIOFILE&);
    void link_project_files();
    void create_project_file_symlinks();
    void delete_project_file_symlinks();
    int write_symlink_for_project_file(FILE_INFO*);
    double project_files_downloaded_time;
        // when last project file download finished
    void update_project_files_downloaded_time();
        // called when a project file download finishes.
        // If it's the last one, set project_files_downloaded_time to now

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
    bool some_download_stalled();
        // a download is backed off
    bool some_result_suspended();
    bool uploading();
    bool has_results();
    int proj_n_concurrent;
        // used to enforce APP_CONFIGS::max_concurrent

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
    bool operator<(const PROJECT& p) {
        return sched_priority > p.sched_priority;
    }

    // stuff related to work fetch
    //
    RSC_PROJECT_WORK_FETCH rsc_pwf[MAX_RSC];
    PROJECT_WORK_FETCH pwf;
    inline void work_fetch_reset() {
        for (int i=0; i<coprocs.n_rsc; i++) {
            rsc_pwf[i].reset(i);
        }
    }
    inline int deadlines_missed(int rsc_type) {
        return rsc_pwf[rsc_type].deadlines_missed;
    }
    void get_task_durs(double& not_started_dur, double& in_progress_dur);
    void check_no_rsc_apps();
        // if flags are set for all resource types,
        // something's wrong; clear them.
    void check_no_apps();
        // set no_X_apps for anonymous platform projects

    int nresults_returned;
        // # of results being returned in current scheduler op
    const char* get_scheduler_url(int index, double r);
        // get scheduler URL with random offset r
    bool checked;
        // temporary used when scanning projects
    bool dont_contact;
        // temp in find_project_with_overdue_results()
    int n_ready;
        // temp in find_project_with_overdue_results()

    FILE_XFER_BACKOFF download_backoff;
    FILE_XFER_BACKOFF upload_backoff;
    inline FILE_XFER_BACKOFF& file_xfer_backoff(bool is_upload) {
        return is_upload?upload_backoff:download_backoff;
    }

    // support for replicated trickle-ups
    //
    std::vector<TRICKLE_UP_OP*> trickle_up_ops;

    // app config stuff
    //
    APP_CONFIGS app_configs;

    // job counting
    //
    int njobs_success;
    int njobs_error;

    // total elapsed time of this project's jobs (for export to GUI)
    //
    double elapsed_time;

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
    const char* project_dir();
    const char* project_dir_absolute();
    void show_no_work_notice();

    // statistic of the last x days
    std::vector<DAILY_STATS> statistics;
    int parse_statistics(FILE*);
    int write_statistics(MIOFILE&);
    int write_statistics_file();
    void trim_statistics();

    void suspend();
    void resume();
    void abort_not_started();
        // abort unstarted jobs

    // clear AMS-related fields
    inline void detach_ams() {
        attached_via_acct_mgr = false;
        for (int i=0; i<MAX_RSC; i++) {
            no_rsc_ams[i] = false;
        }

        ams_resource_share = -1;

        // parse the account file to get right resource share
        // in case AMS had set it
        //
        parse_account_file();
    }

#ifdef SIM
    RANDOM_PROCESS available;
    int proj_index; // order among projects; used for color coding
    int result_index;
    double idle_time;
    double idle_time_sumsq;
    bool idle;
    int max_infeasible_count;
    bool no_apps;
    bool ignore;
    // for DCF variants:
    int completed_task_count;
    double completions_ratio_mean;
    double completions_ratio_s;
    double completions_ratio_stdev;
    double completions_required_stdevs;
    PROJECT_RESULTS project_results;
    void print_results(FILE*, SIM_RESULTS&);
    void backoff();
#endif
};

#endif

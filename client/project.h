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

#ifndef _PROJECT_
#define _PROJECT_

#include "client_types.h"

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

#endif

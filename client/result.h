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

#ifndef _RESULT_
#define _RESULT_

#include "project.h"

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
    double estimated_runtime();
    double estimated_runtime_uncorrected();
    double estimated_runtime_remaining();
    inline double estimated_flops_remaining() {
#ifdef SIM
        return sim_flops_left;
#else
        return estimated_runtime_remaining()*avp->flops;
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

inline bool max_concurrent_exceeded(RESULT* rp) {
    APP* app = rp->app;
    if (!app->max_concurrent) return false;
    return (app->n_concurrent >= app->max_concurrent);

}

inline void max_concurrent_inc(RESULT* rp) {
    rp->app->n_concurrent++;
}

#endif

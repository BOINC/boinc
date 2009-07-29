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

// Matchmaker scheduling code

#include <list>

#include "boinc_db.h"
#include "error_numbers.h"
#include "str_util.h"

#include "main.h"
#include "sched_config.h"
#include "sched_hr.h"
#include "sched_msgs.h"
#include "sched_shmem.h"
#include "sched_send.h"
#include "sched_version.h"
#include "server_types.h"

#include "sched_score.h"

struct JOB {
    int index;
    double score;
    double est_time;
    double disk_usage;
    APP* app;
    BEST_APP_VERSION* bavp;

    bool get_score();
};

struct JOB_SET {
    double work_req;
    double est_time;
    double disk_usage;
    double disk_limit;
    int max_jobs;
    std::list<JOB> jobs;     // sorted high to low

    JOB_SET() {
        work_req = g_request->work_req_seconds;
        est_time = 0;
        disk_usage = 0;
        disk_limit = g_wreq->disk_available;
        max_jobs = g_wreq->max_jobs_per_rpc;

        int n = g_wreq->max_jobs_on_host - g_wreq->njobs_on_host;
        if (n < 0) n = 0;
        if (n < max_jobs) max_jobs = n;
    }
    void add_job(JOB&);
    double higher_score_disk_usage(double);
    double lowest_score();
    inline bool request_satisfied() {
        return est_time >= work_req;
    }
    void send();
};

// reread result from DB, make sure it's still unsent
// TODO: from here to add_result_to_reply()
// (which updates the DB record) should be a transaction
//
int read_sendable_result(DB_RESULT& result) {
    int retval = result.lookup_id(result.id);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] result.lookup_id() failed %d\n",
            result.id, retval
        );
        return ERR_NOT_FOUND;
    }
    if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
        log_messages.printf(MSG_NORMAL,
            "[RESULT#%d] expected to be unsent; instead, state is %d\n",
            result.id, result.server_state
        );
        return ERR_BAD_RESULT_STATE;
    }
    return 0;
}

// compute a "score" for sending this job to this host.
// Return false if the WU is infeasible.
// Otherwise set est_time and disk_usage.
//
bool JOB::get_score() {
    WORKUNIT wu;
    int retval;

    WU_RESULT& wu_result = ssp->wu_results[index];
    wu = wu_result.workunit;
    app = ssp->lookup_app(wu.appid);

    score = 0;

    // Find the best app version to use.
    //
    bavp = get_app_version(wu, true);
    if (!bavp) return false;

    retval = wu_is_infeasible_fast(wu, *app, *bavp);
    if (retval) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] [HOST#%d] [WU#%d %s] WU is infeasible: %s\n",
                g_reply->host.id, wu.id, wu.name, infeasible_string(retval)
            );
        }
        return false;
    }

    score = 1;

#if 0
    // example: for CUDA app, wu.batch is the minimum number of processors.
    // Don't send if #procs is less than this.
    // Otherwise add min/actual to score
    // (this favors sending jobs that need lots of procs to GPUs that have them)
    //
    if (!strcmp(app->name, "foobar") && bavp->host_usage.ncudas) {
        if (!g_request->coproc_cuda) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%d] expected CUDA device\n", g_reply->host.id
            );
            return false;
        }
        int n = g_request->coproc_cuda->prop.multiProcessorCount;
        if (n < wu.batch) {
            return false;
        }
        score += ((double)wu.batch)/n;
    }
#endif

    // check if user has selected apps,
    // and send beta work to beta users
    //
    if (app->beta && !config.distinct_beta_apps) {
        if (g_wreq->allow_beta_work) {
            score += 1;
        } else {
            return false;
        }
    } else {
        if (app_not_selected(wu)) {
            if (!g_wreq->allow_non_preferred_apps) {
                return false;
            } else {
            // Allow work to be sent, but it will not get a bump in its score
            }
        } else {
            score += 1;
        }
    }
            
    // if job needs to get done fast, send to fast/reliable host
    //
    if (g_wreq->reliable && (wu_result.need_reliable)) {
        score += 1;
    }
    
    // if job already committed to an HR class,
    // try to send to host in that class
    //
    if (wu_result.infeasible_count) {
        score += 1;
    }

    // Favor jobs that will run fast
    //
    score += bavp->host_usage.flops/1e9;

    // match large jobs to fast hosts
    //
    if (config.job_size_matching) {
        double host_stdev = (g_reply->host.p_fpops - ssp->perf_info.host_fpops_mean)/ ssp->perf_info.host_fpops_stdev;
        double diff = host_stdev - wu_result.fpops_size;
        score -= diff*diff;
    }

    // TODO: If user has selected some apps but will accept jobs from others,
    // try to send them jobs from the selected apps
    //

    est_time = estimate_duration(wu, *bavp);
    disk_usage = wu.rsc_disk_bound;
    return true;
}

bool wu_is_infeasible_slow(
    WU_RESULT& wu_result, SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply
) {
    char buf[256];
    int retval;
    int n;
    DB_RESULT result;

    // Don't send if we've already sent a result of this WU to this user.
    //
    if (config.one_result_per_user_per_wu) {
        sprintf(buf,
            "where workunitid=%d and userid=%d",
            wu_result.workunit.id, g_reply->user.id
        );
        retval = result.count(n, buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "send_work: can't get result count (%d)\n", retval
            );
            return true;
        } else {
            if (n>0) {
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] send_work: user %d already has %d result(s) for WU %d\n",
                        g_reply->user.id, n, wu_result.workunit.id
                    );
                }
                return true;
            }
        }
    } else if (config.one_result_per_host_per_wu) {
        // Don't send if we've already sent a result
        // of this WU to this host.
        // We only have to check this
        // if we don't send one result per user.
        //
        sprintf(buf,
            "where workunitid=%d and hostid=%d",
            wu_result.workunit.id, g_reply->host.id
        );
        retval = result.count(n, buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "send_work: can't get result count (%d)\n", retval
            );
            return true;
        } else {
            if (n>0) {
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] send_work: host %d already has %d result(s) for WU %d\n",
                        g_reply->host.id, n, wu_result.workunit.id
                    );
                }
                return true;
            }
        }
    }

    APP* app = ssp->lookup_app(wu_result.workunit.appid);
    WORKUNIT wu = wu_result.workunit;
    if (app_hr_type(*app)) {
        if (already_sent_to_different_platform_careful(wu, *app)) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [HOST#%d] [WU#%d %s] WU is infeasible (assigned to different platform)\n",
                    g_reply->host.id, wu.id, wu.name
                );
            }
            // Mark the workunit as infeasible.
            // This ensures that jobs already assigned to a platform
            // are processed first.
            //
            wu_result.infeasible_count++;
            return true;
        }
    }
    return false;
}

double JOB_SET::lowest_score() {
    if (jobs.empty()) return 0;
    return jobs.back().score;
}

// add the given job, and remove lowest-score jobs that
// - are in excess of work request
// - are in excess of per-request or per-day limits
// - cause the disk limit to be exceeded
//
void JOB_SET::add_job(JOB& job) {
    while (!jobs.empty()) {
        JOB& worst_job = jobs.back();
        if (est_time + job.est_time - worst_job.est_time > work_req) {
            est_time -= worst_job.est_time;
            disk_usage -= worst_job.disk_usage;
            jobs.pop_back();
            ssp->wu_results[worst_job.index].state = WR_STATE_PRESENT;
        } else {
            break;
        }
    }
    while (!jobs.empty()) {
        JOB& worst_job = jobs.back();
        if (disk_usage + job.disk_usage > disk_limit) {
            est_time -= worst_job.est_time;
            disk_usage -= worst_job.disk_usage;
            jobs.pop_back();
            ssp->wu_results[worst_job.index].state = WR_STATE_PRESENT;
        } else {
            break;
        }
    }

    if (jobs.size() == max_jobs) {
        JOB& worst_job = jobs.back();
        jobs.pop_back();
        ssp->wu_results[worst_job.index].state = WR_STATE_PRESENT;
    }

    std::list<JOB>::iterator i = jobs.begin();
    while (i != jobs.end()) {
        if (i->score < job.score) {
            jobs.insert(i, job);
            break;
        }
        i++;
    }
    if (i == jobs.end()) {
        jobs.push_back(job);
    }
    est_time += job.est_time;
    disk_usage += job.disk_usage;
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] added job to set.  est_time %.2f disk_usage %.2fGB\n",
            est_time, disk_usage/GIGA
        );
    }
}

// return the disk usage of jobs above the given score
//
double JOB_SET::higher_score_disk_usage(double v) {
    double sum = 0;
    std::list<JOB>::iterator i = jobs.begin();
    while (i != jobs.end()) {
        if (i->score < v) break;
        sum += i->disk_usage;
        i++;
    }
    return sum;
}

void JOB_SET::send() {
    WORKUNIT wu;
    DB_RESULT result;
    int retval;

    std::list<JOB>::iterator i = jobs.begin();
    while (i != jobs.end()) {
        JOB& job = *(i++);
        WU_RESULT wu_result = ssp->wu_results[job.index];
        ssp->wu_results[job.index].state = WR_STATE_EMPTY;
        wu = wu_result.workunit;
        result.id = wu_result.resultid;
        retval = read_sendable_result(result);
        if (!retval) {
            add_result_to_reply(result, wu, job.bavp, false);
        }
    }
}

void send_work_matchmaker() {
    int i, slots_locked=0, slots_nonempty=0;
    JOB_SET jobs;
    int min_slots = config.mm_min_slots;
    if (!min_slots) min_slots = ssp->max_wu_results/2;
    int max_slots = config.mm_max_slots;
    if (!max_slots) max_slots = ssp->max_wu_results;
    int max_locked = 10;

    lock_sema();
    i = rand() % ssp->max_wu_results;

    // scan through the job cache, maintaining a JOB_SET of jobs
    // that we can send to this client, ordered by score.
    //
    for (int slots_scanned=0; slots_scanned<max_slots; slots_scanned++) {
        i = (i+1) % ssp->max_wu_results;
        WU_RESULT& wu_result = ssp->wu_results[i];
        switch (wu_result.state) {
        case WR_STATE_EMPTY:
            continue;
        case WR_STATE_PRESENT:
            slots_nonempty++;
            break;
        default:
            slots_nonempty++;
            if (wu_result.state == g_pid) break;
            slots_locked++;
            continue;
        }

        JOB job;
        job.index = i;

        // get score for this job, and skip it if it fails quick check.
        // NOTE: the EDF check done in get_score()
        // includes only in-progress jobs.
        //
        if (!job.get_score()) {
            continue;
        }
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] score for %s: %f\n", wu_result.workunit.name, job.score
            );
        }

        if (job.score > jobs.lowest_score() || !jobs.request_satisfied()) {
            ssp->wu_results[i].state = g_pid;
            unlock_sema();
            if (wu_is_infeasible_slow(wu_result, *g_request, *g_reply)) {
                // if we can't use this job, put it back in pool
                //
                lock_sema();
                ssp->wu_results[i].state = WR_STATE_PRESENT;
                continue;
            }
            lock_sema();
            jobs.add_job(job);
        }

        if (jobs.request_satisfied() && slots_scanned>=min_slots) break;
    }

    if (!slots_nonempty) {
        log_messages.printf(MSG_CRITICAL,
            "Job cache is empty - check feeder\n"
        );
        g_wreq->no_jobs_available = true;
    }

    // TODO: trim jobs from tail of list until we pass the EDF check
    //
    jobs.send();
    unlock_sema();
    if (slots_locked > max_locked) {
        log_messages.printf(MSG_CRITICAL,
            "Found too many locked slots (%d>%d) - increase array size\n",
            slots_locked, max_locked
        );
    }
}


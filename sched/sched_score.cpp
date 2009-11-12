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

#include "boinc_db.h"
#include "error_numbers.h"
#include "str_util.h"

#include "sched_main.h"
#include "sched_config.h"
#include "sched_hr.h"
#include "sched_msgs.h"
#include "sched_shmem.h"
#include "sched_send.h"
#include "sched_version.h"
#include "sched_types.h"

#include "sched_score.h"

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

    if (slots_nonempty) {
        g_wreq->no_jobs_available = false;
    } else {
        log_messages.printf(MSG_CRITICAL,
            "Job cache is empty - check feeder\n"
        );
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


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

#include <vector>

#include "client_msgs.h"
#include "client_state.h"
#include "client_types.h"
#include "coproc.h"
#include "result.h"

#include "coproc_sched.h"

using std::vector;

#if 0
#define COPROC_DEBUG(x) x
#else
#define COPROC_DEBUG(X)
#endif


////////// Coprocessor scheduling ////////////////
//
// theory of operation:
//
// Jobs can use one or more integral instances, or a fractional instance
//
// RESULT::coproc_indices
//    for a running job, the coprocessor instances it's using
// COPROC::pending_usage[]: for each instance, its usage by running jobs
//    Note: "running" includes jobs suspended due to CPU throttling.
//    That's the only kind of suspended GPU job.
// COPROC::usage[]: for each instance, its usage
//
// enforce_run_list() calls assign_coprocs(),
// which assigns coproc instances to scheduled jobs,
// and prunes jobs for which we can't make an assignment
// (the job list is in order of decreasing priority)
//
// assign_coprocs():
//     clear usage and pending_usage of all instances
//     for each running/suspended job J
//         increment pending_usage for the instances assigned to J
//     for each scheduled job J
//         if J is running
//             if J's assignment fits
//                 confirm assignment: dec pending_usage, inc usage
//             else
//                 prune J
//         else
//             if J.usage is fractional
//                look for an instance that's already fractionally assigned
//                if that fails, look for a free instance
//                if that fails, prune J
//             else
//                if there are enough instances with usage=0
//                    assign instances with pending_usage = usage = 0
//                        (avoid preempting running jobs)
//                    if need more, assign instances with usage = 0
//                else
//                    prune J

// can the given task use this GPU instance?  Enforce
// - GPU exclusions
// - OpenCL availability (relevant if use_all_gpus set)
//
static inline bool can_use_gpu(RESULT* rp, COPROC* cp, int i) {
    if (gpu_excluded(rp->app, *cp, i)) {
        COPROC_DEBUG(msg_printf(rp->project, MSG_INFO, "GPU %d is excluded for %s", i, rp->name));
        return false;
    }
    if (rp->avp->is_opencl()) {
        if (!cp->instance_has_opencl[i]) {
            COPROC_DEBUG(msg_printf(rp->project, MSG_INFO, "GPU %d can't do OpenCL for %s", i, rp->name));
            return false;
        }
    }
    return true;
}

static inline void increment_pending_usage(
    RESULT* rp, double usage, COPROC* cp
) {
    double x = (usage<1)?usage:1;
    for (int i=0; i<usage; i++) {
        int j = rp->coproc_indices[i];
        cp->pending_usage[j] += x;
        if (log_flags.coproc_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[coproc] %s instance %d; %f pending for %s", cp->type, i, x, rp->name
            );
            if (cp->pending_usage[j] > 1) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] huh? %s %d %s pending usage > 1",
                    cp->type, i, rp->name
                );
            }
        }
    }
}

// check the GPU assignment for a currently-running app.
// Note: don't check available RAM.
// It may not be known (e.g. NVIDIA) and in any case,
// if the app is still running, it has enough RAM
//
static inline bool current_assignment_ok(
    RESULT* rp, double usage, COPROC* cp
) {
    double x = (usage<1)?usage:1;
    for (int i=0; i<usage; i++) {
        int j = rp->coproc_indices[i];
        if (cp->usage[j] + x > 1) {
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] %s %f instance of device %d already assigned to task %s",
                    cp->type, x, j, rp->name
                );
            }
            return false;
        }
    }
    return true;
}

static inline void confirm_current_assignment(
    RESULT* rp, double usage, COPROC* cp
) {
    double x = (usage<1)?usage:1;
    for (int i=0; i<usage; i++) {
        int j = rp->coproc_indices[i];
        cp->usage[j] +=x;
        cp->pending_usage[j] -=x;
        if (log_flags.coproc_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[coproc] %s instance %d: confirming %f instance for %s",
                cp->type, j, x, rp->name
            );
        }
    }
}

static inline bool get_fractional_assignment(RESULT* rp, double usage, COPROC* cp) {
    int i;

    // try to assign an instance that's already fractionally assigned
    //
    for (i=0; i<cp->count; i++) {
        if (!can_use_gpu(rp, cp, i)) {
            continue;
        }
        if ((cp->usage[i] || cp->pending_usage[i])
            && (cp->usage[i] + cp->pending_usage[i] + usage <= 1)
        ) {
            rp->coproc_indices[0] = i;
            cp->usage[i] += usage;
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] Assigning %f of %s instance %d to %s",
                    usage, cp->type, i, rp->name
                );
            }
            return true;
        }
    }

    // failing that, assign an unreserved instance
    //
    for (i=0; i<cp->count; i++) {
        if (!can_use_gpu(rp, cp, i)) {
            continue;
        }
        if (!cp->usage[i]) {
            rp->coproc_indices[0] = i;
            cp->usage[i] += usage;
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] Assigning %f of %s free instance %d to %s",
                    usage, cp->type, i, rp->name
                );
            }
            return true;
        }
    }
    if (log_flags.coproc_debug) {
        msg_printf(rp->project, MSG_INFO,
            "[coproc] Insufficient %s for %s: need %f",
            cp->type, rp->name, usage
        );
    }

    return false;
}

static inline bool get_integer_assignment(
    RESULT* rp, double usage, COPROC* cp
) {
    int i;

    // make sure we have enough free instances
    //
    int nfree = 0;
    for (i=0; i<cp->count; i++) {
        if (!can_use_gpu(rp, cp, i)) {
            continue;
        }
        if (!cp->usage[i]) {
            nfree++;
        }
    }
    if (nfree < usage) {
        if (log_flags.coproc_debug) {
            msg_printf(rp->project, MSG_INFO,
                "[coproc] Insufficient %s for %s; need %d, available %d",
                cp->type, rp->name, (int)usage, nfree
            );
        }
        return false;
    }

    int n = 0;

    // assign non-pending instances first

    for (i=0; i<cp->count; i++) {
        if (!can_use_gpu(rp, cp, i)) {
            continue;
        }
        if (!cp->usage[i] && !cp->pending_usage[i]) {
            cp->usage[i] = 1;
            rp->coproc_indices[n++] = i;
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] Assigning %s instance %d to %s",
                    cp->type, i, rp->name
                );
            }
            if (n == usage) return true;
        }
    }

    // if needed, assign pending instances

    for (i=0; i<cp->count; i++) {
        if (!can_use_gpu(rp, cp, i)) {
            continue;
        }
        if (!cp->usage[i]) {
            cp->usage[i] = 1;
            rp->coproc_indices[n++] = i;
            if (log_flags.coproc_debug) {
                msg_printf(rp->project, MSG_INFO,
                    "[coproc] Assigning %s pending instance %d to %s",
                    cp->type, i, rp->name
                );
            }
            if (n == usage) return true;
        }
    }
    if (log_flags.coproc_debug) {
        msg_printf(rp->project, MSG_INFO,
            "[coproc] huh??? ran out of %s instances for %s",
            cp->type, rp->name
        );
    }
    return false;
}

void assign_coprocs(vector<RESULT*>& jobs) {
    unsigned int i;
    COPROC* cp;
    double usage;

    coprocs.clear_usage();

    // fill in pending usage
    //
    for (i=0; i<jobs.size(); i++) {
        RESULT* rp = jobs[i];
        int rt = rp->resource_usage.rsc_type;
        if (rt) {
            usage = rp->resource_usage.coproc_usage;
            cp = &coprocs.coprocs[rt];
        } else {
            continue;
        }
        ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rp);
        if (!atp) continue;
        if (atp->is_gpu_task_running()) {
            increment_pending_usage(rp, usage, cp);
        }
    }

    vector<RESULT*>::iterator job_iter;
    job_iter = jobs.begin();
    while (job_iter != jobs.end()) {
        RESULT* rp = *job_iter;
        int rt = rp->resource_usage.rsc_type;
        if (rt) {
            usage = rp->resource_usage.coproc_usage;
            cp = &coprocs.coprocs[rt];
        } else {
            ++job_iter;
            continue;
        }

        ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rp);
        if (atp && atp->is_gpu_task_running()) {
            if (current_assignment_ok(rp, usage, cp)) {
                confirm_current_assignment(rp, usage, cp);
                ++job_iter;
            } else {
                job_iter = jobs.erase(job_iter);
            }
        } else {
            if (usage < 1) {
                if (get_fractional_assignment(rp, usage, cp)) {
                    ++job_iter;
                } else {
                    job_iter = jobs.erase(job_iter);
                }
            } else {
                if (get_integer_assignment(rp, usage, cp)) {
                    ++job_iter;
                } else {
                    job_iter = jobs.erase(job_iter);
                }
            }
        }
    }

#if 0
    // enforce "don't use GPUs while active" pref in NVIDIA case;
    // it applies only to GPUs running a graphics app
    //
    if (gstate.host_info.coprocs.nvidia.count && gstate.user_active && !gstate.global_prefs.run_gpu_if_user_active) {
        job_iter = jobs.begin();
        while (job_iter != jobs.end()) {
            RESULT* rp = *job_iter;
            if (!rp->avp->ncudas) {
                job_iter++;
                continue;
            }
            ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rp);
            bool some_gpu_busy = false;
            for (i=0; i<rp->avp->ncudas; i++) {
                int dev = atp->coproc_indices[i];
                if (gstate.host_info.coprocs.cuda.running_graphics_app[dev]) {
                    some_gpu_busy = true;
                    break;
                }
            }
            if (some_gpu_busy) {
                job_iter = jobs.erase(job_iter);
            } else {
                job_iter++;
            }
        }
    }
#endif
}

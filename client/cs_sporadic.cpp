// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// logic for handling sporadic jobs
//
// Currently sporadic jobs have priority over others.
// In particular, they can preempt jobs that
// - are in danger of missing their deadline
// - have done a lot of computing and haven't checkpointed
// - are from projects with a resource share debt
// At some point we should fix this.

// Apps can be
// regular: jobs compute when running
// sporadic: jobs run all the time but compute only part of the time
// non-CPU-intensive (NCI): jobs run all the time but don't compute
//
// Projects can have any or all of these, and this can change over time.
// A project is flagged as NCI if it has only NCI apps;
// in that case it's omitted from resource share calculations.

// Note: the client and app communicate via 1-way streams
// that are polled once/sec.
// This introduces potential uncertainty:
// if we send the app a message,
// once second later we don't know if it received the message and responded.
// To avoid this problem, when we send a message to an app
// we ignore its messages for the next 2.5 seconds.
// Perhaps a better approach would be to use sequence numbers and acks.
//
// states and transitions:
// CA_DONT_COMPUTE
//  computing is suspended, or insufficient resources
//  transitions:
//  to COULD_COMPUTE when these no longer hold
// CA_COULD_COMPUTE
//  not computing, but could
//  transitions:
//  to CA_DONT_COMPUTE if computing suspended or insufficient resources
//  to CA_COMPUTING if get AC_WANT_COMPUTE
// CA_COMPUTING
//  job can compute (and is, as far as we know)
//  transitions:
//  to CA_DONT_COMPUTE if computing suspended or insufficient resources
//  to CA_DONT_COMPUTE if get AC_DONT_WANT_COMPUTE or AC_NONE
//      (after timeout - see above)
//
// Interaction with the batch scheduler:
//  If we make a transition that changes resource usage,
//  request a reschedule to start/stop batch jobs
//  The batch scheduler subtracts resources used by sporadic jobs
// Coprocs:
//  If batch jobs are using GPUs, it may take them a few seconds to exit.
//  Sporadic jobs that use GPUs should delay for a few seconds at start,
//  and retry failed VRAM allocations.
//

#include "coproc.h"

#include "client_state.h"
#include "client_msgs.h"
#include "coproc_sched.h"
#include "result.h"
#include "app.h"

#define SPORADIC_MSG_DELAY  2.5

SPORADIC_RESOURCES sporadic_resources;

void SPORADIC_RESOURCES::print() {
    if (!ncpus_used) return;
    msg_printf(NULL, MSG_INFO, "Sporadic resources:");
    msg_printf(NULL, MSG_INFO, "   %f CPUs", ncpus_used);
    msg_printf(NULL, MSG_INFO, "   %f MB RAM", mem_used/MEGA);
    for (int i=1; i<sr_coprocs.n_rsc; i++) {
        COPROC& cp = sr_coprocs.coprocs[i];
        for (int j=0; j<cp.count; j++) {
            if (cp.usage[j] > 0) {
                msg_printf(NULL, MSG_INFO, "   GPU %s instance %d: %f\n",
                    cp.type, j, cp.usage[j]
                );
            }
        }
    }
}

// is computing suspended for this job?
//
static bool computing_suspended(ACTIVE_TASK *atp) {
    if (gstate.suspend_reason) return true;
    if (atp->result->uses_gpu() && gpu_suspend_reason) return true;
    return false;
}

// polling routine, called once/sec
void CLIENT_STATE::sporadic_poll() {
    sporadic_resources.init_poll();
    sporadic_resources.mem_max = available_ram();
    sporadic_resources.ncpus_max = n_usable_cpus;

    bool changed_active = false;
        // whether we need to reschedule regular jobs

    // find jobs that are active but shouldn't be
    // (CA_COMPUTING -> CA_NONE transitions)
    //
    for (ACTIVE_TASK *atp: active_tasks.active_tasks) {
        if (!atp->sporadic()) continue;
        if (atp->sporadic_ca_state != CA_COMPUTING) continue;

        // the job is in state CA_COMPUTING

        // see if the job needs to stop computing
        if (computing_suspended(atp)) {
            atp->sporadic_ca_state = CA_NONE;
            changed_active = true;
            if (log_flags.sporadic_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[sporadic] preempting %s: computing suspended",
                    atp->result->name
                );
            }
        } else if (!sporadic_resources.enough(atp)) {
            // this could happen if user prefs change
            atp->sporadic_ca_state = CA_NONE;
            changed_active = true;
            if (log_flags.sporadic_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[sporadic] preempting %s: insufficient resources",
                    atp->result->name
                );
            }
        } else if (atp->sporadic_ac_state != AC_WANT_COMPUTE) {
            if (now > atp->sporadic_ignore_until) {
                atp->sporadic_ca_state = CA_NONE;
                changed_active = true;
                if (log_flags.sporadic_debug) {
                    msg_printf(atp->result->project, MSG_INFO,
                        "[sporadic] %s: app is done computing",
                        atp->result->name
                    );
                }
            }
        }
        // the job can keep computing - reserve its resources
        if (atp->sporadic_ca_state == CA_COMPUTING) {
            sporadic_resources.reserve(atp);
        }
    }

    // activate jobs as needed
    // (CA_COULD_COMPUTE -> CA_COMPUTING transitions)
    //
    for (ACTIVE_TASK *atp: active_tasks.active_tasks) {
        if (!atp->sporadic()) continue;
        if (atp->sporadic_ca_state != CA_COULD_COMPUTE) continue;
        if (computing_suspended(atp)) {
            atp->sporadic_ca_state = CA_DONT_COMPUTE;
            if (log_flags.sporadic_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[sporadic] %s can no longer compute: suspended",
                    atp->result->name
                );
            }
        } else if (!sporadic_resources.enough(atp)) {
            atp->sporadic_ca_state = CA_DONT_COMPUTE;
            if (log_flags.sporadic_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[sporadic] %s can no longer compute: insufficient resources",
                    atp->result->name
                );
            }
        } else if (atp->sporadic_ac_state == AC_WANT_COMPUTE) {
            if (now > atp->sporadic_ignore_until) {
                atp->sporadic_ca_state = CA_COMPUTING;
                atp->sporadic_ignore_until = now + SPORADIC_MSG_DELAY;
                sporadic_resources.reserve(atp);
                changed_active = true;
                if (log_flags.sporadic_debug) {
                    msg_printf(atp->result->project, MSG_INFO,
                        "[sporadic] starting %s",
                        atp->result->name
                    );
                }
            }
        }
    }

    // assign states to initial, preempted, and done jobs
    //
    for (ACTIVE_TASK *atp: active_tasks.active_tasks) {
        if (!atp->sporadic()) continue;
        if (atp->sporadic_ca_state != CA_NONE) continue;
        if (computing_suspended(atp)) {
            atp->sporadic_ca_state = CA_DONT_COMPUTE;
            if (log_flags.sporadic_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[sporadic] %s can't compute: suspended",
                    atp->result->name
                );
            }
        } else if (!sporadic_resources.enough(atp)) {
            atp->sporadic_ca_state = CA_DONT_COMPUTE;
            if (log_flags.sporadic_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[sporadic] %s can't compute: insufficient resources",
                    atp->result->name
                );
            }
        } else {
            atp->sporadic_ca_state = CA_COULD_COMPUTE;
            if (log_flags.sporadic_debug) {
                msg_printf(atp->result->project, MSG_INFO,
                    "[sporadic] %s can compute",
                    atp->result->name
                );
            }
        }
    }

    if (changed_active) {
        request_schedule_cpus("sporadic apps changed state");
    }

    if (log_flags.sporadic_debug) {
        sporadic_resources.print();
    }
}

void CLIENT_STATE::sporadic_init() {
    sporadic_resources.init();
}

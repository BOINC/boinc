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

// stuff related to scheduling coprocessors

#include <vector>

struct RESULT;

extern void assign_coprocs(std::vector<RESULT*>& jobs);

// The resources (proc and mem) used by running sporadic jobs.
// We recompute these on each poll, since e.g. WSS can change.
//
// Note: this is similar to (but simpler than)
// the PROC_RESOURCES struct in cpu_sched.cpp.
// Perhaps the two could be merged; for now, easier to keep them separate.
//
struct SPORADIC_RESOURCES {
    double ncpus_used, ncpus_max;
    double mem_used, mem_max;
    COPROCS sr_coprocs;

    // clear reservations; called on each poll
    void init_poll() {
        ncpus_used= 0;
        mem_used = 0;
        sr_coprocs.clear_usage();
    }

    // called once at start
    void init() {
        sr_coprocs.clone(coprocs, false);
        init_poll();
    }

    // are there enough free resources to run the task?
    bool enough(ACTIVE_TASK *atp) {
        if (mem_used + atp->procinfo.working_set_size_smoothed > mem_max) {
            return false;
        }
        RESULT *rp = atp->result;
        if (ncpus_used + rp->resource_usage.avg_ncpus > ncpus_max) {
            return false;
        }
        int rt = rp->resource_usage.rsc_type;
        bool found = false;
        if (rt) {
            double u = rp->resource_usage.coproc_usage;
            COPROC& cp = sr_coprocs.coprocs[rt];
            for (int i=0; i<cp.count; i++) {
                if (gpu_excluded(rp->app, cp, i)) continue;
                if (u + cp.usage[i] <= 1) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    // reserve resources for the task
    void reserve(ACTIVE_TASK *atp) {
        RESULT *rp = atp->result;
        mem_used += atp->procinfo.working_set_size_smoothed;
        ncpus_used+= rp->resource_usage.avg_ncpus;
        int rt = rp->resource_usage.rsc_type;
        if (rt) {
            double u = rp->resource_usage.coproc_usage;
            COPROC& cp = sr_coprocs.coprocs[rt];
            for (int i=0; i<cp.count; i++) {
                if (gpu_excluded(rp->app, cp, i)) continue;
                if (u + cp.usage[i] <= 1) {
                    cp.usage[i] += u;
                    break;
                }
            }
        }
    }

    void print();
};

extern SPORADIC_RESOURCES sporadic_resources;

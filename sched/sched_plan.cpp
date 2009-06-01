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
//

// Replace the following with your own code.
// WARNING: after doing this, you must prevent this file from
// being overwritten the next time you update BOINC source code.
// You can either:
// 1) write-protect this file, or
// 2) put this in a differently-named file and change the Makefile.am
//    (and write-protect that)
// In either case, put your version under source-code control, e.g. SVN

#include "str_util.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_send.h"

#include "sched_plan.h"

int app_plan(SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu) {
    if (!strcmp(plan_class, "mt")) {
        // the following is for an app that can use anywhere
        // from 1 to 64 threads, can control this exactly,
        // and whose speedup is .95N
        // (on a uniprocessor, we'll use a sequential app if one is available)
        //
        int nthreads;

        nthreads = effective_ncpus();
        if (nthreads > 64) nthreads = 64;
        hu.avg_ncpus = nthreads;
        hu.max_ncpus = nthreads;
        sprintf(hu.cmdline, "--nthreads %d", nthreads);
        hu.flops = 0.95*sreq.host.p_fpops*nthreads;
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Multi-thread app estimate %.2f GFLOPS\n",
                hu.flops/1e9
            );
        }
        return 0;
    } else if (!strcmp(plan_class, "cuda")) {
        // the following is for an app that uses a CUDA GPU
        //
        COPROC_CUDA* cp = (COPROC_CUDA*)sreq.coprocs.lookup("CUDA");
        if (!cp) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] Host lacks CUDA coprocessor for plan class cuda\n"
                );
            }
            return PLAN_REJECT_CUDA_NO_DEVICE;
        }
        int v = (cp->prop.major)*100 + cp->prop.minor;
        if (v < 100) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] CUDA version %d < 1.0\n", v
                );
            }
            return PLAN_REJECT_CUDA_VERSION;
        } 

        if (cp->drvVersion && cp->drvVersion < PLAN_CUDA_MIN_DRIVER_VERSION) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] NVIDIA driver version %d < PLAN_CUDA_MIN_DRIVER_VERSION\n",
                    cp->drvVersion
                );
            }
            return PLAN_REJECT_NVIDIA_DRIVER_VERSION;
        }

        if (cp->prop.dtotalGlobalMem < PLAN_CUDA_MIN_RAM) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] CUDA mem %d < %d\n",
                    cp->prop.dtotalGlobalMem, PLAN_CUDA_MIN_RAM
                );
            }
            return PLAN_REJECT_CUDA_MEM;
        }
        hu.flops = cp->flops_estimate();

#if 0    // THIS PROBLEM HAS BEEN FIXED IN THE SETI@HOME APP
        // On Windows, slower GPUs sometimes get a blue screen of death
        //
        if (strstr(sreq.host.os_name, "Windows") && hu.flops < 60e9) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] Not sending CUDA job to Win host with slow GPU (%.1f GFLOPS)\n",
                    hu.flops/1e9
                );
            }
            return PLAN_REJECT_CUDA_SPEED;
        }
#endif

        // assume we'll need 0.5% as many CPU FLOPS as GPU FLOPS
        // to keep the GPU fed.
        //
        double x = (hu.flops*0.005)/sreq.host.p_fpops;
        hu.avg_ncpus = x;
        hu.max_ncpus = x;

        hu.ncudas = 1;

        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] CUDA app estimated %.2f GFLOPS (clock %d count %d)\n",
                hu.flops/1e9, cp->prop.clockRate,
                cp->prop.multiProcessorCount
            );
        }
        return 0;
    } else if (!strcmp(plan_class, "nci")) {
        // The following is for a non-CPU-intensive application.
        // Say that we'll use 1% of a CPU.
        // This will cause the client (6.7+) to run it at non-idle priority
        //
        hu.avg_ncpus = .01;
        hu.max_ncpus = .01;
        hu.flops = sreq.host.p_fpops*1.01;
            // The *1.01 is needed to ensure that we'll send this app
            // version rather than a non-plan-class one
        return 0;
    }
    log_messages.printf(MSG_CRITICAL,
        "Unknown plan class: %s\n", plan_class
    );
    return PLAN_REJECT_UNKNOWN;
}

bool app_plan_uses_gpu(const char* plan_class) {
    if (!strcmp(plan_class, "cuda")) {
        return true;
    }
    return false;
}

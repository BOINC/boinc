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

// This file contains functions that can be customized to
// implement project-specific scheduling policies.
// The functions are:
//
// wu_is_infeasible_custom()
//      Decide whether host can run a job using a particular app version
// app_plan()
//      Decide whether host can use an app version,
//      and if so what resources it will use
// app_plan_uses_gpu():
//      Which plan classes use GPUs
// JOB::get_score():
//      Determine the value of sending a particular job to host;
//      (used only by "matchmaker" scheduling)
//
// WARNING: if you modify this file, you must prevent it from
// being overwritten the next time you update BOINC source code.
// You can either:
// 1) write-protect this file, or
// 2) put this in a differently-named file and change the Makefile.am
//    (and write-protect that)
// In either case, put your version under source-code control, e.g. SVN

#include "str_util.h"

#include "sched_config.h"
#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_score.h"
#include "sched_shmem.h"
#include "sched_version.h"
#include "sched_customize.h"

bool wu_is_infeasible_custom(WORKUNIT& wu, APP& app, BEST_APP_VERSION& bav) {
#if 0
    // example: for CUDA app, wu.batch is the minimum number of processors.
    // Don't send if #procs is less than this.
    //
    if (!strcmp(app.name, "foobar") && bav.host_usage.ncudas) {
        if (!g_request->coproc_cuda) {
            log_messages.printf(MSG_CRITICAL,
                "[HOST#%d] expected CUDA device\n", g_reply->host.id
            );
            return true;
        }
        int n = g_request->coproc_cuda->prop.multiProcessorCount;
        if (n < wu.batch) {
           return true;
        }
    }
#endif
    return false;
}

bool app_plan(SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu) {
    char buf[256];
    if (!strcmp(plan_class, "mt")) {
        // the following is for an app that:
        // - can use from 1 to 64 threads, and can control this exactly
        // - if it uses N threads, will use .65N cores on average
        // (hence on a uniprocessor we'll use a sequential app
        // if one is available)
        //
        double ncpus = g_wreq->effective_ncpus;
            // number of usable CPUs, taking user prefs into account
        int nthreads = (int)(ncpus/.65);
        if (!nthreads) {
            add_no_work_message("Your computer has too few CPUs");
            return false;
        }
        if (nthreads > 64) nthreads = 64;
        hu.avg_ncpus = nthreads*.65;
        hu.max_ncpus = nthreads;
        sprintf(hu.cmdline, "--nthreads %d", nthreads);
        hu.flops = sreq.host.p_fpops*hu.avg_ncpus;
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Multi-thread app estimate %.2f GFLOPS\n",
                hu.flops/1e9
            );
        }
        return true;
    } else if (strstr(plan_class, "ati")) {
        // the following is for an app that uses an ATI GPU
        //
        COPROC_ATI* cp = (COPROC_ATI*)sreq.coprocs.lookup("ATI");
        if (!cp) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] Host lacks ATI GPU for plan class ati\n"
                );
            }
            add_no_work_message("Your computer has no ATI GPU");
            return false;
        }

        hu.flops = cp->flops();

        // assume we'll need 0.5% as many CPU FLOPS as GPU FLOPS
        // to keep the GPU fed.
        //
        double x = (hu.flops*0.005)/sreq.host.p_fpops;
        hu.avg_ncpus = x;
        hu.max_ncpus = x;

        hu.natis = 1;

        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] ATI app estimated %.2f GFLOPS\n", hu.flops/1e9
            );
        }
        return true;
    } else if (strstr(plan_class, "cuda")) {
        // the following is for an app that uses an NVIDIA GPU
        //
        COPROC_CUDA* cp = (COPROC_CUDA*)sreq.coprocs.lookup("CUDA");
        if (!cp) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] Host lacks CUDA coprocessor for plan class cuda\n"
                );
            }
            add_no_work_message("Your computer has no NVIDIA GPU");
            return false;
        }

        // check compute capability
        //
        int v = (cp->prop.major)*100 + cp->prop.minor;
        if (v < 100) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] Compute capability %d < 1.0\n", v
                );
            }
            add_no_work_message(
                "Your NVIDIA GPU lacks the needed compute capability"
            );
            return false;
        } 

        // for CUDA 2.3, we need to check the CUDA RT version.
        // Old BOINC clients report display driver version;
        // newer ones report CUDA RT version
        //
        if (!strcmp(plan_class, "cuda23")) {
            if (cp->cuda_version) {
                if (cp->cuda_version < 2030) {
                    add_no_work_message("CUDA version 2.3 needed");
                    return false;
                }
            } else if (cp->display_driver_version) {
                if (cp->display_driver_version < PLAN_CUDA23_MIN_DRIVER_VERSION) {
                    sprintf(buf, "NVIDIA display driver %d or later needed",
                        PLAN_CUDA23_MIN_DRIVER_VERSION
                    );
                    add_no_work_message(buf);
                    return false;
                }
            } else {
                // pre-6.10 Linux clients report neither CUDA nor driver
                // version; they'll end up here
                //
                add_no_work_message("CUDA version 2.3 needed");
                return false;
            }
#ifdef PLAN_CUDA23_MIN_RAM
            if (cp->prop.dtotalGlobalMem < PLAN_CUDA23_MIN_RAM) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] CUDA23 mem %d < %d\n",
                        cp->prop.dtotalGlobalMem, PLAN_CUDA23_MIN_RAM
                    );
                }
                sprintf(buf,
                    "Your NVIDIA GPU has insufficient memory (need %.0fMB)",
                    PLAN_CUDA23_MIN_RAM/MEGA
                );
                add_no_work_message(buf);
                return false;
            }
#endif
        } else {
            if (cp->display_driver_version && cp->display_driver_version < PLAN_CUDA_MIN_DRIVER_VERSION) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] NVIDIA driver version %d < PLAN_CUDA_MIN_DRIVER_VERSION\n",
                        cp->display_driver_version
                    );
                }
                sprintf(buf, "NVIDIA driver version %d or later needed",
                    PLAN_CUDA_MIN_DRIVER_VERSION
                );
                add_no_work_message(buf);
                return false;
            }
        }

        if (cp->prop.dtotalGlobalMem < PLAN_CUDA_MIN_RAM) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] CUDA mem %d < %d\n",
                    cp->prop.dtotalGlobalMem, PLAN_CUDA_MIN_RAM
                );
            }
            sprintf(buf,
                "Your NVIDIA GPU has insufficient memory (need %.0fMB)",
                PLAN_CUDA_MIN_RAM/MEGA
            );
            add_no_work_message(buf);
            return false;
        }

        hu.flops = cp->flops_estimate();
        if (!strcmp(plan_class, "cuda23")) {
            hu.flops *= 1.01;
        }

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
        return true;
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
        return true;
    } else if (!strcmp(plan_class, "sse3")) {
        // the following is for an app that requires a processor with SSE3,
        // and will run 10% faster if so
        //
        downcase_string(sreq.host.p_features);
        if (!strstr(sreq.host.p_features, "sse3")) {
            add_no_work_message("Your CPU lacks SSE3");
            return false;
        }
        hu.avg_ncpus = 1;
        hu.max_ncpus = 1;
        hu.flops = 1.1*sreq.host.p_fpops;
        return true;
    }
    log_messages.printf(MSG_CRITICAL,
        "Unknown plan class: %s\n", plan_class
    );
    return false;
}

// the following is used to enforce limits on in-progress jobs
// for GPUs and CPUs (see handle_request.cpp)
//
bool app_plan_uses_gpu(const char* plan_class) {
    if (!strcmp(plan_class, "cuda")) {
        return true;
    }
    return false;
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

    retval = wu_is_infeasible_fast(
        wu, wu_result.res_server_state, wu_result.res_priority,
        wu_result.res_report_deadline,
        *app, *bavp
    );
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
    // add min/actual to score
    // (this favors sending jobs that need lots of procs to GPUs that have them)
    // IF YOU USE THIS, USE THE PART IN wu_is_infeasible_custom() ALSO
    //
    if (!strcmp(app->name, "foobar") && bavp->host_usage.ncudas) {
        int n = g_request->coproc_cuda->prop.multiProcessorCount;
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

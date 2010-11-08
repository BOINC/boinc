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
//      Decide whether host can run a job using a particular app version.
//      In addition it can:
//      - set the app version's resource usage and/or FLOPS rate estimate
//          (by assigning to bav.host_usage)
//      - modify command-line args
//          (by assigning to bav.host_usage.cmdline)
//      - set the job's FLOPS count
//          (by assigning to wu.rsc_fpops_est)
//
// app_plan()
//      Decide whether host can use an app version,
//      and if so what resources it will use
//
// app_plan_uses_gpu():
//      Which plan classes use GPUs
//
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
#include "util.h"

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
    // example: if WU name contains "_v1", don't use CUDA app
    // Note: this is slightly suboptimal.
    // If the host is able to accept both GPU and CPU jobs,
    // we'll skip this job rather than send it for the CPU.
    // Fixing this would require a big architectural change.
    //
    if (strstr(wu.name, "_v1") && bav.host_usage.ncudas) {
        return true;
    }
#endif
#if 0
    // example: for CUDA app, wu.batch is the minimum number of processors.
    // Don't send if #procs is less than this.
    //
    if (!strcmp(app.name, "foobar") && bav.host_usage.ncudas) {
        int n = g_request->coproc_cuda->prop.multiProcessorCount;
        if (n < wu.batch) {
           return true;
        }
    }
#endif
#if 0
    // example: if CUDA app and WU name contains ".vlar", don't send
    //
    if (bav.host_usage.ncudas) {
        if (strstr(wu.name, ".vlar")) {
            return true;
        }
    }
#endif
    return false;
}

// Suppose we have a computation that uses two devices alternately.
// The devices have speeds s1 and s2.
// The fraction of work done on device 1 is frac.
//
// This function returns:
// 1) the overall speed
// 2) the utilization of device 1, which is always in (0, 1).
//
static inline void coproc_perf(
    double s1, double s2, double frac,
    double& speed, double& u1
) {
    double y = (frac*s2 + (1-frac)*s1);
    speed = s1*s2/y;
        // do the math
    u1 = frac*s2/y;
}

// the following is for an app that can use anywhere from 1 to 64 threads
//
static inline bool app_plan_mt(
    SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu
) {
    double ncpus = g_wreq->effective_ncpus;
        // number of usable CPUs, taking user prefs into account
    int nthreads = (int)ncpus;
    if (nthreads > 64) nthreads = 64;
    hu.avg_ncpus = nthreads;
    hu.max_ncpus = nthreads;
    sprintf(hu.cmdline, "--nthreads %d", nthreads);
    hu.projected_flops = sreq.host.p_fpops*hu.avg_ncpus*.99;
        // the .99 ensures that on uniprocessors a sequential app
        // will be used in preferences to this
    hu.peak_flops = sreq.host.p_fpops*hu.avg_ncpus;
    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] %s Multi-thread app projected %.2fGS\n",
            plan_class, hu.projected_flops/1e9
        );
    }
    return true;
}

GPU_REQUIREMENTS ati_requirements;

static bool ati_check(COPROC_ATI& c, HOST_USAGE& hu,
    int min_driver_version,
    bool need_amd_libs,
    double min_ram,
    double ndevs,       // # of GPUs used; can be fractional
    double cpu_frac,    // fraction of FLOPS performed by CPU
    double flops_scale
) {
    ati_requirements.update(min_driver_version, min_ram);

    if (need_amd_libs) {
        if (!c.amdrt_detected) {
            return false;
        }
    } else {
        if (!c.atirt_detected) {
            return false;
        }
    }
    if (c.version_num < min_driver_version) {
        return false;
    }
    if (c.attribs.localRAM*MEGA < min_ram) {
        return false;
    }

    hu.gpu_ram = min_ram;
    hu.natis = ndevs;

    coproc_perf(
        g_request->host.p_fpops,
        hu.natis*c.peak_flops(),
        cpu_frac,
        hu.projected_flops,
        hu.avg_ncpus
    );
    hu.peak_flops = hu.natis*c.peak_flops() + hu.avg_ncpus*g_request->host.p_fpops;
    hu.max_ncpus = hu.avg_ncpus;
    hu.projected_flops *= flops_scale;
    return true;
}

#define ATI_MIN_RAM 250*MEGA
static inline bool app_plan_ati(
    SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu
) {
    COPROC_ATI& c = sreq.coprocs.ati;
    if (!c.count) {
        return false;
    }

    if (!strcmp(plan_class, "ati")) {
        if (!ati_check(c, hu,
            1000000,
            true,
            ATI_MIN_RAM,
            1, .01,
            1
        )) {
            return false;
        }
    }

    if (!strcmp(plan_class, "ati13amd")) {
        if (!ati_check(c, hu,
            1003000,
            true,
            ATI_MIN_RAM,
            1, .01,
            1.01
        )) {
            return false;
        }
    }

    if (!strcmp(plan_class, "ati13ati")) {
        if (!ati_check(c, hu,
            1003186,
            false,
            ATI_MIN_RAM,
            1, .01,
            1.02
        )) {
            return false;
        }
    }

    if (!strcmp(plan_class, "ati14")) {
        if (!ati_check(c, hu,
            1004000,
            false,
            ATI_MIN_RAM,
            1, .01,
            1.03
        )) {
            return false;
        }
    }
    if (!strcmp(plan_class, "ati_opencl")) {
        // OpenCL not yet supported in standard ATI drivers
        return false;
    }

    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] %s ATI app projected %.2fG peak %.2fG %.3f CPUs\n",
            plan_class,
            hu.projected_flops/1e9,
            hu.peak_flops/1e9,
            hu.avg_ncpus
        );
    }
    return true;
}

GPU_REQUIREMENTS cuda_requirements;

#define CUDA_MIN_DRIVER_VERSION         17700
#define CUDA23_MIN_CUDA_VERSION         2030
#define CUDA23_MIN_DRIVER_VERSION       19038
#define CUDA3_MIN_CUDA_VERSION          3000
#define CUDA3_MIN_DRIVER_VERSION        19500
#define CUDA_OPENCL_MIN_DRIVER_VERSION  19713

static bool cuda_check(COPROC_CUDA& c, HOST_USAGE& hu,
    int min_cc, int max_cc,
    int min_cuda_version, int min_driver_version,
    double min_ram,
    double ndevs,       // # of GPUs used; can be fractional
    double cpu_frac,    // fraction of FLOPS performed by CPU
    double flops_scale
) {
    int cc = c.prop.major*100 + c.prop.minor;
    if (cc < min_cc) return false;
    if (max_cc && cc >= max_cc) return false;

    cuda_requirements.update(min_driver_version, min_ram);

    // Old BOINC clients report display driver version;
    // newer ones report CUDA RT version.
    // Some Linux doesn't return either.
    //
    if (!c.cuda_version && !c.display_driver_version) {
        return false;
    }
    if (c.cuda_version) {
        if (min_cuda_version && (c.cuda_version < min_cuda_version)) {
            return false;
        }
    }
    if (c.display_driver_version) {
        if (min_driver_version && (c.display_driver_version < min_driver_version)) {
            return false;
        }
    }
    if (c.prop.dtotalGlobalMem < min_ram) {
        return false;
    }

    hu.gpu_ram = min_ram;
    hu.ncudas = ndevs;

    coproc_perf(
        g_request->host.p_fpops,
        hu.ncudas*c.peak_flops(),
        cpu_frac,
        hu.projected_flops,
        hu.avg_ncpus
    );
    hu.peak_flops = hu.ncudas*c.peak_flops() + hu.avg_ncpus*g_request->host.p_fpops;
    hu.max_ncpus = hu.avg_ncpus;
    hu.projected_flops *= flops_scale;
    return true;
}

// the following is for an app that uses an NVIDIA GPU
//
static inline bool app_plan_cuda(
    SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu
) {
    COPROC_CUDA& c = sreq.coprocs.cuda;
    if (!c.count) {
        return false;
    }

    // Macs require 6.10.28
    //
    if (strstr(sreq.host.os_name, "Darwin") && (sreq.core_client_version < 61028)) {
        return false;
    }

    // for CUDA 2.3, we need to check the CUDA RT version.
    // Old BOINC clients report display driver version;
    // newer ones report CUDA RT version
    //
    if (!strcmp(plan_class, "cuda_fermi")) {
        if (!cuda_check(c, hu,
            200, 0,
            CUDA3_MIN_CUDA_VERSION, CUDA3_MIN_DRIVER_VERSION,
            384*MEGA,
            1, .01, 1.02
        )) {
            return false;
        }
    } else if (!strcmp(plan_class, "cuda23")) {
        if (!cuda_check(c, hu,
            100,
            200,    // change to zero if app is compiled to byte code
            CUDA23_MIN_CUDA_VERSION, CUDA23_MIN_DRIVER_VERSION,
            384*MEGA,
            1, .01, 1.01
        )) {
            return false;
        }
    } else if (!strcmp(plan_class, "cuda_opencl")) {
        if (!cuda_check(c, hu,
            100, 0,
            0, CUDA_OPENCL_MIN_DRIVER_VERSION,
            384*MEGA,
            1, .01, 1
        )) {
            return false;
        }
    } else if (!strcmp(plan_class, "cuda")) {
        if (!cuda_check(c, hu,
            100,
            200,    // change to zero if app is compiled to byte code
            0, CUDA_MIN_DRIVER_VERSION,
            254*MEGA,
            1, .01, 1
        )) {
            return false;
        }
    } else {
        log_messages.printf(MSG_CRITICAL,
            "UNKNOWN PLAN CLASS %s\n", plan_class
        );
        return false;
    }

    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] %s app projected %.2fG peak %.2fG %.3f CPUs\n",
            plan_class,
            hu.projected_flops/1e9,
            hu.peak_flops/1e9,
            hu.avg_ncpus
        );
    }
    return true;
}

// The following is for a non-CPU-intensive application.
// Say that we'll use 1% of a CPU.
// This will cause the client (6.7+) to run it at non-idle priority
//
static inline bool app_plan_nci(
    SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu
) {
    hu.avg_ncpus = .01;
    hu.max_ncpus = .01;
    hu.projected_flops = sreq.host.p_fpops*1.01;
        // The *1.01 is needed to ensure that we'll send this app
        // version rather than a non-plan-class one
    hu.peak_flops = sreq.host.p_fpops*.01;
    return true;
}

// the following is for an app that requires a processor with SSE3,
// and will run 10% faster if so
//
static inline bool app_plan_sse3(
    SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu
) {
    downcase_string(sreq.host.p_features);
    if (!strstr(sreq.host.p_features, "sse3")) {
        //add_no_work_message("Your CPU lacks SSE3");
        return false;
    }
    hu.avg_ncpus = 1;
    hu.max_ncpus = 1;
    hu.projected_flops = 1.1*sreq.host.p_fpops;
    hu.peak_flops = sreq.host.p_fpops;
    return true;
}

static inline bool app_plan_opencl_nvidia(
    SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu
) {
    return false;
}

static inline bool app_plan_opencl_ati(
    SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu
) {
    return false;
}

// app planning function.
// See http://boinc.berkeley.edu/trac/wiki/AppPlan
//
bool app_plan(SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu) {
    if (!strcmp(plan_class, "mt")) {
        return app_plan_mt(sreq, plan_class, hu);
    } else if (strstr(plan_class, "ati")) {
        return app_plan_ati(sreq, plan_class, hu);
    } else if (strstr(plan_class, "cuda")) {
        return app_plan_cuda(sreq, plan_class, hu);
    } else if (!strcmp(plan_class, "nci")) {
        return app_plan_nci(sreq, plan_class, hu);
    } else if (!strcmp(plan_class, "sse3")) {
        return app_plan_sse3(sreq, plan_class, hu);
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
    if (strstr(plan_class, "cuda")) {
        return true;
    }
    if (strstr(plan_class, "ati")) {
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
    bavp = get_app_version(wu, true, false);
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
    if (bavp->reliable && (wu_result.need_reliable)) {
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
    score += bavp->host_usage.projected_flops/1e9;

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

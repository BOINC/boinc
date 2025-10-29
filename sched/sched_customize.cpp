// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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
//      TODO: get rid of this, and just use XML spec
//
//
// WARNING: if you modify this file, you must prevent it from
// being overwritten the next time you update BOINC source code.
// You can either:
// 1) write-protect this file, or
// 2) put this in a differently-named file and change the Makefile.am
//    (and write-protect that)
// In either case, put your version under source-code control, e.g. SVN
#include "config.h"

#include <string>

using std::string;

#include "str_util.h"
#include "util.h"

#include "sched_check.h"
#include "sched_config.h"
#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_score.h"
#include "sched_shmem.h"
#include "sched_version.h"
#include "sched_customize.h"
#include "plan_class_spec.h"

#ifndef ATI_MIN_RAM
#define ATI_MIN_RAM 256*MEGA
#endif

#ifndef OPENCL_ATI_MIN_RAM
#define OPENCL_ATI_MIN_RAM 256*MEGA
#endif

#ifndef OPENCL_INTEL_GPU_MIN_RAM
#define OPENCL_INTEL_GPU_MIN_RAM 256*MEGA
#endif

#ifndef OPENCL_APPLE_GPU_MIN_RAM
#define OPENCL_APPLE_GPU_MIN_RAM 256*MEGA
#endif

#ifndef CUDA_MIN_RAM
#define CUDA_MIN_RAM 256*MEGA
#endif

#ifndef CUDAFERMI_MIN_RAM
#define CUDAFERMI_MIN_RAM 384*MEGA
#endif

#ifndef CUDA23_MIN_RAM
#define CUDA23_MIN_RAM  384*MEGA
#endif

#ifndef OPENCL_NVIDIA_MIN_RAM
#define OPENCL_NVIDIA_MIN_RAM CUDA_MIN_RAM
#endif

PLAN_CLASS_SPECS plan_class_specs;

/* is there a plan class spec that restricts the workunit (or batch) */
bool wu_restricted_plan_class;

GPU_REQUIREMENTS gpu_requirements[NPROC_TYPES];

bool wu_is_infeasible_custom(
    WORKUNIT& wu,
    APP& /*app*/,
    BEST_APP_VERSION& bav
) {
#if 0
    // example 1: if WU name contains "_v1", don't use GPU apps.
    // Note: this is slightly suboptimal.
    // If the host is able to accept both GPU and CPU jobs,
    // we'll skip this job rather than send it for the CPU.
    // Fixing this would require a big architectural change.
    //
    if (strstr(wu.name, "_v1") && bav.host_usage.uses_gpu()) {
        return true;
    }
#endif
#if 0
    // example 2: for NVIDIA GPU app,
    // wu.batch is the minimum number of GPU processors.
    // Don't send if #procs is less than this.
    //
    if (!strcmp(app.name, "foobar") && bav.host_usage.proc_type == PROC_TYPE_NVIDIA_GPU) {
        int n = g_request->coprocs.nvidia.prop.multiProcessorCount;
        if (n < wu.batch) {
           return true;
        }
    }
#endif
#if 0
    // example 3: require that wu.opaque = user.donated
    //
    if (wu.opaque && wu.opaque != g_reply->user.donated) {
        return true;
    }
#endif

    // WU restriction
    if (wu_restricted_plan_class) {
        if (plan_class_specs.classes.size() > 0) {
            if (plan_class_specs.wu_is_infeasible(bav.avp->plan_class, &wu)) {
                return true;
            }
        }
    }

    return false;
}

#ifndef isnum
#define isnum(x) (((x)>='0') && ((x)<='9'))
#endif

#ifndef isnumorx
#define isnumorx(x) (isnum(x) || ((x=='X') || (x=='x')))
#endif

// the following is for an app that can use anywhere from 1 to 64 threads
//
static inline bool app_plan_mt(SCHEDULER_REQUEST&, HOST_USAGE& hu) {
    double ncpus = g_wreq->effective_ncpus;
        // number of usable CPUs, taking user prefs into account
    if (ncpus < 2) return false;
    int nthreads = (int)ncpus;
    if (nthreads > 64) nthreads = 64;
    hu.avg_ncpus = nthreads;
    sprintf(hu.cmdline, "--nthreads %d", nthreads);
    hu.projected_flops = capped_host_fpops()*hu.avg_ncpus*.99;
        // the .99 ensures that on uniprocessors a sequential app
        // will be used in preferences to this
    hu.peak_flops = capped_host_fpops()*hu.avg_ncpus;
    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] Multi-thread app projected %.2fGS\n",
            hu.projected_flops/1e9
        );
    }
    return true;
}

bool app_plan_opencl_cpu_intel(SCHEDULER_REQUEST& sreq, HOST_USAGE& hu) {
    OPENCL_CPU_PROP ocp;
    if (!sreq.host.get_opencl_cpu_prop("intel", ocp)) {
        return false;
    }
    return app_plan_mt(sreq, hu);
}

static bool ati_check(COPROC_ATI& c, HOST_USAGE& hu,
    int min_driver_version,
    bool need_amd_libs,
    double min_ram,
    double ndevs,       // # of GPUs used; can be fractional
    double cpu_frac,    // fraction of FLOPS performed by CPU
    double flops_scale,
    int min_hd_model=0
) {
    if (c.version_num) {
        gpu_requirements[PROC_TYPE_AMD_GPU].update(min_driver_version, min_ram);
    }

    if (min_hd_model) {
        char *p=strcasestr(c.name,"hd");
        if (p) {
            p+=2;
            while (p && !isnum(*p)) p++;
            char modelnum[64];
            int i=0;
            while ((i<63) && p[i] && isnumorx(p[i])) {
                modelnum[i]=p[i];
                if ((modelnum[i]=='x') || (modelnum[i]=='X')) {
                    modelnum[i]='0';
                }
                i++;
            }
            modelnum[i]=0;
            i=atoi(modelnum);
            if (i<min_hd_model) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] Requires ATI HD%4d+.  Found HD%4d\n",
                        min_hd_model, i
                    );
                }
                return false;
            }
        }
    }


    if (need_amd_libs) {
        if (!c.amdrt_detected) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] AMD run time libraries not found\n"
                );
            }
            return false;
        }
    } else {
        if (!c.atirt_detected) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] ATI run time libraries not found\n"
                );
            }
            return false;
        }
    }
    if (c.version_num < min_driver_version) {
        if (config.debug_version_select) {
            int app_major=min_driver_version/10000000;
            int app_minor=(min_driver_version%10000000)/10000;
            int app_rev=(min_driver_version%10000);
            int dev_major=c.version_num/10000000;
            int dev_minor=(c.version_num%10000000)/10000;
            int dev_rev=(c.version_num%10000);
            log_messages.printf(MSG_NORMAL,
                "[version] Bad display driver revision %d.%d.%d<%d.%d.%d.\n",
                dev_major,dev_minor,dev_rev,app_major,app_minor,app_rev
            );
        }
        return false;
    }
    if (c.available_ram < min_ram) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Insufficient GPU RAM %f>%f.\n",
                min_ram, c.available_ram
            );
        }
        return false;
    }

    hu.gpu_ram = min_ram;
    hu.proc_type = PROC_TYPE_AMD_GPU;
    hu.gpu_usage = ndevs;

    coproc_perf(
        capped_host_fpops(),
        flops_scale * hu.gpu_usage*c.peak_flops,
        cpu_frac,
        hu.projected_flops,
        hu.avg_ncpus
    );
    hu.peak_flops = hu.gpu_usage*c.peak_flops + hu.avg_ncpus*capped_host_fpops();
    return true;
}

static inline bool app_plan_ati(
    SCHEDULER_REQUEST& sreq, const char* plan_class, HOST_USAGE& hu
) {
    COPROC_ATI& c = sreq.coprocs.ati;
    if (!c.count) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,"[version] Host has no ATI GPUs\n");
        }
        return false;
    }

    if (!strcmp(plan_class, "ati")) {
        if (!ati_check(c, hu,
            ati_version_int(1, 0, 0),
            true,
            ATI_MIN_RAM,
            1,
            .01,
            .20
        )) {
            return false;
        }
    }

    if (!strcmp(plan_class, "ati13amd")) {
        if (!ati_check(c, hu,
            ati_version_int(1, 3, 0),
            true,
            ATI_MIN_RAM,
            1, .01,
            .21
        )) {
            return false;
        }
    }

    if (!strcmp(plan_class, "ati13ati")) {
        if (!ati_check(c, hu,
            ati_version_int(1, 3, 186),
            false,
            ATI_MIN_RAM,
            1, .01,
            .22
        )) {
            return false;
        }
    }

    if (!strcmp(plan_class, "ati14")) {
        if (!ati_check(c, hu,
            ati_version_int(1, 4, 0),
            false,
            ATI_MIN_RAM,
            1, .01,
            .23
        )) {
            return false;
        }
    }

#ifdef SETIATHOME
    // ati_opencl_<ver> plan classes are for running
    // opencl ati apps on pre-v7 boinc core clients
    if (!strcmp(plan_class, "ati_opencl_100")) {
        if (!ati_check(c, hu,
            ati_version_int(1, 4, 1386),
            false,
            OPENCL_ATI_MIN_RAM,
            1, .01,
            .14,
            4600
        )) {
            return false;
        }
    }
#endif

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

// Change values for these parameters in shed_customize.h!
#ifndef CUDA_MIN_DRIVER_VERSION
#define CUDA_MIN_DRIVER_VERSION             17700
#endif

#ifndef CUDA23_MIN_CUDA_VERSION
#define CUDA23_MIN_CUDA_VERSION             2030
#endif

#ifndef CUDA23_MIN_DRIVER_VERSION
#define CUDA23_MIN_DRIVER_VERSION           19038
#endif

#ifndef CUDA3_MIN_CUDA_VERSION
#define CUDA3_MIN_CUDA_VERSION              3000
#endif

#ifndef CUDA3_MIN_DRIVER_VERSION
#define CUDA3_MIN_DRIVER_VERSION            19500
#endif

#ifndef CUDA_OPENCL_MIN_DRIVER_VERSION
#define CUDA_OPENCL_MIN_DRIVER_VERSION      19713
#endif

#ifndef CUDA_OPENCL_101_MIN_DRIVER_VERSION
#define CUDA_OPENCL_101_MIN_DRIVER_VERSION  28013
#endif

static bool cuda_check(COPROC_NVIDIA& c, HOST_USAGE& hu,
    int min_cc, int max_cc,
    int min_cuda_version, int min_driver_version,
    double min_ram,
    double ndevs,       // # of GPUs used; can be fractional
    double cpu_frac,    // fraction of FLOPS performed by CPU
    double flops_scale
) {
    int cc = c.prop.major*100 + c.prop.minor;
    if (min_cc && (cc < min_cc)) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] App requires compute capability > %d.%d (has %d.%d).\n",
                min_cc/100,min_cc%100,
                c.prop.major,c.prop.minor
            );
        }
        return false;
    }

    if (max_cc && cc >= max_cc) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] App requires compute capability <= %d.%d (has %d.%d).\n",
                max_cc/100,max_cc%100,
                c.prop.major,c.prop.minor
            );
        }
        return false;
    }

    if (c.display_driver_version) {
        gpu_requirements[PROC_TYPE_NVIDIA_GPU].update(min_driver_version, min_ram);
    }

    // Old BOINC clients report display driver version;
    // newer ones report CUDA RT version.
    // Some Linux doesn't return either.
    //
    if (!c.cuda_version && !c.display_driver_version) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Client did not provide cuda or driver version.\n"
            );
        }
        return false;
    }
    if (c.cuda_version) {
        if (min_cuda_version && (c.cuda_version < min_cuda_version)) {
            if (config.debug_version_select) {
                double app_version=(double)(min_cuda_version/1000)+(double)(min_cuda_version%100)/100.0;
                double client_version=(double)(c.cuda_version/1000)+(double)(c.cuda_version%100)/100.0;
                log_messages.printf(MSG_NORMAL,
                    "[version] Bad CUDA version %f>%f.\n",
                    app_version, client_version
                );
            }
            return false;
        }
    }
    if (c.display_driver_version) {
        if (min_driver_version && (c.display_driver_version < min_driver_version)) {
            if (config.debug_version_select) {
                double app_version=(double)(min_driver_version)/100.0;
                double client_version=(double)(c.display_driver_version)/100.0;
                log_messages.printf(MSG_NORMAL,
                    "[version] Bad display driver revision %f>%f.\n",
                    app_version, client_version
                );
            }
            return false;
        }
    }
    if (c.available_ram < min_ram) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Insufficient GPU RAM %f>%f.\n",
                min_ram, c.available_ram
            );
        }
        return false;
    }

    hu.gpu_ram = min_ram;
    hu.proc_type = PROC_TYPE_NVIDIA_GPU;
    hu.gpu_usage = ndevs;

    coproc_perf(
        capped_host_fpops(),
        flops_scale * hu.gpu_usage*c.peak_flops,
        cpu_frac,
        hu.projected_flops,
        hu.avg_ncpus
    );
    hu.peak_flops = hu.gpu_usage*c.peak_flops + hu.avg_ncpus*capped_host_fpops();
    return true;
}

// the following is for an app that uses an NVIDIA GPU
//
static inline bool app_plan_nvidia(
    SCHEDULER_REQUEST& sreq, const char* plan_class, HOST_USAGE& hu
) {
    COPROC_NVIDIA& c = sreq.coprocs.nvidia;
    if (!c.count) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Host has no NVIDIA GPUs.\n");
        }
        return false;
    }

    // Macs require 6.10.28
    //
    if (strstr(sreq.host.os_name, "Darwin") && (sreq.core_client_version < 61028)) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] CUDA on MacOS requires BOINC 6.10.28 or higher.\n");
        }
        return false;
    }

    // for CUDA 2.3, we need to check the CUDA RT version.
    // Old BOINC clients report display driver version;
    // newer ones report CUDA RT version
#ifdef SETIATHOME
    // cuda_opencl_<ver> plan classes are for running opencl apps on
    // pre-boinc-v7 core clients. May be useful for other projects
    //
    if (!strcmp(plan_class, "cuda_opencl_100")) {
        if (!cuda_check(c, hu,
            100, 0,
            0,CUDA_OPENCL_MIN_DRIVER_VERSION,
            CUDA_MIN_RAM,
            1,
            .01,
            0.14
        )) {
            return false;
        }
    } else if (!strcmp(plan_class, "cuda_opencl_101")) {
        if (!cuda_check(c, hu,
            200, 0,
            0,CUDA_OPENCL_101_MIN_DRIVER_VERSION,
            CUDA_MIN_RAM,
            1,
            .01,
            0.14
        )) {
            return false;
        }
    } else
#endif // SETIATHOME
    if (!strcmp(plan_class, "cuda_fermi")) {
        if (!cuda_check(c, hu,
            200, 0,
            CUDA3_MIN_CUDA_VERSION, CUDA3_MIN_DRIVER_VERSION,
            CUDAFERMI_MIN_RAM,
            1,
            .01,
            .22
        )) {
            return false;
        }
    } else if (!strcmp(plan_class, "cuda23")) {
        if (!cuda_check(c, hu,
            100,
            200,    // change to zero if app is compiled to byte code
            CUDA23_MIN_CUDA_VERSION, CUDA23_MIN_DRIVER_VERSION,
            CUDA23_MIN_RAM,
            1,
            .01,
            .21
        )) {
            return false;
        }
    } else if (!strcmp(plan_class, "cuda")) {
        if (!cuda_check(c, hu,
            100,
            200,    // change to zero if app is compiled to byte code
            0, CUDA_MIN_DRIVER_VERSION,
            CUDA_MIN_RAM,
            1,
            .01,
            .20
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
static inline bool app_plan_nci(SCHEDULER_REQUEST&, HOST_USAGE& hu) {
    hu.avg_ncpus = .01;
    hu.projected_flops = capped_host_fpops()*1.01;
        // The *1.01 is needed to ensure that we'll send this app
        // version rather than a non-plan-class one
    hu.peak_flops = capped_host_fpops()*.01;
    return true;
}

// the following is for an app version that requires a processor with SSE3,
// and will run 10% faster than the non-SSE3 version
// NOTE: clients return "pni" instead of "sse3"
//
static inline bool app_plan_sse3(
    SCHEDULER_REQUEST& sreq, HOST_USAGE& hu
) {
    downcase_string(sreq.host.p_features);
    if (!strstr(sreq.host.p_features, "pni")) {
        // Pre-6.x clients report CPU features in p_model
        //
        if (!strstr(sreq.host.p_model, "pni")) {
            //add_no_work_message("Your CPU lacks SSE3");
            return false;
        }
    }
    hu.avg_ncpus = 1;
    hu.projected_flops = 1.1*capped_host_fpops();
    hu.peak_flops = capped_host_fpops();
    return true;
}

static inline bool opencl_check(
    COPROC& cp, HOST_USAGE& hu,
    int min_opencl_device_version,
    double min_global_mem_size,
    double ndevs,
    double cpu_frac,
    double flops_scale
) {
    if (cp.opencl_prop.opencl_device_version_int < min_opencl_device_version) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] [opencl_check] App requires OpenCL verion >= %d (has %d).\n",
                min_opencl_device_version,
                cp.opencl_prop.opencl_device_version_int
            );
        }
        return false;
    }

#ifdef SETIATHOME
    // fix for ATI drivers that report zero or negative global memory size
    // on some cards. Probably no longer necessary.
    if (cp.opencl_prop.global_mem_size < cp.opencl_prop.local_mem_size) {
        cp.opencl_prop.global_mem_size=cp.opencl_prop.local_mem_size;
    }
#endif

    if (cp.opencl_prop.global_mem_size && (cp.opencl_prop.global_mem_size < min_global_mem_size)) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version]  [opencl_check] Insufficient GPU RAM %f>%ld.\n",
                min_global_mem_size, cp.opencl_prop.global_mem_size
            );
        }
        return false;
    }

    hu.gpu_ram = min_global_mem_size;
    if (!strcmp(cp.type, proc_type_name_xml(PROC_TYPE_NVIDIA_GPU))) {
        hu.proc_type = PROC_TYPE_NVIDIA_GPU;
        hu.gpu_usage = ndevs;
    } else if (!strcmp(cp.type, proc_type_name_xml(PROC_TYPE_AMD_GPU))) {
        hu.proc_type = PROC_TYPE_AMD_GPU;
        hu.gpu_usage = ndevs;
    } else if (!strcmp(cp.type, proc_type_name_xml(PROC_TYPE_INTEL_GPU))) {
        hu.proc_type = PROC_TYPE_INTEL_GPU;
        hu.gpu_usage = ndevs;
    } else if (!strcmp(cp.type, proc_type_name_xml(PROC_TYPE_APPLE_GPU))) {
        hu.proc_type = PROC_TYPE_APPLE_GPU;
        hu.gpu_usage = ndevs;
    }

    coproc_perf(
        capped_host_fpops(),
        flops_scale * ndevs * cp.peak_flops,
        cpu_frac,
        hu.projected_flops,
        hu.avg_ncpus
    );
    hu.peak_flops = ndevs*cp.peak_flops + hu.avg_ncpus*capped_host_fpops();
    return true;
}


static inline bool app_plan_opencl(
    SCHEDULER_REQUEST& sreq, const char* plan_class, HOST_USAGE& hu
) {
    // opencl_*_<ver> plan classes check for a trailing integer which is
    // used as the opencl version number.  This is compatible with the old
    // opencl_nvidia_101 and opencl_ati_101 plan classes, but doesn't require
    // modifications if someone wants a opencl_nvidia_102 plan class.
    const char *p=plan_class+strlen(plan_class);
    while (isnum(p[-1])) {
        p--;
    }
    int ver=atoi(p);
    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] plan_class %s uses OpenCl version %d\n",
            plan_class,
            ver
        );
    }
    if (strstr(plan_class, "nvidia")) {
        COPROC_NVIDIA& c = sreq.coprocs.nvidia;
        if (!c.count) return false;
        if (!c.have_opencl) return false;
        if (strstr(plan_class,"opencl_nvidia") == plan_class) {
            return opencl_check(
                c, hu,
                ver,
                OPENCL_NVIDIA_MIN_RAM,
                1,
                .01,
                .14
            );
        } else {
            log_messages.printf(MSG_CRITICAL,
                "Unknown plan class: %s\n", plan_class
            );
            return false;
        }
    } else if (strstr(plan_class, "amd")) {
        COPROC_ATI& c = sreq.coprocs.ati;
        if (!c.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] [opencl] HOST has no ATI/AMD GPUs\n"
                );
            }
            return false;
        }

        if (!c.have_opencl) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] [opencl] GPU/Driver/BOINC revision doesn not support OpenCL\n"
                );
            }
            return false;
        }

        if (strstr(plan_class,"opencl_ati") == plan_class) {
            return opencl_check(
                c, hu,
                ver,
                OPENCL_ATI_MIN_RAM,
                1,
                .01,
                .14
            );
        } else {
            log_messages.printf(MSG_CRITICAL,
                "[version] [opencl] Unknown plan class: %s\n", plan_class
            );
            return false;
        }

    } else if (strstr(plan_class, "intel_gpu")) {
        COPROC_INTEL& c = sreq.coprocs.intel_gpu;
        if (!c.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] [opencl] HOST has no INTEL GPUs\n"
                );
            }
            return false;
        }

        if (!c.have_opencl) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] [opencl] GPU/Driver/BOINC revision doesn not support OpenCL\n"
                );
            }
            return false;
        }

        if (strstr(plan_class,"opencl_intel_gpu") == plan_class) {
            return opencl_check(
                c, hu,
                ver,
                OPENCL_INTEL_GPU_MIN_RAM,
                1,
                .1,
                .2
            );
        } else {
            log_messages.printf(MSG_CRITICAL,
                "[version] [opencl] Unknown plan class: %s\n", plan_class
            );
            return false;
        }
    } else if (strstr(plan_class, "apple_gpu")) {
        COPROC_APPLE& c = sreq.coprocs.apple_gpu;
        if (!c.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] [opencl] HOST has no Apple GPUs\n"
                );
            }
            return false;
        }

        if (!c.have_opencl) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] [opencl] GPU/Driver/BOINC revision doesn not support OpenCL\n"
                );
            }
            return false;
        }

        if (strstr(plan_class,"opencl_apple_gpu") == plan_class) {
            return opencl_check(
                c, hu,
                ver,
                OPENCL_APPLE_GPU_MIN_RAM,
                1,
                .1,
                .2
            );
        } else {
            log_messages.printf(MSG_CRITICAL,
                "[version] [opencl] Unknown plan class: %s\n", plan_class
            );
            return false;
        }

    // maybe add a clause for multicore CPU

    } else {
        log_messages.printf(MSG_CRITICAL,
            "[version] [opencl] Unknown plan class: %s\n", plan_class
        );
        return false;
    }
}

// handles vbox[32|64][_[mt]|[hwaccel]]
// "mt" is tailored to the needs of CERN:
// use 1 or 2 CPUs

static inline bool app_plan_vbox(
    SCHEDULER_REQUEST& sreq, const char* plan_class, HOST_USAGE& hu
) {
    bool can_use_multicore = true;

    // host must run 7.0+ client
    //
    if (sreq.core_client_major_version < 7) {
        add_no_work_message("BOINC client 7.0+ required for Virtualbox jobs");
        return false;
    }

    // host must have VirtualBox 3.2 or later
    //
    if (strlen(sreq.host.virtualbox_version) == 0) {
        add_no_work_message("VirtualBox is not installed");
        return false;
    }
    int n, maj, min, rel;
    n = sscanf(sreq.host.virtualbox_version, "%d.%d.%d", &maj, &min, &rel);
    if ((n != 3) || (maj < 3) || (maj == 3 and min < 2)) {
        add_no_work_message("VirtualBox version 3.2 or later is required");
        return false;
    }

    // host must have VM acceleration in order to run hwaccel jobs
    // NOTE: 64-bit VM's require hard acceleration extensions or they fail
    // to boot.
    //
    if (strstr(plan_class, "hwaccel") || strstr(plan_class, "64")) {
        if ((!strstr(sreq.host.p_features, "vmx") && !strstr(sreq.host.p_features, "svm"))
            || sreq.host.p_vm_extensions_disabled
        ) {
            add_no_work_message(
                "VirtualBox jobs require hardware acceleration support. Your "
                "processor does not support the required instruction set."
            );
            return false;
        }
    }

    // host must have VM acceleration in order to run multi-core jobs
    //
    if (strstr(plan_class, "mt")) {
        if ((!strstr(sreq.host.p_features, "vmx") && !strstr(sreq.host.p_features, "svm"))
            || sreq.host.p_vm_extensions_disabled
        ) {
            can_use_multicore = false;
        }
    }

    // only send the version for host's primary platform.
    // A Win64 host can't run a 32-bit VM app:
    // it will look in the 32-bit half of the registry and fail
    //
    PLATFORM* p = g_request->platforms.list[0];
    if (is_64b_platform(p->name)) {
        if (!strstr(plan_class, "64")) return false;
    } else {
        if (strstr(plan_class, "64")) return false;
    }

    double flops_scale = 1;
    hu.avg_ncpus = 1;
    if (strstr(plan_class, "mt")) {
        if (can_use_multicore) {
            // Use number of usable CPUs, taking user prefs into account
            double ncpus = g_wreq->effective_ncpus;
            hu.avg_ncpus = ncpus;
            sprintf(hu.cmdline, "--nthreads %f", ncpus);
        }
        // use the non-mt version rather than the mt version with 1 CPU
        //
        flops_scale = .99;
    }
    hu.projected_flops = flops_scale * capped_host_fpops()*hu.avg_ncpus;
    hu.peak_flops = capped_host_fpops()*hu.avg_ncpus;
    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] %s app projected %.2fG\n",
            plan_class, hu.projected_flops/1e9
        );
    }
    return true;
}

static inline bool app_plan_wsl(
    SCHEDULER_REQUEST& sreq, const char* plan_class, HOST_USAGE& hu
) {
    // no additional checks at the moment, just return true
    return true;
}

// if host can handle the plan class, populate host usage and return true
//
// See https://github.com/BOINC/boinc/wiki/AppPlan
//
bool app_plan(
    SCHEDULER_REQUEST& sreq, const char* plan_class, HOST_USAGE& hu,
    const WORKUNIT* wu
) {
    char buf[256];
    static bool check_plan_class_spec = true;
    static bool have_plan_class_spec = false;
    static bool bad_plan_class_spec = false;

    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] Checking plan class '%s' check %d have %d bad %d\n",
            plan_class,
            check_plan_class_spec,
            have_plan_class_spec,
            bad_plan_class_spec
        );
    }

    if (check_plan_class_spec) {
        check_plan_class_spec = false;
        safe_strcpy(buf, config.project_dir);
        safe_strcat(buf, "/plan_class_spec.xml");
        int retval = plan_class_specs.parse_file(buf);
        if (retval == ERR_FOPEN) {
            have_plan_class_spec = false;
        } else if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "Error parsing plan class spec file '%s': %s\n",
                buf, boincerror(retval)
            );
            bad_plan_class_spec = true;
        } else {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] reading plan classes from file '%s'\n", buf
                );
            }
            have_plan_class_spec = true;
        }
    }
    if (bad_plan_class_spec) {
        return false;
    }
    if (have_plan_class_spec) {
        return plan_class_specs.check(sreq, plan_class, hu, wu);
    }

    if (!strcmp(plan_class, "mt")) {
        return app_plan_mt(sreq, hu);
    } else if (strstr(plan_class, "opencl_cpu_intel")) {
        return app_plan_opencl_cpu_intel(sreq, hu);
    } else if (strstr(plan_class, "opencl") == plan_class) {
        return app_plan_opencl(sreq, plan_class, hu);
    } else if (strstr(plan_class, "ati") == plan_class) {
        return app_plan_ati(sreq, plan_class, hu);
    } else if (strstr(plan_class, "cuda")) {
        return app_plan_nvidia(sreq, plan_class, hu);
    } else if (!strcmp(plan_class, "nci")) {
        return app_plan_nci(sreq, hu);
    } else if (!strcmp(plan_class, "sse3")) {
        return app_plan_sse3(sreq, hu);
    } else if (strstr(plan_class, "vbox")) {
        return app_plan_vbox(sreq, plan_class, hu);
    } else if (strstr(plan_class, "wsl")) {
        return app_plan_wsl(sreq, plan_class, hu);
    }
    log_messages.printf(MSG_CRITICAL,
        "Unknown plan class: %s\n", plan_class
    );
    return false;
}

void handle_file_xfer_results() {
    for (unsigned int i=0; i<g_request->file_xfer_results.size(); i++) {
        RESULT& r = g_request->file_xfer_results[i];
        log_messages.printf(MSG_NORMAL,
            "completed file xfer %s\n", r.name
        );
        g_reply->result_acks.push_back(string(r.name));
    }
}

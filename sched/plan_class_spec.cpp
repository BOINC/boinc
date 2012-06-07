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

#include "util.h"
#include "sched_config.h"
#include "sched_customize.h"
#include "plan_class_spec.h"

using std::string;

int PLAN_CLASS_SPECS::parse_file(const char* path) {
#ifndef _USING_FCGI_
    FILE* f = fopen(path, "r");
#else
    FCGI_FILE *f = FCGI::fopen(path, "r");
#endif
    if (!f) return ERR_FOPEN;
    int retval = parse_specs(f);
    fclose(f);
    return retval;
}


bool PLAN_CLASS_SPEC::check(SCHEDULER_REQUEST& sreq, HOST_USAGE& hu) {
    COPROC* cpp = NULL;
    bool can_use_multicore = true;

    // set HOST_USAGE defaults
    //
    hu.ncudas = 0;
    hu.natis = 0;
    hu.gpu_ram = 0;
    hu.avg_ncpus = 1;
    hu.max_ncpus = 1;
    hu.projected_flops = sreq.host.p_fpops;
    hu.peak_flops = sreq.host.p_fpops;
    strcpy(hu.cmdline, "");

    // CPU features
    //
    // older clients report CPU features in p_model,
    // within square brackets
    //
    // the requested features are surrounded by spaces,
    // so we can look for them with strstr()
    //
    if (!cpu_features.empty()) {
        char buf[8192], buf2[512];
        sprintf(buf, " %s ", sreq.host.p_features);
        char* p = strrchr(sreq.host.p_model, '[');
        if (p) {
            sprintf(buf2, " %s", p+1);
            p = strchr(buf2, ']');
            if (p) {
                *p = 0;
            }
            strcat(buf2, " ");
            strcat(buf, buf2);
        }
        downcase_string(buf);

        for (unsigned int i=0; i<cpu_features.size(); i++) {
            if (!strstr(buf, cpu_features[i].c_str())) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] CPU lacks feature '%s' (got '%s')\n",
                        cpu_features[i].c_str(), sreq.host.p_features
                    );
                }
                return false;
            }
        }
    }

    // min NCPUS
    //
    if (min_ncpus && g_wreq->effective_ncpus < min_ncpus) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] not enough CPUs: %d < %f\n",
                g_wreq->effective_ncpus, min_ncpus
            );
        }
        return false;
    }

    // OS version
    //
    if (have_os_regex && regexec(&(os_regex), sreq.host.os_version, 0, NULL, 0)) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] OS version '%s' didn't match regexp\n",
                sreq.host.os_version
            );
        }
        return false;
    }

    if (virtualbox) {

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
        if (n != 3) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] can't parse vbox version\n"
                );
            }
            return false;
        }
        int v = maj*10000 + min*100 + rel;
        if (min_vbox_version && v < min_vbox_version) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] vbox version too low: %d < %d\n",
                    v, min_vbox_version
                );
            }
            return false;
        }
        if (max_vbox_version && v > max_vbox_version) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] vbox version too high: %d > %d\n",
                    v, max_vbox_version
                );
            }
            return false;
        }

        // host must have VM acceleration in order to run multi-core jobs
        //
        if (max_threads > 1) {
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
            if (!is64bit) return false;
        } else {
            if (is64bit) return false;
        }
    }

    // project-specific preference
    //
    if (have_project_prefs_regex && strlen(project_prefs_tag)) {
        char tag[256], value[256];
        char buf[65536];
        extract_venue(g_reply->user.project_prefs, g_reply->host.venue, buf);
        sprintf(tag,"<%s>",project_prefs_tag);
        bool p = parse_str(buf, tag, value, sizeof(value));
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] parsed project prefs setting '%s' : %s\n",
                project_prefs_tag, p?"true":"false"
            );
        }
        if (regexec(&(project_prefs_regex), value, 0, NULL, 0)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] project prefs setting '%s' prevents using plan class.\n",
                    project_prefs_tag
                );
            }
            return false;
        }
    }

    double gpu_ram = 0;
    int driver_version = 0;
    double gpu_utilization = 1.0;

    // user defined gpu_utilization
    //
    if (strlen(gpu_utilization_tag)) {
        char tag[256];
        char buf[65536];
        double v = 0;
        extract_venue(g_reply->user.project_prefs, g_reply->host.venue, buf);
        sprintf(tag,"<%s>",gpu_utilization_tag);
        bool p = parse_double(buf, tag, v);
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] parsed project prefs setting '%s' : %s : %f\n",
                gpu_utilization_tag, p?"true":"false", v
            );
        }
        if (v) {
            gpu_utilization = v;
        }
    }

    // AMD
    //
    if (!strcmp(gpu_type, "amd") || !strcmp(gpu_type, "ati")) {
        COPROC_ATI& cp = sreq.coprocs.ati;
        cpp = &cp;

        if (!cp.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] No ATI devices found\n"
                );
            }
            return false;
        }
        if (min_gpu_ram_mb) {
            ati_requirements.update(0, min_gpu_ram_mb * MEGA);
        }
        if (min_driver_version) {
            ati_requirements.update(abs(min_driver_version), 0);
        }

        if (need_ati_libs) {
            if (!cp.atirt_detected) {
                return false;
            }
        } else {
            if (!cp.amdrt_detected) {
                return false;
            }
        }

        cp.set_peak_flops();
        gpu_ram = cp.opencl_prop.global_mem_size;

        driver_version = 0;
        if (cp.have_cal) {
            int major, minor, release, scanned;
            scanned = sscanf(cp.version, "%d.%d.%d", &major, &minor, &release);
            if (scanned != 3) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] driver version '%s' couldn't be parsed\n",
                        cp.version
                    );
                }
                return false;
            } else {
                driver_version = release + minor * 10000 + major * 1000000;
            }
        } else {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] no CAL, driver version couldn't be determined\n"
                );
            }
        }

    // NVIDIA
    //
    } else if (!strcmp(gpu_type, "nvidia")) {
        COPROC_NVIDIA& cp = sreq.coprocs.nvidia;
        cpp = &cp;

        if (!cp.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] No NVIDIA devices found\n"
                );
            }
            return false;
        }

        if (min_gpu_ram_mb) {
            cuda_requirements.update(0, min_gpu_ram_mb * MEGA);
        }
        if (min_driver_version) {
            cuda_requirements.update(abs(min_driver_version), 0);
        }
        // compute capability
        int v = (cp.prop.major)*100 + cp.prop.minor;
        if (min_nvidia_compcap && min_nvidia_compcap > v) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] NVIDIA compute capability required min: %d, supplied: %d\n",
                    min_nvidia_compcap, v
                );
            }
            return false;
        }
        if (max_nvidia_compcap && max_nvidia_compcap < v) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] CUDA compute capability required max: %d, supplied: %d\n",
                    max_nvidia_compcap, v
                );
            }
            return false;
        }
        if (cuda) {
            // CUDA version
            if (min_cuda_version && min_cuda_version > cp.cuda_version) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] CUDA version required min: %d, supplied: %d\n",
                        min_cuda_version, cp.cuda_version
                    );
                }
                return false;
            }
            if (max_cuda_version && max_cuda_version < cp.cuda_version) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] CUDA version required max: %d, supplied: %d\n",
                        max_cuda_version, cp.cuda_version
                    );
                }
                return false;
            }
        }
        gpu_ram = cp.prop.totalGlobalMem;
        cp.set_peak_flops();
    }

    if (opencl) {
        // check for OpenCL at all
        if (!cpp->have_opencl) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] GPU doesn't support OpenCL\n"
                );
            }
            return false;
        }

        // OpenCL device version
        //
        if (min_opencl_version && min_opencl_version > cpp->opencl_prop.opencl_device_version_int) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] OpenCL device version required min: %d, supplied: %d\n",
                    min_opencl_version, cpp->opencl_prop.opencl_device_version_int
                );
            }
            return false;
        }

        gpu_ram = cpp->opencl_prop.global_mem_size;
    }

    // general GPU
    //
    if (strlen(gpu_type)) {

        // GPU RAM
        if (min_gpu_ram_mb && min_gpu_ram_mb * MEGA > gpu_ram) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] GPU RAM required min: %f, supplied: %f\n",
                    min_gpu_ram_mb * MEGA, gpu_ram
                );
            }
            return false;
        }

        // (display) driver version
        if (min_driver_version && driver_version) {
            if (min_driver_version > driver_version) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] driver version required min: %d, supplied: %d\n",
                        abs(min_driver_version), driver_version
                    );
                }
                return false;
            }
        }
        if (max_driver_version && driver_version) {
            if (max_driver_version < driver_version) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] driver version required max: %d, supplied: %d\n",
                        abs(max_driver_version), driver_version
                    );
                }
                return false;
            }
        }

        hu.gpu_ram = (gpu_ram_used_mb?gpu_ram_used_mb:min_gpu_ram_mb) * MEGA;

        double gpu_usage;

        // if ngpus < 0, set gpu_usage by the fraction of the total
        // video RAM a tasks would take
        // i.e. fill the device memory with tasks
        //
        if (ngpus < 0) {
            gpu_usage = (floor(gpu_ram/ hu.gpu_ram) * hu.gpu_ram) / gpu_ram ;
        } else if (ngpus > 0) {
            gpu_usage = ngpus * gpu_utilization;
        } else {
            gpu_usage = gpu_utilization;
        }

        coproc_perf(
            capped_host_fpops(),
            gpu_peak_flops_scale * gpu_usage * cpp->peak_flops,
            cpu_frac,
            hu.projected_flops,
            hu.avg_ncpus
        );
        if (avg_ncpus) {
            hu.avg_ncpus = avg_ncpus;
        }

        if (!strcmp(gpu_type, "amd") || !strcmp(gpu_type, "ati")) {
            hu.natis = gpu_usage;
        } else if (!strcmp(gpu_type, "nvidia")) {
            hu.ncudas = gpu_usage;
        }

    // CPU only
    //
    } else {
        if (avg_ncpus) {
            hu.avg_ncpus = avg_ncpus;
        } else {
            if (can_use_multicore) {
                if (max_threads > g_wreq->effective_ncpus) {
                    hu.avg_ncpus = g_wreq->effective_ncpus;
                } else {
                    hu.avg_ncpus = max_threads;
                }
            } else {
                hu.avg_ncpus = 1;
            }
        }

        hu.peak_flops = capped_host_fpops() * hu.avg_ncpus;
        hu.projected_flops = capped_host_fpops() * hu.avg_ncpus * projected_flops_scale;
    }
    hu.max_ncpus = hu.avg_ncpus;

    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] host_flops: %e, \tscale: %.2f, \tprojected_flops: %e, \tpeak_flops: %e\n",
            sreq.host.p_fpops, projected_flops_scale, hu.projected_flops,
            hu.peak_flops
        );
    }

    return true;

}


bool PLAN_CLASS_SPECS::check(
    SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu
) {
    for (unsigned int i=0; i<classes.size(); i++) {
        if (!strcmp(classes[i].name, plan_class)) {
            return classes[i].check(sreq, hu);
        }
    }
    log_messages.printf(MSG_CRITICAL, "Unknown plan class: %s\n", plan_class);
    return false;
}

int PLAN_CLASS_SPEC::parse(XML_PARSER& xp) {
    char buf[256];
    while (!xp.get_tag()) {
        if (xp.match_tag("/plan_class")) {
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("gpu_type", gpu_type, sizeof(gpu_type))) continue;
        if (xp.parse_bool("cuda", cuda)) continue;
        if (xp.parse_bool("cal", cal)) continue;
        if (xp.parse_bool("opencl", opencl)) continue;
        if (xp.parse_bool("virtualbox", virtualbox)) continue;
        if (xp.parse_bool("is64bit", is64bit)) continue;
        if (xp.parse_str("cpu_feature", buf, sizeof(buf))) {
            cpu_features.push_back(" " + (string)buf + " ");
            continue;
        }
        if (xp.parse_double("min_ncpus", min_ncpus)) continue;
        if (xp.parse_int("max_threads", max_threads)) continue;
        if (xp.parse_double("projected_flops_scale", projected_flops_scale)) continue;
        if (xp.parse_str("os_regex", buf, sizeof(buf))) {
            if (regcomp(&(os_regex), buf, REG_EXTENDED|REG_NOSUB) ) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
                return ERR_XML_PARSE;
            }
            have_os_regex = true;
            continue;
        }
        if (xp.parse_str("project_prefs_tag", project_prefs_tag, sizeof(project_prefs_tag))) continue;
        if (xp.parse_str("project_prefs_regex", buf, sizeof(buf))) {
            if (regcomp(&(project_prefs_regex), buf, REG_EXTENDED|REG_NOSUB) ) {
                log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", buf);
                return ERR_XML_PARSE;
            }
            have_project_prefs_regex = true;
            continue;
        }
        if (xp.parse_double("avg_ncpus", avg_ncpus)) continue;

        if (xp.parse_double("cpu_frac", cpu_frac)) continue;
        if (xp.parse_double("min_gpu_ram_mb", min_gpu_ram_mb)) continue;
        if (xp.parse_double("gpu_ram_used_mb", gpu_ram_used_mb)) continue;
        if (xp.parse_double("gpu_peak_flops_scale", gpu_peak_flops_scale)) continue;
        if (xp.parse_double("ngpus", ngpus)) continue;
        if (xp.parse_int("min_driver_version", min_driver_version)) continue;
        if (xp.parse_int("max_driver_version", max_driver_version)) continue;
        if (xp.parse_str("gpu_utilization_tag", gpu_utilization_tag, sizeof(gpu_utilization_tag))) continue;

        if (xp.parse_bool("need_ati_libs", need_ati_libs)) continue;

        if (xp.parse_int("min_nvidia_compcap", min_nvidia_compcap)) continue;
        if (xp.parse_int("max_nvidia_compcap", max_nvidia_compcap)) continue;

        if (xp.parse_int("min_cuda_version", min_cuda_version)) continue;
        if (xp.parse_int("max_cuda_version", max_cuda_version)) continue;

        if (xp.parse_int("min_opencl_version", min_opencl_version)) continue;
        if (xp.parse_int("max_opencl_version", max_opencl_version)) continue;

        if (xp.parse_int("min_vbox_version", min_vbox_version)) continue;
        if (xp.parse_int("max_vbox_version", max_vbox_version)) continue;
    }
    return ERR_XML_PARSE;
}

int PLAN_CLASS_SPECS::parse_specs(FILE* f) {
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(f);
    if (!xp.parse_start("plan_classes")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            fprintf(stderr, "PLAN_CLASS_SPECS::parse(): unexpected text %s\n", xp.parsed_tag);
            continue;
        }
        if (xp.match_tag("/plan_classes")) {
            return 0;
        }
        if (xp.match_tag("plan_class")) {
            PLAN_CLASS_SPEC pc;
            int retval = pc.parse(xp);
            if (retval) return retval;
            classes.push_back(pc);
        }
    }
    return ERR_XML_PARSE;
}


PLAN_CLASS_SPEC::PLAN_CLASS_SPEC() {
    strcpy(name, "");
    strcpy(gpu_type, "");
    cuda = false;
    cal = false;
    opencl = false;
    virtualbox = false;
    is64bit = false;
    min_ncpus = 0;
    max_threads = 1;
    projected_flops_scale = 1;
    have_os_regex = false;
    strcpy(project_prefs_tag, "");
    avg_ncpus = 0;

    cpu_frac = .1;
    min_gpu_ram_mb = 0;
    gpu_ram_used_mb = 0;
    gpu_peak_flops_scale = 1;
    ngpus = 1;
    min_driver_version = 0;
    max_driver_version = 0;
    strcpy(gpu_utilization_tag, "");

    need_ati_libs = false;

    min_nvidia_compcap = 0;
    max_nvidia_compcap = 0;
    min_cuda_version = 0;
    max_cuda_version = 0;

    min_opencl_version = 0;
    max_opencl_version = 0;

    min_vbox_version = 0;
    max_vbox_version = 0;
}

#ifdef PLAN_CLASS_TEST

int main() {
    SCHEDULER_REQUEST sreq;
    SCHEDULER_REPLY sreply;
    g_request = &sreq;
    g_reply = &sreply;
    g_wreq = &sreply.wreq;

    PLAN_CLASS_SPECS pcs;
    int retval = pcs.parse_file("plan_class_spec.xml.sample");
    if (retval) {
        printf("parse_file: %d\n");
        exit(1);
    }

    config.debug_version_select = true;

    HOST_USAGE hu;

    strcpy(sreq.host.p_features, "sse3");
    sreq.host.p_fpops =5e9;
    g_wreq->effective_ncpus = 4;
    if (1) {
        sreq.coprocs.nvidia.fake(18000, 512*MEGA, 490*MEGA, 1);
        sreq.coprocs.nvidia.opencl_prop.opencl_device_version_int = 0;
    } else {
        sreq.coprocs.nvidia.count = 0;
    }
    if (1) {
        sreq.coprocs.ati.fake(512*MEGA, 256*MEGA, 1);
        sreq.coprocs.ati.have_cal = true;
        sreq.coprocs.ati.opencl_prop.opencl_device_version_int = 0;
    } else {
        sreq.coprocs.ati.count = 0;
    }

    for (unsigned int i=0; i<pcs.classes.size(); i++) {
        bool b = pcs.check(sreq, pcs.classes[i].name, hu);
        if (b) {
            printf("%s: check succeeded\n", pcs.classes[i].name);
            printf("\tncudas: %f\n\tnatis: %f\n\tgpu_ram: %fMB\n\tavg_ncpus: %f\n\tprojected_flops: %fG\n\tpeak_flops: %fG\n",
                hu.ncudas,
                hu.natis,
                hu.gpu_ram/1e6,
                hu.avg_ncpus,
                hu.projected_flops/1e9,
                hu.peak_flops/1e9
            );
        } else {
            printf("%s: check failed\n", pcs.classes[i].name);
        }
    }
}
#endif

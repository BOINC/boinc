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


int PLAN_CLASS_SPECS::parse_file(char*path) {
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
    // fill HOST_USAGE with defaults
    hu.ncudas = 0;
    hu.natis = 0;
    hu.gpu_ram = 0;
    hu.avg_ncpus = 1;
    hu.max_ncpus = 1;
    hu.projected_flops = sreq.host.p_fpops;
    hu.peak_flops = sreq.host.p_fpops;
    hu.cmdline[0] = '\0';

#if 0
    char buf[256];
    sprintf(buf,
        "No work sent. "
        "See scheduler log messages on <a href=\"%s/host_sched_logs/%d/%d\">%s</a>",
        config.master_url, sreq.hostid / 1000, sreq.hostid, config.long_name
    );
    add_no_work_message(buf);
#endif

    // CPU features
    if (!cpu_features.empty()) {
        char features[512+3] = {0};
        char *c;
        strcpy(features, " ");
        if (strlen(sreq.host.p_features)) {
            strcat(features, sreq.host.p_features);
            strcat(features, " ");
        }
        if ((c = strrchr(sreq.host.p_model, '['))) {
            strcat(features, c+1);
            if((c = strrchr(features, ']')))
            *c = ' ';
        }
        downcase_string(features);
        for (unsigned int i=0; i<cpu_features.size(); i++) {
            if(!strstr(features, cpu_features[i].c_str())) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] CPU lacks feature '%s' (got '%s')\n",
                        cpu_features[i].c_str(), features
                    );
                }
                return false;
            }
        }
    }

    // Mac OS X versions - this version parsing only works for Mac OS
    if (min_macos_version || max_macos_version) {
        unsigned int v1,v2,v3;
        int v;
        if (sscanf(sreq.host.os_version, "%u.%u.%u",&v1,&v2,&v3) != 3) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] Couldn't parse OS version '%s'\n",
                    sreq.host.os_version
                );
            }
            return false;
        }
        v = v1 * 10000 + v2 * 100 + v3;
        if (min_macos_version && min_macos_version > v) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] OS version required min: %d, supplied: %d\n",
                    min_macos_version, v
                );
            }
            return false;
        }
        if (max_macos_version && max_macos_version < v) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] OS version required max: %d, supplied: %d\n",
                    max_macos_version, v
                );
            }
            return false;
        }
    }

    // OS version (regexp match)
    if(*os_version_string && regexec(&(os_version_regex), sreq.host.os_version, 0, NULL, 0)) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] Couldn't match OS version '%s' with required regexp '%s'\n",
                sreq.host.os_version, os_version_string
            );
        }
        return false;
    }

    // project-specific preference
    if (*project_prefs_tag) {
        char tag[256];
        char buf[65536];
        double v = 0;
        extract_venue(g_reply->user.project_prefs, g_reply->host.venue, buf);
        sprintf(tag,"<%s>",project_prefs_tag);
        bool p = parse_double(buf, tag, v);
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] parsed project prefs setting '%s' : %s : %f\n",
                project_prefs_tag, p?"true":"false", v
            );
        }
        if ((v < project_prefs_min) || (v > project_prefs_max)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] project prefs setting '%s' (%f) prevents using plan class.\n",
                    project_prefs_tag, v
                );
            }
            return false;
        }
    }

    double gpu_utilization = 1.0;

    // user defined gpu_utilization
    if (*gpu_utilization_tag) {
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

    // CUDA GPU
    if (type == 1) {
        COPROC_NVIDIA& cp = sreq.coprocs.nvidia;

        // devices
        if (!cp.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] No CUDA devices found\n"
                );
            }
            return false;
        }

        // update requirements
        if (min_gpu_ram_mb) {
            cuda_requirements.update(0, min_gpu_ram_mb * MEGA);
        }
        if (min_driver_version) {
            cuda_requirements.update(abs(min_driver_version), 0);
        }

        // GPU RAM
        if (min_gpu_ram_mb && min_gpu_ram_mb * MEGA > cp.prop.dtotalGlobalMem) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] GPU RAM required min: %f, supplied: %f\n",
                    min_gpu_ram_mb * MEGA, cp.prop.dtotalGlobalMem
                );
            }
            return false;
        }

        // compute capability
        int v = (cp.prop.major)*100 + cp.prop.minor;
        if (min_cuda_compcap && min_cuda_compcap > v) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] CUDA compute capability required min: %d, supplied: %d\n",
                    min_cuda_compcap, v
                );
            }
            return false;
        }
        if (max_cuda_compcap && max_cuda_compcap < v) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] CUDA compute capability required max: %d, supplied: %d\n",
                    max_cuda_compcap, v
                );
            }
            return false;
        }

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

        // (display) driver version
        if (min_driver_version && (min_driver_version > 0 || cp.display_driver_version)
            && abs(min_driver_version) > cp.display_driver_version
        ) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] driver version required min: %d, supplied: %d\n",
                    abs(min_driver_version), cp.display_driver_version
                );
            }
            return false;
        }
        if (max_driver_version && (max_driver_version > 0 || cp.display_driver_version)
            && abs(max_driver_version) < cp.display_driver_version
        ) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] driver version required max: %d, supplied: %d\n",
                    abs(max_driver_version), cp.display_driver_version
                );
            }
            return false;
        }

        hu.gpu_ram = (gpu_ram_used_mb?gpu_ram_used_mb:min_gpu_ram_mb) * MEGA;

        // if ngpus < 0, set ncudas by the fraction of the total video RAM a tasks would take
        // i.e. fill the device memory with tasks
        if (ngpus < 0) {
            hu.ncudas =
                (floor(cp.prop.dtotalGlobalMem / hu.gpu_ram) * hu.gpu_ram) /
                cp.prop.dtotalGlobalMem
            ;
        } else if (ngpus > 0) {
            hu.ncudas = ngpus * gpu_utilization;
        } else {
            hu.ncudas = gpu_utilization;
        }

        double cpu_frac = 1.0;
        if (gpu_flops && cpu_flops) {
            cpu_frac = cpu_flops / gpu_flops;
        } else if (speedup) {
            cpu_frac = 1.0 / speedup;
        }

        cp.set_peak_flops();
        coproc_perf(
            capped_host_fpops(),
            peak_flops_factor * hu.ncudas * cp.peak_flops,
            cpu_frac,
            hu.projected_flops,
            hu.avg_ncpus
        );
        if (avg_ncpus) {
            hu.avg_ncpus = avg_ncpus;
        }

    } // CUDA

    // NVidia OpenCL GPU
    else if (type == 3) {
        COPROC_NVIDIA& cp = sreq.coprocs.nvidia;

        // devices
        if (!cp.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] No NVidia devices found\n"
                );
            }
            return false;
        }

        // check for OpenCL at all
        if (!cp.have_opencl) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] NVidia device (or driver) doesn't support OpenCL\n"
                );
            }
            return false;
        }

        // update requirements
        if (min_gpu_ram_mb) {
            cuda_requirements.update(0, min_gpu_ram_mb * MEGA);
        }
        if (min_driver_version) {
            cuda_requirements.update(abs(min_driver_version), 0);
        }

        // GPU RAM
        if (min_gpu_ram_mb && min_gpu_ram_mb * MEGA > cp.opencl_prop.global_mem_size) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] OpenCL GPU RAM required min: %f, supplied: %lu\n",
                    min_gpu_ram_mb * MEGA, cp.opencl_prop.global_mem_size
                );
            }
            return false;
        }

        // OpenCL device version
        if (min_opencl_version && min_opencl_version > cp.opencl_prop.opencl_device_version_int) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] OpenCL device version required min: %d, supplied: %d\n",
                    min_opencl_version, cp.opencl_prop.opencl_device_version_int
                );
            }
            return false;
        }

        // (display) driver version
        if (min_driver_version && (min_driver_version > 0 || cp.display_driver_version)
            && abs(min_driver_version) > cp.display_driver_version
        ) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] driver version required min: %d, supplied: %d\n",
                    abs(min_driver_version), cp.display_driver_version
                );
            }
            return false;
        }
        if (max_driver_version && (max_driver_version > 0 || cp.display_driver_version)
            && abs(max_driver_version) < cp.display_driver_version
        ) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] driver version required max: %d, supplied: %d\n",
                    abs(max_driver_version), cp.display_driver_version
                );
            }
            return false;
        }

        hu.gpu_ram = (gpu_ram_used_mb?gpu_ram_used_mb:min_gpu_ram_mb) * MEGA;

        // if ngpus < 0, set ncudas by the fraction of the total video RAM a tasks would take
        // i.e. fill the device memory with tasks
        if (ngpus < 0) {
            hu.ncudas =
                (floor(cp.opencl_prop.global_mem_size / hu.gpu_ram) * hu.gpu_ram) /
                cp.opencl_prop.global_mem_size
            ;
        } else if (ngpus > 0) {
            hu.ncudas = ngpus * gpu_utilization;
        } else {
            hu.ncudas = gpu_utilization;
        }

        double cpu_frac = 1.0;
        if (gpu_flops && cpu_flops) {
            cpu_frac = cpu_flops / gpu_flops;
        } else if (speedup) {
            cpu_frac = 1.0 / speedup;
        }

        cp.set_peak_flops();
        coproc_perf(
            capped_host_fpops(),
            peak_flops_factor * hu.ncudas * cp.peak_flops,
            cpu_frac,
            hu.projected_flops,
            hu.avg_ncpus
        );
        if (avg_ncpus) {
            hu.avg_ncpus = avg_ncpus;
        }

    } // NVidia OpenCL


    // ATI OpenCL GPU
    else if (type == 4) {
        COPROC_ATI& cp = sreq.coprocs.ati;

        // devices
        if (!cp.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] No ATI devices found\n"
                );
            }
            return false;
        }

        // check for OpenCL at all
        if (!cp.have_opencl) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] ATI device (or driver) doesn't support OpenCL\n"
                );
            }
            return false;
        }

        // update requirements
        if (min_gpu_ram_mb) {
            ati_requirements.update(0, min_gpu_ram_mb * MEGA);
        }
        if (min_driver_version) {
            ati_requirements.update(abs(min_driver_version), 0);
        }

        // GPU RAM
        if (min_gpu_ram_mb && min_gpu_ram_mb * MEGA > cp.opencl_prop.global_mem_size) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] OpenCL GPU RAM required min: %f, supplied: %lu\n",
                    min_gpu_ram_mb * MEGA, cp.opencl_prop.global_mem_size
                );
            }
            return false;
        }

        // OpenCL device version
        if (min_opencl_version && min_opencl_version > cp.opencl_prop.opencl_device_version_int) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] OpenCL device version required min: %d, supplied: %d\n",
                    min_opencl_version, cp.opencl_prop.opencl_device_version_int
                );
            }
            return false;
        }

        // (display) driver version
        if (min_driver_version || max_driver_version) {
            int driver_version = 0;
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
            if (min_driver_version
                && (min_driver_version > 0 || driver_version)
                && abs(min_driver_version) > driver_version
            ) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] driver version required min: %d, supplied: %d\n",
                        abs(min_driver_version), driver_version
                    );
                }
                return false;
            }
            if (max_driver_version
                && (max_driver_version > 0 || driver_version)
                && abs(max_driver_version) < driver_version
            ) {
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

        // if ngpus < 0, set natis by the fraction of the total video RAM a tasks would take
        // i.e. fill the device memory with tasks
        if (ngpus < 0) {
            hu.natis =
                (floor(cp.opencl_prop.global_mem_size / hu.gpu_ram) * hu.gpu_ram) /
                cp.opencl_prop.global_mem_size
            ;
        } else if (ngpus > 0) {
            hu.natis = ngpus * gpu_utilization;
        } else {
            hu.natis = gpu_utilization;
        }

        double cpu_frac = 1.0;
        if (gpu_flops && cpu_flops) {
            cpu_frac = cpu_flops / gpu_flops;
        } else if (speedup) {
            cpu_frac = 1.0 / speedup;
        }

        cp.set_peak_flops();
        coproc_perf(
            capped_host_fpops(),
            peak_flops_factor * hu.natis * cp.peak_flops,
            cpu_frac,
            hu.projected_flops,
            hu.avg_ncpus
        );
        if (avg_ncpus) {
            hu.avg_ncpus = avg_ncpus;
        }

    } // ATI OpenCL


    else { // CPU only
        if (avg_ncpus) {
            hu.avg_ncpus = avg_ncpus;
        } else {
            hu.avg_ncpus = max_ncpus;
        }

        hu.peak_flops = capped_host_fpops() * hu.avg_ncpus;
        hu.projected_flops = capped_host_fpops() * hu.avg_ncpus * speedup;
    }
    hu.max_ncpus = max_ncpus;

    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] host_flops: %e, \tspeedup: %.2f, \tprojected_flops: %e, \tpeak_flops: %e, \tpeak_flops_factor: %.2f\n",
            sreq.host.p_fpops, speedup, hu.projected_flops, hu.peak_flops, peak_flops_factor
        );
    }

    return true;

} // check()


bool PLAN_CLASS_SPECS::check(SCHEDULER_REQUEST& sreq, char* plan_class, HOST_USAGE& hu) {
    for (unsigned int i=0; i<classes.size(); i++) {
        if (!strcmp(classes[i].name, plan_class)) {
            return classes[i].check(sreq, hu);
        }
    }
    log_messages.printf(MSG_CRITICAL, "Unknown plan class: %s\n", plan_class);
    return false;
} // check()


void PLAN_CLASS_SPECS::print(void) {
    for (unsigned int i=0; i<classes.size(); i++) {
        classes[i].print();
    }
}


void PLAN_CLASS_SPEC::print(void) {
    fprintf(stderr, "<plan_class>\n");
    fprintf(stderr, "  <name> %s </name>\n", name);
    fprintf(stderr, "  <type> %d </type>\n", type);
    if(min_cuda_compcap)
        fprintf(stderr, "  <min_cuda_compcap> %d </min_cuda_compcap>\n", min_cuda_compcap);
    if(max_cuda_compcap)
        fprintf(stderr, "  <max_cuda_compcap> %d </max_cuda_compcap>\n", max_cuda_compcap);
    if(min_cuda_version)
        fprintf(stderr, "  <min_cuda_version> %d </min_cuda_version>\n", min_cuda_version);
    if(max_cuda_version)
        fprintf(stderr, "  <max_cuda_version> %d </max_cuda_version>\n", max_cuda_version);
    if(min_opencl_version)
        fprintf(stderr, "  <min_opencl_version> %d </min_opencl_version>\n", min_opencl_version);
    if(min_driver_version)
        fprintf(stderr, "  <min_driver_version> %d </min_driver_version>\n", min_driver_version);
    if(max_driver_version)
        fprintf(stderr, "  <max_driver_version> %d </max_driver_version>\n", max_driver_version);
    if(min_gpu_ram_mb)
        fprintf(stderr, "  <min_gpu_ram_mb> %f </min_gpu_ram_mb>\n", min_gpu_ram_mb);
    if(gpu_ram_used_mb)
        fprintf(stderr, "  <gpu_ram_used_mb> %f </gpu_ram_used_mb>\n", gpu_ram_used_mb);
    if(*project_prefs_tag) {
        fprintf(stderr, "  <project_prefs_tag> %s </project_prefs_tag>\n", project_prefs_tag);
        fprintf(stderr, "  <project_prefs_min> %f </project_prefs_min>\n", project_prefs_min);
        fprintf(stderr, "  <project_prefs_max> %f </project_prefs_max>\n", project_prefs_max);
    }
    if(*gpu_utilization_tag)
        fprintf(stderr, "  <gpu_utilization_tag> %s </gpu_utilization_tag>\n", project_prefs_tag);
    if(min_macos_version)
        fprintf(stderr, "  <min_macos_version> %d </min_macos_version>\n", min_macos_version);
    if(max_macos_version)
        fprintf(stderr, "  <max_macos_version> %d </max_macos_version>\n", max_macos_version);
    if(*os_version_string)
        fprintf(stderr, "  <os_version> %s </os_version>\n", os_version_string);
    if(speedup)
        fprintf(stderr, "  <speedup> %f </speedup>\n", speedup);
    if(peak_flops_factor)
        fprintf(stderr, "  <peak_flops_factor> %f </peak_flops_factor>\n", peak_flops_factor);
    if(gpu_flops)
        fprintf(stderr, "  <gpu_flops> %f </gpu_flops>\n", gpu_flops);
    if(cpu_flops)
        fprintf(stderr, "  <cpu_flops> %f </cpu_flops>\n", cpu_flops);
    if(avg_ncpus)
        fprintf(stderr, "  <avg_ncpus> %f </avg_ncpus>\n", avg_ncpus);
    if(max_ncpus)
        fprintf(stderr, "  <max_ncpus> %f </max_ncpus>\n", max_ncpus);
    if(ngpus)
        fprintf(stderr, "  <ngpus> %f </ngpus>\n", ngpus);
    for (unsigned int i=0; i<cpu_features.size(); i++) {
        fprintf(stderr, "  <cpu_feature> %s </cpu_feature>\n", cpu_features[i].c_str());
    }
    fprintf(stderr, "</plan_class>\n");
}


int PLAN_CLASS_SPECS::parse_specs(FILE* f) {
    char buf[256];
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

            while (!xp.get_tag()) {
                if (xp.match_tag("/plan_class")) {
                    break;
                }
                if(xp.parse_str("name", pc.name, sizeof(pc.name))) {
                    strcpy(buf,pc.name);
                    downcase_string(buf);
                    if (strstr(buf,"cuda")) {
                        pc.type = 1;
                    } else if (strstr(buf,"nvidia")) {
                        pc.type = 1;
                    } else if (strstr(buf,"ati")) {
                        pc.type = 2;
                    }
                    continue;
                }
                if(xp.parse_str("type", buf, sizeof(buf))) {
                    downcase_string(buf);
                    if (!strcmp(buf,"cpu")) {
                        pc.type = 0;
                    } else if (!strcmp(buf,"cuda")) {
                        pc.type = 1;
                    } else if (strstr(buf,"nvidia")) {
                        pc.type = 1;
                    } else if (!strcmp(buf,"ati")) {
                        pc.type = 2;
                    } else {
                        pc.type = atoi(buf);
                    }
                    continue;
                }
                if(xp.parse_int("min_cuda_compcap", pc.min_cuda_compcap)) continue;
                if(xp.parse_int("max_cuda_compcap", pc.max_cuda_compcap)) continue;
                if(xp.parse_int("min_cuda_version", pc.min_cuda_version)) continue;
                if(xp.parse_int("max_cuda_version", pc.max_cuda_version)) continue;
                if(xp.parse_int("min_opencl_version", pc.min_opencl_version)) continue;
                if(xp.parse_int("min_opencl_device_version", pc.min_opencl_version)) continue;
                if(xp.parse_int("min_driver_version", pc.min_driver_version)) continue;
                if(xp.parse_int("max_driver_version", pc.max_driver_version)) continue;
                if(xp.parse_double("min_gpu_ram_mb", pc.min_gpu_ram_mb)) continue;
                if(xp.parse_double("gpu_ram_used_mb", pc.gpu_ram_used_mb)) continue;
                if(xp.parse_str("project_prefs_tag", pc.project_prefs_tag, sizeof(pc.project_prefs_tag))) continue;
                if(xp.parse_str("gpu_utilization_tag", pc.gpu_utilization_tag, sizeof(pc.gpu_utilization_tag))) continue;
                if(xp.parse_double("project_prefs_min", pc.project_prefs_min)) continue;
                if(xp.parse_double("project_prefs_max", pc.project_prefs_max)) continue;
                if(xp.parse_int("min_macos_version", pc.min_macos_version)) continue;
                if(xp.parse_int("max_macos_version", pc.max_macos_version)) continue;
                if(xp.parse_str("os_version", pc.os_version_string, sizeof(pc.os_version_string))) {
                    if ( regcomp(&(pc.os_version_regex), pc.os_version_string, REG_EXTENDED|REG_NOSUB) ) {
                        log_messages.printf(MSG_CRITICAL, "BAD REGEXP: %s\n", pc.os_version_string);
                        return ERR_XML_PARSE;
                    }
                    continue;
                }
                if(xp.parse_double("speedup", pc.speedup)) continue;
                if(xp.parse_double("peak_flops_factor", pc.peak_flops_factor)) continue;
                if(xp.parse_double("gpu_flops", pc.gpu_flops)) continue;
                if(xp.parse_double("cpu_flops", pc.cpu_flops)) continue;
                if(xp.parse_double("avg_ncpus", pc.avg_ncpus)) continue;
                if(xp.parse_double("max_ncpus", pc.max_ncpus)) continue;
                if(xp.parse_double("ngpus", pc.ngpus)) continue;
                if(xp.parse_str("cpu_feature", buf, sizeof(buf))) {
                    pc.cpu_features.push_back(" " + (string)buf + " ");
                    continue;
                }
            }
            classes.push_back(pc);
        }
    }
    return ERR_XML_PARSE;
} // parse_specs()


PLAN_CLASS_SPEC::PLAN_CLASS_SPEC() {
    type = 0;

    min_cuda_compcap = 0;
    max_cuda_compcap = 0;
    min_cuda_version = 0;
    max_cuda_version = 0;

    min_opencl_version = 0;

    min_driver_version = 0;
    max_driver_version = 0;
    min_gpu_ram_mb = 0;
    gpu_ram_used_mb = 0;

    *project_prefs_tag = '\0';
    *gpu_utilization_tag = '\0';
    project_prefs_min = 0;
    project_prefs_max = 0;

    min_macos_version = 0;
    max_macos_version = 0;
    *os_version_string = '\0';

    speedup = 1.0;
    peak_flops_factor = 1.0;
    cpu_flops = 0;
    gpu_flops = 0;
    avg_ncpus = 1.0;
    max_ncpus = 1.0;
    ngpus = 0; /* defaults to CPU plan classes */
}

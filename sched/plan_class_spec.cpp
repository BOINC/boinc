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

// Support for plan classes defined using an XML file.
// See https://github.com/BOINC/boinc/wiki/AppPlanSpec

#include <cmath>

#include "util.h"
#include "coproc.h"
#include "hostinfo.h"

#include "sched_config.h"
#include "sched_customize.h"
#include "plan_class_spec.h"

using std::string;

// return a numerical OS version for Darwin/OSX and Windows,
// letting us define numerical ranges for these OS versions
//
static double os_version_num(HOST h) {
    unsigned int a, b, c, d;
    if (strstr(h.os_name, "Darwin")) {
        if (sscanf(h.os_version, "%u.%u.%u", &a, &b, &c) == 3) {
            return 10000.0*a + 100.0*b + c;
        }
    } else if (strstr(h.os_name, "Windows")) {
        // example: "Enterprise Server Edition, Service Pack 1, (06.01.7601.00)"
        //
        char *p = strrchr(h.os_version,'(');
        if (p && (sscanf(p, "(%u.%u.%u.%u)", &a, &b, &c, &d) == 4)) {
            return 100000000.0*a + 1000000.0*b + 100.0*c +d;
        }
    } else if (strstr(h.os_name, "Android")) {
        // examples:
        // 3.0.31-g6fb96c9
        // 2.6.36.3
        // 3.4.0
        //
        if (sscanf(h.os_version, "%u.%u.%u", &a, &b, &c) == 3) {
            return 10000.*a + 100.*b + c;
        }
    } else if (strstr(h.os_name, "Linux")) {
        // os_name seems to always contain "Linux".
        // os_version is pretty diverse:
        //
        // Linux Mint 19 Tara [4.18.12-041812-generic|libc 2.27 (Ubuntu GLIBC 2.27-3ubuntu1)]
        // CentOS Linux 7 (Core) [3.10.0-862.14.4.el7.x86_64|libc 2.17 (GNU libc)]
        // Ubuntu 18.04.1 LTS [4.15.0-36-generic|libc 2.27 (Ubuntu GLIBC 2.27-3ubuntu1)]
        // 3.13.0-103-generic
        // 4.9.0-8-amd64
        // Manjaro Linux [4.19.42-1-MANJARO|libc 2.29 (GNU libc)]

        char* p = strchr(h.os_version, '[');
        if (p) {
            p++;
        } else {
            p = h.os_version;
        }
        if (sscanf(p, "%u.%u.%u", &a, &b, &c) == 3) {
            return 10000.*a + 100.*b + c;
        }
    }
    // could not determine numerical OS version
    //
    return 0;
}

// if os_version has [...|libc 2.27 ...], return 227.  else 0
//
static int libc_version(HOST &h) {
    char *p = strstr(h.os_version, "|libc ");
    if (!p) return 0;
    p += strlen("|libc ");
    int maj, min;
    int n = sscanf(p, "%d.%d", &maj, &min);
    if (n != 2) return 0;
    return maj*100+min;
}

// parse version# from "(Android 4.3.1)" or "(Android 4.3)" or "(Android 4)"
//
static int android_version_num(HOST h) {
    int maj, min, rel;
    char* p = strstr(h.os_version, "(Android ");
    if (!p) return 0;
    p += strlen("(Android ");
    int n = sscanf(p, "%d.%d.%d", &maj, &min, &rel);
    if (n == 3) {
        return maj*10000 + min*100 + rel;
    }
    n = sscanf(p, "%d.%d", &maj, &min);
    if (n == 2) {
        return maj*10000 + min*100;
    }
    n = sscanf(p, "%d", &maj);
    if (n == 1) {
        return maj*10000;
    }
    return 0;
}

static bool wu_is_infeasible_for_plan_class(
    const PLAN_CLASS_SPEC* pc, const WORKUNIT* wu
) {
    if (pc->min_wu_id && wu->id < pc->min_wu_id) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] WU#%ld too old for plan class '%s' (%ld)\n",
                wu->id, pc->name, pc->min_wu_id
            );
        }
        return true;
    }
    if (pc->max_wu_id && wu->id > pc->max_wu_id) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] WU#%ld too new for plan class '%s' (%ld)\n",
                wu->id, pc->name, pc->max_wu_id
            );
        }
        return true;
    }
    if (pc->min_batch && wu->batch < pc->min_batch) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] batch#%d too old for plan class '%s' (%ld)\n",
                wu->batch, pc->name, pc->min_batch
            );
        }
        return true;
    }
    if (pc->max_batch && wu->batch > pc->max_batch) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] batch#%d too new for plan class '%s' (%ld)\n",
                wu->batch, pc->name, pc->max_batch
            );
        }
        return true;
    }
    return false;
}

// parse plan_class_spec.xml
//
int PLAN_CLASS_SPECS::parse_file(const char* path) {
    FILE* f = boinc::fopen(path, "r");
    if (!f) return ERR_FOPEN;
    int retval = parse_specs(f);
    boinc::fclose(f);
    return retval;
}

bool PLAN_CLASS_SPEC::opencl_check(OPENCL_DEVICE_PROP& opencl_prop) {
    if (min_opencl_version && opencl_prop.opencl_device_version_int
        && min_opencl_version > opencl_prop.opencl_device_version_int
    ) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] OpenCL device version required min: %d, supplied: %d\n",
                min_opencl_version, opencl_prop.opencl_device_version_int
            );
        }
        return false;
    }

    if (max_opencl_version && opencl_prop.opencl_device_version_int
        && max_opencl_version < opencl_prop.opencl_device_version_int
    ) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] OpenCL device version required max: %d, supplied: %d\n",
                max_opencl_version, opencl_prop.opencl_device_version_int
            );
        }
        return false;
    }

    if (min_opencl_driver_revision && opencl_prop.opencl_device_version_int
        && min_opencl_driver_revision > opencl_prop.opencl_driver_revision
    ) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] OpenCL driver revision required min: %d, supplied: %d\n",
                min_opencl_driver_revision, opencl_prop.opencl_driver_revision
            );
        }
        return false;
    }

    if (max_opencl_driver_revision && opencl_prop.opencl_device_version_int
        && max_opencl_driver_revision < opencl_prop.opencl_driver_revision
    ) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] OpenCL driver revision required max: %d, supplied: %d\n",
                max_opencl_driver_revision, opencl_prop.opencl_driver_revision
            );
        }
        return false;
    }

    if (double_precision_fp && (opencl_prop.double_fp_config == 0)) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] device (or driver) doesn't support double precision fp math\n"
            );
        }
        return false;
    }

    return true;
}

// See whether the given host/user can be sent this plan class.
// If so return the resource usage and estimated FLOPS in hu.
//
bool PLAN_CLASS_SPEC::check(
    SCHEDULER_REQUEST& sreq, HOST_USAGE& hu, const WORKUNIT* wu
) {
    COPROC* cpp = NULL;
    bool can_use_multicore = true;
    string msg;

    if (infeasible_random && drand()<infeasible_random) {
        return false;
    }
    if (user_id && sreq.user_id != user_id) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] not specified user ID (%d %d)\n",
                user_id, sreq.user_id
            );
        }
        return false;
    }

    // default is sequential app
    //
    hu.sequential_app(sreq.host.p_fpops);

    // ID restrictions
    //
    if (min_wu_id || max_wu_id || min_batch || max_batch) {
        if (wu_is_infeasible_for_plan_class(this, wu)) {
            return false;
        }
    }

    // CPU features
    //
    // older clients report CPU features in p_model,
    // within square brackets
    //
    // the requested features are surrounded by spaces,
    // so we can look for them with strstr()
    //
    if (!cpu_features.empty()) {
        char buf[P_FEATURES_SIZE], buf2[512];
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
                        "[version] plan_class_spec: CPU lacks feature '%s' (got '%s')\n",
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
                "[version] plan_class_spec: not enough CPUs: %d < %f\n",
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
                "[version] plan_class_spec: OS version '%s' didn't match regexp\n",
                sreq.host.os_version
            );
        }
        return false;
    }
    if (min_os_version || max_os_version) {
        double host_os_version_num = os_version_num(sreq.host);
        if (!host_os_version_num) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: Can't determine numerical OS version '%s'\n",
                    sreq.host.os_version
                );
            }
            return false;
        }
        if (min_os_version && (host_os_version_num < min_os_version)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: OS version '%s' too low (%.0f / %.0f)\n",
                    sreq.host.os_version, host_os_version_num, min_os_version
                );
            }
            return false;
        }
        if (max_os_version && (host_os_version_num > max_os_version)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: OS version '%s' too high (%.0f / %.0f)\n",
                    sreq.host.os_version, host_os_version_num, max_os_version
                );
            }
            return false;
        }
    }
    if (min_android_version || max_android_version) {
        if (strcasecmp(sreq.host.os_name, "android")) return false;
        int host_android_version = android_version_num(sreq.host);
        if (!host_android_version) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: Can't determine numerical Android version '%s'\n",
                    sreq.host.os_version
                );
            }
            if (min_android_version>0) return false;
        }
        if (min_android_version && (host_android_version < min_android_version)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: Android version '%s' too low (%d / %d)\n",
                    sreq.host.os_version, host_android_version, min_android_version
                );
            }
            return false;
        }
        if (max_android_version && (host_android_version > max_android_version)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: Android version '%s' too high (%d / %d)\n",
                    sreq.host.os_version, host_android_version, max_android_version
                );
            }
            return false;
        }
    }

    // libc version (linux)
    //
    if (min_libc_version) {
        int v = libc_version(sreq.host);
        if (v < min_libc_version) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: libc version too low (%d < %d)\n",
                    v, min_libc_version
                );
            }
            return false;
        }
    }

    // CPU vendor and model
    //
    if (have_cpu_vendor_regex && regexec(&(cpu_vendor_regex), sreq.host.p_vendor, 0, NULL, 0)) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] plan_class_spec: CPU vendor '%s' didn't match regexp\n",
                sreq.host.p_vendor
            );
        }
        return false;
    }

    if (have_cpu_model_regex && regexec(&(cpu_model_regex), sreq.host.p_model, 0, NULL, 0)) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] plan_class_spec: CPU model '%s' didn't match regexp\n",
                sreq.host.p_model
            );
        }
        return false;
    }

    // BOINC versions
    //
    if (min_core_client_version && sreq.core_client_version < min_core_client_version) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] plan_class_spec: Need newer BOINC core client: %d < %d\n",
                sreq.core_client_version, min_core_client_version
            );
        }
        add_no_work_message("A newer BOINC may be required for some tasks.");
        return false;
    }
    if (max_core_client_version && sreq.core_client_version > max_core_client_version) {
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] plan_class_spec: Need older BOINC core client: %d > %d\n",
                sreq.core_client_version, max_core_client_version
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
                    "[version] plan_class_spec: can't parse vbox version\n"
                );
            }
            return false;
        }
        int v = maj*10000 + min*100 + rel;
        if (min_vbox_version && v < min_vbox_version) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: vbox version too low: %d < %d\n",
                    v, min_vbox_version
                );
            }
            return false;
        }
        if (max_vbox_version && v > max_vbox_version) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: vbox version too high: %d > %d\n",
                    v, max_vbox_version
                );
            }
            return false;
        }
        if (in_vector(v, exclude_vbox_version)) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: vbox version %d excluded\n", v
                );
            }
            return false;
        }

        if (vm_accel_required) {
            if ((!strstr(sreq.host.p_features, "vmx") && !strstr(sreq.host.p_features, "svm"))
                || sreq.host.p_vm_extensions_disabled
            ) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] plan_class_spec: missing VM HW acceleration\n"
                    );
                }
                return false;
            }
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

    if (wsl) {
        if (sreq.dont_use_wsl) {
            add_no_work_message("Client config does not allow using WSL");
            return false;
        }
        if (sreq.host.wsl_distros.distros.empty()) {
            add_no_work_message("No WSL distros found");
            return false;
        }
        bool found = false;
        for (WSL_DISTRO &wd: sreq.host.wsl_distros.distros) {
            if (wd.disallowed) continue;
            if (min_libc_version) {
                if (wd.libc_version_int() < min_libc_version) continue;
            }
            found = true;
        }
        if (!found) {
            add_no_work_message("No usable WSL distros found");
            return false;
        }
    }

    // Docker apps: check that:
    // - Docker is allowed
    // - Win:
    //      - WSL is allowed
    //      - There's an allowed WSL distro containing Docker
    // - Unix:
    //      - Docker is present
    if (docker) {
        if (sreq.dont_use_docker) {
            add_no_work_message("Client config does not allow using Docker");
            return false;
        }
        if (strstr(sreq.host.os_name, "Windows")) {
            if (sreq.dont_use_wsl) {
                add_no_work_message("Client config does not allow using WSL");
                return false;
            }
            bool found = false;
            for (WSL_DISTRO &wd: sreq.host.wsl_distros.distros) {
                if (wd.disallowed) continue;
                if (wd.docker_version.empty()) continue;
                found = true;
                break;
            }
            if (!found) {
                add_no_work_message("No usable WSL distros found");
                return false;
            }
        } else {
            if (strlen(sreq.host.docker_version) == 0) {
                add_no_work_message("Docker not present");
                return false;
            }
            if (strstr(sreq.host.os_name, "Darwin")) {
                if (sreq.core_client_version < 80206) {
                    add_no_work_message(
                        "Docker jobs need 8.2.6+ client"
                    );
                    return false;
                }
            }
        }
    }

    // project-specific preference
    //
    if (have_project_prefs_regex && strlen(project_prefs_tag)) {
        char tag[256], value[256];
        char buf[65536];
        extract_venue(g_reply->user.project_prefs, g_reply->host.venue, buf, sizeof(buf));
        sprintf(tag,"<%s>",project_prefs_tag);
        bool p = parse_str(buf, tag, value, sizeof(value));
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] plan_class_spec: parsed project prefs setting '%s' : %s\n",
                project_prefs_tag, p?"true":"false"
            );
        }
        if (p ? regexec(&(project_prefs_regex), value, 0, NULL, 0) : !project_prefs_default_true) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: project prefs setting '%s' value='%s' prevents using plan class.\n",
                    project_prefs_tag, p ? value : "(tag missing)"
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
        extract_venue(g_reply->user.project_prefs, g_reply->host.venue, buf, sizeof(buf));
        sprintf(tag,"<%s>",gpu_utilization_tag);
        bool p = parse_double(buf, tag, v);
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] plan_class_spec: parsed project prefs setting '%s' : %s : %f\n",
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
                    "[version] plan_class_spec: No AMD GPUs found\n"
                );
            }
            return false;
        }
        if (min_gpu_ram_mb) {
            gpu_requirements[PROC_TYPE_AMD_GPU].update(0, min_gpu_ram_mb * MEGA);
        }
        if (min_driver_version) {
            gpu_requirements[PROC_TYPE_AMD_GPU].update(abs(min_driver_version), 0);
        }

        if (need_ati_libs) {
            if (!cp.atirt_detected) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] plan_class_spec: ATI libraries not found\n"
                    );
                }
                return false;
            }
        } else {
            if (need_amd_libs && !cp.amdrt_detected) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] plan_class_spec: AMD libraries not found\n"
                    );
                }
                return false;
            }
        }

        if (without_opencl) {
            if (cp.have_opencl) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] plan_class_spec: OpenCL detected. Plan restricted to CAL only GPUs\n"
                    );
                }
                return false;
            }
        }


        if (min_cal_target && cp.attribs.target < min_cal_target) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: CAL target less than minimum (%d < %d)\n",
                    cp.attribs.target, min_cal_target
                );
            }
            return false;
        }
        if (max_cal_target && cp.attribs.target > max_cal_target) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: CAL target greater than maximum (%d > %d)\n",
                    cp.attribs.target, max_cal_target
                );
            }
            return false;
        }

        if (cp.bad_gpu_peak_flops("AMD", msg)) {
            log_messages.printf(MSG_NORMAL, "%s\n", msg.c_str());
        }
        gpu_ram = cp.opencl_prop.global_mem_size;

        driver_version = 0;
        if (cp.have_cal) {
            int major, minor, release, scanned;
            scanned = sscanf(cp.version, "%d.%d.%d", &major, &minor, &release);
            if (scanned != 3) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] plan_class_spec: driver version '%s' couldn't be parsed\n",
                        cp.version
                    );
                }
                return false;
            } else {
                driver_version = ati_version_int(major,minor,release);
            }
        } else {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: no CAL, driver version couldn't be determined\n"
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
                    "[version] plan_class_spec: No NVIDIA GPUs found\n"
                );
            }
            return false;
        }

        // in analogy to ATI/AMD
        driver_version=cp.display_driver_version;

        if (min_gpu_ram_mb) {
            gpu_requirements[PROC_TYPE_NVIDIA_GPU].update(0, min_gpu_ram_mb * MEGA);
        }
        if (min_driver_version) {
            gpu_requirements[PROC_TYPE_NVIDIA_GPU].update(abs(min_driver_version), 0);
        }
        // compute capability
        int v = (cp.prop.major)*100 + cp.prop.minor;
        if (min_nvidia_compcap && min_nvidia_compcap > v) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: NVIDIA compute capability required min: %d, supplied: %d\n",
                    min_nvidia_compcap, v
                );
            }
            return false;
        }
        if (max_nvidia_compcap && max_nvidia_compcap < v) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: CUDA compute capability required max: %d, supplied: %d\n",
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
                        "[version] plan_class_spec: CUDA version required min: %d, supplied: %d\n",
                        min_cuda_version, cp.cuda_version
                    );
                }
                return false;
            }
            if (max_cuda_version && max_cuda_version < cp.cuda_version) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] plan_class_spec: CUDA version required max: %d, supplied: %d\n",
                        max_cuda_version, cp.cuda_version
                    );
                }
                return false;
            }
        }
        gpu_ram = cp.prop.totalGlobalMem;
        if (cp.bad_gpu_peak_flops("NVIDIA", msg)) {
            log_messages.printf(MSG_NORMAL, "%s\n", msg.c_str());
        }

    // Intel GPU
    //
    } else if (strstr(gpu_type, "intel") == gpu_type) {
        COPROC& cp = sreq.coprocs.intel_gpu;
        cpp = &cp;

        if (!cp.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: No Intel GPUs found\n"
                );
            }
            return false;
        }
        if (min_gpu_ram_mb) {
            gpu_requirements[PROC_TYPE_INTEL_GPU].update(0, min_gpu_ram_mb * MEGA);
        }
        if (cp.bad_gpu_peak_flops("Intel GPU", msg)) {
            log_messages.printf(MSG_NORMAL, "%s\n", msg.c_str());
        }

    // Apple GPU

    } else if (!strcmp(gpu_type, "apple")) {
        COPROC& cp = sreq.coprocs.apple_gpu;
        cpp = &cp;

        if (!cp.count) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: No Apple GPUs found\n"
                );
            }
            return false;
        }
        if (min_gpu_ram_mb) {
            gpu_requirements[PROC_TYPE_APPLE_GPU].update(0, min_gpu_ram_mb * MEGA);
        }
        if (cp.bad_gpu_peak_flops("Apple GPU", msg)) {
            log_messages.printf(MSG_NORMAL, "%s\n", msg.c_str());
        }

        if (min_metal_support) {
            if (sreq.coprocs.apple_gpu.metal_support < min_metal_support) {
                return false;
            }
        }

    // custom GPU type
    //
    } else if (strlen(gpu_type)) {
        cpp = sreq.coprocs.lookup_type(gpu_type);
        if (!cpp) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: No %s found\n", gpu_type
                );
            }
            return false;
        }
        if (config.debug_version_select) {
            log_messages.printf(MSG_NORMAL,
                "[version] plan_class_spec: Custom coproc %s found\n", gpu_type
            );
        }
        if (cpp->bad_gpu_peak_flops("Custom GPU", msg)) {
            log_messages.printf(MSG_NORMAL, "%s\n", msg.c_str());
        }
    }

    if (opencl) {
        if (cpp) {
            if (!cpp->have_opencl) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] GPU doesn't support OpenCL\n"
                    );
                }
                return false;
            }
            if (!opencl_check(cpp->opencl_prop)) {
                return false;
            }
            gpu_ram = cpp->opencl_prop.global_mem_size;
        } else {
            // OpenCL CPU app version.
            // The host may have several OpenCL CPU libraries.
            // See if any of them works.
            // TODO: there should be a way of saying which library
            // the app version requires,
            // or a way of conveying to the app which one to use.
            //
            bool found = false;
            for (int i=0; i<sreq.host.num_opencl_cpu_platforms; i++) {
                if (opencl_check(sreq.host.opencl_cpu_prop[i].opencl_prop)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] CPU doesn't support OpenCL\n"
                    );
                }
                return false;
            }
        }
    }

    // general GPU
    //
    if (strlen(gpu_type)) {

        // GPU RAM
        //
        if (min_gpu_ram_mb && min_gpu_ram_mb * MEGA > gpu_ram) {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: GPU RAM required min: %f, supplied: %f\n",
                    min_gpu_ram_mb * MEGA, gpu_ram
                );
            }
            return false;
        }

        // display driver version
        //
        if (min_driver_version && driver_version) {
            if (min_driver_version > driver_version) {
                if (config.debug_version_select) {
                    log_messages.printf(MSG_NORMAL,
                        "[version] plan_class_spec: driver version required min: %d, supplied: %d\n",
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
                        "[version] plan_class_spec: driver version required max: %d, supplied: %d\n",
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

        // if we don't know GPU peak flops, treat it like a CPU app
        //
        if (cpp->peak_flops == 0) {
            strcpy(hu.custom_coproc_type, gpu_type);
            hu.avg_ncpus = cpu_frac;
            hu.gpu_usage = gpu_usage;
        } else {
            if (min_gpu_peak_flops && cpp->peak_flops < min_gpu_peak_flops) {
                return false;
            }
            if (max_gpu_peak_flops && cpp->peak_flops > max_gpu_peak_flops) {
                return false;
            }
            coproc_perf(
                capped_host_fpops(),
                gpu_peak_flops_scale * gpu_usage * cpp->peak_flops,
                cpu_frac,
                hu.projected_flops,
                hu.avg_ncpus
            );
            hu.projected_flops *= projected_flops_scale;
            if (avg_ncpus) {
                hu.avg_ncpus = avg_ncpus;
            }
            // I believe the first term here is just hu.projected_flops,
            // but I'm leaving it spelled out to match GPU scheduling
            // code in sched_customize.cpp
            //
            hu.peak_flops = gpu_peak_flops_scale*gpu_usage*cpp->peak_flops
                + hu.avg_ncpus*capped_host_fpops()
            ;
        }

        if (!strcmp(gpu_type, "amd") || !strcmp(gpu_type, "ati")) {
            hu.proc_type = PROC_TYPE_AMD_GPU;
            hu.gpu_usage = gpu_usage;
        } else if (!strcmp(gpu_type, "nvidia")) {
            hu.proc_type = PROC_TYPE_NVIDIA_GPU;
            hu.gpu_usage = gpu_usage;
        } else if (strstr(gpu_type, "intel")==gpu_type) {
            hu.proc_type = PROC_TYPE_INTEL_GPU;
            hu.gpu_usage = gpu_usage;
        } else if (strstr(gpu_type, "apple_gpu")==gpu_type) {
            hu.proc_type = PROC_TYPE_APPLE_GPU;
            hu.gpu_usage = gpu_usage;
        } else {
            if (config.debug_version_select) {
                log_messages.printf(MSG_NORMAL,
                    "[version] plan_class_spec: unknown GPU supplied: %s\n",
                    gpu_type
                );
            }
        }
    } else {
        // CPU only
        //
        if (avg_ncpus) {
            hu.avg_ncpus = avg_ncpus;
        } else {
            if (can_use_multicore) {
                if (max_threads > g_wreq->effective_ncpus) {
                    hu.avg_ncpus = g_wreq->effective_ncpus;
                } else {
                    hu.avg_ncpus = max_threads;
                }

                // if per-CPU mem usage given
                //
                if (mem_usage_per_cpu) {
                    if (!min_ncpus) min_ncpus = 1;
                    double mem_usage_seq = mem_usage_base + min_ncpus*mem_usage_per_cpu;

                    // see if client has enough memory to run at all
                    //
                    if (mem_usage_seq > g_wreq->usable_ram) {
                        if (config.debug_version_select) {
                            log_messages.printf(MSG_NORMAL,
                                "[version] plan_class_spec: insufficient multicore RAM; %f < %f",
                                g_wreq->usable_ram, mem_usage_seq
                            );
                        }
                        return false;
                    }

                    // see how many CPUs we could use given memory usage
                    //
                    int n = (g_wreq->usable_ram - mem_usage_base)/mem_usage_per_cpu;
                    // don't use more than this many
                    //
                    if (n < hu.avg_ncpus) {
                        hu.avg_ncpus = n;
                    }

                    // compute memory usage; overrides wu.rsc_memory_bound
                    //
                    hu.mem_usage = mem_usage_base + hu.avg_ncpus*mem_usage_per_cpu;
                    char buf[256];
                    sprintf(buf, " --memory_size_mb %.0f", hu.mem_usage/MEGA);
                    strcat(hu.cmdline, buf);
                }
            } else {
                hu.avg_ncpus = 1;
            }
        }
        if (nthreads_cmdline) {
            char buf[256];
            sprintf(buf, " --nthreads %d", (int)hu.avg_ncpus);
            strcat(hu.cmdline, buf);
        }

        hu.peak_flops = capped_host_fpops() * hu.avg_ncpus;
        hu.projected_flops = capped_host_fpops() * hu.avg_ncpus * projected_flops_scale;

        // end CPU case
    }

#if 0
    if (config.debug_version_select) {
        log_messages.printf(MSG_NORMAL,
            "[version] plan_class_spec: host_flops: %e, \tscale: %.2f, \tprojected_flops: %e, \tpeak_flops: %e\n",
            sreq.host.p_fpops, projected_flops_scale, hu.projected_flops,
            hu.peak_flops
        );
    }
#endif

    return true;
}

bool PLAN_CLASS_SPECS::check(
    SCHEDULER_REQUEST& sreq, const char* plan_class, HOST_USAGE& hu,
    const WORKUNIT* wu
) {
    for (unsigned int i=0; i<classes.size(); i++) {
        if (!strcmp(classes[i].name, plan_class)) {
            return classes[i].check(sreq, hu, wu);
        }
    }
    log_messages.printf(MSG_CRITICAL, "Unknown plan class: %s\n", plan_class);
    return false;
}

bool PLAN_CLASS_SPECS::wu_is_infeasible(
    const char* plan_class_name, const WORKUNIT* wu
) {
    if(wu_restricted_plan_class) {
        for (unsigned int i=0; i<classes.size(); i++) {
            if(!strcmp(classes[i].name, plan_class_name)) {
                return wu_is_infeasible_for_plan_class(&classes[i], wu);
            }
        }
    }
    return false;
}

int PLAN_CLASS_SPEC::parse(XML_PARSER& xp) {
    char buf[256];
    int i;
    while (!xp.get_tag()) {
        if (xp.match_tag("/plan_class")) {
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_int("min_core_client_version", min_core_client_version)) continue;
        if (xp.parse_int("max_core_client_version", max_core_client_version)) continue;
        if (xp.parse_str("gpu_type", gpu_type, sizeof(gpu_type))) continue;
        if (xp.parse_bool("cuda", cuda)) continue;
        if (xp.parse_bool("cal", cal)) continue;
        if (xp.parse_bool("opencl", opencl)) continue;
        if (xp.parse_bool("virtualbox", virtualbox)) continue;
        if (xp.parse_bool("wsl", wsl)) continue;
        if (xp.parse_bool("docker", docker)) continue;
        if (xp.parse_bool("is64bit", is64bit)) continue;
        if (xp.parse_str("cpu_feature", buf, sizeof(buf))) {
            cpu_features.push_back(" " + (string)buf + " ");
            continue;
        }
        if (xp.parse_double("min_ncpus", min_ncpus)) continue;
        if (xp.parse_int("max_threads", max_threads)) continue;
        if (xp.parse_double("mem_usage_base_mb", mem_usage_base)) {
            mem_usage_base *= MEGA;
            continue;
        }
        if (xp.parse_double("mem_usage_per_cpu_mb", mem_usage_per_cpu)) {
            mem_usage_per_cpu *= MEGA;
            continue;
        }
        if (xp.parse_bool("nthreads_cmdline", nthreads_cmdline)) continue;
        if (xp.parse_double("projected_flops_scale", projected_flops_scale)) continue;
        if (xp.parse_str("os_regex", buf, sizeof(buf))) {
            if (regcomp(&(os_regex), buf, REG_EXTENDED|REG_NOSUB) ) {
                log_messages.printf(MSG_CRITICAL, "BAD OS REGEXP: %s\n", buf);
                return ERR_XML_PARSE;
            }
            have_os_regex = true;
            continue;
        }
        if (xp.parse_str("cpu_vendor_regex", buf, sizeof(buf))) {
            if (regcomp(&(cpu_vendor_regex), buf, REG_EXTENDED|REG_NOSUB) ) {
                log_messages.printf(MSG_CRITICAL, "BAD CPU VENDOR REGEXP: %s\n", buf);
                return ERR_XML_PARSE;
            }
            have_cpu_vendor_regex = true;
            continue;
        }
        if (xp.parse_str("cpu_model_regex", buf, sizeof(buf))) {
            if (regcomp(&(cpu_model_regex), buf, REG_EXTENDED|REG_NOSUB) ) {
                log_messages.printf(MSG_CRITICAL, "BAD CPU MODEL REGEXP: %s\n", buf);
                return ERR_XML_PARSE;
            }
            have_cpu_model_regex = true;
            continue;
        }
        if (xp.parse_int("user_id", user_id)) continue;
        if (xp.parse_double("infeasible_random", infeasible_random)) continue;
        if (xp.parse_double("min_os_version", min_os_version)) continue;
        if (xp.parse_double("max_os_version", max_os_version)) continue;
        if (xp.parse_int("min_android_version", min_android_version)) continue;
        if (xp.parse_int("max_android_version", max_android_version)) continue;
        if (xp.parse_int("min_libc_version", min_libc_version)) continue;
        if (xp.parse_str("project_prefs_tag", project_prefs_tag, sizeof(project_prefs_tag))) continue;
        if (xp.parse_str("project_prefs_regex", buf, sizeof(buf))) {
            if (regcomp(&(project_prefs_regex), buf, REG_EXTENDED|REG_NOSUB) ) {
                log_messages.printf(MSG_CRITICAL, "BAD PROJECT PREFS REGEXP: %s\n", buf);
                return ERR_XML_PARSE;
            }
            have_project_prefs_regex = true;
            continue;
        }
        if (xp.parse_bool("project_prefs_default_true", project_prefs_default_true)) continue;
        if (xp.parse_double("avg_ncpus", avg_ncpus)) continue;

        if (xp.parse_double("cpu_frac", cpu_frac)) continue;
        if (xp.parse_double("min_gpu_ram_mb", min_gpu_ram_mb)) continue;
        if (xp.parse_double("gpu_ram_used_mb", gpu_ram_used_mb)) continue;
        if (xp.parse_double("gpu_peak_flops_scale", gpu_peak_flops_scale)) continue;
        if (xp.parse_double("ngpus", ngpus)) continue;
        if (xp.parse_double("min_gpu_peak_flops", min_gpu_peak_flops)) continue;
        if (xp.parse_double("max_gpu_peak_flops", max_gpu_peak_flops)) continue;
        if (xp.parse_int("min_driver_version", min_driver_version)) continue;
        if (xp.parse_int("max_driver_version", max_driver_version)) continue;
        if (xp.parse_str("gpu_utilization_tag", gpu_utilization_tag, sizeof(gpu_utilization_tag))) continue;
        if (xp.parse_long("min_wu_id", min_wu_id)) {wu_restricted_plan_class = true; continue;}
        if (xp.parse_long("max_wu_id", max_wu_id)) {wu_restricted_plan_class = true; continue;}
        if (xp.parse_long("min_batch", min_batch)) {wu_restricted_plan_class = true; continue;}
        if (xp.parse_long("max_batch", max_batch)) {wu_restricted_plan_class = true; continue;}

        if (xp.parse_bool("need_ati_libs", need_ati_libs)) continue;
        if (xp.parse_bool("need_amd_libs", need_amd_libs)) continue;
        if (xp.parse_bool("without_opencl", without_opencl)) continue;
        if (xp.parse_int("min_cal_target", min_cal_target)) continue;
        if (xp.parse_int("max_cal_target", max_cal_target)) continue;

        if (xp.parse_int("min_nvidia_compcap", min_nvidia_compcap)) continue;
        if (xp.parse_int("max_nvidia_compcap", max_nvidia_compcap)) continue;

        if (xp.parse_int("min_cuda_version", min_cuda_version)) continue;
        if (xp.parse_int("max_cuda_version", max_cuda_version)) continue;

        if (xp.parse_int("min_opencl_version", min_opencl_version)) continue;
        if (xp.parse_int("max_opencl_version", max_opencl_version)) continue;

        if (xp.parse_int("min_opencl_driver_revision", min_opencl_driver_revision)) continue;
        if (xp.parse_int("max_opencl_driver_revision", max_opencl_driver_revision)) continue;
        if (xp.parse_bool("double_precision_fp", double_precision_fp)) continue;

        if (xp.parse_int("min_metal_support", min_metal_support)) continue;

        if (xp.parse_int("min_vbox_version", min_vbox_version)) continue;
        if (xp.parse_int("max_vbox_version", max_vbox_version)) continue;
        if (xp.parse_int("exclude_vbox_version", i)) {
            exclude_vbox_version.push_back(i);
            continue;
        }
        if (xp.parse_bool("vm_accel_required", vm_accel_required)) continue;
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
            boinc::fprintf(stderr, "PLAN_CLASS_SPECS::parse(): unexpected text %s\n", xp.parsed_tag);
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
    wsl = false;
    docker = false;
    is64bit = false;
    min_ncpus = 0;
    max_threads = 1;
    mem_usage_base = 0;
    mem_usage_per_cpu = 0;
    nthreads_cmdline = false;
    projected_flops_scale = 1;
    have_os_regex = false;
    have_cpu_vendor_regex = false;
    have_cpu_model_regex = false;
    min_os_version = 0;
    max_os_version = 0;
    min_android_version = 0;
    max_android_version = 0;
    min_libc_version = 0;
    strcpy(project_prefs_tag, "");
    have_project_prefs_regex = false;
    project_prefs_default_true = false;
    avg_ncpus = 0;
    min_core_client_version = 0;
    max_core_client_version = 0;
    user_id = 0;
    infeasible_random = 0;
    min_wu_id=0;
    max_wu_id=0;
    min_batch=0;
    max_batch=0;

    cpu_frac = .1;
    min_gpu_ram_mb = 0;
    gpu_ram_used_mb = 0;
    gpu_peak_flops_scale = 1;
    ngpus = 1;
    min_gpu_peak_flops = 0;
    max_gpu_peak_flops = 0;
    min_driver_version = 0;
    max_driver_version = 0;
    strcpy(gpu_utilization_tag, "");

    need_ati_libs = false;
    need_amd_libs = false;
    min_cal_target = 0;
    max_cal_target = 0;
    without_opencl = false;

    min_nvidia_compcap = 0;
    max_nvidia_compcap = 0;
    min_cuda_version = 0;
    max_cuda_version = 0;

    min_opencl_version = 0;
    max_opencl_version = 0;
    min_opencl_driver_revision = 0;
    max_opencl_driver_revision = 0;
    double_precision_fp = false;

    min_vbox_version = 0;
    max_vbox_version = 0;
    vm_accel_required = false;
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
        printf("parse_file: %d\n", retval);
        exit(1);
    }

    config.debug_version_select = true;

    HOST_USAGE hu;

    strcpy(sreq.host.p_features, "pni");
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
        WORKUNIT wu;
        wu.id = 100;
        wu.batch = 100;
        bool b = pcs.check(sreq, pcs.classes[i].name, hu, &wu);
        if (b) {
            printf("%s: check succeeded\n", pcs.classes[i].name);
            printf("\tgpu_usage: %f\n\tgpu_ram: %fMB\n\tavg_ncpus: %f\n\tprojected_flops: %fG\n\tpeak_flops: %fG\n",
                hu.gpu_usage,
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

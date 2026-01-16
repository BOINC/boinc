// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// we record some properties of hosts (RAM size, CPU info, etc.)
// directly in fields of the host table.
//
// For other stuff (GPUs, Docker info) we use text.
// Old way:
// use strings like [BOINC|8.2.4][CUDA|NVIDIA GeForce RTX 3060 Ti|1|8191MB|57260|300][vbox|7.1.6|1|1]
// and store them in host.serialnum.
// This is bad for several reasons (e.g. we can overflow the 254 chars)
//
// New way:
// store them in JSON in a new field host.misc

// This file has code for both ways.
// At some point we can delete the old way.

#include <string.h>
#include <string>

using std::string;

#include "sched_types.h"

#include "sched_host.h"

static void get_docker_info(
    string &version, int &type, string &wsl_distro, int &bbr_version
) {
    wsl_distro = "";
    bbr_version = 0;
    if (strstr(g_request->host.os_name, "Windows")) {
        for (WSL_DISTRO &wd: g_request->host.wsl_distros.distros) {
            if (wd.disallowed) continue;
            if (wd.docker_version.empty()) continue;
            version = wd.docker_version;
            type = wd.docker_type;
            wsl_distro = wd.distro_name;
            bbr_version = wd.boinc_buda_runner_version;
        }
    } else {
        version = g_request->host.docker_version;
        type = g_request->host.docker_type;
    }
}

// return misc host info as JSON
//
void host_info_json(string &out) {
    char buf[1024];
    sprintf(buf,
        "{\n"\
"    \"client_version\": \"%d.%d.%d\"",
        g_request->core_client_major_version,
        g_request->core_client_minor_version,
        g_request->core_client_release
    );
    out = buf;

    string gpus;
    g_request->coprocs.summary_string_json(gpus);
    if (!gpus.empty()) {
        out += ",\n"\
"    \"gpus\": [\n";
        out += gpus;
        out += "\n    ]";
    }

    string docker_version;
    int docker_type;
    string wsl_distro;
    int bbr_version;
    get_docker_info(docker_version, docker_type, wsl_distro, bbr_version);
    if (!docker_version.empty()) {
        sprintf(buf,
            ",\n"\
"    \"docker\": {\n"\
"        \"version\": \"%s\",\n"\
"        \"type\": %d",
            docker_version.c_str(),
            docker_type
        );
        out += buf;
        if (!wsl_distro.empty()) {
            sprintf(buf,
                ",\n        \"wsl_distro\": \"%s\"",
                wsl_distro.c_str()
            );
            out += buf;
        }
        if (bbr_version) {
            sprintf(buf,
                ",\n        \"boinc_buda_runner_version\": %d",
                bbr_version
            );
            out += buf;
        }
        out += "\n    }";
    }

    if (strlen(g_request->host.virtualbox_version)) {
        sprintf(buf, ",\n"\
"    \"vbox\": {\n"\
"        \"version\": \"%s\",\n"\
"        \"hw_accel\": %s,\n"\
"        \"hw_accel_enabled\": %s\n"\
"    }",
            g_request->host.virtualbox_version,
            (strstr(g_request->host.p_features, "vmx") || strstr(g_request->host.p_features, "svm"))?"true":"false",
            g_request->host.p_vm_extensions_disabled?"false":"true"
        );
        out += buf;
    }

    if (g_request->dont_use_docker || g_request->dont_use_wsl) {
        sprintf(buf, ",\n"\
"    \"config\": {\n"\
"        \"dont_use_docker\": %s,\n"\
"        \"dont_use_wsl\": %s\n"\
"    }",
            g_request->dont_use_docker?"true":"false",
            g_request->dont_use_wsl?"true":"false"
        );
        out += buf;
    }

    out += "\n}";
}
